#include "ui.h"
#include "ui_app.h"
#include "ui_sidebar.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/* ===================== 页面注册表 ===================== */
/* 新增页面：在 ui_app.h 的 app_page_id_t 枚举中登记索引，
 * 并在下面的 s_page_ops 表中注册 create / on_event / on_show /
 * on_hide / destroy 函数即可。某回调不需要时填 NULL。 */
/**後續統一每個頁面的接口改成，
 * // 页面结构体
    typedef struct {
        void (*init)(void);
        void (*deinit)(void);
        lv_obj_t **page_obj;
    } Page_t;
    Page_t Page_Home = {ui_HomePage_screen_init, ui_HomePage_screen_deinit, &ui_HomePage};
    Page_t Page_Power = {ui_PowerPage_screen_init, ui_PowerPage_screen_deinit, &ui_PowerPage};
    這樣每個頁面的創建、 銷毀、隱藏、顯示、就都能調用同一個函數

 */
static const page_ops_t s_page_ops[APP_PAGE_COUNT] = {
    [APP_PAGE_PLAY] = {
        .create   = ui_page_play_create,
        .on_event = ui_page_play_on_event,
        .on_show  = ui_page_play_on_show,
        .on_hide  = ui_page_play_on_hide,
        .destroy  = ui_page_play_destroy,
    },
    [APP_PAGE_UPGRADE] = {
        .create   = ui_page_upgrade_create,
        .on_event = ui_page_upgrade_on_event,
        .on_show  = ui_page_upgrade_on_show,
        .on_hide  = ui_page_upgrade_on_hide,
        .destroy  = ui_page_upgrade_destroy,
    },
    [APP_PAGE_DEBUG] = {
        .create   = ui_page_debug_create,
        .on_event = ui_page_debug_on_event,
        .on_show  = ui_page_debug_on_show,
        .on_hide  = ui_page_debug_on_hide,
        .destroy  = ui_page_debug_destroy,
    },[APP_PAGE_MULTPLAY] = {
        .create   = ui_page_multplay_create,
        .on_event = ui_page_multplay_on_event,
        .on_show  = ui_page_multplay_on_show,
        .on_hide  = ui_page_multplay_on_hide,
        .destroy  = ui_page_multplay_destroy,
    },
};

app_ctx_t g_app;

/* ===================== 页面切换 ===================== */

void app_show_page(int idx)
{
    printf("current_page id %d\n",g_app.current_page);
    if(idx < 0 || idx >= APP_PAGE_COUNT) return;
    if(g_app.pages[idx] == NULL) return;
    if(idx == g_app.current_page) return;  /* 已经是当前页，不重复切换 */

    /* ① 隐藏旧页面 → 调用 on_hide（可阻止离开） */
    if(g_app.current_page >= 0 && g_app.current_page < APP_PAGE_COUNT) {
        int old = g_app.current_page;
        if(s_page_ops[old].on_hide != NULL) {
            bool allow = s_page_ops[old].on_hide(idx);
            if(!allow) {
                /* 旧页面拒绝离开 → 恢复旧页面显示，取消切换 */
                return;
            }
        }
        lv_obj_add_flag(g_app.pages[old], LV_OBJ_FLAG_HIDDEN);
    }

    /* ② 显示新页面 → 调用 on_show */
    lv_obj_remove_flag(g_app.pages[idx], LV_OBJ_FLAG_HIDDEN);
    if(s_page_ops[idx].on_show)
        s_page_ops[idx].on_show();



    /* Play 页隐藏时同步隐藏其 RTSP 播放器，避免浮在其它页面之上 */
    ui_page_play_set_visible(idx == APP_PAGE_PLAY);

    g_app.current_page = idx;
}

/* ===================== 对外访问 ===================== */

app_ctx_t * app_get_ctx(void)
{
    return &g_app;
}

int app_refresh_device_dropdown(lv_obj_t * dd)
{
    if(dd == NULL) return 0;

    int cnt = comm_get_device_count();
    if(cnt <= 0) {
        lv_dropdown_set_options(dd, "No device");
        return 0;
    }

    char items[512];
    char line[128];
    items[0] = '\0';
    for(int i = 0; i < cnt; i++) {
        comm_device_t dev;
        comm_get_device(i, &dev);
        snprintf(line, sizeof(line), "%s  ID:%u", dev.ip, dev.dev_id);
        if(i > 0) strncat(items, "\n", sizeof(items) - strlen(items) - 1);
        strncat(items, line, sizeof(items) - strlen(items) - 1);
    }
    lv_dropdown_set_options(dd, items);
    if((int)lv_dropdown_get_selected(dd) < 0) lv_dropdown_set_selected(dd, 0);
    return cnt;
}

/* ===================== 事件轮询 ===================== */

static void poll_timer_cb(lv_timer_t * t)
{
    (void)t;
    comm_event_t evt;
    while(comm_pull_event(&evt)) {
        for(int i = 0; i < APP_PAGE_COUNT; i++) {
            if(s_page_ops[i].on_event != NULL) s_page_ops[i].on_event(&evt);
        }
    }
}
void ui_event_sidebar_change_page(lv_event_t * e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    printf("ui_event_sidebar_change_page event_code %d\n ",event_code);
    if(event_code == LV_EVENT_GESTURE)
    {
        int nowpage = g_app.current_page;
        int newpageindex = nowpage;
        switch (lv_indev_get_gesture_dir(lv_indev_get_act()))
        {
        case LV_DIR_TOP:

            newpageindex = (nowpage + 1)/APP_PAGE_COUNT;

            break;
        case LV_DIR_BOTTOM:
            newpageindex = (nowpage - 1)/APP_PAGE_COUNT;
            break;

        default:
            break;
        }
        printf("LV_EVENT_GESTURE :current_page %d\tnewpageindex%d \n ",g_app.current_page,newpageindex);
            app_show_page(newpageindex);
    }
}
/* ===================== 布局初始化 ===================== */
/* root 为全屏 flex 行容器：侧边栏固定宽，content 用 flex_grow 自适应
 * 窗口尺寸变化，避免初始化时一次性取分辨率导致 resize 后错乱。 */

