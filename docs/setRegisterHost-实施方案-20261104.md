# setRegisterHost 实施方案（详细）

| 项 | 内容 |
|----|------|
| 方案编号 | PLAN-REGISTER-HOST-20261104 |
| 依据 | `docs/setRegisterHost.md` v2（IMPL-REGISTER-HOST-20261103）+ `docs/setRegisterHost-审核报告-20261103.md` |
| 目标 | 让 **PC 客户端**经网络订阅 `face_proc` 的 `FaceResult`（板端 advertise 地址从 `127.0.0.1` 改为板端可达 IP） |
| 改动位置 | **仅板端** `3516-media-refactor/src/face_core/face_proc_app.cpp`（+1 个工具函数 +1 行调用 +1 行日志） |
| 不改 | 框架（`F:\git-clone\OmniBinder\`）、media_proc/stream_proc/ai_proc、IDL、PC 端已有代码 |
| 风险等级 | **低**（未设环境变量时行为与现状**完全一致**） |
| 预计工作量 | 代码 0.5h + 编译部署 0.5h + 联调 1h ≈ **2h** |

---

## 1. 目标与范围

### 1.1 要解决的问题

```
现状：face_proc 注册到 SM 的 advertise = 127.0.0.1
      ⇒ PC 能 lookupService（SM 监听 0.0.0.0:9900）
      ⇒ 但 subscribeTopic 时拿到的数据面地址是 127.0.0.1 → PC 连自己 → 超时
目标：advertise = 板端可达 IP（如 192.168.10.168）
      ⇒ PC 订阅成功、持续收到 FaceResult（含 det_count=0 的心跳帧）
```

### 1.2 范围内（做）

| # | 改动 | 文件 | 行数 |
|---|------|------|------|
| D1 | 新增 `GetBoardAdvertiseIp()`（读环境变量） | `face_proc_app.cpp` 匿名命名空间 | +12 |
| D2 | `Init()` 中 `rt_.init()` 之后、`registerService()` 之前调 `setRegisterHost` | 同上（141~147 之间） | +3 |
| D3 | advertise 结果日志（应用侧可观测） | 同上 | +1 |
| D4（可选） | 框架侧加 advertise INFO 日志（3 行，独立提交） | `OmniBinder/src/core/runtime_service.cpp:79` | +3 |

### 1.3 范围外（不做）

| 项 | 原因 |
|----|------|
| 改框架的 `normalizeAdvertiseHost` 默认逻辑 | 会影响所有使用者（含板内），风险不可控 |
| 自动探测本机 IP（`gethostname`/枚举网卡） | **板端网络由部署方预先配置好（固定 IP / 单网段），程序只消费一个明确地址**；引入探测只会带来"选错网卡"的额外失败模式 |
| 网段判断 / IP 合法性校验 | 同上——**网络可达性是部署前提**（见 §2.0），程序不判断、不校验、不打日志猜测 |
| 配置文件方案（`cfg.advertise_ip`） | MVP 不需要；多实例需求出现再引入 |
| media_proc / stream_proc 也设 advertise | 它们的订阅方都在板内（走 SHM），无需跨机 |
| 改 IDL / 改 PC 端已有代码 | 无关 |

---

## 2. 现状分析（改动前的代码事实）

```cpp
// src/face_core/face_proc_app.cpp（当前）
141    if (rt_.init("127.0.0.1", 9900) != 0) {      // 连本机 SM
142        MIPC_LOG_ERROR("OmniRuntime init fail");
143        finfer_.Deinit();
144        omni_mmz_sys_detach();
145        return -1;
146    }
147    rt_.registerService(&fsvc_);                  // ★ advertise 在此计算并上报 SM
148    fsvc_.PublishFaceResult();                    // ★ topic 发布者地址同样在此计算
149    fsvc_.SetReady(true);
```

**advertise 计算链**（框架侧，只读）：
```cpp
// runtime_service.cpp:79
advertise_host = resolveRegisterHost(service, listenerAdvertiseHost());

