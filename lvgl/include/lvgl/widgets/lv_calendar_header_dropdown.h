/**
 * @file lv_calendar_header_dropdown.h
 *
 * 日历头部下拉框控件
 */

#ifndef LV_CALENDAR_HEADER_DROPDOWN_H
#define LV_CALENDAR_HEADER_DROPDOWN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../core/lv_obj.h"
#if LV_USE_CALENDAR && LV_USE_CALENDAR_HEADER_DROPDOWN

#if LV_USE_DROPDOWN == 0
#error "LV_USE_DROPDOWN needs to be enabled"
#endif

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/
/** 日历头部下拉框控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_calendar_header_dropdown_class;

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
lv_obj_t * lv_calendar_add_header_dropdown(lv_obj_t * parent);

/**
 * Sets a custom calendar year list
 * 设置自定义的日历年份列表
 * @param parent        pointer to a calendar object
 *                      指向一个日历对象
 * @param years_list    pointer to an const char array with the years list, see lv_dropdown set_options for more information.
 *                      指向包含年份列表的const char数组，更多信息请参考lv_dropdown的set_options。
 *                      E.g. `const char * years = "2023\n2022\n2021\n2020\n2019"
 *                      例如：`const char * years = "2023\n2022\n2021\n2020\n2019"
 *                      Only the pointer will be saved so this variable can't be local which will be destroyed later.
 *                      只保存指针本身，因此该变量不能是之后会被销毁的局部变量。
 */
void lv_calendar_header_dropdown_set_year_list(lv_obj_t * parent, const char * years_list);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_CALENDAR_HEADER_ARROW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CALENDAR_HEADER_DROPDOWN_H*/
