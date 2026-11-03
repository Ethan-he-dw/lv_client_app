#include "ui_page_play_multplayer.h"
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



static lv_obj_t * s_multplayers = NULL;
#define MAX_PLAYERS 9
static lv_obj_t * s_players[MAX_PLAYERS] = {0};
static int s_player_count = 0;

/* 用于延迟重建的 timer 参数 */
typedef struct {
    int ch_index;
} rebuild_timer_data_t;


static int32_t video_width(void)
{
    int32_t cw = lv_obj_get_content_width(s_page);
    if(s_overlay && !lv_obj_is_hidden(s_overlay)) {
        cw -= DATA_PANEL_W + 10;
    }
    return cw;
}

static void apply_player_layout(void)
{
    printf("apply_player_layout\n");
    if(s_player == NULL) return;
    if(s_fullscreen) {
        printf("apply_player_layout s_fullscreen\n");
        int32_t sw = lv_display_get_horizontal_resolution(lv_display_get_default());
        int32_t sh = lv_display_get_vertical_resolution(lv_display_get_default());
        lv_obj_set_pos(s_player, 0, 0);
        lv_obj_set_size(s_player, sw, sh);
        lv_obj_set_hidden(s_overlay,true);
    }
    else {
        printf("apply_player_layout not fullscreen\n");
        lv_area_t ca;
        lv_obj_get_content_coords(s_page, &ca);
        lv_obj_set_pos(s_player, ca.x1, ca.y1 + PLAYER_TOP);
        lv_obj_set_size(s_player, video_width(), PLAYER_H);
        lv_obj_set_hidden(s_overlay,false);
    }
}
static void apply_single_player_layout(lv_obj_t *player,bool fullscreen)
{
    if(player == NULL) return;
    if(fullscreen) {
        printf("apply_single_player_layout s_fullscreen\n");
        int32_t sw = lv_display_get_horizontal_resolution(lv_display_get_default());
        int32_t sh = lv_display_get_vertical_resolution(lv_display_get_default());
        lv_obj_set_pos(player, 0, 0);
        lv_obj_set_size(player, sw, sh);
        lv_obj_set_hidden(player,false);
    }
    else {
        printf("apply_player_layout  not s_fullscreen\n");
        //lv_area_t ca;
        //lv_obj_get_content_coords(player, &ca);
        ////lv_obj_set_pos(player, ca.x1, ca.y1 + PLAYER_TOP);
        ////lv_obj_set_size(player, lv_obj_get_content_width(player), lv_obj_get_content_height(player));
        //lv_obj_set_pos(player, 0, 0);
        //lv_obj_set_size(player, lv_obj_get_content_width(player), lv_obj_get_content_height(player));
        ////lv_obj_set_hidden(player,false);
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

        lv_obj_set_hidden(s_overlay,true);
        lv_label_set_text(s_status_lbl, "fullscreen, dbl-click to restore");
    }
    else {

        lv_obj_set_hidden(s_overlay,false);
        lv_label_set_text(s_status_lbl, "playing");
    }
    apply_player_layout();

}
static void set_player_dbl_cb(lv_event_t * e)
{
    lv_obj_t *btn = lv_event_get_target_obj(e);
    lv_obj_t *opt = lv_obj_get_parent(btn);           /* btn → opt */
    lv_obj_t *single_player = lv_obj_get_parent(opt);  /* opt → single_player */
    player_data_t *data = (player_data_t *)lv_obj_get_user_data(single_player);
    if (data) {
        LV_LOG_USER("当前 fullscreen: %d", data->fullscreen);
        printf("当前 fullscreen: %d", data->fullscreen);
        data->fullscreen = (!data->fullscreen);
    }

    player_widgets_t *player_widgets = (player_widgets_t *)lv_event_get_user_data(e);
    if(data->fullscreen) {
        lv_obj_set_hidden(player_widgets->player,true);
    }
    else {

        lv_obj_set_hidden(player_widgets->player,false);
    }
    apply_single_player_layout(player_widgets->player,data->fullscreen);

}
/* 退出指定通道的全屏（不中断播放）
 * 注意：调用时 data->fullscreen 仍为 true，函数内部负责设为 false */