// runtime_service.cpp:84  →  listenerAdvertiseHost() = normalizeAdvertiseHost("0.0.0.0")
// runtime_helpers.cpp:13  →  返回 "127.0.0.1"           ← 根因
```

**优先级**（`runtime_service.cpp:resolveRegisterHost`，已逐行核对）：
```
1) service->getRegisterHost()  非空 → 用它
2) runtime.register_host_      非空 → 用它     ← 本方案走这条
3) normalizeAdvertiseHost(listener_host) → 127.0.0.1
```

⇒ **只要在 141~147 之间设置 Runtime 级 host，且设置值非空**，后续 `registerService` 与 `PublishFaceResult` 都会用新地址。

### 2.0 前提假设（本方案的边界条件）

| # | 假设 | 由谁保证 |
|---|------|---------|
| A1 | **板端网络已预先配置好**：一个固定的、PC 可达的 IP（单网段，无多网卡歧义） | **部署方**（现场网络规划） |
| A2 | 该 IP 通过 `FACE_REGISTER_HOST` 传给进程（部署脚本/起栈脚本注入） | 部署方 |
| A3 | 板与 PC 之间路由/防火墙放通（SM 端口 + 动态数据面端口） | 部署方 |

> **程序侧的职责只有一件**：把 A2 给的值原样交给框架。
> **不做**：探测本机 IP、判断哪个网卡、校验 IP 格式、猜测网段——这些都属 A1/A2，程序介入只会增加失败模式与排查成本。
> 若 A1/A2 不成立（IP 设错/未注入），表现为"跨机不通但板内正常"，由 §6 的 V1（日志打印 `<unset>` 或错误 IP）当场暴露。

### 2.1 关键时序约束（必须理解）

| 操作 | 效果 |
|------|------|
| 在 141（init）**之前**调用 | ⚠ 技术上可行但**不建议**：`impl_` 在 `OmniRuntime` 构造时已创建（`omni_runtime.cpp:21`），`init()` 不重置 `register_host_`（`:237` 起），故值会保留。但语义上不应依赖"未初始化的 runtime"，统一写在 init 之后更清晰、便于 review |
| **在 141~147 之间调用** | ✅ **生效**（registerService 尚未发生） |
| 在 147 之后（含 Init 末尾、Run 内）调用 | ❌ **无效**（已注册，不会重新广告；SM 里仍是旧值） |

---

## 3. 详细设计

### 3.1 D1：`GetBoardAdvertiseIp()` 工具函数

**放置位置**：`face_proc_app.cpp` 文件顶部已有匿名命名空间（`namespace { ... }`，与 `NowUs()`/`InferStageName()` 等工具函数同处）。

**完整代码**（可直接粘贴）：

```cpp
namespace {

/** 读取"对外广告地址"：由部署脚本注入环境变量 FACE_REGISTER_HOST。
 *
 * 设计取舍：
 *  - 只做环境变量（不自动探测、不判断网段、不加配置项）——板端网络由部署方预先配置好，
 *    程序只消费一个明确地址（见 PLAN-REGISTER-HOST-20261104 §2.0 假设 A1/A2）；
 *  - 未设置（空串）⇒ 返回空串 ⇒ 框架退化为 127.0.0.1（= 现状，板内行为不变）；
 *  - 不做 IP 合法性校验（IPv4 字符串/主机名/域名由使用者自负）；非法值会导致跨机连不上，
 *    但板内 SHM 路径不受影响（回滚只需去掉环境变量）。
 *  - getenv 只在 Init 调一次（结果作为参数传入），运行期不再读取（避免环境被改导致前后不一致）。
 */
std::string GetBoardAdvertiseIp() {
    const char* env = getenv("FACE_REGISTER_HOST");
    if (env == nullptr || env[0] == '\0') return std::string();
    return std::string(env);
}

}  // namespace
```

**边界与异常**：

| 输入 | 返回 | 结果 |
|------|------|------|
| 未设置 | `""` | `setRegisterHost("")` → 框架按"未设置"处理 → advertise=127.0.0.1（**与现状完全一致**） |
| `FACE_REGISTER_HOST=""` | `""` | 同上 |
| `FACE_REGISTER_HOST=192.168.10.168` | 该串 | advertise=192.168.10.168 ✅ |
| `FACE_REGISTER_HOST=abc` | `"abc"` | advertise=abc → PC 解析失败（跨机不通），板内仍正常；`omni-cli info` 可见，便于发现 |
| 超长串（>64 字节） | 原样返回 | 框架内部为 `std::string`，无截断风险 |
| 地址属于其他网段 | 原样返回 | **程序不判断**（A1 由部署方保证）；现象=跨机不通、板内正常，V1 日志可直接看到值 |

> **内存安全**：`std::string` 值返回，无 `strdup`/裸指针；`getenv` 返回值只在函数内使用，不做持久化引用。

### 3.2 D2：在 `Init()` 中插入调用

**精确位置**：`face_proc_app.cpp` **141（`rt_.init` 的 `}` 之后）～147（`rt_.registerService`）之间**。

**修改后的完整片段**（替换 141~149 这一段）：

```cpp
    if (rt_.init("127.0.0.1", 9900) != 0) {
        MIPC_LOG_ERROR("OmniRuntime init fail");
        finfer_.Deinit();
        omni_mmz_sys_detach();
        return -1;
    }

    /* 【PLAN-REGISTER-HOST-20261104 D2】advertise 地址：必须在 registerService 之前设置，
     * 否则（147 行之后）已注册完成、SM 里仍是 127.0.0.1，设置静默失效。
     * 未设置环境变量时传空串 ⇒ 框架退化为 127.0.0.1（与改动前行为完全一致）。 */
    const std::string advertise_ip = GetBoardAdvertiseIp();
    rt_.setRegisterHost(advertise_ip);
    MIPC_LOG_INFO("[face] advertise host=%s (env FACE_REGISTER_HOST=%s)",
                  advertise_ip.empty() ? "<unset:127.0.0.1>" : advertise_ip.c_str(),
                  advertise_ip.empty() ? "unset" : "set");

    rt_.registerService(&fsvc_);
    fsvc_.PublishFaceResult();
    fsvc_.SetReady(true);
