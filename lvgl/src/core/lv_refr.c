/**
 * @file lv_refr.c
 * 屏幕刷新(重绘)核心模块：管理无效区域、图层渲染与缓冲区刷新
 *
 */

/*********************
 *      INCLUDES
 *      包含的头文件
 *********************/
#include "lv_refr_private.h"
#include "lv_obj_draw_private.h"
#include "../misc/lv_area_private.h"
#include "../draw/sw/lv_draw_sw_mask_private.h"
#include "lv_obj_private.h"
#include "lv_obj_event_private.h"
#include "../display/lv_display_private.h"
#include "../misc/lv_timer_private.h"
#include "../draw/lv_draw_private.h"
#include "../draw/opengles/lv_draw_opengles.h"
#include "lv_global.h"
#include "../lvgl_public.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/*Display being refreshed*/
/*正在被刷新的显示器*/
#define disp_refr LV_GLOBAL_DEFAULT()->disp_refresh

/**********************
 *      TYPEDEFS
 *      自定义类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数原型声明
 **********************/
static void lv_refr_join_area(void);
static void refr_invalid_areas(void);
static void refr_sync_areas(void);
static void refr_area(const lv_area_t * area_p, int32_t y_offset);
static void refr_configured_layer(lv_layer_t * layer);
static void refr_obj_and_children(lv_layer_t * layer, lv_obj_t * top_obj);
static uint32_t get_max_row(lv_display_t * disp, int32_t area_w, int32_t area_h);
static void draw_buf_flush(lv_display_t * disp);
static void call_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);
static void wait_for_flushing(lv_display_t * disp);
static void call_sync_cb(lv_display_t * disp, const lv_area_t * area);
static void wait_for_syncing(lv_display_t * disp);
static lv_result_t layer_get_area(lv_layer_t * layer, lv_obj_t * obj, lv_layer_type_t layer_type,
                                  lv_area_t * layer_area_out, lv_area_t * obj_draw_size_out);
static bool alpha_test_area_on_obj(lv_obj_t * obj, const lv_area_t * area);
#if LV_DRAW_TRANSFORM_USE_MATRIX
    static bool refr_check_obj_clip_overflow(lv_layer_t * layer, lv_obj_t * obj);
    static void refr_obj_matrix(lv_layer_t * layer, lv_obj_t * obj);
#endif

/**********************
 *  STATIC VARIABLES
 *  静态变量定义
 **********************/

/**********************
 *      MACROS
 *      宏定义
 **********************/
#if LV_USE_LOG && LV_LOG_TRACE_DISP_REFR
    #define LV_TRACE_REFR(...) LV_LOG_TRACE(__VA_ARGS__)
#else
    #define LV_TRACE_REFR(...)
#endif

/**********************
 *   GLOBAL FUNCTIONS
 *   全局函数实现
 **********************/

/**
 * Initialize the screen refresh subsystem
 * 初始化屏幕刷新子系统
 */
void lv_refr_init(void)
{
}

/* 反初始化屏幕刷新子系统 */
void lv_refr_deinit(void)
{
}

/* 立即执行一次刷新(强制动画立即刷新并触发指定或全部显示器的刷新定时器) */
void lv_refr_now(lv_display_t * disp)
{
    lv_anim_refr_now();

    if(disp) {
        if(disp->refr_timer) lv_display_refr_timer(disp->refr_timer);
    }
    else {
        lv_display_t * d;
        d = lv_display_get_next(NULL);
        while(d) {
            if(d->refr_timer) lv_display_refr_timer(d->refr_timer);
            d = lv_display_get_next(d);
        }
    }
}

/* 在指定图层上重绘指定对象及其子对象 */
void lv_obj_redraw(lv_layer_t * layer, lv_obj_t * obj)
{
    LV_CHECK_ARG(layer != NULL, return);
    LV_CHECK_ARG(obj != NULL, return);

    LV_PROFILER_REFR_BEGIN;
    lv_area_t clip_area_ori = layer->_clip_area;
    lv_area_t clip_coords_for_obj;

    /*The widget will be rendered.
     *So setters from now should use animations. */
    /*该部件即将被渲染。
     *因此从现在开始,属性设置操作应使用动画方式。*/
    obj->rendered = 1;

    /*Truncate the clip area to `obj size + ext size` area*/
    /*将裁剪区域截断到“对象尺寸 + 扩展尺寸”的范围*/
    lv_area_t obj_coords_ext;
    lv_obj_get_coords(obj, &obj_coords_ext);
    int32_t ext_draw_size = lv_obj_get_ext_draw_size(obj);
    lv_area_increase(&obj_coords_ext, ext_draw_size, ext_draw_size);

    if(!lv_area_intersect(&clip_coords_for_obj, &clip_area_ori, &obj_coords_ext)) {
        LV_PROFILER_REFR_END;
        return;
    }
    /*If the object is visible on the current clip area*/
    /*若对象在当前裁剪区域内可见*/
    layer->_clip_area = clip_coords_for_obj;

    lv_obj_send_event(obj, LV_EVENT_DRAW_MAIN_BEGIN, layer);
    lv_obj_send_event(obj, LV_EVENT_DRAW_MAIN, layer);
    lv_obj_send_event(obj, LV_EVENT_DRAW_MAIN_END, layer);
#if LV_USE_REFR_DEBUG
    lv_color_t debug_color = lv_color_make(lv_rand(0, 0xFF), lv_rand(0, 0xFF), lv_rand(0, 0xFF));
    lv_draw_rect_dsc_t draw_dsc;
    lv_draw_rect_dsc_init(&draw_dsc);
    draw_dsc.bg_color = debug_color;
    draw_dsc.bg_opa = LV_OPA_20;
    draw_dsc.border_width = 1;
    draw_dsc.border_opa = LV_OPA_30;
    draw_dsc.border_color = debug_color;
    lv_draw_rect(layer, &draw_dsc, &obj_coords_ext);
#endif

    const lv_area_t * obj_coords;
    if(lv_obj_is_overflow_visible(obj)) {
        obj_coords = &obj_coords_ext;
    }
    else {
        obj_coords = &obj->coords;
    }
    lv_area_t clip_coords_for_children;
    bool refr_children = true;
    if(!lv_area_intersect(&clip_coords_for_children, &layer->_clip_area, obj_coords) || layer->opa <= LV_OPA_MIN) {
        refr_children = false;
    }

    if(refr_children) {
        uint32_t i;
        uint32_t child_cnt = lv_obj_get_child_count(obj);
        if(child_cnt == 0) {
            /*If the object was visible on the clip area call the post draw events too*/
            /*若对象在裁剪区域内可见,则同时调用绘制后事件*/
            /*If all the children are redrawn make 'post draw' draw*/
            /*若所有子对象都已重绘,则执行“绘制后”绘制*/
            lv_obj_send_event(obj, LV_EVENT_DRAW_POST_BEGIN, layer);
            lv_obj_send_event(obj, LV_EVENT_DRAW_POST, layer);
            lv_obj_send_event(obj, LV_EVENT_DRAW_POST_END, layer);
        }
        else {
            layer->_clip_area = clip_coords_for_children;
            bool clip_corner = lv_obj_get_style_clip_corner(obj, LV_PART_MAIN);

            int32_t radius = 0;
            if(clip_corner) {
                radius = lv_obj_get_style_radius(obj, LV_PART_MAIN);
                if(radius == 0) clip_corner = false;
            }

            if(clip_corner == false) {
                for(i = 0; i < child_cnt; i++) {
                    lv_obj_t * child = obj->spec_attr->children[i];
                    lv_obj_refr(layer, child);
                }

                /*If the object was visible on the clip area call the post draw events too*/
                /*若对象在裁剪区域内可见,则同时调用绘制后事件*/
                /*If all the children are redrawn make 'post draw' draw*/
                /*若所有子对象都已重绘,则执行“绘制后”绘制*/
                lv_obj_send_event(obj, LV_EVENT_DRAW_POST_BEGIN, layer);
                lv_obj_send_event(obj, LV_EVENT_DRAW_POST, layer);
                lv_obj_send_event(obj, LV_EVENT_DRAW_POST_END, layer);
            }
            else {
                lv_layer_t * layer_children;
                lv_draw_mask_rect_dsc_t mask_draw_dsc;
                lv_draw_mask_rect_dsc_init(&mask_draw_dsc);
                mask_draw_dsc.radius = radius;
                mask_draw_dsc.area = obj->coords;

                lv_draw_image_dsc_t img_draw_dsc;
                lv_draw_image_dsc_init(&img_draw_dsc);

                int32_t short_side = LV_MIN(lv_area_get_width(&obj->coords), lv_area_get_height(&obj->coords));
                int32_t rout = LV_MIN(radius, short_side >> 1);

                lv_area_t bottom = obj->coords;
                bottom.y1 = bottom.y2 - rout + 1;
                if(lv_area_intersect(&bottom, &bottom, &layer->_clip_area)) {
                    layer_children = lv_draw_layer_create(layer, LV_COLOR_FORMAT_ARGB8888, &bottom);

                    for(i = 0; i < child_cnt; i++) {
                        lv_obj_t * child = obj->spec_attr->children[i];
                        lv_obj_refr(layer_children, child);
                    }

                    /*If all the children are redrawn send 'post draw' draw*/
                    /*若所有子对象都已重绘,则发送“绘制后”绘制*/
                    lv_obj_send_event(obj, LV_EVENT_DRAW_POST_BEGIN, layer_children);
                    lv_obj_send_event(obj, LV_EVENT_DRAW_POST, layer_children);
                    lv_obj_send_event(obj, LV_EVENT_DRAW_POST_END, layer_children);

                    lv_draw_mask_rect(layer_children, &mask_draw_dsc);

                    img_draw_dsc.src = layer_children;
                    lv_draw_layer(layer, &img_draw_dsc, &bottom);
                }

                lv_area_t top = obj->coords;
                top.y2 = top.y1 + rout - 1;
                if(lv_area_intersect(&top, &top, &layer->_clip_area)) {
                    layer_children = lv_draw_layer_create(layer, LV_COLOR_FORMAT_ARGB8888, &top);

                    for(i = 0; i < child_cnt; i++) {
                        lv_obj_t * child = obj->spec_attr->children[i];
                        lv_obj_refr(layer_children, child);
                    }

                    /*If all the children are redrawn send 'post draw' draw*/
                    /*若所有子对象都已重绘,则发送“绘制后”绘制*/
                    lv_obj_send_event(obj, LV_EVENT_DRAW_POST_BEGIN, layer_children);
                    lv_obj_send_event(obj, LV_EVENT_DRAW_POST, layer_children);
                    lv_obj_send_event(obj, LV_EVENT_DRAW_POST_END, layer_children);

                    lv_draw_mask_rect(layer_children, &mask_draw_dsc);

                    img_draw_dsc.src = layer_children;
                    lv_draw_layer(layer, &img_draw_dsc, &top);

                }

                lv_area_t mid = obj->coords;
                mid.y1 += rout;
                mid.y2 -= rout;
                if(lv_area_intersect(&mid, &mid, &layer->_clip_area)) {
                    layer->_clip_area = mid;
                    for(i = 0; i < child_cnt; i++) {
                        lv_obj_t * child = obj->spec_attr->children[i];
                        lv_obj_refr(layer, child);
                    }

                    /*If all the children are redrawn make 'post draw' draw*/
                    /*若所有子对象都已重绘,则执行“绘制后”绘制*/
                    lv_obj_send_event(obj, LV_EVENT_DRAW_POST_BEGIN, layer);
                    lv_obj_send_event(obj, LV_EVENT_DRAW_POST, layer);
                    lv_obj_send_event(obj, LV_EVENT_DRAW_POST_END, layer);

                }

            }
        }
    }

    layer->_clip_area = clip_area_ori;

    LV_PROFILER_REFR_END;
}

