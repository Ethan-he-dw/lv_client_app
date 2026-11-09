#include "ui_page_play.h"
#include "ui_app.h"
#include "ui_ivs_panel.h"
#include "ui_ivs_overlay.h"

#include "ivs_source.h"
#include "ivs_store.h"

#include <string.h>
#include <stdlib.h>
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
static lv_obj_t * s_osd_btn_lbl;

static lv_timer_t * s_tmr_panel;
static lv_timer_t * s_tmr_osd;

static char g_av_err[256];
static char s_board_ip[64];            /* 当前会话的板端 IP（重试用） */
static bool s_fullscreen = false;
/* 面板显隐拆成两个独立概念，避免互相覆盖：
 *   s_panel_user_visible —— 用户想不想看面板（只由 Data 按钮改）
 *   全屏时临时隐藏面板，但不修改用户选择；退出全屏按用户选择恢复。 */
static bool s_panel_user_visible = true;

/* ---------------- 工具 ---------------- */

/** rtsp://192.168.10.168:554/live0 → "192.168.10.168" */
static bool parse_rtsp_host(const char * url, char * out, size_t out_sz)
{
    if(url == NULL || out == NULL || out_sz == 0) return false;

    const char * p = strstr(url, "://");
    p = (p != NULL) ? (p + 3) : url;

    const char * end = p;
    while(*end != '\0' && *end != ':' && *end != '/' && *end != '?') end++;

    const size_t n = (size_t)(end - p);
    if(n == 0 || n >= out_sz) return false;

    memcpy(out, p, n);
    out[n] = '\0';
    return true;
}

static int32_t video_width(void)
{
    int32_t cw = lv_obj_get_content_width(s_page);
    if(ui_ivs_panel_is_visible()) {
        cw -= APP_PANEL_W + 10;
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
        ui_ivs_panel_set_visible(false);            /* 临时隐藏，不改用户选择 */
    }
    else {
        lv_area_t ca;
        lv_obj_get_content_coords(s_page, &ca);
        /* 必须先恢复面板再算宽度：否则 video_width() 按"面板隐藏"算，
         * 播放器会一直保持全宽，面板虽然显示但被压在下面（退出全屏时的旧问题） */
        ui_ivs_panel_set_visible(s_panel_user_visible);
        lv_obj_set_pos(s_player, ca.x1, ca.y1 + APP_PLAYER_TOP);
        lv_obj_set_size(s_player, video_width(), APP_PLAYER_H);
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

/* ---------------- 播放控制 ---------------- */

static void player_dbl_cb(lv_event_t * e)
{
    (void)e;
    s_fullscreen = !s_fullscreen;
    apply_player_layout();              /* 内部处理面板隐藏/恢复，不要在这里再 toggle */
    LV_LOG("dblclick: fullscreen=%d", (int)s_fullscreen);

    lv_label_set_text(s_status_lbl, s_fullscreen ? "fullscreen, dbl-click to restore" : "playing");
}

static void stop_session(void)
{
    ivs_close();
    ui_ivs_overlay_clear();
    ui_ivs_overlay_set_player(NULL);
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

    stop_session();                     /* 换流：先关旧会话，避免残留框 */
    if(s_player) {
        lv_obj_delete(s_player);
        s_player = NULL;
    }

    s_player = lv_ffmpeg_player_create(lv_screen_active());
    lv_image_set_inner_align(s_player, LV_IMAGE_ALIGN_CONTAIN);
    lv_obj_set_clickable(s_player, true);
    lv_obj_add_event_cb(s_player, player_dbl_cb, LV_EVENT_DOUBLE_CLICKED, NULL);
    apply_player_layout();

    if(lv_ffmpeg_player_set_src(s_player, url) != LV_RESULT_OK) {
        lv_obj_delete(s_player);
        s_player = NULL;
        lv_label_set_text(s_status_lbl, (g_av_err[0] != '\0') ? g_av_err : "open failed");
        return;
    }
    lv_ffmpeg_player_set_auto_restart(s_player, false);
    lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_START);
    lv_label_set_text(s_status_lbl, "playing");

    /* 绑定几何基准（播放器已在位） */
    ui_ivs_overlay_set_player(s_player);

    /* 只有 RTSP 流才去连识别服务：本地文件/图片不启动会话（避免拿路径当 host） */
    if(strncmp(url, "rtsp://", 7) == 0) {
        if(parse_rtsp_host(url, s_board_ip, sizeof(s_board_ip))) {
            if(ivs_open(s_board_ip, APP_IVS_PORT, "face") != 0) {
                lv_label_set_text(s_status_lbl, "ivs open failed");
            }
        }
        else {
            s_board_ip[0] = '\0';
            lv_label_set_text(s_status_lbl, "bad rtsp url (no host)");
        }
    }
    else {
        s_board_ip[0] = '\0';
    }
}

/** 面板「重试」按钮：同 host 重开会话 */
static void retry_cb(void)
{
    if(s_board_ip[0] == '\0') return;
    ivs_close();
    ivs_open(s_board_ip, APP_IVS_PORT, "face");
}

static void play_btn_cb(lv_event_t * e)
{
    (void)e;
    do_play(lv_textarea_get_text(s_ip_ta));
}

static void stop_btn_cb(lv_event_t * e)
{
    (void)e;
    if(s_player) {
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_STOP);
        lv_obj_delete(s_player);
        s_player = NULL;
        stop_session();
        lv_label_set_text(s_status_lbl, "stopped");
    }
}