static void app_layout_init(lv_obj_t * scr)
{
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_color(scr, APP_COLOR_SCREEN, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    g_app.root = lv_obj_create(scr);
    lv_obj_remove_style_all(g_app.root);
    lv_obj_set_size(g_app.root, lv_pct(100), lv_pct(100));
    lv_obj_set_pos(g_app.root, 0, 0);
    lv_obj_set_style_bg_color(g_app.root, APP_COLOR_SCREEN, 0);
    lv_obj_set_style_bg_opa(g_app.root, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(g_app.root, 0, 0);
    lv_obj_set_flex_flow(g_app.root, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_app.root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    g_app.sidebar = lv_obj_create(g_app.root);
    lv_obj_remove_style_all(g_app.sidebar);
    lv_obj_set_size(g_app.sidebar, APP_SIDEBAR_W, lv_pct(100));
    lv_obj_set_style_bg_color(g_app.sidebar, APP_COLOR_SIDEBAR, 0);
    lv_obj_set_style_bg_opa(g_app.sidebar, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(g_app.sidebar, 0, 0);
    lv_obj_add_event_cb(g_app.sidebar,ui_event_sidebar_change_page,LV_EVENT_GESTURE,NULL);


    g_app.content = lv_obj_create(g_app.root);
    lv_obj_remove_style_all(g_app.content);
    lv_obj_set_flex_grow(g_app.content, 1);           /* 占满剩余宽度 */
    lv_obj_set_size(g_app.content, 0, lv_pct(100));   /* 宽度由 flex_grow 决定，高度 100% */
    lv_obj_set_style_pad_all(g_app.content, APP_PAGE_PAD, 0);
    lv_obj_set_style_border_width(g_app.content, 0, 0);
    lv_obj_set_style_bg_color(g_app.content, APP_COLOR_SCREEN, 0);
    lv_obj_set_style_bg_opa(g_app.content, LV_OPA_COVER, 0);
}

/* ===================== 页面创建 ===================== */
/* 先统一设置页面容器样式，再调用各页面的 create 填充子控件，
 * 杜绝原实现中 "create 之后 remove_style_all" 覆盖样式的顺序缺陷。 */

static void app_pages_create(void)
{
    for(int i = 0; i < APP_PAGE_COUNT; i++) {
        lv_obj_t * p = lv_obj_create(g_app.content);
        lv_obj_remove_style_all(p);
        lv_obj_set_size(p, lv_pct(100), lv_pct(100));
        lv_obj_set_style_pad_all(p, APP_PAGE_PAD, 0);
        lv_obj_set_style_bg_color(p, APP_COLOR_PANEL, 0);
        lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(p, 0, 0);
        lv_obj_set_style_radius(p, 8, 0);
        lv_obj_add_flag(p, LV_OBJ_FLAG_HIDDEN);   /* 创建即隐藏，避免页面闪现 */
        g_app.pages[i] = p;
        /* 遍历注册表，依次创建各个分页 */
        if(s_page_ops[i].create != NULL) s_page_ops[i].create(p);
    }
}
/* ③ 更新侧边栏高亮 */
void ui_page_set_sidebar_btns_hightlight(int index)
{

   for(int i = 0; i < APP_PAGE_COUNT; i++) {
    if(g_app.sidebar_btns[i] != NULL) {
        lv_obj_set_style_bg_color(g_app.sidebar_btns[i],
                                  (i == index) ? APP_COLOR_ACCENT : APP_COLOR_BTN_IDLE, 0);
    printf("set btn id %d HL\n",i);
                                }
    }
}
/* ===================== 页面销毁 ===================== */

static void app_pages_destroy(void)
{
    for(int i = 0; i < APP_PAGE_COUNT; i++) {
        if(s_page_ops[i].destroy) s_page_ops[i].destroy();
    }
}

/* ===================== 轮询定时器 ===================== */

static void app_poll_timer_init(void)
{
    g_app.poll_timer = lv_timer_create(poll_timer_cb, 50, NULL);
}

/* ===================== 对外入口 ===================== */
/* 注意：comm_init() 已从本函数移出，由 main() 在调用 ui_init()
 * 之前按序初始化（UI 层不再负责通信层生命周期）。 */

void ui_init(void)
{
    /* 初始化上下文 */
    g_app.current_page = -1;

    lv_obj_t * scr = lv_screen_active();

    /* 主屏幕的布局 */
    app_layout_init(scr);
    /* 設置左側導航欄内容*/
    ui_sidebar_create(g_app.sidebar);
    /* 一次性創建三個頁面*/
    app_pages_create();

    /* 默认页：可用环境变量 APP_START_PAGE 覆盖（play/upgrade/debug/multplay），
     * 便于调试与自动化截图；未设置时保持原行为（multPlay）。 */
    {
        int start = APP_PAGE_MULTPLAY;
        const char *env = getenv("APP_START_PAGE");
        if(env != NULL) {
            if(strcmp(env, "play") == 0)         start = APP_PAGE_PLAY;
            else if(strcmp(env, "upgrade") == 0) start = APP_PAGE_UPGRADE;
            else if(strcmp(env, "debug") == 0)   start = APP_PAGE_DEBUG;
        }
        app_show_page(start);
    }

    app_poll_timer_init();
}