```

**为什么先取到局部变量再传**：
- 一次 `getenv` + 一次日志，避免重复调用；
- 日志在**注册前**打印 → 与 `omni-cli info` 的输出可以直接对照（顺序清晰）；
- `advertise_ip` 是 `Init()` 局部变量，生命周期覆盖后续所有注册调用 ✓。

### 3.3 D3：日志设计

```
未设置：  [face] advertise host=<unset:127.0.0.1> (env FACE_REGISTER_HOST=unset)
已设置：  [face] advertise host=192.168.10.168 (env FACE_REGISTER_HOST=set)
```

- 用 `INFO` 级（不是 WARN）——正常配置路径；
- **不要打印环境变量原值**（避免特殊字符/超长串污染日志），只打印 `set/unset`；
- 该行是**应用侧唯一**能直接看到 advertise 的通道（框架侧无日志，见 D4）。

### 3.4 D4（可选，推荐）：框架侧 advertise 日志

**位置**：`OmniBinder/src/core/runtime_service.cpp`，`advertise_host` 计算之后（约 79~80 行）：

```cpp
    advertise_host = resolveRegisterHost(service, listenerAdvertiseHost());
    /* 【2026-11-04】跨机排查唯一直接证据：注册时实际广告的地址与端口 */
    OMNI_LOG_INFO(LOG_TAG, "service %s advertise %s:%u",
                  service->name().c_str(), advertise_host.c_str(), entry->port);
