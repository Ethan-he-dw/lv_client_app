# 改造路线：从 Demo 到稳定客户端 + 接入 FaceResult

> 日期：2026-11-03 ｜ 依据：本项目源码现状 + `PC客户端订阅FaceResult-对接文档-20261103.md`
> 目标读者：项目负责人（你自己），用于确认方向与排期

---

## 0. 一页结论

| 问题 | 答案 |
|---|---|
| 现在是什么 | **一个能演示的 UI 骨架**。四页导航、页面注册表、FFmpeg 播放、UDP 设备发现齐了，但：通信数据是模拟的、资源生命周期没人管、错误只是改个 label、配置全硬编码 |
| 目标是什么 | **一个能长期挂着跑的产品客户端**：断线自愈、不崩不漏、能消费板端真实 FaceResult 并叠加到视频上 |
| 两件事的先后 | **必须先稳定底座，再接数据**。理由：FaceResult 是持续高频数据流（板端 15~25fps），会放大现有的每一个资源/线程缺陷——底座不稳，接完只会从"偶发卡顿"变成"必崩" |
| 关键决策 | 走 **bridge（TCP + JSON Lines）**，不走原生 OmniBinder 客户端（你的 PC 是 Windows，方案 A 无库可用） |
| 最大隐藏坑 | **FaceResult 没有通道号字段**，多路播放时无法区分是哪一路的结果；bridge 必须自己补 `channel` 与 `enc_w/enc_h`，否则 OSD 一定画错 |

---

## 1. 目标状态（验收长什么样）

### 稳定性指标

| 指标 | 目标 | 测法 |
|---|---|---|
| 连续运行 | 8 小时不崩、不卡死 | 挂机 + 每 30min 截图 |
| 内存 | RSS 波动 < 5%，无单调增长 | 任务管理器/`tasklist` 每 5min 采样 |
| 句柄 | 线程数、socket 数恒定 | 进程资源监视器 |
| 断线恢复 | 拔网线 30s、插回 → 10s 内自动恢复 | 手动物理断网 |
| 设备掉线 | 板端重启 → 客户端自动重连并继续出数据 | 重启 face_proc / bridge |
| 错误可见 | 任何失败都必须有可读提示 + 日志，不静默 | 人为构造：地址错、端口不通、文件不存在 |

### 功能指标（FaceResult）

| 指标 | 目标 |
|---|---|
| 数据链路 | Face 页显示真实 `seq` 递增，心跳（`det_count=0`）可辨识 |
| 时延 | 板端发布 → PC 显示 < 100ms（同网段 TCP） |
| OSD 精度 | 框与视频画面偏差 < 2px（1920×1080 与 640×360 两种源都要过） |
| 多路 | 4 宫格下每格只显示自己通道的框 |

---

## 2. Gap 分析：现在缺什么

### A. 资源生命周期（当前最致命）

| 现状 | 位置 | 后果 |
|---|---|---|
| `player_widgets_t` / `player_data_t` 用 `lv_malloc` 分配，对象删除时**从不释放**；`player_data->url` 还 `lv_strdup` 了一份 | `ui_page_play_multplayer.c:547/566/573` | 每次切换宫格（CH4→CH9→CH4）泄漏 N×(2 结构体+1 字符串)，长跑必爆 |
| `rebuild_players()` 只 `lv_obj_delete` 对象，不回收自定义数据 | `:605-611` | 同上 |
| 播放失败路径 `return` 前不清理已创建的 player | `:397-402` | 半初始化对象残留 |
| 无统一销毁入口（`app_pages_destroy()` 存在但从未被调用） | `ui.c:239` | 退出/重载时资源不释放 |

**这一项不修，任何"稳定"都是空话。**

### B. 错误处理与自恢复

| 现状 | 需要 |
|---|---|
| 播放失败只 `lv_label_set_text("open failed")` | 分级错误（URL 非法/网络不可达/解码失败）+ 自动重试策略 |
| UDP 搜索发出后不管结果 | 超时判定（如 3s 无应答 → "未发现设备"） |
| 无重连机制（UDP 无连接无所谓，但 TCP 接入后必须有） | 指数退避 + 状态机（Disconnected/Connecting/Connected/Degraded） |
| 升级失败/中途断开无反馈 | 失败态 + 可重试按钮 |

### C. 配置与可部署性

| 现状 | 需要 |
|---|---|
| RTSP 地址硬编码 `rtsp://192.168.10.248:555/stream1` | 配置文件（`config/app.json` 或 ini），首次运行生成默认 |
| `video_Null.png` 绝对路径 `M:/LVGL_learn/...`（2 处） | 相对可执行文件的路径解析 |
| 窗口 1600×900、播放区 720、日志区 746 全是字面量 | 集中到 `ui_app.h` 常量 + 按窗口尺寸计算 |
| 版本号 `V0.0.0` 硬编码 | 编译期宏（CMake `-DAPP_VERSION`） |

