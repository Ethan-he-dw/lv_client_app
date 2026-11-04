/**
 * @file ui_ivs_panel.c
 * @brief NPU 结果面板实现（通用，不认具体任务字段）
 */

#include "ui_ivs_panel.h"
#include "ui_app.h"

#include "ivs_source.h"
#include "ivs_store.h"

#include <stdio.h>
#include <string.h>

#define PANEL_BG       0x101014
#define PANEL_BORDER   0x3a3f47
#define TEXT_NORMAL    0xcccccc
#define TEXT_DIM       0x777777
#define TEXT_TITLE     0x4da3ff
#define TEXT_FAIL      0xff6b6b
#define DOT_STREAM     0x22C55E
#define DOT_WAIT       0x888780
#define DOT_NODATA     0xFACC15
#define DOT_FAIL       0xE24B4A

#define STATUS_BUF  128
#define ATTRS_BUF   640
#define OBJS_BUF    640

static lv_obj_t     *s_panel;
static lv_obj_t     *s_dot;
static lv_obj_t     *s_lbl_status;
static lv_obj_t     *s_lbl_attrs;
static lv_obj_t     *s_lbl_objs;
static lv_obj_t     *s_btn_retry;

static char          s_prev_status[STATUS_BUF];
static char          s_prev_attrs[ATTRS_BUF];
static char          s_prev_objs[OBJS_BUF];

static ivs_result_t  s_last;
static uint32_t      s_ver;
static uint32_t      s_rx_frames;
static uint64_t      s_last_rx_ms;

static void        (*s_retry_cb)(void);

/* ---------------- 小工具 ---------------- */

static void set_text_if_changed(lv_obj_t *lbl, char *prev, size_t prev_sz, const char *now)
{
    if (lbl == NULL || now == NULL) return;
    if (strncmp(prev, now, prev_sz) == 0) return;      /* 未变化：不碰控件，避免重排 */
    strncpy(prev, now, prev_sz - 1);
    prev[prev_sz - 1] = '\0';
    lv_label_set_text(lbl, now);
}

static void dot_set(lv_color_t c)
{
    if (s_dot) lv_obj_set_style_bg_color(s_dot, c, 0);
}

static void retry_clicked(lv_event_t *e)
{
    (void)e;
    if (s_retry_cb) s_retry_cb();
}

/* ---------------- 创建 ---------------- */

