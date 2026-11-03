# 改造路线 v3.3：砍掉自定义嗅探，回到框架原生

> 日期：2026-11-03 ｜ 版本：v3.3（8 项决定落实 + 字体问题纠正）
> 依据：本项目源码 + `PC客户端订阅FaceResult-对接文档-20261103.md` + `OmniBinder/docs/api-reference.md` + 需求澄清
>
> **v3 相对 v2 的核心变化：删掉整个自定义嗅探协议（probe / streams 列表 / 码流匹配）。**
> 理由：框架已有 `lookupService` / `queryPublishedTopics` / `subscribeTopic` 三个权威接口，再做一层 TCP 协议纯属重复建设。
> 副作用：主方案从 **B（bridge）翻回 A（原生客户端）** —— 因为"起没起"的判定本身就是框架 API，只有原生客户端能调（§3）。
>
> **v3.1**：双出口（面板 + OSD）、订阅源抽象成 `face_source_ops_t`（A/B 降级为实现细节）。
> **v3.2**：面板显示 kps、OSD 默认开、OSD 视觉规范、`face_source` 单例。
> **v3.3 落实（本轮 8 条）**：
> ① kps **只上 OSD**（面板不显示，改为画 5 个蓝点，§5.3）；
> ② 姓名标签**未 matched 也显示**（内容 `unknown`，§5.3）；
> ③ **板 IP 取自 RTSP 流的 host**，SM 端口用默认 9900（§3.3，配置项减少一个）；
> ④ 重试 **10 次**（1/2/4/8/16 后固定 16s，累计约 111s）；
> ⑤ 重试耗尽后**仅用户操作时重试**（已确认）；
> ⑥ `GetFrameCount()` 的用途说明（§1.2）；
> ⑦ 多路设备**确认不存在** → H1 从"假设"升级为"前提"；
> ⑧ **字体结论（已定）**：后期不会出现中文姓名，识别结果用**字母或编号**（如 `P001`），因此统一用 `montserrat_12`，不引入 CJK 字体、不处理生僻字；中文支持仅作备选记录（§5.5）。

---

## 0. 结论

| 项 | v3 的决定 |
|---|---|
| 自定义嗅探协议 | **删除**。端口 9100、probe/streams/match 三段握手、URL 归一化与四级匹配，全部不要 |
| 能力判定 | `lookupService("FaceService")` → 服务在不在；`queryPublishedTopics()` → 有没有人发 `FaceResult` |
| 订阅后没数据 | **正常态**，不显示内容即可，不报错；连接保持，生产者起来后自动开始显示 |
| 连接/订阅失败 | 重试 5 次（1/2/4/8/16s），仍失败则**停止尝试**，UI 提示 + 留给用户手动重试 |
| 主方案 | **A（原生客户端）**：PC 直连板端 `service_manager:9900` |
| 数据呈现 | **两条出口**：BACKEND DATA 面板（文本全量，节流刷新）+ 画面 OSD（绿框 2px / best 黄框 / 姓名（含未命中的 `unknown`）/ kps 5 蓝点，**默认开**） |
| 连接目标 | **板 IP = 当前 RTSP 流的 host**（从播放地址解析，无需单独配置）；SM 端口默认 **9900** |
| 订阅源 | **抽象成 `face_source_ops_t`（单例）**：线程/状态机封装在实现内，mock / omnibinder / tcp 三种实现可替换 |
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

> **前提 H1（已确认，2026-11-03）**：一台设备上只有一路 face_proc，且 PC 播放的是**这台设备自己**输出的流。**多路设备确认不存在**，因此不需要通道号字段。

- 满足 H1：一切正常，框一定对得上
- 若将来出现多路（需求变更）：框会画在错误的位置，**而且不会报任何错**，届时必须重新引入通道标识

这是砍掉嗅探的**唯一代价**。它可接受的原因：部署形态是"板端既有 RTSP 又有 face"，天然一对一。

