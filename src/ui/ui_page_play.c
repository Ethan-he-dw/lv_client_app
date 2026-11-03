#include "ui_page_play.h"
#include "ui_app.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <libavutil/log.h>
#ifdef _WIN32
#include <windows.h>
#endif

static lv_obj_t * s_page;
static lv_obj_t * s_ip_ta;
static lv_obj_t * s_status_lbl;
static lv_obj_t * s_player;
static lv_obj_t * s_overlay;
static lv_obj_t * s_data_lbl;
static lv_obj_t * s_data_cont;
static char g_backend_buf[8192];
static char g_av_err[256];
static bool s_fullscreen = false;

#define DATA_PANEL_W 400
#define PLAYER_TOP   90
#define PLAYER_H     720

static int32_t video_width(void)
{
    int32_t cw = lv_obj_get_content_width(s_page);
    if(s_overlay && !lv_obj_has_flag(s_overlay, LV_OBJ_FLAG_HIDDEN)) {
        cw -= DATA_PANEL_W + 10;
    }
    return cw;
}

static void apply_player_layout(void)
{
    if(s_player == NULL) return;
    if(s_fullscreen) {
        int32_t sw = lv_display_get_horizontal_resolution(lv_display_get_default());
        int32_t sh = lv_display_get_vertical_resolution(lv_display_get_default());
        lv_obj_set_pos(s_player, 0, 0);
        lv_obj_set_size(s_player, sw, sh);
        lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_area_t ca;
        lv_obj_get_content_coords(s_page, &ca);
        lv_obj_set_pos(s_player, ca.x1, ca.y1 + PLAYER_TOP);
        lv_obj_set_size(s_player, video_width(), PLAYER_H);
        lv_obj_remove_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

static void av_log_cb(void * avcl, int level, const char * fmt, va_list vl)
{
    (void)avcl;
    if(level <= AV_LOG_ERROR) {
        vsnprintf(g_av_err, sizeof(g_av_err), fmt, vl);
        size_t n = strlen(g_av_err);
        while(n > 0 && (g_av_err[n-1] == '\n' || g_av_err[n-1] == '\r')) g_av_err[--n] = '\0';
    }
}

static void player_dbl_cb(lv_event_t * e)
{
    (void)e;
    s_fullscreen = !s_fullscreen;
    if(s_fullscreen) {
        lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(s_status_lbl, "fullscreen, dbl-click to restore");
    }
    else {
        lv_obj_remove_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(s_status_lbl, "playing");
    }
    apply_player_layout();
}

static void backend_append(const char * line)
{
    size_t len = strlen(g_backend_buf);
    if(len > 6000) {
        memmove(g_backend_buf, g_backend_buf + len - 3000, 3000 + 1);
        len = 3000;
    }
    snprintf(g_backend_buf + len, sizeof(g_backend_buf) - len, "%s\n", line);
    lv_label_set_text(s_data_lbl, g_backend_buf);
    lv_obj_scroll_to_y(s_data_cont, LV_COORD_MAX, LV_ANIM_OFF);
}

static void do_play(const char * url)
{
    if(url == NULL || url[0] == '\0') {
        lv_label_set_text(s_status_lbl, "empty url");
        return;
    }
    g_av_err[0] = '\0';
    av_log_set_level(AV_LOG_ERROR);
    av_log_set_callback(av_log_cb);
    if(s_player) {
        lv_obj_delete(s_player);
        s_player = NULL;
    }
    
    s_player = lv_ffmpeg_player_create(lv_screen_active());
    lv_image_set_inner_align(s_player, LV_IMAGE_ALIGN_CONTAIN);
    lv_obj_add_flag(s_player, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(s_player, player_dbl_cb, LV_EVENT_DOUBLE_CLICKED, NULL);
    apply_player_layout();

    if(lv_ffmpeg_player_set_src(s_player, url) != LV_RESULT_OK) {
        lv_obj_delete(s_player);
        s_player = NULL;
        if(g_av_err[0] != '\0') lv_label_set_text(s_status_lbl, g_av_err);
        else lv_label_set_text(s_status_lbl, "open failed");
        return;
    }
    lv_ffmpeg_player_set_auto_restart(s_player, false);
    lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_START);
    lv_label_set_text(s_status_lbl, "playing");
}

static void play_btn_cb(lv_event_t * e)
{
    (void)e;
    do_play(lv_textarea_get_text(s_ip_ta));
}

static void pin_label_cb(lv_event_t * e)
{
    (void)e;
    lv_obj_t * lbl = lv_textarea_get_label(s_ip_ta);
    if(lbl && (lv_obj_get_x(lbl) != 0 || lv_obj_get_y(lbl) != 0)) {
       // lv_obj_set_pos(lbl, 0, 0);
    }
}

static void pin_label_timer_cb(lv_timer_t * t)
{
    lv_timer_delete(t);
    pin_label_cb(NULL);
}

static void stop_btn_cb(lv_event_t * e)
{
    (void)e;
    if(s_player) {
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_STOP);
        lv_obj_delete(s_player);
        s_player = NULL;
        lv_label_set_text(s_status_lbl, "stopped");
    }
}

static void data_btn_cb(lv_event_t * e)
{
    (void)e;
    if(s_fullscreen) return;
    if(lv_obj_has_flag(s_overlay, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    apply_player_layout();
}

void ui_page_play_create(lv_obj_t * parent)
{
    s_page = parent;

    lv_obj_t * ip_lbl = lv_label_create(parent);
    lv_label_set_text(ip_lbl, "RTSP:");
    lv_obj_set_style_text_color(ip_lbl, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(ip_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(ip_lbl, 0, 12); 
    
    /* 创建文本区域（输入框）对象，父容器为 parent */
    s_ip_ta = lv_textarea_create(parent);
        /* 将整个文本区域控件定位在父容器的 (70, 8) 位置 */
        lv_obj_set_pos(s_ip_ta, 70, 8);

        /* 设置文本区域的尺寸：宽500，高62（像素） */
        lv_obj_set_size(s_ip_ta, 500, 62);
        /* 设置占位符文本（当内容为空时显示灰色提示）"rtsp://ip:port/stream1" */
        lv_textarea_set_placeholder_text(s_ip_ta, "rtsp://ip:port/stream1");

        /* 设置默认文本（实际显示的内容，用于快速测试） */
        lv_textarea_set_text(s_ip_ta, "rtsp://192.168.10.248:555/stream1");

        /* 禁止点击输入框时移动光标（固定光标位置） */
        //lv_textarea_set_cursor_click_pos(s_ip_ta, false);

        /* （已注释）原意图：禁止控件获得焦点时自动滚动到可视区域，保持默认行为 */
        //lv_obj_set_scroll_on_focus(s_ip_ta, false);

        /* 移除滚动标志，彻底禁止内部滚动（超出部分直接裁剪） */
        lv_obj_remove_flag(s_ip_ta, LV_OBJ_FLAG_SCROLLABLE);

        /* （已注释）原意图：强制关闭滚动条，已被上一句替代 */
        //lv_obj_set_scrollbar_mode(s_ip_ta, LV_SCROLLBAR_MODE_OFF);

        /* 将光标部件的动画时长设为 0，消除光标移动/闪烁动画 */
        //lv_obj_set_style_anim_duration(s_ip_ta, 0, LV_PART_CURSOR);

        /* 将光标透明度设为完全透明，视觉上隐藏光标（配合上句实现彻底不可见） */
        //lv_obj_set_style_opa(s_ip_ta, LV_OPA_TRANSP, LV_PART_CURSOR);

        /* 获取文本区域内部的标签（用于显示文本的子对象），并相对于文本区域内部偏移 (0, 10)
           这相当于自定义垂直内边距，让文本内容下移10像素（不推荐，可用样式替代） */
        //lv_obj_set_pos(lv_textarea_get_label(s_ip_ta), 0, 10);

        /* 清空所有内边距（上下左右均为0），但标签已偏移，实际效果由标签位置决定 */
        lv_obj_set_style_pad_all(s_ip_ta, 10, 0);

        /* 设置输入框内文本水平居中显示 */
        lv_obj_set_style_text_align(s_ip_ta, LV_TEXT_ALIGN_CENTER, 0);
    
    lv_obj_t * play_btn = lv_btn_create(parent);
        lv_obj_set_pos(play_btn, 585, 8);
        lv_obj_set_size(play_btn, 90, 36);
        lv_obj_add_event_cb(play_btn, play_btn_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_t * play_lbl = lv_label_create(play_btn);
        lv_label_set_text(play_lbl, "Play");
        lv_obj_center(play_lbl);

    lv_obj_t * stop_btn = lv_btn_create(parent);
        lv_obj_set_pos(stop_btn, 685, 8);
        lv_obj_set_size(stop_btn, 90, 36);
        lv_obj_add_event_cb(stop_btn, stop_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * stop_lbl = lv_label_create(stop_btn);
        lv_label_set_text(stop_lbl, "Stop");
        lv_obj_center(stop_lbl);

    lv_obj_t * data_btn = lv_btn_create(parent);
        lv_obj_set_pos(data_btn, 785, 8);
        lv_obj_set_size(data_btn, 110, 36);
        lv_obj_add_event_cb(data_btn, data_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * data_lbl = lv_label_create(data_btn);
        lv_label_set_text(data_lbl, "Data");
        lv_obj_center(data_lbl);

    s_status_lbl = lv_label_create(parent);
        lv_label_set_text(s_status_lbl, "idle");
        lv_obj_set_style_text_color(s_status_lbl, lv_color_hex(0xcccccc), 0);
        lv_obj_set_style_text_font(s_status_lbl, &lv_font_montserrat_16, 0);
        lv_obj_set_pos(s_status_lbl, 910, 12);

    /* Pin the label position via a timer only — using LV_EVENT_DRAW_MAIN_BEGIN
     * is forbidden because lv_obj_set_pos() triggers invalidation during
     * rendering, which asserts and causes a black-screen hang. */
    //lv_timer_create(pin_label_timer_cb, 800, NULL);

    s_overlay = lv_obj_create(parent);
        lv_obj_set_size(s_overlay, DATA_PANEL_W, PLAYER_H);
        lv_obj_align(s_overlay, LV_ALIGN_TOP_RIGHT, 0, PLAYER_TOP);
        lv_obj_set_style_bg_color(s_overlay, lv_color_hex(0x101014), 0);
        lv_obj_set_style_bg_opa(s_overlay, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(s_overlay, lv_color_hex(0x3a3f47), 0);
        lv_obj_set_style_pad_all(s_overlay, 10, 0);

    lv_obj_t * ov_title = lv_label_create(s_overlay);
        lv_label_set_text(ov_title, "BACKEND DATA");
        lv_obj_set_style_text_color(ov_title, lv_color_hex(0x4da3ff), 0);
        lv_obj_set_style_text_font(ov_title, &lv_font_montserrat_16, 0);
    //BACKEND DATA overlay
    s_data_cont = lv_obj_create(s_overlay);
        lv_obj_set_pos(s_data_cont, 10, 34);
        lv_obj_set_size(s_data_cont, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_color(s_data_cont, lv_color_hex(0x181c20), 0);//lv_palette_main(LV_PALETTE_PINK)   lv_color_hex(0x181c20)
        lv_obj_set_style_border_width(s_data_cont, 0, 0);
        lv_obj_set_style_pad_all(s_data_cont, 8, 0);

    s_data_lbl = lv_label_create(s_data_cont);
    lv_label_set_text(s_data_lbl, "");
    lv_label_set_long_mode(s_data_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_data_lbl, lv_pct(100));
    lv_obj_set_style_text_color(s_data_lbl, lv_color_hex(0xcccccc), 0);
}

void ui_page_play_on_event(const comm_event_t * evt)
{
    if(evt->type == COMM_EVT_BACKEND_DATA) {
        if(evt->data_len > 0) backend_append(evt->data);
    }
}

void ui_page_play_set_visible(bool v)
{
    if(s_player == NULL) return;
    if(v) lv_obj_remove_flag(s_player, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(s_player, LV_OBJ_FLAG_HIDDEN);
}

/* ===================== 生命周期回调 ===================== */

void ui_page_play_on_show(void)
{
    /* 恢复播放器显示并继续播放（如果之前在播放） */
    if(s_player != NULL) {
        lv_obj_remove_flag(s_player, LV_OBJ_FLAG_HIDDEN);
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_START);
    }
    ui_page_set_sidebar_btns_hightlight(APP_PAGE_PLAY);

}

bool ui_page_play_on_hide(int target_page)
{
    (void)target_page;
    /* ★ 暂停播放器，释放 CPU/GPU 资源 */
    if(s_player != NULL) {
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_STOP);
        lv_obj_add_flag(s_player, LV_OBJ_FLAG_HIDDEN);
    }
    /* 退出全屏状态 */
    if(s_fullscreen) {
        s_fullscreen = false;
    }
    return true;  /* Play 页始终允许离开 */
}

void ui_page_play_destroy(void)
{
    /* 停止并删除播放器，释放所有资源 */
    if(s_player != NULL) {
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_STOP);
        lv_obj_delete(s_player);
        s_player = NULL;
    }
    /* 清空数据缓冲区 */
    g_backend_buf[0] = '\0';
    g_av_err[0] = '\0';
    s_page = NULL;
    s_fullscreen = false;
}
