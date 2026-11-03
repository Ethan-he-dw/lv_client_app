/**
 * @file lv_canvas.h
 * 画布(canvas)控件头文件：提供像素读写、缓冲区复制、背景填充、图层绘制等功能。
 *
 */

#ifndef LV_CANVAS_H
#define LV_CANVAS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_CANVAS

#include "lv_image.h"

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/
/** 画布控件的类结构体 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_canvas_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a canvas object
 * 创建一个画布对象
 * @param parent     pointer to an object, it will be the parent of the new canvas
 * @param parent     指向一个对象，它将成为新画布的父对象
 * @return           pointer to the created canvas
 * @return           指向创建好的画布对象的指针
 */
lv_obj_t * lv_canvas_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set a buffer for the canvas.
 * 为画布设置一个缓冲区。
 *
 * Use lv_canvas_set_draw_buf() instead if you need to set a buffer with alignment requirement.
 * 如果需要对缓冲区有对齐要求，请改用 lv_canvas_set_draw_buf()。
 *
 * @param obj    pointer to a canvas object
 * @param obj    指向画布对象的指针
 * @param buf    buffer where content of canvas will be.
 * @param buf    画布内容将存放的缓冲区。
 *                 The required size is (lv_image_color_format_get_px_size(cf) * w) / 8 * h)
 *                 所需大小为 (lv_image_color_format_get_px_size(cf) * w) / 8 * h)
 *                 It can be allocated with `lv_malloc()` or
 *                 可以使用 `lv_malloc()` 分配，
 *                 it can be statically allocated array (e.g. static lv_color_t buf[100*50]) or
 *                 也可以使用静态分配的数组(例如 static lv_color_t buf[100*50])，
 *                 it can be an address in RAM or external SRAM
 *                 还可以是 RAM 或外部 SRAM 中的地址
 * @param w      width of canvas
 * @param w      画布的宽度
 * @param h      height of canvas
 * @param h      画布的高度
 * @param cf     color format. `LV_COLOR_FORMAT...`
 * @param cf     颜色格式。`LV_COLOR_FORMAT...`
 */
void lv_canvas_set_buffer(lv_obj_t * obj, void * buf, int32_t w, int32_t h, lv_color_format_t cf);

/**
 * Set a draw buffer for the canvas. A draw buffer either can be allocated by `lv_draw_buf_create()`
 * 为画布设置一个绘制缓冲区。绘制缓冲区可以由 `lv_draw_buf_create()` 分配，
 * or defined statically by `LV_DRAW_BUF_DEFINE_STATIC`. When buffer start address and stride has alignment
 * 或通过 `LV_DRAW_BUF_DEFINE_STATIC` 静态定义。当缓冲区起始地址和步长有对齐
 * requirement, it's recommended to use `lv_draw_buf_create`.
 * 要求时，建议使用 `lv_draw_buf_create`。
 * @param obj       pointer to a canvas object
 * @param obj       指向画布对象的指针
 * @param draw_buf  pointer to a draw buffer
 * @param draw_buf  指向绘制缓冲区的指针
 */
void lv_canvas_set_draw_buf(lv_obj_t * obj, lv_draw_buf_t * draw_buf);

/**
 * Set a pixel's color and opacity
 * 设置一个像素的颜色和不透明度
 * @param obj   pointer to a canvas
 * @param obj   指向画布对象的指针
 * @param x     X coordinate of the pixel
 * @param x     像素的 X 坐标
 * @param y     Y coordinate of the pixel
 * @param y     像素的 Y 坐标
 * @param color the color
 * @param color 颜色
 * @param opa   the opacity
 * @param opa   不透明度
 * @note        The following color formats are supported
 * @note        支持以下颜色格式
 *              LV_COLOR_FORMAT_I1/2/4/8, LV_COLOR_FORMAT_A8,
 *              LV_COLOR_FORMAT_I1/2/4/8、LV_COLOR_FORMAT_A8、
 *              LV_COLOR_FORMAT_RGB565, LV_COLOR_FORMAT_RGB888,
 *              LV_COLOR_FORMAT_RGB565、LV_COLOR_FORMAT_RGB888、
 *              LV_COLOR_FORMAT_XRGB8888, LV_COLOR_FORMAT_ARGB8888
 *              LV_COLOR_FORMAT_XRGB8888、LV_COLOR_FORMAT_ARGB8888
 * @note    this function invalidates the canvas object every time,
 * @note    每次调用本函数都会使画布对象失效，
 *      for best performance, if you're changing a lot of pixels in a loop,
 *      为获得最佳性能，如果要在循环中修改大量像素，
 *      call `lv_display_enable_invalidation` before the loop starts
 *      请在循环开始前调用 `lv_display_enable_invalidation`
 *      so the invalidation isn't done on every call
 *      这样就不必在每次调用时都进行失效处理
 */
void lv_canvas_set_px(lv_obj_t * obj, int32_t x, int32_t y, lv_color_t color, lv_opa_t opa);

/**
 * Set the palette color of a canvas for index format. Valid only for `LV_COLOR_FORMAT_I1/2/4/8`
 * 为索引格式的画布设置调色板颜色。仅对 `LV_COLOR_FORMAT_I1/2/4/8` 有效
 * @param obj       pointer to canvas object
 * @param obj       指向画布对象的指针
 * @param index     the palette color to set:
 * @param index     要设置的调色板颜色索引：
 *                  - for `LV_COLOR_FORMAT_I1`: 0..1
 *                  - 对于 `LV_COLOR_FORMAT_I1`：0..1
 *                  - for `LV_COLOR_FORMAT_I2`: 0..3
 *                  - 对于 `LV_COLOR_FORMAT_I2`：0..3
 *                  - for `LV_COLOR_FORMAT_I4`: 0..15
 *                  - 对于 `LV_COLOR_FORMAT_I4`：0..15
 *                  - for `LV_COLOR_FORMAT_I8`: 0..255
 *                  - 对于 `LV_COLOR_FORMAT_I8`：0..255
 * @param color     the color to set
 * @param color     要设置的颜色
 */
