# RTSP 地址输入框文字“跳动”问题 — 完整排查记录

- 日期：2026-08-10
- 环境：LVGL v9.6.0-dev（官方 `lv_port_pc_vscode` 包）+ SDL 模拟器，MinGW (UCRT64)，Windows
- 现象：播放页面 RTSP 地址输入框（`lv_textarea`）内的文字在垂直方向**上下往复滑动**，永不停止；滑动速度开始时较快，后变慢但仍存在。

---

## 一、排查工具与方法

本问题的定位完全依赖以下手段（按使用顺序）：

| 手段 | 说明 |
|---|---|
| 截屏像素对比 | PowerShell + System.Drawing 对程序窗口连续截屏，逐像素比对，输出差异区域 |
| ASCII 可视化 | 将截图区域按亮度映射为字符（`#` `+` `=` `-` `.`），“人眼”观察布局 |
| 应用内调试日志 | 在 LVGL 定时器/事件回调中写入 `scroll`、label 坐标等数据到文件 |
| 事件流日志 | 给 textarea 挂 `LV_EVENT_ALL` 回调，记录每个事件号 + 滚动值 |
| 调用栈捕获 | `CaptureStackBackTrace` 抓 SCROLL_BEGIN 的调用栈 |
| LVGL 源码阅读 | 逐一核对 `lv_textarea.c` / `lv_obj_scroll.c` / `lv_draw_sdl.c` 的实现 |
| 对照实验 | 每次只改一个变量（关光标闪烁 / 换纯 label / 关滚动条…），确认/排除嫌疑 |

---

## 二、排查时间线

### 阶段 1：确认现象，锁定变化区域

**截屏分析**：窗口 1600×900，以 400ms 间隔截 6 帧，逐像素对比。

- 文本框区域（capture 坐标 x=317..522, y=62..68）出现约 205px 宽的横条，字形**周期性地出现/消失**
- 周期约 1.6~2s，与程序内 1s 周期的模拟事件、光标闪烁周期都不完全吻合
- 文本框边框位置稳定 → 移动的是**文字本身**，不是整个控件

**ASCII 可视化**（step 10px / 4px）：

```
=== shot0 ===   === shot2 ===
  =+++++...       =+++++...     ← 文本框上部（E0 底色）
  ###...###       ###...###     ← 中部白色带（内容区）
  =+++++...       =+++++...     ← 文本框下部
                   +            ← shot2 多出光标（位置不对！）
```

- 文本框内容区被画成**白色**（正常应为 E0E0E0），光标出现在文本框**右下角错误位置**
- 关键线索：白色带 16px ≈ 一行文字高度 → 是 label 的绘制区域

### 阶段 2：逐一排除表面嫌疑

| 实验 | 改动 | 结果 |
|---|---|---|
| A | 光标闪烁关掉 `anim_duration(ta,0,LV_PART_CURSOR)` | ❌ 仍跳动（且文字行位置变了 10px，说明位置不稳定） |
| B | 去掉 `lv_textarea_set_one_line()` | ❌ 仍跳动 |
| C | 禁用 poll timer（不派发 comm 事件） | 只闪 1 次后稳定 → **事件派发只是“陪跑”，不是根因** |
| D | 去掉 `LV_OBJ_FLAG_SCROLLABLE` | 跳动明显变慢 → **滚动确实是主运动**，但没拦住 |

### 阶段 3：应用内日志 — 抓到“元凶运动”

在 100ms 定时器里输出 textarea 的 `scroll` 与 label 坐标：

```
cnt=1  scroll=(0,45)  label=(313,46)-(686,61)
cnt=2  scroll=(0,70)  label=(313,21)-(686,36)   ← label 在顶部
cnt=3  scroll=(0,37)  label=(313,54)
cnt=4  scroll=(0,4)   label=(313,87)            ← label 在底部
cnt=14 scroll=(0,0)   label=(313,91)
cnt=16 scroll=(0,70)  label=(313,21)            ← 又回顶部
...（0~70 往复振荡，永不收敛）
```

**实锤**：`scroll_top` 在 0..70 间反复振荡，label 被垂直滚动 —— 这就是“上下往复滑动”。

### 阶段 4：事件流 — 找到触发循环

给 textarea 挂 `LV_EVENT_ALL` 回调，记录事件号与滚动值（解码：12=SCROLL_BEGIN, 15=SCROLL, 40=STATE_CHANGED, 51=STYLE_CHANGED, 50=SIZE_CHANGED）：

```
evt=12(SCROLL_BEGIN) scroll=0
evt=51(STYLE_CHANGED) scroll=0
evt=40(STATE_CHANGED) scroll=0
evt=12(SCROLL_BEGIN) scroll=0      ← 第二次 SCROLL_BEGIN！
evt=14(SCROLL_END) scroll=0
...DRAW 事件...
evt=15(SCROLL) scroll=19           ← 动画运行中，数值递增
evt=15(SCROLL) scroll=34
...（循环重复）
```

