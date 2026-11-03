#ifndef UI_PAGE_PLAY_H
#define UI_PAGE_PLAY_H

#include "lvgl/lvgl.h"
#include "comm.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_page_play_create(lv_obj_t * parent);
void ui_page_play_on_event(const comm_event_t * evt);
void ui_page_play_set_visible(bool v);
void ui_page_play_on_show(void);
bool ui_page_play_on_hide(int target_page);
void ui_page_play_destroy(void);

#ifdef __cplusplus
}
#endif

#endif