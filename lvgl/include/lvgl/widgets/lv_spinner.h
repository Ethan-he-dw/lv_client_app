/**
 * @file lv_spinner.h
 *
 * 加载指示器（spinner）控件
 */

#ifndef LV_SPINNER_H
#define LV_SPINNER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_SPINNER

#include "../lv_types.h"
#include "../core/lv_obj_property.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/
/** 加载指示器（spinner）控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_spinner_class;

#if LV_USE_OBJ_PROPERTY
/** 加载指示器控件属性ID枚举（用于对象属性系统） */
enum _lv_property_spinner_id_t {
    /** 动画持续时间属性 */
    LV_PROPERTY_ID(SPINNER, ANIM_DURATION, LV_PROPERTY_TYPE_INT, 0),
    /** 圆弧扫过角度属性 */
    LV_PROPERTY_ID(SPINNER, ARC_SWEEP,     LV_PROPERTY_TYPE_INT, 1),
    LV_PROPERTY_SPINNER_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a spinner widget
 * 创建一个加载指示器（spinner）控件
 * @param parent    pointer to an object, it will be the parent of the new spinner.
 *                  指向一个对象，它将成为新spinner的父对象。
 * @return          the created spinner
 *                  创建的spinner
 */
lv_obj_t * lv_spinner_create(lv_obj_t * parent);

/**
 * Set the animation time and arc length of the spinner
 * 设置spinner的动画时间和圆弧长度
 * The animation is suited for angle values between 180 and 360.
 * 该动画适用于180到360之间的角度值。
 * @param obj       pointer to a spinner
 *                  指向一个spinner
 * @param t         the animation time in milliseconds
 *                  动画时间（毫秒）
 * @param angle     the angle of the arc in degrees
 *                  圆弧的角度（度）
 */
void lv_spinner_set_anim_params(lv_obj_t * obj, uint32_t t, uint32_t angle);

/**
 * Set the animation time of the spinner
 * 设置spinner的动画时间
 * @param obj       pointer to a spinner
 *                  指向一个spinner
 * @param t         the animation time in milliseconds
 *                  动画时间（毫秒）
 */
void lv_spinner_set_anim_duration(lv_obj_t * obj, uint32_t t);

/**
 * Set the animation arc length of the spinner.
 * 设置spinner的动画圆弧长度。
 * The animation is suited to values between 180 and 360.
 * 该动画适用于180到360之间的角度值。
 * @param obj       pointer to a spinner
 *                  指向一个spinner
 * @param angle     the angle of the arc in degrees
 *                  圆弧的角度（度）
 */
void lv_spinner_set_arc_sweep(lv_obj_t * obj, uint32_t angle);

/**
 * Get the animation duration of the spinner
 * 获取spinner的动画持续时间
 * @param obj       pointer to a spinner
 *                  指向一个spinner
 * @return          the animation time in milliseconds
 *                  动画时间（毫秒）
 */
uint32_t lv_spinner_get_anim_duration(lv_obj_t * obj);

/**
 * Get the animation arc length of the spinner
 * 获取spinner的动画圆弧长度
 * @param obj       pointer to a spinner
 *                  指向一个spinner
 * @return          the angle of the arc in degrees
 *                  圆弧的角度（度）
 */
uint32_t lv_spinner_get_arc_sweep(lv_obj_t * obj);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_SPINNER*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SPINNER_H*/
