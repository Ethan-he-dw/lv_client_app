# Multplay 頁面切換卡死問題排查記錄

## 1. 現象描述

切換到 multplay 頁面時應用程式卡死（畫面凍結、無響應），必須強制結束程序。

**觸發路徑**：`app_show_page(APP_PAGE_MULTPLAY)` → `ui_page_multplay_on_show()` → `rebuild_players(0)` → 卡死

---

## 2. 排查思路

### 2.1 第一步：定位卡死發生點

通過在關鍵函數入口加入 `printf` 日誌，定位卡死發生在 `rebuild_players()` 函數內部，具體是在 `creat_single_palyer()` 調用 `lv_canvas_set_buffer()` 時。

```
rebuild_players
[creat_single_palyer] canvas_w=0, canvas_h=0   ← 異常尺寸
→ lv_canvas_set_buffer(canvas, cbuf, 0, 0, ...) → 卡死
```

### 2.2 第二步：追蹤異常尺寸的來源

`rebuild_players` 中計算每個格子像素尺寸的邏輯：

```c
int32_t area_w = lv_obj_get_content_width(s_multplayers);   // 返回 0
int32_t area_h = lv_obj_get_content_height(s_multplayers);   // 返回 0
int gap = 4;
int cell_w = (area_w - (cols + 1) * gap) / cols;  // (0 - 12) / 2 = -6
int cell_h = (area_h - (rows + 1) * gap) / rows;  // (0 - 12) / 2 = -6
```

**為什麼返回 0？**

查閱 `ui.c` 中的頁面切換流程：

```c
void app_show_page(int idx)
{
    // ① 隱藏舊頁面
    lv_obj_add_flag(g_app.pages[old], LV_OBJ_FLAG_HIDDEN);

    // ② 顯示新頁面
    lv_obj_remove_flag(g_app.pages[idx], LV_OBJ_FLAG_HIDDEN);

    // ③ 調用 on_show
    s_page_ops[idx].on_show();   // ← 此時 LVGL 尚未完成佈局重算
}
```

**關鍵問題**：`lv_obj_remove_flag` 僅標記頁面為可見，但 LVGL 的佈局計算（layout recalculation）是在當前事件循環結束後、下一幀渲染前才執行。因此在 `on_show()` 中調用 `lv_obj_get_content_width/height` 時，容器雖然已經標記為可見，但其幾何尺寸尚未被重新計算，仍然返回 0。

### 2.3 第三步：確認 LVGL canvas 行為

查閱 LVGL 源碼 `lv_canvas_set_buffer()`：

```c
// lvgl/src/widgets/canvas/lv_canvas.c
void lv_canvas_set_buffer(lv_obj_t * obj, void * buf, int32_t w, int32_t h, ...)
{
    // w 或 h 為 0 或負數時，會導致：
    // 1. 創建 0 像素的 image descriptor
    // 2. 後續渲染時觸發無限循環或斷言失敗
}
```

這確認了非法尺寸（0 或負數）傳入 `lv_canvas_set_buffer` 是導致卡死的直接原因。

---

## 3. 根因分析

| 層級 | 問題 | 影響 |
|------|------|------|
| **直接原因** | `lv_canvas_set_buffer` 接收 0 或負數的寬高 | LVGL 渲染引擎卡死（無限循環/斷言） |
| **根本原因** | `on_show` 中立即調用 `rebuild_players`，但此時 LVGL 佈局尚未計算完成 | `lv_obj_get_content_width/height` 返回 0 |
| **設計缺陷** | 缺少任何尺寸校驗和錯誤處理 | 異常值直接傳入 LVGL 底層 API |

---

## 4. 修復方案對比

### 方案 A：Timer 延遲重建（✅ 最終採用）

**原理**：在 `on_show` 中使用 `lv_timer` 將 `rebuild_players` 延遲到下一幀執行，此時 LVGL 已完成佈局計算。

