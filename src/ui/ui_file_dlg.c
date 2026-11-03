#include "ui_file_dlg.h"
#include "ui_app.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    ui_file_dlg_cb_t cb;
    void * user_data;
    lv_obj_t * container;
} file_dlg_ctx_t;

static void fe_event_cb(lv_event_t * e)
{
    lv_obj_t * fe = lv_event_get_target_obj(e);
    file_dlg_ctx_t * ctx = (file_dlg_ctx_t *)lv_event_get_user_data(e);

    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        const char * path = lv_file_explorer_get_current_path(fe);
        const char * name = lv_file_explorer_get_selected_file_name(fe);
        if(name == NULL) return;
        char full[LV_FILE_EXPLORER_PATH_MAX_LEN + 64];
        snprintf(full, sizeof(full), "%s%s", path, name);
        if(ctx->cb) ctx->cb(ctx->user_data, full);
        lv_obj_delete(ctx->container);
        lv_free(ctx);
    }
}

static void close_btn_cb(lv_event_t * e)
{
    file_dlg_ctx_t * ctx = (file_dlg_ctx_t *)lv_event_get_user_data(e);
    lv_obj_delete(ctx->container);
    lv_free(ctx);
}

void ui_file_dlg_open(lv_obj_t * parent, ui_file_dlg_cb_t cb, void * user_data)
{
    file_dlg_ctx_t * ctx = lv_malloc(sizeof(file_dlg_ctx_t));
    ctx->cb = cb;
    ctx->user_data = user_data;

    ctx->container = lv_obj_create(parent);
    lv_obj_set_size(ctx->container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(ctx->container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(ctx->container, LV_OPA_50, 0);
    lv_obj_set_style_border_width(ctx->container, 0, 0);
    lv_obj_set_style_pad_all(ctx->container, 0, 0);
    lv_obj_center(ctx->container);

    lv_obj_t * box = lv_obj_create(ctx->container);
    lv_obj_set_size(box, 1000, 640);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2b2f36), 0);
    lv_obj_set_style_pad_all(box, 10, 0);
    lv_obj_center(box);

    lv_obj_t * close_btn = lv_btn_create(box);
    lv_obj_set_size(close_btn, 80, 32);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_add_event_cb(close_btn, close_btn_cb, LV_EVENT_CLICKED, ctx);
    lv_obj_t * close_lbl = lv_label_create(close_btn);
    lv_label_set_text(close_lbl, "Close");
    lv_obj_center(close_lbl);

    lv_obj_t * fe = lv_file_explorer_create(box);
    lv_obj_set_size(fe, 950, 560);
    lv_obj_align(fe, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_file_explorer_set_sort(fe, LV_EXPLORER_SORT_KIND);
    lv_file_explorer_open_dir(fe, "A:C:/");
    lv_file_explorer_set_quick_access_path(fe, LV_EXPLORER_FS_DIR, "A:C:/");
    lv_obj_add_event_cb(fe, fe_event_cb, LV_EVENT_VALUE_CHANGED, ctx);
}