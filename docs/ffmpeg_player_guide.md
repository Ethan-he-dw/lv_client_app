# LVGL FFmpeg Player 使用指南

## 一、概述

`lv_ffmpeg_player` 是 LVGL 内置的视频播放控件，基于 FFmpeg 库实现视频解码和播放。它封装为一个 LVGL 控件（继承自 `lv_image`），可以直接嵌入 UI 布局中，像使用普通图片一样显示视频画面。

---

## 二、环境配置（lv_conf.h）

使用前需要在 `lv_conf.h` 中启用以下配置：

```c
/* 启用 FFmpeg 库 */
#define LV_USE_FFMPEG 1

#if LV_USE_FFMPEG
    /* 是否将输入路径通过 LVGL 文件系统转换（一般设为 0，直接使用系统路径） */
    #define LV_FFMPEG_PLAYER_USE_LV_FS 0

    /* 是否将 FFmpeg 的 format 信息 dump 到 stderr（调试用，一般设为 0） */
    #define LV_FFMPEG_DUMP_FORMAT 0
#endif
```

**CMakeLists.txt 链接依赖**（PC 端）：

```cmake
if(LV_USE_FFMPEG)
    list(APPEND MAIN_LIBS avformat avcodec avutil swscale z)
endif()
```

**FFmpeg 日志控制**（可选）：

```c
#include <libavutil/log.h>

// 设置 FFmpeg 日志级别，减少输出噪音
av_log_set_level(AV_LOG_ERROR);  // 只输出错误

// 注册自定义日志回调，捕获错误信息
static void av_log_cb(void * avcl, int level, const char * fmt, va_list vl) {
    if(level <= AV_LOG_ERROR) {
        vsnprintf(err_buf, sizeof(err_buf), fmt, vl);
    }
}
av_log_set_callback(av_log_cb);
```

---

## 三、API 详解

### 3.1 创建播放器

```c
lv_obj_t * lv_ffmpeg_player_create(lv_obj_t * parent);
```

| 参数 | 说明 |
|------|------|
| `parent` | 父对象，决定播放器在哪个容器中显示 |
| **返回值** | 新创建的 ffmpeg_player 对象指针 |

创建后，播放器是一个空白的图片控件，需要通过 `set_src` 设置视频源。

---

### 3.2 设置视频源

```c
lv_result_t lv_ffmpeg_player_set_src(lv_obj_t * obj, const char * path);
```

| 参数 | 说明 |
|------|------|
| `obj` | ffmpeg_player 对象 |
| `path` | 视频文件路径（本地文件）或 RTSP/RTMP 等网络流地址 |
| **返回值** | `LV_RESULT_OK` 成功，`LV_RESULT_INVALID` 失败 |

**支持的路径格式**：
```
本地文件：  /home/user/video.mp4
Windows：  C:/Videos/test.mp4
RTSP 流：  rtsp://192.168.1.100:554/stream1
RTMP 流：  rtmp://server/live/stream
HTTP 流：  http://example.com/video.mp4
```

---

### 3.3 播放控制命令

```c
void lv_ffmpeg_player_set_cmd(lv_obj_t * obj, lv_ffmpeg_player_cmd_t cmd);
```

| 命令 | 说明 |
|------|------|
| `LV_FFMPEG_PLAYER_CMD_START` | 开始播放（从头开始） |
| `LV_FFMPEG_PLAYER_CMD_STOP` | 停止播放（释放解码上下文，回到第 0 帧） |
| `LV_FFMPEG_PLAYER_CMD_PAUSE` | 暂停播放（保留当前帧画面） |
| `LV_FFMPEG_PLAYER_CMD_RESUME` | 恢复播放（从暂停位置继续） |

---

### 3.4 设置自动重播

```c
void lv_ffmpeg_player_set_auto_restart(lv_obj_t * obj, bool en);
```

| 参数 | 说明 |
|------|------|
| `en` | `true`：视频播放结束后自动从头重播；`false`：播放结束停在最后一帧 |

---

### 3.5 设置解码器

```c
void lv_ffmpeg_player_set_decoder(lv_obj_t * obj, const char * decoder_name);
```

| 参数 | 说明 |
|------|------|
| `decoder_name` | FFmpeg 解码器名称，如 `"h264_cuvid"`（NVIDIA 硬解）、`"h264_qsv"`（Intel 硬解） |

不调用此函数时，使用 FFmpeg 默认的软件解码器。

---

### 3.6 获取帧数

```c
int lv_ffmpeg_get_frame_num(const char * path);
```