**结论**：滚动不是一次性操作，而是被反复触发 —— 每次 `SCROLL_BEGIN → STYLE_CHANGED（滚动条状态变化）→ 再次触发滚动` 的连锁。

调用栈（`CaptureStackBackTrace`）显示每次 SCROLL_BEGIN 的调用链**完全相同** → 同一个触发路径在循环。

### 阶段 5：源码定位 — 三个“隐藏”环节

1. **`lv_textarea.c` 的 `lv_textarea_event`**（约 931 行）：
   ```c
   else if(code == LV_EVENT_SIZE_CHANGED || code == LV_EVENT_STYLE_CHANGED) {
       lv_textarea_scroll_to_cursor_pos(obj, ta->cursor.pos);
   }
   ```
   → 任何尺寸/样式变化都会去滚动。

2. **`lv_textarea_scroll_to_cursor_pos`（约 1409 行）** 使用 **`LV_ANIM_ON`** 动画滚动：
   ```c
   if(cur_pos.y < lv_obj_get_scroll_top(obj)) lv_obj_scroll_to_y(obj, cur_pos.y, LV_ANIM_ON);
   if(cur_pos.y + font_h - lv_obj_get_scroll_top(obj) > h)
       lv_obj_scroll_to_y(obj, cur_pos.y - h + font_h, LV_ANIM_ON);
   ```
   动画期间 label 位置变化 → 条件反向成立 → 反向滚动 → 无限循环。

3. **`lv_obj_scroll.c` 的 `lv_obj_scroll_by`（约 341 行）不检查 `LV_OBJ_FLAG_SCROLLABLE`**：
   ```c
   if(dx == 0 && dy == 0) return;
   if(anim_en) { ... }   // ← 没有 is_scrollable 检查！
   ```
   → 移除 SCROLLABLE 标志并不能阻止滚动（实验 D 失败的原因）。

### 阶段 6：核心补丁后 — 仍未完全解决

给 `lv_obj_scroll_by` 加滚动标志检查 → 滚动确实死了，但**文字仍在闪烁**（区域缩小到文本末尾约 1 字符宽 + 光标位置）→ 还有第三个因素。

### 阶段 7：对照实验 — 纯 label 稳定

把输入框临时换成普通 `lv_label` → **连续 5 帧 0 像素差异**。

→ 证明问题 100% 在 `lv_textarea` 内部，且与 comm 事件、SDL 窗口无关。

### 阶段 8：滚动条关闭 — 部分改善

`lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF)` → 闪烁区域进一步缩小。

### 阶段 9：光标透明化 — 彻底解决 ✅

`lv_obj_set_style_opa(ta, LV_OPA_TRANSP, LV_PART_CURSOR)` → **连续 5 帧 0 像素差异，完全稳定。**

---

## 三、根因总结（三层叠加）

```
┌─────────────────────────────────────────────────────────────┐
│ 1. 垂直滚动振荡（主运动）                                      │
│    SIZE/STYLE_CHANGED → scroll_to_cursor_pos(LV_ANIM_ON)     │
│    → 滚动条状态变化 → STYLE_CHANGED → 再次滚动 → 无限循环       │
│    修复：移除 SCROLLABLE 标志 + lv_obj_scroll.c 补丁          │
├─────────────────────────────────────────────────────────────┤
│ 2. 滚动条 fade 状态循环（放大器）                              │
│    滚动 → 滚动条出现 → STYLE_CHANGED → 再滚动                  │
│    修复：LV_SCROLLBAR_MODE_OFF                               │
├─────────────────────────────────────────────────────────────┤
│ 3. 光标区域重绘伪影（残留闪烁）                                │
│    SDL 渲染器对光标区域（文本末尾 1 字符）局部重绘出现字形伪影    │
│    修复：LV_PART_CURSOR 透明度 0                             │
└─────────────────────────────────────────────────────────────┘
```

三层缺一不可：补丁只解决第 1 层后，第 3 层的伪影依然可见（这正是“滑动变慢但还存在”的原因）。

---

## 四、最终修复清单

### 1. LVGL 核心补丁（`lvgl/src/core/lv_obj_scroll.c`）

```c
void lv_obj_scroll_by(lv_obj_t * obj, int32_t dx, int32_t dy, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    if(dx == 0 && dy == 0) return;
    if(!lv_obj_is_scrollable(obj)) return;   // ← 新增
    if(anim_en) {
```

语义正确：不可滚动的对象不应发生滚动。此补丁同时保护了应用内所有输入框。

### 2. 三个输入框统一防护（`ui_page_play.c` / `ui_page_upgrade.c` / `ui_page_debug.c`）

