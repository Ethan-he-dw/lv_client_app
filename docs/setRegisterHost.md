# setRegisterHost 实施说明

> 编号：IMPL-REGISTER-HOST-20261103 ｜ 修订：v2（按 `setRegisterHost-审核报告-20261103.md` 修正 4 处错误 + 5 处遗漏）
> 适用：`3516-media-refactor` 的 `face_proc`
> 目的：让 PC 客户端能跨机拿到 `FaceResult`（不改框架，只在应用侧加几行）
> 结论：**在 `rt_.init()` 之后、`rt_.registerService()` 之前，调一次 `rt_.setRegisterHost("<板端可达IP>")`**

---

## 1. 不改会怎样

| 层 | 行为 | 后果 |
|---|---|---|
| `service_manager` | 监听 `0.0.0.0:9900` | ✅ PC 能连上注册中心，`lookupService` 能成功 |
| **服务/Topic 的 advertise 地址** | 被规范化成 `127.0.0.1` | ❌ PC 拿到的是**回环地址**，连的是自己 |
| 板内订阅（media_proc → face_proc） | 正常走 SHM，同机 | ✅ 正常（**例外见 §6 G3**） |

**这也是最容易误判的地方**：板端 OSD 一切正常、`omni-cli` 也能列出服务，但 PC 一订阅就超时。日志里能看到 `data_connect_fallback`——它 fallback 到的那个地址就是 `127.0.0.1`。

---

## 2. 原理：为什么一定是 127.0.0.1

三级调用链：

```cpp
// ① src/core/runtime_service.cpp:84 —— 监听地址被硬编码成通配地址
std::string OmniRuntime::Impl::listenerAdvertiseHost() const {
    return normalizeAdvertiseHost("0.0.0.0");
}

// ② src/core/runtime_helpers.cpp:12 —— 通配地址被归一成回环
std::string normalizeAdvertiseHost(const std::string& host) {
    if (host.empty() || host == "0.0.0.0") return "127.0.0.1";   // ← 根因
    return host;
}

// ③ src/core/runtime_service.cpp:79 —— 启动服务时才计算 advertise
advertise_host = resolveRegisterHost(service, listenerAdvertiseHost());
```

`resolveRegisterHost()` 的优先级：

| 优先级 | 来源 | 设置方式 | 说明 |
|---|---|---|---|
| 1（最高） | Service 级 | `fsvc_.setRegisterHost(ip)` | 只影响这一个服务 |
| 2 | **Runtime 级** | **`rt_.setRegisterHost(ip)`** | 影响该 runtime 注册的**所有**服务与 topic（见 §6 G2） |
| 3 | 监听地址推导 | 不设置 | 固定得到 `127.0.0.1`（当前状态） |

**结论：Runtime 级设一次就够了**，不必逐个服务设置。

---

## 3. 改哪里（精确到行，位置错了会静默失效）

⚠ **服务注册与 topic 发布都发生在 `Init()` 内，不是 `Run()` 之后。** 实际顺序（`src/face_core/face_proc_app.cpp`）：

```
141   rt_.init("127.0.0.1", 9900)
147   rt_.registerService(&fsvc_)          ← advertise 在这里被计算
148   fsvc_.PublishFaceResult()
149   fsvc_.SetReady(true)
572   void FaceProcApp::Run()              ← 只是事件循环
```

正确改法——**插在 141 与 147 之间**：

```cpp
    // 141
    if (rt_.init("127.0.0.1", 9900) != 0) {
        MIPC_LOG_ERROR("OmniRuntime init fail");
        finfer_.Deinit();
        omni_mmz_sys_detach();
        return -1;
    }

    /* ★ 新增：必须在 registerService（147）之前 */
    rt_.setRegisterHost(GetBoardAdvertiseIp());

    rt_.registerService(&fsvc_);        // 147
    fsvc_.PublishFaceResult();          // 148
    fsvc_.SetReady(true);               // 149
```

### 3.1 时机约束（必须遵守）

`advertise_host` 只在 **startService 路径**计算（`runtime_service.cpp:79`），所以：

- ✅ 在 `rt_.init()` 之后、`registerService()` 之前设置 → 生效
- ❌ 放到 `Init()` 末尾（150 行之后）或 `Run()` 里 → **不生效**，SM 里登记的仍是旧值
- ❌ 服务注册完成后再设置 → 不会重新广告，必须重启进程

这两条是同一个约束的两面：**设置必须早于第一次注册**。

---

## 4. 不要硬编码：IP 怎么取

板子常有多个网卡/网段（eth0、wlan0、4G、虚拟网桥），**必须选与 PC 同网段的那个 IP**。