| 参数 | 说明 |
|------|------|
| `path` | 视频文件路径 |
| **返回值** | 帧数（≥0 有效，< 0 表示失败） |

---

### 3.7 FFmpeg 初始化/反初始化

```c
void lv_ffmpeg_init(void);    // 注册 FFmpeg 图片解码器
void lv_ffmpeg_deinit(void);  // 注销 FFmpeg 图片解码器
```

通常不需要手动调用，LVGL 会在 `lv_init()` 中自动调用 `lv_ffmpeg_init()`。

---

## 四、常用事件

ffmpeg_player 继承自 `lv_image`，支持所有 `lv_obj` 事件：

| 事件 | 触发时机 |
|------|----------|
| `LV_EVENT_CLICKED` | 点击播放器 |
| `LV_EVENT_DOUBLE_CLICKED` | 双击播放器（常用于全屏切换） |
| `LV_EVENT_SIZE_CHANGED` | 播放器尺寸改变 |

---

## 五、图片对齐方式

由于 ffmpeg_player 继承自 `lv_image`，可以使用图片对齐控制视频画面的缩放和定位：

```c
lv_image_set_inner_align(player, LV_IMAGE_ALIGN_CONTAIN);   // 等比缩放，完整显示
lv_image_set_inner_align(player, LV_IMAGE_ALIGN_COVER);     // 等比缩放，填满裁剪
lv_image_set_inner_align(player, LV_IMAGE_ALIGN_STRETCH);   // 拉伸填满（可能变形）
lv_image_set_inner_align(player, LV_IMAGE_ALIGN_CENTER);    // 原始尺寸居中
```

---

## 六、生命周期与资源管理

```
创建 ──────── 设置源 ──────── 播放 ──────── 停止 ──────── 删除
create()   set_src()     CMD_START    CMD_STOP     lv_obj_delete()
```

**重要**：删除播放器前务必先 `CMD_STOP`，否则可能资源泄漏：

```c
lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_STOP);
lv_obj_delete(player);
```

---

## 七、当前项目中的使用（s_player）

### 7.1 变量定义

```c
static lv_obj_t * s_player;    // FFmpeg 播放器实例
static char g_av_err[256];     // FFmpeg 错误信息缓冲区
static bool s_fullscreen;      // 是否全屏
```

### 7.2 创建与播放流程（do_play）

```
用户点击 Play 按钮
  │
  ├─ 清空错误缓冲区
  ├─ 设置 FFmpeg 日志回调（捕获错误）
  ├─ 删除旧的 s_player（如果存在）
  ├─ 创建新播放器 lv_ffmpeg_player_create()
  ├─ 设置图片对齐 LV_IMAGE_ALIGN_CONTAIN（等比缩放）
  ├─ 注册双击事件（用于全屏切换）
  ├─ 调用 apply_player_layout() 设置位置和大小
  ├─ 设置视频源 lv_ffmpeg_player_set_src()
  │   ├─ 失败 → 显示错误信息，删除播放器
  │   └─ 成功 → 继续
  ├─ 关闭自动重播
  ├─ 发送 CMD_START 开始播放
  └─ 更新状态标签为 "playing"
```

### 7.3 布局管理（apply_player_layout）

播放器支持两种布局模式：

```
普通模式：
┌──────────────────────────────────────┐
│  [RTSP输入框] [Play] [Stop] [Data]   │
│  ┌──────────────┐ ┌───────────────┐  │
│  │              │ │  BACKEND DATA │  │
│  │   视频画面   │ │  (数据面板)    │  │
│  │              │ │               │  │
│  └──────────────┘ └───────────────┘  │
└──────────────────────────────────────┘

全屏模式：
┌──────────────────────────────────────┐
│                                      │
│           视频画面 (全屏)             │
│                                      │
│              (数据面板隐藏)           │
└──────────────────────────────────────┘
```

**普通模式**：
- 播放器位于内容区域，留出顶部 90px 给控制栏
- 数据面板（overlay）显示在右侧，宽度 400px
- 视频宽度 = 内容区宽度 - 数据面板宽度 - 间距

**全屏模式**：
- 播放器覆盖整个屏幕（`lv_display_get_horizontal/vertical_resolution`）
- 数据面板隐藏
- 双击可退出全屏

### 7.4 页面生命周期管理

| 回调 | 行为 |
|------|------|
| `ui_page_play_create` | 创建 RTSP 输入框、按钮、数据面板等控件（不创建播放器） |
| `ui_page_play_on_show` | 恢复播放器显示 + 发送 CMD_START（如果之前在播放） |
| `ui_page_play_on_hide` | 发送 CMD_STOP + 隐藏播放器 + 退出全屏 |
| `ui_page_play_destroy` | 彻底停止 + 删除播放器 + 清空缓冲区 |
| `ui_page_play_set_visible` | 控制播放器可见性（非 Play 页时隐藏，避免浮在其他页面上） |

