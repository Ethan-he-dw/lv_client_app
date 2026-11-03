/**
 * @file lv_obj_scroll.c
 * @brief 对象滚动管理模块：实现对象的滚动、滚动动画与滚动条绘制等功能。
 *
 */

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "lv_obj_scroll_private.h"
#include "../lvgl_public.h"
#include "../misc/lv_anim_private.h"
#include "lv_obj_private.h"
#include "../indev/lv_indev_scroll.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/
#define MY_CLASS (&lv_obj_class)
#ifndef SCROLL_ANIM_TIME_MIN
    #define SCROLL_ANIM_TIME_MIN    200    /*ms，毫秒*/
#endif
#ifndef SCROLL_ANIM_TIME_MAX
    #define SCROLL_ANIM_TIME_MAX    400    /*ms，毫秒*/
#endif
#define SCROLLBAR_MIN_SIZE      (LV_DPX(10))

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 *  GLOBAL PROTOTYPES
 *  全局函数原型
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数原型
 **********************/
static void scroll_x_anim(void * obj, int32_t v);
static void scroll_y_anim(void * obj, int32_t v);
static void scroll_end_cb(lv_anim_t * a);
static void scroll_area_into_view(const lv_area_t * area, lv_obj_t * child, lv_point_t * scroll_value,
                                  lv_anim_enable_t anim_en);

/**********************
 *  STATIC VARIABLES
 *  静态变量
 **********************/

/**********************
 *      MACROS
 *      宏
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 *   全局函数
 **********************/

/*=====================
 * Setter functions
 * 设置器函数
 *====================*/

/* 设置对象滚动条的显示模式 */
void lv_obj_set_scrollbar_mode(lv_obj_t * obj, lv_scrollbar_mode_t mode)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    if(!lv_obj_allocate_spec_attr(obj)) {
        return;
    }

    if(obj->spec_attr->scrollbar_mode == mode) return;
    obj->spec_attr->scrollbar_mode = mode;
    lv_obj_invalidate(obj);
}

/* 设置对象允许的滚动方向 */
void lv_obj_set_scroll_dir(lv_obj_t * obj, lv_dir_t dir)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    if(!lv_obj_allocate_spec_attr(obj)) {
        return;
    }

    if(dir != obj->spec_attr->scroll_dir) {
        obj->spec_attr->scroll_dir = dir;
    }
}

/* 设置对象水平方向的滚动吸附方式 */
void lv_obj_set_scroll_snap_x(lv_obj_t * obj, lv_scroll_snap_t align)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    if(!lv_obj_allocate_spec_attr(obj)) {
        return;
    }
    obj->spec_attr->scroll_snap_x = align;
}

/* 设置对象垂直方向的滚动吸附方式 */
void lv_obj_set_scroll_snap_y(lv_obj_t * obj, lv_scroll_snap_t align)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    if(!lv_obj_allocate_spec_attr(obj)) {
        return;
    }
    obj->spec_attr->scroll_snap_y = align;
}

/*=====================
 * Getter functions
 * 获取器函数
 *====================*/

/* 获取对象滚动条的显示模式 */
lv_scrollbar_mode_t lv_obj_get_scrollbar_mode(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return LV_SCROLLBAR_MODE_OFF);

    if(obj->spec_attr) return (lv_scrollbar_mode_t) obj->spec_attr->scrollbar_mode;
    else return LV_SCROLLBAR_MODE_AUTO;
}

/* 获取对象允许的滚动方向 */
lv_dir_t lv_obj_get_scroll_dir(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return LV_DIR_NONE);

    if(obj->spec_attr) return (lv_dir_t) obj->spec_attr->scroll_dir;
    else return LV_DIR_ALL;
}

/* 获取对象水平方向的滚动吸附方式 */
lv_scroll_snap_t lv_obj_get_scroll_snap_x(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return LV_SCROLL_SNAP_NONE);

    if(obj->spec_attr) return (lv_scroll_snap_t) obj->spec_attr->scroll_snap_x;
    else return LV_SCROLL_SNAP_NONE;
}

/* 获取对象垂直方向的滚动吸附方式 */
lv_scroll_snap_t lv_obj_get_scroll_snap_y(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return LV_SCROLL_SNAP_NONE);

    if(obj->spec_attr) return (lv_scroll_snap_t) obj->spec_attr->scroll_snap_y;
    else return LV_SCROLL_SNAP_NONE;
}

/* 获取对象当前的水平滚动位置 */
int32_t lv_obj_get_scroll_x(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);

    if(obj->spec_attr == NULL) return 0;
    return -obj->spec_attr->scroll.x;
}

/* 获取对象当前的垂直滚动位置 */
int32_t lv_obj_get_scroll_y(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);

    if(obj->spec_attr == NULL) return 0;
    return -obj->spec_attr->scroll.y;
}