```c
void ui_page_multplay_on_show(void)
{
    if(s_player_count == 0) {
        lv_timer_t * t = lv_timer_create_basic();
        lv_timer_set_cb(t, rebuild_players_timer_cb);
        lv_timer_set_user_data(t, (void *)(intptr_t)0);
        lv_timer_set_repeat_count(t, 1);
        lv_timer_set_period(t, 10);  // 10ms 後觸發
    }
}

static void rebuild_players_timer_cb(lv_timer_t * t)
{
    int ch_index = (int)(intptr_t)lv_timer_get_user_data(t);
    lv_timer_delete(t);  // 一次性 timer，執行後刪除
    rebuild_players(ch_index);
}
```

**優點**：
- 符合 LVGL 的設計理念（事件驅動、非阻塞）
- 保證佈局計算已完成，尺寸值正確
- 代碼清晰，意圖明確

**缺點**：
- 有輕微延遲（10ms，用戶不可感知）
- 額外的 timer 對象開銷（一次性，執行後自動銷毀）

---

### 方案 B：在 rebuild_players 內部加尺寸校驗（輔助措施）

**原理**：在 `rebuild_players` 入口檢查尺寸是否有效，無效時直接返回。

```c
static void rebuild_players(int ch_index)
{
    int32_t area_w = lv_obj_get_content_width(s_multplayers);
    int32_t area_h = lv_obj_get_content_height(s_multplayers);
    if(area_w <= 0 || area_h <= 0) {
        printf("[rebuild_players] skip: area not ready (%dx%d)\n",
               (int)area_w, (int)area_h);
        return;  // 直接跳過，不創建任何 player
    }
    // ... 後續邏輯
}
```

**優點**：
- 防禦性編程，即使調用時機錯誤也不會崩潰
- 實現簡單

**缺點**：
- **單獨使用時無法解決問題**：直接返回意味著 player 永遠不會被創建，頁面顯示為空白
- 需要配合方案 A 或方案 C 使用

---

### 方案 C：強制觸發佈局更新

**原理**：在調用 `rebuild_players` 前，強制 LVGL 立即計算佈局。

```c
void ui_page_multplay_on_show(void)
{
    if(s_player_count == 0) {
        // 強制更新佈局
        lv_obj_update_layout(s_multplayers);
        rebuild_players(0);
    }
}
```

**優點**：
- 同步執行，無延遲
- 確保尺寸值正確

**缺點**：
- `lv_obj_update_layout` 可能在某些場景下觸發遞歸佈局計算
- 在 `on_show` 回調中強制佈局更新，可能與 LVGL 內部狀態管理衝突
- 如果父容器的佈局依賴更多層級的計算，可能仍返回不正確的值

---

### 方案 D：在創建時就分配固定尺寸

**原理**：不依賴容器的 content size，改用螢幕解析度或固定常數計算格子尺寸。

```c
void ui_page_multplay_on_show(void)
{
    if(s_player_count == 0) {
        int32_t sw = lv_display_get_horizontal_resolution(lv_display_get_default());
        int32_t sh = lv_display_get_vertical_resolution(lv_display_get_default());
        // 用螢幕尺寸減去 sidebar 寬度和 padding
        int32_t area_w = sw - APP_SIDEBAR_W - 2 * APP_PAGE_PAD;
        int32_t area_h = (int32_t)(sh * 0.75);  // 75% 高度
        rebuild_players_fixed(0, area_w, area_h);
    }
}
```

**優點**：
- 完全不依賴 LVGL 的佈局計算時機
- 可預測性強

**缺點**：
- 與實際容器尺寸脫節（忽略了邊框、padding 等）
- 窗口大小改變後需要重新計算
- 硬編碼依賴多，維護成本高

---

### 方案對比總結

| 維度 | 方案 A: Timer 延遲 | 方案 B: 尺寸校驗 | 方案 C: 強制佈局 | 方案 D: 固定尺寸 |
|------|:---:|:---:|:---:|:---:|
| **能否解決卡死** | ✅ | ⚠️ 僅防禦 | ✅ | ✅ |
| **能否正確顯示** | ✅ | ❌ 單獨使用空白 | ✅ | ⚠️ 可能偏差 |
| **代碼侵入性** | 低 | 極低 | 中 | 高 |
| **與 LVGL 設計理念一致** | ✅ | ✅ | ⚠️ | ❌ |
| **窗口 resize 兼容** | ✅ | ✅ | ✅ | ❌ |
| **可靠性** | 高 | 高（配合使用） | 中 | 中 |