/* 使显示器的某个区域无效(标记为需要重绘) */
lv_result_t lv_inv_area(lv_display_t * disp, const lv_area_t * area_p)
{
    if(!disp) disp = lv_display_get_default();
    if(!disp) return LV_RESULT_INVALID;
    if(!lv_display_is_invalidation_enabled(disp)) return LV_RESULT_INVALID;

    /**
     * There are two reasons for this issue:
     *  1.LVGL API is being used across threads, such as modifying widget properties in another thread
     *    or within an interrupt handler during the main thread rendering process.
     *  2.User-customized widget modify widget properties/styles again within the DRAW event.
     *
     * Therefore, ensure that LVGL is used in a single-threaded manner, or refer to
     * documentation: https://docs.lvgl.io/master/porting/os.html for proper locking mechanisms.
     * Additionally, ensure that only drawing-related tasks are performed within the DRAW event,
     * and move widget property/style modifications to other events.
     */
    /**
     * 出现该问题有两种原因:
     *  1.跨线程使用 LVGL API,例如在主线程渲染过程中于另一线程或中断处理函数中修改部件属性;
     *  2.用户自定义部件在 DRAW 事件中再次修改部件属性/样式。
     *
     * 因此,请确保以单线程方式使用 LVGL,或参考文档
     * https://docs.lvgl.io/master/porting/os.html 了解正确的锁机制。
     * 此外,请确保在 DRAW 事件中仅执行与绘制相关的任务,
     * 并将部件属性/样式的修改放到其他事件中。
     */
    LV_ASSERT_MSG(!disp->rendering_in_progress, "Invalidate area is not allowed during rendering.");

    /*Clear the invalidate buffer if the parameter is NULL*/
    /*若参数为 NULL,则清空无效区域缓冲区*/
    if(area_p == NULL) {
        disp->inv_p = 0;
        return LV_RESULT_OK;
    }

    lv_area_t scr_area;
    scr_area.x1 = 0;
    scr_area.y1 = 0;
    scr_area.x2 = lv_display_get_horizontal_resolution(disp) - 1;
    scr_area.y2 = lv_display_get_vertical_resolution(disp) - 1;

    lv_area_t com_area;
    bool suc;

    suc = lv_area_intersect(&com_area, area_p, &scr_area);
    if(suc == false)  return LV_RESULT_INVALID; /*Out of the screen, 在屏幕之外*/

    if(disp->color_format == LV_COLOR_FORMAT_I1) {
        /*Make sure that the X coordinates start and end on byte boundary.
         *E.g. convert 11;27 to 8;31*/
        /*确保 X 坐标起始和结束于字节边界。
         *例如将 11;27 转换为 8;31*/
        com_area.x1 &= ~0x7; /*Round down: Nx8, 向下取整到 Nx8 的倍数*/
        com_area.x2 |= 0x7;    /*Round up: Nx8 - 1, 向上取整到 Nx8-1*/
    }

    /*If there were at least 1 invalid area in full refresh mode, redraw the whole screen*/
    /*在全屏刷新模式下,只要有至少 1 个无效区域,就重绘整个屏幕*/
    if(disp->render_mode == LV_DISPLAY_RENDER_MODE_FULL) {
        disp->inv_areas[0] = scr_area;
        disp->inv_p = 1;
        lv_display_send_event(disp, LV_EVENT_REFR_REQUEST, NULL);
        return LV_RESULT_OK;
    }

    lv_result_t res = lv_display_send_event(disp, LV_EVENT_INVALIDATE_AREA, &com_area);
    if(res != LV_RESULT_OK) return LV_RESULT_INVALID;

    /*Save only if this area is not in one of the saved areas*/
    /*仅当该区域不在已保存的区域内时才保存*/
    uint16_t i;
    for(i = 0; i < disp->inv_p; i++) {
        if(lv_area_is_in(&com_area, &disp->inv_areas[i], 0) != false) return LV_RESULT_OK;
    }

    /*Save the area*/
    /*保存该区域*/
    lv_area_t * tmp_area_p = &com_area;
    if(disp->inv_p >= LV_INV_BUF_SIZE) { /*If no place for the area add the screen, 若区域存放空间已满,则改为保存整个屏幕*/
        disp->inv_p = 0;
        tmp_area_p = &scr_area;
    }
    lv_area_copy(&disp->inv_areas[disp->inv_p], tmp_area_p);
    disp->inv_p++;

    lv_display_send_event(disp, LV_EVENT_REFR_REQUEST, NULL);

    return LV_RESULT_OK;
}

/**
 * Get the display which is being refreshed
 * 获取正在刷新的显示器
 * @return the display being refreshed
 * @return 正在刷新的显示器
 */
lv_display_t * lv_refr_get_disp_refreshing(void)
{
    return disp_refr;
}

/**
 * Get the display which is being refreshed
 * 设置当前正在刷新的显示器
 * @return the display being refreshed
 * @return 正在刷新的显示器
 */
void lv_refr_set_disp_refreshing(lv_display_t * disp)
{
    disp_refr = disp;
}

/* 显示器刷新定时器回调：执行一次完整的刷新流程 */
void lv_display_refr_timer(lv_timer_t * tmr)
{
    LV_PROFILER_REFR_BEGIN;
    LV_TRACE_REFR("begin");

    if(tmr) {
        disp_refr = tmr->user_data;
        /* Ensure the timer does not run again automatically.
         * This is done before refreshing in case refreshing invalidates something else.
         * However if the performance monitor is enabled keep the timer running to count the FPS.*/
        /* 确保定时器不会再次自动运行。
         * 此操作在刷新前完成,以防刷新过程中又使其他内容失效。
         * 但如果启用了性能监视器,则保持定时器运行以统计 FPS。*/
#if !LV_USE_PERF_MONITOR
        lv_timer_pause(tmr);
#endif
    }
    else {
        disp_refr = lv_display_get_default();
    }

    if(disp_refr == NULL) {
        LV_LOG_WARN("No display registered");
        LV_PROFILER_REFR_END;
        return;
    }

    lv_draw_buf_t * buf_act = disp_refr->buf_act;
    if(!(buf_act && buf_act->data && buf_act->data_size)) {
        LV_LOG_WARN("No draw buffer");
        LV_PROFILER_REFR_END;
        return;
    }

    lv_result_t res = lv_display_send_event(disp_refr, LV_EVENT_REFR_START, NULL);
    if(res == LV_RESULT_INVALID) {
        LV_TRACE_REFR("deleted");
        LV_PROFILER_REFR_END;
        return;
    }

    /*Refresh the screen's layout if required*/
    /*如有需要,先刷新屏幕布局*/
    LV_PROFILER_LAYOUT_BEGIN_TAG("layout");
    lv_obj_update_layout(disp_refr->act_scr);
    if(disp_refr->prev_scr) lv_obj_update_layout(disp_refr->prev_scr);

    lv_obj_update_layout(disp_refr->bottom_layer);
    lv_obj_update_layout(disp_refr->top_layer);
    lv_obj_update_layout(disp_refr->sys_layer);
    LV_PROFILER_LAYOUT_END_TAG("layout");

    /*Do nothing if there is no active screen*/
    /*若没有活动屏幕,则不做任何事*/
    if(disp_refr->act_scr == NULL) {
        disp_refr->inv_p = 0;
        LV_LOG_WARN("there is no active screen");
        goto refr_finish;
    }

    lv_refr_join_area();
    refr_sync_areas();
    refr_invalid_areas();

    if(disp_refr->inv_p == 0) goto refr_finish;
    /*In double buffered direct mode or if sync callback is set, save the updated areas.
     *They will be used on the next call to synchronize the buffers.*/
    /*在双缓冲直接模式或设置了同步回调时,保存已更新的区域。
     *它们将在下一次调用时用于同步缓冲区。*/
    if((lv_display_is_double_buffered(disp_refr) && disp_refr->render_mode == LV_DISPLAY_RENDER_MODE_DIRECT) ||
       disp_refr->sync_cb) {
        uint32_t i;
        for(i = 0; i < disp_refr->inv_p; i++) {
            if(disp_refr->inv_area_joined[i])
                continue;

            lv_area_t * sync_area = lv_ll_ins_tail(&disp_refr->sync_areas);
            *sync_area = disp_refr->inv_areas[i];
        }
    }

    lv_memzero(disp_refr->inv_areas, sizeof(disp_refr->inv_areas));
    lv_memzero(disp_refr->inv_area_joined, sizeof(disp_refr->inv_area_joined));
    disp_refr->inv_p = 0;

refr_finish:

#if LV_DRAW_SW_COMPLEX == 1
    lv_draw_sw_mask_cleanup();
#endif

    lv_display_send_event(disp_refr, LV_EVENT_REFR_READY, NULL);

    LV_TRACE_REFR("finished");
    LV_PROFILER_REFR_END;
}