/* 获取对象向上还能滚动的距离 */
int32_t lv_obj_get_scroll_top(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);

    if(obj->spec_attr == NULL) return 0;
    return -obj->spec_attr->scroll.y;
}

/* 获取对象向下还能滚动的距离 */
int32_t lv_obj_get_scroll_bottom(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);

    int32_t child_res = LV_COORD_MIN;
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_count(obj);
    for(i = 0; i < child_cnt; i++) {
        const lv_obj_t * child = obj->spec_attr->children[i];
        if((lv_obj_is_hidden(child) || lv_obj_is_floating(child))) continue;

        int32_t tmp_y = child->coords.y2 + lv_obj_get_style_margin_bottom(child, LV_PART_MAIN);
        child_res = LV_MAX(child_res, tmp_y);
    }

    int32_t space_top = lv_obj_get_style_space_top(obj, LV_PART_MAIN);
    int32_t space_bottom = lv_obj_get_style_space_bottom(obj, LV_PART_MAIN);

    if(child_res != LV_COORD_MIN) {
        child_res -= (obj->coords.y2 - space_bottom);
    }

    int32_t self_h = lv_obj_get_self_height(obj);
    self_h = self_h - (lv_obj_get_height(obj) - space_top - space_bottom);
    self_h -= lv_obj_get_scroll_y(obj);
    return LV_MAX(child_res, self_h);
}

/* 获取对象向左还能滚动的距离 */
int32_t lv_obj_get_scroll_left(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);

    /*Normally can't scroll the object out on the left.
     *So simply use the current scroll position as "left size"
     *通常情况下无法向左滚出对象，因此直接用当前的滚动位置作为"左侧大小"*/
    if(lv_obj_get_style_base_dir(obj, LV_PART_MAIN) != LV_BASE_DIR_RTL) {
        if(obj->spec_attr == NULL) return 0;
        return -obj->spec_attr->scroll.x;
    }

    /*With RTL base direction scrolling the left is normal so find the left most coordinate*/
    /*在 RTL 基准方向下向左滚动是正常的，因此找出最左侧的坐标*/
    int32_t space_right = lv_obj_get_style_space_right(obj, LV_PART_MAIN);
    int32_t space_left = lv_obj_get_style_space_left(obj, LV_PART_MAIN);

    int32_t child_res = 0;

    uint32_t i;
    int32_t x1 = LV_COORD_MAX;
    uint32_t child_cnt = lv_obj_get_child_count(obj);
    for(i = 0; i < child_cnt; i++) {
        const lv_obj_t * child = obj->spec_attr->children[i];
        if((lv_obj_is_hidden(child) || lv_obj_is_floating(child))) continue;

        int32_t tmp_x = child->coords.x1 - lv_obj_get_style_margin_left(child, LV_PART_MAIN);
        x1 = LV_MIN(x1, tmp_x);
    }

    if(x1 != LV_COORD_MAX) {
        child_res = x1;
        child_res = (obj->coords.x1 + space_left) - child_res;
    }
    else {
        child_res = LV_COORD_MIN;
    }

    int32_t self_w = lv_obj_get_self_width(obj);
    self_w = self_w - (lv_obj_get_width(obj) - space_right - space_left);
    self_w += lv_obj_get_scroll_x(obj);

    return LV_MAX(child_res, self_w);
}

/* 获取对象向右还能滚动的距离 */
int32_t lv_obj_get_scroll_right(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);

    /*With RTL base dir can't scroll to the object out on the right.
     *So simply use the current scroll position as "right size"
     *在 RTL 基准方向下无法向右滚出对象，因此直接用当前的滚动位置作为"右侧大小"*/
    if(lv_obj_get_style_base_dir(obj, LV_PART_MAIN) == LV_BASE_DIR_RTL) {
        if(obj->spec_attr == NULL) return 0;
        return obj->spec_attr->scroll.x;
    }

    /*With other base direction (LTR) scrolling to the right is normal so find the right most coordinate*/
    /*在其它基准方向（LTR）下向右滚动是正常的，因此找出最右侧的坐标*/
    int32_t child_res = LV_COORD_MIN;
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_count(obj);
    for(i = 0; i < child_cnt; i++) {
        const lv_obj_t * child = obj->spec_attr->children[i];
        if((lv_obj_is_hidden(child) || lv_obj_is_floating(child))) continue;

        int32_t tmp_x = child->coords.x2 + lv_obj_get_style_margin_right(child, LV_PART_MAIN);
        child_res = LV_MAX(child_res, tmp_x);
    }

    int32_t space_right = lv_obj_get_style_space_right(obj, LV_PART_MAIN);
    int32_t space_left = lv_obj_get_style_space_left(obj, LV_PART_MAIN);

    if(child_res != LV_COORD_MIN) {
        child_res -= (obj->coords.x2 - space_right);
    }

    int32_t self_w;
    self_w = lv_obj_get_self_width(obj);
    self_w = self_w - (lv_obj_get_width(obj) - space_right - space_left);
    self_w -= lv_obj_get_scroll_x(obj);
    return LV_MAX(child_res, self_w);
}

