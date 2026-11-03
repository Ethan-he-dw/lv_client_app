/**
 * @file lv_obj_draw_private.h
 * 对象绘制私有模块：提供获取对象扩展绘制区域与图层类型的内部接口。
 *
 */

#ifndef LV_OBJ_DRAW_PRIVATE_H
#define LV_OBJ_DRAW_PRIVATE_H

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
 * Get the extended draw area of an object.
 * 获取对象的扩展绘制区域。
 * @param obj       pointer to an object
 *                  指向对象的指针
 * @return          the size extended draw area around the real coordinates
 *                  实际坐标周围扩展绘制区域的大小
 */
int32_t lv_obj_get_ext_draw_size(const lv_obj_t * obj);

lv_layer_type_t lv_obj_get_layer_type(const lv_obj_t * obj);   /**< 获取对象的图层类型 */

/**********************
 *      MACROS
 *      宏定义
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_DRAW_PRIVATE_H*/