**零成本兜底**（三条，都不用新协议）：

1. `GetFrameCount()`（IDL 里已有的 RPC）——**作用：返回 face_proc 累计完成推理的帧数**。它回答的是"服务在不在干活"，而不是"服务在不在"（后者由 `lookupService` 回答）。两种用法：
   - **排除空壳服务**：订阅成功但 3s 没数据时，调两次（间隔 2s）。计数在涨 → face_proc 在推理却没发布（异常，查发布侧）；计数不变 → 推理停摆或进程卡死
   - **交叉验证帧率**：`Δcount / Δt` 应接近 `perf.fps_link`；差得多说明数据面被节流或丢帧
   - 它**不参与主流程**：不阻塞状态迁移，只作为诊断/日志，避免多一次同步 RPC 拖慢连接
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
| `Connecting` | 正在 init / lookup / subscribe | 状态栏灰点「连接识别服务…」 | 失败按退避表重试，**最多 10 次** |
| `Failed` | 10 次都失败 | 黄点「识别服务不可用」+「重试」按钮 | **停止自动重试**，仅在重播/切 URL/点重试时重启 |
| `Subscribed` | 订阅成功，尚无数据 | 灰点「已连接，等待数据」 | 保持长连接，不做额外动作 |
| `NoData` | 订阅成功且超时无数据（含心跳） | 灰点「暂无人脸数据」 | **不重试**：生产者起来后数据自然到达 |
| `Streaming` | 收到 FaceResult（含 `det_count=0` 心跳） | 绿点 + fps / 延迟 | 开始画框；2s 无数据 → 回 `NoData` 并清框 |

关键点：

1. **`NoData` 不重试**——订阅已建立，face_proc 启动后发布者自动出现，不需要重新握手。这条比"循环重连"省得多。
2. **`det_count == 0` 也是数据**——它是心跳（对接文档 §3.3），收到就说明链路活着，状态算 `Streaming`，只是不画框。
3. **只有 `Connecting` 才重试**，且次数封顶，避免"设备根本没有这个服务"时无限打洞。
4. **退避表（已定：10 次）**：`1, 2, 4, 8, 16, 16, 16, 16, 16, 16` 秒，累计约 **111 秒（≈1.9 分钟）**。耗尽后进 `Failed` 并停止；重播 / 切 URL / 点「重试」都会把计数清零重新开始。

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

> 有了 §3.2 的抽象层，这个选择**不再有沉没成本**：A 与 B 都只是 `face_source` 的一个实现，切换 = 换一个源文件 + 改一行配置。

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

### 3.2 订阅源抽象：把"框架差异"关在实现里

订阅本质是"另起一个异步线程拿数据，不同框架方法不同"。所以 UI 绝不能直接依赖任何具体框架，必须收敛成一个 C 接口。

```c
/* src/face/face_source.h —— 纯 C，UI / comm 只依赖这个头 */
typedef enum {
    FACE_IDLE = 0, FACE_CONNECTING, FACE_SUBSCRIBED,
    FACE_STREAMING, FACE_NODATA, FACE_FAILED
} face_state_t;

typedef struct face_source_ops {
    const char *name;                                   /* "mock" / "omnibinder" / "tcp" */
    int  (*open)(const char *addr, int port);           /* 必须立即返回，内部起线程 */
    void (*close)(void);                                /* 幂等，可被 UI 线程调用 */
    face_state_t (*state)(void);                        /* 线程安全 */
    bool (*latest)(face_result_t *out, uint32_t *ver);  /* 单槽读，ver 不变＝无新帧 */
    const char *(*last_error)(void);                    /* 失败原因，给面板 / 日志 */
} face_source_ops_t;

const face_source_ops_t *face_source_active(void);      /* **单例**（已定）：全局唯一，按 config 选实现 */
```

