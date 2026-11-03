/**
 * @file lv_checkbox.h
 *
 * 复选框控件
 */

#ifndef LV_CHECKBOX_H
#define LV_CHECKBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"
#include "../core/lv_obj.h"

#if LV_USE_CHECKBOX != 0

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/** 复选框控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_checkbox_class;

#if LV_USE_OBJ_PROPERTY
/** 复选框控件属性ID枚举（用于对象属性系统） */
enum _lv_property_checkbox_id_t {
    /** 复选框文本属性 */
    LV_PROPERTY_ID(CHECKBOX, TEXT, LV_PROPERTY_TYPE_TEXT, 0),
    LV_PROPERTY_CHECKBOX_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a check box object
 * 创建一个复选框对象
 * @param parent    pointer to an object, it will be the parent of the new button
 *                  指向一个对象，它将成为新按钮的父对象
 * @return          pointer to the created check box
 *                  指向创建的复选框的指针
 */
lv_obj_t * lv_checkbox_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the text of a check box. `txt` will be copied and may be deallocated
 * after this function returns.
 * 设置复选框的文本。`txt` 会被复制，在此函数返回后即可将其释放。
 * @param obj   pointer to a check box
 *              指向一个复选框
 * @param txt   the text of the check box. NULL to refresh with the current text.
 *              复选框的文本。传NULL表示用当前文本刷新。
 */
void lv_checkbox_set_text(lv_obj_t * obj, const char * txt);

/**
 * Set the text of a check box. `txt` must not be deallocated during the life
 * of this checkbox.
 * 设置复选框的文本。`txt` 在该复选框的整个生命周期内不能被释放。
 * @param obj   pointer to a check box
 *              指向一个复选框
 * @param txt   the text of the check box.
 *              复选框的文本。
 */
void lv_checkbox_set_text_static(lv_obj_t * obj, const char * txt);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the text of a check box
 * 获取复选框的文本
 * @param obj   pointer to check box object
 *              指向复选框对象
 * @return      pointer to the text of the check box
 *              指向复选框文本的指针
 */
const char * lv_checkbox_get_text(const lv_obj_t * obj);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_CHECKBOX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CHECKBOX_H*/
