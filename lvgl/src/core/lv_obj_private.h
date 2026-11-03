/**
 * @file lv_obj_private.h
 * 对象私有模块：定义 lv_obj_t 对象结构体及其私有 API。
 *
 */

#ifndef LV_OBJ_PRIVATE_H
#define LV_OBJ_PRIVATE_H

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
 * Special, rarely used attributes.
 * They are allocated automatically if any elements is set.
 * 特殊的、很少使用的属性。
 * 只要设置了任意元素，它们就会被自动分配。
 */
typedef struct _lv_obj_spec_attr_t {
    lv_obj_t ** children;           /**< Store the pointer of the children in an array.
                                     * 以数组形式存储子对象的指针。*/
    lv_group_t * group_p;
#if LV_DRAW_TRANSFORM_USE_MATRIX
    lv_matrix_t * matrix;           /**< The transform matrix
                                     * 变换矩阵 */
#endif
    lv_event_list_t event_list;
#if LV_USE_OBJ_NAME
    const char * name;              /**< Pointer to the name
                                     * 指向名称的指针 */
#endif
    lv_point_t scroll;              /**< The current X/Y scroll offset
                                     * 当前的 X/Y 滚动偏移量 */

    int32_t ext_click_pad;          /**< Extra click padding in all direction
                                     * 各方向上的额外点击内边距 */
    int32_t ext_draw_size;          /**< EXTend the size in every direction for drawing.
                                     * 绘制时向各个方向扩展的大小。 */

    uint16_t child_cnt;             /**< Number of children
                                     * 子对象数量 */
    uint16_t scrollbar_mode : 2;    /**< How to display scrollbars, see `lv_scrollbar_mode_t`
                                     * 滚动条的显示方式，参见 `lv_scrollbar_mode_t` */
    uint16_t scroll_snap_x : 2;     /**< Where to align the snappable children horizontally, see `lv_scroll_snap_t`
                                     * 可吸附子对象在水平方向上的对齐位置，参见 `lv_scroll_snap_t` */
    uint16_t scroll_snap_y : 2;     /**< Where to align the snappable children vertically
                                     * 可吸附子对象在垂直方向上的对齐位置 */
    uint16_t scroll_dir : 4;        /**< The allowed scroll direction(s), see `lv_dir_t`
                                     * 允许的滚动方向，参见 `lv_dir_t` */
    uint16_t layer_type : 2;        /**< Cache the layer type here. Element of lv_intermediate_layer_type_t
                                     * 在此缓存图层类型。lv_intermediate_layer_type_t 的元素 */
    uint16_t name_static : 1;       /**< 1: `name` was not dynamically allocated
                                     * 1: `name` 不是动态分配的 */
    uint16_t user_flags : 8;         /**< Store custom flags
                                      * 存储自定义标志 */
} lv_obj_spec_attr_t;


struct _lv_obj_t {
#if LV_USE_EXT_DATA
    lv_ext_data_t ext_data;
#endif
    const lv_obj_class_t * class_p;
    lv_obj_t * parent;
    lv_obj_spec_attr_t * spec_attr;
    lv_obj_style_t * styles;
#if LV_OBJ_STYLE_CACHE
    uint32_t style_main_prop_is_set;
    uint32_t style_other_prop_is_set;
#endif
    void * user_data;
#if LV_USE_OBJ_ID
    void * id;
#endif
    lv_area_t coords;
    /** Make the object hidden. (Like it wasn't there at all)
     * 使对象隐藏。(就像它根本不存在一样) */
    uint32_t hidden : 1;

    /** Make the object clickable by the input devices
     * 使对象可被输入设备点击 */
    uint32_t clickable : 1;

    /** Add focused state to the object when clicked
     * 点击时给对象添加焦点状态 */
    uint32_t click_focusable : 1;

    /** Toggle checked state when the object is clicked
     * 点击对象时切换选中状态 */
    uint32_t checkable : 1;

    /** Make the object scrollable
     * 使对象可滚动 */
    uint32_t scrollable : 1;

    /** Allow scrolling inside but with slower speed
     * 允许内部滚动，但速度较慢 */
    uint32_t scroll_elastic : 1;

    /** Make the object scroll further when "thrown"
     * 使对象在“抛出”后继续滚动 */
    uint32_t scroll_momentum : 1;

