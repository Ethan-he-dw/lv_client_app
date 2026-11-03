#include "ui_sidebar.h"
#include "ui_app.h"

static const char * s_btn_names[APP_PAGE_COUNT] = {"Play", "Upgrade", "Debug","multPlay"};

static void sidebar_btn_cb(lv_event_t * e)
{
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    app_show_page(idx);
}

void ui_sidebar_create(lv_obj_t * parent)
{
    lv_obj_set_style_pad_all(parent, 0, 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x2b2f36), 0);

    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text(title, "CLIENT APP");
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    //設置左側導航欄{"Play", "Upgrade", "Debug"};
    for(int i = 0; i < APP_PAGE_COUNT; i++) {
        lv_obj_t * btn = lv_btn_create(parent);
        lv_obj_set_size(btn, APP_SIDEBAR_W - 20, 54);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 70 + i * 70);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x3a3f47), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1f6feb), LV_STATE_PRESSED);
        lv_obj_add_event_cb(btn, sidebar_btn_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        lv_obj_t * lbl = lv_label_create(btn);
        lv_label_set_text(lbl, s_btn_names[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, 0);
        lv_obj_center(lbl);
        g_app.sidebar_btns[i] = btn;
    }
}