/* 获取滚动动画结束后对象最终的滚动位置 */
void lv_obj_get_scroll_end(lv_obj_t * obj, lv_point_t * end)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    LV_CHECK_ARG(end != NULL, return);

    lv_anim_t * a;
    a = lv_anim_get(obj, scroll_x_anim);
    end->x = a ? -a->end_value : lv_obj_get_scroll_x(obj);

    a = lv_anim_get(obj, scroll_y_anim);
    end->y = a ? -a->end_value : lv_obj_get_scroll_y(obj);
}

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/* 将对象滚动指定的距离，并对滚动范围进行边界限制 */
void lv_obj_scroll_by_bounded(lv_obj_t * obj, int32_t dx, int32_t dy, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    if(dx == 0 && dy == 0) return;
    if(!lv_obj_is_scrollable(obj)) return;

    /*We need to know the final sizes for bound check*/
    /*我们需要知道最终尺寸以进行边界检查*/
    lv_obj_update_layout(obj);

    /*Don't let scroll more than naturally possible by the size of the content*/
    /*不要让滚动量超过内容尺寸自然允许的范围*/
    int32_t x_current = -lv_obj_get_scroll_x(obj);
    int32_t x_bounded = x_current + dx;

    if(lv_obj_get_style_base_dir(obj, LV_PART_MAIN) != LV_BASE_DIR_RTL) {
        if(x_bounded > 0) x_bounded = 0;
        if(x_bounded < 0) {
            int32_t  scroll_max = lv_obj_get_scroll_left(obj) + lv_obj_get_scroll_right(obj);
            if(scroll_max < 0) scroll_max = 0;

            if(x_bounded < -scroll_max) x_bounded = -scroll_max;
        }
    }
    else {
        if(x_bounded < 0) x_bounded = 0;
        if(x_bounded > 0) {
            int32_t  scroll_max = lv_obj_get_scroll_left(obj) + lv_obj_get_scroll_right(obj);
            if(scroll_max < 0) scroll_max = 0;

            if(x_bounded > scroll_max) x_bounded = scroll_max;
        }
    }

    /*Don't let scroll more than naturally possible by the size of the content*/
    /*不要让滚动量超过内容尺寸自然允许的范围*/
    int32_t y_current = -lv_obj_get_scroll_y(obj);
    int32_t y_bounded = y_current + dy;

    if(y_bounded > 0) y_bounded = 0;
    if(y_bounded < 0) {
        int32_t  scroll_max = lv_obj_get_scroll_top(obj) + lv_obj_get_scroll_bottom(obj);
        if(scroll_max < 0) scroll_max = 0;
        if(y_bounded < -scroll_max) y_bounded = -scroll_max;
    }

    dx = x_bounded - x_current;
    dy = y_bounded - y_current;
    if(dx || dy) {
        lv_obj_scroll_by(obj, dx, dy, anim_en);
    }
}

/* 将对象滚动指定的距离，可选择是否使用动画 */
void lv_obj_scroll_by(lv_obj_t * obj, int32_t dx, int32_t dy, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    if(dx == 0 && dy == 0) return;
    if(!lv_obj_is_scrollable(obj)) return;
    if(anim_en) {
        lv_display_t * d = lv_obj_get_display(obj);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, obj);
        lv_anim_set_deleted_cb(&a, scroll_end_cb);

        if(dx) {
            uint32_t t = lv_anim_speed_clamped((lv_display_get_horizontal_resolution(d)) >> 1, SCROLL_ANIM_TIME_MIN,
                                               SCROLL_ANIM_TIME_MAX);
            lv_anim_set_duration(&a, t);
            int32_t sx = lv_obj_get_scroll_x(obj);
            lv_anim_set_values(&a, -sx, -sx + dx);
            lv_anim_set_exec_cb(&a, scroll_x_anim);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

            lv_result_t res;
            res = lv_obj_send_event(obj, LV_EVENT_SCROLL_BEGIN, &a);
            if(res != LV_RESULT_OK) return;
            lv_anim_start(&a);
        }

        if(dy) {
            uint32_t t = lv_anim_speed_clamped((lv_display_get_vertical_resolution(d)) >> 1, SCROLL_ANIM_TIME_MIN,
                                               SCROLL_ANIM_TIME_MAX);
            lv_anim_set_duration(&a, t);
            int32_t sy = lv_obj_get_scroll_y(obj);
            lv_anim_set_values(&a, -sy, -sy + dy);
            lv_anim_set_exec_cb(&a,  scroll_y_anim);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

            lv_result_t res;
            res = lv_obj_send_event(obj, LV_EVENT_SCROLL_BEGIN, &a);
            if(res != LV_RESULT_OK) return;
            lv_anim_start(&a);
        }
    }
    else {
        /*Remove pending animations*/
        /*移除挂起的滚动动画*/
        lv_anim_delete(obj, scroll_y_anim);
        lv_anim_delete(obj, scroll_x_anim);

        lv_result_t res;
        res = lv_obj_send_event(obj, LV_EVENT_SCROLL_BEGIN, NULL);
        if(res != LV_RESULT_OK) return;

        res = lv_obj_scroll_by_raw(obj, dx, dy);
        if(res != LV_RESULT_OK) return;

        res = lv_obj_send_event(obj, LV_EVENT_SCROLL_END, NULL);
        if(res != LV_RESULT_OK) return;
    }
}

