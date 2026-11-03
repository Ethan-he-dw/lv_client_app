/**
 * @file lv_win.h
 *
 * 窗口控件
 */

#ifndef LV_WIN_H
#define LV_WIN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"
#include "../core/lv_obj.h"
#if LV_USE_WIN
/*********************
 *      DEFINES
 *      宏定义
 *********************/

/** 窗口控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_win_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a window widget
 * 创建一个窗口控件
 * @param parent    pointer to a parent widget
 *                  指向父控件
 * @return          the created window
 *                  创建的窗口
 */
lv_obj_t * lv_win_create(lv_obj_t * parent);

/**
 * Add a title to the window
 * 向窗口添加标题
 * @param obj       pointer to a window widget
 *                  指向窗口控件
 * @param txt       the text of the title
 *                  标题文本
 * @return          the widget where the content of the title can be created
 *                  可在其中创建标题内容的控件
 */
lv_obj_t * lv_win_add_title(lv_obj_t * win, const char * txt);

/**
 * Add a button to the window
 * 向窗口添加按钮
 * @param obj       pointer to a window widget
 *                  指向窗口控件
 * @param icon      an icon to be displayed on the button
 *                  要显示在按钮上的图标
 * @param btn_w     width of the button
 *                  按钮的宽度
 * @return          the widget where the content of the button can be created
 *                  可在其中创建按钮内容的控件
 */
lv_obj_t * lv_win_add_button(lv_obj_t * win, const void * icon, int32_t btn_w);

/**
 * Get the header of the window
 * 获取窗口的标题栏（头部）
 * @param win       pointer to a window widget
 *                  指向窗口控件
 * @return          the header of the window
 *                  窗口的标题栏
 */
lv_obj_t * lv_win_get_header(lv_obj_t * win);

/**
 * Get the content of the window
 * 获取窗口的内容区域
 * @param win       pointer to a window widget
 *                  指向窗口控件
 * @return          the content of the window
 *                  窗口的内容区域
 */
lv_obj_t * lv_win_get_content(lv_obj_t * win);
/**********************
 *      MACROS
 *      宏
 **********************/
#endif /*LV_USE_WIN*/
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_WIN_H*/