/**
 * Search the most top object which fully covers an area
 * 查找完全覆盖某区域的最顶层对象
 * @param area_p pointer to an area
 * @param area_p 指向某个区域的指针
 * @param obj the first object to start the searching (typically a screen)
 * @param obj 搜索起始的第一个对象(通常是屏幕)
 * @return
 * @return 找到的最顶层对象
 */
lv_obj_t * lv_refr_get_top_obj(const lv_area_t * area_p, lv_obj_t * obj)
{
    LV_CHECK_ARG(area_p != NULL, return NULL);
    LV_CHECK_ARG(obj != NULL, return NULL);

    lv_obj_t * found_p = NULL;

    if(lv_area_is_in(area_p, &obj->coords, 0) == false) return NULL;
    if(lv_obj_is_hidden(obj)) return NULL;
    if(lv_obj_get_layer_type(obj) != LV_LAYER_TYPE_NONE) return NULL;
    if(lv_obj_get_style_opa(obj, LV_PART_MAIN) < LV_OPA_MAX) return NULL;

    /*If this object is fully cover the draw area then check the children too*/
    /*若该对象完全覆盖绘制区域,则继续检查其子对象*/
    lv_cover_check_info_t info;
    info.res = LV_COVER_RES_COVER;
    info.area = area_p;
    lv_obj_send_event(obj, LV_EVENT_COVER_CHECK, &info);
    if(info.res == LV_COVER_RES_MASKED) return NULL;

    int32_t i;
    int32_t child_cnt = lv_obj_get_child_count(obj);
    for(i = child_cnt - 1; i >= 0; i--) {
        lv_obj_t * child = obj->spec_attr->children[i];
        found_p = lv_refr_get_top_obj(area_p, child);

        /*If a children is ok then break*/
        /*若某个子对象满足条件则跳出循环*/
        if(found_p != NULL) {
            break;
        }
    }

    /*If no better children use this object*/
    /*若没有更合适的子对象,则使用该对象*/
    if(found_p == NULL && info.res == LV_COVER_RES_COVER) {
        found_p = obj;
    }

    return found_p;
}


/* 在图层上刷新一个对象(必要时使用独立图层进行变换与混合) */
void lv_obj_refr(lv_layer_t * layer, lv_obj_t * obj)
{
    LV_CHECK_ARG(layer != NULL, return);
    LV_CHECK_ARG(obj != NULL, return);

    if(lv_obj_is_hidden(obj)) return;

    /*If `opa_layered != LV_OPA_COVER` draw the widget on a new layer and blend that layer with the given opacity.*/
    /*若 opa_layered != LV_OPA_COVER,则把部件绘制到新图层上,并以指定不透明度混合该图层。*/
    const lv_opa_t opa_layered = lv_obj_get_style_opa_layered(obj, LV_PART_MAIN);
    if(opa_layered <= LV_OPA_MIN) return;

    const lv_opa_t layer_opa_ori = layer->opa;
    const lv_color32_t layer_recolor = layer->recolor;

    /*Normal `opa` (not layered) will just scale down `bg_opa`, `text_opa`, etc, in the upcoming drawings.*/
    /*普通 opa(非分层)只会在后续绘制中按比例降低 bg_opa、text_opa 等。*/
    const lv_opa_t opa_main = lv_obj_get_style_opa(obj, LV_PART_MAIN);
    if(opa_main < LV_OPA_MAX) {
        layer->opa = LV_OPA_MIX2(layer_opa_ori, opa_main);
    }

    layer->recolor = lv_obj_style_apply_recolor(obj, LV_PART_MAIN, layer->recolor);

    lv_layer_type_t layer_type = lv_obj_get_layer_type(obj);
    if(layer_type == LV_LAYER_TYPE_NONE) {
        lv_obj_redraw(layer, obj);
    }
#if LV_DRAW_TRANSFORM_USE_MATRIX
    /*If the layer opa is full then use the matrix transform*/
    /*若图层完全不透明,则使用矩阵变换*/
    else if(opa_layered >= LV_OPA_MAX && !refr_check_obj_clip_overflow(layer, obj)) {
        refr_obj_matrix(layer, obj);
    }
#endif /* LV_DRAW_TRANSFORM_USE_MATRIX, LV_DRAW_TRANSFORM_USE_MATRIX 条件编译结束 */
    else {
        lv_area_t layer_area_full;
        lv_area_t obj_draw_size;
        lv_result_t res = layer_get_area(layer, obj, layer_type, &layer_area_full, &obj_draw_size);
        if(res != LV_RESULT_OK) {
            layer->opa = layer_opa_ori;
            layer->recolor = layer_recolor;
            return;
        }

        /*Simple layers can be subdivided into smaller layers*/
        /*简单图层可细分为更小的图层*/
        uint32_t max_rgb_row_height = lv_area_get_height(&layer_area_full);
        uint32_t max_argb_row_height = lv_area_get_height(&layer_area_full);
        if(layer_type == LV_LAYER_TYPE_SIMPLE) {
            int32_t w = lv_area_get_width(&layer_area_full);
            uint8_t px_size = lv_color_format_get_size(disp_refr->color_format);
            max_rgb_row_height = LV_DRAW_LAYER_SIMPLE_BUF_SIZE / w / px_size;
            max_argb_row_height = LV_DRAW_LAYER_SIMPLE_BUF_SIZE / w / sizeof(lv_color32_t);
        }

        lv_area_t layer_area_act;
        layer_area_act.x1 = layer_area_full.x1;
        layer_area_act.x2 = layer_area_full.x2;
        layer_area_act.y1 = layer_area_full.y1;
        layer_area_act.y2 = layer_area_full.y1;

        while(layer_area_act.y2 < layer_area_full.y2) {
            /* Test with an RGB layer size (which is larger than the ARGB layer size)
             * If it really doesn't need alpha use it. Else switch to the ARGB size*/
            /* 先按 RGB 图层尺寸(比 ARGB 图层尺寸大)进行测试
             * 若确实不需要透明度则使用它,否则改用 ARGB 尺寸*/
            layer_area_act.y2 = layer_area_act.y1 + max_rgb_row_height - 1;
            if(layer_area_act.y2 > layer_area_full.y2) layer_area_act.y2 = layer_area_full.y2;

            const void * bitmap_mask_src = lv_obj_get_style_bitmap_mask_src(obj, LV_PART_MAIN);
            bool area_need_alpha = bitmap_mask_src || alpha_test_area_on_obj(obj, &layer_area_act);

            if(area_need_alpha) {
                layer_area_act.y2 = layer_area_act.y1 + max_argb_row_height - 1;
                if(layer_area_act.y2 > layer_area_full.y2) layer_area_act.y2 = layer_area_full.y2;
            }

            lv_layer_t * new_layer = lv_draw_layer_create(layer,
                                                          area_need_alpha ? LV_COLOR_FORMAT_ARGB8888 : LV_COLOR_FORMAT_NATIVE, &layer_area_act);
            lv_obj_redraw(new_layer, obj);

            lv_point_t pivot = {
                .x = lv_obj_get_style_transform_pivot_x(obj, LV_PART_MAIN),
                .y = lv_obj_get_style_transform_pivot_y(obj, LV_PART_MAIN)
            };

            if(LV_COORD_IS_PCT(pivot.x)) {
                pivot.x = (LV_COORD_GET_PCT(pivot.x) * lv_area_get_width(&obj->coords)) / 100;
            }
            if(LV_COORD_IS_PCT(pivot.y)) {
                pivot.y = (LV_COORD_GET_PCT(pivot.y) * lv_area_get_height(&obj->coords)) / 100;
            }

            lv_draw_image_dsc_t layer_draw_dsc;
            lv_draw_image_dsc_init(&layer_draw_dsc);
            layer_draw_dsc.pivot.x = obj->coords.x1 + pivot.x - new_layer->buf_area.x1;
            layer_draw_dsc.pivot.y = obj->coords.y1 + pivot.y - new_layer->buf_area.y1;

            layer_draw_dsc.opa = opa_layered;
            layer_draw_dsc.rotation = lv_obj_get_style_transform_rotation(obj, LV_PART_MAIN);
            while(layer_draw_dsc.rotation > 3600) layer_draw_dsc.rotation -= 3600;
            while(layer_draw_dsc.rotation < 0) layer_draw_dsc.rotation += 3600;
            layer_draw_dsc.scale_x = lv_obj_get_style_transform_scale_x(obj, LV_PART_MAIN);
            layer_draw_dsc.scale_y = lv_obj_get_style_transform_scale_y(obj, LV_PART_MAIN);
            layer_draw_dsc.skew_x = lv_obj_get_style_transform_skew_x(obj, LV_PART_MAIN);
            layer_draw_dsc.skew_y = lv_obj_get_style_transform_skew_y(obj, LV_PART_MAIN);
            layer_draw_dsc.blend_mode = lv_obj_get_style_blend_mode(obj, LV_PART_MAIN);
            layer_draw_dsc.antialias = disp_refr->antialiasing;
            layer_draw_dsc.bitmap_mask_src = bitmap_mask_src;
            layer_draw_dsc.image_area = obj_draw_size;
            layer_draw_dsc.src = new_layer;

            lv_draw_layer(layer, &layer_draw_dsc, &layer_area_act);

            layer_area_act.y1 = layer_area_act.y2 + 1;
        }
    }

    /* Restore the original layer opa and recolor */
    /* 恢复图层原有的不透明度与重新着色 */
    layer->opa = layer_opa_ori;
    layer->recolor = layer_recolor;
}

