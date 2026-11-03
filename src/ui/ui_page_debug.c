#include "ui_page_debug.h"
#include "ui_app.h"
#include <string.h>
#include <stdio.h>

static lv_obj_t * s_dev_dd;
static lv_obj_t * s_cmd_ta;
static lv_obj_t * s_log_lbl;
static lv_obj_t * s_log_cont;
static lv_obj_t * s_status_lbl;
static char g_log_buf[16384];

static void log_append(const char * line)
{
    size_t len = strlen(g_log_buf);
    if(len > 12000) {
        memmove(g_log_buf, g_log_buf + len - 6000, 6000 + 1);
        len = 6000;
    }
    snprintf(g_log_buf + len, sizeof(g_log_buf) - len, "%s\n", line);
    lv_label_set_text(s_log_lbl, g_log_buf);
    lv_obj_scroll_to_y(s_log_cont, LV_COORD_MAX, LV_ANIM_OFF);
}

static void search_btn_cb(lv_event_t * e)
{
    (void)e;
    app_refresh_device_dropdown(s_dev_dd);
    comm_search_devices();
    lv_label_set_text(s_status_lbl, "searching...");
}

static void send_btn_cb(lv_event_t * e)
{
    (void)e;
    int idx = (int)lv_dropdown_get_selected(s_dev_dd);
    if(idx < 0) {
        lv_label_set_text(s_status_lbl, "no device selected");
        return;
    }
    comm_device_t dev;
    if(!comm_get_device(idx, &dev)) {
        lv_label_set_text(s_status_lbl, "no device selected");
        return;
    }
    const char * cmd = lv_textarea_get_text(s_cmd_ta);
    if(cmd == NULL || cmd[0] == '\0') {
        lv_label_set_text(s_status_lbl, "empty command");
        return;
    }
    comm_send_debug_cmd(dev.ip, cmd);
    lv_textarea_set_text(s_cmd_ta, "");
}

void ui_page_debug_create(lv_obj_t * parent)
{
    lv_obj_t * search_btn = lv_btn_create(parent);
    lv_obj_set_pos(search_btn, 0, 8);
    lv_obj_set_size(search_btn, 110, 36);
    lv_obj_add_event_cb(search_btn, search_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * search_lbl = lv_label_create(search_btn);
    lv_label_set_text(search_lbl, "Search");
    lv_obj_center(search_lbl);

    s_dev_dd = lv_dropdown_create(parent);
    lv_dropdown_set_options(s_dev_dd, "No device");
    lv_obj_set_pos(s_dev_dd, 140, 8);
    lv_obj_set_size(s_dev_dd, 380, 36);

    lv_obj_t * send_btn = lv_btn_create(parent);
    lv_obj_set_pos(send_btn, 540, 8);
    lv_obj_set_size(send_btn, 90, 36);
    lv_obj_add_event_cb(send_btn, send_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * send_lbl = lv_label_create(send_btn);
    lv_label_set_text(send_lbl, "Send");
    lv_obj_center(send_lbl);

    s_cmd_ta = lv_textarea_create(parent);
    lv_textarea_set_one_line(s_cmd_ta, true);
    lv_textarea_set_placeholder_text(s_cmd_ta, "command");
    lv_obj_remove_flag(s_cmd_ta, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_cmd_ta, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_anim_duration(s_cmd_ta, 0, LV_PART_CURSOR);
    lv_obj_set_style_opa(s_cmd_ta, LV_OPA_TRANSP, LV_PART_CURSOR);
    lv_obj_set_pos(s_cmd_ta, 0, 56);
    lv_obj_set_size(s_cmd_ta, 520, 36);

    s_log_cont = lv_obj_create(parent);
    lv_obj_set_pos(s_log_cont, 0, 104);
    lv_obj_set_size(s_log_cont, lv_pct(100), 746);
    lv_obj_set_style_bg_color(s_log_cont, lv_color_hex(0x181c20), 0);
    lv_obj_set_style_border_width(s_log_cont, 0, 0);
    lv_obj_set_style_pad_all(s_log_cont, 8, 0);

    s_log_lbl = lv_label_create(s_log_cont);
    lv_label_set_text(s_log_lbl, "");
    lv_label_set_long_mode(s_log_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_log_lbl, lv_pct(100));
    lv_obj_set_style_text_color(s_log_lbl, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(s_log_lbl, &lv_font_montserrat_14, 0);

    s_status_lbl = lv_label_create(parent);
    lv_label_set_text(s_status_lbl, "ready");
    lv_obj_set_style_text_color(s_status_lbl, lv_color_hex(0xcccccc), 0);
    lv_obj_set_pos(s_status_lbl, 540, 60);
}

void ui_page_debug_on_event(const comm_event_t * evt)
{
    if(evt->type == COMM_EVT_DEVICE_FOUND) {
        app_refresh_device_dropdown(s_dev_dd);
    }
    else if(evt->type == COMM_EVT_SEARCH_DONE) {
        int n = app_refresh_device_dropdown(s_dev_dd);
        lv_label_set_text_fmt(s_status_lbl, "search done, %d device(s)", n);
    }
    else if(evt->type == COMM_EVT_LOG) {
        if(evt->data_len > 0) log_append(evt->data);
    }
}

/* ===================== 生命周期回调 ===================== */

void ui_page_debug_on_show(void)
{
    /* 切回调试页时刷新设备列表 */
    app_refresh_device_dropdown(s_dev_dd);
    ui_page_set_sidebar_btns_hightlight(APP_PAGE_DEBUG);
}

bool ui_page_debug_on_hide(int target_page)
{
    (void)target_page;
    /* Debug 页无特殊资源需要暂停，始终允许离开 */
    return true;
}

void ui_page_debug_destroy(void)
{
    /* 清空日志缓冲区 */
    g_log_buf[0] = '\0';
}
