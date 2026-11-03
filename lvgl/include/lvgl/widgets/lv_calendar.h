/**
 * @file lv_calendar.h
 * 日历控件:显示年月并选择日期,内部基于按钮矩阵实现
 *
 */

#ifndef LV_CALENDAR_H
#define LV_CALENDAR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "lv_buttonmatrix.h"

#if LV_USE_CALENDAR

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

/**
 * Represents a date on the calendar object (platform-agnostic).
 * 表示日历对象上的一个日期(与平台无关)。
 */
typedef struct {
    /** 年份 */
    uint16_t year;
    uint8_t month;  /**< 1..12 */
    /**< 月份 1..12 */
    uint8_t day;    /**< 1..31 */
    /**< 日期 1..31 */
} lv_calendar_date_t;

/** 日历对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_calendar_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a calendar widget
 * 创建日历控件
 * @param parent    pointer to an object, it will be the parent of the new calendar
 * @param parent    指向一个对象的指针,该对象将成为新日历的父对象
 * @return          pointer the created calendar
 * @return          指向所创建的日历的指针
 */
lv_obj_t * lv_calendar_create(lv_obj_t * parent);

/*======================
 * Add/remove functions
 * 添加/删除函数
 *=====================*/

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the today's year, month and day at once
 * 同时设置今天的年、月、日
 * @param obj  pointer to a calendar object
 * @param obj  指向日历对象的指针
 * @param year      today's year
 * @param year      今天的年份
 * @param month     today's month [1..12]
 * @param month     今天的月份 [1..12]
 * @param day       today's day [1..31]
 * @param day       今天的日期 [1..31]
 */
void lv_calendar_set_today_date(lv_obj_t * obj, uint32_t year, uint32_t month, uint32_t day);

/**
 * Set the today's year
 * 设置今天的年份
 * @param obj  pointer to a calendar object
 * @param obj  指向日历对象的指针
 * @param year      today's year
 * @param year      今天的年份
 */
void lv_calendar_set_today_year(lv_obj_t * obj, uint32_t year);

/**
 * Set the today's year
 * 设置今天的月份
 * @param obj  pointer to a calendar object
 * @param obj  指向日历对象的指针
 * @param month     today's month [1..12]
 * @param month     今天的月份 [1..12]
 */
void lv_calendar_set_today_month(lv_obj_t * obj, uint32_t month);

/**
 * Set the today's year
 * 设置今天的日期
 * @param obj  pointer to a calendar object
 * @param obj  指向日历对象的指针
 * @param day       today's day [1..31]
 * @param day       今天的日期 [1..31]
 */
void lv_calendar_set_today_day(lv_obj_t * obj, uint32_t day);

/**
 * Set the currently shown year and month at once
 * 同时设置当前显示的年和月
 * @param obj           pointer to a calendar object
 * @param obj           指向日历对象的指针
 * @param year          shown year
 * @param year          显示的年份
 * @param month         shown month [1..12]
 * @param month         显示的月份 [1..12]
 */
void lv_calendar_set_month_shown(lv_obj_t * obj, uint32_t year, uint32_t month);

/**
 * Set the currently shown year
 * 设置当前显示的年份
 * @param obj           pointer to a calendar object
 * @param obj           指向日历对象的指针
 * @param year          shown year
 * @param year          显示的年份
 */
void lv_calendar_set_shown_year(lv_obj_t * obj, uint32_t year);

/**
 * Set the currently shown month
 * 设置当前显示的月份
 * @param obj           pointer to a calendar object
 * @param obj           指向日历对象的指针
 * @param month         shown month [1..12]
 * @param month         显示的月份 [1..12]
 */
void lv_calendar_set_shown_month(lv_obj_t * obj, uint32_t month);

/**
 * Set the highlighted dates
 * 设置高亮显示的日期
 * @param obj           pointer to a calendar object
 * @param obj           指向日历对象的指针
 * @param highlighted   pointer to an `lv_calendar_date_t` array containing the dates.
 *                      Only the pointer will be saved so this variable can't be local which will be destroyed later.
 *                      highlighted:指向包含日期的 `lv_calendar_date_t` 数组的指针。
 *                      只保存该指针,因此该变量不能是之后会被销毁的局部变量。
 * @param date_num number of dates in the array
 * @param date_num 数组中日期的数量
 */
