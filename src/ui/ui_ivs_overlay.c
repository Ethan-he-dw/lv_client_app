/**
 * @file ui_ivs_overlay.c
 * @brief 画面叠加层实现（全池化，热路径零创建/删除）
 *
 * 几何：不需要编码分辨率。两步缩放互相抵消，等价于
 *   画布(640×640, pad_top=140) ──归一化──► 播放器内的显示矩形
 * 显示矩形由「解码尺寸 + CONTAIN」本地算出。
 */

#include "ui_ivs_overlay.h"
#include "ui_app.h"

#include "ivs_source.h"
#include "ivs_store.h"

#include <stdio.h>
#include <string.h>

#define DOT_SZ      6
#define BOX_W       2
#define KPS_PER_OBJ 5

/* 任务样式表：新增任务在这里加一行即可 */
typedef struct {
    uint32_t box;          /* 普通框 */
    uint32_t box_best;     /* 主目标 */
    uint32_t kps;          /* 关键点 */
    uint8_t  draw_kps;
    const char *label_empty;   /* 主目标无标签时的占位文案 */
} ivs_style_t;

static const ivs_style_t s_styles[] = {
    [IVS_TASK_FACE]    = { 0x22C55E, 0xFACC15, 0x60A5FA, 1, "unknown" },
    [IVS_TASK_PERSON]  = { 0x38BDF8, 0xFACC15, 0xF472B6, 1, "?"       },
    [IVS_TASK_VEHICLE] = { 0xA3E635, 0xFACC15, 0xFB923C, 0, "?"       },
    [IVS_TASK_CUSTOM]  = { 0x94A3B8, 0xFACC15, 0x94A3B8, 0, "?"       },
};

static lv_obj_t *s_cont;
static lv_obj_t *s_box[IVS_MAX_OBJS];
static lv_obj_t *s_dot[IVS_MAX_OBJS * KPS_PER_OBJ];
static lv_obj_t *s_name;

static lv_obj_t     *s_player;
static ivs_result_t  s_res;
static uint32_t      s_ver;
static bool          s_visible = true;

/* ---------------- 创建 ---------------- */

