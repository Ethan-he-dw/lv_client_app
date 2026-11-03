/**
 * @file lv_obj_event_private.h
 * 对象事件私有模块：定义命中测试与覆盖检查等事件参数结构体。
 *
 */

#ifndef LV_OBJ_EVENT_PRIVATE_H
#define LV_OBJ_EVENT_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/

#include "../lvgl_public.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**
 * Used as the event parameter of ::LV_EVENT_HIT_TEST to check if an `point` can click the object or not.
 * 用作 ::LV_EVENT_HIT_TEST 的事件参数，用于检查某个 `point` 是否可以点击该对象。
 * `res` should be set like this:
 * `res` 应按如下方式设置：
 *   - If already set to `false` another event wants that point non clickable. If you want to respect it leave it as `false` or set `true` to overwrite it.
 *   - 如果已被设置为 `false`，说明另一事件希望该点不可点击。若要尊重该结果则保持 `false`，或设为 `true` 覆盖它。
 *   - If already set `true` and `point` shouldn't be clickable set to `false`
 *   - 如果已设置为 `true` 而 `point` 不应可点击，则设为 `false`
 *   - If already set to `true` you agree that `point` can click the object leave it as `true`
 *   - 如果已设置为 `true` 且你同意 `point` 可以点击对象，则保持 `true`
 */
struct _lv_hit_test_info_t {
    const lv_point_t * point;   /**< A point relative to screen to check if it can click the object or not
                                 * 相对于屏幕的坐标点，用于检查该点是否可以点击对象 */
    bool res;                   /**< true: `point` can click the object; false: it cannot
                                 * true: `point` 可以点击对象; false: 不可以 */
};

/**
 * Used as the event parameter of ::LV_EVENT_COVER_CHECK to check if an area is covered by the object or not.
 * 用作 ::LV_EVENT_COVER_CHECK 的事件参数，用于检查某个区域是否被该对象覆盖。
 * In the event use `const lv_area_t * area = lv_event_get_cover_area(e)` to get the area to check
 * 在事件中使用 `const lv_area_t * area = lv_event_get_cover_area(e)` 获取要检查的区域
 * and `lv_event_set_cover_res(e, res)` to set the result.
 * 并使用 `lv_event_set_cover_res(e, res)` 设置结果。
 */
struct _lv_cover_check_info_t {
    lv_cover_res_t res;             /**< 覆盖检查结果 */
    const lv_area_t * area;         /**< 要检查的区域 */
};


/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**********************
 *      MACROS
 *      宏定义
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_EVENT_PRIVATE_H*/