```

| 项 | 说明 |
|----|------|
| 收益 | 以后所有"跨机连不上"问题，一眼看到 advertise 值与端口 |
| 成本 | 3 行；`LOG_TAG`/`OMNI_LOG_INFO` 该文件已用 |
| 风险 | 极低（只在服务注册时打印一次，非热路径） |
| 提交 | **独立提交**，与业务改动分离（框架仓 vs 业务仓） |

---

## 4. 完整 diff（业务仓）

```diff
--- a/src/face_core/face_proc_app.cpp
+++ b/src/face_core/face_proc_app.cpp
@@ (匿名命名空间，与 NowUs/InferStageName 等同处)
 namespace {
+
+/** 读取对外广告地址（部署脚本注入 FACE_REGISTER_HOST）；空串=不设，框架退化为 127.0.0.1 */
+std::string GetBoardAdvertiseIp() {
+    const char* env = getenv("FACE_REGISTER_HOST");
+    if (env == nullptr || env[0] == '\0') return std::string();
+    return std::string(env);
+}
+
 ...（原有工具函数）
 }  // namespace

@@ Init() 内 141~149
     if (rt_.init("127.0.0.1", 9900) != 0) {
         MIPC_LOG_ERROR("OmniRuntime init fail");
         finfer_.Deinit();
         omni_mmz_sys_detach();
         return -1;
     }
+
+    /* 【PLAN-REGISTER-HOST-20261104】advertise 必须在 registerService 之前设置 */
+    const std::string advertise_ip = GetBoardAdvertiseIp();
+    rt_.setRegisterHost(advertise_ip);
+    MIPC_LOG_INFO("[face] advertise host=%s (env FACE_REGISTER_HOST=%s)",
+                  advertise_ip.empty() ? "<unset:127.0.0.1>" : advertise_ip.c_str(),
+                  advertise_ip.empty() ? "unset" : "set");
+
     rt_.registerService(&fsvc_);
```

**头文件**：`<string>` 已包含（`face_proc_app.cpp` 已用 `std::string`），`<cstdlib>` 需确认（`getenv`）——若未包含则补 `#include <cstdlib>`。

---

## 5. 编译与部署（含前提检查）

### 5.1 本地 → VM → 编译

```bash
# ① 本地改代码后同步到 VM（按 AGENTS §2.1；只同步改动文件更快）
rsync -az src/face_core/face_proc_app.cpp \
  ebaina@192.168.10.96:/home/ebaina/Workspace/3516-media-refactor/src/face_core/

# ② VM 上编译（工程根执行，路径不能省 scripts/）
ssh ebaina@192.168.10.96
cd /home/ebaina/Workspace/3516-media-refactor
bash scripts/build_hi3516.sh > logs/build_register_host.log 2>&1
echo "exit=$?"          # 必须 0
grep -cE "error:|warning:" logs/build_register_host.log   # 期望 0（除并行会话既有 warning）
```

### 5.2 板端起栈（**含前提检查**，审核报告 P1）

```bash
cd /root/nfs/3516-media-refactor/dist/hi3516/bin
export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH

# ⚠ 前提：SM 与 media_proc 必须已在跑——否则 face_proc 无帧可推理，
#    PC 能订阅但永远收不到 FaceResult（会误判为"跨机失败"）
pgrep -x service_manager >/dev/null || { echo "先起 service_manager"; exit 1; }
pgrep -x media_proc      >/dev/null || { echo "先起 media_proc";      exit 1; }

# 停旧进程 → 用环境变量启动（参数与平时起栈一致，只多一个 env）
# FACE_REGISTER_HOST = 现场规划好的板端 IP（A1/A2 由部署方保证，程序不校验）
kill $(pidof face_proc) 2>/dev/null; sleep 1

# ⚠ face_proc **没有单实例锁**（全仓无 flock/lockf/O_EXCL/"already running" 检查）。
#   必须复核旧进程已退出：否则新旧两个实例同时向 SM 注册同名服务，
#   advertise 可能仍是旧值，现象是"日志说设了、omni-cli 还是 127.0.0.1"，极难定位。
pgrep -x face_proc >/dev/null && { echo "旧 face_proc 仍在运行，先处理再启动"; exit 1; }

FACE_REGISTER_HOST=192.168.10.168 ./face_proc \
  /root/nfs/faceRec/insightface/models/cheetah_s/det_500m.om \
  /root/nfs/faceRec/insightface/models/cheetah_s/w600k_mbf.om \
  /root/nfs/faceRec/insightface/w3_demo/mbf_recog/build/bin/dw-workers 0.7 0.5 \
  > /tmp/face.log 2>&1 &
```