    /** Allow scrolling only one snappable child
     * 只允许滚动一个可吸附的子对象 */
    uint32_t scroll_one : 1;

    /** Allow propagating the horizontal scroll to a parent
     * 允许将水平滚动传播给父对象 */
    uint32_t scroll_chain_hor : 1;

    /** Allow propagating the vertical scroll to a parent
     * 允许将垂直滚动传播给父对象 */
    uint32_t scroll_chain_ver : 1;

    /** Automatically scroll object to make it visible when focused
     * 获得焦点时自动滚动对象使其可见 */
    uint32_t scroll_on_focus : 1;

    /** Allow scrolling the focused object with arrow keys
     * 允许使用方向键滚动获得焦点的对象 */
    uint32_t scroll_with_arrow : 1;

    /** If scroll snap is enabled on the parent it can snap to this object
     * 如果父对象启用了滚动吸附，它可以吸附到这个对象上 */
    uint32_t snappable : 1;

    /** Keep the object pressed even if the press slid from the object
     * 即使按压滑出对象也保持对象为按下状态 */
    uint32_t press_lock : 1;

    /** Propagate the events to the parent too
     * 同时将事件传播给父对象 */
    uint32_t event_bubble : 1;

    /** Propagate the gestures to the parent
     * 将手势传播给父对象 */
    uint32_t gesture_bubble : 1;

    /** Allow performing more accurate hit (click) test
     * 允许执行更精确的命中(点击)测试 */
    uint32_t adv_hittest : 1;

    /** Make the object not positioned by the layouts
     * 使对象不被布局定位 */
    uint32_t ignore_layout : 1;

    /** Do not scroll the object when the parent scrolls and ignore layout
     * 父对象滚动时不随其滚动，并忽略布局 */
    uint32_t floating : 1;

    /** Send LV_EVENT_DRAW_TASK_ADDED events
     * 发送 LV_EVENT_DRAW_TASK_ADDED 事件 */
    uint32_t send_draw_task_events : 1;

    /** Do not clip the children to the parent's ext draw size
     * 不将子对象裁剪到父对象的扩展绘制大小 */
    uint32_t overflow_visible : 1;

    /** Propagate the events to the children too
     * 同时将事件传播给子对象 */
    uint32_t event_trickle : 1;

    /** Propagate the states to the children too
     * 同时将状态传播给子对象 */
    uint32_t state_trickle : 1;

    /** Allow only one RADIO_BUTTON sibling to be checked
     * 只允许选中一个 RADIO_BUTTON 兄弟对象 */
    uint32_t radio_button : 1;

    uint16_t state;
    uint16_t layout_inv : 1;
    uint16_t readjust_scroll_after_layout : 1;
    uint16_t scr_layout_inv : 1;
    uint16_t skip_trans : 1;
    uint16_t style_cnt  : 6;
    uint16_t h_layout   : 1;
    uint16_t w_layout   : 1;
    uint16_t h_ignore_size : 1; /* ignore this obj when calculating content height of parent
                                 * 计算父对象内容高度时忽略此对象 */
    uint16_t w_ignore_size : 1; /* ignore this obj when calculating content width of parent
                                 * 计算父对象内容宽度时忽略此对象 */
    uint16_t is_deleting : 1;

    /** The widget is rendered at least once already.
     * It's used to skip initial animations and transitions.
     * 该部件已被渲染至少一次。
     * 用于跳过初始动画和过渡效果。 */
    uint16_t rendered : 1;
};

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Allocate special data for an object if not allocated yet.
 * 若对象的特殊数据尚未分配，则为其分配。
 * @param obj   pointer to an object
 *              指向对象的指针
 * @return the spec_attr created or NULL if something went wrong
 *         创建的 spec_attr；若出错则返回 NULL
 */
lv_obj_spec_attr_t * lv_obj_allocate_spec_attr(lv_obj_t * obj);

lv_result_t lv_obj_add_child(lv_obj_t * parent, lv_obj_t * child);   /**< 添加子对象到父对象 */
void lv_obj_remove_child(lv_obj_t * parent, lv_obj_t * child);       /**< 从父对象移除子对象 */

/**********************
 *      MACROS
 *      宏定义
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_PRIVATE_H*/