/* 将对象滚动到指定的位置，可选择是否使用动画 */
void lv_obj_scroll_to(lv_obj_t * obj, int32_t x, int32_t y, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    lv_obj_scroll_to_x(obj, x, anim_en);
    lv_obj_scroll_to_y(obj, y, anim_en);
}

/* 将对象水平滚动到指定位置，可选择是否使用动画 */
void lv_obj_scroll_to_x(lv_obj_t * obj, int32_t x, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    lv_anim_delete(obj, scroll_x_anim);

    int32_t scroll_x = lv_obj_get_scroll_x(obj);
    int32_t diff = -x + scroll_x;

    lv_obj_scroll_by_bounded(obj, diff, 0, anim_en);
}

/* 将对象垂直滚动到指定位置，可选择是否使用动画 */
void lv_obj_scroll_to_y(lv_obj_t * obj, int32_t y, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    lv_anim_delete(obj, scroll_y_anim);

    int32_t scroll_y = lv_obj_get_scroll_y(obj);
    int32_t diff = -y + scroll_y;

    lv_obj_scroll_by_bounded(obj, 0, diff, anim_en);
}

/* 将对象滚动到父对象的可视区域中 */
void lv_obj_scroll_to_view(lv_obj_t * obj, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    /*Be sure the screens layout is correct*/
    /*确保屏幕布局是正确的*/
    lv_obj_update_layout(obj);

    lv_point_t p = {0, 0};
    scroll_area_into_view(&obj->coords, obj, &p, anim_en);
}

/* 递归地将对象及其各级父对象滚动到可视区域中 */
void lv_obj_scroll_to_view_recursive(lv_obj_t * obj, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    /*Be sure the screens layout is correct*/
    /*确保屏幕布局是正确的*/
    lv_obj_update_layout(obj);

    lv_point_t p = {0, 0};
    lv_obj_t * child = obj;
    lv_obj_t * parent = lv_obj_get_parent(child);
    while(parent) {
        scroll_area_into_view(&obj->coords, child, &p, anim_en);
        child = parent;
        parent = lv_obj_get_parent(parent);
    }
}

/* 直接滚动对象，不做边界检查也不使用动画 */
lv_result_t lv_obj_scroll_by_raw(lv_obj_t * obj, int32_t x, int32_t y)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return LV_RESULT_INVALID);

    if(x == 0 && y == 0) return LV_RESULT_OK;
    if(!lv_obj_is_scrollable(obj)) return LV_RESULT_OK;

    if(!lv_obj_allocate_spec_attr(obj)) {
        return LV_RESULT_INVALID;
    }

    obj->spec_attr->scroll.x += x;
    obj->spec_attr->scroll.y += y;

    lv_obj_move_children_by(obj, x, y, true);
    lv_result_t res = lv_obj_send_event(obj, LV_EVENT_SCROLL, NULL);
    if(res != LV_RESULT_OK) return res;
    lv_obj_invalidate(obj);
    return LV_RESULT_OK;
}

/* 判断对象当前是否正在滚动 */
bool lv_obj_is_scrolling(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return false);

    lv_indev_t * indev = lv_indev_get_next(NULL);
    while(indev) {
        if(lv_indev_get_scroll_obj(indev) == obj) return true;
        indev = lv_indev_get_next(indev);
    }

    if(lv_anim_get((lv_obj_t *)obj, scroll_x_anim) != NULL ||
       lv_anim_get((lv_obj_t *)obj, scroll_y_anim) != NULL) {
        return true;
    }

    return false;
}

/* 停止对象正在进行的滚动动画 */
void lv_obj_stop_scroll_anim(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    lv_anim_delete((lv_obj_t *)obj, scroll_y_anim);
    lv_anim_delete((lv_obj_t *)obj, scroll_x_anim);
}

/* 根据吸附规则更新对象的滚动位置 */
void lv_obj_update_snap(lv_obj_t * obj, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    lv_obj_update_layout(obj);
    lv_point_t p;
    lv_indev_scroll_get_snap_dist(obj, &p);
    if(p.x == LV_COORD_MAX || p.x == LV_COORD_MIN) p.x = 0;
    if(p.y == LV_COORD_MAX || p.y == LV_COORD_MIN) p.y = 0;
    lv_obj_scroll_by(obj, p.x, p.y, anim_en);
}

