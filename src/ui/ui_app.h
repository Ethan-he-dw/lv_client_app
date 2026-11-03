#ifndef UI_APP_H
#define UI_APP_H

#include "lvgl/lvgl.h"
#include "comm.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===================== 页面注册表 =====================
 * 新增页面时：在枚举中加入索引，并在 ui.c 的
 * s_page_ops[] 中登记对应函数即可。 */
typedef enum {
    APP_PAGE_PLAY = 0,
    APP_PAGE_UPGRADE,
    APP_PAGE_DEBUG,
    APP_PAGE_MULTPLAY,
    APP_PAGE_COUNT
} app_page_id_t;

/* ===================== 布局常量 ===================== */
#define APP_SIDEBAR_W   160
#define APP_PAGE_PAD    10

/* ===================== 主题色 ===================== */
#define APP_COLOR_SCREEN   lv_color_hex(0x1a1d21)
#define APP_COLOR_PANEL    lv_color_hex(0x22252b)
#define APP_COLOR_SIDEBAR  lv_color_hex(0x2b2f36)
#define APP_COLOR_BTN_IDLE lv_color_hex(0x3a3f47)
#define APP_COLOR_ACCENT   lv_color_hex(0x1f6feb)

/* ===================== 页面生命周期回调 ===================== */
typedef void (*page_create_fn)(lv_obj_t * parent);
typedef void (*page_event_fn)(const comm_event_t * evt);
typedef void (*page_lifecycle_fn)(void);

/* on_hide 特殊签名：返回 true 允许离开，返回 false 阻止离开（如升级中）
 * target_page 为用户想要切换到的目标页面索引 */
typedef bool (*page_hide_fn)(int target_page);

typedef struct {
    page_create_fn    create;     /* 创建页面控件 */
    page_event_fn     on_event;   /* 接收通信事件 */
    page_lifecycle_fn on_show;    /* 页面变为可见 */
    page_hide_fn      on_hide;    /* 页面被隐藏（可阻止） */
    page_lifecycle_fn destroy;    /* 页面被销毁 */
} page_ops_t;

/* ===================== 应用上下文 ===================== */
typedef struct {
    lv_obj_t * root;                       /* 全屏 flex 行容器 */
    lv_obj_t * sidebar;
    lv_obj_t * content;
    lv_obj_t * pages[APP_PAGE_COUNT];      /* 页面对象，索引与 app_page_id_t 对应 */
    lv_obj_t * sidebar_btns[APP_PAGE_COUNT];
    lv_timer_t * poll_timer;
    int current_page;                      /* 当前显示的页面索引，-1 表示无 */
} app_ctx_t;

extern app_ctx_t g_app;

/* ===================== 对外接口 ===================== */
void app_show_page(int idx);
app_ctx_t * app_get_ctx(void);
int app_refresh_device_dropdown(lv_obj_t * dd);

/* ===================== 页面生命周期（注册表） ===================== */
/* Play 页 */
void ui_page_play_create(lv_obj_t * parent);
void ui_page_play_on_event(const comm_event_t * evt);
void ui_page_play_set_visible(bool v);
void ui_page_play_on_show(void);
bool ui_page_play_on_hide(int target_page);
void ui_page_play_destroy(void);
/* multPlay 页 */
void ui_page_multplay_create(lv_obj_t * parent);
void ui_page_multplay_on_event(const comm_event_t * evt);
void ui_page_multplay_set_visible(bool v);
void ui_page_multplay_on_show(void);
bool ui_page_multplay_on_hide(int target_page);
void ui_page_multplay_destroy(void);
/* Upgrade 页 */
void ui_page_upgrade_create(lv_obj_t * parent);
void ui_page_upgrade_on_event(const comm_event_t * evt);
void ui_page_upgrade_on_show(void);
bool ui_page_upgrade_on_hide(int target_page);
void ui_page_upgrade_destroy(void);

/* Debug 页 */
void ui_page_debug_create(lv_obj_t * parent);
void ui_page_debug_on_event(const comm_event_t * evt);
void ui_page_debug_on_show(void);
bool ui_page_debug_on_hide(int target_page);
void ui_page_debug_destroy(void);

/* 侧边栏 */
void ui_sidebar_create(lv_obj_t * parent);
void ui_page_set_sidebar_btns_hightlight(int index);

#ifdef __cplusplus
}
#endif

#endif