#ifndef UI_FILE_DLG_H
#define UI_FILE_DLG_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_file_dlg_cb_t)(void * user_data, const char * full_path);

void ui_file_dlg_open(lv_obj_t * parent, ui_file_dlg_cb_t cb, void * user_data);

#ifdef __cplusplus
}
#endif

#endif