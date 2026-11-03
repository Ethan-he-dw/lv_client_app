/**
 * @file lv_calendar.c
 * 日历(calendar)控件：以按钮矩阵形式显示一个月的日期，支持设置今天、高亮日期与切换月份。
 *
 */

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/

#include "lv_calendar_private.h"

#if LV_USE_CALENDAR

#include "../../draw/lv_draw_private.h"
#include "../../core/lv_obj_class_private.h"

#include "../../lvgl_public.h"
/*********************
 *      DEFINES
 *      宏定义
 *********************/
#define LV_CALENDAR_CTRL_TODAY      LV_BUTTONMATRIX_CTRL_CUSTOM_1
#define LV_CALENDAR_CTRL_HIGHLIGHT  LV_BUTTONMATRIX_CTRL_CUSTOM_2

#define MY_CLASS (&lv_calendar_class)

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数声明
 **********************/
static void lv_calendar_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void draw_task_added_event_cb(lv_event_t * e);

static uint8_t get_day_of_week(uint32_t year, uint32_t month, uint32_t day);
static uint8_t get_month_length(int32_t year, int32_t month);
static uint8_t is_leap_year(uint32_t year);
static void highlight_update(lv_obj_t * calendar);

#if LV_USE_CALENDAR_CHINESE
static lv_calendar_date_t gregorian_get_last_month_time(lv_calendar_date_t * time);
static lv_calendar_date_t gregorian_get_next_month_time(lv_calendar_date_t * time);
static void chinese_calendar_set_day_name(lv_obj_t * calendar, uint8_t index, uint8_t day,
                                          lv_calendar_date_t * gregorian_time);
#endif

/**********************
 *  STATIC VARIABLES
 *  静态变量
 **********************/

const lv_obj_class_t lv_calendar_class = {
    .constructor_cb = lv_calendar_constructor,
    .width_def = (LV_DPI_DEF * 3) / 2,
    .height_def = (LV_DPI_DEF * 3) / 2,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_calendar_t),
    .base_class = &lv_obj_class,
    .name = "lv_calendar",
};

static const char * day_names_def[7] = LV_CALENDAR_DEFAULT_DAY_NAMES;

/**********************
 *      MACROS
 *      宏
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 *   全局函数
 **********************/

/* 创建日历(calendar)对象 */
lv_obj_t * lv_calendar_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(&lv_calendar_class, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*=====================
 * Setter functions
 * 设置(set)函数
 *====================*/

/* 设置星期名称数组 */
void lv_calendar_set_day_names(lv_obj_t * obj, const char * day_names[])
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;

    uint32_t i;
    for(i = 0; i < 7; i++) {
        calendar->map[i] = day_names[i];
    }
    lv_obj_invalidate(obj);
}

/* 设置"今天"的日期 */
void lv_calendar_set_today_date(lv_obj_t * obj, uint32_t year, uint32_t month, uint32_t day)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;

    if(calendar->today.year == year && calendar->today.month == month
       && calendar->today.day == day) return;

    calendar->today.year         = year;
    calendar->today.month        = month;
    calendar->today.day          = day;

    highlight_update(obj);
}

/* 设置"今天"的年份 */
void lv_calendar_set_today_year(lv_obj_t * obj, uint32_t year)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;
    lv_calendar_set_today_date(obj, year, calendar->today.month, calendar->today.day);

}

/* 设置"今天"的月份 */
void lv_calendar_set_today_month(lv_obj_t * obj, uint32_t month)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;
    lv_calendar_set_today_date(obj, calendar->today.year, month, calendar->today.day);
}

/* 设置"今天"的日 */
void lv_calendar_set_today_day(lv_obj_t * obj, uint32_t day)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;
    lv_calendar_set_today_date(obj, calendar->today.year, calendar->today.month, day);
}

/* 设置需要高亮显示的日期数组 */
void lv_calendar_set_highlighted_dates(lv_obj_t * obj, lv_calendar_date_t highlighted[], size_t date_num)
{
    LV_ASSERT_NULL(highlighted);

    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;

    calendar->highlighted_dates     = highlighted;
    calendar->highlighted_dates_num = date_num;

    highlight_update(obj);
}

