/**
 * @file lv_lottie.h
 * Lottie 动画控件头文件：用于播放 Lottie 格式的动画
 *
 */

#ifndef LV_LOTTIE_H
#define LV_LOTTIE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../lv_types.h"
#if LV_USE_LOTTIE

#include "../draw/lv_draw_buf.h"

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
 * Create a lottie animation
 * 创建一个 Lottie 动画控件
 * @param parent    pointer to the parent widget
 * @param parent    指向父控件
 * @return          pointer to the created Lottie animation widget
 * @return          指向创建好的 Lottie 动画控件的指针
 */
lv_obj_t  * lv_lottie_create(lv_obj_t * parent);

/**
 * Set a buffer for the animation. It also defines the size of the animation
 * 为动画设置缓冲区，同时该缓冲区也决定了动画的尺寸
 * @param obj       pointer to a lottie widget
 * @param obj       指向 lottie 控件
 * @param w         width of the animation and buffer
 * @param w         动画和缓冲区的宽度
 * @param h         height of the animation and buffer
 * @param h         动画和缓冲区的高度
 * @param buf       a static buffer with `width x height x 4` byte size
 * @param buf       一个静态缓冲区，大小为 `宽度 x 高度 x 4` 字节
 */
void lv_lottie_set_buffer(lv_obj_t * obj, int32_t w, int32_t h, void * buf);

/**
 * Set a draw buffer for the animation. It also defines the size of the animation
 * 为动画设置绘制缓冲区，同时该缓冲区也决定了动画的尺寸
 * @param obj       pointer to a lottie widget
 * @param obj       指向 lottie 控件
 * @param draw_buf  an initialized draw buffer with ARGB8888 color format
 * @param draw_buf  一个已初始化的 ARGB8888 颜色格式的绘制缓冲区
 */
void lv_lottie_set_draw_buf(lv_obj_t * obj, lv_draw_buf_t * draw_buf);

/**
 * Set the source for the animation as an array
 * 以数组形式设置动画的数据源
 * @param obj       pointer to a lottie widget
 * @param obj       指向 lottie 控件
 * @param src       the lottie animation converted to an nul terminated array
 * @param src       将 lottie 动画转换成的以 NUL 结尾的数组
 * @param src_size  size of the source array in bytes
 * @param src_size  数据源数组的大小(以字节为单位)
 */
void lv_lottie_set_src_data(lv_obj_t * obj, const void * src, size_t src_size);

/**
 * Set the source for the animation as a path.
 * 以文件路径形式设置动画的数据源。
 * Lottie doesn't use LVGL's File System API.
 * Lottie 不使用 LVGL 的文件系统 API。
 * @param obj       pointer to a lottie widget
 * @param obj       指向 lottie 控件
 * @param src       path to a json file, e.g. "path/to/file.json"
 * @param src       json 文件的路径，例如 "path/to/file.json"
 */
void lv_lottie_set_src_file(lv_obj_t * obj, const char * src);

/**
 * Get the LVGL animation which controls the lottie animation
 * 获取控制 lottie 动画的 LVGL 动画对象
 * @param obj       pointer to a lottie widget
 * @param obj       指向 lottie 控件
 * @return          the LVGL animation
 * @return          LVGL 动画对象
 */
lv_anim_t * lv_lottie_get_anim(lv_obj_t * obj);

/**********************
 * GLOBAL VARIABLES
 * 全局变量
 **********************/

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_LOTTIE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LOTTIE_H*/
