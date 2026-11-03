/**
 * @file lv_qrcode.h
 * 二维码控件头文件：基于 lv_canvas 绘制二维码
 *
 */

#ifndef LV_QRCODE_H
#define LV_QRCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"
#if LV_USE_QRCODE

#include "../draw/lv_color.h"
#include "../lv_types.h"
#include "lv_canvas.h"
#include LV_STDBOOL_INCLUDE
#include LV_STDINT_INCLUDE

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/** 二维码控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_qrcode_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create an empty QR code (an `lv_canvas`) object.
 * 创建一个空的二维码(一个 `lv_canvas`)对象
 * @param parent point to an object where to create the QR code
 * @param parent 指向要在其上创建二维码的对象
 * @return pointer to the created QR code object
 * @return 指向创建好的二维码对象的指针
 */
lv_obj_t * lv_qrcode_create(lv_obj_t * parent);

/**
 * Set QR code size.
 * 设置二维码尺寸
 * @param obj pointer to a QR code object
 * @param obj 指向二维码对象
 * @param size width and height of the QR code
 * @param size 二维码的宽度和高度
 */
void lv_qrcode_set_size(lv_obj_t * obj, int32_t size);

/**
 * Set QR code dark color.
 * 设置二维码深色颜色
 * @param obj pointer to a QR code object
 * @param obj 指向二维码对象
 * @param color dark color of the QR code
 * @param color 二维码的深色颜色
 */
void lv_qrcode_set_dark_color(lv_obj_t * obj, lv_color_t color);

/**
 * Set QR code light color.
 * 设置二维码浅色颜色
 * @param obj pointer to a QR code object
 * @param obj 指向二维码对象
 * @param color light color of the QR code
 * @param color 二维码的浅色颜色
 */
void lv_qrcode_set_light_color(lv_obj_t * obj, lv_color_t color);

/**
 * Set the data of a QR code object
 * 设置二维码对象的数据
 * @param obj pointer to a QR code object
 * @param obj 指向二维码对象
 * @param data data to display
 * @param data 要显示的数据
 * @param data_len length of data in bytes
 * @param data_len 数据长度(以字节为单位)
 * @return LV_RESULT_OK: if no error; LV_RESULT_INVALID: on error
 * @return LV_RESULT_OK: 无错误；LV_RESULT_INVALID: 发生错误
 */
lv_result_t lv_qrcode_update(lv_obj_t * obj, const void * data, uint32_t data_len);

/**
 * Helper function to set the data of a QR code object
 * 设置二维码对象数据的辅助函数
 * @param obj pointer to a QR code object
 * @param obj 指向二维码对象
 * @param data data to display as a string
 * @param data 要以字符串形式显示的数据
 */
void lv_qrcode_set_data(lv_obj_t * obj, const char * data);

/**
 * Enable or disable quiet zone.
 * 启用或禁用静区(空白边距)
 * Quiet zone is the area around the QR code where no data is encoded.
 * 静区是二维码周围不编码数据的区域。
 * @param obj pointer to a QR code object
 * @param obj 指向二维码对象
 * @param enable true: enable quiet zone; false: disable quiet zone
 * @param enable true: 启用静区；false: 禁用静区
 */
void lv_qrcode_set_quiet_zone(lv_obj_t * obj, bool enable);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_QRCODE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_QRCODE_H*/