/* 设置当前显示的月份与年份 */
void lv_calendar_set_month_shown(lv_obj_t * obj, uint32_t year, uint32_t month)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;

    /*Don't return if the new value is the same, as this function is also
     *used the update the calendar e.g. when switching to Chinese mode*/
    /*即使新值与当前相同也不提前返回，因为该函数还用于刷新日历(如切换中文模式时)*/

    calendar->showed_date.year   = year;
    calendar->showed_date.month  = month;
    calendar->showed_date.day    = 1;

    lv_calendar_date_t d;
    d.year = calendar->showed_date.year;
    d.month = calendar->showed_date.month;
    d.day = calendar->showed_date.day;

    uint32_t i;

    /*Remove the disabled state but revert it for day names*/
    /*清除所有按钮的禁用状态，但星期名称按钮重新恢复禁用*/
    lv_buttonmatrix_clear_button_ctrl_all(calendar->btnm, LV_BUTTONMATRIX_CTRL_DISABLED);
    for(i = 0; i < 7; i++) {
        lv_buttonmatrix_set_button_ctrl(calendar->btnm, i, LV_BUTTONMATRIX_CTRL_DISABLED);
    }

    uint8_t act_mo_len = get_month_length(d.year, d.month);
    uint8_t day_first = get_day_of_week(d.year, d.month, 1);
    uint8_t c;

#if LV_USE_CALENDAR_CHINESE
    lv_calendar_date_t gregorian_time;
    gregorian_time = d;
#endif

    for(i = day_first, c = 1; i < act_mo_len + day_first; i++, c++) {
#if LV_USE_CALENDAR_CHINESE
        if(calendar->use_chinese_calendar) {
            gregorian_time.day = c;
            chinese_calendar_set_day_name(obj, i, c, &gregorian_time);
        }
        else
#endif
            lv_snprintf(calendar->nums[i], sizeof(calendar->nums[0]), "%d", c);
    }

    uint8_t prev_mo_len = get_month_length(d.year, d.month - 1);

#if LV_USE_CALENDAR_CHINESE
    gregorian_time = gregorian_get_last_month_time(&d);
#endif

    for(i = 0, c = prev_mo_len - day_first + 1; i < day_first; i++, c++) {
#if LV_USE_CALENDAR_CHINESE
        if(calendar->use_chinese_calendar) {
            gregorian_time.day = c;
            chinese_calendar_set_day_name(obj, i, c, &gregorian_time);
        }
        else
#endif
            lv_snprintf(calendar->nums[i], sizeof(calendar->nums[0]), "%d", c);
        lv_buttonmatrix_set_button_ctrl(calendar->btnm, i + 7, LV_BUTTONMATRIX_CTRL_DISABLED);
    }

#if LV_USE_CALENDAR_CHINESE
    gregorian_time = gregorian_get_next_month_time(&d);
#endif

    for(i = day_first + act_mo_len, c = 1; i < 6 * 7; i++, c++) {
#if LV_USE_CALENDAR_CHINESE
        if(calendar->use_chinese_calendar) {
            gregorian_time.day = c;
            chinese_calendar_set_day_name(obj, i, c, &gregorian_time);
        }
        else
#endif
            lv_snprintf(calendar->nums[i], sizeof(calendar->nums[0]), "%d", c);
        lv_buttonmatrix_set_button_ctrl(calendar->btnm, i + 7, LV_BUTTONMATRIX_CTRL_DISABLED);
    }

    highlight_update(obj);

    /*Reset the focused button if the days changes*/
    /*如果显示的日期发生变化，则重置焦点按钮*/
    if(lv_buttonmatrix_get_selected_button(calendar->btnm) != LV_BUTTONMATRIX_BUTTON_NONE) {
        lv_buttonmatrix_set_selected_button(calendar->btnm, day_first + 7);
    }

    lv_obj_invalidate(obj);

    /* The children of the calendar are probably headers.
     * Notify them to let the headers updated to the new date*/
    /* 日历的子对象通常是页眉(headers)，通知它们更新到新的日期 */
    uint32_t child_cnt = lv_obj_get_child_count(obj);
    for(i = 0; i < child_cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(obj, i);
        if(child == calendar->btnm) continue;
        lv_obj_send_event(child, LV_EVENT_VALUE_CHANGED, obj);
    }
}