**最終選擇**：方案 A + 方案 B 組合使用。
- 方案 A 作為主修復：確保正確時機調用
- 方案 B 作為防禦性保護：防止未來其他路徑意外調用

---

## 5. 同時修復的其他問題

### 5.1 `player_btn_cb` 無法正確獲取 `player_data_t`

**現象**：`player_data_t *data = (player_data_t *)lv_obj_get_user_data(paraent)` 返回 NULL。

**原因**：LVGL 對象層級結構如下：
```
single_player          ← player_data_t 綁定在這裡
  ├── ffmpeg_player
  └── opt               ← btn 的父級（paraent）
        ├── label_addr
        ├── text_addr
        └── btn_play    ← 事件觸發的按鈕
```

代碼中 `paraent = lv_obj_get_parent(btn)` 得到的是 `opt`，但 `player_data_t` 綁定在 `single_player`（`opt` 的父級）上。

**修復**：多上溯一層
```c
lv_obj_t *opt = lv_obj_get_parent(btn);
lv_obj_t *single_player = lv_obj_get_parent(opt);
player_data_t *data = (player_data_t *)lv_obj_get_user_data(single_player);
```

### 5.2 `player_btn_cb` 中播放後立即刪除播放器

**現象**：點擊 play 按鈕後播放器被刪除。

**原因**：原代碼 else 分支邏輯錯誤：
```c
// 原代碼（錯誤）
else {
    data->state = LV_player_play;
    lv_ffmpeg_player_set_cmd(ffmpeg_player, LV_FFMPEG_PLAYER_CMD_START);
    lv_obj_delete(ffmpeg_player);   // ← 啟動後立刻刪除！
    ffmpeg_player = NULL;
}
```

**修復**：移除 `lv_obj_delete(ffmpeg_player)`。

### 5.3 `creat_single_palyer` 缺少尺寸校驗

**修復**：函數入口增加守衛：
```c
if(canvas_w <= 0 || canvas_h <= 0) {
    printf("[creat_single_palyer] skip: invalid size %dx%d\n", canvas_w, canvas_h);
    return NULL;
}
```

### 5.4 `rebuild_players` 缺少 NULL 返回值檢查

**修復**：
```c
lv_obj_t * p = creat_single_palyer(s_multplayers, cell_w, cell_h);
if(p == NULL) {
    printf("[rebuild_players] creat_single_palyer returned NULL for i=%d\n", i);
    continue;
}
```

---

## 6. 修改文件清單

| 文件 | 修改內容 |
|------|----------|
| `src/ui/ui_page_play_multplayer.c` | 1. `on_show` 改用 timer 延遲重建<br>2. `rebuild_players` 增加尺寸守衛<br>3. `creat_single_palyer` 增加尺寸守衛 + NULL 檢查<br>4. `player_btn_cb` 修正 user_data 獲取路徑<br>5. `player_btn_cb` 移除錯誤的 `lv_obj_delete` |

---

## 7. 經驗教訓

1. **LVGL 的佈局是延遲計算的**：`lv_obj_remove_flag(HIDDEN)` 不會立即更新幾何尺寸，必須等到下一幀渲染前。在顯示/隱藏操作後立即查詢尺寸是不可靠的。

2. **防禦性編程**：所有傳入 LVGL 底層 API 的數值都應做有效性檢查，尤其是尺寸、座標等幾何參數。

3. **LVGL 對象層級與 user_data**：`lv_obj_get_user_data` 只能獲取直接綁定在指定對象上的數據。如果數據綁定在祖先對象上，需要通過 `lv_obj_get_parent` 層層上溯。

4. **定時器回調中刪除自身**：LVGL timer 回調中可以安全調用 `lv_timer_delete(t)` 來刪除自身（LVGL 內部有處理），這是一次性 timer 的標準做法。
