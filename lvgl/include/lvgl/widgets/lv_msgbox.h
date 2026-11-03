/**
 * @file lv_msgbox.h
 * 消息框控件头文件：用于创建带标题、内容和按钮的模态或非模态消息框
 *
 */

#ifndef LV_MSGBOX_H
#define LV_MSGBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../core/lv_obj.h"

#if LV_USE_MSGBOX

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/** 消息框控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_msgbox_class;
/** 消息框头部控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_msgbox_header_class;
/** 消息框内容区域控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_msgbox_content_class;
/** 消息框底部区域控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_msgbox_footer_class;
/** 消息框头部按钮控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_msgbox_header_button_class;
/** 消息框底部按钮控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_msgbox_footer_button_class;
/** 消息框遮罩(背景)控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_msgbox_backdrop_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create an empty message box
 * 创建一个空的消息框
 * @param parent        the parent or NULL to create a modal msgbox
 * @param parent        父对象，传 NULL 可创建模态消息框
 * @return              the created message box
 * @return              创建好的消息框对象
 */
lv_obj_t * lv_msgbox_create(lv_obj_t * parent);

/**
 * Add title to the message box. It also creates a header for the title.
 * 向消息框添加标题，同时会为标题创建头部区域
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @param title         the text of the tile
 * @param title         标题的文本内容
 * @return              the created title label
 * @return              创建好的标题标签对象
 */
lv_obj_t * lv_msgbox_add_title(lv_obj_t * obj, const char * title);

/**
 * Add a button to the header of to the message box. It also creates a header.
 * 向消息框头部添加按钮，同时会创建头部区域
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @param icon          the icon of the button
 * @param icon          按钮的图标
 * @return              the created button
 * @return              创建好的按钮对象
 */
lv_obj_t * lv_msgbox_add_header_button(lv_obj_t * obj, const void * icon);

/**
 * Add a text to the content area of message box. Multiple texts will be created below each other.
 * 向消息框内容区域添加文本，多个文本将依次上下排列
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @param text          text to add
 * @param text          要添加的文本
 * @return              the created label
 * @return              创建好的标签对象
 */
lv_obj_t * lv_msgbox_add_text(lv_obj_t * obj, const char * text);

/**
 * Add a formatted text to the content area of message box. Multiple texts will be created below each other.
 * 向消息框内容区域添加格式化文本，多个文本将依次上下排列
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @param fmt           `printf`-like format string
 * @param fmt           类似 `printf` 的格式化字符串
 * @return              the created label
 * @return              创建好的标签对象
 */
lv_obj_t * lv_msgbox_add_text_fmt(lv_obj_t * obj, const char * fmt, ...) LV_FORMAT_ATTRIBUTE(2, 3);

/**
 * Add a button to the footer of to the message box. It also creates a footer.
 * 向消息框底部区域添加按钮，同时会创建底部区域
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @param text          the text of the button
 * @param text          按钮的文本
 * @return              the created button
 * @return              创建好的按钮对象
 */
lv_obj_t * lv_msgbox_add_footer_button(lv_obj_t * obj, const char * text);

/**
 * Add a close button to the message box. It also creates a header.
 * 向消息框添加关闭按钮，同时会创建头部区域
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @return              the created close button
 * @return              创建好的关闭按钮对象
 */
lv_obj_t * lv_msgbox_add_close_button(lv_obj_t * obj);

/**
 * Get the header widget
 * 获取头部控件
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @return              the header, or NULL if not exists
 * @return              头部控件，若不存在则返回 NULL
 */
lv_obj_t * lv_msgbox_get_header(lv_obj_t * obj);

/**
 * Get the footer widget
 * 获取底部区域控件
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @return              the footer, or NULL if not exists
 * @return              底部区域控件，若不存在则返回 NULL
 */
lv_obj_t * lv_msgbox_get_footer(lv_obj_t * obj);

/**
 * Get the content widget
 * 获取内容区域控件
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @return              the content
 * @return              内容区域控件
 */
lv_obj_t * lv_msgbox_get_content(lv_obj_t * obj);

/**
 * Get the title label
 * 获取标题标签
 * @param obj           pointer to a message box
 * @param obj           指向消息框
 * @return              the title, or NULL if it does not exist
 * @return              标题标签，若不存在则返回 NULL
 */
lv_obj_t * lv_msgbox_get_title(lv_obj_t * obj);

/**
 * Close a message box
 * 关闭消息框
 * @param mbox           pointer to a message box
 * @param mbox           指向消息框
 */
void lv_msgbox_close(lv_obj_t * mbox);

/**
 * Close a message box in the next call of the message box
 * 在消息框的下一次调用中关闭消息框(异步关闭)
 * @param mbox           pointer to a message box
 * @param mbox           指向消息框
 */
void lv_msgbox_close_async(lv_obj_t * mbox);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_MSGBOX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_MSGBOX_H*/