/* 设置当前显示的年份 */
void lv_calendar_set_shown_year(lv_obj_t * obj, uint32_t year)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;
    lv_calendar_set_month_shown(obj, year, calendar->showed_date.month);

}

/* 设置当前显示的月份 */
void lv_calendar_set_shown_month(lv_obj_t * obj, uint32_t month)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;
    lv_calendar_set_month_shown(obj, calendar->showed_date.year, month);
}

/*=====================
 * Getter functions
 * 获取(get)函数
 *====================*/

/* 获取日历内部的按钮矩阵(btnm)对象 */
lv_obj_t * lv_calendar_get_btnmatrix(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);
    const lv_calendar_t * calendar = (lv_calendar_t *)obj;
    return calendar->btnm;
}

/* 获取"今天"的日期 */
const lv_calendar_date_t * lv_calendar_get_today_date(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);
    const lv_calendar_t * calendar = (lv_calendar_t *)obj;

    return &calendar->today;
}

/* 获取当前显示的日期 */
const lv_calendar_date_t * lv_calendar_get_showed_date(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);
    const lv_calendar_t * calendar = (lv_calendar_t *)obj;

    return &calendar->showed_date;
}

/* 获取高亮显示的日期数组 */
lv_calendar_date_t * lv_calendar_get_highlighted_dates(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;

    return calendar->highlighted_dates;
}

/* 获取高亮显示的日期数量 */
size_t lv_calendar_get_highlighted_dates_num(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;

    return calendar->highlighted_dates_num;
}

/* 获取当前按下的日期 */
lv_result_t lv_calendar_get_pressed_date(const lv_obj_t * obj, lv_calendar_date_t * date)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return LV_RESULT_INVALID);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;

    uint32_t d = lv_buttonmatrix_get_selected_button(calendar->btnm);
    if(d == LV_BUTTONMATRIX_BUTTON_NONE) {
        date->year = 0;
        date->month = 0;
        date->day = 0;
        return LV_RESULT_INVALID;
    }

    const char * txt = lv_buttonmatrix_get_button_text(calendar->btnm, lv_buttonmatrix_get_selected_button(calendar->btnm));

    if(txt[1] == 0) date->day = txt[0] - '0';
    else date->day = (txt[0] - '0') * 10 + (txt[1] - '0');

    date->year = calendar->showed_date.year;
    date->month = calendar->showed_date.month;

    return LV_RESULT_OK;
}

/**********************
 *  STATIC FUNCTIONS
 *  静态函数
 **********************/