/**********************
 *   STATIC FUNCTIONS
 *   静态函数实现
 **********************/

/**
 * Join the areas which has got common parts
 * 合并具有公共部分的区域
 */
static void lv_refr_join_area(void)
{
    LV_PROFILER_REFR_BEGIN;
    uint32_t join_from;
    uint32_t join_in;
    lv_area_t joined_area;
    for(join_in = 0; join_in < disp_refr->inv_p; join_in++) {
        if(disp_refr->inv_area_joined[join_in] != 0) continue;

        /*Check all areas to join them in 'join_in'*/
        /*检查所有区域,尝试将其合并到 join_in 中*/
        for(join_from = 0; join_from < disp_refr->inv_p; join_from++) {
            /*Handle only unjoined areas and ignore itself*/
            /*仅处理未合并的区域,并忽略自身*/
            if(disp_refr->inv_area_joined[join_from] != 0 || join_in == join_from) {
                continue;
            }

            /*Check if the areas are on each other*/
            /*检查两个区域是否相互接触*/
            if(lv_area_is_on(&disp_refr->inv_areas[join_in], &disp_refr->inv_areas[join_from]) == false) {
                continue;
            }

            lv_area_join(&joined_area, &disp_refr->inv_areas[join_in], &disp_refr->inv_areas[join_from]);

            /*Join two area only if the joined area size is smaller*/
            /*仅当合并后的区域尺寸更小时才执行合并*/
            if(lv_area_get_size(&joined_area) < (lv_area_get_size(&disp_refr->inv_areas[join_in]) +
                                                 lv_area_get_size(&disp_refr->inv_areas[join_from]))) {
                lv_area_copy(&disp_refr->inv_areas[join_in], &joined_area);

                /*Mark 'join_form' is joined into 'join_in'*/
                /*标记 join_from 已被合并到 join_in 中*/
                disp_refr->inv_area_joined[join_from] = 1;
            }
        }
    }
    LV_PROFILER_REFR_END;
}

/**
 * Refresh the sync areas
 * 刷新(同步)同步区域
 */
static void refr_sync_areas(void)
{
    /*Do not sync if not direct double buffered and no sync callback set*/
    /*若非直接双缓冲且未设置同步回调,则不进行同步*/
    const bool auto_sync = disp_refr->render_mode == LV_DISPLAY_RENDER_MODE_DIRECT &&
                           lv_display_is_double_buffered(disp_refr);
    const bool user_sync = disp_refr->sync_cb != NULL;
    if(!auto_sync && !user_sync) return;

    /*Do not sync if no sync areas*/
    /*若无同步区域,则不进行同步*/
    if(lv_ll_is_empty(&disp_refr->sync_areas)) return;

    LV_PROFILER_REFR_BEGIN;
    /*With double buffered direct mode synchronize the rendered areas to the other buffer*/
    /*在双缓冲直接模式下,将渲染好的区域同步到另一个缓冲区*/
    /*We need to wait for ready here to not mess up the active screen*/
    /*此处需等待缓冲区就绪,以免打乱活动屏幕*/
    wait_for_flushing(disp_refr);

    /*Iterate through invalidated areas to see if sync area should be copied*/
    /*遍历无效区域,判断同步区域是否需要进行复制*/
    uint16_t i;
    int8_t j;
    lv_area_t res[4] = {0};
    int8_t res_c;
    lv_area_t * sync_area, * new_area, * next_area;
    for(i = 0; i < disp_refr->inv_p; i++) {
        /*Skip joined areas*/
        /*跳过已合并的区域*/
        if(disp_refr->inv_area_joined[i]) continue;

        /*Iterate over sync areas*/
        /*遍历同步区域*/
        sync_area = lv_ll_get_head(&disp_refr->sync_areas);
        while(sync_area != NULL) {
            /*Get next sync area*/
            /*获取下一个同步区域*/
            next_area = lv_ll_get_next(&disp_refr->sync_areas, sync_area);

            /*Remove intersect of redraw area from sync area and get remaining areas*/
            /*从同步区域中减去与重绘区域的交集,得到剩余区域*/
            res_c = lv_area_diff(res, sync_area, &disp_refr->inv_areas[i]);

            /*New sub areas created after removing intersect*/
            /*去除交集后新产生的子区域*/
            if(res_c != -1) {
                /*Replace old sync area with new areas*/
                /*用新区域替换旧的同步区域*/
                for(j = 0; j < res_c; j++) {
                    new_area = lv_ll_ins_prev(&disp_refr->sync_areas, sync_area);
                    *new_area = res[j];
                }
                lv_ll_remove(&disp_refr->sync_areas, sync_area);
                lv_free(sync_area);
            }

            /*Move on to next sync area*/
            /*继续处理下一个同步区域*/
            sync_area = next_area;
        }
    }

    /*The buffers are already swapped.
     *So the active buffer is the off screen buffer where LVGL will render*/
    /*缓冲区已完成交换。
     *因此活动缓冲区就是 LVGL 将要渲染的离屏缓冲区*/
    lv_draw_buf_t * off_screen = disp_refr->buf_act;
    /*Triple buffer sync buffer for off-screen2 updates.*/
    /*三缓冲时为 off-screen2 的更新同步缓冲区。*/
    lv_draw_buf_t * off_screen2 = NULL;
    lv_draw_buf_t * on_screen = NULL;

    /* Only compute buffer relationships when auto_sync (direct double-buffered) is used.
     * When only user_sync is active, these pointers are not needed. */
    /* 仅在使用 auto_sync(直接双缓冲)时计算缓冲区之间的关系。
     * 若只启用了 user_sync,则不需要这些指针。 */
    if(auto_sync) {
        if(disp_refr->buf_act == disp_refr->buf_1) {
            off_screen2 = disp_refr->buf_2;
            on_screen = disp_refr->buf_3 ? disp_refr->buf_3 : disp_refr->buf_2;
        }
        else if(disp_refr->buf_act == disp_refr->buf_2) {
            off_screen2 = disp_refr->buf_3 ? disp_refr->buf_3 : disp_refr->buf_1;
            on_screen = disp_refr->buf_1;
        }
        else {
            off_screen2 = disp_refr->buf_1;
            on_screen = disp_refr->buf_2;
        }
    }

    uint32_t hor_res = lv_display_get_horizontal_resolution(disp_refr);
    uint32_t ver_res = lv_display_get_vertical_resolution(disp_refr);

    lv_area_t disp_area = {0, 0, (int32_t)hor_res - 1, (int32_t)ver_res - 1};
    /*Copy sync areas (if any remaining)*/
    /*复制同步区域(如果还有剩余)*/
    for(sync_area = lv_ll_get_head(&disp_refr->sync_areas); sync_area != NULL;
        sync_area = lv_ll_get_next(&disp_refr->sync_areas, sync_area)) {
        /**
         * @todo Resize SDL window will trigger crash because of sync_area is larger than disp_area
         * @todo 调整 SDL 窗口大小可能因 sync_area 大于 disp_area 而导致崩溃
         */
        if(!lv_area_intersect(sync_area, sync_area, &disp_area)) {
            continue;
        }
#if LV_DRAW_TRANSFORM_USE_MATRIX
        if(lv_display_get_matrix_rotation(disp_refr)) {
            lv_display_rotate_area(disp_refr, sync_area);
        }
#endif
        /*Call sync callback (if set)*/
        /*调用同步回调(若已设置)*/
        if(disp_refr->sync_cb) {
            /*Set syncing flags*/
            /*设置同步中标志*/
            disp_refr->syncing = true;
            disp_refr->syncing_last = lv_ll_get_tail(&disp_refr->sync_areas) == sync_area;

            /*Call sync callback and wait for sync to complete*/
            /*调用同步回调并等待同步完成*/
            call_sync_cb(disp_refr, sync_area);
            wait_for_syncing(disp_refr);
        }
        /*Fallback to internal double buffered direct mode sync*/
        /*否则回退到内部双缓冲直接模式的同步方式*/
        else {
            lv_draw_buf_copy(off_screen, sync_area, on_screen, sync_area);
            if(off_screen2 != on_screen)
                lv_draw_buf_copy(off_screen2, sync_area, on_screen, sync_area);
        }
    }

    /*Clear sync areas*/
    /*清空同步区域*/
    lv_ll_clear(&disp_refr->sync_areas);
    LV_PROFILER_REFR_END;
}

/**
 * Refresh the joined areas
 * 刷新已合并的无效区域
 */
