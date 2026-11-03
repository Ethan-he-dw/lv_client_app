# 《setRegisterHost 实施说明》审核报告

| 项 | 内容 |
|----|------|
| 被审文档 | `M:\LVGL_learn\lv_client_app\docs\setRegisterHost.md`（编号 IMPL-REGISTER-HOST-20261103） |
| 审核对象 | 需求/实施说明的技术正确性、可执行性、风险完整性 |
| 审核方法 | **逐条对照框架源码核对**（`F:\git-clone\OmniBinder\`：`runtime_service.cpp` / `runtime_config.cpp` / `runtime_helpers.cpp` / `omnibinder_c.cpp` / `tools/omni_cli`），并核对被改文件实际行号与成员名 |
| 审核结论 | **主干正确、可执行**；发现 **4 处必须修正的错误**（含 1 处代码级错误）、**2 处表述不准**、**5 处遗漏** |
| 审核日期 | 2026-11-03 |

---

## 一、总体评估

| 维度 | 评分 | 说明 |
|------|------|------|
| **技术准确性** | ★★★★☆（85%） | 根因、优先级链、时机约束、端口区分、影响面判断**全部正确**；错误集中在"成员名 / 插入点理由 / 验证手段"三类实施细节 |
| **可执行性** | ★★★★☆ | 补齐 §四 的 4 项修正后即可照做，无结构性缺口 |
| **风险意识** | ★★★★★ | §4"不猜 IP"、§5.2 端口区分、§6 影响面、§7 失败对照——均超出一般实施文档水平 |
| **完整性** | ★★★☆☆ | 缺"部署/重启步骤"、缺"本进程其他服务受影响"、缺"同机 SHM fallback 场景" |

**一句话结论**：**可以直接作为实施依据，但必须先改 §二 的 4 处**——其中 §2-1（成员名不存在）和 §2-2（插入点理由错）会直接导致实施失败。

---

## 二、必须修正（4 处，含 1 处代码级错误）

### M1【代码级错误】§9 服务级设置用了不存在的成员名

| 项 | 内容 |
|----|------|
| 原文 | `face_service_.setRegisterHost(GetBoardAdvertiseIp());` |
| 实际 | face_proc 的 Service 成员名是 **`fsvc_`**：`FaceServiceImpl fsvc_;`（`src/face_core/face_proc_app.h:181`），且在 `face_proc_app.cpp:147` 以 `rt_.registerService(&fsvc_)` 注册 |
| 后果 | 照抄编译不过 |
| **修正** | `fsvc_.setRegisterHost(GetBoardAdvertiseIp());` |

### M2【理由错误，结论仍成立】§3 称"服务发布发生在 Run() 之后"

| 项 | 内容 |
|----|------|
| 原文 | 「`face_proc_app.cpp` 里 `Run()` 在 572 行，服务发布发生在 Run 之后，所以插在 141 行之后是安全的」 |
| 实际 | **服务注册与 topic 发布都在 `Init()` 内**：<br>· `141` `rt_.init("127.0.0.1", 9900)`<br>· `147` `rt_.registerService(&fsvc_)`<br>· `148` `fsvc_.PublishFaceResult()`<br>· `149` `fsvc_.SetReady(true)` |
| 风险 | 结论"插 141 行之后"**恰好正确**，但理由错 → 实施者若把 `setRegisterHost` 放到 `Init()` 末尾（如 150 行后），**注册已完成，设置无效**（§3.1 自己写的约束） |
| **修正** | 明确写成：**"必须插在 141（`rt_.init`）与 147（`registerService`）之间"**；理由改为"服务注册与 topic 发布均在 Init 内完成（147/148 行）" |

### M3【验证手段失效】§5.4 的日志关键字不存在

| 项 | 内容 |
|----|------|
| 原文 | `grep -E "register_host\|advertise\|data_connect_fallback" /tmp/face.log /tmp/media.log` |
| 实际 | 全仓搜索：**没有** `register_host` / `advertise` 的日志输出（仅 `runtime_topic.cpp:124` 有一条 `publishTopic requires a registered local service...` 的 WARN）。只有 `data_connect_fallback` 可 grep |
| 后果 | 按文档验证时前两个关键字永远 0 命中，会误判"改动没生效" |
| **修正**（二选一） | ① 改文档：只保留 `data_connect_fallback`，并改用 §5.1 的 `omni-cli` 验证；<br>② **建议**：给 `resolveRegisterHost` 结果加一行日志（`runtime_service.cpp:79` 处），例如<br>`OMNI_LOG_INFO(LOG_TAG, "service %s advertise %s:%u", name, advertise_host.c_str(), port);`<br>—— 这条日志对**以后所有跨机排查**都有价值，建议提为框架侧小改动 |

### M4【验证手段受环境影响】§5.1 的 `omni-cli` 在 PC 上默认连本机

| 项 | 内容 |
|----|------|
| 原文 | `omni-cli info FaceService`（放在"板端"小节，但 §5.3 又暗示 PC 侧也这么做） |
| 实际 | `omni-cli` 的 SM 地址默认 **`127.0.0.1`**（`tools/omni_cli/main.cpp:52` 帮助、`:926` 默认值），PC 上执行必须显式指定 |
| **修正** | 板端：`omni-cli info FaceService`；**PC 端：`omni-cli -h <板IP> info FaceService`**（并说明 `-h` 是 SM 地址，不是数据面端口） |

---

## 三、表述不准（2 处）

| # | 位置 | 原文 | 实际 | 影响 |
|---|------|------|------|------|
| N1 | §7 失败对照 | 「`seq` 不连续…**由发布侧节流导致**」 | 是**队列丢弃**：`JobQueue::drop_oldest()`（q_drop）与 `FaceResultRing` 丢弃（ring_drop）。face_proc 不做"节流"（有 `--enroll-gap` 节流，但仅注册模式） | 误导排查方向（会去查发布频率，而不是查消费能力/池满） |
| N2 | §8 衔接表 | `subscribeTopic("FaceResult")` | PC 端应使用生成代码 **`FaceServiceProxy::SubscribeFaceResult(cb)`**（内部即 subscribeTopic，但带类型与 IDL hash 校验） | 实施者可能绕开生成代码手写 topic 名，丢失类型安全与 hash 校验 |

---

## 四、遗漏补充（5 条）

| # | 缺失内容 | 为什么必须补 |
|---|---------|-------------|
| **G1** | **实施步骤缺"改完 build → deploy → 重启 face_proc"** | §3.1 强调了"注册后设置无效"，但 §3 的"改哪里"只给代码片段。新人照做后不重启，`omni-cli` 仍显示 `127.0.0.1`，会以为代码写错 |
| **G2** | **Runtime 级设置影响"本进程注册的所有服务/topic"** | 当前 face_proc 只有 `FaceService` + `FaceResult` 两项 ✓ 无副作用；但需写明，否则将来若加"仅板内"服务会被连带暴露到网络 |
| **G3** | **同机 SHM 失败会 fallback TCP（走 advertise 地址）** | §6 写"板内完全不受影响"**过于绝对**。实测出现过 `shm_open failed → data_connect_fallback TCP`（`/dev/shm` 满/残留）。该场景下同机订阅也走 advertise，设错 IP 会**影响板内 OSD**（罕见但真实） |
| **G4** | **§4 的"配置文件方案"只有注释、无实现** | 应明确"MVP 只做环境变量 `FACE_REGISTER_HOST`，`cfg.advertise_ip` 列为后续"；否则实施者以为两套都要做 |
| **G5** | **PC 端回调约束未交叉引用** | 回调内禁止同步 RPC/写盘/重活（框架硬约束⑤，`Re-entrant waitForReply` 死锁）。可引用对接文档 `PC客户端订阅FaceResult-对接文档-20261103.md` §9 |

---

## 五、核对通过的部分（附证据，可放心采纳）

| 文档论断 | 核对证据 | 结论 |
|---------|---------|------|
| §2 三级调用链 + "通配归一成回环"根因 | `runtime_helpers.cpp:13-18`：`if (host.empty() \|\| host == "0.0.0.0") return "127.0.0.1";` | ✅ 准确 |
| §2 **优先级表**（Service 级 > Runtime 级 > 监听推导） | `runtime_service.cpp:resolveRegisterHost`：<br>`if (service && !service->getRegisterHost().empty()) return ...;`<br>`if (!register_host_.empty()) return register_host_;`<br>`return normalizeAdvertiseHost(listener_host);` | ✅ **逐行一致** |
| §2 "Runtime 级设一次就够" | 同上（优先级 2 对所有服务生效） | ✅ 准确 |
| §3.1 注册后再设无效（不会重新广告） | `advertise_host` 仅在 `startService` 路径计算（`runtime_service.cpp:79`） | ✅ 准确 |
| §4 空串/不设 → 退化 127.0.0.1 | `setRegisterHost` 直接赋值；`resolveRegisterHost` 用 `!empty()` 判断后回落 | ✅ 准确 |
| §4 "不要自动探测 IP" | 项目实测（多网卡/虚拟网桥/4G 场景） | ✅ 务实且正确 |
| §5.2 **advertise 端口 ≠ 9900** | 9900 是 SM 端口；服务数据面端口由 `initializeServiceListener` 动态分配并写入 advertise | ✅ **关键细节，写对了** |
| §5.1 `omni-cli info` 输出 Host 字段 | `tools/omni_cli/main.cpp:204` `printf("  Host: %s\n", info.host.c_str());` | ✅ 手段有效（仅需补 `-h`） |
| §6 设错 IP 不致板内中断 | 板内同机优先 SHM，与 advertise 无关（除 G3 的 fallback 场景） | ✅ 基本成立 |
| §10 C API | `omnibinder_c.cpp:454` `omni_runtime_set_register_host`；`:424` `omni_service_set_register_host` | ✅ 行号准确 |
| 附录 7 处文件位置 | runtime.h:367 / service.h:53 / runtime_config.cpp:26 / runtime_service.cpp:79,84 / runtime_helpers.cpp:12-13 / face_proc_app.cpp:141 | ✅ **全部命中** |

---

## 六、最小修改集（建议直接替换这 4 段）

```diff
【§3 改哪里】
-  face_proc_app.cpp 里 Run() 在 572 行，服务发布发生在 Run 之后，所以插在 141 行之后是安全的。
+  服务注册与 topic 发布均发生在 Init() 内：
+    141  rt_.init("127.0.0.1", 9900)
+    147  rt_.registerService(&fsvc_)
+    148  fsvc_.PublishFaceResult()
+  ⇒ setRegisterHost 必须插在 141 与 147 之间（插入到 147 之后将不再生效，见 §3.1）。