/* 日历对象的构造函数(constructor) */
static void lv_calendar_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_calendar_t * calendar = (lv_calendar_t *)obj;

    lv_memzero(calendar->nums, sizeof(calendar->nums));
    uint8_t i;
    uint8_t j = 0;
    for(i = 0; i < 8 * 7; i++) {
        /*Every 8th string is "\n"*/
        /*每第 8 个字符串是换行符 "\n"*/
        if(i != 0 && (i + 1) % 8 == 0) {
            calendar->map[i] = "\n";
        }
        else if(i < 7) {
            calendar->map[i] = day_names_def[i];
        }
        else {
            calendar->nums[j][0] = 'x';
            calendar->map[i] = calendar->nums[j];
            j++;
        }
    }
    calendar->map[8 * 7 - 1] = "";

    calendar->btnm = lv_buttonmatrix_create(obj);
    lv_buttonmatrix_set_map(calendar->btnm, calendar->map);
    lv_buttonmatrix_set_button_ctrl_all(calendar->btnm, LV_BUTTONMATRIX_CTRL_CLICK_TRIG | LV_BUTTONMATRIX_CTRL_NO_REPEAT);
    lv_obj_add_event_cb(calendar->btnm, draw_task_added_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_set_width(calendar->btnm, lv_pct(100));

    lv_obj_set_event_bubble(calendar->btnm, true);
    lv_obj_set_send_draw_task_events(calendar->btnm, true);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(calendar->btnm, 1);

    lv_obj_set_style_text_align(calendar->btnm, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_calendar_set_month_shown(obj, 2024, 1);
    lv_calendar_set_today_date(obj, 2024, 1, 1);
}

/* 绘制任务添加事件回调：根据按钮状态调整其绘制样式 */
static void draw_task_added_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_current_target(e);
    lv_draw_task_t * draw_task = lv_event_get_param(e);
    if(((lv_draw_dsc_base_t *)draw_task->draw_dsc)->part != LV_PART_ITEMS) return;

    lv_draw_fill_dsc_t * fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
    lv_draw_border_dsc_t * border_draw_dsc = lv_draw_task_get_border_dsc(draw_task);

    if(!fill_draw_dsc && !border_draw_dsc) {
        return;
    }

    int32_t id = ((lv_draw_dsc_base_t *)draw_task->draw_dsc)->id1;

    /*Day name styles*/
    /*星期名称行的样式*/
    if(id < 7) {
        if(fill_draw_dsc) fill_draw_dsc->opa = LV_OPA_TRANSP;
        if(border_draw_dsc) border_draw_dsc->opa = LV_OPA_TRANSP;
    }
    else if(lv_buttonmatrix_has_button_ctrl(obj, id, LV_BUTTONMATRIX_CTRL_DISABLED)) {
        if(fill_draw_dsc) fill_draw_dsc->opa = LV_OPA_TRANSP;
        if(border_draw_dsc) border_draw_dsc->opa = LV_OPA_TRANSP;
    }

    if(lv_buttonmatrix_has_button_ctrl(obj, id, LV_CALENDAR_CTRL_HIGHLIGHT)) {
        if(border_draw_dsc) border_draw_dsc->color = lv_theme_get_color_primary(obj);
        if(fill_draw_dsc) fill_draw_dsc->opa = LV_OPA_40;
        if(fill_draw_dsc) fill_draw_dsc->color = lv_theme_get_color_primary(obj);
        if(lv_buttonmatrix_get_selected_button(obj) == (uint32_t)id) {
            if(fill_draw_dsc) fill_draw_dsc->opa = LV_OPA_70;
        }
    }

    if(lv_buttonmatrix_has_button_ctrl(obj, id, LV_CALENDAR_CTRL_TODAY)) {
        if(border_draw_dsc) border_draw_dsc->opa = LV_OPA_COVER;
        if(border_draw_dsc) border_draw_dsc->color = lv_theme_get_color_primary(obj);
        if(border_draw_dsc) border_draw_dsc->width += 1;
    }
}

/**
 * Get the number of days in a month
 * 获取一个月的天数
 * @param year a year
 * @param month a month. The range is basically [1..12] but [-11..0] or [13..24] is also
 *              supported to handle next/prev. year
 *              月份，范围基本为 [1..12]，也支持 [-11..0] 或 [13..24] 以处理上/下一年
 * @return [28..31]
 */
static uint8_t get_month_length(int32_t year, int32_t month)
{
    month--;
    if(month < 0) {
        year--;             /*Already in the previous year (won't be less than -12 to skip a whole year)，已退到上一年(不会小于 -12 从而跳过整个年份)*/
        month = 12 + month; /*`month` is negative, the result will be < 12，month 为负，结果将 < 12*/
    }
    if(month >= 12) {
        year++;
        month -= 12;
    }

    /*month == 1 is february*/
    /*month == 1 表示二月*/
    return (month == 1) ? (28 + is_leap_year(year)) : 31 - month % 7 % 2;
}

/**
 * Tells whether a year is leap year or not
 * 判断某年是否为闰年
 * @param year a year
 * @return 0: not leap year; 1: leap year
 *         0：非闰年；1：闰年
 */