### D. 可观测性

| 现状 | 需要 |
|---|---|
| 全是 `printf`（`ui.c:61/130/141/160/233` 等） | 分级日志：`LOG_DEBUG/INFO/WARN/ERROR` + 时间戳 + 模块名，输出到控制台 + 滚动文件（如 5MB×3） |
| 无运行时指标 | 状态栏显示：链路状态、收到帧数、丢帧、CPU/内存（可选） |
| 崩溃无现场 | SEH/信号处理，崩溃时 dump 栈到文件 |

### E. 通信抽象（接入 FaceResult 的前置）

| 现状 | 需要 |
|---|---|
| `comm.c` 只有「设备表 + 事件队列 + 模拟线程」 | 保持队列/事件模型**不变**，新增 TCP 数据源，把 FaceResult 也变成一种 `comm_event_t` |
| 只有 UDP（`comm_udp.c`，Windows-only） | 新增 `comm_tcp.c`（同样 Windows/Posix 双实现） |
| 无 JSON/文本协议解析 | 新增 `face_proto.c` |
| 事件类型 5 种，无"连接状态"概念 | 增 `COMM_EVT_LINK_STATE`、`COMM_EVT_FACE_RESULT` |

> 设计原则：**UI 层永远只认 `comm_event_t`**。UDP 设备和 TCP 人脸结果都从同一个队列出来，页面通过 `evt->type` 过滤。这样接入新数据源不需要改任何页面代码。

---

## 3. 主线一：稳定性改造（分三步走）

### P0 · 止血（预计 1~2 天，必须最先做）

| # | 事项 | 文件 |
|---|---|---|
| 1 | 修宫格切换内存泄漏：给 `player_widgets_t`/`player_data_t` 挂 `LV_EVENT_DELETE` 回调统一 `lv_free` | `ui_page_play_multplayer.c` |
| 2 | 修手势翻页 bug：`/` → `%` | `ui.c:150,154` |
| 3 | 删死代码 `ui_page_multplayer_create()`（~110 行）与 `ui_multpage_play_*` 死声明 | `ui_page_play_multplayer.c:802`、`.h` |
| 4 | `video_Null.png` 改相对路径；`creat_single_palyer` 改 `static` + 改名 `create_single_player` | 同上 |
| 5 | 关掉 `printf` 噪音，换统一日志宏（先只做宏封装，分级后置） | 全部 |

**验收**：反复切 CH4/CH9/CH4 各 50 次，进程 RSS 不涨。

### P1 · 加固（预计 3~5 天）

| # | 事项 | 说明 |
|---|---|---|
| 6 | 配置模块 `src/core/config.c` | 读写 `config.json`：RTSP 地址、bridge IP/端口、窗口尺寸、日志级别 |
| 7 | 日志模块 `src/core/log.c` | 分级 + 文件轮转 + 崩溃处理 |
| 8 | 链路状态机 | 抽成 `link_state_t`，UI 顶栏用颜色点表示（灰/黄/绿/红） |
| 9 | 播放资源上限 | 9 宫格同时软解会打满 CPU → 限并发（如最多 4 路），超出排队或提示 |
| 10 | 布局参数化 | 分辨率/侧边栏宽/播放区高度集中到 `ui_app.h`，改窗口尺寸只改一处 |
| 11 | 退出清理 | 真正调用 `app_pages_destroy()` + `comm_deinit()` |

### P2 · 工程化（持续）

- CMake 拆分：`app_core`（静态库）+ `main`，便于加测试
- 单元测试：协议解析（JSON→结构体）、坐标换算（给定输入断言输出）——这两块最容易错，且纯函数好测
- `clang-format` + 编译告警清零（现在 `-Wall -Wextra` 下有若干 unused/format 告警）

---

## 4. 主线二：接入 FaceResult

### 4.1 为什么只能走 bridge

| 维度 | 方案 A（原生客户端） | 方案 B（bridge） | 你的情况 |
|---|---|---|---|
| PC 依赖 | `libomnibinder.a`（**Linux x86 静态库**）+ `omni-idlc` 生成代码 | 无，普通 socket | ❌ A 在 Windows/MinGW 下无库可用，框架 Windows 支持"未实测" |
| 板端改动 | `face_proc` 加 `setRegisterHost(板IP)` 1 行 | 新增 bridge 进程 | B 的改动局限在新文件，风险更低 |
| 协议耦合 | 强绑 IDL hash（改 IDL 两端要同步重建） | JSON，可裁剪 | B 更适合迭代 |
| 双向控制 | 原生支持 RPC | 需 bridge 代理 | 现在没有控制需求，先不管 |

