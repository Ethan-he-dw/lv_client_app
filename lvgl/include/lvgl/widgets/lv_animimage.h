/**
 * @file lv_animimage.h
 * 动画图像控件:将一系列图像按照动画方式播放显示
 *
 */

/**
 * Modified by NXP in 2025
 * 由 NXP 于 2025 年修改
 */

#ifndef LV_ANIMIMAGE_H
#define LV_ANIMIMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "lv_image.h"
#include "../lv_types.h"

#if LV_USE_ANIMIMG


/*********************
 *      DEFINES
 * 宏定义
 *********************/

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

#if LV_USE_OBJ_PROPERTY
/** 属性系统:动画图像对象的属性 ID 枚举 */
enum _lv_property_animimage_id_t {
    LV_PROPERTY_ID2(ANIMIMAGE, SRC,         LV_PROPERTY_TYPE_POINTER,  LV_PROPERTY_TYPE_INT,  0),
    LV_PROPERTY_ID(ANIMIMAGE, DURATION,     LV_PROPERTY_TYPE_INT,   1),
    LV_PROPERTY_ID(ANIMIMAGE, REPEAT_COUNT, LV_PROPERTY_TYPE_INT,   2),
    LV_PROPERTY_ID(ANIMIMAGE, SRC_COUNT,    LV_PROPERTY_TYPE_INT,   3),
    LV_PROPERTY_ANIMIMAGE_END,
};
#endif

/** 动画图像对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_animimg_class;

/** Image parts */
/** 图像部件 */
typedef enum {
    /** 主体部件(默认) */
    LV_ANIM_IMAGE_PART_MAIN,
} lv_animimg_part_t;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create an animation image objects
 * 创建动画图像对象
 * @param parent pointer to an object, it will be the parent of the new button
 * @param parent 指向一个对象的指针,该对象将成为新对象的父对象
 * @return pointer to the created animation image object
 * @return 指向所创建的动画图像对象的指针
 */
lv_obj_t * lv_animimg_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the image animation images source.
 * 设置动画图像的图像序列源。
 * @param obj       pointer to an animation image object
 * @param obj       指向动画图像对象的指针
 * @param dsc       pointer to a series images
 * @param dsc       指向一系列图像的指针
 * @param num       images' number
 * @param num       图像数量
 */
void lv_animimg_set_src(lv_obj_t * obj, const void * dsc[], size_t num);

/**
 *  Set the images source for flip playback of animation image.
 *  设置动画图像反转(倒放)播放的图像源。
 * @param obj       pointer to an animation image object
 * @param obj       指向动画图像对象的指针
 * @param dsc       pointer to a series images
 * @param dsc       指向一系列图像的指针
 * @param num       images' number
 * @param num       图像数量
 */
void lv_animimg_set_src_reverse(lv_obj_t * obj, const void * dsc[], size_t num);

/**
 * Startup the image animation.
 * 启动图像动画。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 */
void lv_animimg_start(lv_obj_t * obj);

/**
 * Delete the image animation.
 * 删除图像动画。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 */
bool lv_animimg_delete(lv_obj_t * obj);

/**
 * Set the image animation duration time. unit:ms
 * 设置图像动画的持续时间,单位:毫秒。
 * @param obj       pointer to an animation image object
 * @param obj       指向动画图像对象的指针
 * @param duration  the duration in milliseconds
 * @param duration  持续时间,单位:毫秒
 */
void lv_animimg_set_duration(lv_obj_t * obj, uint32_t duration);

/**
 * Set the image animation repeatedly play times.
 * 设置图像动画的重复播放次数。
 * @param obj       pointer to an animation image object
 * @param obj       指向动画图像对象的指针
 * @param count     the number of times to repeat the animation
 * @param count     动画重复播放的次数
 */
void lv_animimg_set_repeat_count(lv_obj_t * obj, uint32_t count);

/**
 * Make the image animation to play back to when the forward direction is ready.
 * 使图像动画在正向播放结束时反向播放。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 * @param duration   the duration of the playback image animation in milliseconds. 0: disable playback
 * @param duration   反向播放动画的时长,单位:毫秒。0:禁用反向播放
 */
void lv_animimg_set_reverse_duration(lv_obj_t * obj, uint32_t duration);

/**
 * Make the image animation to play back to when the forward direction is ready.
 * 使图像动画在正向播放结束时反向播放。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 * @param duration   delay in milliseconds before starting the playback image animation.
 * @param duration   开始反向播放动画前的延时,单位:毫秒
 */
void lv_animimg_set_reverse_delay(lv_obj_t * obj, uint32_t duration);

/**
 * Set a function call when the animation image really starts (considering `delay`)
 * 设置在动画图像真正开始播放时(考虑 `delay` 延时)调用的回调函数。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 * @param start_cb   a function call when the animation is start
 * @param start_cb   动画开始播放时调用的回调函数
 */
void lv_animimg_set_start_cb(lv_obj_t * obj, lv_anim_start_cb_t start_cb);

/**
 * Set a function call when the animation is completed
 * 设置动画播放完成时调用的回调函数。
 * @param obj pointer to an animation image object
 * @param obj 指向动画图像对象的指针
 * @param completed_cb  a function call when the animation is completed
 * @param completed_cb  动画播放完成时调用的回调函数
 */
void lv_animimg_set_completed_cb(lv_obj_t * obj, lv_anim_completed_cb_t completed_cb);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the image animation images source.
 * 获取动画图像的图像序列源。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 * @return a     pointer that will point to a series images
 * @return      指向一系列图像的指针
 */
const void ** lv_animimg_get_src(lv_obj_t * obj);

/**
 * Get the image animation images source.
 * 获取动画图像的图像数量。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 * @return      the number of source images
 * @return      源图像的数量
 */
uint8_t lv_animimg_get_src_count(lv_obj_t * obj);

/**
 * Get the image animation duration time. unit:ms
 * 获取图像动画的持续时间,单位:毫秒。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 * @return      the animation duration time
 * @return      动画持续时间
 */
uint32_t lv_animimg_get_duration(lv_obj_t * obj);

/**
 * Get the image animation repeat play times.
 * 获取图像动画的重复播放次数。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 * @return      the repeat count
 * @return      重复播放次数
 */
uint32_t lv_animimg_get_repeat_count(lv_obj_t * obj);

/**
 * Get the image animation underlying animation.
 * 获取图像动画底层的动画对象。
 * @param obj   pointer to an animation image object
 * @param obj   指向动画图像对象的指针
 * @return      the animation reference
 * @return      底层动画的引用
 */
lv_anim_t * lv_animimg_get_anim(lv_obj_t * obj);

#endif /*LV_USE_ANIMIMG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_ANIMIMAGE_H*/