/* 获取对象水平和垂直滚动条的区域 */
void lv_obj_get_scrollbar_area(lv_obj_t * obj, lv_area_t * hor_area, lv_area_t * ver_area)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    LV_CHECK_ARG(hor_area != NULL, return);
    LV_CHECK_ARG(ver_area != NULL, return);

    lv_area_set(hor_area, 0, 0, -1, -1);
    lv_area_set(ver_area, 0, 0, -1, -1);

    if(lv_obj_is_scrollable(obj) == false) return;

    lv_scrollbar_mode_t sm = lv_obj_get_scrollbar_mode(obj);
    if(sm == LV_SCROLLBAR_MODE_OFF)  return;

    /*If there is no indev scrolling this object but `mode==active` return*/
    /*如果没有输入设备正在滚动该对象且模式为 active，则直接返回*/
    lv_indev_t * indev = lv_indev_get_next(NULL);
    if(sm == LV_SCROLLBAR_MODE_ACTIVE) {
        while(indev) {
            if(lv_indev_get_scroll_obj(indev) == obj) break;
            indev = lv_indev_get_next(indev);
        }
        if(indev == NULL)  return;
    }

    int32_t st = lv_obj_get_scroll_top(obj);
    int32_t sb = lv_obj_get_scroll_bottom(obj);
    int32_t sl = lv_obj_get_scroll_left(obj);
    int32_t sr = lv_obj_get_scroll_right(obj);

    lv_dir_t dir = lv_obj_get_scroll_dir(obj);

    bool ver_draw = false;
    if((dir & LV_DIR_VER) &&
       ((sm == LV_SCROLLBAR_MODE_ON) ||
        (sm == LV_SCROLLBAR_MODE_AUTO && (st > 0 || sb > 0)) ||
        (sm == LV_SCROLLBAR_MODE_ACTIVE && lv_indev_get_scroll_dir(indev) == LV_DIR_VER))) {
        ver_draw = true;
    }

    bool hor_draw = false;
    if((dir & LV_DIR_HOR) &&
       ((sm == LV_SCROLLBAR_MODE_ON) ||
        (sm == LV_SCROLLBAR_MODE_AUTO && (sl > 0 || sr > 0)) ||
        (sm == LV_SCROLLBAR_MODE_ACTIVE && lv_indev_get_scroll_dir(indev) == LV_DIR_HOR))) {
        hor_draw = true;
    }

    if(!hor_draw && !ver_draw) return;

    bool rtl = lv_obj_get_style_base_dir(obj, LV_PART_SCROLLBAR) == LV_BASE_DIR_RTL;

    int32_t top_space = lv_obj_get_style_pad_top(obj, LV_PART_SCROLLBAR);
    int32_t bottom_space = lv_obj_get_style_pad_bottom(obj, LV_PART_SCROLLBAR);
    int32_t left_space = lv_obj_get_style_pad_left(obj, LV_PART_SCROLLBAR);
    int32_t right_space = lv_obj_get_style_pad_right(obj, LV_PART_SCROLLBAR);
    int32_t thickness = lv_obj_get_style_width(obj, LV_PART_SCROLLBAR);
    int32_t length = lv_obj_get_style_length(obj, LV_PART_SCROLLBAR);

    int32_t obj_h = lv_obj_get_height(obj);
    int32_t obj_w = lv_obj_get_width(obj);

    /*Space required for the vertical and horizontal scrollbars*/
    /*垂直和水平滚动条所需的空间*/
    int32_t ver_reg_space = ver_draw ? thickness : 0;
    int32_t hor_req_space = hor_draw ? thickness : 0;
    int32_t rem;

    if(lv_obj_get_style_bg_opa(obj, LV_PART_SCROLLBAR) <= LV_OPA_MIN &&
       lv_obj_get_style_border_opa(obj, LV_PART_SCROLLBAR) <= LV_OPA_MIN) {
        return;
    }

    /*Draw vertical scrollbar if the mode is ON or can be scrolled in this direction*/
    /*如果模式为 ON 或该方向可以滚动，则绘制垂直滚动条*/
    int32_t content_h = obj_h + st + sb;
    if(ver_draw && content_h) {
        ver_area->y1 = obj->coords.y1;
        ver_area->y2 = obj->coords.y2;
        if(rtl) {
            ver_area->x1 = obj->coords.x1 + left_space;
            ver_area->x2 = ver_area->x1 + thickness - 1;
        }
        else {
            ver_area->x2 = obj->coords.x2 - right_space;
            ver_area->x1 = ver_area->x2 - thickness + 1;
        }

        int32_t sb_h = ((obj_h - top_space - bottom_space - hor_req_space) * obj_h) / content_h;
        sb_h = LV_MAX(length > 0 ? length : sb_h, SCROLLBAR_MIN_SIZE); /*Style-defined size, calculated size, or minimum size，样式定义的大小、计算得到的大小或最小大小*/
        sb_h = LV_MIN(sb_h, obj_h); /*Limit scrollbar length to parent height，将滚动条长度限制在父对象高度内*/
        rem = (obj_h - top_space - bottom_space - hor_req_space) -
              sb_h;  /*Remaining size from the scrollbar track that is not the scrollbar itself，滚动条轨道中除去滚动条本身后剩余的空间*/
        int32_t scroll_h = content_h - obj_h; /*The size of the content which can be really scrolled，内容真正可以滚动的距离*/
        if(scroll_h <= 0) {
            ver_area->y1 = obj->coords.y1 + top_space;
            ver_area->y2 = obj->coords.y2 - bottom_space - hor_req_space - 1;
        }
        else {
            int32_t sb_y = (rem * sb) / scroll_h;
            sb_y = rem - sb_y;

            ver_area->y1 = obj->coords.y1 + sb_y + top_space;
            ver_area->y2 = ver_area->y1 + sb_h - 1;
            if(ver_area->y1 < obj->coords.y1 + top_space) {
                ver_area->y1 = obj->coords.y1 + top_space;
                if(ver_area->y1 + SCROLLBAR_MIN_SIZE > ver_area->y2) {
                    ver_area->y2 = ver_area->y1 + SCROLLBAR_MIN_SIZE;
                }
            }
            if(ver_area->y2 > obj->coords.y2 - hor_req_space - bottom_space) {
                ver_area->y2 = obj->coords.y2 - hor_req_space - bottom_space;
                if(ver_area->y2 - SCROLLBAR_MIN_SIZE < ver_area->y1) {
                    ver_area->y1 = ver_area->y2 - SCROLLBAR_MIN_SIZE;
                }
            }
        }
    }

    /*Draw horizontal scrollbar if the mode is ON or can be scrolled in this direction*/
    /*如果模式为 ON 或该方向可以滚动，则绘制水平滚动条*/
    int32_t content_w = obj_w + sl + sr;
    if(hor_draw && content_w) {
        hor_area->y2 = obj->coords.y2 - bottom_space;
        hor_area->y1 = hor_area->y2 - thickness + 1;
        hor_area->x1 = obj->coords.x1;
        hor_area->x2 = obj->coords.x2;

        int32_t sb_w = ((obj_w - left_space - right_space - ver_reg_space) * obj_w) / content_w;
        sb_w = LV_MAX(length > 0 ? length : sb_w, SCROLLBAR_MIN_SIZE); /*Style-defined size, calculated size, or minimum size，样式定义的大小、计算得到的大小或最小大小*/
        sb_w = LV_MIN(sb_w, obj_w); /*Limit scrollbar length to parent width，将滚动条长度限制在父对象宽度内*/
        rem = (obj_w - left_space - right_space - ver_reg_space) -
              sb_w;  /*Remaining size from the scrollbar track that is not the scrollbar itself，滚动条轨道中除去滚动条本身后剩余的空间*/
        int32_t scroll_w = content_w - obj_w; /*The size of the content which can be really scrolled，内容真正可以滚动的距离*/
        if(scroll_w <= 0) {
            if(rtl) {
                hor_area->x1 = obj->coords.x1 + left_space + ver_reg_space - 1;
                hor_area->x2 = obj->coords.x2 - right_space;
            }
            else {
                hor_area->x1 = obj->coords.x1 + left_space;
                hor_area->x2 = obj->coords.x2 - right_space - ver_reg_space - 1;
            }
        }
        else {
            int32_t sb_x = (rem * sr) / scroll_w;
            sb_x = rem - sb_x;

            if(rtl) {
                hor_area->x1 = obj->coords.x1 + sb_x + left_space + ver_reg_space;
                hor_area->x2 = hor_area->x1 + sb_w - 1;
                if(hor_area->x1 < obj->coords.x1 + left_space + ver_reg_space) {
                    hor_area->x1 = obj->coords.x1 + left_space + ver_reg_space;
                    if(hor_area->x1 + SCROLLBAR_MIN_SIZE > hor_area->x2) {
                        hor_area->x2 = hor_area->x1 + SCROLLBAR_MIN_SIZE;
                    }
                }
                if(hor_area->x2 > obj->coords.x2 - right_space) {
                    hor_area->x2 = obj->coords.x2 - right_space;
                    if(hor_area->x2 - SCROLLBAR_MIN_SIZE < hor_area->x1) {
                        hor_area->x1 = hor_area->x2 - SCROLLBAR_MIN_SIZE;
                    }
                }
            }
            else {
                hor_area->x1 = obj->coords.x1 + sb_x + left_space;
                hor_area->x2 = hor_area->x1 + sb_w - 1;
                if(hor_area->x1 < obj->coords.x1 + left_space) {
                    hor_area->x1 = obj->coords.x1 + left_space;
                    if(hor_area->x1 + SCROLLBAR_MIN_SIZE > hor_area->x2) {
                        hor_area->x2 = hor_area->x1 + SCROLLBAR_MIN_SIZE;
                    }
                }
                if(hor_area->x2 > obj->coords.x2 - ver_reg_space - right_space) {
                    hor_area->x2 = obj->coords.x2 - ver_reg_space - right_space;
                    if(hor_area->x2 - SCROLLBAR_MIN_SIZE < hor_area->x1) {
                        hor_area->x1 = hor_area->x2 - SCROLLBAR_MIN_SIZE;
                    }
                }
            }
        }
    }
}

