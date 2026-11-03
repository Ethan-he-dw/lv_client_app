/**
 * @file lv_gif.h
 * GIF 动图控件头文件：提供 GIF 的加载、播放、暂停、恢复等功能。
 *
 */

#ifndef LV_GIF_H
#define LV_GIF_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/

#include "../config/lv_conf_internal.h"

#if LV_USE_GIF

#include "../draw/lv_color.h"

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

/** GIF 动图控件的类结构体 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_gif_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a gif object
 * 创建一个 GIF 动图对象
 * @param parent    pointer to an object, it will be the parent of the new gif.
 * @param parent    指向一个对象，它将成为新 GIF 对象的父对象。
 * @return          pointer to the gif obj
 * @return          指向 GIF 对象的指针
 */
lv_obj_t * lv_gif_create(lv_obj_t * parent);

/**
 * Set the color format of the internally allocated framebuffer that the gif
 * 设置 GIF 解码到的内部帧缓冲区的颜色格式。
 * will be decoded to. The default is LV_COLOR_FORMAT_ARGB8888.
 * 默认值为 LV_COLOR_FORMAT_ARGB8888。
 * Call this before `lv_gif_set_src` to avoid reallocating the framebuffer.
 * 在调用 `lv_gif_set_src` 之前调用本函数，以避免重新分配帧缓冲区。
 * @param obj            pointer to a gif object
 * @param obj            指向 GIF 对象的指针
 * @param color_format   the color format of the gif framebuffer
 * @param color_format   GIF 帧缓冲区的颜色格式
 */
void lv_gif_set_color_format(lv_obj_t * obj, lv_color_format_t color_format);

/**
 * Set the gif data to display on the object
 * 设置要在对象上显示的 GIF 数据
 * @param obj       pointer to a gif object
 * @param obj       指向 GIF 对象的指针
 * @param src       1) pointer to an ::lv_image_dsc_t descriptor (which contains gif raw data) or
 * @param src       1) 指向 ::lv_image_dsc_t 描述符的指针(其中包含 GIF 原始数据)或
 *                  2) path to a gif file (e.g. "S:/dir/anim.gif")
 *                  2) GIF 文件的路径(例如 "S:/dir/anim.gif")
 */
void lv_gif_set_src(lv_obj_t * obj, const void * src);

/**
 * Restart a gif animation.
 * 重新开始 GIF 动画。
 * @param obj pointer to a gif obj
 * @param obj 指向 GIF 对象的指针
 */
void lv_gif_restart(lv_obj_t * obj);

/**
 * Pause a gif animation.
 * 暂停 GIF 动画。
 * @param obj pointer to a gif obj
 * @param obj 指向 GIF 对象的指针
 */
void lv_gif_pause(lv_obj_t * obj);

/**
 * Resume a gif animation.
 * 恢复 GIF 动画。
 * @param obj pointer to a gif obj
 * @param obj 指向 GIF 对象的指针
 */
void lv_gif_resume(lv_obj_t * obj);

/**
 * Checks if the GIF was loaded correctly.
 * 检查 GIF 是否已正确加载。
 * @param obj pointer to a gif obj
 * @param obj 指向 GIF 对象的指针
 */
bool lv_gif_is_loaded(lv_obj_t * obj);

/**
 * Get the loop count for the GIF.
 * 获取 GIF 的循环次数。
 * @param obj pointer to a gif obj
 * @param obj 指向 GIF 对象的指针
 */
int32_t lv_gif_get_loop_count(lv_obj_t * obj);

/**
 * Set the loop count for the GIF.
 * 设置 GIF 的循环次数。
 * @param obj   pointer to a gif obj
 * @param obj   指向 GIF 对象的指针
 * @param count the loop count to set
 * @param count 要设置的循环次数
 */
void lv_gif_set_loop_count(lv_obj_t * obj, int32_t count);

/**
 * Set whether to decode invisible object.
 * 设置是否解码不可见的对象。
 * @param obj        pointer to a gif object
 * @param obj        指向 GIF 对象的指针
 * @param auto_pause true: auto pause when invisible, false: don't auto pause
 * @param auto_pause true：不可见时自动暂停；false：不自动暂停
 */
void lv_gif_set_auto_pause_invisible(lv_obj_t * obj, bool auto_pause);

/**
 * Get gif width & height
 * 获取 GIF 的宽度和高度
 * @param src pointer to a gif file
 * @param src 指向 GIF 文件的指针
 * @param w pointer to store width
 * @param w 用于存储宽度的指针
 * @param h pointer to store height
 * @param h 用于存储高度的指针
 * @return true: success; false: failed
 * @return true：成功；false：失败
 */
bool lv_gif_get_size(const char * src, uint16_t * w, uint16_t * h);

/**
 * Get frame count of the GIF.
 * 获取 GIF 的帧数。
 * @param obj pointer to a gif object
 * @param obj 指向 GIF 对象的指针
 * @return frame count of the GIF
 * @return GIF 的帧数
 */
int32_t lv_gif_get_frame_count(lv_obj_t * obj);

/**
 * Get the current frame index of the GIF.
 * 获取 GIF 的当前帧索引。
 * @param obj pointer to a gif object
 * @param obj 指向 GIF 对象的指针
 * @return current frame index of the GIF
 * @return GIF 的当前帧索引
 */
int32_t lv_gif_get_current_frame_index(lv_obj_t * obj);

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_GIF*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_GIF_H*/
