#ifndef UI_PAGE_PLAY_H
#define UI_PAGE_PLAY_H

#include "lvgl/lvgl.h"
#include "comm.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    LV_player_stop =0,
    LV_player_play,
    LV_player_pause,

} player_state;

typedef struct {
    int ch_count;
    int cols;
    int rows;
} ch_grid_t;

static const ch_grid_t s_grids[] = {
    { 4, 2, 2 },
    { 6, 3, 2 },
    { 8, 4, 2 },
    { 9, 3, 3 },
};

typedef struct {
    lv_obj_t *btn;
    lv_obj_t *addr;
    lv_obj_t *canvas;
    lv_obj_t *player;
} player_widgets_t;

typedef struct {
    char *url;                 // RTSP 地址
    int state;                 // 播放状态（0停止，1播放，2暂停）
    bool org_ratio_show;
    bool fullscreen;
    // ... 其他需要的属性
    lv_obj_t *btn;
    lv_obj_t *addr;
    lv_obj_t *canvas;
    lv_obj_t *player;
    lv_obj_t *opt;             // 底部控制栏
    /* 全屏前保存的原始位置和大小 */
    lv_obj_t *orig_parent;     // 全屏前的父对象（single_player）
    int32_t orig_x;
    int32_t orig_y;
    int32_t orig_w;
    int32_t orig_h;

} player_data_t;

void ui_multpage_play_create(lv_obj_t * parent);
void ui_multpage_play_on_event(const comm_event_t * evt);
void ui_multpage_play_set_visible(bool v);
void ui_multpage_play_on_show(void);
bool ui_multpage_play_on_hide(int target_page);
void ui_multpage_play_destroy(void);

#ifdef __cplusplus
}
#endif

#endif
