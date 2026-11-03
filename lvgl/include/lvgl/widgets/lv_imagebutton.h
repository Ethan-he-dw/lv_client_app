/**
 * @file lv_imagebutton.h
 * 图片按钮(Image Button)控件：由左、中、右三部分图片拼接而成的按钮
 *
 */

#ifndef LV_IMAGEBUTTON_H
#define LV_IMAGEBUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../core/lv_obj.h"

#if LV_USE_IMAGEBUTTON != 0

/*********************
 *      DEFINES
 *      宏定义
 *********************/
typedef enum {
    /*释放（未按下）状态*/
    LV_IMAGEBUTTON_STATE_RELEASED,
    /*按下状态*/
    LV_IMAGEBUTTON_STATE_PRESSED,
    /*禁用状态*/
    LV_IMAGEBUTTON_STATE_DISABLED,
    /*选中且释放状态*/
    LV_IMAGEBUTTON_STATE_CHECKED_RELEASED,
    /*选中且按下状态*/
    LV_IMAGEBUTTON_STATE_CHECKED_PRESSED,
    /*选中且禁用状态*/
    LV_IMAGEBUTTON_STATE_CHECKED_DISABLED,
    /*状态数量，用于定义数组大小*/
    LV_IMAGEBUTTON_STATE_NUM,
} lv_imagebutton_state_t;

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/
/*图片按钮控件的类对象*/
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_imagebutton_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create an image button object
 * 创建图片按钮对象
 * @param parent pointer to an object, it will be the parent of the new image button
 * @param parent 指向父对象的指针，它将成为新图片按钮的父对象
 * @return pointer to the created image button
 * @return 指向已创建图片按钮对象的指针
 */
lv_obj_t * lv_imagebutton_create(lv_obj_t * parent);

/*======================
 * Add/remove functions
 * 添加/移除函数
 *=====================*/

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set images for a state of the image button
 * 为图片按钮的某个状态设置图片
 * @param obj           pointer to an image button object
 * @param obj           指向图片按钮对象的指针
 * @param state         for which state set the new image
 * @param state         为哪个状态设置新图片
 * @param src_left      pointer to an image source for the left side of the button (a C array or path to
 * @param src_left      指向按钮左侧图片源的指针（C数组或文件路径）
 * a file)
 * 文件路径）
 * @param src_mid       pointer to an image source for the middle of the button (ideally 1px wide) (a C
 * @param src_mid       指向按钮中间图片源的指针（理想情况下宽1像素）（C数组或文件路径）
 * array or path to a file)
 * 文件路径）
 * @param src_right     pointer to an image source for the right side of the button (a C array or path
 * @param src_right     指向按钮右侧图片源的指针（C数组或文件路径）
 * to a file)
 * 文件路径）
 */
void lv_imagebutton_set_src(lv_obj_t * obj, lv_imagebutton_state_t state, const void * src_left,
                            const void * src_mid,
                            const void * src_right);

/**
 * Set the left image for a state of the image button
 * 为图片按钮的某个状态设置左侧图片
 * @param obj           pointer to an image button object
 * @param obj           指向图片按钮对象的指针
 * @param state         for which state set the new image
 * @param state         为哪个状态设置新图片
 * @param src_left      pointer to an image source for the left side of the button
 * @param src_left      指向按钮左侧图片源的指针
 *                      (a C array or path to a file)
 *                      （C数组或文件路径）
 */
void lv_imagebutton_set_src_left(lv_obj_t * obj, lv_imagebutton_state_t state, const void * src_left);

/**
 * Set the right image for a state of the image button
 * 为图片按钮的某个状态设置右侧图片
 * @param obj           pointer to an image button object
 * @param obj           指向图片按钮对象的指针
 * @param state         for which state set the new image
 * @param state         为哪个状态设置新图片
 * @param src_right      pointer to an image source for the right side of the button
 * @param src_right      指向按钮右侧图片源的指针
 *                      (a C array or path to a file)
 *                      （C数组或文件路径）
 */
void lv_imagebutton_set_src_right(lv_obj_t * obj, lv_imagebutton_state_t state, const void * src_right);

/**
 * Set the middle image for a state of the image button
 * 为图片按钮的某个状态设置中间图片
 * @param obj           pointer to an image button object
 * @param obj           指向图片按钮对象的指针
 * @param state         for which state set the new image
 * @param state         为哪个状态设置新图片
 * @param src_mid       pointer to an image source for the middle of the button
 * @param src_mid       指向按钮中间图片源的指针
 *                      (a C array or path to a file)
 *                      （C数组或文件路径）
 */
void lv_imagebutton_set_src_mid(lv_obj_t * obj, lv_imagebutton_state_t state, const void * src_mid);

/**
 * Use this function instead of `lv_obj_add/remove_state` to set a state manually
 * 使用此函数代替 `lv_obj_add/remove_state` 来手动设置状态
 * @param obj           pointer to an image button object
 * @param obj           指向图片按钮对象的指针
 * @param state         the new state
 * @param state         新的状态
 */
void lv_imagebutton_set_state(lv_obj_t * obj, lv_imagebutton_state_t state);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the left image in a given state
 * 获取指定状态下的左侧图片
 * @param obj           pointer to an image button object
 * @param obj           指向图片按钮对象的指针
 * @param state         the state where to get the image (from `lv_button_state_t`) `
 * @param state         要获取图片的状态（来自 `lv_button_state_t`）
 * @return              pointer to the left image source (a C array or path to a file)
 * @return              指向左侧图片源的指针（C数组或文件路径）
 */
const void * lv_imagebutton_get_src_left(lv_obj_t * obj, lv_imagebutton_state_t state);

/**
 * Get the middle image in a given state
 * 获取指定状态下的中间图片
 * @param obj           pointer to an image button object
 * @param obj           指向图片按钮对象的指针
 * @param state         the state where to get the image (from `lv_button_state_t`) `
 * @param state         要获取图片的状态（来自 `lv_button_state_t`）
 * @return              pointer to the middle image source (a C array or path to a file)
 * @return              指向中间图片源的指针（C数组或文件路径）
 */
const void * lv_imagebutton_get_src_middle(lv_obj_t * obj, lv_imagebutton_state_t state);

/**
 * Get the right image in a given state
 * 获取指定状态下的右侧图片
 * @param obj           pointer to an image button object
 * @param obj           指向图片按钮对象的指针
 * @param state         the state where to get the image (from `lv_button_state_t`) `
 * @param state         要获取图片的状态（来自 `lv_button_state_t`）
 * @return              pointer to the left image source (a C array or path to a file)
 * @return              指向左侧图片源的指针（C数组或文件路径）
 */
const void * lv_imagebutton_get_src_right(lv_obj_t * obj, lv_imagebutton_state_t state);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_IMAGEBUTTON*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_IMAGEBUTTON_H*/
