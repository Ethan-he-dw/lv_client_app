#ifndef UI_PAGE_DEBUG_H
#define UI_PAGE_DEBUG_H

#include "lvgl/lvgl.h"
#include "comm.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_page_debug_create(lv_obj_t * parent);
void ui_page_debug_on_event(const comm_event_t * evt);
void ui_page_debug_on_show(void);
bool ui_page_debug_on_hide(int target_page);
void ui_page_debug_destroy(void);

#ifdef __cplusplus
}
#endif

#endif