/* 使对象的滚动条区域失效以触发重绘 */
void lv_obj_scrollbar_invalidate(lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    lv_area_t hor_area;
    lv_area_t ver_area;
    lv_obj_get_scrollbar_area(obj, &hor_area, &ver_area);

    if(lv_area_get_size(&hor_area) <= 0 && lv_area_get_size(&ver_area) <= 0) return;

    if(lv_area_get_size(&hor_area) > 0) lv_obj_invalidate_area(obj, &hor_area);
    if(lv_area_get_size(&ver_area) > 0) lv_obj_invalidate_area(obj, &ver_area);
}

/* 重新调整对象的滚动位置，避免内容滚入空白区域 */
void lv_obj_readjust_scroll(lv_obj_t * obj, lv_anim_enable_t anim_en)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    /*Be sure the bottom side is not remains scrolled in*/
    /*确保底部没有残留的滚入状态*/
    /*With snapping the content can't be scrolled in*/
    /*启用了吸附时内容不会被滚入*/
    if(lv_obj_get_scroll_snap_y(obj) == LV_SCROLL_SNAP_NONE) {
        int32_t st = lv_obj_get_scroll_top(obj);
        int32_t sb = lv_obj_get_scroll_bottom(obj);
        if(sb < 0 && st > 0) {
            sb = LV_MIN(st, -sb);
            lv_obj_scroll_by(obj, 0, sb, anim_en);
        }
    }

    if(lv_obj_get_scroll_snap_x(obj) == LV_SCROLL_SNAP_NONE) {
        int32_t sl = lv_obj_get_scroll_left(obj);
        int32_t sr = lv_obj_get_scroll_right(obj);
        if(lv_obj_get_style_base_dir(obj, LV_PART_MAIN) != LV_BASE_DIR_RTL) {
            /*Be sure the left side is not remains scrolled in*/
            /*确保左侧没有残留的滚入状态*/
            if(sr < 0 && sl > 0) {
                sr = LV_MIN(sl, -sr);
                lv_obj_scroll_by(obj, sr, 0, anim_en);
            }
        }
        else {
            /*Be sure the right side is not remains scrolled in*/
            /*确保右侧没有残留的滚入状态*/
            if(sl < 0 && sr > 0) {
                sr = LV_MIN(sr, -sl);
                lv_obj_scroll_by(obj, sl, 0, anim_en);
            }
        }
    }
}