> 如果现场用固定脚本起栈（`scripts/run_phase1_board.sh` 等），则在脚本里 export，不要每次手敲。

---

## 6. 验证方案

### 6.1 板端（3 步）

| # | 命令 | 期望 |
|---|------|------|
| V1 | `grep "advertise host" /var/log/media_ipc/face_proc.log`（若用 stdout 重定向起栈，也可 grep `/tmp/face.log`） | `advertise host=192.168.10.168 (env FACE_REGISTER_HOST=set)` |
| V2 | `omni-cli info FaceService` | `Host: 192.168.10.168`（**不是** 127.0.0.1） |
| V3 | `omni-cli ps` | `FaceResult` 发布者地址 = 板 IP（端口为动态分配的数据面端口，**非 9900**） |

### 6.2 PC 端（3 步）

| # | 命令 | 期望 |
|---|------|------|
| V4 | `nc -vz 192.168.10.168 9900` | `succeeded`（SM 可达） |
| V5 | `omni-cli -h 192.168.10.168 info FaceService` | 同上 Host=板 IP（PC 上必须带 `-h`） |
| V6 | 客户端探针：`init(板IP,9900) → connect → SubscribeFaceResult(cb)` | **3s 内收到 FaceResult**（`det_count=0` 的心跳帧也算通） |

### 6.3 回归（2 步，证明没破坏板内）

| # | 检查 | 期望 |
|---|------|------|
| R1 | live2 拉流看 OSD | 框/名字/性能条正常（media_proc 订阅未受影响；走 SHM） |
| R2 | `grep -E "data_connect_fallback|Service died" /var/log/media_ipc/media_proc.log`（或 `/tmp/media.log`） | 无新增 fallback（若出现，说明 SHM 异常，见风险 §7.2） |

### 6.4 失败判定表

| 现象 | 归属 | 处置 |
|------|------|------|
| V1 日志里 host=`<unset:...>` | env 没传进去 | 检查起栈脚本/`su`/`&` 环境继承 |
| V1 有 host，V2 仍 127.0.0.1 | 设置发生在注册之后 | 确认插入点在 147 之前 |
| V2 正确，V6 超时 | PC 侧网络/防火墙 | `nc -vz 板IP <omni-cli info 里的端口>` |
| V2 正确，V6 立即失败 | IDL hash 不一致 | PC 用同一份 `face_service.bidl` 重新生成 |
| V6 收到但 `seq` 跳变 | `ring_drop`（结果环满/消费慢） | 看 `ring_drop=` 日志，不是配置问题 |

---

## 7. 风险与回滚

### 7.1 风险矩阵

| 风险 | 概率 | 影响 | 缓解 |
|------|------|------|------|
| env 未传导致"以为改了其实没改" | 中 | 跨机仍不通，但**板内无影响** | D3 日志明确打印 `<unset>`；V1 必查 |
| IP 设错（非本机/不通网段） | 中 | 跨机不通；板内正常 | V2/V4 可当场发现；回滚=去掉 env |
| 设置时机错（放 147 之后） | 低 | 静默失效 | 本方案给出精确插入点 + §3.1 时序表 |
| 板内 SHM 不可用 → fallback TCP（用 advertise） | 低 | 若 IP 错则**板内 OSD 也挂** | §6.3 R2 检查；advertise 设对则无影响 |
| 影响同进程其他服务 | 极低 | face_proc 仅 FaceService+FaceResult ✓ | 若将来加"仅板内"服务 → 改 Service 级（§9） |
| 框架侧 D4 日志引入噪音 | 极低 | 每服务注册一次 INFO | 仅注册时打印，非热路径 |

