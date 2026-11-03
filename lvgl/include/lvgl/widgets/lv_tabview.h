/**
 * @file lv_tabview.h
 * @brief 标签页(tabview)控件
 */

#ifndef LV_TABVIEW_H
#define LV_TABVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"
#include "../core/lv_obj.h"
#include "../core/lv_obj_property.h"

#if LV_USE_TABVIEW

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/*标签页控件类(lv_obj_class_t)*/
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_tabview_class;

#if LV_USE_OBJ_PROPERTY
/*标签页对象属性ID枚举*/
enum _lv_property_tabview_id_t {
    LV_PROPERTY_ID(TABVIEW, TAB_ACTIVE,       LV_PROPERTY_TYPE_INT, 0),   /*当前激活的标签页索引*/
    LV_PROPERTY_ID(TABVIEW, TAB_BAR_POSITION, LV_PROPERTY_TYPE_INT, 1),   /*标签栏的位置*/
    LV_PROPERTY_TABVIEW_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a tabview widget
 * 创建一个标签页控件
 * @param parent    pointer to a parent widget
 *                 指向父控件的指针
 * @return          the created tabview
 *                 创建出的标签页对象
 */
lv_obj_t * lv_tabview_create(lv_obj_t * parent);

/**
 * Add a tab to the tabview
 * 向标签页添加一个标签页
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @param name      the name of the tab, it will be displayed on the tab bar
 *                 标签页的名称，将显示在标签栏上
 * @return          the widget where the content of the tab can be created
 *                 可在其中创建该标签页内容的容器对象
 */
lv_obj_t * lv_tabview_add_tab(lv_obj_t * obj, const char * name);

/**
 * Change the name of the tab
 * 修改标签页的名称
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @param idx       the index of the tab to rename
 *                 要重命名的标签页索引
 * @param new_name  the new name as a string
 *                 新的名称字符串
 */
void lv_tabview_set_tab_text(lv_obj_t * obj, uint32_t idx, const char * new_name);

#if LV_USE_TRANSLATION

/**
 * Add a tab with a translation tag to the tabview.
 * 向标签页添加一个带翻译标签(tag)的标签页。
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @param tag       translation key used for the tab label; will be displayed on the tab bar
 *                 用于标签页标签的翻译键；将显示在标签栏上
 * @return          the widget where the content of the tab can be created
 *                 可在其中创建该标签页内容的容器对象
 */
lv_obj_t * lv_tabview_set_tab_translation_tag(lv_obj_t * obj, const char * tag);

#endif

/**
 * Show a tab
 * 显示某个标签页
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @param idx       the index of the tab to show
 *                 要显示的标签页索引
 * @param anim_en   LV_ANIM_ON/OFF
 *                 是否启用切换动画 (LV_ANIM_ON/OFF)
 */
void lv_tabview_set_active(lv_obj_t * obj, uint32_t idx, lv_anim_enable_t anim_en);

/**
 * Set the position of the tab bar
 * 设置标签栏的位置
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @param dir       LV_DIR_TOP/BOTTOM/LEFT/RIGHT
 *                 标签栏方向 (LV_DIR_TOP/BOTTOM/LEFT/RIGHT)
 */
void lv_tabview_set_tab_bar_position(lv_obj_t * obj, lv_dir_t dir);

/**
 * Set the width or height of the tab bar
 * 设置标签栏的宽度或高度
 * @param obj       pointer to tabview widget
 *                 指向标签页控件的指针
 * @param size      size of the tab bar in pixels or percentage.
 *                  will be used as width or height based on the position of the tab bar)
 *                 标签栏的尺寸(像素或百分比)。
 *                 根据标签栏的位置，该值将作为宽度或高度使用)
 */
void lv_tabview_set_tab_bar_size(lv_obj_t * obj, int32_t size);

/**
 * Get the number of tabs
 * 获取标签页的数量
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @return          the number of tabs
 *                 标签页的数量
 */
uint32_t lv_tabview_get_tab_count(lv_obj_t * obj);

/**
 * Get the current tab's index
 * 获取当前标签页的索引
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @return          the zero based index of the current tab
 *                 当前标签页从 0 开始的索引
 */
uint32_t lv_tabview_get_tab_active(lv_obj_t * obj);

/**
 * Get a given tab button by index
 * 按索引获取指定的标签页按钮
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @param idx       zero based index of the tab button to get.
 *                  < 0 means start counting tab button from the back (-1 is the last tab button)
 *                 要获取的标签页按钮从 0 开始的索引。
 *                 < 0 表示从末尾开始倒数标签页按钮(-1 表示最后一个标签页按钮)
 * @return          pointer to the tab button, or NULL if the index was out of range
 *                 指向标签页按钮的指针，若索引超出范围则返回 NULL
 */
lv_obj_t * lv_tabview_get_tab_button(lv_obj_t * obj, int32_t idx);

/**
 * Get the widget where the container of each tab is created
 * 获取用于创建各标签页内容的容器控件
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @return          the main container widget
 *                 主容器控件
 */
lv_obj_t * lv_tabview_get_content(lv_obj_t * obj);

/**
 * Get the tab bar where the buttons are created
 * 获取用于创建标签页按钮的标签栏
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @return          the tab bar
 *                 标签栏对象
 */
lv_obj_t * lv_tabview_get_tab_bar(lv_obj_t * obj);

/**
 * Get the position of the tab bar
 * 获取标签栏的位置
 * @param obj       pointer to a tabview widget
 *                 指向标签页控件的指针
 * @return          LV_DIR_TOP/BOTTOM/LEFT/RIGHT
 *                 标签栏方向 (LV_DIR_TOP/BOTTOM/LEFT/RIGHT)
 */
lv_dir_t lv_tabview_get_tab_bar_position(lv_obj_t * obj);

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_TABVIEW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TABVIEW_H*/