/**********************
 *   STATIC FUNCTIONS
 *   静态函数
 **********************/

static void scroll_x_anim(void * obj, int32_t v)
{
    lv_obj_scroll_by_raw(obj, v + lv_obj_get_scroll_x(obj), 0);
}

static void scroll_y_anim(void * obj, int32_t v)
{
    lv_obj_scroll_by_raw(obj, 0, v + lv_obj_get_scroll_y(obj));
}

static void scroll_end_cb(lv_anim_t * a)
{
    /*Do not sent END event if there wasn't a BEGIN*/
    /*如果没有发送过 BEGIN 事件，就不要发送 END 事件*/
    if(a->start_cb_called) lv_obj_send_event(a->var, LV_EVENT_SCROLL_END, NULL);
}

static void scroll_area_into_view(const lv_area_t * area, lv_obj_t * child, lv_point_t * scroll_value,
                                  lv_anim_enable_t anim_en)
{
    lv_obj_t * parent = lv_obj_get_parent(child);
    if(!lv_obj_is_scrollable(parent)) return;

    lv_dir_t scroll_dir = lv_obj_get_scroll_dir(parent);
    int32_t snap_goal = 0;
    int32_t act = 0;
    const lv_area_t * area_tmp;

    int32_t y_scroll = 0;
    lv_scroll_snap_t snap_y = lv_obj_get_scroll_snap_y(parent);
    if(snap_y != LV_SCROLL_SNAP_NONE) area_tmp = &child->coords;
    else area_tmp = area;

    int32_t stop = lv_obj_get_style_space_top(parent, LV_PART_MAIN);
    int32_t sbottom = lv_obj_get_style_space_bottom(parent, LV_PART_MAIN);
    int32_t top_diff = parent->coords.y1 + stop - area_tmp->y1 - scroll_value->y;
    int32_t bottom_diff = -(parent->coords.y2 - sbottom - area_tmp->y2 - scroll_value->y);
    int32_t parent_h = lv_obj_get_height(parent) - stop - sbottom;
    if((top_diff >= 0 && bottom_diff >= 0)) y_scroll = 0;
    else if(top_diff > 0) {
        y_scroll = top_diff;
        /*Do not let scrolling in*/
        /*不允许滚入*/
        int32_t st = lv_obj_get_scroll_top(parent);
        if(st - y_scroll < 0) y_scroll = 0;
    }
    else if(bottom_diff > 0) {
        y_scroll = -bottom_diff;
        /*Do not let scrolling in*/
        /*不允许滚入*/
        int32_t sb = lv_obj_get_scroll_bottom(parent);
        if(sb + y_scroll < 0) y_scroll = 0;
    }

    switch(snap_y) {
        case LV_SCROLL_SNAP_START:
            snap_goal = parent->coords.y1 + stop;
            act = area_tmp->y1 + y_scroll;
            y_scroll += snap_goal - act;
            break;
        case LV_SCROLL_SNAP_END:
            snap_goal = parent->coords.y2 - sbottom;
            act = area_tmp->y2 + y_scroll;
            y_scroll += snap_goal - act;
            break;
        case LV_SCROLL_SNAP_CENTER:
            snap_goal = parent->coords.y1 + stop + parent_h / 2;
            act = lv_area_get_height(area_tmp) / 2 + area_tmp->y1 + y_scroll;
            y_scroll += snap_goal - act;
            break;
        case LV_SCROLL_SNAP_NONE:
            break;
    }

    int32_t x_scroll = 0;
    lv_scroll_snap_t snap_x = lv_obj_get_scroll_snap_x(parent);
    if(snap_x != LV_SCROLL_SNAP_NONE) area_tmp = &child->coords;
    else area_tmp = area;

    int32_t sleft = lv_obj_get_style_space_left(parent, LV_PART_MAIN);
    int32_t sright = lv_obj_get_style_space_right(parent, LV_PART_MAIN);
    int32_t left_diff = parent->coords.x1 + sleft - area_tmp->x1 - scroll_value->x;
    int32_t right_diff = -(parent->coords.x2 - sright - area_tmp->x2 - scroll_value->x);
    if((left_diff >= 0 && right_diff >= 0)) x_scroll = 0;
    else if(left_diff > 0) {
        x_scroll = left_diff;
        /*Do not let scrolling in*/
        /*不允许滚入*/
        int32_t sl = lv_obj_get_scroll_left(parent);
        if(sl - x_scroll < 0) x_scroll = 0;
    }
    else if(right_diff > 0) {
        x_scroll = -right_diff;
        /*Do not let scrolling in*/
        /*不允许滚入*/
        int32_t sr = lv_obj_get_scroll_right(parent);
        if(sr + x_scroll < 0) x_scroll = 0;
    }

    int32_t parent_w = lv_obj_get_width(parent) - sleft - sright;
    switch(snap_x) {
        case LV_SCROLL_SNAP_START:
            snap_goal = parent->coords.x1 + sleft;
            act = area_tmp->x1 + x_scroll;
            x_scroll += snap_goal - act;
            break;
        case LV_SCROLL_SNAP_END:
            snap_goal = parent->coords.x2 - sright;
            act = area_tmp->x2 + x_scroll;
            x_scroll += snap_goal - act;
            break;
        case LV_SCROLL_SNAP_CENTER:
            snap_goal = parent->coords.x1 + sleft + parent_w / 2;
            act = lv_area_get_width(area_tmp) / 2 + area_tmp->x1 + x_scroll;
            x_scroll += snap_goal - act;
            break;
        case LV_SCROLL_SNAP_NONE:
            break;
    }

    /*Remove any pending scroll animations.*/
    /*移除所有挂起的滚动动画。*/
    lv_anim_delete(parent, scroll_y_anim);
    lv_anim_delete(parent, scroll_x_anim);

    if((scroll_dir & LV_DIR_LEFT) == 0 && x_scroll < 0) x_scroll = 0;
    if((scroll_dir & LV_DIR_RIGHT) == 0 && x_scroll > 0) x_scroll = 0;
    if((scroll_dir & LV_DIR_TOP) == 0 && y_scroll < 0) y_scroll = 0;
    if((scroll_dir & LV_DIR_BOTTOM) == 0 && y_scroll > 0) y_scroll = 0;

    scroll_value->x += anim_en ? x_scroll : 0;
    scroll_value->y += anim_en ? y_scroll : 0;
    lv_obj_scroll_by(parent, x_scroll, y_scroll, anim_en);
}