void ui_ivs_panel_create(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
    if (s_panel) return;

    s_panel = lv_obj_create(parent);
    lv_obj_set_size(s_panel, w, h);
    lv_obj_align(s_panel, LV_ALIGN_TOP_RIGHT, 0, APP_PLAYER_TOP);
    lv_obj_set_style_bg_color(s_panel, lv_color_hex(PANEL_BG), 0);
    lv_obj_set_style_bg_opa(s_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_panel, lv_color_hex(PANEL_BORDER), 0);
    lv_obj_set_style_border_width(s_panel, 1, 0);
    lv_obj_set_style_radius(s_panel, 6, 0);
    lv_obj_set_style_pad_all(s_panel, 8, 0);
    lv_obj_set_style_pad_row(s_panel, 6, 0);
    lv_obj_set_scrollable(s_panel, false);
    lv_obj_set_flex_flow(s_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    /* 标题行：色点 + 标题 + 重试按钮 */
    lv_obj_t *row = lv_obj_create(s_panel);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), 20);
    lv_obj_set_scrollable(row, false);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 6, 0);

    s_dot = lv_obj_create(row);
    lv_obj_remove_style_all(s_dot);
    lv_obj_set_size(s_dot, 10, 10);
    lv_obj_set_style_radius(s_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_dot, lv_color_hex(DOT_WAIT), 0);
    lv_obj_set_style_bg_opa(s_dot, LV_OPA_COVER, 0);

    lv_obj_t *title = lv_label_create(row);
    lv_label_set_text(title, "NPU RESULT");
    lv_obj_set_style_text_color(title, lv_color_hex(TEXT_TITLE), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_flex_grow(title, 1);

    s_btn_retry = lv_btn_create(row);
    lv_obj_set_size(s_btn_retry, 56, 20);
    lv_obj_set_style_pad_all(s_btn_retry, 0, 0);
    lv_obj_add_event_cb(s_btn_retry, retry_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_t *rl = lv_label_create(s_btn_retry);
    lv_label_set_text(rl, "retry");
    lv_obj_set_style_text_font(rl, &lv_font_montserrat_12, 0);
    lv_obj_center(rl);
    lv_obj_set_hidden(s_btn_retry, true);

    /* 状态行 */
    s_lbl_status = lv_label_create(s_panel);
    lv_label_set_text(s_lbl_status, "st=IDLE");
    lv_obj_set_style_text_color(s_lbl_status, lv_color_hex(TEXT_NORMAL), 0);
    lv_obj_set_style_text_font(s_lbl_status, &lv_font_montserrat_12, 0);
    lv_obj_set_width(s_lbl_status, lv_pct(100));
    lv_label_set_long_mode(s_lbl_status, LV_LABEL_LONG_WRAP);

    /* 属性区（键值对，来自 attrs[]，与本模块无关的任务字段也照此显示） */
    s_lbl_attrs = lv_label_create(s_panel);
    lv_label_set_text(s_lbl_attrs, "");
    lv_obj_set_style_text_color(s_lbl_attrs, lv_color_hex(TEXT_NORMAL), 0);
    lv_obj_set_style_text_font(s_lbl_attrs, &lv_font_montserrat_12, 0);
    lv_obj_set_width(s_lbl_attrs, lv_pct(100));
    lv_label_set_long_mode(s_lbl_attrs, LV_LABEL_LONG_WRAP);

    /* 对象区 */
    s_lbl_objs = lv_label_create(s_panel);
    lv_label_set_text(s_lbl_objs, "");
    lv_obj_set_style_text_color(s_lbl_objs, lv_color_hex(TEXT_NORMAL), 0);
    lv_obj_set_style_text_font(s_lbl_objs, &lv_font_montserrat_12, 0);
    lv_obj_set_width(s_lbl_objs, lv_pct(100));
    lv_label_set_long_mode(s_lbl_objs, LV_LABEL_LONG_WRAP);

    memset(s_prev_status, 0, sizeof(s_prev_status));
    memset(s_prev_attrs, 0, sizeof(s_prev_attrs));
    memset(s_prev_objs, 0, sizeof(s_prev_objs));
    s_ver = 0;
    s_rx_frames = 0;
    s_last_rx_ms = 0;
    s_retry_cb = NULL;
}

void ui_ivs_panel_destroy(void)
{
    if (s_panel) {
        lv_obj_delete(s_panel);
        s_panel = NULL;
    }
    s_dot = s_lbl_status = s_lbl_attrs = s_lbl_objs = s_btn_retry = NULL;
    s_ver = 0;
    s_rx_frames = 0;
    s_last_rx_ms = 0;
    s_retry_cb = NULL;
}

void ui_ivs_panel_set_retry_cb(void (*cb)(void))
{
    s_retry_cb = cb;
}

/* ---------------- 显隐 ---------------- */

bool ui_ivs_panel_is_visible(void)
{
    if (s_panel == NULL) return false;
    return !lv_obj_is_hidden(s_panel);
}

void ui_ivs_panel_set_visible(bool v)
{
    if (s_panel == NULL) return;
    if (v) lv_obj_set_hidden(s_panel, false);
    else   lv_obj_set_hidden(s_panel, true);
}

bool ui_ivs_panel_toggle(void)
{
    if (s_panel == NULL) return false;
    bool v = !ui_ivs_panel_is_visible();
    ui_ivs_panel_set_visible(v);
    return v;
}

/* ---------------- 刷新（5Hz） ---------------- */

static void build_attr_text(const ivs_result_t *r, char *out, size_t out_sz)
{
    out[0] = '\0';
    size_t used = 0;
    for (uint8_t i = 0; i < r->attr_n && i < IVS_MAX_ATTRS; i++) {
        int n = snprintf(out + used, out_sz - used, "%s%-9s: %s",
                         (i == 0) ? "" : "\n", r->attrs[i].key, r->attrs[i].val);
        if (n < 0 || (size_t)n >= out_sz - used) break;
        used += (size_t)n;
    }
}

static void build_obj_text(const ivs_result_t *r, char *out, size_t out_sz)
{
    out[0] = '\0';
    size_t used = 0;
    const uint16_t shown = (r->obj_count < 4) ? r->obj_count : 4;

    for (uint16_t i = 0; i < shown; i++) {
        const ivs_object_t *o = &r->objs[i];
        int n = snprintf(out + used, out_sz - used,
                         "%s#%u s=%.2f (%d,%d)-(%d,%d)%s%s",
                         (i == 0) ? "" : "\n",
                         (unsigned)i, (double)o->score,
                         (int)o->x1, (int)o->y1, (int)o->x2, (int)o->y2,
                         (o->label[0] != '\0') ? " " : "",
                         o->label);
        if (n < 0 || (size_t)n >= out_sz - used) break;
        used += (size_t)n;
    }

    if (r->obj_count > shown) {
        snprintf(out + used, out_sz - used, "\n... +%u more", (unsigned)(r->obj_count - shown));
    }
}

void ui_ivs_panel_refresh(void)
{
    if (s_panel == NULL) return;

    const ivs_state_t st = ivs_state();

    if (ivs_store_get_if_new(&s_last, &s_ver)) {
        s_rx_frames++;
        s_last_rx_ms = ivs_now_ms();
    }

    /* 状态行：状态永远刷新（即使没有新帧），保证"链路死了"能被看见 */
    char sbuf[STATUS_BUF];
    if (st == IVS_NODATA) {
        const uint64_t age_ms = (s_last_rx_ms != 0) ? (ivs_now_ms() - s_last_rx_ms) : 0;
        snprintf(sbuf, sizeof(sbuf), "st=%s(%llus)  seq=%llu  objs=%u  rx=%u",
                 ivs_state_str(st), (unsigned long long)(age_ms / 1000),
                 (unsigned long long)s_last.seq, s_last.obj_count, s_rx_frames);
        dot_set(lv_color_hex(DOT_NODATA));
    } else if (st == IVS_FAILED) {
        const char *err = ivs_last_error();
        snprintf(sbuf, sizeof(sbuf), "st=%s  %s", ivs_state_str(st), (err && err[0]) ? err : "");
        dot_set(lv_color_hex(DOT_FAIL));
    } else if (st == IVS_STREAMING) {
        snprintf(sbuf, sizeof(sbuf), "st=%s  seq=%llu  objs=%u  rx=%u",
                 ivs_state_str(st), (unsigned long long)s_last.seq, s_last.obj_count, s_rx_frames);
        dot_set(lv_color_hex(DOT_STREAM));
    } else {
        snprintf(sbuf, sizeof(sbuf), "st=%s  rx=%u", ivs_state_str(st), s_rx_frames);
        dot_set(lv_color_hex(DOT_WAIT));
    }
    set_text_if_changed(s_lbl_status, s_prev_status, sizeof(s_prev_status), sbuf);

    /* 属性与对象：只在有新帧时重拼 */
    if (s_ver != 0) {
        static char buf[ATTRS_BUF];
        build_attr_text(&s_last, buf, sizeof(buf));
        set_text_if_changed(s_lbl_attrs, s_prev_attrs, sizeof(s_prev_attrs), buf);

        static char obuf[OBJS_BUF];
        build_obj_text(&s_last, obuf, sizeof(obuf));
        set_text_if_changed(s_lbl_objs, s_prev_objs, sizeof(s_prev_objs), obuf);

        const lv_color_t c = (st == IVS_NODATA || st == IVS_FAILED) ? lv_color_hex(TEXT_DIM)
                                                                    : lv_color_hex(TEXT_NORMAL);
        lv_obj_set_style_text_color(s_lbl_attrs, c, 0);
        lv_obj_set_style_text_color(s_lbl_objs, c, 0);
    }

    /* 重试按钮只在 FAILED 且有回调时出现 */
    if (s_btn_retry) {
        if (st == IVS_FAILED && s_retry_cb) lv_obj_set_hidden(s_btn_retry, false);
        else                                lv_obj_set_hidden(s_btn_retry, true);
    }
}
