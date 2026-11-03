# 改造路线 v3：砍掉自定义嗅探，回到框架原生

> 日期：2026-11-03 ｜ 版本：v3
> 依据：本项目源码 + `PC客户端订阅FaceResult-对接文档-20261103.md` + `OmniBinder/docs/api-reference.md` + 2026-11-03 需求澄清
>
> **v3 相对 v2 的核心变化：删掉整个自定义嗅探协议（probe / streams 列表 / 码流匹配）。**
> 理由：框架已有 `lookupService` / `queryPublishedTopics` / `subscribeTopic` 三个权威接口，再做一层 TCP 协议纯属重复建设。
> 副作用：主方案从 **B（bridge）翻回 A（原生客户端）** —— 因为"起没起"的判定本身就是框架 API，只有原生客户端能调（§3）。

---

## 0. 结论

| 项 | v3 的决定 |
|---|---|
| 自定义嗅探协议 | **删除**。端口 9100、probe/streams/match 三段握手、URL 归一化与四级匹配，全部不要 |
| 能力判定 | `lookupService("FaceService")` → 服务在不在；`queryPublishedTopics()` → 有没有人发 `FaceResult` |
| 订阅后没数据 | **正常态**，不显示内容即可，不报错；连接保持，生产者起来后自动开始显示 |
| 连接/订阅失败 | 重试 5 次（1/2/4/8/16s），仍失败则**停止尝试**，UI 提示 + 留给用户手动重试 |
| 主方案 | **A（原生客户端）**：PC 直连板端 `service_manager:9900` |
| 保留的工程工作 | P0 止血（内存泄漏等）、最新值单槽、坐标归一化、OSD 叠加、配置/日志/重连状态机 |

---

## 1. 为什么不需要嗅探

### 1.1 三层论证

| 你想知道的 | 框架已有答案 | 自定义嗅探能额外给的 |
|---|---|---|
| 这台设备跑没跑 face | `lookupService("FaceService")`，返回 0 = 在 | 无（只是换个端口再问一次） |
| 现在有没有人在发结果 | `queryPublishedTopics()` 里有没有 `FaceResult`；订阅后有没有数据 | 无（订阅本身就是最权威的探针） |
| 这条流是不是它算的 | **没有** | 一个"码流列表"，但可靠性存疑（见 §1.2） |

前两行已经说明：**嗅探的三个用途里，两个是重复的**。第三行才是嗅探的独有价值，而它的可靠性恰恰最低。

### 1.2 那"码流匹配"真的不需要吗

砍掉之后，"这条流是不是它算的那条"从**可判定**变成**隐含假设**。必须把假设写下来，否则以后出问题查不到根因：

> **假设 H1**：一台设备上只有一路 face_proc，且 PC 播放的是**这台设备自己**输出的流。

- 满足 H1：一切正常，框一定对得上
- 不满足 H1（比如 Play 页填的是旁边一台 IPC 的地址，face 跑在板端）：框会画在错误的位置，**而且不会报任何错**

这是砍掉嗅探的**唯一代价**。它可接受的原因：你们的部署是"板端既有 RTSP 又有 face"，天然是一对一。

**零成本兜底**（三条，都不用新协议）：

1. 订阅成功后调一次 `GetFrameCount()`（IDL 里已有），隔 2s 再调一次 —— **计数在涨**说明推理在跑，不是空壳服务
2. 画面里明明有人却没有框 → 说明流不匹配，而不是"没检测到"
3. 框的位置系统性偏移（比如整体偏上/偏左一个固定量）→ 优先怀疑坐标契约（`pad_top`），其次怀疑流不匹配

### 1.3 一处推理要修正（不影响结论）

你说"RTSP 流能显示，就说明框架的线程都起了"——这个**不完全成立**：`face_proc` 是可以单独死掉的（对接文档 §3.3 明确：media_proc 以 2s 无数据判定降级，就是在防这种情况）。RTSP 由 media/stream_proc 提供，和 face_proc 是独立进程。

但**你的结论依然正确**，而且理由更强：face_proc 死了，客户端重试也没用——要重启的是设备，不是连接。所以"重试一段就停"是对的，只是判定条件应该是"lookupService 失败 / 订阅失败"，而不是"RTSP 能播"。

---

## 2. 新流程：四态 + 一个不重试的分支

