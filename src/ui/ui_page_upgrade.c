#include "ui_page_upgrade.h"
#include "ui_app.h"
#include "ui_file_dlg.h"
#include <string.h>
#include <stdio.h>

static lv_obj_t * s_page;
static lv_obj_t * s_dev_dd;
static lv_obj_t * s_file_ta;
static lv_obj_t * s_status_lbl;
static lv_obj_t * s_upg_bar;
static lv_obj_t * s_upg_lbl;
static bool s_upgrading = false;

/* ---- 离开确认对话框 ---- */
static int s_pending_target = -1;  /* 用户想要切换到的目标页面 */

static void confirm_dlg_close_cb(lv_event_t * e)
{
    /* "Stay" 按钮：关闭对话框，留在当前页 */
    lv_obj_t * dlg = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_delete(dlg);
    s_pending_target = -1;
}

static void confirm_dlg_leave_cb(lv_event_t * e)
{
    /* "Leave" 按钮：关闭对话框，强制结束升级，执行页面切换 */
    lv_obj_t * dlg = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_delete(dlg);
    s_upgrading = false;
    if(s_pending_target >= 0) {
        int target = s_pending_target;
        s_pending_target = -1;
        app_show_page(target);  /* 此时 on_hide 会返回 true，切换正常进行 */
    }
}