static void refr_invalid_areas(void)
{
    if(disp_refr->inv_p == 0) return;
    LV_PROFILER_REFR_BEGIN;

    /*Notify the display driven rendering has started*/
    /*通知显示驱动渲染已开始*/
    lv_display_send_event(disp_refr, LV_EVENT_RENDER_START, NULL);

    /*Find the last area which will be drawn*/
    /*找到最后一个将要绘制的区域*/
    int32_t i;
    int32_t last_i = 0;
    for(i = disp_refr->inv_p - 1; i >= 0; i--) {
        if(disp_refr->inv_area_joined[i] == 0) {
            last_i = i;
            break;
        }
    }

    disp_refr->last_area = 0;
    disp_refr->last_part = 0;
    disp_refr->rendering_in_progress = true;

    for(i = 0; i < (int32_t)disp_refr->inv_p; i++) {
        /*Refresh the unjoined areas*/
        /*刷新未被合并的区域*/
        if(disp_refr->inv_area_joined[i]) continue;

        if(i == last_i) disp_refr->last_area = 1;
        disp_refr->last_part = 0;

        lv_area_t inv_a = disp_refr->inv_areas[i];
        if(disp_refr->render_mode == LV_DISPLAY_RENDER_MODE_PARTIAL) {
            /*Calculate the max row num*/
            /*计算最大行数*/
            int32_t w = lv_area_get_width(&inv_a);
            int32_t h = lv_area_get_height(&inv_a);

            int32_t max_row = get_max_row(disp_refr, w, h);

            int32_t row;
            int32_t row_last = 0;
            lv_area_t sub_area;
            sub_area.x1 = inv_a.x1;
            sub_area.x2 = inv_a.x2;
            int32_t y_off = 0;
            for(row = inv_a.y1; row + max_row - 1 <= inv_a.y2; row += max_row) {
                /*Calc. the next y coordinates of draw_buf*/
                /*计算 draw_buf 的下一个 y 坐标范围*/
                sub_area.y1 = row;
                sub_area.y2 = row + max_row - 1;
                if(sub_area.y2 > inv_a.y2) sub_area.y2 = inv_a.y2;
                row_last = sub_area.y2;
                if(inv_a.y2 == row_last) disp_refr->last_part = 1;
                refr_area(&sub_area, y_off);
                y_off += lv_area_get_height(&sub_area);
                draw_buf_flush(disp_refr);
            }

            /*If the last y coordinates are not handled yet ...*/
            /*若最后的 y 坐标尚未处理...*/
            if(inv_a.y2 != row_last) {
                /*Calc. the next y coordinates of draw_buf*/
                /*计算 draw_buf 的下一个 y 坐标范围*/
                sub_area.y1 = row;
                sub_area.y2 = inv_a.y2;
                disp_refr->last_part = 1;
                refr_area(&sub_area, y_off);
                y_off += lv_area_get_height(&sub_area);
                draw_buf_flush(disp_refr);
            }
        }
        else if(disp_refr->render_mode == LV_DISPLAY_RENDER_MODE_FULL ||
                disp_refr->render_mode == LV_DISPLAY_RENDER_MODE_DIRECT) {
            disp_refr->last_part = 1;
            refr_area(&disp_refr->inv_areas[i], 0);
            draw_buf_flush(disp_refr);
        }
    }

    lv_display_send_event(disp_refr, LV_EVENT_RENDER_READY, NULL);
    disp_refr->rendering_in_progress = false;
    LV_PROFILER_REFR_END;
}

/**
 * Reshape the draw buffer if required
 * 如有需要,重新调整绘制缓冲区的大小
 * @param layer  pointer to a layer which will be drawn
 * @param layer  指向将要绘制的图层的指针
 */
static void layer_reshape_draw_buf(lv_layer_t * layer, uint32_t stride)
{
    lv_draw_buf_t * ret = lv_draw_buf_reshape(
                              layer->draw_buf,
                              layer->color_format,
                              lv_area_get_width(&layer->buf_area),
                              lv_area_get_height(&layer->buf_area),
                              stride);
    LV_UNUSED(ret);
    LV_ASSERT_NULL(ret);
}

/**
 * Refresh an area if there is Virtual Display Buffer
 * 刷新一个区域(如果存在虚拟显示缓冲区)
 * @param area_p  pointer to an area to refresh
 * @param area_p  指向待刷新区域的指针
 */
static void refr_area(const lv_area_t * area_p, int32_t y_offset)
{
    LV_PROFILER_REFR_BEGIN;
    lv_layer_t * layer = disp_refr->layer_head;
    layer->draw_buf = disp_refr->buf_act;
    layer->_clip_area = *area_p;
    layer->phy_clip_area = *area_p;
    layer->partial_y_offset = y_offset;
    layer->all_tasks_added = false;

    if(disp_refr->render_mode == LV_DISPLAY_RENDER_MODE_PARTIAL) {
        /*In partial mode render this area to the buffer*/
        /*在部分渲染模式下,将该区域渲染到缓冲区*/
        layer->buf_area = *area_p;
        layer_reshape_draw_buf(layer, LV_STRIDE_AUTO);
    }
    else if(disp_refr->render_mode == LV_DISPLAY_RENDER_MODE_DIRECT ||
            disp_refr->render_mode == LV_DISPLAY_RENDER_MODE_FULL) {
        /*In direct mode and full mode the buffer area is always the whole screen, not considering rotation*/
        /*在直接模式和全屏模式下,缓冲区区域始终为整个屏幕,不考虑旋转*/
        layer->buf_area.x1 = 0;
        layer->buf_area.y1 = 0;
        if(lv_display_get_matrix_rotation(disp_refr)) {
            layer->buf_area.x2 = lv_display_get_original_horizontal_resolution(disp_refr) - 1;
            layer->buf_area.y2 = lv_display_get_original_vertical_resolution(disp_refr) - 1;
        }
        else {
            layer->buf_area.x2 = lv_display_get_horizontal_resolution(disp_refr) - 1;
            layer->buf_area.y2 = lv_display_get_vertical_resolution(disp_refr) - 1;
        }
        layer_reshape_draw_buf(layer, disp_refr->stride_is_auto ? LV_STRIDE_AUTO : layer->draw_buf->header.stride);
    }

    /*Try to divide the area to smaller tiles*/
    /*尝试将区域划分为更小的瓦片*/
    uint32_t tile_cnt = 1;
    int32_t tile_h = lv_area_get_height(area_p);
    if(LV_COLOR_FORMAT_IS_INDEXED(layer->color_format) == false) {
        /* Assume that the buffer size (can be screen sized or smaller in case of partial mode)
         * and max tile size are the optimal scenario. From this calculate the ideal tile size
         * and set the tile count and tile height accordingly.
         */
        /* 假设缓冲区大小(在部分渲染模式下可为屏幕大小或更小)
         * 与最大瓦片大小是最优情形。据此计算出理想瓦片尺寸,
         * 并相应设置瓦片数量和瓦片高度。
         */
        uint32_t max_tile_cnt = disp_refr->tile_cnt;
        uint32_t total_buf_size = layer->draw_buf->data_size;
        uint32_t ideal_tile_size = total_buf_size / max_tile_cnt;
        uint32_t area_buf_size = lv_area_get_size(area_p) * lv_color_format_get_size(layer->color_format);

        tile_cnt = (area_buf_size + (ideal_tile_size - 1)) / ideal_tile_size; /*Round up, 向上取整*/
        tile_h = lv_area_get_height(area_p) / tile_cnt;
    }

    if(tile_cnt == 1) {
        refr_configured_layer(layer);
        layer->all_tasks_added = true;
    }
    else {
        /* Don't draw to the layers buffer of the display but create smaller dummy layers which are using the
         * display's layer buffer. These will be the tiles. By using tiles it's more likely that there will
         * be independent areas for each draw unit. */
        /* 不直接绘制到显示器的图层缓冲区,而是创建共享该缓冲区的小型虚拟图层。
         * 这些虚拟图层即瓦片。使用瓦片更有可能为每个绘制单元
         * 提供相互独立的区域。 */
        lv_layer_t * tile_layers = lv_malloc(tile_cnt * sizeof(lv_layer_t));
        LV_ASSERT_MALLOC(tile_layers);
        if(tile_layers == NULL) {
            disp_refr->refreshed_area = *area_p;
            LV_PROFILER_REFR_END;
            return;
        }
        uint32_t i;
        for(i = 0; i < tile_cnt; i++) {
            lv_area_t tile_area;
            lv_area_set(&tile_area, area_p->x1, area_p->y1 + i * tile_h,
                        area_p->x2, area_p->y1 + (i + 1) * tile_h - 1);

            if(i == tile_cnt - 1) {
                tile_area.y2 = area_p->y2;
            }

            lv_layer_t * tile_layer = &tile_layers[i];
            lv_draw_layer_init(tile_layer, NULL, layer->color_format, &tile_area);
            tile_layer->buf_area = layer->buf_area; /*the buffer is still large, 缓冲区仍然很大*/
            tile_layer->draw_buf = layer->draw_buf;
            refr_configured_layer(tile_layer);
            tile_layer->all_tasks_added = true;
        }


        /*Wait until all tiles are ready and destroy remove them*/
        /*等待所有瓦片绘制完成并销毁它们*/
        for(i = 0; i < tile_cnt; i++) {
            lv_layer_t * tile_layer = &tile_layers[i];
            while(tile_layer->draw_task_head) {
                lv_draw_dispatch_wait_for_request();
                lv_draw_dispatch();
            }

            lv_layer_t * layer_i = disp_refr->layer_head;
            while(layer_i) {
                if(layer_i->next == tile_layer) {
                    layer_i->next = tile_layer->next;
                    break;
                }
                layer_i = layer_i->next;
            }

            lv_draw_unit_send_event(NULL, LV_EVENT_CHILD_DELETED, tile_layer);
            if(disp_refr->layer_deinit) disp_refr->layer_deinit(disp_refr, tile_layer);
        }
        lv_free(tile_layers);

        layer->all_tasks_added = true;
    }

    disp_refr->refreshed_area = *area_p;
    LV_PROFILER_REFR_END;
}