void lv_canvas_set_palette(lv_obj_t * obj, uint8_t index, lv_color32_t color);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/** 获取画布对象的绘制缓冲区 */
lv_draw_buf_t * lv_canvas_get_draw_buf(lv_obj_t * obj);

/**
 * Get a pixel's color and opacity
 * 获取一个像素的颜色和不透明度
 * @param obj   pointer to a canvas
 * @param obj   指向画布对象的指针
 * @param x     X coordinate of the pixel
 * @param x     像素的 X 坐标
 * @param y     Y coordinate of the pixel
 * @param y     像素的 Y 坐标
 * @return      ARGB8888 color of the pixel
 * @return      像素的 ARGB8888 颜色
 */
lv_color32_t lv_canvas_get_px(lv_obj_t * obj, int32_t x, int32_t y);

/**
 * Get the image of the canvas as a pointer to an `lv_image_dsc_t` variable.
 * 以 `lv_image_dsc_t` 变量指针的形式获取画布的图像。
 * @param canvas    pointer to a canvas object
 * @param canvas    指向画布对象的指针
 * @return          pointer to the image descriptor.
 * @return          指向图像描述符的指针。
 */
lv_image_dsc_t * lv_canvas_get_image(lv_obj_t * canvas);

/**
 * Return the pointer for the buffer.
 * 返回缓冲区的指针。
 * It's recommended to use this function instead of the buffer form the
 * 建议使用本函数而不是 lv_canvas_get_image() 返回值中的缓冲区，
 * return value of lv_canvas_get_image() as is can be aligned
 * 因为它可以是经过对齐的
 * @param canvas    pointer to a canvas object
 * @param canvas    指向画布对象的指针
 * @return          pointer to the buffer
 * @return          指向缓冲区的指针
 */
const void * lv_canvas_get_buf(lv_obj_t * canvas);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**
 * Copy a buffer to the canvas
 * 将缓冲区数据复制到画布上
 * @param obj           pointer to a canvas object
 * @param obj           指向画布对象的指针
 * @param canvas_area   the area of the canvas to copy the new data to
 * @param canvas_area   新数据要复制到的画布区域
 * @param src_buf       pointer to a buffer holding the source data
 * @param src_buf       指向存放源数据的缓冲区的指针
 * @param src_area      the area of the source buffer to copy from. If NULL, copy the whole buffer.
 * @param src_area      要复制的源缓冲区区域。如果为 NULL，则复制整个缓冲区。
 * @note  canvas_area and src_area should be the same size. If canvas_area and the size of src_buf are the same,
 * @note  canvas_area 和 src_area 的大小应相同。如果 canvas_area 与 src_buf 的大小相同，
 *        src_area can be left NULL.
 *        则 src_area 可以保持为 NULL。
 */
void lv_canvas_copy_buf(lv_obj_t * obj, const lv_area_t * canvas_area, lv_draw_buf_t * src_buf,
                        const lv_area_t * src_area);

/**
 * Fill the canvas with color
 * 用颜色填充画布
 * @param obj       pointer to a canvas
 * @param obj       指向画布对象的指针
 * @param color     the background color
 * @param color     背景颜色
 * @param opa       the desired opacity
 * @param opa       期望的不透明度
 */
void lv_canvas_fill_bg(lv_obj_t * obj, lv_color_t color, lv_opa_t opa);

/**
 * Initialize a layer to use LVGL's generic draw functions (lv_draw_rect/label/...) on the canvas.
 * 初始化一个图层，以便在画布上使用 LVGL 的通用绘制函数(lv_draw_rect/label/...)。
 * Needs to be usd in pair with `lv_canvas_finish_layer`.
 * 需要与 `lv_canvas_finish_layer` 成对使用。
 * @param canvas    pointer to a canvas
 * @param canvas    指向画布对象的指针
 * @param layer     pointer to a layer variable to initialize
 * @param layer     指向要初始化的图层变量的指针
 */
void lv_canvas_init_layer(lv_obj_t * canvas, lv_layer_t * layer);

/**
 * Wait until all the drawings are finished on layer.
 * 等待图层上的所有绘制操作完成。
 * Needs to be usd in pair with `lv_canvas_init_layer`.
 * 需要与 `lv_canvas_init_layer` 成对使用。
 * @param canvas    pointer to a canvas
 * @param canvas    指向画布对象的指针
 * @param layer     pointer to a layer to finalize
 * @param layer     指向要结束绘制的图层的指针
 */
void lv_canvas_finish_layer(lv_obj_t * canvas, lv_layer_t * layer);

/**********************
 *      MACROS
 * 宏
 **********************/

/** 计算画布缓冲区所需的大小(字节，含对齐) */
#define LV_CANVAS_BUF_SIZE(w, h, bpp, stride) (((((w * bpp + 7) >> 3) + stride - 1) & ~(stride - 1)) * h + LV_DRAW_BUF_ALIGN)

/**
 * Just a wrapper to `LV_CANVAS_BUF_SIZE` for bindings.
 * 仅为绑定(bindings)提供的 `LV_CANVAS_BUF_SIZE` 封装函数。
 */
uint32_t lv_canvas_buf_size(int32_t w, int32_t h, uint8_t bpp, uint8_t stride);

#endif /*LV_USE_CANVAS*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CANVAS_H*/
