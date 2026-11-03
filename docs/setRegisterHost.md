# setRegisterHost 实施说明

> 编号：IMPL-REGISTER-HOST-20261103 ｜ 适用：`3516-media-refactor` 的 `face_proc`
> 目的：让 PC 客户端能跨机拿到 `FaceResult`（不改框架，只在应用侧加一行）
> 结论：**在 `rt_.init()` 之后、启动任何服务/发布任何 topic 之前，调一次 `rt_.setRegisterHost("<板端可达IP>")`**

---

## 1. 不改会怎样

| 层 | 行为 | 后果 |
|---|---|---|
| `service_manager` | 监听 `0.0.0.0:9900` | ✅ PC 能连上注册中心，`lookupService` 能成功 |
| **服务/Topic 的 advertise 地址** | 被规范化成 `127.0.0.1` | ❌ PC 拿到的是**回环地址**，连的是自己 |
| 板内订阅（media_proc → face_proc） | 走 SHM，同机 | ✅ 完全正常 |

**这也是最容易误判的地方**：板端 OSD 一切正常、`omni-cli` 也能列出服务，但 PC 一订阅就超时。日志里能看到 `data_connect_fallback` 或连接超时——它 fallback 到的那个地址就是 `127.0.0.1`。

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

`resolveRegisterHost()` 的优先级（`src/core/runtime_service.cpp:87`）：

| 优先级 | 来源 | 设置方式 | 说明 |
|---|---|---|---|
| 1（最高） | Service 级 | `service->setRegisterHost(ip)` | 只影响这一个服务 |
| 2 | **Runtime 级** | **`rt_.setRegisterHost(ip)`** | 影响该 runtime 注册的**所有**服务与 topic |
| 3 | 监听地址推导 | 不设置 | 固定得到 `127.0.0.1`（当前状态） |

**结论：Runtime 级设一次就够了**，不必逐个服务设置。

---

## 3. 改哪里

位置：`3516-media-refactor/src/face_core/face_proc_app.cpp`，第 141 行附近。

```cpp
    // 现状（141 行）
    if (rt_.init("127.0.0.1", 9900) != 0) {
        /* ... 原有错误处理 ... */
        return -1;
    }

    /* ===== 新增：告诉 SM "别人应该用哪个地址连我" ===== */
    rt_.setRegisterHost(GetBoardAdvertiseIp());
    /* ================================================= */
```

`face_proc_app.cpp` 里 `Run()` 在 572 行，服务发布发生在 Run 之后，所以插在 141 行之后是安全的。

### 3.1 时机约束（必须遵守）

`advertise_host` 是在 **StartService 时**计算的（`runtime_service.cpp:79`），所以：

- ✅ **init 之后、服务启动之前** 设置 → 生效
- ❌ 服务已经注册完成后再设置 → **不会重新广告**，已经登记到 SM 的仍是旧地址；必须在重启进程或重新注册后才能生效

---

## 4. 不要硬编码：IP 怎么取

板子常有多个网卡/多个网段（eth0、wlan0、4G、虚拟网桥），**必须选与 PC 同网段的那个 IP**。推荐优先级：

```cpp
// 建议放在 face_core 里，作为一个小工具函数
static std::string GetBoardAdvertiseIp()
{
    // 1) 显式指定：部署脚本注入，最可控
    const char* env = getenv("FACE_REGISTER_HOST");
    if (env && env[0] != '\0') return std::string(env);

    // 2) 配置文件（与其它参数同一处，便于现场修改）
    //    cfg.advertise_ip

    // 3) 兜底：留空 —— 行为退化为 127.0.0.1（即"不支持跨机"）
    //    注意：不要在这里猜 IP，猜错比不设更难排查
    return std::string();
}
```

**为什么不建议自动探测**：用 `gethostname()` / 枚举网卡取到的"第一个非回环 IP"经常是虚拟网卡或 4G 网段，表面能设上、实际 PC 连不通，排查成本远高于一行环境变量。

**验证 IP 是否正确的最快方法**：在板子上看 `ip addr`，找与 PC 同网段的那张网卡。

---

## 5. 验证

### 5.1 板端

```bash
omni-cli info FaceService     # 看输出里的 host 字段：应为板 IP，不该是 127.0.0.1
omni-cli ps                   # 看 FaceResult 的发布者地址
```

### 5.2 端口说明（容易搞错）

advertise 里带的端口是**服务/数据面自己的 TCP 端口**（启动监听时动态分配），**不是 9900**。9900 只是 `service_manager` 的端口。