**单例语义（已定）**：一次只订阅一个设备/一条流，与 Play 页一一对应；`open()` 时若已有连接，先 `close()` 再开（幂等切换）。不支持多设备并行——若将来要，把 ops 改成 `ops + ctx` 实例化即可，UI 侧调用形状不变。

**契约**（写进头文件注释，所有实现必须遵守，违反即崩）：

| 条款 | 原因 |
|---|---|
| `open()` 立即返回，连接/订阅在内部线程做 | 调用方是 UI 线程，阻塞就冻画面 |
| `state()` / `latest()` 任意线程可调，内部加锁 | UI 每 50ms 调一次 |
| 实现内部禁止任何 `lv_*` 调用 | LVGL 非线程安全 |
| `close()` 幂等，且必须能打断阻塞中的 `recv` / 同步 API | 切 URL、离页时会并发调用 |
| `ver` 只在**新帧**时递增（同一帧重复读不增） | UI 靠它决定要不要重绘 |

**三种实现**：

| 实现 | 用途 | 要点 |
|---|---|---|
| `face_src_mock.c` | **无板开发 / UI 联调 / 单测** | 内部定时器造数据：`det_count=0` 心跳、1~3 个随机框、缓慢移动的假人、**每框 5 组合法 kps**（OSD 要画点）、姓名字母编号（如 `P001`）。PC 上就能把面板与 OSD 调到满意 |
| `face_src_omnibinder.cpp` | 真机（A） | init → lookup → subscribe；独立线程 `run()` 泵事件循环；回调只写单槽 |
| `face_src_tcp.c` | 备用（B） | 连 bridge，按行解析；纯 C |

选择方式：编译期 `-DFACE_SRC=mock` 只编对应实现（避免 mock 拖进 C++ 库）；运行期也可由 `config.json` 的 `face.source` 指定。

**这一步的实际价值**：不用再纠结 A 还是 B —— 先写 `mock` + 抽象 + 面板 + OSD，UI 侧做完且可测；等板端就绪，只补一个实现文件切过去。A 与 B 的差异被压缩成"多写一个 200 行的 .c 或 .cpp"。

### 3.3 连接目标：从 RTSP 地址推导（已定）

**不新增配置项**，直接用播放地址的 host：

```
do_play("rtsp://192.168.10.168:554/0")
   → 解析 host = 192.168.10.168
   → face_source_open(host, 9900)        ← SM 端口取框架默认
```

| 项 | 取值 |
|---|---|
| SM IP | 播放 URL 的 host（若是域名，先解析成 IP） |
| SM 端口 | **9900**（框架默认，暂不做配置） |
| 覆盖入口 | 保留 `face_host` 配置，**留空＝跟随 URL**（开发期用 IPC 测试流时才填） |

细节：

- 解析失败（URL 里没有 host）→ 直接 `Failed`，**不重试**（这是本地参数错误，重试没有意义）
- 换 URL → 先 `close()` 再 `open()`（单例语义，§3.2）
- 这只解决"**连哪台设备**"，不解决"**是不是这条流**"（后者见 §1.2 H1 前提）

> **默认地址已改（2026-11-03）**：Play 页默认从 `rtsp://192.168.10.248:555/stream1` 改为 **`rtsp://192.168.10.168:554/live0`**（板端 `stream_proc` 的 RTSP 路由为 `/live0`~`/live3`，`live0`=主码流、`live2`=子码流）。这样"板 IP = 流 IP"的前提在开箱状态下成立，`face_host` 无需配置。
> **遗留**：multPlay 页各格默认仍是 `rtsp://192.168.10.206/0`（测试用 IPC），暂未改；它不参与 FaceResult 链路，不影响本方案。

---

## 4. 线程模型（约束在实现内部，抽象层只约定边界）

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

## 5. 数据通路与两条呈现出口

### 5.1 通路：最新值单槽

不需要和画面同步 ⇒ 不排队列。25fps 在 50ms 轮询里必然堆积，队列（64 槽）满了还会丢新帧。