static void data_btn_cb(lv_event_t * e)
{
    (void)e;
    if(s_fullscreen) return;            /* 全屏中不改用户选择，退出后自然恢复 */
    s_panel_user_visible = !s_panel_user_visible;
    apply_player_layout();              /* 面板显隐改变播放器可用宽度 */
}

static void osd_btn_cb(lv_event_t * e)
{
    (void)e;
    /* 注意：全屏时也要能切，否则全屏看不了框 */
    const bool v = !ui_ivs_overlay_is_visible();
    ui_ivs_overlay_set_visible(v);
    if(s_osd_btn_lbl) lv_label_set_text(s_osd_btn_lbl, v ? "OSD on" : "OSD off");
}

/* ---------------- 定时器 ---------------- */

static void panel_timer_cb(lv_timer_t * t)
{
    (void)t;
    ui_ivs_panel_refresh();             /* 5Hz：文本节流 */
}

/* 开发用：APP_AUTOPLAY=<路径或 URL> 时自动开播（便于无人工交互地验收/截图） */
static void autoplay_timer_cb(lv_timer_t * t)
{
    lv_timer_delete(t);
    const char * p = getenv("APP_AUTOPLAY");
    if(p != NULL && p[0] != '\0') {
        lv_textarea_set_text(s_ip_ta, p);
        do_play(p);
    }
}

static void osd_timer_cb(lv_timer_t * t)
{
    (void)t;
    ui_ivs_overlay_update();            /* ~33Hz：跟随帧率 */
}

/* ---------------- 页面创建 ---------------- */

