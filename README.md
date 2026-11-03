# LVGL 视频客户端 App（PC 模拟器版）

基于 [LVGL](https://github.com/lvgl/lvgl) 9.6 + SDL2 打造的 PC 端视频监控客户端原型。在官方 `lv_port_pc_vscode` 模拟器基础上改造成一个完整的桌面应用：左侧导航栏 + 多页面切换、RTSP 视频播放（单路/多路宫格）、UDP 设备发现、固件升级与调试命令下发。

所有界面代码可直接移植到嵌入式平台 —— LVGL 部分与硬件无关，只需替换 `src/hal/hal.c` 中的显示与输入驱动。

![platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)
![lvgl](https://img.shields.io/badge/LVGL-9.6-blue)
![licence](https://img.shields.io/badge/licence-MIT-green)

---

## 功能

| 模块 | 能力 |
|---|---|
| **Play** | 输入 RTSP 地址播放单路视频，双击切换全屏，右侧「BACKEND DATA」面板实时显示后端推送数据 |
| **multPlay** | 多路播放页，支持 4 / 6 / 8 / 9 宫格（2×2、3×2、4×2、3×3）自由切换，每格独立输入地址与播控，支持「play all」一键全播、单格双击全屏 |
| **Upgrade** | 搜索局域网设备 → 选择固件文件 → 升级进度条；**升级进行中尝试离开页面会弹出确认框拦截** |
| **Debug** | 选择目标设备下发调试命令，实时滚动日志窗口 |
| **导航** | 侧边栏 4 页切换 + 手势上下翻页，当前页高亮 |

## 界面结构

```
┌────────────┬──────────────────────────────────────────────┐
│  CLIENT APP│                                              │
│ ┌────────┐ │   内容区（1600×900 窗口，当前页占满）          │
│ │  Play  │ │                                              │
│ ├────────┤ │                                              │
│ │Upgrade │ │                                              │
│ ├────────┤ │                                              │
│ │ Debug  │ │                                              │
│ ├────────┤ │                                              │
│ │multPlay│ │                                              │
│ └────────┘ │                                              │
└────────────┴──────────────────────────────────────────────┘
   160px 固定宽            flex_grow(1) 自适应
```

## 架构

三层结构，自上而下单向依赖：

```
main.c  →  lv_init() → sdl_hal_init() → comm_init() → ui_init() → while(1) lv_timer_handler()
             │
   ┌─────────┴──────────┬───────────────────┬──────────────────┐
   │  UI 层  src/ui      │ 通信层 src/comm   │ 平台层            │
   │ 页面注册表 + 4 页面  │ 事件队列 + 设备表  │ hal.c(SDL) + LVGL │
   └─────────────────────┴───────────────────┴──────────────────┘
```

核心设计是 **页面注册表**（`src/ui/ui_app.h`）：每个页面实现五个生命周期回调，在 `ui.c` 的 `s_page_ops[]` 中登记即可，新增页面无需改动框架代码。

```c
typedef struct {
    void (*create)(lv_obj_t *parent);         /* 创建控件 */
    void (*on_event)(const comm_event_t *);   /* 接收通信事件 */
    void (*on_show)(void);                    /* 页面变为可见 */
    bool (*on_hide)(int target_page);         /* 离开前拦截：false = 拒绝离开 */
    void (*destroy)(void);                    /* 释放资源 */
} page_ops_t;
```

通信采用「后台线程写队列 + LVGL 主线程 50ms 轮询消费」的单线程 UI 模型，跨线程数据全部经由带互斥锁的环形队列传递，避免在非 UI 线程触碰控件。

## 目录结构

```
├── src/
│   ├── main.c                    入口：初始化链 + 主循环
│   ├── hal/hal.c                 SDL 显示/鼠标/滚轮/键盘驱动
│   ├── comm/                     通信门面 + UDP 收发
│   └── ui/
│       ├── ui.c                  应用骨架：布局、页面切换、事件轮询
│       ├── ui_app.h              全局上下文 / 页面枚举 / 主题色
│       ├── ui_sidebar.c          左侧导航栏
│       ├── ui_page_play.c        单路播放页
│       ├── ui_page_play_multplayer.c  多路宫格页
│       ├── ui_page_upgrade.c     升级页
│       ├── ui_page_debug.c       调试页
│       └── ui_file_dlg.c         文件选择对话框
├── lvgl/                         LVGL 9.6 源码（已补充中文注释）
├── docs/                         架构梳理与问题排查记录
├── CMakeLists.txt
└── lv_conf.h                     LVGL 配置
```

## 构建

### 环境（已验证）

Windows + **MSYS2 UCRT64** 工具链，CMake + Ninja 构建：

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc \
          mingw-w64-ucrt-x86_64-cmake \
          mingw-w64-ucrt-x86_64-ninja \
          mingw-w64-ucrt-x86_64-SDL2 \
          mingw-w64-ucrt-x86_64-SDL2_image \
          mingw-w64-ucrt-x86_64-libpng \
          mingw-w64-ucrt-x86_64-freetype \
          mingw-w64-ucrt-x86_64-ffmpeg
```

Linux 下对应 `build-essential cmake libsdl2-dev libsdl2-image-dev libpng-dev libfreetype6-dev libavformat-dev libavcodec-dev libswscale-dev`；macOS 可用 Homebrew 安装同名包。也可用 vcpkg（`vcpkg install sdl2 sdl2-image libpng freetype ffmpeg`）。

### 编译运行

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
./bin/main.exe          # Linux/macOS 为 ./bin/main
```

VS Code 用户可直接打开 `simulator.code-workspace`，按 F5 使用预置的 GDB 调试配置。

### 可选编译开关

| 选项 | 默认 | 说明 |
|---|---|---|
| `USE_FREERTOS` | OFF | 启用 FreeRTOS 版入口（`src/freertos_main.c`）替代 `main.c` |
| `LV_USE_DRAW_SDL` | ON | SDL 硬件加速绘制单元 |
| `LV_USE_FFMPEG` | ON | 视频播放能力（依赖 avformat/avcodec/avutil/swscale） |
| `LV_USE_LIBPNG` | ON | PNG 解码 |
| `LV_USE_LIBJPEG_TURBO` | OFF | JPEG 硬解 |
| `LV_USE_FREETYPE` | ON | TrueType 字体渲染 |

## 关键配置（lv_conf.h）

```c
#define LV_USE_OS            LV_OS_SDL2      /* 与 FreeRTOS 互斥，决定入口走哪个 main */
#define LV_MEM_SIZE          (16 * 1024 * 1024)
#define LV_DEF_REFR_PERIOD   33              /* ms */
#define LV_USE_FFMPEG        1
#define LV_USE_FILE_EXPLORER 1               /* 升级页选文件依赖 */
```

窗口分辨率为 **1600×900**，在 `main.c` 的 `sdl_hal_init(1600, 900)` 中设定；界面内有大量按该分辨率布置的像素坐标。

## 通信协议

UDP，端口 `8888`：

| 方向 | 报文 | 说明 |
|---|---|---|
| 客户端 → 广播 | `MYAPP_SEARCH` | 搜索局域网设备 |
| 设备 → 客户端 | `MYAPP_DEV:<id>` | 应答设备 ID，客户端登记 IP + ID |
| 客户端 → 设备 | `MYAPP_UPGRADE:<path>` | 通知设备拉取固件 |
| 客户端 → 设备 | 任意文本 | 调试命令原样下发 |

当前 `src/comm/comm.c` 内置一个 1 秒周期的模拟线程，用于在没有真实设备时产生设备发现、升级进度、后端数据与日志事件，便于界面联调。接入真实设备时移除该线程即可。

## 新增一个页面

1. 在 `src/ui/ui_app.h` 的 `app_page_id_t` 枚举中加入索引（放在 `APP_PAGE_COUNT` 之前）
2. 在 `src/ui/ui.c` 的 `s_page_ops[]` 中登记该页面的五个回调
3. 在 `src/ui/ui_sidebar.c` 的 `s_btn_names[]` 中补上按钮名称

CMake 使用 `file(GLOB)` 收集 `src/ui/*.c`，新增源文件后重新 configure 即自动参与编译。

## 已知限制

- 窗口分辨率与部分控件尺寸（播放区 720px、日志区 746px 等）为硬编码，缩放窗口不会自适应
- `video_Null.png` 占位图在代码中以绝对路径引用，换机器需调整
- 通信层 `comm_udp.c` 目前仅实现 Windows Socket，其他平台为空桩
- 多路播放的软解码并发受 CPU 限制，9 宫格在低配机器上可能掉帧

## 文档

| 文档 | 内容 |
|---|---|
| [docs/project_map.md](docs/project_map.md) | 项目整体梳理：启动链路、页面注册表机制、通信层、技术债清单 |
| [docs/ffmpeg_player_guide.md](docs/ffmpeg_player_guide.md) | `lv_ffmpeg_player` 使用指南与配置要点 |
| [docs/multplay_page_freeze_debug.md](docs/multplay_page_freeze_debug.md) | 多路页切换卡死（canvas 0×0）根因与修复 |
| [docs/textarea_jumping_debug.md](docs/textarea_jumping_debug.md) | RTSP 输入框文字跳动问题的完整排查过程 |

## 许可

MIT，见 [licence.txt](licence.txt)。LVGL 本身同样以 MIT 协议发布。
