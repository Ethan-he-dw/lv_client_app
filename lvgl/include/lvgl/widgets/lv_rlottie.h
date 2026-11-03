/**
 * @file lv_rlottie.h
 *
 * RLottie动画控件
 */

#ifndef LV_RLOTTIE_H
#define LV_RLOTTIE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"
#if LV_USE_RLOTTIE

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/
/** RLottie播放控制模式 */
typedef enum {
    /** 正向播放 */
    LV_RLOTTIE_CTRL_FORWARD  = 0,
    /** 反向播放 */
    LV_RLOTTIE_CTRL_BACKWARD = 1,
    /** 暂停 */
    LV_RLOTTIE_CTRL_PAUSE    = 2,
    /** 播放（play = 0 为默认模式） */
    LV_RLOTTIE_CTRL_PLAY     = 0, /* Yes, play = 0 is the default mode */
    /** 循环播放 */
    LV_RLOTTIE_CTRL_LOOP     = 8,
} lv_rlottie_ctrl_t;

/** RLottie动画控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_rlottie_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * 从文件创建RLottie动画控件
 * @param parent    父对象
 * @param width     控件宽度
 * @param height    控件高度
 * @param path      动画文件路径
 * @return          指向创建的RLottie控件的指针
 */
lv_obj_t * lv_rlottie_create_from_file(lv_obj_t * parent, int32_t width, int32_t height, const char * path);

/**
 * 从原始数据（rlottie描述）创建RLottie动画控件
 * @param parent        父对象
 * @param width         控件宽度
 * @param height        控件高度
 * @param rlottie_desc  rlottie动画描述数据
 * @return              指向创建的RLottie控件的指针
 */
lv_obj_t * lv_rlottie_create_from_raw(lv_obj_t * parent, int32_t width, int32_t height,
                                      const char * rlottie_desc);

/**
 * 设置RLottie动画的播放模式
 * @param rlottie   指向RLottie控件
 * @param ctrl      播放控制模式（lv_rlottie_ctrl_t）
 */
void lv_rlottie_set_play_mode(lv_obj_t * rlottie, const lv_rlottie_ctrl_t ctrl);

/**
 * 设置RLottie动画跳转到指定帧
 * @param rlottie      指向RLottie控件
 * @param goto_frame   目标帧序号
 */
void lv_rlottie_set_current_frame(lv_obj_t * rlottie, const size_t goto_frame);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_RLOTTIE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_RLOTTIE_H*/