static uint8_t is_leap_year(uint32_t year)
{
    return (year % 4) || ((year % 100 == 0) && (year % 400)) ? 0 : 1;
}

/**
 * Get the day of the week
 * 获取某天是星期几
 * @param year a year
 * @param month a  month [1..12]
 * @param day a day [1..32]
 * @return [0..6] which means [Sun..Sat] or [Mon..Sun] depending on LV_CALENDAR_WEEK_STARTS_MONDAY
 *         [0..6]，根据 LV_CALENDAR_WEEK_STARTS_MONDAY 表示 [周日..周六] 或 [周一..周日]
 */
static uint8_t get_day_of_week(uint32_t year, uint32_t month, uint32_t day)
{
    uint32_t a = month < 3 ? 1 : 0;
    uint32_t b = year - a;

#if LV_CALENDAR_WEEK_STARTS_MONDAY
    uint32_t day_of_week = (day + (31 * (month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400) - 1) % 7;
#else
    uint32_t day_of_week = (day + (31 * (month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400)) % 7;
#endif

    return day_of_week  ;
}

/* 更新"今天"与高亮日期的按钮状态 */
static void highlight_update(lv_obj_t * obj)
{
    lv_calendar_t * calendar = (lv_calendar_t *)obj;
    uint32_t i;

    /*Clear all kind of selection*/
    /*清除所有类型的选择状态*/
    lv_buttonmatrix_clear_button_ctrl_all(calendar->btnm, LV_CALENDAR_CTRL_TODAY | LV_CALENDAR_CTRL_HIGHLIGHT);

    uint8_t day_first = get_day_of_week(calendar->showed_date.year, calendar->showed_date.month, 1);
    if(calendar->highlighted_dates) {
        for(i = 0; i < calendar->highlighted_dates_num; i++) {
            if(calendar->highlighted_dates[i].year == calendar->showed_date.year &&
               calendar->highlighted_dates[i].month == calendar->showed_date.month) {
                lv_buttonmatrix_set_button_ctrl(calendar->btnm, calendar->highlighted_dates[i].day - 1 + day_first + 7,
                                                LV_CALENDAR_CTRL_HIGHLIGHT);
            }
        }
    }

    if(calendar->showed_date.year == calendar->today.year && calendar->showed_date.month == calendar->today.month) {
        lv_buttonmatrix_set_button_ctrl(calendar->btnm, calendar->today.day - 1 + day_first + 7, LV_CALENDAR_CTRL_TODAY);
    }
}

#if LV_USE_CALENDAR_CHINESE

/* 获取指定公历日期的上一个月份 */
static lv_calendar_date_t gregorian_get_last_month_time(lv_calendar_date_t * time)
{
    lv_calendar_date_t last_month_time;
    if(time->month == 1) {
        last_month_time.month = 12;
        last_month_time.year = time->year - 1;
    }
    else {
        last_month_time.month = time->month - 1;
        last_month_time.year = time->year;
    }
    return last_month_time;
}

/* 获取指定公历日期的下一个月份 */
static lv_calendar_date_t gregorian_get_next_month_time(lv_calendar_date_t * time)
{
    lv_calendar_date_t next_month_time;
    if(time->month == 12) {
        next_month_time.month = 1;
        next_month_time.year = time->year + 1;
    }
    else {
        next_month_time.month = time->month + 1;
        next_month_time.year = time->year;
    }
    return next_month_time;
}

/* 为日历按钮设置农历日期名称 */
static void chinese_calendar_set_day_name(lv_obj_t * obj, uint8_t index, uint8_t day,
                                          lv_calendar_date_t * gregorian_time)
{
    lv_calendar_t * calendar = (lv_calendar_t *)obj;
    const char * day_name = lv_calendar_get_day_name(gregorian_time);
    if(day_name != NULL)
        lv_snprintf(calendar->nums[index], sizeof(calendar->nums[0]),
                    "%d\n%s",
                    day,
                    day_name);
    else
        lv_snprintf(calendar->nums[index], sizeof(calendar->nums[0]), "%d", day);
}
#endif

#endif  /*LV_USE_CALENDAR*/