```c
/* 回调线程：覆盖式写入 */
face_store_put(const face_result_t *r);          /* 加锁覆写 + version++ */
/* UI 线程：只在 version 变化时取一次 */
bool face_store_get_if_new(face_result_t *out, uint32_t *last_ver);
```

状态变化（`Connecting/Subscribed/Streaming/Failed`）走**事件队列**，因为它们不能丢。

### 5.2 出口一：BACKEND DATA 面板（文本，全量）

复用 Play 页右侧已有的 `BACKEND DATA` 面板（400px 宽，`DATA_PANEL_W`），把占位的假数据换成真实字段。

**字段分流表**：

| 字段 | 面板 | OSD | 说明 |
|---|---|---|---|
| 链路状态 / 收帧计数 / 丢帧 | ✅ | 否 | 面板顶部一行状态条 |
| `seq` | ✅ | 可选 | OSD 上做调试角标时用 |
| `det_count` | ✅ | 否 | — |
| `perf.fps_link` / `exec_det_ms` / `exec_mbf_ms` / `decode_ms` | ✅ | 可选 | 面板固定显示；OSD 只在"调试角标"开关打开时显示 |
| `score_thr` / `match_thr` | ✅ | 可选 | 调参时看 |
| `best.name` / `best.matched` | ✅ | ✅（框上方 label） | OSD 只在 `matched` 为真时显示姓名 |
| `best.cosine` | ✅ | 可选 | ≥ 阈值才在 OSD 上显示 |
| `dets[i]` 框 ×N | ✅（坐标文本） | ✅（矩形） | 面板显示画布坐标 + 换算后显示坐标 |
| `dets[i].score` | ✅ | 可选 | — |
| `dets[i].kps`（5 点 ×xy） | 否 | **✅（已定）** | 只在画面上画 5 个关键点，**不占面板**（见 §5.3） |
| `alts`（≤4 备选） | ✅（姓名 + 余弦） | 否 | 调参/佐证用 |
| `ts_us` | ✅（**相对时间**：距上一帧差 / 距启动差） | 否 | 单调时钟，不能当墙钟显示 |

**面板行格式**（每框一行；kps 不上面板，见 §5.3）：

```
[00] seq=12841 det=2  st=STREAMING      fps=24.8
[01] best: 张三  matched  cos=0.732  thr=0.55
[02] det=18.2ms mbf=6.1ms dec=4.3ms     score_thr=0.50
[03] ts=+0.041s  rx=12841  drop=0
[04] #0 s=0.98 (210,320)-(360,500)
[05] #1 s=0.91 (410,330)-(520,470)
[06] alt: 李四 0.71 | 王五 0.68
```

- 坐标为**画布坐标**（640×640，含 `pad_top=140`），不做换算——换算后的显示坐标只在 OSD 内部用，面板保持"原始值"便于和板端 `/tmp/media.log` 对照
- 超过 4 个框时只列前 4 个，末行 `... +N more`（面板用于定位问题，不是逐帧观看）
- **文本为英文/数字/编号 → `montserrat_12` 即可**；若将来姓名出现中文，按 §5.5 切 CJK 字体

**性能红线：面板刷新必须节流。** 25fps 全量 `lv_label_set_text()` 重设多行文本，会持续触发文本重排 + 整块重绘，LVGL 单线程会被吃掉。做法：

- 面板按 **4~5Hz（200~250ms）** 刷新，与数据到达频率解耦
- 内容用**分段 label**（状态行 / best 行 / 指标行 / 框列表），只改变化的那几段，避免整块重建
- 框列表用定长缓冲拼接，不动态扩容

### 5.3 出口二：画面 OSD（几何 + 姓名）

**视觉规范（已定）**：