```c
lv_obj_remove_flag(ta, LV_OBJ_FLAG_SCROLLABLE);          // 关闭滚动能力
lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);    // 关闭滚动条
lv_obj_set_style_anim_duration(ta, 0, LV_PART_CURSOR);   // 停止光标闪烁
lv_obj_set_style_opa(ta, LV_OPA_TRANSP, LV_PART_CURSOR); // 光标透明（消除重绘伪影）
```

注意：光标透明只影响“可见光标”，**文字输入功能不受影响**（字符仍插入到光标位置）。

---

## 五、经验与教训

1. **现象驱动、一次只改一个变量**：每个实验都只改一处并重新截屏对比，避免混淆。
2. **日志要打到最底层**：从“看到跳动”到“看到 scroll_top 振荡”再到“看到事件链”，数据越来越接近真相。
3. **不要相信“应该有效”**：`LV_OBJ_FLAG_SCROLLABLE` 移除后以为解决了，实际 LVGL 该版本 `lv_obj_scroll_by` 不检查该标志 —— 必须验证而不是假设。
4. **对照实验能快速收敛**：纯 label 稳定的实验直接把问题锁定在 textarea 内部，排除了 comm/渲染器全局因素。
5. **小区域残留闪烁往往是最后一个未排除的绘制项**：闪烁区域从整行 → 文本尾部 → 1 字符宽，一步步收敛到光标。

---

## 六、后续发现：文字“看不见”（label 坐标与渲染失步）

在“跳动”修复后，用户反馈默认 RTSP 地址在输入框里**看不到**。

### 排查过程

1. **程序内地面真相日志**（不是截屏猜测）：
   ```
   ta=(250,28)-(749,89)          ← 文本框：屏幕坐标
   lbl=(313,91)-(686,106)        ← label：完全在文本框下方！(y91 > 文本框底 y89)
   text='rtsp://192.168.10.248:555/stream1' len=33 scroll=(0,0)
   ```
   → 文字确实存在（33 字符），但 label 被排到了文本框**正下方 2px**处，被裁剪 → 不可见。

2. **label 事件日志**：label 的**存储位置**一直是 (0,0)，但**屏幕坐标**偏离 (58,58)。

3. **机制确认**：`lv_obj_move_children_by()`（lv_obj_pos.c:951）直接平移 children 的 `coords`，**不更新存储位置、不发事件** —— 滚动/布局过程中 children 被平移后，坐标与存储位置失步。

4. **反复尝试**：`lv_obj_set_pos(label, 0, 0)`、三个滚动入口补丁（`scroll_by` / `scroll_by_bounded` / `scroll_by_raw` 全部加 `lv_obj_is_scrollable` 检查）—— label 坐标每次被拉回后**又在下一次绘制时被移走**（该 dev 版 LVGL 的坐标计算有失步缺陷）。

5. **最终方案 —— 绘制期“钉住”label 位置**：
   ```c
   static void pin_label_cb(lv_event_t * e)
   {
       lv_obj_t * lbl = lv_textarea_get_label(s_ip_ta);
       if(lbl && (lv_obj_get_x(lbl) != 0 || lv_obj_get_y(lbl) != 0)) {
           lv_obj_set_pos(lbl, 0, 0);
       }
   }
   lv_obj_add_event_cb(s_ip_ta, pin_label_cb, LV_EVENT_DRAW_MAIN_BEGIN, NULL);
   lv_timer_create(pin_label_timer_cb, 800, NULL);
   ```
   在文本框每次绘制前把 label 钉回 (0,0) → 文字在正确位置渲染（截屏确认完整 URL 可见、5 帧 0 差异）。

6. **结论**：该 dev 版 LVGL 中 textarea 的 label 坐标 API（`lv_obj_get_coords`）与渲染位置存在失步缺陷（坐标报错、渲染被 pin 修正后正确）。**遇到“控件位置对但内容渲染错位/不可见”时，优先怀疑坐标与渲染失步，用绘制钩子强制定位是有效的兜底手段。**

---

## 七、播放相关补充（另一次排查）

- 本地视频（birds.mp4）经 FFmpeg CLI 与应用内播放均验证正常 → 播放管线无问题。
- 默认地址 `rtsp://192.168.10.248:555/stream1` 是占位 IP，无设备必然 `open failed`。
- `lv_ffmpeg.c:135` 强制 `av_log_set_level(AV_LOG_QUIET)` 静音 FFmpeg 错误 → 应用层恢复 `AV_LOG_ERROR` + `av_log` 回调，失败原因显示到状态栏。
- `avformat_open_input` 增加 `stimeout/timeout=5s`，不可达地址快速失败，不再卡 UI。
- 本机 DNS/外网不通，公共测试流无法使用；播放必须连本地真实 RTSP 设备。