void lv_calendar_set_highlighted_dates(lv_obj_t * obj, lv_calendar_date_t highlighted[], size_t date_num);

/**
 * Set the name of the days
 * 设置星期的名称
 * @param obj           pointer to a calendar object
 * @param obj           指向日历对象的指针
 * @param day_names     pointer to an array with the names.
 *                      E.g. `const char * days[7] = {"Sun", "Mon", ...}`
 *                      Only the pointer will be saved so this variable can't be local which will be destroyed later.
 *                      day_names:指向名称数组的指针。
 *                      例如 `const char * days[7] = {"Sun", "Mon", ...}`
 *                      只保存该指针,因此该变量不能是之后会被销毁的局部变量。
 */
void lv_calendar_set_day_names(lv_obj_t * obj, const char ** day_names);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the button matrix object of the calendar.
 * 获取日历的按钮矩阵对象。
 * It shows the dates and day names.
 * 该按钮矩阵用于显示日期和星期名称。
 * @param obj       pointer to a calendar object
 * @param obj       指向日历对象的指针
 * @return          pointer to a the button matrix
 * @return          指向按钮矩阵的指针
 */
lv_obj_t * lv_calendar_get_btnmatrix(const lv_obj_t * obj);

/**
 * Get the today's date
 * 获取今天的日期
 * @param calendar  pointer to a calendar object
 * @param calendar  指向日历对象的指针
 * @return          return pointer to an `lv_calendar_date_t` variable containing the date of today.
 * @return          返回指向包含今天日期的 `lv_calendar_date_t` 变量的指针。
 */
const lv_calendar_date_t * lv_calendar_get_today_date(const lv_obj_t * calendar);

/**
 * Get the currently showed
 * 获取当前显示的日期
 * @param calendar  pointer to a calendar object
 * @param calendar  指向日历对象的指针
 * @return          pointer to an `lv_calendar_date_t` variable containing the date is being shown.
 * @return          指向包含当前显示日期的 `lv_calendar_date_t` 变量的指针。
 */
const lv_calendar_date_t * lv_calendar_get_showed_date(const lv_obj_t * calendar);

/**
 * Get the highlighted dates
 * 获取高亮显示的日期
 * @param calendar  pointer to a calendar object
 * @param calendar  指向日历对象的指针
 * @return          pointer to an `lv_calendar_date_t` array containing the dates.
 * @return          指向包含高亮日期的 `lv_calendar_date_t` 数组的指针。
 */
lv_calendar_date_t * lv_calendar_get_highlighted_dates(const lv_obj_t * calendar);

/**
 * Get the number of the highlighted dates
 * 获取高亮显示日期的数量
 * @param calendar  pointer to a calendar object
 * @param calendar  指向日历对象的指针
 * @return          number of highlighted days
 * @return          高亮显示的天数
 */
size_t lv_calendar_get_highlighted_dates_num(const lv_obj_t * calendar);

/**
 * Get the currently pressed day
 * 获取当前按下的日期
 * @param calendar  pointer to a calendar object
 * @param calendar  指向日历对象的指针
 * @param date      store the pressed date here
 * @param date      在此处存储按下的日期
 * @return          LV_RESULT_OK: there is a valid pressed date
 *                  LV_RESULT_INVALID: there is no pressed data
 * @return          LV_RESULT_OK:存在有效的按下日期
 *                  LV_RESULT_INVALID:不存在按下的日期
 */
lv_result_t lv_calendar_get_pressed_date(const lv_obj_t * calendar, lv_calendar_date_t * date);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**********************
 *      MACROS
 * 宏
 **********************/

#include "lv_calendar_header_arrow.h"
#include "lv_calendar_header_dropdown.h"
#include "lv_calendar_chinese.h"

#endif  /*LV_USE_CALENDAR*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CALENDAR_H*/