所以：
- 测 SM 通不通：`nc -vz <板IP> 9900`
- 测数据面通不通：**用 `omni-cli info` 打出来的实际端口**，不要拿 9900 去测

### 5.3 PC 端

```bash
nc -vz <板IP> 9900            # SM 可达
# 然后用 M1 探针：init → lookupService → subscribeTopic → 打印 seq
```

观察点：`subscribeTopic` 返回 0，且 3s 内能收到 FaceResult（包含 `det_count=0` 的心跳帧）。

### 5.4 日志关键字（板端）

```bash
grep -E "register_host|advertise|data_connect_fallback" /tmp/face.log /tmp/media.log
```

---

## 6. 影响范围与风险

| 项 | 影响 |
|---|---|
| 板内同机订阅（media_proc → face_proc） | **不受影响**：同机优先走 SHM，与 advertise 地址无关 |
| 服务监听 | **不改**：core 固定 `0.0.0.0` 绑定，无需改 bind |
| 其他进程（stream_proc / media_proc） | 不受影响：各自 runtime 独立，只有设了的那一个才变 |
| 设错 IP | 跨机连不上（表现为 PC 侧超时），板内仍一切正常 → 不会导致现场业务中断 |
| 回滚 | 删掉该行、重新部署即可恢复原状 |

> 一句话：**这是一个"只影响跨机可达性、不影响板内运行"的低风险改动**，最坏结果就是跨机仍然连不上。

---

## 7. 常见失败对照

| 现象 | 原因 | 处置 |
|---|---|---|
| `lookupService` 失败 | SM 不可达 / 服务未启动 | 先 `nc -vz 板IP 9900`；再 `omni-cli list` 看服务在不在 |
| `lookupService` 成功、订阅立即失败 | IDL hash 不一致（`topic_idl_mismatch`） | PC 端用**与板端同一份** `face_service.bidl` 重新生成 |
| 订阅成功但 3s 无任何数据 | ①advertise 未设/设错 ②face_proc 无发布者 | ①`omni-cli info` 看 host ②`omni-cli ps` 看发布者 |
| 板端 `omni-cli` 显示 host 仍是 `127.0.0.1` | 设置在服务注册之后 / 没重新部署 | 确认插入位置在 init 之后、服务启动之前，重启进程 |
| PC 端连接后立刻断开 | 数据面端口被防火墙拦 | 放通 `omni-cli info` 显示的实际端口 |
| 收到数据但 `seq` 不连续 | 正常：TCP 传输下偶尔丢帧由发布侧节流导致 | 用 `GetFrameCount()` 差值核对，不必处理 |

---

## 8. 与 PC 端方案的衔接

| PC 端动作 | 依赖本改动 |
|---|---|
| `rt.init(rtsp_host, 9900)` | 否（SM 本身监听 0.0.0.0） |
| `lookupService("FaceService")` | 否（注册信息在 SM，不涉及数据面地址） |
| `subscribeTopic("FaceResult")` | **是**（要用 advertise 的地址建数据面连接） |
| 收数据 | **是** |

也就是说：**不做这个改动，PC 能发现服务、但永远收不到数据**。这就是它被列为方案 A 硬前提的原因。

---

## 9. 备选：服务级设置

只希望影响 `FaceService` 而不影响该 runtime 下其它服务时：

```cpp
// 在 FaceService 实例注册前设置（service.h:53）
face_service_.setRegisterHost(GetBoardAdvertiseIp());
```

一般不需要，Runtime 级更省事。只有当同一个进程里既有"仅板内访问"的服务、又有"需要跨机"的服务时才用得上。

---

## 10. C API 形式（若某处只能用 C）

```c
omni_runtime_set_register_host(runtime, "192.168.10.168");   /* src/core/omnibinder_c.cpp:454 */
const char* h = omni_runtime_get_register_host(runtime);
```

`NULL` 或空串等价于"不设置"，行为退化为 `127.0.0.1`。

---

## 附：涉及文件

| 内容 | 位置 |
|---|---|
| 公开头（Runtime 级） | `OmniBinder/include/omnibinder/runtime.h:367` |
| 公开头（Service 级） | `OmniBinder/include/omnibinder/service.h:53` |
| 实现 | `OmniBinder/src/core/runtime_config.cpp:26` |
| advertise 计算 | `OmniBinder/src/core/runtime_service.cpp:79`、`:84` |
| 根因（归一化） | `OmniBinder/src/core/runtime_helpers.cpp:12` |
| 应用侧插入点 | `3516-media-refactor/src/face_core/face_proc_app.cpp:141` |
| 架构说明 | `OmniBinder/docs/architecture.md:255-257` |