【§3 代码块】
+  rt_.setRegisterHost(GetBoardAdvertiseIp());   /* 必须在 147 行 registerService 之前 */
   rt_.registerService(&fsvc_);                  /* 147 */

【§5.1 验证】
-  omni-cli info FaceService
+  板端： omni-cli info FaceService
+  PC 端：omni-cli -h <板IP> info FaceService      # omni-cli 默认连 127.0.0.1

【§5.4 日志】
-  grep -E "register_host|advertise|data_connect_fallback" /tmp/face.log /tmp/media.log
+  grep -E "data_connect_fallback|Service died" /tmp/face.log /tmp/media.log
+  # 注：框架当前无 advertise 日志输出；如需该信息，先在 runtime_service.cpp:79 加一行 INFO

【§9 代码块】
-  face_service_.setRegisterHost(GetBoardAdvertiseIp());
+  fsvc_.setRegisterHost(GetBoardAdvertiseIp());
```

---

## 七、评估结论与实施建议

### 7.1 可行性评估

| 项 | 评估 |
|----|------|
| 技术路线 | ✅ 成立（框架原生支持，无需改框架） |
| 改动量 | **face_proc 加 3~8 行**（env 读取 + setRegisterHost + 日志） |
| 回归风险 | **低**——只影响 advertise 字符串；板内 SHM 路径不变（G3 场景除外） |
| 可验证性 | 中（`omni-cli info` 的 Host 字段是唯一直接证据 → 建议补 advertise 日志） |
| 工作量 | 改动 0.5h + 编译部署 0.5h + PC 联调 1h ≈ **半天** |

### 7.2 建议的实施顺序

```
1) 修正本审核报告的 M1/M2（否则实施必踩坑）
2) face_proc 加 GetBoardAdvertiseIp()（env 优先）+ setRegisterHost（141~147 之间）
3) 【可选但强烈建议】框架侧加 advertise INFO 日志（runtime_service.cpp:79）
4) build → deploy → 重启 face_proc
5) 板端 omni-cli info FaceService 确认 Host = 板 IP（不是 127.0.0.1）
6) PC 端 M1 探针：init → connect → SubscribeFaceResult → 3s 内收到 seq
7) 回归：板内 OSD 仍正常（media_proc 订阅不受影响）
```

### 7.3 需要决策的两个点

| # | 决策项 | 建议 |
|---|--------|------|
| 1 | **advertise 日志要不要加**（框架侧） | **加**——成本 3 行，收益是"以后所有跨机问题一眼可查"；本项目已因"无日志"排查过 OnServiceDied |
| 2 | **配置文件方案做不做** | MVP **不做**，只用 `FACE_REGISTER_HOST` 环境变量（部署脚本注入）；等有多实例需求再引入 |

---

## 八、审核清单（供提交前自检）

- [ ] M1 `fsvc_` 已改
- [ ] M2 插入点说明已改为"141~147 之间"（含理由）
- [ ] M4 `omni-cli -h <板IP>` 已改
- [ ] M3 验证段已改（或已加 advertise 日志）
- [ ] N1 "节流"→"队列丢弃"
- [ ] N2 `SubscribeFaceResult` 已改
- [ ] G1 部署/重启步骤已补
- [ ] G2 "影响本进程所有注册项"已注明
- [ ] G3 SHM fallback 场景已补
- [ ] G4 env 为唯一实现已明确
- [ ] G5 回调约束交叉引用已补