**结论：走 B。** 若将来确实要从 PC 下发控制（调门限、切库），再让 bridge 代理 RPC 或补 A，不影响现有结构。

### 4.2 板端（`3516-media-refactor`）：新增 `face_bridge`

```
face_proc ──FaceResult──> face_bridge ──JSON Lines over TCP:9100──> PC
```

实现要点（务必覆盖文档 §5.2 的"生产化注意"）：

1. **回调只入队**：`SubscribeFaceResult` 回调里只 `push` 到无锁/加锁队列，**不做 send、不做序列化重活**（框架约束⑤，且避免 PC 慢导致回调阻塞）
2. **独立发送线程**：从队列取 → 序列化 → `send(MSG_NOSIGNAL)`，慢客户端不拖累订阅
3. **多客户端**：`poll()` + 客户端列表 + 每客户端发送队列；新连上先补 `hello`
4. **协议**：JSON Lines（一行一帧）。建议首行 hello：
   ```json
   {"type":"hello","ver":1,"idl_hash":"0x368f4259","channel":0,"enc_w":1920,"enc_h":1080}
   {"type":"face","seq":12345,"det_count":2,"best_index":0,"matched":true,"name":"张三","cos":0.732,"dets":[{"s":0.98,"x1":210.5,"y1":320.1,"x2":360.2,"y2":500.8,"kps":[...]}],"fps":24.8,"det_ms":18.2,"mbf_ms":6.1,"dec_ms":4.3,"ts_us":123456789}
   ```
   **hello 里的 `channel` 和 `enc_w/enc_h` 是文档里没有、但你必须有字段**（原因见 §4.4、§4.5）
5. 打包进 `CMakeLists.txt` + `build_hi3516.sh`（参照 `face_mbf_probe`）
6. bridge 是**订阅方**，连 `127.0.0.1:9900` 即可，**不需要 `setRegisterHost`**

### 4.3 PC 端：新增四个模块（不动现有页面）

| 模块 | 职责 | 关键设计 |
|---|---|---|
| `src/comm/comm_tcp.c` | TCP 客户端 | 独立 rx 线程 + 行缓冲；断线指数退避重连（1s→2s→4s→…→30s）；心跳超时（3s 无 hello/face → Degraded） |
| `src/core/json.c` | 极简 JSON 解析 | **建议直接 vendor cJSON**（2 个文件、MIT、成熟）；也可只写定长 `sscanf` 解析（协议自己定，够用且零依赖） |
| `src/comm/face_proto.c` | 行 → `face_result_t` | 纯函数，便于单测；字段缺失要容错（旧版 bridge 兼容） |
| `comm.c` 改造 | 接入新事件 | 增 `COMM_EVT_FACE_RESULT`、`COMM_EVT_LINK_STATE`；TCP 线程把事件 `queue_push` 进**同一个队列** |

UI 侧（两个都要，但**先做表格页**）：

1. **数据验证优先**：新增 `APP_PAGE_FACE` 页，用 `lv_table` 显示近 N 帧：`seq / det_count / best_name / cos / fps / det_ms / mbf_ms`，顶部链路状态灯 + 收包速率。这一步能在没有 OSD 的情况下确认整条链路正确
2. **再做 OSD 叠加**：`src/ui/ui_face_osd.c`，在播放器之上用 **`lv_obj` 矩形 + label** 画框（**不要用 `lv_canvas` 逐帧重绘**——每帧清屏+重画会吃掉大量 CPU，而框的数量只有几个）

### 4.4 坐标换算：必须分两步，少一步框就歪

文档 §3.2 给的是**第一步**，你的场景还需要**第二步**：

```
第一步：检测画布(640×640, pad_top=140) → 编码画面(enc_w×enc_h)
        px1 = x_canvas * (enc_w / 640)
        py1 = (y_canvas - 140) * (enc_h / 360)

第二步：编码画面 → 播放器控件显示区（LV_IMAGE_ALIGN_CONTAIN 会留黑边）
        scale  = min(w_widget / enc_w, h_widget / enc_h)
        off_x  = (w_widget - enc_w * scale) / 2
        off_y  = (h_widget - enc_h * scale) / 2
        px2 = px1 * scale + off_x
        py2 = py1 * scale + off_y
```

合并后 `px2 = x_canvas * (enc_w/640) * scale + off_x`。**`enc_w/enc_h` 必须来自 bridge hello（或 FFmpeg 实测），不能假设 1920×1080。**

### 4.5 两个语义陷阱（文档 §3.3、§9 强调，容易误判）