| 元素 | 参数 | 取值 |
|---|---|---|
| 普通框 | 颜色 | **绿色** `0x22C55E` |
| | 线宽 | **2 px** |
| | 填充 / 圆角 | 无填充 / 直角（`radius=0`） |
| **best 框** | 颜色 | **黄色** `0xFACC15`（覆盖绿色，优先最高） |
| 姓名标签 | 字号 | **`lv_font_montserrat_12`**（姓名用字母/编号，拉丁字形足够；若将来出现中文，按 §5.5 切 CJK 字体） |
| | 文字色 / 底色 | 白色 / **黑色 `LV_OPA_60`** 半透明 |
| | 内边距 | 水平 4px、垂直 2px；圆角 2px |
| | 位置 | best 框上沿外侧 4px；上方空间不足时放到框内顶部 |
| | 显示条件（已定） | **`matched` 与否都显示标签**：命中显示 `name`；未命中显示 `unknown`（或 `cos=0.xx`，二选一，建议前者，调参时切后者） |
| **kps 5 点** | 形态 / 尺寸 | 实心圆点，直径 **4px**（`lv_obj` + `radius=50`） |
| | 颜色 | `0x60A5FA` 蓝（与绿/黄框区分，避免和框线混淆） |
| | 数量上限 | **只给前 4 个框画点**（4×5=20 个对象），超出的框只画框不画点 |
| | 顺序 | 左眼 / 右眼 / 鼻尖 / 左嘴角 / 右嘴角（与 IDL 注释一致，不连线） |
| 层级 | z-order | 必须在播放器之上、按钮与面板之下（面板保持可点） |
| 裁剪 | 边界 | 框 clamp 到视频显示矩形内；换算后 `< 4px` 的框丢弃（与板端一致） |

```c
/* ui_app.h 里集中定义，改样式只动这里 */
#define OSD_BOX_COLOR        lv_color_hex(0x22C55E)                    /* 绿：普通框 */
#define OSD_BOX_COLOR_BEST   lv_color_hex(0xFACC15)                    /* 黄：best 框 */
#define OSD_BOX_WIDTH        2
#define OSD_NAME_FONT        &lv_font_montserrat_12                    /* 拉丁/编号 */
#define OSD_NAME_BG_OPA      LV_OPA_60
#define OSD_NAME_PAD_X       4
#define OSD_NAME_PAD_Y       2
#define OSD_MAX_BOXES        16
#define OSD_KPS_MAX_BOXES    4                                         /* 只为前 4 框画点 */
#define OSD_KPS_DOT          4                                         /* 点直径 px */
#define OSD_KPS_COLOR        lv_color_hex(0x60A5FA)                    /* 蓝：关键点 */
```

**效果示意**（best 黄框带姓名，其余绿框）：

```
┌──────────────────────────────────────────┐
│                                          │
│      ┌ P001 ┐                            │
│   ┏━━┷━━━━━━┓        ┌────────┐          │
│   ┃ (黄,best)┃        │  绿     │          │
│   ┗━━━━━━━━━┛        └────────┘          │
│                                          │
└──────────────────────────────────────────┘
```

**实现要点**：

- **对象池**：预创建 `OSD_MAX_BOXES` 个框 + 1 个姓名标签 + `OSD_KPS_MAX_BOXES × 5` 个圆点，按 `det_count` 显隐复用。**绝不每帧创建/删除对象**（LVGL 最常见的掉帧原因）
- 坐标由 §6 归一化换算得到；**坐标没变就只改显隐**，不重复 `set_pos/set_size`（避免无谓的 invalidate）
- OSD 层挂在播放器**父容器**下（与播放器同级、后创建），这样全屏时跟随播放器一起铺满；全屏切换只需重算显示矩形，不用重建对象
- 关 OSD 时**只 `add_flag(HIDDEN)` 整个 OSD 容器**，不要逐个隐藏（一次操作，避免 N 次重绘）
- kps 点与框**共用同一套换算**（`x,y` 各走一遍 §6 公式），不要为点单独写一套
- kps 点是否常显：**跟随 OSD 开关**。若点数影响观看，后续可再加一个"仅 best 显示 kps"的开关，不影响当前实现