```
点 Play
  │
  ├─ rt.init(板IP, 9900) → 失败 → 重试 5 次 → [Failed]（停）
  │
  ├─ lookupService("FaceService") → 失败 → 同上
  │
  ├─ subscribeTopic("FaceResult") → 失败 → 同上
  │
  └─ 成功 → [Subscribed] ──有数据──> [Streaming]（显示框）
                 │
                 └──无数据──> [NoData]（不显示，**不重试**，连接保持）

Stop / 换 URL / 离开 Play 页 → 断开，回 [Idle]
```

| 状态 | 判定条件 | UI | 动作 |
|---|---|---|---|
| `Idle` | 未播放 | 无任何人脸 UI | — |
| `Connecting` | 正在 init / lookup / subscribe | 状态栏灰点「连接识别服务…」 | 失败按 1/2/4/8/16s 重试，最多 5 次 |
| `Failed` | 5 次都失败 | 黄点「识别服务不可用」+「重试」按钮 | **停止自动重试**，仅在重播/切 URL/点重试时重启 |
| `Subscribed` | 订阅成功，尚无数据 | 灰点「已连接，等待数据」 | 保持长连接，不做额外动作 |
| `NoData` | 订阅成功且超时无数据（含心跳） | 灰点「暂无人脸数据」 | **不重试**：生产者起来后数据自然到达 |
| `Streaming` | 收到 FaceResult（含 `det_count=0` 心跳） | 绿点 + fps / 延迟 | 开始画框；2s 无数据 → 回 `NoData` 并清框 |

关键点：

1. **`NoData` 不重试**——订阅已建立，face_proc 启动后发布者自动出现，不需要重新握手。这条比"循环重连"省得多。
2. **`det_count == 0` 也是数据**——它是心跳（对接文档 §3.3），收到就说明链路活着，状态算 `Streaming`，只是不画框。
3. **只有 `Connecting` 才重试**，且次数封顶，避免"设备根本没有这个服务"时无限打洞。
4. 退避：1 → 2 → 4 → 8 → 16 秒（共 31 秒），之后停。手动重试立即重置计数。

---

## 3. 岔路：lookupService 放在哪一侧

砍掉嗅探后，这个岔路重新变成主决策。**注意 v2 推荐 B，v3 推荐 A**，翻转理由是明确的：

| 维度 | A：PC 直连框架（原生客户端） | B：板端 bridge |
|---|---|---|
| 能力判定 | ✅ `lookupService` 原生，权威 | 需在 bridge 里再实现一遍并转述给 PC |
| 新增协议 | ✅ 零（就是框架协议） | 要定 status / face 行格式 |
| 新增进程 | ✅ 无 | 板端多一个进程 + 打包 |
| 板端改动 | ❌ `face_proc` 加 `setRegisterHost(板IP)` 1 行 + 重部署 | ✅ 不动现有进程 |
| PC 依赖 | ❌ `libomnibinder.a`（已在 Windows 编通，988K）+ C++ 包装层 | ✅ 纯 C socket |
| IDL 耦合 | ❌ hash 必须一致；板端改 IDL 要重编 PC | ✅ JSON 松耦合 |
| RPC（`GetStatus`/`GetFrameCount`） | ✅ 直接可用（§1.2 的兜底靠它） | 需代理 |

**结论：走 A。** 三条理由：

1. v2 里 B 的最大优势是"协议可裁剪、可加自定义字段"——嗅探砍掉后，**这个优势不存在了**
2. A 的库你已经在 Windows 上编通并验证过（`build_win/target/lib/libomnibinder.a`），前提障碍消失
3. §1.2 的兜底手段要用 `GetFrameCount()`，A 直接有，B 还得代理

**A 的唯一硬阻塞**：板端能否加那 1 行 `setRegisterHost`。如果不能（固件冻结），退回 B，其余设计不变。

### 3.1 工程形态：C 项目里的 C++ 边界

你的 `src/` 全是 `.c`，而 `libomnibinder.a` 是 C++（STL 接口）。做法：

```
src/face/face_client.cpp     ← 唯一 C++ 文件，包装成 extern "C"
src/face/face_client.h       ← 纯 C 头，给 ui/comm 用
```

`CMakeLists.txt` 本来就是 `project(lvgl C CXX)`，`.cpp` 能被识别，只需：

```cmake
file(GLOB FACE_SOURCES "${PROJECT_SOURCE_DIR}/src/face/*.cpp")
list(APPEND MAIN_SOURCES ${FACE_SOURCES})
target_include_directories(lvgl PUBLIC F:/git-clone/OmniBinder/include)
target_link_libraries(main ${MAIN_LIBS} F:/git-clone/OmniBinder/build_win/target/lib/libomnibinder.a)
# Windows 还要 ws2_32（本项目已链）；Linux 改 -ldl -lpthread
```

