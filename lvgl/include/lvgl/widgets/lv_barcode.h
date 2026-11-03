/**
 * @file lv_barcode.h
 * 条形码控件:基于画布(canvas)生成并显示条形码
 *
 */

#ifndef LV_BARCODE_H
#define LV_BARCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"
#include "../lv_types.h"
#include "../draw/lv_color.h"
#include "lv_canvas.h"

#if LV_USE_BARCODE

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

/** 条形码编码方式 */
typedef enum {
    /**
     * Code 128 with GS1 encoding. Strips `[FCN1]` and spaces.
     * 采用 GS1 编码的 Code 128,会去除 `[FCN1]` 和空格。
     */
    LV_BARCODE_ENCODING_CODE128_GS1,
    /**
     * Code 128 with raw encoding.
     * 采用原始编码的 Code 128。
     */
    LV_BARCODE_ENCODING_CODE128_RAW,
} lv_barcode_encoding_t;

/** 条形码对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_barcode_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create an empty barcode (an `lv_canvas`) object.
 * 创建空的条形码(一个 `lv_canvas`)对象。
 * @param parent point to an object where to create the barcode
 * @param parent 指向一个对象的指针,条形码将创建在该对象上
 * @return pointer to the created barcode object
 * @return 指向所创建的条形码对象的指针
 */
lv_obj_t * lv_barcode_create(lv_obj_t * parent);

/**
 * Set the dark color of a barcode object
 * 设置条形码对象的深色(条)颜色
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @param color dark color of the barcode
 * @param color 条形码的深色颜色
 */
void lv_barcode_set_dark_color(lv_obj_t * obj, lv_color_t color);

/**
 * Set the light color of a barcode object
 * 设置条形码对象的浅色(背景)颜色
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @param color light color of the barcode
 * @param color 条形码的浅色颜色
 */
void lv_barcode_set_light_color(lv_obj_t * obj, lv_color_t color);

/**
 * Set the scale of a barcode object
 * 设置条形码对象的缩放比例
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @param scale scale factor
 * @param scale 缩放因子
 */
void lv_barcode_set_scale(lv_obj_t * obj, uint16_t scale);

/**
 * Set the direction of a barcode object
 * 设置条形码对象的绘制方向
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @param direction draw direction (`LV_DIR_HOR` or `LB_DIR_VER`)
 * @param direction 绘制方向(`LV_DIR_HOR` 或 `LB_DIR_VER`)
 */
void lv_barcode_set_direction(lv_obj_t * obj, lv_dir_t direction);

/**
 * Set the tiled mode of a barcode object
 * 设置条形码对象的平铺模式
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @param tiled true: tiled mode, false: normal mode (default)
 * @param tiled true:平铺模式,false:普通模式(默认)
 */
void lv_barcode_set_tiled(lv_obj_t * obj, bool tiled);

/**
 * Set the encoding of a barcode object
 * 设置条形码对象的编码方式
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @param encoding encoding (default is `LV_BARCODE_CODE128_GS1`)
 * @param encoding 编码方式(默认为 `LV_BARCODE_CODE128_GS1`)
 */
void lv_barcode_set_encoding(lv_obj_t * obj, lv_barcode_encoding_t encoding);

/**
 * Set the data of a barcode object
 * 设置条形码对象的数据
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @param data data to display
 * @param data 要显示的数据
 * @return LV_RESULT_OK: if no error; LV_RESULT_INVALID: on error
 * @return LV_RESULT_OK:无错误;LV_RESULT_INVALID:出错
 */
lv_result_t lv_barcode_update(lv_obj_t * obj, const char * data);

/**
 * Get the dark color of a barcode object
 * 获取条形码对象的深色(条)颜色
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @return dark color of the barcode
 * @return 条形码的深色颜色
 */
lv_color_t lv_barcode_get_dark_color(lv_obj_t * obj);

/**
 * Get the light color of a barcode object
 * 获取条形码对象的浅色(背景)颜色
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @return light color of the barcode
 * @return 条形码的浅色颜色
 */
lv_color_t lv_barcode_get_light_color(lv_obj_t * obj);

/**
 * Get the scale of a barcode object
 * 获取条形码对象的缩放比例
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @return scale factor
 * @return 缩放因子
 */
uint16_t lv_barcode_get_scale(lv_obj_t * obj);

/**
 * Get the encoding of a barcode object
 * 获取条形码对象的编码方式
 * @param obj pointer to barcode object
 * @param obj 指向条形码对象的指针
 * @return encoding
 * @return 编码方式
 */
lv_barcode_encoding_t lv_barcode_get_encoding(const lv_obj_t * obj);

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_BARCODE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_BARCODE_H*/