### 7.2 已知边界情形

- **SHM 失败 fallback**：`/dev/shm` 满或残留段 ⇒ 同机订阅走 TCP（用 advertise 地址）。此时 **advertise 必须正确**，否则板内 OSD 也会挂。排查时优先看 `data_connect_fallback`。
- **face_proc 重启**：SM 里会重新登记新 advertise；**PC 端不需要手动重订阅**——框架在 SM 重连时会遍历已订阅 topic 自动重放（`runtime_sm.cpp:362-371` 调 `restoreSubscribeTopic()`），发布方重启时 SM 还会回推 publisher notify 重建数据通路（`runtime_topic.cpp:339` 注释）；重复订阅按幂等处理，只告警（`runtime_topic.cpp:344-352`）。PC 端只需观察数据是否恢复，30s 未恢复再走掉线重连。
- **多实例**：⚠ **face_proc 没有单实例保护**（全仓无 flock/O_EXCL/"already running" 检查）。启动前必须确认旧进程已退出（见 §5.2 的 `pgrep` 复核），否则两个实例同时注册同名服务，advertise 行为不可预期。将来多板接入时，PC 端按不同 SM 地址区分。

### 7.3 回滚

| 层级 | 操作 | 恢复时间 |
|------|------|---------|
| 最快 | 不传 `FACE_REGISTER_HOST` 重启 face_proc | <10s |
| 代码级 | `git revert <该提交>` → 重编 | ~15min |
| 影响面 | 板内功能**全程不受影响** | — |

---

## 8. 工作量与交付物

| 项 | 内容 | 工时 |
|----|------|------|
| 代码（D1~D3） | ~16 行 | 0.5h |
| 编译 + 部署 | build + 上板 | 0.5h |
| 板端验证（V1~V3 + R1~R2） | — | 0.3h |
| PC 端验证（V4~V6） | 用 PC 探针程序 | 0.5h |
| （可选）D4 框架日志 | 3 行 + 框架重编 | 0.3h |
| **合计** | | **≈2h** |

**交付物**：
1. `face_proc_app.cpp` 改动（1 个提交，含完整日志）；
2. （可选）`runtime_service.cpp` 框架日志（独立提交）；
3. 验证记录（V1~V6 / R1~R2 的实际输出，回填到本文档 §6 的表格）。

---

## 9. PC 端配套（简要）

| 项 | 说明 |
|----|------|
| 连接 | `rt.init("192.168.10.168", 9900)`（SM 监听 0.0.0.0，无需改） |
| 发现 | `lookupService("FaceService")` 或直接用生成代理 `connect()` |
| 订阅 | **`proxy.SubscribeFaceResult(cb)`**（用生成代码，带类型 + IDL hash 校验） |
| 回调 | 只入队/打印，**禁止**同步 RPC/写盘/重活（硬约束⑤） |
| 必需 | 事件循环泵 `rt.pollOnce(ms)`，否则收不到回调 |
| 详细 | 见 `docs/PC客户端订阅FaceResult-对接文档-20261103.md` |

---

## 10. 验收标准（Definition of Done）

- [ ] `grep "advertise host" /tmp/face.log` 输出板 IP
- [ ] `omni-cli info FaceService` 的 `Host` = 板 IP（非 127.0.0.1）
- [ ] `omni-cli ps` 中 `FaceResult` 发布者为板 IP
- [ ] PC 端 3s 内收到 FaceResult（含 `det_count=0` 心跳帧）
- [ ] 未设置 env 时：`advertise host=<unset:127.0.0.1>`，PC 不可达，**板内一切正常**（证明改动可安全回退）
- [ ] 板内 OSD（live2 框/名字/性能条）无变化
- [ ] 板端 `pgrep -x face_proc` 只有**一个**实例（face_proc 无单实例锁，见 §5.2）
- [ ] `git status` 只含本方案涉及文件；提交信息注明"未设 env 时行为与改动前一致"