### 7.5 双击全屏切换

```c
// 注册双击事件
lv_obj_add_event_cb(s_player, player_dbl_cb, LV_EVENT_DOUBLE_CLICKED, NULL);

static void player_dbl_cb(lv_event_t * e) {
    s_fullscreen = !s_fullscreen;
    if(s_fullscreen) {
        // 全屏：隐藏数据面板，扩大播放器到整个屏幕
        lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 普通：恢复数据面板，播放器回到内容区域
        lv_obj_remove_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    apply_player_layout();  // 重新计算布局
}
```

### 7.6 错误处理

```c
// 注册自定义 FFmpeg 日志回调
av_log_set_level(AV_LOG_ERROR);
av_log_set_callback(av_log_cb);

// 当 set_src 失败时，检查 FFmpeg 错误信息
if(lv_ffmpeg_player_set_src(s_player, url) != LV_RESULT_OK) {
    if(g_av_err[0] != '\0')
        lv_label_set_text(s_status_lbl, g_av_err);  // 显示 FFmpeg 错误
    else
        lv_label_set_text(s_status_lbl, "open failed");  // 通用错误
}
```

---

## 八、使用注意事项

| 事项 | 说明 |
|------|------|
| **删除前先停止** | 删除播放器前必须发送 CMD_STOP，否则 FFmpeg 解码上下文会泄漏 |
| **不要在渲染回调中操作播放器** | 避免在 `LV_EVENT_DRAW_*` 中调用 `set_cmd`/`set_src`，会触发断言 |
| **RTSP 流地址** | 需要网络可达，超时可能导致 LVGL 主循环阻塞（建议用独立线程） |
| **播放器创建在 lv_screen_active()** | 当前项目中 `do_play` 创建播放器在 `lv_screen_active()` 上（非页面容器），所以需要手动管理其可见性 |
| **内存占用** | FFmpeg 解码器 + 帧缓冲区占用较大内存，播放器停止后资源才释放 |
| **LVGL 主循环阻塞** | 如果 FFmpeg 解码耗时过长，会导致 UI 卡顿。可以考虑降低视频分辨率或使用硬件加速解码器 |

---

## 九、常见问题

### Q1: 播放器无画面？
- 检查 `lv_ffmpeg_player_set_src()` 返回值
- 确认视频路径/URL 正确且可访问
- 检查 FFmpeg 错误日志（通过自定义回调捕获）

### Q2: 播放器浮在其他页面上？
- 因为播放器创建在 `lv_screen_active()` 而非页面容器中
- 需要在页面切换时手动调用 `ui_page_play_set_visible(false)` 隐藏

### Q3: 播放 RTSP 流时启动很慢？
- FFmpeg 建立 RTSP 连接需要时间，可能阻塞主循环
- 可以考虑在独立线程中打开流，或者增加加载动画

### Q4: 如何调整视频画质/性能？
- 使用硬件加速解码器：`lv_ffmpeg_player_set_decoder(player, "h264_cuvid")`
- 降低源视频分辨率
- 调整 FFmpeg 的线程数等参数

---

## 十、代码示例

### 最简播放示例

```c
lv_obj_t * player = lv_ffmpeg_player_create(lv_screen_active());
lv_obj_set_size(player, 640, 480);
lv_obj_center(player);
lv_ffmpeg_player_set_src(player, "test.mp4");
lv_ffmpeg_player_set_auto_restart(player, true);
lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_START);
```

### RTSP 流播放示例

```c
lv_obj_t * player = lv_ffmpeg_player_create(lv_screen_active());
lv_obj_set_size(player, 800, 600);
lv_image_set_inner_align(player, LV_IMAGE_ALIGN_CONTAIN);

if(lv_ffmpeg_player_set_src(player, "rtsp://192.168.1.100:554/stream1") == LV_RESULT_OK) {
    lv_ffmpeg_player_set_auto_restart(player, false);
    lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_START);
}
```

### 带控制的播放器

```c
// 创建
lv_obj_t * player = lv_ffmpeg_player_create(parent);
lv_ffmpeg_player_set_src(player, url);

// 播放
lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_START);

// 暂停
lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_PAUSE);

// 恢复
lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_RESUME);

// 停止
lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_STOP);

// 安全删除
lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_STOP);
lv_obj_delete(player);