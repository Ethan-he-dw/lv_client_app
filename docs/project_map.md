# 项目梳理（Project Map）

> 生成时间：2026-11-03 ｜ 依据：`src/` 全部源码 + `CMakeLists.txt` + `lv_conf.h`
> 状态：`ninja -C build -n` → no work to do（当前可正常编译，产物 `bin/main.exe`）

---

## 1. 这是什么

在 LVGL 官方 `lv_port_pc_vscode`（PC 端 SDL 模拟器）基础上**改造成一个"客户端 App"**：

- 左侧固定侧边栏导航 + 右侧内容区，四页切换（Play / Upgrade / Debug / multPlay）
- UDP 广播做设备发现，向上做固件升级、调试命令下发
- 用 `lv_ffmpeg_player` 播放 RTSP 视频，支持 1 路（Play 页）和多路网格（multPlay 页，4/6/8/9 宫格）
- 目前通信层跑在 **Windows Socket 真实现 + 1 秒周期的模拟数据线程**上（后端设备尚未接入）

启动后默认进入 **multPlay 页**（`ui_init()` 末尾 `app_show_page(APP_PAGE_MULTPLAY)`）。

---

## 2. 目录结构

```
lv_port_pc_vscode/
├── src/
│   ├── main.c                   ← 唯一入口（111 行）
│   ├── freertos_main.c          ← FreeRTOS 版本入口（USE_FREERTOS=ON 时替代 main.c，当前未启用）
│   ├── hal/hal.c                ← SDL 显示 + 鼠标/滚轮/键盘输入 + 鼠标光标图
│   ├── comm/
│   │   ├── comm.c/.h            ← 通信门面：设备表 + 事件队列 + 模拟线程
│   │   └── comm_udp.c/.h        ← UDP 收发包（Windows 实现；非 Windows 为空桩）
│   ├── ui/
│   │   ├── ui.c/.h              ← 应用骨架：布局、页面注册表、页面切换、事件轮询
│   │   ├── ui_app.h             ← 全局上下文 g_app / page_ops_t / 页面 ID / 主题色 / 对外接口
│   │   ├── ui_sidebar.c         ← 左侧导航栏（4 个按钮，回调 → app_show_page）
│   │   ├── ui_page_play.c       ← 单路播放页（RTSP 输入 + 右侧 BACKEND DATA 面板 + 双击全屏）
│   │   ├── ui_page_play_multplayer.c ← 多路播放页（宫格）+ 一段旧的单通道页（死代码）
│   │   ├── ui_page_upgrade.c    ← 升级页（搜索设备 / 选文件 / 进度条 / 升级中禁止离开）
│   │   ├── ui_page_debug.c      ← 调试页（选设备 / 发命令 / 日志窗）
│   │   ├── ui_file_dlg.c        ← 文件选择对话框（lv_file_explorer 封装）
│   │   └── ui_learn.c           ← LVGL 学习示例集合（693 行，main 里已注释，但仍参与编译）
│   └── mouse_cursor_icon.c      ← 光标图片资源
├── config/FreeRTOSConfig.h
├── docs/                        ← 3 份历史排查记录 + 本文档
├── lvgl/                        ← LVGL 子模块（v9.6.0-dev）
├── FreeRTOS/                    ← 可选 RTOS 源码（当前 OFF）
├── CMakeLists.txt               ← 构建脚本（自动 glob src/ui、src/comm）
└── bin/main.exe                 ← 构建产物
```

---

## 3. main.c 逐段解读（项目的"总纲"）

```c
#if LV_USE_OS != LV_OS_FREERTOS     // 编译期开关；lv_conf.h 里是 LV_OS_SDL2 → 走这一条
int main(int argc, char **argv)
{
  lv_init();                        // ① LVGL 内核初始化
  sdl_hal_init(1600, 900);          // ② 1600x900 窗口 + 3 个输入设备（鼠标/滚轮/键盘）
  comm_init();                      // ③ 通信层必须在 UI 之前（页面创建时就要读设备表）
  ui_init();                        // ④ 整个 UI 树 + 页面 + 事件轮询定时器

  while(1) {                        // ⑤ 主循环：LVGL 心跳
    uint32_t sleep_time_ms = lv_timer_handler();
    if(sleep_time_ms == LV_NO_TIMER_READY) sleep_time_ms = LV_DEF_REFR_PERIOD;
    usleep(sleep_time_ms * 1000);   // 把 LVGL 算出的"下次该跑的时间"当作休眠
  }
}
#endif
```

要点：