### 5.4 按钮控制：OSD 与面板各自独立

Play 页现有三个按钮（Play / Stop / Data）。调整为四个：

| 按钮 | 控制对象 | 默认（已定） | 备注 |
|---|---|---|---|
| Play / Stop | 播放器 | — | 不变 |
| **Data** | BACKEND DATA 面板 | 关 | 现有逻辑保留（切换 `s_overlay` 显隐） |
| **OSD** | 画面叠加框 | **开** | 新增。**全屏时必须仍然可用**（全屏看框才是主要用途） |
| （可选）Debug 角标 | OSD 上的 seq/fps/耗时 | 关 | 与 OSD 分开，避免调参信息干扰观看；本版可不做 |

两个交互细节：

1. 现 `data_btn_cb()` 里有 `if(s_fullscreen) return;` —— OSD 按钮**不能**照抄这个判断，否则全屏就看不了框
2. 面板与 OSD 互不影响：关面板不隐藏框，关 OSD 不影响面板数字

### 5.5 字体策略：姓名用字母/编号（已定）

- 已确认：**后期不会出现中文姓名**，识别结果用**字母或编号**显示（如 `P001`、`zhangsan`）
- 因此姓名与面板文本统一用 **`lv_font_montserrat_12`**，不引入 CJK 字体，也**不需要支持生僻字**
- 面板里的其余内容（`seq`/`fps`/坐标/状态）本来就是拉丁字符，同字体即可，行高一致

**若将来确实要中文**（仅记录备选方案，当前不实施）：

| 方案 | 做法 | 代价 |
|---|---|---|
| 内置 CJK 位图字体 | `lv_conf.h:728` 的 `LV_FONT_SOURCE_HAN_SANS_SC_16_CJK` 已是 `1`，把 `OSD_NAME_FONT` 改指向它即可 | 16px 比 12px 大；只覆盖 1338 常用字，生僻字缺字 |
| Freetype 运行时加载 | 启用 `LV_USE_FREETYPE 1`（当前 `0`） + `lvgl/scripts/built_in_font/SourceHanSansSC-Normal.otf` | 需评估 16MB `LV_MEM_SIZE`；启动变慢 |

> 这一节保留的原因：字体是上线后才暴露的问题。真出现中文需求时，改的是**一个宏**（方案一），不需要动布局。

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

配置模块（`face_host` 覆盖、日志级别；**板 IP 已由 URL 推导、默认 RTSP 已指向板端，不再需要配置**）、日志分级 + 文件轮转、9 宫格软解并发上限、布局参数集中到 `ui_app.h`、退出时真正 `app_pages_destroy()` + `comm_deinit()`。

### P2 · 工程化

CMake 拆 `app_core` + `main`；单测覆盖 **坐标换算 / 解析 / 状态机迁移**；告警清零。

---

## 8. 里程碑

| 里程碑 | 内容 | 验收 |
|---|---|---|
| **M0**（1~2 天） | P0 止血 | 切宫格 50 次 RSS 不涨；构建无新增告警 |
| **M1**（1~2 天） | **抽象层 + mock 实现**：`face_source.h` + `face_src_mock.c`（造心跳/框/移动假人） | 无板情况下 `latest()` 能持续返回新 version；状态机可手动驱动到各态 |
| **M2**（3~5 天） | **BACKEND DATA 面板**接真实字段（先用 mock 数据） | 面板按 4~5Hz 刷新；格式与 §5.2 一致；姓名（字母/编号）正常显示；25fps 下 CPU 无明显上升 |
| **M3**（3~5 天） | **OSD 叠加 + 按钮**：对象池 + 坐标换算 + kps 点 + OSD 开关（含全屏） | 绿框 2px、best 黄框、姓名（未命中显示 `unknown`）、5 个蓝色关键点都正确；mock 假人移动时框与点跟得住；真实源下 1080p/360p 贴合 < 2px；全屏仍可开关 |
| **M4**（2~3 天） | 接真机：`face_src_omnibinder.cpp`（或 tcp）替换 mock | 不动 UI 代码，只改一个实现 + 一处配置即上线 |
| **M5**（1 周） | 产品化 | 8 小时挂机通过；各状态提示正确；断链能自愈 |