/* 配置图层并执行实际的绘制流程 */
static void refr_configured_layer(lv_layer_t * layer)
{
    LV_PROFILER_REFR_BEGIN;

    lv_layer_reset(layer);

#if LV_DRAW_TRANSFORM_USE_MATRIX
    if(lv_display_get_matrix_rotation(disp_refr)) {
        const lv_display_rotation_t rotation = lv_display_get_rotation(disp_refr);
        if(rotation != LV_DISPLAY_ROTATION_0) {
            lv_display_rotate_area(disp_refr, &layer->phy_clip_area);

            /**
             * The screen rotation direction defined by LVGL is opposite to the drawing angle.
             * Use direct matrix assignment to reduce precision loss and improve efficiency.
             * LVGL 定义的屏幕旋转方向与绘制角度相反。
             * 直接赋值矩阵以减少精度损失并提高效率。
             */
            switch(rotation) {
                case LV_DISPLAY_ROTATION_90:
                    /**
                     * lv_matrix_rotate(&layer->matrix, 270);
                     * lv_matrix_translate(&layer->matrix, -disp_refr->ver_res, 0);
                     * 等价于上面的旋转加平移组合(旋转 270 度并平移)
                     */
                    layer->matrix.m[0][0] = 0;
                    layer->matrix.m[0][1] = 1;
                    layer->matrix.m[0][2] = 0;
                    layer->matrix.m[1][0] = -1;
                    layer->matrix.m[1][1] = 0;
                    layer->matrix.m[1][2] = disp_refr->ver_res;
                    break;

                case LV_DISPLAY_ROTATION_180:
                    /**
                     * lv_matrix_rotate(&layer->matrix, 180);
                     * lv_matrix_translate(&layer->matrix, -disp_refr->hor_res, -disp_refr->ver_res);
                     * 等价于上面的旋转加平移组合(旋转 180 度并平移)
                     */
                    layer->matrix.m[0][0] = -1;
                    layer->matrix.m[0][1] = 0;
                    layer->matrix.m[0][2] = disp_refr->hor_res;
                    layer->matrix.m[1][0] = 0;
                    layer->matrix.m[1][1] = -1;
                    layer->matrix.m[1][2] = disp_refr->ver_res;
                    break;

                case LV_DISPLAY_ROTATION_270:
                    /**
                     * lv_matrix_rotate(&layer->matrix, 90);
                     * lv_matrix_translate(&layer->matrix, 0, -disp_refr->hor_res);
                     * 等价于上面的旋转加平移组合(旋转 90 度并平移)
                     */
                    layer->matrix.m[0][0] = 0;
                    layer->matrix.m[0][1] = -1;
                    layer->matrix.m[0][2] = disp_refr->hor_res;
                    layer->matrix.m[1][0] = 1;
                    layer->matrix.m[1][1] = 0;
                    layer->matrix.m[1][2] = 0;
                    break;

                default:
                    LV_LOG_WARN("Invalid rotation: %d", rotation);
                    break;
            }
        }
    }
#endif /* LV_DRAW_TRANSFORM_USE_MATRIX, LV_DRAW_TRANSFORM_USE_MATRIX 条件编译结束 */

    /* In single buffered mode wait here until the buffer is freed.
     * Else we would draw into the buffer while it's still being transferred to the display*/
    /* 在单缓冲模式下,在此等待直到缓冲区被释放。
     * 否则会在缓冲区仍向显示器传输数据时绘制到其中*/
    if(!lv_display_is_double_buffered(disp_refr)) {
        wait_for_flushing(disp_refr);
    }
    /*If the screen is transparent initialize it when the flushing is ready*/
    /*若屏幕是透明的,则在刷新就绪时初始化它*/
    if(lv_color_format_has_alpha(disp_refr->color_format)) {
#if LV_USE_DRAW_OPENGLES
        lv_layer_t * clear_target_layer = disp_refr->layer_head ? disp_refr->layer_head : layer;
        /* TODO: this driver-specific branch is a temporary workaround.
         * The proper fix may be a generic per-draw-unit clear callback (e.g.
         * a `clear_area_cb` on `lv_draw_unit_t`) so `lv_refr` can just dispatch
         * the right clear function. LVGL does not currently expose that hook now.
         * This is a special-case for Draw_OpenGLES to fix issue #9912 (PR #9987).
         */
        /* TODO: 这个针对具体驱动的分支只是临时解决办法。
         * 正确的修复方案可能是提供通用的逐绘制单元清屏回调(例如
         * lv_draw_unit_t 上的 clear_area_cb),这样 lv_refr 只需分发
         * 正确的清屏函数。目前 LVGL 尚未公开该钩子。
         * 这是为修复 issue #9912(PR #9987)而针对 Draw_OpenGLES 的特殊处理。
         */
        /*With Draw_OpenGLES the layer's draw_buf is a dummy CPU buffer and the
         *real pixels live in a GL texture. Clearing the CPU buffer is a no-op
         *on the texture, so perform a GPU-side clear of the dirty area.
         *Key this off the refreshing display's real backing layer instead of
         *the current layer, because tiled rendering can use temporary tile
         *layers with NULL user_data.*/
        /*使用 Draw_OpenGLES 时,图层的 draw_buf 是虚拟的 CPU 缓冲区,
         *真正的像素存在于 GL 纹理中。清除 CPU 缓冲区对纹理无任何作用,
         *因此需在 GPU 侧清除脏区域。
         *应基于正在刷新的显示器的真实后备图层而非当前图层来做此判断,
         *因为瓦片渲染可能使用 user_data 为 NULL 的临时瓦片图层。*/
        if(disp_refr->layer_head != NULL && disp_refr->layer_head->user_data != NULL) {
            lv_draw_opengles_clear_layer_area(clear_target_layer, &layer->_clip_area);
        }
        else
#endif
        {
            lv_area_t clear_area = layer->_clip_area;
            lv_area_move(&clear_area, -layer->buf_area.x1, -layer->buf_area.y1);
            lv_draw_buf_clear(layer->draw_buf, &clear_area);
        }
    }

    lv_obj_t * top_act_scr = NULL;
    lv_obj_t * top_prev_scr = NULL;

    /*Get the most top object which is not covered by others*/
    /*获取未被其他对象覆盖的最顶层对象*/
    top_act_scr = lv_refr_get_top_obj(&layer->_clip_area, lv_display_get_screen_active(disp_refr));
    if(disp_refr->prev_scr) {
        top_prev_scr = lv_refr_get_top_obj(&layer->_clip_area, disp_refr->prev_scr);
    }

    /*Draw a bottom layer background if there is no top object*/
    /*若无顶层对象,则绘制底层背景*/
    if(top_act_scr == NULL && top_prev_scr == NULL) {
        refr_obj_and_children(layer, lv_display_get_layer_bottom(disp_refr));
    }

    if(disp_refr->draw_prev_over_act) {
        if(top_act_scr == NULL) top_act_scr = disp_refr->act_scr;
        refr_obj_and_children(layer, top_act_scr);

        /*Refresh the previous screen if any*/
        /*如有前一个屏幕,则刷新它*/
        if(disp_refr->prev_scr) {
            if(top_prev_scr == NULL) top_prev_scr = disp_refr->prev_scr;
            refr_obj_and_children(layer, top_prev_scr);
        }
    }
    else {
        /*Refresh the previous screen if any*/
        /*如有前一个屏幕,则刷新它*/
        if(disp_refr->prev_scr) {
            if(top_prev_scr == NULL) top_prev_scr = disp_refr->prev_scr;
            refr_obj_and_children(layer, top_prev_scr);
        }

        if(top_act_scr == NULL) top_act_scr = disp_refr->act_scr;
        refr_obj_and_children(layer, top_act_scr);
    }

    /*Also refresh top and sys layer unconditionally*/
    /*同时无条件刷新顶层和系统层*/
    refr_obj_and_children(layer, lv_display_get_layer_top(disp_refr));
    refr_obj_and_children(layer, lv_display_get_layer_sys(disp_refr));

    LV_PROFILER_REFR_END;
}

/**
 * Make the refreshing from an object. Draw all its children and the youngers too.
 * 从某个对象开始刷新,绘制它的所有子对象以及更年轻的兄弟对象。
 * @param top_p pointer to an objects. Start the drawing from it.
 * @param top_p 指向一个对象的指针,绘制从它开始。
 * @param mask_p pointer to an area, the objects will be drawn only here
 * @param mask_p 指向某个区域的指针,对象仅绘制在该区域内
 */
