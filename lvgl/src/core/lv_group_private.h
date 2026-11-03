/**
 * @file lv_group_private.h
 * 组私有模块：定义焦点组(group)的内部数据结构及私有接口。
 *
 */

#ifndef LV_GROUP_PRIVATE_H
#define LV_GROUP_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../lvgl_public.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**
 * Groups can be used to logically hold objects so that they can be individually focused.
 * They are NOT for laying out objects on a screen (try layouts for that).
 * 组可以用于逻辑上容纳对象，以便它们能够被单独聚焦。
 * 组不用于在屏幕上布局对象(布局请使用 layout)。
 */
struct _lv_group_t {
#if LV_USE_EXT_DATA
    lv_ext_data_t ext_data;
#endif
    lv_ll_t obj_ll;        /**< Linked list to store the objects in the group
                            * 存储组内对象的链表 */
    lv_obj_t ** obj_focus; /**< The object in focus
                            * 处于焦点中的对象 */

    lv_group_focus_cb_t focus_cb;              /**< A function to call when a new object is focused (optional)
                                                * 新对象获得焦点时调用的函数(可选) */
    lv_group_edge_cb_t  edge_cb;               /**< A function to call when an edge is reached, no more focus
                                                    targets are available in this direction (to allow edge feedback
                                                    like a sound or a scroll bounce)
                                                * 到达边缘且该方向没有更多焦点目标时调用的函数
                                                * (用于提供边缘反馈，例如声音或滚动回弹) */

    void * user_data;

    uint8_t frozen : 1;         /**< 1: can't focus to new object
                                 * 1: 不能聚焦到新对象 */
    uint8_t editing : 1;        /**< 1: Edit mode, 0: Navigate mode
                                 * 1: 编辑模式, 0: 导航模式 */
    uint8_t refocus_policy : 1; /**< 1: Focus prev if focused on deletion. 0: Focus next if focused on
                                   deletion.
                                 * 1: 焦点对象被删除时聚焦上一个。0: 焦点对象被删除时聚焦下一个。 */
    uint8_t wrap : 1;           /**< 1: Focus next/prev can wrap at end of list. 0: Focus next/prev stops at end
                                   of list.
                                 * 1: 焦点在列表末尾可循环到另一端。0: 焦点在列表末尾停止。 */
};


/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Init the group module
 * 初始化组模块
 * @remarks Internal function, do not call directly.
 *          内部函数，请勿直接调用。
 */
void lv_group_init(void);

/**
 * Deinit the group module
 * 反初始化组模块
 * @remarks Internal function, do not call directly.
 *          内部函数，请勿直接调用。
 */
void lv_group_deinit(void);

/**********************
 *      MACROS
 *      宏定义
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GROUP_PRIVATE_H*/