static void exit_channel_fullscreen(player_data_t *data)
{
    LV_LOG_USER("exit_channel_fullscreen \n");
    if(data == NULL) return;
    if(!data->fullscreen) return;  /* 仅在确实处于全屏时才执行恢复 */
    lv_obj_t *single_player = data->orig_parent;
    if(single_player == NULL) {
        LV_LOG_USER("get  single_player false  \n");
        return;
    }

    /* 恢复 single_player 的原始位置和大小 */
    lv_obj_set_pos(single_player, data->orig_x, data->orig_y);
    lv_obj_set_size(single_player, data->orig_w, data->orig_h);
    /* 显示底部控制栏 opt */
    if(data->opt) {
        lv_obj_set_hidden(data->opt, false);
    }
    /* 恢复 ffmpeg_player 的原始布局 */
    if(data->player) {
        lv_obj_set_pos(data->player, lv_pct(0), 0);
        lv_obj_set_size(data->player, lv_pct(100), lv_pct(85));
        lv_image_set_inner_align(data->player, LV_IMAGE_ALIGN_CONTAIN);
    }

    /* 移除溢出标志，恢复裁剪 */
    lv_obj_t *par = single_player;
    while(par) {
        lv_obj_remove_flag(par, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        par = lv_obj_get_parent(par);
    }

    /* 显示被隐藏的其他通道 */
    for(int i = 0; i < s_player_count; i++) {
        if(s_players[i] && s_players[i] != single_player) {
            lv_obj_remove_flag(s_players[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    data->fullscreen = false;
    printf("[exit_channel_fullscreen] restored pos(%d,%d) size(%dx%d)\n",
           (int)data->orig_x, (int)data->orig_y,
           (int)data->orig_w, (int)data->orig_h);
}

/* 退出所有通道的全屏状态（页面隐藏/销毁时调用） */
static void exit_all_fullscreen(void)
{
    for(int i = 0; i < s_player_count; i++) {
        if(s_players[i]) {
            player_data_t *d = (player_data_t *)lv_obj_get_user_data(s_players[i]);
            if(d) exit_channel_fullscreen(d);
        }
    }
}

/* 双击 ffmpeg_player 切换全屏（不中断播放） */
static void set_player_dbl_cb_new(lv_event_t * e)
{
    printf("set_player_dbl_cb_new\n");
    lv_obj_t *player = lv_event_get_target_obj(e);
    lv_obj_t *single_player = lv_obj_get_parent(player);  /* ffmpeg_player → single_player */
    player_data_t *data = (player_data_t *)lv_obj_get_user_data(single_player);
    if(data == NULL) {
        printf("[set_player_dbl_cb_new] player_data is NULL\n");
        return;
    }

    bool new_fullscreen = !data->fullscreen;

    printf("[set_player_dbl_cb_new] fullscreen %s \n", new_fullscreen ? "true" : "false");

    if(new_fullscreen) {
        data->fullscreen = true;
        /* ============ 进入全屏 ============ */
        LV_LOG_USER("set full screen show");
        /* 1. 保存 single_player 的原始位置和大小（相对于 s_multplayers） */
        data->orig_parent = single_player;
        data->orig_x = lv_obj_get_x(single_player);
        data->orig_y = lv_obj_get_y(single_player);
        data->orig_w = lv_obj_get_width(single_player);
        data->orig_h = lv_obj_get_height(single_player);
        printf("[set_player_dbl_cb_new] saved orig: pos(%d,%d) size(%dx%d)\n",
               (int)data->orig_x, (int)data->orig_y,
               (int)data->orig_w, (int)data->orig_h);

        /* 2. 计算 s_multplayers 在屏幕上的全局坐标 */
        lv_area_t mp_coords;
        lv_obj_get_coords(s_multplayers, &mp_coords);
        printf("[set_player_dbl_cb_new] s_multplayers global: (%d,%d)-(%d,%d)\n",
               (int)mp_coords.x1, (int)mp_coords.y1,
               (int)mp_coords.x2, (int)mp_coords.y2);

        /* 3. 允许 s_multplayers 溢出裁剪，逐级向上设置 */
        lv_obj_t *par = s_multplayers;
        while(par) {
            lv_obj_set_overflow_visible(par, false);
            par = lv_obj_get_parent(par);
        }


        /* 4. 设置 single_player 覆盖整个屏幕
         *    single_player 的位置是相对于 s_multplayers 的，
         *    要让 single_player 覆盖屏幕(0,0)起点，
         *    需要设 pos = (0,0) - s_multplayers_global */
        int32_t sw = lv_display_get_horizontal_resolution(lv_display_get_default());
        int32_t sh = lv_display_get_vertical_resolution(lv_display_get_default());
        int32_t s_multplayers_w = (int)mp_coords.x2 - (int)mp_coords.x1;
        int32_t s_multplayers_h = (int)mp_coords.y2 - (int)mp_coords.y1;
        lv_obj_set_pos(single_player, 0,0);
        lv_obj_set_size(single_player,  lv_pct(100),  lv_pct(100));

        /* 5. 提升 z-order 到最上层，确保覆盖其他内容 */
        lv_obj_move_foreground(single_player);

        /* 6. 隐藏其他通道的 single_player */
        for(int i = 0; i < s_player_count; i++) {
            if(s_players[i] && s_players[i] != single_player) {
                lv_obj_add_flag(s_players[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
        lv_obj_t *opt = data->opt;  /* 直接用 player_data 中保存的指针 */
        lv_obj_set_hidden(opt, true);
        lv_obj_set_pos(player, 0, 0);
        lv_obj_set_size(player,  lv_pct(100),  lv_pct(100));
        lv_image_set_inner_align(player, LV_IMAGE_ALIGN_CONTAIN);

        printf("[set_player_dbl_cb_new] entered fullscreen (%dx%d)\n", (int)sw, (int)sh);
    }
    else {
        /* ============ 退出全屏 ============ */
        LV_LOG_USER("exit full screen show");
        exit_channel_fullscreen(data);  /* 函数内部会将 data->fullscreen 设为 false */
    }
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
    lv_obj_set_clickable(s_player, true);
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
static void playall_btn_cb(lv_event_t * e)
{
    lv_obj_t *playerall_btn = lv_event_get_target_obj(e);
    lv_obj_t *opt = lv_obj_get_parent(playerall_btn);
    lv_obj_t *multplayer = lv_obj_get_parent(opt);//s_multplayers


    lv_obj_t *dropdown = lv_obj_get_child(opt,0);
    uint16_t ch_index = lv_dropdown_get_selected(dropdown);

    int total = s_grids[ch_index].ch_count;

    for(int i = 0; i < total; i++) {
        lv_obj_t *singalplayer = lv_obj_get_child(s_multplayers, i);
        if(singalplayer == NULL) continue;

        player_data_t *data = (player_data_t *)lv_obj_get_user_data(singalplayer);
        if(data == NULL || data->btn == NULL) continue;

        lv_obj_send_event(data->btn, LV_EVENT_CLICKED, NULL);
    }
    lv_obj_t *player_all_labe = lv_obj_get_child(playerall_btn,0);
    char* playall_status =  lv_label_get_text(player_all_labe);
    char *result = strstr(playall_status, "play");

    if (result != NULL) {
        lv_label_set_text(player_all_labe,"stop all");
    } else {
        lv_label_set_text(player_all_labe,"play all");
    }

}

static void player_btn_cb(lv_event_t * e)
{
    lv_obj_t *btn = lv_event_get_target_obj(e);
    lv_obj_t *opt = lv_obj_get_parent(btn);           /* btn → opt */
    lv_obj_t *single_player = lv_obj_get_parent(opt);  /* opt → single_player */
    printf("player_btn_cb\n");
    player_widgets_t *player_widgets = (player_widgets_t *)lv_event_get_user_data(e);
    if(player_widgets == NULL){
        printf("player userdata is null\n");
        return;
    }
    if(player_widgets->canvas) {
        lv_obj_set_hidden(player_widgets->canvas, true);
    }
    if(player_widgets->player) {
        lv_obj_set_hidden(player_widgets->player, false);
    }

    const char * url = lv_textarea_get_text(player_widgets->addr);
    if(url == NULL || url[0] == '\0') {
        printf("error url\n");
        return;
    } else{
        printf("[player_btn_cb] url:%s\n",url);
    }

    lv_obj_t * ffmpeg_player = player_widgets->player;
    if(ffmpeg_player == NULL) {
        printf("[player_btn_cb] ffmpeg_player is NULL and rebuilt \n");
        ffmpeg_player = lv_ffmpeg_player_create(single_player);
    }

    g_av_err[0] = '\0';
    av_log_set_level(AV_LOG_ERROR);
    av_log_set_callback(av_log_cb);

    lv_image_set_inner_align(ffmpeg_player, LV_IMAGE_ALIGN_CONTAIN);
    printf("[player_btn_cb] lv_image_set_inner_align\n");
    lv_obj_add_flag(ffmpeg_player, LV_OBJ_FLAG_CLICKABLE);



    if(lv_ffmpeg_player_set_src(ffmpeg_player, url) != LV_RESULT_OK) {
        if(g_av_err[0] != '\0') lv_label_set_text(s_status_lbl, g_av_err);
        else lv_label_set_text(s_status_lbl, "open failed");
        printf("[player_btn_cb] open failed\n");
        return;
    }
    printf("[player_btn_cb] lv_ffmpeg_player_set_src\n");
    lv_ffmpeg_player_set_auto_restart(ffmpeg_player, false);

    /* player_data_t 绑定在 single_player 上，btn → opt → single_player */
    player_data_t *data = (player_data_t *)lv_obj_get_user_data(single_player);
    if (data != NULL) {
        printf("[player_btn_cb] play state %d\n", data->state);
        if(data->state != LV_player_stop) {
            /* 当前正在播放 → 停止 */
            data->state = LV_player_stop;
            lv_label_set_text(lv_obj_get_child(player_widgets->btn, 0), "play");
            lv_ffmpeg_player_set_cmd(ffmpeg_player, LV_FFMPEG_PLAYER_CMD_STOP);
            printf("[player_btn_cb] stop\n");
            lv_ffmpeg_player_set_src(ffmpeg_player, "M:/LVGL_learn/lv_port_pc_vscode/video_Null.png");
            lv_ffmpeg_player_set_cmd(ffmpeg_player, LV_FFMPEG_PLAYER_CMD_START);

        } else {
            /* 当前已停止 → 播放 */
            data->state = LV_player_play;
            lv_label_set_text(lv_obj_get_child(player_widgets->btn, 0), "stop");
            lv_ffmpeg_player_set_cmd(ffmpeg_player, LV_FFMPEG_PLAYER_CMD_START);
            printf("[player_btn_cb] start\n");
        }
    } else {
        printf("[player_btn_cb] player_data is NULL!\n");
    }
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
static void play_btn_cb(lv_event_t * e)
{
    (void)e;
    do_play(lv_textarea_get_text(s_ip_ta));
}

static void data_btn_cb(lv_event_t * e)
{
    (void)e;
    if(s_fullscreen) return;
    if(lv_obj_is_hidden(s_overlay)) {

        lv_obj_set_hidden(s_overlay,false);
    }
    else {

        lv_obj_set_hidden(s_overlay,true);
    }
    apply_player_layout();
}
lv_obj_t * creat_single_palyer(lv_obj_t * parent, int canvas_w, int canvas_h)
{
    /* 拒绝非法尺寸，避免 lv_canvas_set_buffer 传入 <=0 的宽高导致卡死 */
    if(canvas_w <= 0 || canvas_h <= 0) {
        printf("[creat_single_palyer] skip: invalid size %dx%d\n", canvas_w, canvas_h);
        return NULL;
    }

    lv_obj_t * single_player = lv_obj_create(parent);
    lv_obj_set_size(single_player, canvas_w, canvas_h);
    lv_obj_set_style_pad_all(single_player, 0, 0);
    lv_obj_set_style_border_width(single_player, 4, 0);
    lv_obj_set_style_radius(single_player, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(single_player, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(single_player, LV_OPA_COVER, 0);
    lv_obj_t * ffmpeg_player = lv_ffmpeg_player_create(single_player);
    /* 使用软解码（默认），移除硬件解码器避免多路花屏 */
    lv_obj_set_size(ffmpeg_player, lv_pct(100),lv_pct(85));
    lv_obj_set_pos(ffmpeg_player, lv_pct(0),0);
    lv_image_set_inner_align(ffmpeg_player, LV_IMAGE_ALIGN_CONTAIN);

    lv_obj_set_clickable(ffmpeg_player,true);
    apply_single_player_layout(ffmpeg_player,false);
    lv_ffmpeg_player_set_src(ffmpeg_player, "M:/LVGL_learn/lv_port_pc_vscode/video_Null.png");
    lv_ffmpeg_player_set_cmd(ffmpeg_player, LV_FFMPEG_PLAYER_CMD_START);
    lv_obj_add_event_cb(ffmpeg_player, set_player_dbl_cb_new, LV_EVENT_DOUBLE_CLICKED, NULL);

    /* canvas 已移除，ffmpeg_player 直接渲染到 single_player */

    lv_obj_t * opt = lv_obj_create(single_player);
    lv_obj_set_style_bg_color(opt, lv_color_hex(0x4da3ff), 0);
    lv_obj_set_style_pad_all(opt, 1, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(opt, LV_OPA_10, 0);
    lv_obj_set_style_border_color(opt, lv_color_hex(0x3a3f47), 0);
    lv_obj_set_style_radius(opt, 0, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_set_size(opt,lv_pct(100),lv_pct(15));
    lv_obj_set_pos(opt,lv_pct(0),lv_pct(85));

    lv_obj_t * label_addr = lv_label_create(opt);
    lv_label_set_text(label_addr,"addr:");
    lv_obj_set_style_text_align(label_addr, LV_TEXT_ALIGN_CENTER, 0);  // 关键！文本居中（固定label大小是時使用）
    lv_obj_set_style_radius(label_addr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(label_addr,lv_pct(0),lv_pct(5));
    lv_obj_set_size(label_addr,lv_pct(10),lv_pct(100));
    lv_obj_set_style_pad_all(label_addr, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label_addr, LV_OPA_60, 0);
    lv_obj_set_style_border_width(label_addr, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(label_addr, lv_color_hex(0x9a3f47), 0);



    lv_obj_t * text_addr = lv_textarea_create(opt);
    lv_obj_set_style_pad_all(text_addr, 0, 0);
    lv_obj_set_style_border_width(text_addr, 2, 0);
    lv_obj_set_size(text_addr,lv_pct(60),lv_pct(100));
    lv_obj_set_pos(text_addr,lv_pct(11),lv_pct(0));
    lv_textarea_set_align(text_addr,LV_TEXT_ALIGN_LEFT);
    lv_obj_set_style_radius(text_addr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    //lv_textarea_set_placeholder_text(text_addr, "plz input video  addr\n");
    //lv_textarea_set_placeholder_text(text_addr, "rtsp://192.168.10.206/0");
    lv_textarea_set_text(text_addr, "rtsp://192.168.10.206/0");
    lv_obj_set_scrollable(text_addr,false);

    lv_obj_t * btn_play = lv_btn_create(opt);
    lv_obj_set_style_pad_all(btn_play, 0, 0);
    lv_obj_set_style_border_width(btn_play, 1, 0);
    lv_obj_set_size(btn_play,lv_pct(15),lv_pct(100));
    lv_obj_set_pos(btn_play,lv_pct(80),lv_pct(0));
    lv_obj_set_style_radius(btn_play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    player_widgets_t *player_widgets = lv_malloc(sizeof(player_widgets_t));
    player_widgets->addr = text_addr;
    player_widgets->btn = btn_play;
    player_widgets->canvas = NULL;
    player_widgets->player = ffmpeg_player;
    lv_obj_add_event_cb(btn_play, player_btn_cb, LV_EVENT_CLICKED, player_widgets);
    //lv_obj_add_event_cb(ffmpeg_player, set_player_dbl_cb, LV_EVENT_DOUBLE_CLICKED, player_widgets);



    lv_obj_t * label_play = lv_label_create(btn_play);
    //lv_obj_set_size(label_play,lv_pct(100),lv_pct(100));
    lv_obj_set_style_pad_all(label_play, 0, 0);
    lv_obj_set_style_border_width(label_play, 0, 0);
    lv_label_set_text(label_play,"play");
    lv_obj_center(label_play);
    lv_obj_set_style_radius(label_play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 分配并初始化数据结构
    player_data_t *player_data = (player_data_t *)lv_malloc(sizeof(player_data_t));
    if (!player_data) {
        // 处理内存分配失败
        //return NULL;
        LV_LOG_ERROR("init player_data_t false");
    }
    // 初始化字段
    player_data->url = lv_strdup("rtsp://...");
    player_data->state = LV_player_stop;
    player_data->org_ratio_show = false;
    player_data->fullscreen = false;
    player_data->addr = text_addr;
    player_data->btn = btn_play;
    player_data->canvas = NULL;
    player_data->player = ffmpeg_player;
    player_data->opt = opt;
    player_data->orig_parent = single_player;
    player_data->orig_x = 0;
    player_data->orig_y = 0;
    player_data->orig_w = canvas_w;
    player_data->orig_h = (int32_t)(canvas_h * 0.85);
    // 绑定到 single_player 对象（不是 opt）
    lv_obj_set_user_data(single_player, player_data);

    return single_player;
}
static void rebuild_players(int ch_index)
{
    printf("rebuild_players ch_index=%d\n", ch_index);

    /* 安全检查：如果 s_multplayers 尚未完成布局（宽高为 0），则跳过 */
    int32_t area_w = lv_obj_get_content_width(s_multplayers);
    int32_t area_h = lv_obj_get_content_height(s_multplayers);
    if(area_w <= 0 || area_h <= 0) {
        printf("[rebuild_players] skip: area not ready (%dx%d)\n", (int)area_w, (int)area_h);
        return;
    }

    // 1. 清除舊 player（記得釋放 canvas buffer）
    for(int i = 0; i < s_player_count; i++) {
        if(s_players[i]) {
            lv_obj_delete(s_players[i]);
            s_players[i] = NULL;
        }
    }
    s_player_count = 0;

    // 2. 查表
    int cols = s_grids[ch_index].cols;
    int rows = s_grids[ch_index].rows;
    int total = s_grids[ch_index].ch_count;

    // 3. 計算每個格子的像素尺寸
    int gap = 4;  // 間距
    int cell_w = (area_w - (cols + 1) * gap) / cols;
    int cell_h = (area_h - (rows + 1) * gap) / rows;

    if(cell_w <= 0 || cell_h <= 0) {
        printf("[rebuild_players] skip: cell size too small (%dx%d)\n", cell_w, cell_h);
        return;
    }

    // 4. 逐個創建 + 設定大小位置
    for(int i = 0; i < total; i++) {
        lv_obj_t * p = creat_single_palyer(s_multplayers, cell_w, cell_h);
        if(p == NULL) {
            printf("[rebuild_players] creat_single_palyer returned NULL for i=%d\n", i);
            continue;
        }
        lv_obj_set_size(p, cell_w, cell_h);

        // 計算行列位置
        int col = i % cols;
        int row = i / cols;
        lv_obj_set_pos(p, gap + col * (cell_w + gap),
                          gap + row * (cell_h + gap));

        s_players[i] = p;
        s_player_count++;
    }
}

/* 延迟重建的 timer 回调：等 LVGL 完成布局后再填充玩家 */
static void rebuild_players_timer_cb(lv_timer_t * t)
{
    int ch_index = (int)(intptr_t)lv_timer_get_user_data(t);
    lv_timer_delete(t);
    rebuild_players(ch_index);
}

static void chN_event_cb(lv_event_t * e)
{
    uint16_t sel = lv_dropdown_get_selected(lv_event_get_target(e));
    rebuild_players(sel);
}
void ui_page_multplay_create(lv_obj_t * parent)
{
    //lv_obj_t * multplayers = lv_obj_create(parent);
    s_multplayers = lv_obj_create(parent);
    lv_obj_set_style_pad_all(s_multplayers,0,0);
    lv_obj_set_pos(s_multplayers, 0, 0);
    lv_obj_set_size(s_multplayers,lv_pct(100),lv_pct(75));
    lv_obj_set_style_bg_color(s_multplayers, lv_color_hex(0x4da3ff), 0);
    lv_obj_set_style_bg_opa(s_multplayers, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_multplayers, lv_color_hex(0x3a3f47), 0);
    /*// 设置圆角半径（单位：像素）
    radius 值	效果
    0	直角（无圆角）
    10	圆角，半径 10px
    LV_RADIUS_CIRCLE	完全圆形（正方形变圆，矩形变椭圆）
    */
    lv_obj_set_style_radius(s_multplayers, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 注意：不在這裡調用 rebuild_players，因為此時父容器還是 HIDDEN，
       get_content_width/height 返回 0。改在 on_show 中用 timer 延迟填充。 */


    lv_obj_t * setopt = lv_obj_create(parent);
    lv_obj_set_style_pad_all(setopt, 0, 0);
    lv_obj_set_style_border_width(setopt, 0, 0);

    lv_obj_set_scrollable(setopt,false);
    lv_obj_set_pos(setopt, 0, lv_pct(75));
    lv_obj_set_size(setopt, lv_pct(100), lv_pct(5));
    lv_obj_set_style_radius(setopt, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        /* 创建下拉框 */
    lv_obj_t *chN = lv_dropdown_create(setopt);
    lv_obj_set_pos(chN, 0, 0);
    lv_obj_set_size(chN,lv_pct(15),lv_pct(90));
    lv_obj_set_style_radius(chN, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(chN,0,0);
    /* 设置选项 */
    lv_dropdown_set_options(chN, "CH4\nCH6\nCH8\nCH9");
    /* 设置默认选中项（索引从 0 开始） */
    lv_dropdown_set_selected(chN, 0);

    /* 下拉列表样式 */
    lv_obj_t *list = lv_dropdown_get_list(chN);  /* 获取弹出列表对象 */
    if (list) {
        lv_obj_set_style_bg_color(list, lv_color_hex(0x1A1A1A), LV_PART_MAIN);
        lv_obj_set_style_text_color(list, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_radius(list, 8, LV_PART_MAIN);
        lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);

        /* 选中项高亮 */
        lv_obj_set_style_bg_color(list, lv_color_hex(0x2196F3), LV_PART_SELECTED);
        lv_obj_set_style_text_color(list, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
    }
    lv_obj_set_flex_align(chN,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);

    /* 綁定下拉框值改變事件 */
    lv_obj_add_event_cb(chN, chN_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *cb = lv_checkbox_create(setopt);
    lv_obj_set_style_pad_all(cb,0,0);
    lv_obj_set_pos(cb,  lv_pct(20), 0);
    lv_obj_set_size(cb,lv_pct(40),lv_pct(90));
    /* 设置文本 */
    lv_checkbox_set_text(cb, "Whether to display in the original proportion");
    /* 设置为选中 */
    lv_obj_add_state(cb, LV_STATE_CHECKED);
    ///* 设置为未选中 */
    //lv_obj_clear_state(cb, LV_STATE_CHECKED);
    /* 切换选中状态 */
    if (lv_obj_has_state(cb, LV_STATE_CHECKED)) {
        lv_obj_clear_state(cb, LV_STATE_CHECKED);
    } else {
        lv_obj_add_state(cb, LV_STATE_CHECKED);
    }
    lv_obj_t *btn_is_all_play = lv_btn_create(setopt);
    lv_obj_set_pos(btn_is_all_play, lv_pct(65), 0);
    lv_obj_set_size(btn_is_all_play, lv_pct(15), lv_pct(90));
    lv_obj_set_style_pad_all(btn_is_all_play, 0, 0);  /* 移除按钮默认内边距，给标签留空间 */
    lv_obj_t *label_is_all_play = lv_label_create(btn_is_all_play);
    lv_label_set_text(label_is_all_play, "play all");
    lv_obj_center(label_is_all_play);  /* 标签居中显示 */
    lv_obj_add_event_cb(btn_is_all_play, playall_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_is_all_recording = lv_btn_create(setopt);
    lv_obj_set_pos(btn_is_all_recording, lv_pct(85), 0);
    lv_obj_set_size(btn_is_all_recording, lv_pct(15), lv_pct(90));
    lv_obj_set_style_pad_all(btn_is_all_recording, 0, 0);  /* 移除按钮默认内边距，给标签留空间 */
    lv_obj_t *label_is_all_recording = lv_label_create(btn_is_all_recording);
    lv_label_set_text(label_is_all_recording, "record all");  /* ← 修正变量名 */
    lv_obj_center(label_is_all_recording);  /* 标签居中显示 */

    lv_obj_t * msginfo = lv_obj_create(parent);
    lv_obj_set_style_radius(msginfo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(msginfo,0,0);
    lv_obj_set_pos(msginfo, 0, lv_pct(80));
    lv_obj_set_size(msginfo,lv_pct(100),lv_pct(15));
    //lv_obj_set_style_bg_color(msginfo, lv_color_hex(0x4da3ff), 0);


    lv_obj_t *playinfo =  lv_textarea_create(msginfo);
    lv_obj_set_style_radius(playinfo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(playinfo,0,0);
    lv_obj_set_pos(playinfo, 0, 0);
    lv_obj_set_size(playinfo,lv_pct(100),lv_pct(100));
    lv_textarea_set_placeholder_text(playinfo, "beging...\n");
    //lv_obj_set_style_bg_color(playinfo, lv_color_hex(0x6da3ff), 0);
    /* 禁止用户交互 */
    lv_textarea_set_cursor_click_pos(playinfo, false);   /* 禁止点击定位光标 */
    lv_textarea_set_password_mode(playinfo, false);

    lv_obj_set_click_focusable(playinfo,false);
    lv_obj_set_clickable(playinfo,false);


    lv_obj_t * auth_info_ = lv_obj_create(parent);
    lv_obj_set_style_pad_all(auth_info_,0,0);
    lv_obj_set_pos(auth_info_, 0, lv_pct(95));
    lv_obj_set_size(auth_info_,lv_pct(100),lv_pct(5));
    lv_obj_set_style_radius(auth_info_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 启用 Flex 布局 */
    lv_obj_set_layout(auth_info_, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(auth_info_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(auth_info_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * Provider = lv_label_create(auth_info_);
    lv_obj_set_style_pad_all(Provider,0,0);
    lv_label_set_text(Provider,"Provider: tv_h");

    lv_obj_t * version = lv_label_create(auth_info_);
    lv_obj_set_style_pad_all(version,0,0);
    lv_label_set_text(version,"version: V0.0.0");

    lv_obj_t * dd = lv_label_create(auth_info_);
    lv_obj_set_style_pad_all(dd,0,0);
    lv_label_set_text(dd,"data:20260826");




}
void ui_page_multplayer_create(lv_obj_t * parent)
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
        lv_obj_set_scrollable(s_ip_ta,false);

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

void ui_page_multplay_on_event(const comm_event_t * evt)
{
    if(evt->type == COMM_EVT_BACKEND_DATA) {
        if(evt->data_len > 0) backend_append(evt->data);
    }
}

void ui_page_multplay_set_visible(bool v)
{
    if(s_player == NULL) return;
    if(v) lv_obj_set_hidden(s_player, false);
    else lv_obj_set_hidden(s_player, true);
}

/* ===================== 生命周期回调 ===================== */

void ui_page_multplay_on_show(void)
{
    /* 首次顯示時，用 timer 延遲一幀再填充，確保 LVGL 已完成布局計算 */
    if(s_player_count == 0) {
        lv_timer_t * t = lv_timer_create_basic();
        lv_timer_set_cb(t, rebuild_players_timer_cb);
        lv_timer_set_user_data(t, (void *)(intptr_t)0);  /* 默認 CH4，索引 0 */
        lv_timer_set_repeat_count(t, 1);
        lv_timer_set_period(t, 10);  /* 10ms 後觸發，等布局完成 */
    }
    /* 恢复播放器显示并继续播放（如果之前在播放） */
    if(s_player != NULL) {
        lv_obj_set_hidden(s_player,false);
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_START);
    }
    ui_page_set_sidebar_btns_hightlight(APP_PAGE_MULTPLAY);
}

bool ui_page_multplay_on_hide(int target_page)
{
    (void)target_page;
    /* ★ 暂停播放器，释放 CPU/GPU 资源 */
    if(s_player != NULL) {
        lv_ffmpeg_player_set_cmd(s_player, LV_FFMPEG_PLAYER_CMD_STOP);
        lv_obj_set_hidden(s_player, true);
    }
    /* 退出全屏状态 */
    if(s_fullscreen) {
        s_fullscreen = false;
    }
    /* 退出所有通道的全屏状态 */
    exit_all_fullscreen();
    return true;  /* multPlay 页始终允许离开 */
}

void ui_page_multplay_destroy(void)
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
