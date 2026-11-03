/**
 * @file lv_calendar_chinese.h
 *
 * 中国农历日历控件
 */

#ifndef LV_CALENDAR_CHINESE_H
#define LV_CALENDAR_CHINESE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "lv_calendar.h"
#if LV_USE_CALENDAR && LV_USE_CALENDAR_CHINESE

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

typedef struct {
    /** 今天的公历日期 */
    lv_calendar_date_t today;
    /** 是否为闰月 */
    bool leep_month;
} lv_calendar_chinese_t;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Enable the chinese calendar.
 * 启用中国农历日历。
 * @param obj   pointer to a calendar object.
 *              指向一个日历对象。
 * @param en    true: enable chinese calendar; false: disable
 *              true：启用农历日历；false：禁用
 */
void lv_calendar_set_chinese_mode(lv_obj_t * obj, bool en);

/**
 * Get the name of the day
 * 获取某天的名称
 * @param gregorian   to obtain the gregorian time for the name
 *                    用于获取该名称对应的公历时间
 * @return            return the name of the day
 *                    返回该天的名称
 */
const char * lv_calendar_get_day_name(lv_calendar_date_t * gregorian);

/**
 * Get the chinese time of the gregorian time (reference: https://www.cnblogs.com/liyang31tg/p/4123171.html)
 * 将公历时间转换为农历时间（参考：https://www.cnblogs.com/liyang31tg/p/4123171.html）
 * @param gregorian_time   need to convert to chinese time in gregorian time
 *                         需要转换为农历时间的公历时间
 * @param chinese_time     the chinese time convert from gregorian time
 *                         由公历时间转换得到的农历时间
 */
void lv_calendar_gregorian_to_chinese(lv_calendar_date_t * gregorian_time, lv_calendar_chinese_t * chinese_time);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_CALENDAR_CHINESE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CALENDAR_CHINESE_H*/
