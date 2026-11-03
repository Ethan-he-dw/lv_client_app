/**
 * @file lv_calendar_header_arrow.h
 *
 * 日历头部箭头控件
 */

#ifndef LV_CALENDAR_HEADER_ARROW_H
#define LV_CALENDAR_HEADER_ARROW_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../core/lv_obj.h"
#if LV_USE_CALENDAR && LV_USE_CALENDAR_HEADER_ARROW

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/
/** 日历头部箭头控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_calendar_header_arrow_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a calendar header with drop-drowns to select the year and month
 * 创建一个带有下拉框以选择年份和月份的日历头部
 * @param parent    pointer to a calendar object.
 *                  指向一个日历对象。
 * @return          the created header
 *                  创建的日历头部
 */
lv_obj_t * lv_calendar_add_header_arrow(lv_obj_t * parent);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_CALENDAR_HEADER_ARROW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CALENDAR_HEADER_ARROW_H*/