| 顺序 | 调用 | 为什么必须在这个位置 |
|---|---|---|
| ① | `lv_init()` | 之后所有 `lv_*` 才合法 |
| ② | `sdl_hal_init(1600,900)` | 创建 display，**分辨率在这里定死**；UI 里大量写死像素（PLAYER_H=720、日志区 746 等）都依赖 1600x900 |
| ③ | `comm_init()` | 页面 `create` 阶段就会 `comm_get_device_count()` 填下拉框，所以通信层必须先就绪 |
| ④ | `ui_init()` | 建侧边栏 + 一次性创建全部 4 个页面（创建即隐藏）+ 显示 multPlay + 起 50ms 轮询定时器 |
| ⑤ | `lv_timer_handler()` 循环 | 单线程模型：LVGL 的定时器、动画、输入、渲染全在这一个循环里推进 |

已被注释掉的备选入口（说明这项目是"学习 → 产品"演进而来）：

```c
//lv_demo_benchmark();  //lv_demo_widgets();
//ui_learn_lvgl_init(); //ui_learn_layout_init();
//ui_learn_otherwidgets_init(); //ui_learn_canvas_init();
```

---

## 4. UI 骨架：页面注册表机制（这是本项目最核心的设计）

`ui_app.h` 定义了一张"页面能力表"，新增页面只需 2 步，不用改 `ui.c` 的流程代码。

```c
typedef enum { APP_PAGE_PLAY=0, APP_PAGE_UPGRADE, APP_PAGE_DEBUG, APP_PAGE_MULTPLAY, APP_PAGE_COUNT } app_page_id_t;

typedef struct {
    void (*create)(lv_obj_t *parent);         // 建控件
    void (*on_event)(const comm_event_t *);   // 收通信事件
    void (*on_show)(void);                    // 变可见
    bool (*on_hide)(int target_page);         // 离开前拦截：false = 拒绝离开
    void (*destroy)(void);                    // 释放资源
} page_ops_t;
```

`ui.c` 中的 `s_page_ops[APP_PAGE_COUNT]` 就是注册表，四个页面各自登记。

### 初始化时序

```
ui_init()
 ├─ app_layout_init(screen)        屏 → root(flex row) → [sidebar 160px] + [content flex_grow=1]
 ├─ ui_sidebar_create(sidebar)     4 个按钮，回调带索引 → app_show_page(idx)
 ├─ app_pages_create()             循环 4 次：建容器(100%x100%, 隐藏) → 调 s_page_ops[i].create()
 │   ├─ Play:   RTSP 输入 + Play/Stop/Data + BACKEND DATA 面板
 │   ├─ Upgrade:Search + 设备下拉 + 选文件 + 进度条
 │   ├─ Debug:  Search + 设备下拉 + 命令框 + 日志窗
 │   └─ multPlay: 宫格容器(75%) + 下拉 CH4/6/8/9 + play all + 信息栏
 ├─ app_show_page(APP_PAGE_MULTPLAY)   默认页 → on_show() 里用 10ms 定时器延迟 rebuild_players(0)
 └─ app_poll_timer_init()              50ms 定时器
```

### 切换时序（`app_show_page`）

```
app_show_page(idx)
 ① 旧页 on_hide(idx)  ── false ──> 直接 return（例：升级中弹框拦住）
 ② 旧页 add HIDDEN
 ③ 新页 remove HIDDEN → on_show()（更新侧边栏高亮 / 刷新设备列表 / 恢复播放）
 ④ ui_page_play_set_visible(idx==PLAY)   把 Play 页的 ffmpeg 播放器一起藏起来
 ⑤ g_app.current_page = idx
```

### 事件流（跨线程 → UI 单线程）

```
comm 模拟线程 / UDP 收包线程        LVGL 主循环
        │ queue_push()                    │
        ▼                                 ▼
   [环形队列 64 槽 + 互斥锁] ──comm_pull_event()──> poll_timer_cb(50ms)
                                                      │ 广播给 4 个页面
                                                      ▼
                                       s_page_ops[i].on_event(&evt)
```

注意：**4 个页面都会收到全部事件**（页面自己按 `evt->type` 过滤），隐藏页面也会刷新。

---

## 5. 通信层

```
comm.c（门面，平台无关）
 ├─ 设备表 s_devs[32] + 锁
 ├─ 事件环形队列 s_q[64] + 锁
 ├─ comm_search_devices() → 清空设备表 → 广播
 ├─ comm_start_upgrade()  → 发 "MYAPP_UPGRADE:<path>"，置 s_upgrading
 ├─ comm_send_debug_cmd() → 发原文 + 本地回显 LOG 事件
 └─ sim_thread（每 1s）：SEARCH_DONE / UPGRADE_PROGRESS(+8%) / BACKEND_DATA(假 JSON) / LOG(uptime)

comm_udp.c
 ├─ 端口 8888，SO_BROADCAST
 ├─ 广播 "MYAPP_SEARCH"；收 "MYAPP_DEV:<id>" → 回调登记设备
 └─ 非 Windows 编译为空桩（项目实际在 Windows 上跑）
```

事件类型：`SEARCH_DONE / DEVICE_FOUND / BACKEND_DATA / LOG / UPGRADE_PROGRESS`

---

## 6. 构建与配置