**MVP 只做环境变量**（部署脚本注入），配置文件方案列为后续，不要求两套都做：

```cpp
// 建议放在 face_core 里作为小工具函数
static std::string GetBoardAdvertiseIp()
{
    const char* env = getenv("FACE_REGISTER_HOST");   // 部署脚本注入，最可控
    if (env && env[0] != '\0') return std::string(env);

    return std::string();   // 留空 ⇒ 行为退化为 127.0.0.1（即"不支持跨机"）
}
```

**为什么不自动探测**：`gethostname()` / 枚举网卡取到的"第一个非回环 IP"经常是虚拟网卡或 4G 网段，表面能设上、实际 PC 连不通，排查成本远高于一行环境变量。

**为什么不现在做配置文件**：多实例/多网卡场景还没出现，加一层配置只会增加现场排错路径。等有多实例需求再引入。

**验证 IP 是否正确的最快方法**：板子上 `ip addr`，找与 PC 同网段的那张网卡。

---

## 5. 验证

### 5.1 板端

```bash
omni-cli info FaceService
# 看输出里的 Host 字段：应为板 IP，不该是 127.0.0.1
# 输出位置：tools/omni_cli/main.cpp:204  printf("  Host: %s\n", info.host.c_str());
```

### 5.2 PC 端

**`omni-cli` 默认连 `127.0.0.1`**（`tools/omni_cli/main.cpp:926`），在 PC 上必须显式指定 SM 地址：

```bash
omni-cli -h <板IP> info FaceService
#        ↑ -h 是 ServiceManager 地址，不是数据面端口
```

### 5.3 端口说明（容易搞错）

advertise 里带的端口是**服务/数据面自己的 TCP 端口**（启动监听时动态分配），**不是 9900**。9900 只是 SM 的端口。

- 测 SM 通不通：`nc -vz <板IP> 9900`
- 测数据面通不通：**用 `omni-cli info` 打出来的实际端口**

### 5.4 日志（注意：框架当前没有 advertise 日志）

```bash
grep -E "data_connect_fallback|Service died" /tmp/face.log /tmp/media.log
```

> 全仓核对：`advertise` / `register_host` **没有**任何日志输出（仅 `runtime_topic.cpp:124` 一条 WARN）。所以**不能**用这两个关键字验证改动是否生效，只能靠 §5.1 的 `omni-cli`。

**建议（框架侧，3 行，强烈推荐）**：在 `runtime_service.cpp:79` 之后加一行 INFO——

```cpp
OMNI_LOG_INFO(LOG_TAG, "service %s advertise %s:%u",
              service->name().c_str(), advertise_host.c_str(), entry->port);
```

理由：这是唯一能直接看到 advertise 结果的通道，以后所有跨机问题都靠它一眼定性。

---

## 6. 影响范围与风险

### G2 · Runtime 级设置会影响本进程注册的所有服务/topic

`rt_.setRegisterHost()` 对 `registerService()` 注册的每一项都生效。当前 face_proc 只有 **`FaceService` + `FaceResult`** 两项，所以无副作用。

⚠ 将来若在同一进程里加"只允许板内访问"的服务，会被连带暴露到网络——那时改用 Service 级设置（§9）。

### G3 · 同机 SHM 失败会 fallback TCP，此时**板内也会受影响**

"板内完全不受影响"这个说法**过于绝对**。实测出现过：

```
shm_open failed → data_connect_fallback TCP
```

`/dev/shm` 满、上次异常退出留下残留段时，同机订阅会退化为走 advertise 地址。**该场景下若 advertise 设错，板内 OSD 也会挂**——虽然罕见，但排查时应优先想到它。

### 其他

| 项 | 影响 |
|---|---|
| 服务监听 | **不改**：core 固定 `0.0.0.0` 绑定，无需改 bind |
| 其他进程（stream_proc / media_proc） | 不受影响：各自 runtime 独立 |
| 设错 IP | 跨机连不上（PC 超时），板内正常路径不受影响（G3 场景除外） |
| 回滚 | 删掉该行、重新部署即可 |

---

## 7. 常见失败对照