static void refr_obj_and_children(lv_layer_t * layer, lv_obj_t * top_obj)
{
    /*Normally always will be a top_obj (at least the screen)
     *but in special cases (e.g. if the screen has alpha) it won't.
     *In this case use the screen directly*/
    /*通常总会有一个 top_obj(至少是屏幕),
     *但在特殊情况下(例如屏幕带透明度)可能没有。
     *此时直接使用屏幕*/
    if(top_obj == NULL) top_obj = lv_display_get_screen_active(disp_refr);
    if(top_obj == NULL) return;  /*Shouldn't happen, 理论上不会发生*/

    LV_PROFILER_REFR_BEGIN;
    /*Draw the 'younger' sibling objects because they can be on top_obj*/
    /*绘制“更年轻”的兄弟对象,因为它们可能位于 top_obj 之上*/
    lv_obj_t * parent;
    lv_obj_t * border_p = top_obj;

    parent = lv_obj_get_parent(top_obj);

    /*Calculate the recolor before the parent*/
    /*在父对象之前计算重新着色*/
    if(parent) {
        layer->recolor = lv_obj_get_style_recolor_recursive(parent, LV_PART_MAIN);
    }

    /*Refresh the top object and its children*/
    /*刷新顶层对象及其子对象*/
    lv_obj_refr(layer, top_obj);

    /*Do until not reach the screen*/
    /*一直执行,直到到达屏幕层*/
    while(parent != NULL) {
        bool go = false;
        uint32_t i;
        uint32_t child_cnt = lv_obj_get_child_count(parent);
        for(i = 0; i < child_cnt; i++) {
            lv_obj_t * child = parent->spec_attr->children[i];
            if(!go) {
                if(child == border_p) go = true;
            }
            else {
                /*Refresh the objects*/
                /*刷新这些对象*/
                lv_obj_refr(layer, child);
            }
        }

        /*Call the post draw function of the parents of the to object*/
        /*调用顶层对象父对象的绘制后函数*/
        lv_obj_send_event(parent, LV_EVENT_DRAW_POST_BEGIN, (void *)layer);
        lv_obj_send_event(parent, LV_EVENT_DRAW_POST, (void *)layer);
        lv_obj_send_event(parent, LV_EVENT_DRAW_POST_END, (void *)layer);

        /*The new border will be the last parents,
         *so the 'younger' brothers of parent will be refreshed*/
        /*新的边界将是上一级父对象,
         *因此将刷新该父对象的“更年轻”的兄弟对象*/
        border_p = parent;
        /*Go a level deeper*/
        /*向上一级移动*/
        parent = lv_obj_get_parent(parent);
    }
    LV_PROFILER_REFR_END;
}

/* 计算对象所在图层需要更新的区域 */
static lv_result_t layer_get_area(lv_layer_t * layer, lv_obj_t * obj, lv_layer_type_t layer_type,
                                  lv_area_t * layer_area_out, lv_area_t * obj_draw_size_out)
{
    int32_t ext_draw_size = lv_obj_get_ext_draw_size(obj);
    lv_obj_get_coords(obj, obj_draw_size_out);
    lv_area_increase(obj_draw_size_out, ext_draw_size, ext_draw_size);

    if(layer_type == LV_LAYER_TYPE_TRANSFORM) {
        /*Get the transformed area and clip it to the current clip area.
         *This area needs to be updated on the screen.*/
        /*获取变换后的区域,并用当前裁剪区域裁剪它。
         *该区域需要在屏幕上更新。*/
        lv_area_t clip_coords_for_obj;
        lv_area_t tranf_coords = *obj_draw_size_out;
        lv_obj_get_transformed_area(obj, &tranf_coords, LV_OBJ_POINT_TRANSFORM_FLAG_NONE);
        if(!lv_area_intersect(&clip_coords_for_obj, &layer->_clip_area, &tranf_coords)) {
            return LV_RESULT_INVALID;
        }

        /*Transform back (inverse) the transformed area.
         *It will tell which area of the non-transformed widget needs to be redrawn
         *in order to cover transformed area after transformation.*/
        /*对变换后的区域进行逆变换。
         *由此可知未变换部件上的哪个区域需要重绘,
         *以便在变换后覆盖变换后的区域。*/
        lv_area_t inverse_clip_coords_for_obj = clip_coords_for_obj;
        lv_obj_get_transformed_area(obj, &inverse_clip_coords_for_obj, LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE);
        if(!lv_area_intersect(&inverse_clip_coords_for_obj, &inverse_clip_coords_for_obj, obj_draw_size_out)) {
            return LV_RESULT_INVALID;
        }

        *layer_area_out = inverse_clip_coords_for_obj;
        lv_area_increase(layer_area_out, 5, 5); /*To avoid rounding error, 避免舍入误差*/
    }
    else if(layer_type == LV_LAYER_TYPE_SIMPLE) {
        lv_area_t clip_coords_for_obj;
        if(!lv_area_intersect(&clip_coords_for_obj, &layer->_clip_area, obj_draw_size_out)) {
            return LV_RESULT_INVALID;
        }
        *layer_area_out = clip_coords_for_obj;
    }
    else {
        LV_LOG_WARN("Unhandled layer type");
        return LV_RESULT_INVALID;
    }

    return LV_RESULT_OK;
}

/* 测试对象的某区域是否需要透明度通道 */
static bool alpha_test_area_on_obj(lv_obj_t * obj, const lv_area_t * area)
{
    /*Test for alpha by assuming there is no alpha. If it fails, fall back to rendering with alpha*/
    /*假设没有透明度来进行测试。若测试失败,则回退为带透明度渲染*/
    /*If the layer area is not fully on the object, it can't fully cover it*/
    /*若图层区域未完全位于对象之上,则对象无法完全覆盖它*/
    if(!lv_area_is_on(area, &obj->coords)) return true;

    lv_cover_check_info_t info;
    info.res = LV_COVER_RES_COVER;
    info.area = area;
    lv_obj_send_event(obj, LV_EVENT_COVER_CHECK, &info);
    if(info.res == LV_COVER_RES_COVER) return false;
    else return true;
}

#if LV_DRAW_TRANSFORM_USE_MATRIX

/* 获取对象的变换矩阵(优先使用自定义矩阵,否则由变换样式生成) */
static bool obj_get_matrix(lv_obj_t * obj, lv_matrix_t * matrix)
{
    lv_matrix_identity(matrix);

    const lv_matrix_t * obj_matrix = lv_obj_get_transform(obj);
    if(obj_matrix) {
        lv_matrix_translate(matrix, obj->coords.x1, obj->coords.y1);
        lv_matrix_multiply(matrix, obj_matrix);
        lv_matrix_translate(matrix, -obj->coords.x1, -obj->coords.y1);
        return true;
    }

    lv_point_t pivot = {
        .x = lv_obj_get_style_transform_pivot_x(obj, LV_PART_MAIN),
        .y = lv_obj_get_style_transform_pivot_y(obj, LV_PART_MAIN)
    };

    pivot.x = obj->coords.x1 + lv_pct_to_px(pivot.x, lv_area_get_width(&obj->coords));
    pivot.y = obj->coords.y1 + lv_pct_to_px(pivot.y, lv_area_get_height(&obj->coords));

    int32_t rotation = lv_obj_get_style_transform_rotation(obj, LV_PART_MAIN);
    int32_t scale_x = lv_obj_get_style_transform_scale_x(obj, LV_PART_MAIN);
    int32_t scale_y = lv_obj_get_style_transform_scale_y(obj, LV_PART_MAIN);
    int32_t skew_x = lv_obj_get_style_transform_skew_x(obj, LV_PART_MAIN);
    int32_t skew_y = lv_obj_get_style_transform_skew_y(obj, LV_PART_MAIN);

    if(scale_x <= 0 || scale_y <= 0) {
        /* NOT draw if scale is negative or zero */
        /* 若缩放为负值或零则不绘制 */
        return false;
    }

    /* generate the obj matrix */
    /* 生成对象矩阵 */
    lv_matrix_translate(matrix, pivot.x, pivot.y);
    if(rotation != 0) {
        lv_matrix_rotate(matrix, rotation * 0.1f);
    }

    if(scale_x != LV_SCALE_NONE || scale_y != LV_SCALE_NONE) {
        lv_matrix_scale(
            matrix,
            (float)scale_x / LV_SCALE_NONE,
            (float)scale_y / LV_SCALE_NONE
        );
    }

    if(skew_x != 0 || skew_y != 0) {
        lv_matrix_skew(matrix, skew_x, skew_y);
    }

    lv_matrix_translate(matrix, -pivot.x, -pivot.y);
    return true;
}

/* 使用矩阵变换方式重绘对象(将变换矩阵应用到图层上) */
static void refr_obj_matrix(lv_layer_t * layer, lv_obj_t * obj)
{
    LV_PROFILER_REFR_BEGIN;
    lv_matrix_t obj_matrix;
    if(!obj_get_matrix(obj, &obj_matrix)) {
        /* NOT draw if obj matrix is not available */
        /* 若对象矩阵不可用则不绘制 */
        LV_PROFILER_REFR_END;
        return;
    }

    lv_matrix_t matrix_inv;
    if(!lv_matrix_inverse(&matrix_inv, &obj_matrix)) {
        /* NOT draw if matrix is not invertible */
        /* 若矩阵不可逆则不绘制 */
        LV_PROFILER_REFR_END;
        return;
    }

    /* save original matrix */
    /* 保存原始矩阵 */
    lv_matrix_t ori_matrix = layer->matrix;

    /* apply the obj matrix */
    /* 应用对象矩阵 */
    lv_matrix_multiply(&layer->matrix, &obj_matrix);

    /* calculate clip area without transform */
    /* 计算未变换的裁剪区域 */
    lv_area_t clip_area = layer->_clip_area;
    lv_area_t clip_area_ori = layer->_clip_area;
    clip_area = lv_matrix_transform_area(&matrix_inv, &clip_area);

    /* increase the clip area by 1 pixel to avoid rounding errors */
    /* 将裁剪区域扩大 1 像素以避免舍入误差 */
    if(!lv_matrix_is_identity_or_translation(&obj_matrix)) {
        lv_area_increase(&clip_area, 1, 1);
    }

    layer->_clip_area = clip_area;

    /* redraw obj */
    /* 重绘对象 */
    lv_obj_redraw(layer, obj);

    /* restore original matrix */
    /* 恢复原始矩阵 */
    layer->matrix = ori_matrix;
    /* restore clip area */
    /* 恢复裁剪区域 */
    layer->_clip_area = clip_area_ori;
    LV_PROFILER_REFR_END;
}