- **构建**：CMake + Ninja，`cmake -B build && cmake --build build`；产物 `bin/main.exe`（`main` target）
- **源文件收集**：`file(GLOB src/ui/*.c)`、`src/comm/*.c` → 新增文件**不用改 CMakeLists**，但要重新 configure 才生效
- **关键开关**：`USE_FREERTOS`(OFF)、`LV_USE_DRAW_SDL`(ON)、`LV_USE_FFMPEG`(ON)、`LV_USE_LIBPNG`(ON)、`LV_USE_FREETYPE`(ON)、`LV_USE_LIBJPEG_TURBO`(OFF)
- **lv_conf.h 要点**：`LV_USE_OS = LV_OS_SDL2`、`LV_MEM_SIZE = 16MB`、`LV_DEF_REFR_PERIOD = 33ms`、`LV_USE_FILE_EXPLORER = 1`
- **链接库**：lvgl / lvgl::examples / lvgl::demos / thorvg / SDL2 / SDL2_image / png / freetype / avformat avcodec avutil swscale z / 平台库(m,pthread 或 ws2_32)

---

## 7. 已知问题 / 技术债（按优先级）

| # | 位置 | 问题 | 影响 |
|---|---|---|---|
| 1 | `ui.c:150,154` | `(nowpage + 1)/APP_PAGE_COUNT`，应为 `%`（取模） | 侧边栏手势翻页逻辑错误，永远算不到目标页 |
| 2 | `ui_page_play_multplayer.h:58-63` | 声明的是 `ui_multpage_play_*`，实际实现叫 `ui_page_multplay_*` | 头文件与实现不符，声明全是死声明 |
| 3 | `ui_page_play_multplayer.h:24-29` | `static const ch_grid_t s_grids[]` 放在头文件里 | 每个包含它的 .c 各生成一份副本，易产生"未使用"告警 |
| 4 | `ui_page_play_multplayer.c:802` | `ui_page_multplayer_create()` 是旧的单通道页（含 BACKEND DATA 面板），无人调用 | 死代码 ~110 行，且与新版共用 `s_page/s_ip_ta/s_overlay` 等静态变量，误接上去会串状态 |
| 5 | `ui_page_play_multplayer.c:416,499` | 硬编码 `M:/LVGL_learn/lv_port_pc_vscode/video_Null.png` | 换机器/换盘符直接黑屏 |
| 6 | `ui_page_play_multplayer.c:476` | `creat_single_palyer` 缺 `static`，且拼写应为 player | 暴露全局符号；拼写不一致易混淆 |
| 7 | `CMakeLists.txt:81-82` | 同一句 `list(APPEND MAIN_SOURCES ...)` 写了两遍 | 冗余（CMake 会去重，无害但易误导） |
| 8 | `CMakeLists.txt:42` | `include_directories(${PROJECT_SOURCE_DIR}/main/inc)`，该目录不存在 | 无效配置，遗留 |
| 9 | `ui.c:126-135` | 事件广播给全部页面，含隐藏页 | 隐藏页仍在刷新控件（日志窗/下拉框），无谓开销 |
| 10 | `ui.c:61,130,141,160,233` 等 | 大量 `printf` 调试输出（`app_log.txt` 即由此产生） | 发布版噪音、影响帧率 |
| 11 | `comm.c:139-145` | 模拟线程每秒必推一条 LOG | Debug 页日志恒增长（靠 16KB 缓冲区截断兜底） |
| 12 | `ui_learn.c`（693 行） | 学习示例，仍被 glob 编译，main 里只注释保留 | 增大编译量，干扰阅读 |
| 13 | 多处布局 | 写死像素（`PLAYER_H 720`、日志区 `746`、`PLAYER_TOP 90`） | 窗口 resize 后不自适应 |
| 14 | `ui_file_dlg.c:69` | `lv_file_explorer_open_dir(fe, "A:C:/")` 盘符 hack | 换盘符/换系统失效 |

---

## 8. 建议的下一步（未执行，待确认）

1. **清死代码**：删 `ui_page_multplayer_create()`（802-913）与 `ui_multpage_play_*` 死声明；`ui_learn.c` 移出 `src/ui/` 或加编译开关。
2. **修 bug**：手势 `%` 取模；`s_grids` 移到 .c；`creat_single_palyer` 加 `static` 并改名。
3. **去硬编码**：`video_Null.png` 改为相对路径（`LV_FFMPEG_PLAYER_USE_LV_FS` 或基于 `argv[0]` 拼绝对路径）。
4. **统一日志**：`printf` → `LV_LOG_*` 宏，Debug 页的 LOG 走统一出口。
5. **分层收口**：事件只派发给当前可见页（`app_show_page` 里只调 `s_page_ops[current].on_event`），或给页面加 `visible` 判断。
6. **抽出配置头**：分辨率、侧边栏宽、播放区高度集中到 `ui_app.h`，为后续自适应铺路。
