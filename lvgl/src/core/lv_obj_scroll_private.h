/**
 * @file lv_obj_scroll_private.h
 * 对象滚动私有模块：提供对象滚动的底层内部接口。
 *
 */

#ifndef LV_OBJ_SCROLL_PRIVATE_H
#define LV_OBJ_SCROLL_PRIVATE_H

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

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Low level function to scroll by given x and y coordinates.
 * 按给定的 x 和 y 坐标进行滚动的底层函数。
 * `LV_EVENT_SCROLL` is sent.
 * 会发送 `LV_EVENT_SCROLL` 事件。
 * @param obj       pointer to an object to scroll
 *                  指向要滚动对象的指针
 * @param x         pixels to scroll horizontally
 *                  水平方向滚动的像素数
 * @param y         pixels to scroll vertically
 *                  垂直方向滚动的像素数
 * @return          `LV_RESULT_INVALID`: to object was deleted in `LV_EVENT_SCROLL`;
 *                  `LV_RESULT_OK`: if the object is still valid
 *                  `LV_RESULT_INVALID`: 对象在 `LV_EVENT_SCROLL` 中被删除;
 *                  `LV_RESULT_OK`: 对象仍然有效
 */
lv_result_t lv_obj_scroll_by_raw(lv_obj_t * obj, int32_t x, int32_t y);

/**********************
 *      MACROS
 *      宏定义
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_SCROLL_PRIVATE_H*/