static void show_leave_confirm_dialog(void)
{
    lv_obj_t * scr = lv_screen_active();

    /* 半透明遮罩层 */
    lv_obj_t * overlay = lv_obj_create(scr);
    lv_obj_remove_style_all(overlay);
    lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
    lv_obj_center(overlay);

    /* 对话框面板 */
    lv_obj_t * panel = lv_obj_create(overlay);
    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, 420, 200);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x2b2f36), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(panel, 12, 0);
    lv_obj_set_style_pad_all(panel, 20, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xff6600), 0);
    lv_obj_set_style_border_width(panel, 2, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* 警告图标文字 */
    lv_obj_t * warn_lbl = lv_label_create(panel);
    lv_label_set_text(warn_lbl, LV_SYMBOL_WARNING "  Upgrade in Progress!");
    lv_obj_set_style_text_color(warn_lbl, lv_color_hex(0xff6600), 0);
    lv_obj_set_style_text_font(warn_lbl, &lv_font_montserrat_20, 0);

    /* 说明文字 */
    lv_obj_t * desc_lbl = lv_label_create(panel);
    lv_label_set_text(desc_lbl, "Firmware upgrade is still running.\nLeaving now may brick the device.");
    lv_obj_set_style_text_color(desc_lbl, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(desc_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_pad_top(desc_lbl, 10, 0);

    /* 按钮容器 */
    lv_obj_t * btn_row = lv_obj_create(panel);
    lv_obj_remove_style_all(btn_row);
    lv_obj_set_size(btn_row, lv_pct(100), 50);
    lv_obj_set_style_pad_top(btn_row, 15, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* "Stay" 按钮 — 留在当前页 */
    lv_obj_t * stay_btn = lv_btn_create(btn_row);
    lv_obj_set_size(stay_btn, 140, 40);
    lv_obj_set_style_bg_color(stay_btn, lv_color_hex(0x3a3f47), 0);
    lv_obj_add_event_cb(stay_btn, confirm_dlg_close_cb, LV_EVENT_CLICKED, overlay);
    lv_obj_t * stay_lbl = lv_label_create(stay_btn);
    lv_label_set_text(stay_lbl, "Stay");
    lv_obj_set_style_text_color(stay_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_center(stay_lbl);

    /* "Leave" 按钮 — 确认离开 */
    lv_obj_t * leave_btn = lv_btn_create(btn_row);
    lv_obj_set_size(leave_btn, 140, 40);
    lv_obj_set_style_bg_color(leave_btn, lv_color_hex(0xcc3300), 0);
    lv_obj_add_event_cb(leave_btn, confirm_dlg_leave_cb, LV_EVENT_CLICKED, overlay);
    lv_obj_t * leave_lbl = lv_label_create(leave_btn);
    lv_label_set_text(leave_lbl, "Leave");
    lv_obj_set_style_text_color(leave_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_center(leave_lbl);
}

/* ---- 原有回调 ---- */

static void search_btn_cb(lv_event_t * e)
{
    (void)e;
    app_refresh_device_dropdown(s_dev_dd);
    lv_bar_set_value(s_upg_bar, 0, LV_ANIM_OFF);
    comm_search_devices();
    lv_label_set_text(s_status_lbl, "searching...");
}

static void file_sel_cb(void * user_data, const char * full_path)
{
    (void)user_data;
    lv_textarea_set_text(s_file_ta, full_path);
}

static void file_btn_cb(lv_event_t * e)
{
    (void)e;
    ui_file_dlg_open(lv_screen_active(), file_sel_cb, NULL);
}

static void upg_btn_cb(lv_event_t * e)
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
    const char * path = lv_textarea_get_text(s_file_ta);
    if(path == NULL || path[0] == '\0') {
        lv_label_set_text(s_status_lbl, "no file selected");
        return;
    }
    lv_bar_set_value(s_upg_bar, 0, LV_ANIM_OFF);
    lv_label_set_text_fmt(s_upg_lbl, "%d%%", 0);
    if(comm_start_upgrade(dev.ip, path) != 0) {
        lv_label_set_text(s_status_lbl, "upgrade busy");
        return;
    }
    s_upgrading = true;
    lv_label_set_text_fmt(s_status_lbl, "upgrading %s...", dev.ip);
}

void ui_page_upgrade_create(lv_obj_t * parent)
{
    s_page = parent;
    s_upgrading = false;

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
    lv_obj_set_size(s_dev_dd, 420, 36);

    lv_obj_t * file_btn = lv_btn_create(parent);
    lv_obj_set_pos(file_btn, 0, 56);
    lv_obj_set_size(file_btn, 110, 36);
    lv_obj_add_event_cb(file_btn, file_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * file_lbl = lv_label_create(file_btn);
    lv_label_set_text(file_lbl, "File");
    lv_obj_center(file_lbl);

    s_file_ta = lv_textarea_create(parent);
    lv_textarea_set_one_line(s_file_ta, true);
    lv_textarea_set_placeholder_text(s_file_ta, "path of upgrade archive");
    lv_obj_remove_flag(s_file_ta, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_file_ta, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_anim_duration(s_file_ta, 0, LV_PART_CURSOR);
    lv_obj_set_style_opa(s_file_ta, LV_OPA_TRANSP, LV_PART_CURSOR);
    lv_obj_set_pos(s_file_ta, 140, 56);
    lv_obj_set_size(s_file_ta, 520, 36);

    lv_obj_t * upg_btn = lv_btn_create(parent);
    lv_obj_set_pos(upg_btn, 0, 104);
    lv_obj_set_size(upg_btn, 110, 36);
    lv_obj_add_event_cb(upg_btn, upg_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * upg_lbl = lv_label_create(upg_btn);
    lv_label_set_text(upg_lbl, "Upgrade");
    lv_obj_center(upg_lbl);

    s_upg_bar = lv_bar_create(parent);
    lv_obj_set_pos(s_upg_bar, 140, 104);
    lv_obj_set_size(s_upg_bar, 400, 24);
    lv_bar_set_range(s_upg_bar, 0, 100);
    lv_bar_set_value(s_upg_bar, 0, LV_ANIM_OFF);

    s_upg_lbl = lv_label_create(parent);
    lv_label_set_text(s_upg_lbl, "0%");
    lv_obj_set_style_text_color(s_upg_lbl, lv_color_hex(0xcccccc), 0);
    lv_obj_set_pos(s_upg_lbl, 560, 108);

    s_status_lbl = lv_label_create(parent);
    lv_label_set_text(s_status_lbl, "ready");
    lv_obj_set_style_text_color(s_status_lbl, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(s_status_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(s_status_lbl, 0, 160);
}

void ui_page_upgrade_on_event(const comm_event_t * evt)
{
    if(evt->type == COMM_EVT_DEVICE_FOUND) {
        app_refresh_device_dropdown(s_dev_dd);
    }
    else if(evt->type == COMM_EVT_SEARCH_DONE) {
        int n = app_refresh_device_dropdown(s_dev_dd);
        lv_label_set_text_fmt(s_status_lbl, "search done, %d device(s)", n);
    }
    else if(evt->type == COMM_EVT_UPGRADE_PROGRESS) {
        lv_bar_set_value(s_upg_bar, evt->progress, LV_ANIM_OFF);
        lv_label_set_text_fmt(s_upg_lbl, "%d%%", evt->progress);
        if(evt->progress >= 100) {
            lv_label_set_text(s_status_lbl, "upgrade done");
            s_upgrading = false;
        }
    }
}

/* ===================== 生命周期回调 ===================== */

void ui_page_upgrade_on_show(void)
{
    /* 切回升级页时刷新设备列表 */
    app_refresh_device_dropdown(s_dev_dd);
    ui_page_set_sidebar_btns_hightlight(APP_PAGE_UPGRADE);
}

bool ui_page_upgrade_on_hide(int target_page)
{
    /* ★ 升级进行中 → 弹出确认对话框，阻止离开 */
    if(s_upgrading) {
        s_pending_target = target_page;  /* 记录目标页面，供对话框回调使用 */
        show_leave_confirm_dialog();
        return false;  /* 阻止本次切换 */
    }
    return true;  /* 非升级状态，允许离开 */
}

void ui_page_upgrade_destroy(void)
{
    s_upgrading = false;
    s_page = NULL;
}