**顺序的关键**：M1~M3 全部可以**在没有板子的情况下完成**，因为 mock 顶上。等板端 `setRegisterHost` 就绪，M4 只是"换一个实现文件"。这个排法把"等硬件"从关键路径上摘掉了。

M4 之前先跑一次 C++ 控制台探针（init → lookup → subscribe → 打印 seq/det/name/perf），它就是这个实现的最小可运行版本，代码可以直接长进去。

---

## 9. 决定清单

### 已定（v3.3）

| # | 事项 | 结论 |
|---|---|---|
| 1 | 连接目标 | **板 IP = RTSP 流的 host**，SM 端口 **9900**（默认） |
| 2 | 重试策略 | **10 次**，间隔 `1,2,4,8,16,16,16,16,16,16`s（≈111s） |
| 3 | 重试耗尽 | 进 `Failed`，**仅用户操作**（重播/切 URL/点重试）才重启 |
| 4 | kps | **只上 OSD**（5 个蓝点），面板不显示 |
| 5 | 姓名标签 | `matched` 与否都显示；未命中显示 `unknown` |
| 6 | OSD 视觉 | 绿框 `0x22C55E` 2px / best 黄 `0xFACC15` / 姓名 `montserrat_12` + 黑底 60% / kps 点蓝 4px |
| 7 | OSD 默认 | **开**（全屏仍可用） |
| 8 | 数据源 | `face_source` **单例**，不支持多设备并行 |
| 9 | 多路设备 | **确认不存在** → H1 成立，不需要通道号 |
| 10 | `GetFrameCount()` | 仅作**诊断**（排除空壳服务、交叉验证帧率），不进主流程 |
| 11 | Play 默认地址 | 已改为 `rtsp://192.168.10.168:554/live0`（`ui_page_play.c:190`），开箱即满足"板 IP = 流 IP" |
| 12 | 姓名显示 | **字母或编号**（如 `P001`），后期不会出现中文；不引入 CJK 字体、不支持生僻字 |

### 仍需确认（1 项）

| # | 事项 | 说明 |
|---|---|---|
| A | **板端实施 `setRegisterHost` 的时间点** | 方案 A 的硬前提（**唯一阻塞项**），落地步骤见 `docs/setRegisterHost.md`。不实施的退路是退回 bridge（B），其余设计不变 |

---

## 10. 不要做的事

1. **不要在 UI 线程调任何同步 API**（`lookupService`/`subscribeTopic`/`invoke`）——会冻住画面
2. **不要在 `TopicCallback` 里发起同步 API**——文档明确"不建议"，会自锁
3. **不要两个线程同时驱动 `run()`/`pollOnce()`**——同一实例禁止并发驱动
4. **不要在 `NoData` 时重连**——订阅还在，生产者起来自然就有数据
5. **不要把 FaceResult 排队列**——不同步场景下正确结构是"最新值单槽"
6. **不要在 `ui_page_play.c` 里直接 include 框架头**（`omni_runtime.h` / `face_service.bidl.h`）——UI 只能 include `face_source.h`，否则抽象层白做
7. **不要每帧创建/删除 OSD 对象**——用对象池复用，否则 25fps 下必掉帧
8. **不要让面板跟着 25fps 刷新**——节流到 4~5Hz，且只改变化的分段
9. **不要先做 OSD**——先让状态机和数字对，再让框对
10. **不要用 `lv_canvas` 逐帧画框**——几个矩形用 `lv_obj` 叠加，CPU 差一个数量级

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