`face_client.h` 对外只暴露 5 个函数：

```c
bool face_start(const char *board_ip, int port);          /* 起线程：init + lookup + subscribe */
void face_stop(void);
bool face_get_latest(face_result_t *out, uint32_t *ver);  /* 单槽读取 */
int  face_get_state(void);                                /* Idle/Connecting/…/Streaming */
const char *face_state_str(int st);                       /* 状态栏文案 */
```

UI 层只 include 这个 `.h`，**完全看不到 C++**。

---

## 4. 线程模型（A 方案，必须严格遵守）

框架文档给出两条硬约束（`api-reference.md:148`、`:160`）：

- `lookupService()` / `subscribeTopic()` / `invoke()` 是**同步阻塞** API，**"不建议在 owner event-loop 回调中再发起"**
- `run()` / `pollOnce()` **同一实例不允许两个线程并发驱动**

所以：

```
[C++ 驱动线程]  rt.run()  或  for(;;) rt.pollOnce(100)      ← 只这一个线程驱动 event-loop
                  └─ TopicCallback（在此线程执行）
                       └─ 只做：写最新值单槽 + version++     ← 禁止阻塞 / IO / 同步 API

[C++ 控制线程]  init → lookupService → subscribeTopic
                  ↑ 必须与驱动线程分开：同步 API 要 event-loop 在跑才能返回
                  ↑ 且在回调里绝对不能调它们（自锁）

[UI 线程 LVGL]  50ms 定时器：face_get_state() + face_get_latest()
                  ← 禁止任何网络 / 同步 API 调用
```

顺序必须是：**先起驱动线程 `run()`，再由控制线程调 `lookupService`**。反过来（先同步 lookup）会因为没人泵 event-loop 而超时。

停止：`rt.stop()` 可从任意线程调用（文档明确），用它中断阻塞等待，再由 `face_stop()` join 线程。

---

## 5. 数据通路：最新值单槽

不需要和画面同步 ⇒ 不排队列。25fps 在 50ms 轮询里必然堆积，队列（64 槽）满了还会丢新帧。

```c
/* 回调线程：覆盖式写入 */
face_store_put(const face_result_t *r);          /* 加锁覆写 + version++ */
/* UI 线程：只在 version 变化时取一次 */
bool face_store_get_if_new(face_result_t *out, uint32_t *last_ver);
```

状态变化（`Connecting/Subscribed/NoData/Streaming/Failed`）走**事件队列**，因为它们不能丢。

---

## 6. 坐标换算：不需要编码分辨率

板端要 `enc_w/enc_h`，因为框画在**编码帧**上（`osd_face.cpp:63`，1920×1080 时 ×3）。PC 画在**控件**上，两步缩放会抵消：

```
px_enc  = x * (enc_w/640)      px_disp = px_enc * (W/enc_w)  ⇒  px_disp = x * W/640
```

所以：

```
u  = clamp(x_canvas / 640.0, 0, 1)
v  = clamp((y_canvas - 140.0) / 360.0, 0, 1)
px = X0 + u * W_disp ,  py = Y0 + v * H_disp
```

- `640 / 360 / 140` 是画布常量（`frame_types.h`、`osd_face.cpp:50`），编译期写死
- `X0/Y0/W_disp/H_disp` = 视频在控件内的显示矩形：`lv_image_get_src_width/height(player)` + `LV_IMAGE_ALIGN_CONTAIN` 算黑边 + 与控件区域求交，**纯本地**
- 前提：画布活动区 640×360 与视频同为 16:9（板端 `g_enc_w=1920, g_enc_h=1080` 按 16:9 写死）
- 与板端一致：换算后 `< 4px` 丢弃（`osd_face.cpp:75`）
- **过期清除**：`NoData` 或断开 → 立即隐藏全部框，否则留鬼影

---

## 7. 稳定性改造（与接入解耦，先做）

### P0 · 止血（1~2 天）