void ui_page_play_create(lv_obj_t * parent)
{
    s_page = parent;

    lv_obj_t * ip_lbl = lv_label_create(parent);
    lv_label_set_text(ip_lbl, "RTSP:");
    lv_obj_set_style_text_color(ip_lbl, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(ip_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(ip_lbl, 0, 12);

    s_ip_ta = lv_textarea_create(parent);
    lv_obj_set_pos(s_ip_ta, 70, 8);
    lv_obj_set_size(s_ip_ta, 500, 62);
    lv_textarea_set_placeholder_text(s_ip_ta, "rtsp://ip:port/stream1");
    /* 板端 stream_proc 的 RTSP 路由为 /live0../live3（live0=主码流，live2=子码流） */
    lv_textarea_set_text(s_ip_ta, "rtsp://192.168.10.168:554/live0");
    lv_obj_set_scrollable(s_ip_ta, false);
    lv_obj_set_style_pad_all(s_ip_ta, 10, 0);
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
    lv_obj_set_size(data_btn, 90, 36);
    lv_obj_add_event_cb(data_btn, data_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * data_lbl = lv_label_create(data_btn);
    lv_label_set_text(data_lbl, "Data");
    lv_obj_center(data_lbl);

    lv_obj_t * osd_btn = lv_btn_create(parent);
    lv_obj_set_pos(osd_btn, 885, 8);
    lv_obj_set_size(osd_btn, 90, 36);
    lv_obj_add_event_cb(osd_btn, osd_btn_cb, LV_EVENT_CLICKED, NULL);
    s_osd_btn_lbl = lv_label_create(osd_btn);
    lv_label_set_text(s_osd_btn_lbl, "OSD on");
    lv_obj_set_style_text_font(s_osd_btn_lbl, &lv_font_montserrat_14, 0);
    lv_obj_center(s_osd_btn_lbl);

    s_status_lbl = lv_label_create(parent);
    lv_label_set_text(s_status_lbl, "idle");
    lv_obj_set_style_text_color(s_status_lbl, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(s_status_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(s_status_lbl, 990, 12);

    /* 文本面板（默认展开）与画面叠加层 */
    ui_ivs_panel_create(parent, APP_PANEL_W, APP_PLAYER_H);
    ui_ivs_panel_set_visible(true);
    ui_ivs_panel_set_retry_cb(retry_cb);

    ui_ivs_overlay_create();
    {
        /* 开发用：APP_OSD=0 可关闭叠加层，便于隔离"遮挡/渲染"类问题 */
        const char * osd_env = getenv("APP_OSD");
        ui_ivs_overlay_set_visible(!(osd_env != NULL && osd_env[0] == '0'));
    }

    s_tmr_panel = lv_timer_create(panel_timer_cb, 200, NULL);
    s_tmr_osd   = lv_timer_create(osd_timer_cb, 30, NULL);

    if(getenv("APP_AUTOPLAY") != NULL) {
        lv_timer_create(autoplay_timer_cb, 300, NULL);   /* 一次性，等界面就绪 */
    }
}

void ui_page_play_on_event(const comm_event_t * evt)
{
    /* 识别结果不再走通信事件队列，直接由 ivs_store 单槽提供；
     * 这里保留空实现以满足页面注册表接口。 */
    (void)evt;
}

void ui_page_play_set_visible(bool v)
{
    if(s_player == NULL) return;
    if(v) lv_obj_set_hidden(s_player, false);
    else lv_obj_set_hidden(s_player, true);
}

/* ===================== 生命周期回调 ===================== */

void ui_page_play_on_show(void)
{
    if(s_player != NULL) {
        lv_obj_set_hidden(s_player, false);
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_START);
    }
    ui_page_set_sidebar_btns_hightlight(APP_PAGE_PLAY);
}

bool ui_page_play_on_hide(int target_page)
{
    (void)target_page;

    if(s_player != NULL) {
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_STOP);
        lv_obj_set_hidden(s_player, true);
    }
    /* 离开页面即断开识别会话，回来重播时再建立 */
    stop_session();

    if(s_fullscreen) {
        s_fullscreen = false;
    }
    return true;
}

void ui_page_play_destroy(void)
{
    if(s_tmr_panel) { lv_timer_delete(s_tmr_panel); s_tmr_panel = NULL; }
    if(s_tmr_osd)   { lv_timer_delete(s_tmr_osd);   s_tmr_osd = NULL; }

    if(s_player != NULL) {
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_STOP);
        lv_obj_delete(s_player);
        s_player = NULL;
    }
    stop_session();
    ui_ivs_panel_destroy();
    ui_ivs_overlay_destroy();

    g_av_err[0] = '\0';
    s_board_ip[0] = '\0';
    s_page = NULL;
    s_fullscreen = false;
}