| 现象 | 原因 | 处置 |
|---|---|---|
| `lookupService` 失败 | SM 不可达 / 服务未启动 | `nc -vz 板IP 9900`；`omni-cli list` |
| `lookupService` 成功、订阅立即失败 | IDL hash 不一致（`topic_idl_mismatch`） | PC 端用**与板端同一份** `face_service.bidl` 重新生成 |
| 订阅成功但 3s 无数据 | ①advertise 未设/设错 ②无发布者 | ①`omni-cli info` 看 Host ②`omni-cli ps` 看发布者 |
| 板端 `omni-cli` 显示 Host 仍是 `127.0.0.1` | 设置在注册之后 / **没重新部署或没重启进程** | 见 §8 部署步骤 |
| PC 连接后立刻断开 | 数据面端口被防火墙拦 | 放通 `omni-cli info` 显示的实际端口 |
| `seq` 不连续 | **`FaceResultRing` 取不到槽 → `ring_drop`**（结果环满/消费慢），不是"发布侧节流" | 看 `ring_drop=` 日志（`face_proc_app.cpp:343`）；查消费端与环深，不要去调发布频率 |

---

## 8. 实施与部署步骤（照这个顺序做）

```bash
# 1) 代码修复（本文件 §3 / §4 / §9）
# 2) 编译
./build_hi3516.sh            # 或项目既有板端构建脚本

# 3) 部署 + 重启（不重启 = 改动不生效，omni-cli 仍显示 127.0.0.1）
#    先停 face_proc，替换二进制，再起
FACE_REGISTER_HOST=192.168.10.168 ./face_proc &

# 4) 板端确认
omni-cli info FaceService           # Host 应为板 IP

# 5) PC 端确认
omni-cli -h 192.168.10.168 info FaceService
nc -vz 192.168.10.168 9900

# 6) PC 端探针：init → connect → SubscribeFaceResult → 3s 内应收到 seq
# 7) 回归：板内 OSD 是否仍正常（media_proc 订阅未受影响）
```

---

## 9. 备选：服务级设置

只希望影响 `FaceService` 而不影响该 runtime 下其它服务时，用**真实成员名 `fsvc_`**：

```cpp
fsvc_.setRegisterHost(GetBoardAdvertiseIp());   // face_proc_app.h:181 的 FaceServiceImpl fsvc_
```

（注意：原文档误写为 `face_service_`，该成员不存在，照抄编译不过。）

一般不需要，Runtime 级更省事。只有当同进程里既有"仅板内"又有"需要跨机"的服务时才用。

---

## 10. C API 形式

```c
omni_runtime_set_register_host(runtime, "192.168.10.168");   /* src/core/omnibinder_c.cpp:454 */
omni_service_set_register_host(service, "192.168.10.168");   /* src/core/omnibinder_c.cpp:424 */
```

`NULL` 或空串等价于"不设置"，行为退化为 `127.0.0.1`。

---

## 11. PC 端衔接（含两条约束）

| PC 端动作 | 依赖本改动 | 备注 |
|---|---|---|
| `rt.init(rtsp_host, 9900)` | 否 | SM 本身监听 0.0.0.0 |
| `lookupService("FaceService")` | 否 | 只查注册信息，不涉及数据面地址 |
| **`proxy.SubscribeFaceResult(cb)`** | **是** | **用生成代码的 API**，不要手写 `subscribeTopic("FaceResult")`——生成代码带类型检查与 IDL hash 校验 |
| 收数据 | **是** | — |

⚠ **回调约束（框架硬约束⑤）**：`SubscribeFaceResult` 的回调里**禁止同步 RPC / 写盘 / 重活**，否则 `Re-entrant waitForReply` 死锁。详见对接文档 `PC客户端订阅FaceResult-对接文档-20261103.md` §9 第 6 条。

---

## 附：涉及文件

| 内容 | 位置 |
|---|---|
| 公开头（Runtime 级） | `OmniBinder/include/omnibinder/runtime.h:367` |
| 公开头（Service 级） | `OmniBinder/include/omnibinder/service.h:53` |
| 实现 | `OmniBinder/src/core/runtime_config.cpp:26` |
| advertise 计算 | `OmniBinder/src/core/runtime_service.cpp:79`、`:84` |
| 根因（归一化） | `OmniBinder/src/core/runtime_helpers.cpp:12` |
| **应用侧插入点** | `3516-media-refactor/src/face_core/face_proc_app.cpp:141~147` |
| Service 成员 | `.../face_proc_app.h:181`（`FaceServiceImpl fsvc_;`） |
| omni-cli 默认地址 | `OmniBinder/tools/omni_cli/main.cpp:926` |
| omni-cli Host 输出 | `OmniBinder/tools/omni_cli/main.cpp:204` |
| 结果环丢弃 | `.../face_core/face_proc_app.cpp:309-343` |
| 架构说明 | `OmniBinder/docs/architecture.md:255-257` |
| **本文件的审核报告** | `docs/setRegisterHost-审核报告-20261103.md` |