| # | 事项 | 文件 |
|---|---|---|
| 1 | 宫格切换内存泄漏：`player_widgets_t` / `player_data_t` / `lv_strdup(url)` 都不释放 | `ui_page_play_multplayer.c:547/566/573`，用 `LV_EVENT_DELETE` 统一 `lv_free` |
| 2 | 手势翻页 `/` → `%` | `ui.c:150,154` |
| 3 | 删死代码 `ui_page_multplayer_create()`（~110 行）+ 死声明 | `ui_page_play_multplayer.c:802`、`.h` |
| 4 | `video_Null.png` 绝对路径 → 相对路径；`creat_single_palyer` 加 `static` 并改名 | 同上 `:416,499,476` |
| 5 | `printf` → 统一日志宏 | 全局 |

### P1 · 加固（3~5 天）

配置模块（板 IP/端口、默认 RTSP、日志级别 → `config.json`）、日志分级 + 文件轮转、9 宫格软解并发上限、布局参数集中到 `ui_app.h`、退出时真正 `app_pages_destroy()` + `comm_deinit()`。

### P2 · 工程化

CMake 拆 `app_core` + `main`；单测覆盖 **坐标换算 / 解析 / 状态机迁移**；告警清零。

---

## 8. 里程碑

| 里程碑 | 内容 | 验收 |
|---|---|---|
| **M0**（1~2 天） | P0 止血 | 切宫格 50 次 RSS 不涨；构建无新增告警 |
| **M1**（2~3 天） | **C++ 控制台探针**：init → lookup → subscribe → 打印 seq/det/name/perf | 控制台能看到 seq 持续递增；拔网线能看到超时；`det_count=0` 心跳可见 |
| **M2**（3~5 天） | 接进 App：`face_client.cpp` + 单槽 + Play 页表格 | App 里能看到真实 seq；`NoData` / `Failed` 两态可辨 |
| **M3**（3~5 天） | OSD 叠加 | 1080p/360p 两种源下框贴合 < 2px；快速挥手滞后不超过 1 帧 |
| **M4**（1 周） | 产品化 | 8 小时挂机通过；各状态提示正确；断链能自愈 |

M1 的探针不是临时脚本，**它就是 A 方案的最小可运行客户端**，后面的 `face_client.cpp` 直接从它长出来。

---

## 9. 待你确认

| # | 事项 | 影响 |
|---|---|---|
| 1 | **板端能否加 `setRegisterHost(板IP)`** | A 的硬前提。不行就得退回 B（bridge），其余设计不变 |
| 2 | 板 IP / SM 端口 | 定 `config.json` 默认值 |
| 3 | 重试次数与间隔是否按 1/2/4/8/16s × 5 次 | 影响"多久放弃" |
| 4 | `Failed` 后是否只在用户操作时重试 | 建议是；否则退化成无限重连 |
| 5 | 是否需要 `GetFrameCount()` 兜底（§1.2） | 建议要，一次调用即可排除"空壳服务" |
| 6 | 多路设备是否真的不存在 | H1 假设；若存在多路，需要重新引入某种通道标识 |

---

## 10. 不要做的事

1. **不要在 UI 线程调任何同步 API**（`lookupService`/`subscribeTopic`/`invoke`）——会冻住画面
2. **不要在 `TopicCallback` 里发起同步 API**——文档明确"不建议"，会自锁
3. **不要两个线程同时驱动 `run()`/`pollOnce()`**——同一实例禁止并发驱动
4. **不要在 `NoData` 时重连**——订阅还在，生产者起来自然就有数据
5. **不要把 FaceResult 排队列**——不同步场景下正确结构是"最新值单槽"
6. **不要先做 OSD**——先让状态机和数字对，再让框对
7. **不要用 `lv_canvas` 逐帧画框**——几个矩形用 `lv_obj` 叠加，CPU 差一个数量级

---

## 附：参考索引

| 内容 | 位置 |
|---|---|
| 对接文档 | `F:\git-clone\media-refactor-arch\docs\PC客户端订阅FaceResult-对接文档-20261103.md` |
| 框架 API（`lookupService` / 阻塞语义） | `F:\git-clone\OmniBinder\docs\api-reference.md:148,276` |
| 线程模型 | `F:\git-clone\OmniBinder\docs\threading-model.md` |
| IDL 契约 | `...\3516-media-refactor\idl\face_service.bidl` |
| 板端坐标换算参考 | `...\src\media_core\osd_face.cpp:59-85` |
| 画布常量 | `...\src\frame_pool\frame_types.h` |
| Windows 版框架库 | `F:\git-clone\OmniBinder\build_win\target\lib\libomnibinder.a`（988K） |
| 本项目架构梳理 | `docs/project_map.md` |
