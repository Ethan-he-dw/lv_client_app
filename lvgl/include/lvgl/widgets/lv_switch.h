/**
 * @file lv_switch.h
 *
 * 开关控件
 */

#ifndef LV_SWITCH_H
#define LV_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_SWITCH != 0

#include "../core/lv_obj.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/** Switch knob extra area correction factor */
/** 开关旋钮额外区域修正系数 */
#define LV_SWITCH_KNOB_EXT_AREA_CORRECTION 2

/** 开关控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_switch_class;

#if LV_USE_OBJ_PROPERTY
/** 开关控件属性ID枚举（用于对象属性系统） */
enum _lv_property_switch_id_t {
    /** 开关方向属性 */
    LV_PROPERTY_ID(SWITCH, ORIENTATION, LV_PROPERTY_TYPE_INT, 0),
    LV_PROPERTY_SWITCH_END,
};
#endif

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/** 开关方向枚举 */
typedef enum {
    /** 自动方向（默认） */
    LV_SWITCH_ORIENTATION_AUTO,
    /** 水平方向 */
    LV_SWITCH_ORIENTATION_HORIZONTAL,
    /** 垂直方向 */
    LV_SWITCH_ORIENTATION_VERTICAL
} lv_switch_orientation_t;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a switch object
 * 创建一个开关对象
 * @param parent    pointer to an object, it will be the parent of the new switch
 *                  指向一个对象，它将成为新开关的父对象
 * @return          pointer to the created switch
 *                  指向创建的开关的指针
 */
lv_obj_t * lv_switch_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the orientation of switch.
 * 设置开关的方向。
 * @param obj           pointer to switch object
 *                      指向开关对象
 * @param orientation   switch orientation from `lv_switch_orientation_t`
 *                      开关方向，取自`lv_switch_orientation_t`
 */
void lv_switch_set_orientation(lv_obj_t * obj, lv_switch_orientation_t orientation);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the orientation of switch.
 * 获取开关的方向。
 * @param obj       pointer to switch object
 *                  指向开关对象
 * @return          switch orientation from ::lv_switch_orientation_t
 *                  开关方向，取自::lv_switch_orientation_t
 */
lv_switch_orientation_t lv_switch_get_orientation(lv_obj_t * obj);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_SWITCH*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SWITCH_H*/