1. **`det_count == 0` 的 FaceResult 照常发布** → 它是心跳，代表"链路活着"，不要当异常、更不要当"检测失败"红色告警
2. **`ts_us` 是板端单调时钟**（上电起算）→ 只能算相对时间（相邻帧差），**不能**直接显示成日期时间；要显示"延迟"需 bridge 额外带一个板端 epoch 或由 PC 用收包时间估算
3. 反向判定：**长时间（>2s）收不到任何 FaceResult** 才是异常 → 触发 Degraded 状态并尝试重连

### 4.6 多路场景的真实缺口（需要你先拍板）

FaceResult 只有 `seq / buffer_id`，**没有"这是哪一路摄像头"的字段**。而你的 multPlay 页是 4/6/8/9 路并存：

- 如果板端 face_proc **只跑一路**（比如主码流）→ 那么 9 宫格只有一格能叠框，其余格不该显示脸
- 如果板端**每路各跑一个 face_proc** → bridge 要起多个实例（不同端口）或订阅多个 topic，且必须在报文里带 `channel`

**建议**：bridge 的 hello 带 `channel`（默认 0），每帧 `face` 消息也带 `channel`（可省，默认同 hello）；PC 端按 channel 路由到对应宫格。这样单路/多路都能兼容，不用改协议。

---

## 5. 里程碑与验收

| 里程碑 | 内容 | 验收方式 |
|---|---|---|
| **M0 基线**（1~2 天） | P0 止血 | 切宫格 50 次 RSS 不涨；构建告警清零 |
| **M1 数据通**（3~5 天） | bridge 跑起来 + `comm_tcp` + Face 页表格 | Face 页能看到真实 seq 持续递增；`det_count=0` 心跳可辨识 |
| **M2 加固**（1 周） | P1 配置/日志/重连/状态机 | 拔插网线自动恢复；配置改地址免重编译 |
| **M3 OSD**（3~5 天） | 坐标换算 + 框/姓名叠加 + 全屏适配 | 1080p 与 360p 源下框误差 < 2px；切全屏后框跟随 |
| **M4 产品化**（1 周） | 打包、版本号、崩溃日志、并发上限、关于页 | 8 小时挂机测试通过；异常场景全部有提示 |

**建议顺序不可颠倒**：M1 只做"看到数字"，把 OSD 留到 M3。因为 OSD 的坐标错误会伪装成"数据错"，两者混在一起排查会非常痛苦。

---

## 6. 需要你先拍板的 5 件事

| # | 决策 | 选项 / 影响 |
|---|---|---|
| 1 | bridge 谁写、在哪编 | 你写（我在 PC 侧配合给骨架代码）/ 已有同事在写。板端工具链是交叉编译，必须能编进 `build_hi3516.sh` |
| 2 | 板端是否存在多路 face_proc | 决定 §4.6 要不要多实例 + channel 路由 |
| 3 | PC 端 JSON 方案 | vendor cJSON（省事、可扩展）/ 自定义 `sscanf` 文本（零依赖、更快，但扩展字段要改代码） |
| 4 | 板端 IP 与端口 | 文档示例 `192.168.10.168:9100`，确认与现有服务不冲突、防火墙放通 |
| 5 | 是否需要 PC→板 控制 | 决定 bridge 是否现在就把 RPC 代理做进去（建议先不做） |

---

## 7. 明确"现在不要做"的事（避免返工）

1. **不要为了方案 A 迁到 Linux**：你整套 UI 是 Windows+MSYS2 编的，迁移成本远大于 bridge
2. **不要在网络线程/订阅回调里碰任何 `lv_*`**：LVGL 不是线程安全的，必须走事件队列 → 主循环
3. **不要先做 OSD**：先让数字对，再让框对
4. **不要用 `lv_canvas` 逐帧画框**：几张矩形用 `lv_obj` 叠加即可，CPU 差一个数量级
5. **不要把 bridge 的 JSON 解析放进 UI 线程**：解析在 rx 线程做完，UI 只消费结构体
6. **不要继续往 `ui_page_play_multplayer.c`（980 行）里加逻辑**：接入前先把它拆成 `multplay_grid.c`（布局/重建）+ `multplay_cell.c`（单格播放），否则这个文件会变成 1500 行的不可维护体

---

## 8. 参考索引

| 内容 | 位置 |
|---|---|
| 对接文档（本路线依据） | `F:\git-clone\media-refactor-arch\docs\PC客户端订阅FaceResult-对接文档-20261103.md` |
| IDL 契约 | `F:\git-clone\OmniBinder\3516-media-refactor\idl\face_service.bidl` |
| 订阅参考实现（板端） | `...\src\media_core\osd_face.cpp`（`OnFaceResult`） |
| 发布参考实现（板端） | `...\src\face_core\face_proc_app.cpp`（`PublishReady`/`PublishNoFace`） |
| 框架 API | `F:\git-clone\OmniBinder\docs\api-reference.md`、`threading-model.md` |
| 本项目架构梳理 | `docs/project_map.md` |
