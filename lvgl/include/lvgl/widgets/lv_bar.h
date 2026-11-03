/**
 * @file lv_bar.h
 * 进度条控件:显示数值或进度,支持对称与区间模式
 *
 */

#ifndef LV_BAR_H
#define LV_BAR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_BAR != 0

#include "../core/lv_obj.h"
#include "lv_label.h"
#include "../core/lv_observer.h"

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/
/** 进度条模式 */
typedef enum {
    /** 普通模式,从最小值方向开始增长 */
    LV_BAR_MODE_NORMAL,
    /** 对称模式,从中间向两侧增长 */
    LV_BAR_MODE_SYMMETRICAL,
    /** 区间模式,支持起始值与结束值 */
    LV_BAR_MODE_RANGE
} lv_bar_mode_t;

/** 进度条方向 */
typedef enum {
    /** 自动方向(根据宽高判断) */
    LV_BAR_ORIENTATION_AUTO,
    /** 水平方向 */
    LV_BAR_ORIENTATION_HORIZONTAL,
    /** 垂直方向 */
    LV_BAR_ORIENTATION_VERTICAL
} lv_bar_orientation_t;

/** 进度条对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_bar_class;

#if LV_USE_OBJ_PROPERTY
/** 属性系统:进度条对象的属性 ID 枚举 */
enum _lv_property_bar_id_t {
    LV_PROPERTY_ID(BAR, VALUE,          LV_PROPERTY_TYPE_INT,   0),
    LV_PROPERTY_ID(BAR, START_VALUE,    LV_PROPERTY_TYPE_INT,   1),
    LV_PROPERTY_ID(BAR, MIN_VALUE,      LV_PROPERTY_TYPE_INT,   2),
    LV_PROPERTY_ID(BAR, MAX_VALUE,      LV_PROPERTY_TYPE_INT,   3),
    LV_PROPERTY_ID(BAR, MODE,           LV_PROPERTY_TYPE_INT,   4),
    LV_PROPERTY_ID(BAR, ORIENTATION,    LV_PROPERTY_TYPE_INT,   5),
    LV_PROPERTY_BAR_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a bar object
 * 创建进度条对象
 * @param parent        pointer to an object, it will be the parent of the new bar
 * @param parent        指向一个对象的指针,该对象将成为新进度条的父对象
 * @return              pointer to the created bar
 * @return              指向所创建的进度条对象的指针
 */
lv_obj_t * lv_bar_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set a new value on the bar
 * 设置进度条的新值
 * @param obj           pointer to a bar object
 * @param obj           指向进度条对象的指针
 * @param value         new value
 * @param value         新值
 * @param anim          LV_ANIM_ON: set the value with an animation; LV_ANIM_OFF: change the value immediately
 * @param anim          LV_ANIM_ON:带动画设置值;LV_ANIM_OFF:立即改变值
 */
void lv_bar_set_value(lv_obj_t * obj, int32_t value, lv_anim_enable_t anim);

/**
 * Set a new start value on the bar
 * 设置进度条的新起始值
 * @param obj             pointer to a bar object
 * @param obj             指向进度条对象的指针
 * @param start_value     new start value
 * @param start_value     新的起始值
 * @param anim            LV_ANIM_ON: set the value with an animation; LV_ANIM_OFF: change the value immediately
 * @param anim            LV_ANIM_ON:带动画设置值;LV_ANIM_OFF:立即改变值
 */
void lv_bar_set_start_value(lv_obj_t * obj, int32_t start_value, lv_anim_enable_t anim);

/**
 * Set minimum and the maximum values of a bar
 * 设置进度条的最小值和最大值
 * @param obj       pointer to the bar object
 * @param obj       指向进度条对象的指针
 * @param min       minimum value
 * @param min       最小值
 * @param max       maximum value
 * @param max       最大值
 * @note If min is greater than max, the drawing direction becomes to the opposite direction.
 * @note 如果 min 大于 max,绘制方向将变为相反方向。
 */
void lv_bar_set_range(lv_obj_t * obj, int32_t min, int32_t max);

/**
 * Set minimum value of a bar
 * 设置进度条的最小值
 * @param obj       pointer to the bar object
 * @param obj       指向进度条对象的指针
 * @param min       minimum value
 * @param min       最小值
 */
void lv_bar_set_min_value(lv_obj_t * obj, int32_t min);

/**
 * Set maximum value of a bar
 * 设置进度条的最大值
 * @param obj       pointer to the bar object
 * @param obj       指向进度条对象的指针
 * @param max       maximum value
 * @param max       最大值
 */
void lv_bar_set_max_value(lv_obj_t * obj, int32_t max);

/**
 * Set the type of bar.
 * 设置进度条的类型(模式)。
 * @param obj       pointer to bar object
 * @param obj       指向进度条对象的指针
 * @param mode      bar type from `lv_bar_mode_t`
 * @param mode      进度条模式,取自 `lv_bar_mode_t`
 */
void lv_bar_set_mode(lv_obj_t * obj, lv_bar_mode_t mode);

/**
 * Set the orientation of bar.
 * 设置进度条的方向。
 * @param obj           pointer to bar object
 * @param obj           指向进度条对象的指针
 * @param orientation   bar orientation from `lv_bar_orientation_t`
 * @param orientation   进度条方向,取自 `lv_bar_orientation_t`
 */
void lv_bar_set_orientation(lv_obj_t * obj, lv_bar_orientation_t orientation);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the value of a bar
 * 获取进度条的值
 * @param obj       pointer to a bar object
 * @param obj       指向进度条对象的指针
 * @return          the value of the bar
 * @return          进度条的值
 */
int32_t lv_bar_get_value(const lv_obj_t * obj);

/**
 * Get the start value of a bar
 * 获取进度条的起始值
 * @param obj       pointer to a bar object
 * @param obj       指向进度条对象的指针
 * @return          the start value of the bar
 * @return          进度条的起始值
 */
int32_t lv_bar_get_start_value(const lv_obj_t * obj);

/**
 * Get the minimum value of a bar
 * 获取进度条的最小值
 * @param obj       pointer to a bar object
 * @param obj       指向进度条对象的指针
 * @return          the minimum value of the bar
 * @return          进度条的最小值
 */
int32_t lv_bar_get_min_value(const lv_obj_t * obj);

/**
 * Get the maximum value of a bar
 * 获取进度条的最大值
 * @param obj       pointer to a bar object
 * @param obj       指向进度条对象的指针
 * @return          the maximum value of the bar
 * @return          进度条的最大值
 */
int32_t lv_bar_get_max_value(const lv_obj_t * obj);

/**
 * Get the type of bar.
 * 获取进度条的类型(模式)。
 * @param obj       pointer to bar object
 * @param obj       指向进度条对象的指针
 * @return          bar type from `lv_bar_mode_t`
 * @return          进度条模式,取自 `lv_bar_mode_t`
 */
lv_bar_mode_t lv_bar_get_mode(lv_obj_t * obj);

/**
 * Get the orientation of bar.
 * 获取进度条的方向。
 * @param obj       pointer to bar object
 * @param obj       指向进度条对象的指针
 * @return          bar orientation from `lv_bar_orientation_t`
 * @return          进度条方向,取自 `lv_bar_orientation_t`
 */
lv_bar_orientation_t lv_bar_get_orientation(lv_obj_t * obj);

/**
 * Give the bar is in symmetrical mode or not
 * 返回进度条是否处于对称模式
 * @param obj       pointer to bar object
 * @param obj       指向进度条对象的指针
 * @return          true: in symmetrical mode false : not in
 * @return          true:处于对称模式 false:不处于
*/
bool lv_bar_is_symmetrical(lv_obj_t * obj);

#if LV_USE_OBSERVER
/**
 * Bind an integer or float Subject to a Bar's value.
 * 将整数或浮点主题(Subject)绑定到进度条的值上。
 * @param obj       pointer to Bar
 * @param obj       指向进度条的指针
 * @param subject   pointer to Subject
 * @param subject   指向主题(Subject)的指针
 * @return          pointer to newly-created Observer
 * @return          指向新创建的观察者(Observer)的指针
 */
lv_observer_t * lv_bar_bind_value(lv_obj_t * obj, lv_subject_t * subject);
#endif

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_BAR*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_BAR_H*/