/* 检查对象的变换区域是否超出当前裁剪区域 */
static bool refr_check_obj_clip_overflow(lv_layer_t * layer, lv_obj_t * obj)
{
    if(lv_obj_get_style_transform_rotation(obj, LV_PART_MAIN) == 0) {
        return false;
    }

    /*Truncate the area to the object*/
    /*将区域截断到对象范围*/
    lv_area_t obj_coords;
    int32_t ext_size = lv_obj_get_ext_draw_size(obj);
    lv_area_copy(&obj_coords, &obj->coords);
    lv_area_increase(&obj_coords, ext_size, ext_size);

    lv_obj_get_transformed_area(obj, &obj_coords, LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE);

    lv_area_t clip_coords_for_obj;
    if(!lv_area_intersect(&clip_coords_for_obj, &layer->_clip_area, &obj_coords)) {
        return true;
    }

    bool has_clip = lv_memcmp(&clip_coords_for_obj, &obj_coords, sizeof(lv_area_t)) != 0;
    return has_clip;
}

#endif /* LV_DRAW_TRANSFORM_USE_MATRIX, LV_DRAW_TRANSFORM_USE_MATRIX 条件编译结束 */

/* 根据绘制缓冲区大小计算一屏可渲染的最大行数 */
static uint32_t get_max_row(lv_display_t * disp, int32_t area_w, int32_t area_h)
{
    lv_color_format_t cf = disp->color_format;
    uint32_t stride = lv_draw_buf_width_to_stride(area_w, cf);
    uint32_t overhead = LV_COLOR_INDEXED_PALETTE_SIZE(cf) * sizeof(lv_color32_t);

    if(stride == 0) {
        LV_LOG_WARN("Invalid stride. Value is 0");
        return 0;
    }

    int32_t max_row = (uint32_t)(disp->buf_act->data_size - overhead) / stride;

    if(max_row > area_h) max_row = area_h;

    /*Round down the lines of draw_buf if rounding is added*/
    /*若启用取整处理,则向下取整 draw_buf 的行数*/
    lv_area_t tmp;
    tmp.x1 = 0;
    tmp.x2 = 0;
    tmp.y1 = 0;

    int32_t h_tmp = max_row;
    do {
        tmp.y2 = h_tmp - 1;
        lv_display_send_event(disp_refr, LV_EVENT_INVALIDATE_AREA, &tmp);

        /*If this height fits into `max_row` then fine*/
        /*若该高度能放进 max_row 则没问题*/
        if(lv_area_get_height(&tmp) <= max_row) break;

        /*Decrement the height of the area until it fits into `max_row` after rounding*/
        /*递减区域高度,直到取整后能放入 max_row*/
        h_tmp--;
    } while(h_tmp > 0);

    if(h_tmp <= 0) {
        LV_LOG_WARN("Can't set draw_buf height using the round function. (Wrong round_cb or too "
                    "small draw_buf)");
        return 0;
    }
    else {
        max_row = tmp.y2 + 1;
    }

    return max_row;
}

/**
 * Flush the content of the draw buffer
 * 将绘制缓冲区的内容刷到显示器
 */
static void draw_buf_flush(lv_display_t * disp)
{
    /*Flush the rendered content to the display*/
    /*将渲染好的内容刷到显示器*/
    lv_layer_t * layer = disp->layer_head;

    while(layer->draw_task_head) {
        lv_draw_dispatch_wait_for_request();
        lv_draw_dispatch();
    }

    /* In double buffered mode wait until the other buffer is freed
     * and driver is ready to receive the new buffer.
     * If we need to wait here it means that the content of one buffer is being sent to display
     * and other buffer already contains the new rendered image. */
    /* 在双缓冲模式下,等待另一缓冲区被释放
     * 且驱动准备好接收新缓冲区。
     * 若需在此等待,说明一个缓冲区的内容正被送往显示器,
     * 而另一个缓冲区已包含新渲染的图像。 */
    if(lv_display_is_double_buffered(disp)) {
        wait_for_flushing(disp_refr);
    }

    disp->flushing = 1;

    if(disp->last_area && disp->last_part) disp->flushing_last = 1;
    else disp->flushing_last = 0;

    bool flushing_last = disp->flushing_last;

    if(disp->flush_cb) {
        call_flush_cb(disp, &disp->refreshed_area, layer->draw_buf->data);
    }
    /*If there are 2 buffers swap them. With direct mode swap only on the last area*/
    /*若有两个缓冲区则交换它们。直接模式下仅在最后一个区域时交换*/
    if(lv_display_is_double_buffered(disp) && (disp->render_mode != LV_DISPLAY_RENDER_MODE_DIRECT || flushing_last)) {
        if(disp->buf_act == disp->buf_1) {
            disp->buf_act = disp->buf_2;
        }
        else if(disp->buf_act == disp->buf_2) {
            disp->buf_act = disp->buf_3 ? disp->buf_3 : disp->buf_1;
        }
        else {
            disp->buf_act = disp->buf_1;
        }
    }
}

/* 调用显示器的 flush_cb 回调,把像素数据交给驱动 */
static void call_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    LV_PROFILER_REFR_BEGIN;
    LV_TRACE_REFR("Calling flush_cb on (%d;%d)(%d;%d) area with %p image pointer",
                  (int)area->x1, (int)area->y1, (int)area->x2, (int)area->y2, (void *)px_map);

    lv_area_t offset_area = {
        .x1 = area->x1 + disp->offset_x,
        .y1 = area->y1 + disp->offset_y,
        .x2 = area->x2 + disp->offset_x,
        .y2 = area->y2 + disp->offset_y
    };

    lv_display_send_event(disp, LV_EVENT_FLUSH_START, &offset_area);

    /*
     * For backward compatibility support LV_COLOR_16_SWAP (from v8)
     * TODO:(v10) remove this
     * 为向后兼容而支持 LV_COLOR_16_SWAP(来自 v8)
     * TODO:(v10) 移除这段代码
     */
#if defined(LV_COLOR_16_SWAP) && LV_COLOR_16_SWAP
    if(lv_display_get_render_mode(disp) == LV_DISPLAY_RENDER_MODE_DIRECT) {
        uint16_t * fb = (uint16_t *)px_map;
        uint32_t stride_px = disp->buf_act->header.stride / 2; /* RGB565: 2 bytes/px, RGB565 格式:每像素 2 字节 */
        int32_t w = lv_area_get_width(area);
        int32_t h = lv_area_get_height(area);
        for(int32_t y = 0; y < h; y++) {
            lv_draw_sw_rgb565_swap(fb + (uint32_t)(area->y1 + y) * stride_px + area->x1, w);
        }
    }
    else {
        lv_draw_sw_rgb565_swap(px_map, lv_area_get_size(&offset_area));
    }
#endif

    disp->flush_cb(disp, &offset_area, px_map);
    lv_display_send_event(disp, LV_EVENT_FLUSH_FINISH, &offset_area);

    LV_PROFILER_REFR_END;
}

/* 等待显示器刷新(flush)完成 */
static void wait_for_flushing(lv_display_t * disp)
{
    LV_PROFILER_REFR_BEGIN;
    LV_LOG_TRACE("begin");

    lv_display_send_event(disp, LV_EVENT_FLUSH_WAIT_START, NULL);

    if(disp->flush_wait_cb) {
        if(disp->flushing) {
            disp->flush_wait_cb(disp);
            disp->flushing = 0;
        }
    }
    else {
        while(disp->flushing);
    }
    disp->flushing_last = 0;

    lv_display_send_event(disp, LV_EVENT_FLUSH_WAIT_FINISH, NULL);

    LV_LOG_TRACE("end");
    LV_PROFILER_REFR_END;
}

/* 调用显示器的 sync_cb 回调同步指定区域 */
static void call_sync_cb(lv_display_t * disp, const lv_area_t * area)
{
    LV_PROFILER_REFR_BEGIN;

    /* Apply display offsets to the sync area for consistency with flush_cb */
    /* 对同步区域应用显示器偏移,以与 flush_cb 保持一致 */
    lv_area_t offset_area = *area;
    offset_area.x1 += disp->offset_x;
    offset_area.x2 += disp->offset_x;
    offset_area.y1 += disp->offset_y;
    offset_area.y2 += disp->offset_y;

    LV_TRACE_REFR("Calling sync_cb on (%d;%d)(%d;%d) area",
                  (int)offset_area.x1, (int)offset_area.y1,
                  (int)offset_area.x2, (int)offset_area.y2);

    lv_display_send_event(disp, LV_EVENT_SYNC_START, (void *)&offset_area);

    disp->sync_cb(disp, &offset_area);

    lv_display_send_event(disp, LV_EVENT_SYNC_FINISH, (void *)&offset_area);

    LV_PROFILER_REFR_END;
}

/* 等待显示器同步(sync)完成 */
static void wait_for_syncing(lv_display_t * disp)
{
    LV_PROFILER_REFR_BEGIN;
    LV_LOG_TRACE("begin");

    lv_display_send_event(disp, LV_EVENT_SYNC_WAIT_START, NULL);

    if(disp->sync_wait_cb) {
        if(disp->syncing) {
            disp->sync_wait_cb(disp);
            disp->syncing = 0;
        }
    }
    else {
        while(disp->syncing);
    }
    disp->syncing_last = 0;

    lv_display_send_event(disp, LV_EVENT_SYNC_WAIT_FINISH, NULL);

    LV_LOG_TRACE("end");
    LV_PROFILER_REFR_END;
}
