/**
 * @file lv_button.h
 *
 * 按钮控件
 */

#ifndef LV_BUTTON_H
#define LV_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_BUTTON != 0
#include "../core/lv_obj.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/** 按钮控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_button_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a button object
 * 创建一个按钮对象
 * @param parent    pointer to an object, it will be the parent of the new button
 *                  指向一个对象，它将成为新按钮的父对象
 * @return          pointer to the created button
 *                  指向创建的按钮的指针
 */
lv_obj_t * lv_button_create(lv_obj_t * parent);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_BUTTON*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_BUTTON_H*/