void ui_ivs_overlay_create(void)
{
    if (s_cont) return;

    /* 容器挂 screen 且透明：坐标直接按屏幕算（播放器也在 screen 上） */
    s_cont = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(s_cont);
    lv_obj_set_pos(s_cont, 0, 0);
    lv_obj_set_size(s_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(s_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_set_style_pad_all(s_cont, 0, 0);
    lv_obj_set_scrollable(s_cont, false);
    lv_obj_set_clickable(s_cont, false);   /* 不吃事件 */

    for (int i = 0; i < IVS_MAX_OBJS; i++) {
        lv_obj_t *b = lv_obj_create(s_cont);
        lv_obj_remove_style_all(b);
        lv_obj_set_style_bg_opa(b, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(b, BOX_W, 0);
        lv_obj_set_style_border_color(b, lv_color_hex(s_styles[IVS_TASK_FACE].box), 0);
        lv_obj_set_style_radius(b, 0, 0);
        lv_obj_set_hidden(b, true);
        s_box[i] = b;
    }

    for (int i = 0; i < IVS_MAX_OBJS * KPS_PER_OBJ; i++) {
        lv_obj_t *d = lv_obj_create(s_cont);
        lv_obj_remove_style_all(d);
        lv_obj_set_size(d, DOT_SZ, DOT_SZ);
        lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(d, lv_color_hex(s_styles[IVS_TASK_FACE].kps), 0);
        lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
        lv_obj_set_hidden(d, true);
        s_dot[i] = d;
    }

    s_name = lv_label_create(s_cont);
    lv_label_set_text(s_name, "");
    lv_obj_set_style_text_font(s_name, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_name, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(s_name, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(s_name, LV_OPA_60, 0);
    lv_obj_set_style_pad_hor(s_name, 4, 0);
    lv_obj_set_style_pad_ver(s_name, 2, 0);
    lv_obj_set_style_radius(s_name, 2, 0);
    lv_obj_set_hidden(s_name, true);

    s_ver = 0;
    memset(&s_res, 0, sizeof(s_res));
}

void ui_ivs_overlay_destroy(void)
{
    if (s_cont) {
        lv_obj_delete(s_cont);
        s_cont = NULL;
    }
    for (int i = 0; i < IVS_MAX_OBJS; i++) s_box[i] = NULL;
    for (int i = 0; i < IVS_MAX_OBJS * KPS_PER_OBJ; i++) s_dot[i] = NULL;
    s_name = NULL;
    s_player = NULL;
    s_ver = 0;
}

void ui_ivs_overlay_set_player(lv_obj_t *player)
{
    s_player = player;
    if (s_cont == NULL) return;

    if (player == NULL) {
        ui_ivs_overlay_clear();
        return;
    }
    /* 播放器通常后创建，必须提升层级否则被盖住 */
    lv_obj_move_foreground(s_cont);
    s_ver = 0;
    memset(&s_res, 0, sizeof(s_res));
}

void ui_ivs_overlay_set_visible(bool v)
{
    s_visible = v;
    if (!s_cont) return;
    if (!v) lv_obj_set_hidden(s_cont, true);
    /* 打开时不在这里解除隐藏：由 update() 根据状态决定，避免无数据时闪现空层 */
}

bool ui_ivs_overlay_is_visible(void)
{
    return s_visible;
}

void ui_ivs_overlay_clear(void)
{
    for (int i = 0; i < IVS_MAX_OBJS; i++) {
        if (s_box[i]) lv_obj_set_hidden(s_box[i], true);
    }
    for (int i = 0; i < IVS_MAX_OBJS * KPS_PER_OBJ; i++) {
        if (s_dot[i]) lv_obj_set_hidden(s_dot[i], true);
    }
    if (s_name) lv_obj_set_hidden(s_name, true);
    if (s_cont) lv_obj_set_hidden(s_cont, true);
    s_ver = 0;
    memset(&s_res, 0, sizeof(s_res));
}

/* ---------------- 几何 ---------------- */

/** 视频在播放器控件内的实际矩形（CONTAIN：等比缩放 + 居中留黑边） */
static bool video_rect(lv_area_t *out)
{
    if (s_player == NULL) return false;

    const int32_t aw = lv_image_get_src_width(s_player);
    const int32_t ah = lv_image_get_src_height(s_player);
    if (aw <= 0 || ah <= 0) return false;          /* 还没解出第一帧 */

    lv_area_t pc;
    lv_obj_get_coords(s_player, &pc);
    const int32_t pw = lv_area_get_width(&pc);
    const int32_t ph = lv_area_get_height(&pc);
    if (pw <= 0 || ph <= 0) return false;

    float s = (float)pw / (float)aw;
    const float sy = (float)ph / (float)ah;
    if (sy < s) s = sy;

    const int32_t dw = (int32_t)((float)aw * s);
    const int32_t dh = (int32_t)((float)ah * s);

    out->x1 = pc.x1 + (pw - dw) / 2;
    out->y1 = pc.y1 + (ph - dh) / 2;
    out->x2 = out->x1 + dw - 1;
    out->y2 = out->y1 + dh - 1;
    return true;
}

static int32_t map_x(float cx, const lv_area_t *vr)
{
    float u = cx / IVS_CANVAS_W;
    if (u < 0.0f) u = 0.0f;
    if (u > 1.0f) u = 1.0f;
    return vr->x1 + (int32_t)(u * (float)lv_area_get_width(vr));
}

static int32_t map_y(float cy, const lv_area_t *vr)
{
    float v = (cy - IVS_CANVAS_PAD) / IVS_ACTIVE_H;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    return vr->y1 + (int32_t)(v * (float)lv_area_get_height(vr));
}

/* ---------------- 更新 ---------------- */

static void hide_all(void)
{
    for (int i = 0; i < IVS_MAX_OBJS; i++) {
        if (s_box[i]) lv_obj_set_hidden(s_box[i], true);
    }
    for (int i = 0; i < IVS_MAX_OBJS * KPS_PER_OBJ; i++) {
        if (s_dot[i]) lv_obj_set_hidden(s_dot[i], true);
    }
    if (s_name) lv_obj_set_hidden(s_name, true);
    if (s_cont) lv_obj_set_hidden(s_cont, true);
}

void ui_ivs_overlay_update(void)
{
    if (s_cont == NULL) return;

    const ivs_state_t st = ivs_state();

    /* 不该显示的情形：开关关、无播放器、播放器被隐藏（切页）、链路无数据 */
    if (!s_visible || s_player == NULL ||
        lv_obj_is_hidden(s_player) ||
        st == IVS_IDLE || st == IVS_FAILED || st == IVS_NODATA) {
        hide_all();
        return;
    }

    lv_area_t vr;
    if (!video_rect(&vr)) { hide_all(); return; }

    ivs_store_get_if_new(&s_res, &s_ver);

    const ivs_style_t *sty = &s_styles[IVS_TASK_CUSTOM];
    if (s_res.task < (uint8_t)(sizeof(s_styles) / sizeof(s_styles[0]))) {
        sty = &s_styles[s_res.task];
    }

    lv_obj_set_hidden(s_cont, false);

    uint16_t n = s_res.obj_count;
    if (n > IVS_MAX_OBJS) n = IVS_MAX_OBJS;

    for (int i = 0; i < IVS_MAX_OBJS; i++) {
        lv_obj_t *b = s_box[i];
        if (b == NULL) continue;

        if (i >= (int)n) { lv_obj_set_hidden(b, true); continue; }

        const ivs_object_t *o = &s_res.objs[i];
        const int32_t px1 = map_x(o->x1, &vr);
        const int32_t py1 = map_y(o->y1, &vr);
        const int32_t px2 = map_x(o->x2, &vr);
        const int32_t py2 = map_y(o->y2, &vr);
        const int32_t w = px2 - px1;
        const int32_t h = py2 - py1;

        if (w < 4 || h < 4) {                       /* 与板端一致：<4px 丢弃 */
            lv_obj_set_hidden(b, true);
            continue;
        }

        lv_obj_set_pos(b, px1, py1);
        lv_obj_set_size(b, w, h);
        lv_obj_set_style_border_color(b, lv_color_hex(o->is_best ? sty->box_best : sty->box), 0);
        lv_obj_set_hidden(b, false);

        /* 关键点：全部框都画（按需可关） */
        const bool draw_kps = sty->draw_kps && (o->kps_n >= KPS_PER_OBJ * 2);
        for (int k = 0; k < KPS_PER_OBJ; k++) {
            lv_obj_t *d = s_dot[i * KPS_PER_OBJ + k];
            if (d == NULL) continue;
            if (!draw_kps) { lv_obj_set_hidden(d, true); continue; }

            const int32_t dx = map_x(o->kps[k * 2 + 0], &vr);
            const int32_t dy = map_y(o->kps[k * 2 + 1], &vr);
            lv_obj_set_pos(d, dx - DOT_SZ / 2, dy - DOT_SZ / 2);
            lv_obj_set_style_bg_color(d, lv_color_hex(sty->kps), 0);
            lv_obj_set_hidden(d, false);
        }
    }

    /* 标签：只给主目标显示 */
    if (s_name) {
        int bi = -1;
        for (int i = 0; i < (int)n; i++) {
            if (s_res.objs[i].is_best) { bi = i; break; }
        }
        if (bi < 0) {
            lv_obj_set_hidden(s_name, true);
        } else {
            const ivs_object_t *o = &s_res.objs[bi];
            const char *txt = (o->label[0] != '\0') ? o->label : sty->label_empty;
            lv_label_set_text(s_name, txt);

            const int32_t px = map_x(o->x1, &vr);
            const int32_t py = map_y(o->y1, &vr);
            const int32_t lh = lv_obj_get_height(s_name);
            int32_t ly = py - lh - 4;
            if (ly < vr.y1) ly = py + 2;            /* 顶部空间不足 → 放到框内 */
            lv_obj_set_pos(s_name, px, ly);
            lv_obj_set_hidden(s_name, false);
        }
    }
}
