/**
 * @file lv_list.h
 * 列表控件头文件：用于创建由文本、按钮等列表项组成的列表
 *
 */

#ifndef LV_LIST_H
#define LV_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../core/lv_obj.h"

#if LV_USE_LIST

#if LV_USE_FLEX == 0
#error "lv_list: lv_flex is required. Enable it in lv_conf.h (LV_USE_FLEX 1)"
#endif

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/** 列表控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_list_class;
/** 列表文本控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_list_text_class;
/** 列表按钮控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_list_button_class;
/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a list object
 * 创建一个列表对象
 * @param parent    pointer to an object, it will be the parent of the new list
 * @param parent    指向一个对象，它将成为新列表的父对象
 * @return          pointer to the created list
 * @return          指向创建好的列表对象的指针
 */
lv_obj_t * lv_list_create(lv_obj_t * parent);

/**
 * Add text to a list
 * 向列表添加文本
 * @param list      pointer to a list, it will be the parent of the new label
 * @param list      指向列表，它将成为新标签的父对象
 * @param txt       text of the new label
 * @param txt       新标签的文本内容
 * @return          pointer to the created label
 * @return          指向创建好的标签对象的指针
 */
lv_obj_t * lv_list_add_text(lv_obj_t * list, const char * txt);

/**
 * Add button to a list
 * 向列表添加按钮
 * @param list      pointer to a list, it will be the parent of the new button
 * @param list      指向列表，它将成为新按钮的父对象
 * @param icon      icon for the button, when NULL it will have no icon
 * @param icon      按钮的图标，为 NULL 时不显示图标
 * @param txt       text of the new button, when NULL no text will be added
 * @param txt       新按钮的文本，为 NULL 时不添加文本
 * @return          pointer to the created button
 * @return          指向创建好的按钮对象的指针
 */
lv_obj_t * lv_list_add_button(lv_obj_t * list, const void * icon, const char * txt);

/**
 * Get text of a given list button
 * 获取指定列表按钮的文本
 * @param list      pointer to a list
 * @param list      指向列表
 * @param btn       pointer to the button
 * @param btn       指向按钮
 * @return          text of btn, if btn doesn't have text "" will be returned
 * @return          按钮的文本，若按钮没有文本则返回 ""
 */
const char * lv_list_get_button_text(lv_obj_t * list, lv_obj_t * btn);

/**
 * Set text of a given list button
 * 设置指定列表按钮的文本
 * @param list      pointer to a list
 * @param list      指向列表
 * @param btn       pointer to the button
 * @param btn       指向按钮
 * @param txt       pointer to the text
 * @param txt       指向文本内容
 */
void lv_list_set_button_text(lv_obj_t * list, lv_obj_t * btn, const char * txt);

#if LV_USE_TRANSLATION

/**
 * Add translation tag text to a list
 * 向列表添加翻译标签文本
 * @param list      pointer to a list, it will be the parent of the new label
 * @param list      指向列表，它将成为新标签的父对象
 * @param tag       translation tag of the new label
 * @param tag       新标签的翻译标签
 * @return          pointer to the created label
 * @return          指向创建好的标签对象的指针
 */
lv_obj_t * lv_list_add_translation_tag(lv_obj_t * list, const char * tag);

/**
 * Add translation tag button to a list
 * 向列表添加带翻译标签的按钮
 * @param list      pointer to a list, it will be the parent of the new button
 * @param list      指向列表，它将成为新按钮的父对象
 * @param icon      icon for the button, when NULL it will have no icon
 * @param icon      按钮的图标，为 NULL 时不显示图标
 * @param tag       translation tag of the new button, when NULL no translation tag will be added
 * @param tag       新按钮的翻译标签，为 NULL 时不添加翻译标签
 * @return          pointer to the created button
 * @return          指向创建好的按钮对象的指针
 */
lv_obj_t * lv_list_add_button_translation_tag(lv_obj_t * list, const void * icon, const char * tag);

/**
 * Set translation tag text of a given list button
 * 设置指定列表按钮的翻译标签
 * @param list      pointer to a list
 * @param list      指向列表
 * @param btn       pointer to the button
 * @param btn       指向按钮
 * @param tag       pointer to the translation tag
 * @param tag       指向翻译标签
 */
void lv_list_set_button_translation_tag(lv_obj_t * list, lv_obj_t * btn, const char * tag);

#endif

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_LIST*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LIST_H*/
