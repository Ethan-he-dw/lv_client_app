/**
 * @file lv_obj.h
 *
 * LVGL 核心对象（lv_obj）模块
 *
 * lv_obj 是 LVGL 中所有可见控件（Widget）的基类。它是一个通用的矩形容器对象，
 * 具有以下核心能力：
 *
 * 1. **父子层级结构** — 所有 lv_obj 组成树状结构，子对象的坐标相对于父对象
 * 2. **样式系统** — 通过样式（style）控制颜色、边框、阴影、圆角等外观
 * 3. **滚动** — 内置滚动能力，支持弹性滚动、惯性滚动、滚动吸附等
 * 4. **事件系统** — 响应点击、拖拽、手势、键盘等输入事件
 * 5. **布局** — 支持 Flex 和 Grid 布局，自动排列子对象
 * 6. **状态管理** — 支持 pressed、focused、disabled、checked 等状态
 *
 * 所有控件（如 lv_btn、lv_label、lv_slider 等）都是 lv_obj 的子类。
 *
 * 常见用法示例：
 * @code
 *   // 创建一个基础矩形容器
 *   lv_obj_t * cont = lv_obj_create(lv_screen_active());
 *   lv_obj_set_size(cont, 200, 100);
 *   lv_obj_set_pos(cont, 50, 50);
 *
 *   // 添加点击事件
 *   lv_obj_add_event_cb(cont, my_cb, LV_EVENT_CLICKED, NULL);
 *
 *   // 设置样式
 *   lv_obj_set_style_bg_color(cont, lv_color_hex(0x0000ff), 0);
 *   lv_obj_set_style_radius(cont, 10, 0);
 * @endcode
 */

#ifndef LV_OBJ_H
#define LV_OBJ_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../config/lv_conf_internal.h"

#include "../lv_types.h"
#include "lv_style.h"
#include "lv_area.h"
#include "../draw/lv_color.h"
#include "../debugging/lv_assert.h"

#include "lv_obj_tree.h"
#include "lv_obj_pos.h"
#include "lv_obj_scroll.h"
#include "lv_obj_style.h"
#include "lv_obj_draw.h"
#include "lv_obj_class.h"
#include "lv_obj_event.h"
#include "lv_obj_property.h"
#include "lv_group.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** 删除回调函数类型，当对象被删除时自动调用 */
typedef void(*lv_delete_cb_t)(void * user_data);

/**
 * 对象标志位枚举 (lv_obj_flag_t)
 *
 * 控制对象行为的标志位，可以通过位或（|）组合使用。
 * 使用 lv_obj_add_flag() / lv_obj_remove_flag() 设置，
 * 或使用对应的专用 setter/getter 函数。
 *
 * 常用标志速查：
 *   HIDDEN        — 隐藏对象（完全不存在）
 *   CLICKABLE     — 可被输入设备点击
 *   SCROLLABLE    — 可滚动（默认开启）
 *   SCROLL_ELASTIC — 弹性越界回弹
 *   SCROLL_MOMENTUM — 惯性滚动
 *   SCROLL_CHAIN  — 滚动事件传递给父对象
 *   EVENT_BUBBLE  — 事件冒泡到父对象
 *   FLOATING      — 不随父对象滚动，忽略布局
 */
typedef enum {
    LV_OBJ_FLAG_HIDDEN          = (1u << 0),  /**< 隐藏对象：完全不可见，不占布局空间，不响应事件 */
    LV_OBJ_FLAG_CLICKABLE       = (1u << 1),  /**< 可被输入设备点击，点击时触发 LV_EVENT_CLICKED */
    LV_OBJ_FLAG_CLICK_FOCUSABLE = (1u << 2),  /**< 点击时自动获得焦点 */
    LV_OBJ_FLAG_CHECKABLE       = (1u << 3),  /**< 点击时切换选中状态（LV_STATE_CHECKED） */
    LV_OBJ_FLAG_SCROLLABLE      = (1u << 4),  /**< 允许滚动，默认开启。父对象的子对象超出边界时可滚动查看 */
    LV_OBJ_FLAG_SCROLL_ELASTIC  = (1u << 5),  /**< 弹性越界：允许拖拽超出内容边界，松手后弹回（橡皮筋效果） */
    LV_OBJ_FLAG_SCROLL_MOMENTUM = (1u << 6),  /**< 惯性滚动：手指松开后继续滑动一段距离 */
    LV_OBJ_FLAG_SCROLL_ONE      = (1u << 7),  /**< 一次只滚动一个吸附子项（翻页效果） */
    LV_OBJ_FLAG_SCROLL_CHAIN_HOR = (1u << 8), /**< 水平滚动到达边界时传递给父对象继续滚动 */
    LV_OBJ_FLAG_SCROLL_CHAIN_VER = (1u << 9), /**< 垂直滚动到达边界时传递给父对象继续滚动 */
    LV_OBJ_FLAG_SCROLL_CHAIN     = (LV_OBJ_FLAG_SCROLL_CHAIN_HOR | LV_OBJ_FLAG_SCROLL_CHAIN_VER), /**< 同时启用水平和垂直滚动链传递 */
    LV_OBJ_FLAG_SCROLL_ON_FOCUS = (1u << 10),  /**< 获得焦点时自动滚动到可见区域 */
    LV_OBJ_FLAG_SCROLL_WITH_ARROW  = (1u << 11), /**< 允许用方向键滚动已聚焦的对象 */
    LV_OBJ_FLAG_SNAPPABLE       = (1u << 12), /**< 父对象启用滚动吸附时，此对象可以被吸附（翻页对齐） */
    LV_OBJ_FLAG_PRESS_LOCK      = (1u << 13), /**< 按住后即使手指滑出对象区域也保持按下状态 */
    LV_OBJ_FLAG_EVENT_BUBBLE    = (1u << 14), /**< 事件冒泡：未处理的事件传递给父对象 */
    LV_OBJ_FLAG_GESTURE_BUBBLE  = (1u << 15), /**< 手势事件冒泡给父对象 */
    LV_OBJ_FLAG_ADV_HITTEST     = (1u << 16), /**< 高精度命中测试：考虑圆角等非矩形区域 */
    LV_OBJ_FLAG_IGNORE_LAYOUT   = (1u << 17), /**< 布局忽略：不由 Flex/Grid 布局自动定位 */
    LV_OBJ_FLAG_FLOATING        = (1u << 18), /**< 浮动模式：不随父对象滚动，同时忽略布局（适合悬浮按钮等） */
    LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS = (1u << 19), /**< 发送 LV_EVENT_DRAW_TASK_ADDED 事件，用于自定义绘制管线 */
    LV_OBJ_FLAG_OVERFLOW_VISIBLE = (1u << 20),/**< 允许子对象绘制到父对象边界之外（默认会裁剪） */
    LV_OBJ_FLAG_EVENT_TRICKLE   = (1u << 21), /**< 事件向下传递给子对象 */
    LV_OBJ_FLAG_STATE_TRICKLE   = (1u << 22), /**< 状态向下传递给子对象（如 disabled 状态） */

    LV_OBJ_FLAG_LAYOUT_1        = (1u << 23), /**< 布局专用自定义标志（布局模块内部使用） */
    LV_OBJ_FLAG_LAYOUT_2        = (1u << 24), /**< 布局专用自定义标志 */
#if LV_USE_FLEX
    LV_OBJ_FLAG_FLEX_IN_NEW_TRACK = LV_OBJ_FLAG_LAYOUT_1,     /**< Flex 布局：强制此子项换行到新的一条轨道 */
#endif

    LV_OBJ_FLAG_WIDGET_1        = (1u << 25), /**< 控件专用自定义标志（控件内部使用） */
    LV_OBJ_FLAG_WIDGET_2        = (1u << 26), /**< 控件专用自定义标志 */
    LV_OBJ_FLAG_USER_1          = (1u << 27), /**< 用户自定义标志 1，可自由使用 */
    LV_OBJ_FLAG_USER_2          = (1u << 28), /**< 用户自定义标志 2 */
    LV_OBJ_FLAG_USER_3          = (1u << 29), /**< 用户自定义标志 3 */
    LV_OBJ_FLAG_USER_4          = (1u << 30), /**< 用户自定义标志 4 */
} lv_obj_flag_t;

#if LV_USE_OBJ_PROPERTY
enum _lv_signed_prop_id_t {
    /*OBJ flag properties */
    LV_PROPERTY_ID(OBJ, FLAG_START,                 LV_PROPERTY_TYPE_INT,       0),
    LV_PROPERTY_ID(OBJ, FLAG_HIDDEN,                LV_PROPERTY_TYPE_INT,       0),
    LV_PROPERTY_ID(OBJ, FLAG_CLICKABLE,             LV_PROPERTY_TYPE_INT,       1),
    LV_PROPERTY_ID(OBJ, FLAG_CLICK_FOCUSABLE,       LV_PROPERTY_TYPE_INT,       2),
    LV_PROPERTY_ID(OBJ, FLAG_CHECKABLE,             LV_PROPERTY_TYPE_INT,       3),
    LV_PROPERTY_ID(OBJ, FLAG_SCROLLABLE,            LV_PROPERTY_TYPE_INT,       4),
    LV_PROPERTY_ID(OBJ, FLAG_SCROLL_ELASTIC,        LV_PROPERTY_TYPE_INT,       5),
    LV_PROPERTY_ID(OBJ, FLAG_SCROLL_MOMENTUM,       LV_PROPERTY_TYPE_INT,       6),
    LV_PROPERTY_ID(OBJ, FLAG_SCROLL_ONE,            LV_PROPERTY_TYPE_INT,       7),
    LV_PROPERTY_ID(OBJ, FLAG_SCROLL_CHAIN_HOR,      LV_PROPERTY_TYPE_INT,       8),
    LV_PROPERTY_ID(OBJ, FLAG_SCROLL_CHAIN_VER,      LV_PROPERTY_TYPE_INT,       9),
    LV_PROPERTY_ID(OBJ, FLAG_SCROLL_ON_FOCUS,       LV_PROPERTY_TYPE_INT,       10),
    LV_PROPERTY_ID(OBJ, FLAG_SCROLL_WITH_ARROW,     LV_PROPERTY_TYPE_INT,       11),
    LV_PROPERTY_ID(OBJ, FLAG_SNAPPABLE,             LV_PROPERTY_TYPE_INT,       12),
    LV_PROPERTY_ID(OBJ, FLAG_PRESS_LOCK,            LV_PROPERTY_TYPE_INT,       13),
    LV_PROPERTY_ID(OBJ, FLAG_EVENT_BUBBLE,          LV_PROPERTY_TYPE_INT,       14),
    LV_PROPERTY_ID(OBJ, FLAG_GESTURE_BUBBLE,        LV_PROPERTY_TYPE_INT,       15),
    LV_PROPERTY_ID(OBJ, FLAG_ADV_HITTEST,           LV_PROPERTY_TYPE_INT,       16),
    LV_PROPERTY_ID(OBJ, FLAG_IGNORE_LAYOUT,         LV_PROPERTY_TYPE_INT,       17),
    LV_PROPERTY_ID(OBJ, FLAG_FLOATING,              LV_PROPERTY_TYPE_INT,       18),
    LV_PROPERTY_ID(OBJ, FLAG_SEND_DRAW_TASK_EVENTS, LV_PROPERTY_TYPE_INT,       19),
    LV_PROPERTY_ID(OBJ, FLAG_OVERFLOW_VISIBLE,      LV_PROPERTY_TYPE_INT,       20),
    LV_PROPERTY_ID(OBJ, FLAG_EVENT_TRICKLE,         LV_PROPERTY_TYPE_INT,       21),
    LV_PROPERTY_ID(OBJ, FLAG_STATE_TRICKLE,         LV_PROPERTY_TYPE_INT,       22),
    LV_PROPERTY_ID(OBJ, FLAG_LAYOUT_1,              LV_PROPERTY_TYPE_INT,       23),
    LV_PROPERTY_ID(OBJ, FLAG_LAYOUT_2,              LV_PROPERTY_TYPE_INT,       24),
    LV_PROPERTY_ID(OBJ, FLAG_FLEX_IN_NEW_TRACK,     LV_PROPERTY_TYPE_INT,       23), /*Mapped to FLAG_LAYOUT_1*/
    LV_PROPERTY_ID(OBJ, FLAG_WIDGET_1,              LV_PROPERTY_TYPE_INT,       25),
    LV_PROPERTY_ID(OBJ, FLAG_WIDGET_2,              LV_PROPERTY_TYPE_INT,       26),
    LV_PROPERTY_ID(OBJ, FLAG_USER_1,                LV_PROPERTY_TYPE_INT,       27),
    LV_PROPERTY_ID(OBJ, FLAG_USER_2,                LV_PROPERTY_TYPE_INT,       28),
    LV_PROPERTY_ID(OBJ, FLAG_USER_3,                LV_PROPERTY_TYPE_INT,       29),
    LV_PROPERTY_ID(OBJ, FLAG_USER_4,                LV_PROPERTY_TYPE_INT,       30),
    LV_PROPERTY_ID(OBJ, FLAG_END,                   LV_PROPERTY_TYPE_INT,       30),

    LV_PROPERTY_ID(OBJ, STATE_START,                LV_PROPERTY_TYPE_INT,       31),
    LV_PROPERTY_ID(OBJ, STATE_ALT,                  LV_PROPERTY_TYPE_INT,       31),
    /*1 reserved*/
    LV_PROPERTY_ID(OBJ, STATE_CHECKED,              LV_PROPERTY_TYPE_INT,       33),
    LV_PROPERTY_ID(OBJ, STATE_FOCUSED,              LV_PROPERTY_TYPE_INT,       34),
    LV_PROPERTY_ID(OBJ, STATE_FOCUS_KEY,            LV_PROPERTY_TYPE_INT,       35),
    LV_PROPERTY_ID(OBJ, STATE_EDITED,               LV_PROPERTY_TYPE_INT,       36),
    LV_PROPERTY_ID(OBJ, STATE_HOVERED,              LV_PROPERTY_TYPE_INT,       37),
    LV_PROPERTY_ID(OBJ, STATE_PRESSED,              LV_PROPERTY_TYPE_INT,       38),
    LV_PROPERTY_ID(OBJ, STATE_SCROLLED,             LV_PROPERTY_TYPE_INT,       39),
    LV_PROPERTY_ID(OBJ, STATE_DISABLED,             LV_PROPERTY_TYPE_INT,       40),
    /*2 reserved*/
    LV_PROPERTY_ID(OBJ, STATE_USER_1,               LV_PROPERTY_TYPE_INT,       43),
    LV_PROPERTY_ID(OBJ, STATE_USER_2,               LV_PROPERTY_TYPE_INT,       44),
    LV_PROPERTY_ID(OBJ, STATE_USER_3,               LV_PROPERTY_TYPE_INT,       45),
    LV_PROPERTY_ID(OBJ, STATE_USER_4,               LV_PROPERTY_TYPE_INT,       46),
    LV_PROPERTY_ID(OBJ, STATE_ANY,                  LV_PROPERTY_TYPE_INT,       47),
    LV_PROPERTY_ID(OBJ, STATE_END,                  LV_PROPERTY_TYPE_INT,       47),

    /*OBJ normal properties*/
    LV_PROPERTY_ID(OBJ, PARENT,                     LV_PROPERTY_TYPE_OBJ,       48),
    LV_PROPERTY_ID(OBJ, X,                          LV_PROPERTY_TYPE_INT,       49),
    LV_PROPERTY_ID(OBJ, Y,                          LV_PROPERTY_TYPE_INT,       50),
    LV_PROPERTY_ID(OBJ, W,                          LV_PROPERTY_TYPE_INT,       51),
    LV_PROPERTY_ID(OBJ, H,                          LV_PROPERTY_TYPE_INT,       52),
    LV_PROPERTY_ID(OBJ, CONTENT_WIDTH,              LV_PROPERTY_TYPE_INT,       53),
    LV_PROPERTY_ID(OBJ, CONTENT_HEIGHT,             LV_PROPERTY_TYPE_INT,       54),
    LV_PROPERTY_ID(OBJ, LAYOUT,                     LV_PROPERTY_TYPE_INT,       55),
    LV_PROPERTY_ID(OBJ, ALIGN,                      LV_PROPERTY_TYPE_INT,       56),
    LV_PROPERTY_ID(OBJ, SCROLLBAR_MODE,             LV_PROPERTY_TYPE_INT,       57),
    LV_PROPERTY_ID(OBJ, SCROLL_DIR,                 LV_PROPERTY_TYPE_INT,       58),
    LV_PROPERTY_ID(OBJ, SCROLL_SNAP_X,              LV_PROPERTY_TYPE_INT,       59),
    LV_PROPERTY_ID(OBJ, SCROLL_SNAP_Y,              LV_PROPERTY_TYPE_INT,       60),
    LV_PROPERTY_ID(OBJ, SCROLL_X,                   LV_PROPERTY_TYPE_INT,       61),
    LV_PROPERTY_ID(OBJ, SCROLL_Y,                   LV_PROPERTY_TYPE_INT,       62),
    LV_PROPERTY_ID(OBJ, SCROLL_TOP,                 LV_PROPERTY_TYPE_INT,       63),
    LV_PROPERTY_ID(OBJ, SCROLL_BOTTOM,              LV_PROPERTY_TYPE_INT,       64),
    LV_PROPERTY_ID(OBJ, SCROLL_LEFT,                LV_PROPERTY_TYPE_INT,       65),
    LV_PROPERTY_ID(OBJ, SCROLL_RIGHT,               LV_PROPERTY_TYPE_INT,       66),
    LV_PROPERTY_ID(OBJ, SCROLL_END,                 LV_PROPERTY_TYPE_POINT,     67),
    LV_PROPERTY_ID(OBJ, EXT_DRAW_SIZE,              LV_PROPERTY_TYPE_INT,       68),
    LV_PROPERTY_ID(OBJ, EVENT_COUNT,                LV_PROPERTY_TYPE_INT,       69),
    LV_PROPERTY_ID(OBJ, SCREEN,                     LV_PROPERTY_TYPE_OBJ,       70),
    LV_PROPERTY_ID(OBJ, DISPLAY,                    LV_PROPERTY_TYPE_POINTER,   71),
    LV_PROPERTY_ID(OBJ, CHILD_COUNT,                LV_PROPERTY_TYPE_INT,       72),
    LV_PROPERTY_ID(OBJ, INDEX,                      LV_PROPERTY_TYPE_INT,       73),

    /*Dedicated boolean properties backing the per-flag setters/getters.
     *The OBJ_FLAG_* properties above are kept for backward compatibility.*/
    LV_PROPERTY_ID(OBJ, HIDDEN,                     LV_PROPERTY_TYPE_BOOL,      74),
    LV_PROPERTY_ID(OBJ, CLICKABLE,                  LV_PROPERTY_TYPE_BOOL,      75),
    LV_PROPERTY_ID(OBJ, CLICK_FOCUSABLE,            LV_PROPERTY_TYPE_BOOL,      76),
    LV_PROPERTY_ID(OBJ, CHECKABLE,                  LV_PROPERTY_TYPE_BOOL,      77),
    LV_PROPERTY_ID(OBJ, SCROLLABLE,                 LV_PROPERTY_TYPE_BOOL,      78),
    LV_PROPERTY_ID(OBJ, SCROLL_ELASTIC,             LV_PROPERTY_TYPE_BOOL,      79),
    LV_PROPERTY_ID(OBJ, SCROLL_MOMENTUM,            LV_PROPERTY_TYPE_BOOL,      80),
    LV_PROPERTY_ID(OBJ, SCROLL_ONE,                 LV_PROPERTY_TYPE_BOOL,      81),
    LV_PROPERTY_ID(OBJ, SCROLL_CHAIN_HOR,           LV_PROPERTY_TYPE_BOOL,      82),
    LV_PROPERTY_ID(OBJ, SCROLL_CHAIN_VER,           LV_PROPERTY_TYPE_BOOL,      83),
    LV_PROPERTY_ID(OBJ, SCROLL_ON_FOCUS,            LV_PROPERTY_TYPE_BOOL,      84),
    LV_PROPERTY_ID(OBJ, SCROLL_WITH_ARROW,          LV_PROPERTY_TYPE_BOOL,      85),
    LV_PROPERTY_ID(OBJ, SNAPPABLE,                  LV_PROPERTY_TYPE_BOOL,      86),
    LV_PROPERTY_ID(OBJ, PRESS_LOCK,                 LV_PROPERTY_TYPE_BOOL,      87),
    LV_PROPERTY_ID(OBJ, EVENT_BUBBLE,               LV_PROPERTY_TYPE_BOOL,      88),
    LV_PROPERTY_ID(OBJ, GESTURE_BUBBLE,             LV_PROPERTY_TYPE_BOOL,      89),
    LV_PROPERTY_ID(OBJ, ADV_HITTEST,                LV_PROPERTY_TYPE_BOOL,      90),
    LV_PROPERTY_ID(OBJ, IGNORE_LAYOUT,              LV_PROPERTY_TYPE_BOOL,      91),
    LV_PROPERTY_ID(OBJ, FLOATING,                   LV_PROPERTY_TYPE_BOOL,      92),
    LV_PROPERTY_ID(OBJ, SEND_DRAW_TASK_EVENTS,      LV_PROPERTY_TYPE_BOOL,      93),
    LV_PROPERTY_ID(OBJ, OVERFLOW_VISIBLE,           LV_PROPERTY_TYPE_BOOL,      94),
    LV_PROPERTY_ID(OBJ, EVENT_TRICKLE,              LV_PROPERTY_TYPE_BOOL,      95),
    LV_PROPERTY_ID(OBJ, STATE_TRICKLE,              LV_PROPERTY_TYPE_BOOL,      96),
    LV_PROPERTY_ID(OBJ, FLEX_IN_NEW_TRACK,          LV_PROPERTY_TYPE_BOOL,      97),

    LV_PROPERTY_OBJ_END,
};
#endif

/**
 * 公开 lv_obj 基类对象，所有控件的类都继承自它。
 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_obj_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * 创建一个基础对象（矩形容器）
 *
 * 这是 LVGL 中最基本的创建函数。创建的对象是一个矩形容器，
 * 可以包含子对象、设置样式、响应事件、滚动等。
 *
 * @param parent    父对象指针。如果为 NULL，则创建一个屏幕（screen）。
 * @return          新创建的对象指针，失败返回 NULL
 *
 * @code
 *   // 在当前屏幕上创建一个容器
 *   lv_obj_t * cont = lv_obj_create(lv_screen_active());
 *
 *   // 创建一个新屏幕
 *   lv_obj_t * new_screen = lv_obj_create(NULL);
 * @endcode
 */
lv_obj_t * lv_obj_create(lv_obj_t * parent);

/*=====================
 * Setter functions（设置函数）
 *====================*/

/**
 * 添加一个或多个标志位（已废弃，建议使用专用 setter）
 *
 * @deprecated 请使用专用 setter，例如 lv_obj_set_hidden(obj, true)。
 * @param obj   对象指针
 * @param f     要添加的标志（可位或组合）
 */
LV_DEPRECATED("Use the dedicated lv_obj_set_<flag>() setters instead, e.g. lv_obj_set_hidden(obj, true).")
void lv_obj_add_flag(lv_obj_t * obj, lv_obj_flag_t f);

/**
 * 移除一个或多个标志位（已废弃，建议使用专用 setter）
 *
 * @deprecated 请使用专用 setter，例如 lv_obj_set_hidden(obj, false)。
 * @param obj   对象指针
 * @param f     要移除的标志（可位或组合）
 */
LV_DEPRECATED("Use the dedicated lv_obj_set_<flag>() setters instead, e.g. lv_obj_set_hidden(obj, false).")
void lv_obj_remove_flag(lv_obj_t * obj, lv_obj_flag_t f);

/**
 * 设置或清除一个或多个标志位（已废弃，建议使用专用 setter）
 *
 * @deprecated 请使用专用 setter，例如 lv_obj_set_hidden(obj, en)。
 * @param obj   对象指针
 * @param f     要操作的标志（可位或组合）
 * @param v     true: 添加标志；false: 移除标志
 */
LV_DEPRECATED("Use the dedicated lv_obj_set_<flag>() setters instead, e.g. lv_obj_set_hidden(obj, en).")
void lv_obj_set_flag(lv_obj_t * obj, lv_obj_flag_t f, bool v);

/** 隐藏/显示对象（设置 LV_OBJ_FLAG_HIDDEN 标志）
 * @param obj  对象指针
 * @param en   true: 隐藏；false: 显示 */
void lv_obj_set_hidden(lv_obj_t * obj, bool en);

/** 设置对象是否可点击（设置 LV_OBJ_FLAG_CLICKABLE 标志）
 * @param obj  对象指针
 * @param en   true: 可点击；false: 不可点击 */
void lv_obj_set_clickable(lv_obj_t * obj, bool en);

/** 设置点击时是否获得焦点（设置 LV_OBJ_FLAG_CLICK_FOCUSABLE 标志）
 * @param obj  对象指针
 * @param en   true: 点击获焦；false: 点击不获焦 */
void lv_obj_set_click_focusable(lv_obj_t * obj, bool en);

/** 设置点击时是否切换选中状态（设置 LV_OBJ_FLAG_CHECKABLE 标志）
 * @param obj  对象指针
 * @param en   true: 可切换选中；false: 不可切换 */
void lv_obj_set_checkable(lv_obj_t * obj, bool en);

/** 设置对象是否可滚动（设置 LV_OBJ_FLAG_SCROLLABLE 标志）
 * @param obj  对象指针
 * @param en   true: 可滚动；false: 不可滚动 */
void lv_obj_set_scrollable(lv_obj_t * obj, bool en);

/** 设置是否允许弹性越界滚动（设置 LV_OBJ_FLAG_SCROLL_ELASTIC 标志）
 * @param obj  对象指针
 * @param en   true: 允许弹性滚动；false: 不允许 */
void lv_obj_set_scroll_elastic(lv_obj_t * obj, bool en);

/** 设置是否允许惯性滚动（设置 LV_OBJ_FLAG_SCROLL_MOMENTUM 标志）
 * @param obj  对象指针
 * @param en   true: 允许惯性滚动；false: 不允许 */
void lv_obj_set_scroll_momentum(lv_obj_t * obj, bool en);

/** 设置是否只滚动一个吸附子项（设置 LV_OBJ_FLAG_SCROLL_ONE 标志）
 * @param obj  对象指针
 * @param en   true: 一次滚动一项；false: 自由滚动 */
void lv_obj_set_scroll_one(lv_obj_t * obj, bool en);

/** 设置滚动事件是否传递给父对象（设置 SCROLL_CHAIN 标志）
 * @param obj  对象指针
 * @param en   true: 启用链传递；false: 禁用 */
void lv_obj_set_scroll_chain(lv_obj_t * obj, bool en);

/** 设置水平滚动是否传递给父对象（设置 SCROLL_CHAIN_HOR 标志）
 * @param obj  对象指针
 * @param en   true: 启用；false: 禁用 */
void lv_obj_set_scroll_chain_hor(lv_obj_t * obj, bool en);

/** 设置垂直滚动是否传递给父对象（设置 SCROLL_CHAIN_VER 标志）
 * @param obj  对象指针
 * @param en   true: 启用；false: 禁用 */
void lv_obj_set_scroll_chain_ver(lv_obj_t * obj, bool en);

/** 设置获得焦点时是否自动滚动到可见区域（设置 SCROLL_ON_FOCUS 标志）
 * @param obj  对象指针
 * @param en   true: 自动滚动；false: 不滚动 */
void lv_obj_set_scroll_on_focus(lv_obj_t * obj, bool en);

/** 设置方向键是否可滚动已聚焦的对象（设置 SCROLL_WITH_ARROW 标志）
 * @param obj  对象指针
 * @param en   true: 可以；false: 不可以 */
void lv_obj_set_scroll_with_arrow(lv_obj_t * obj, bool en);

/** 设置对象是否可被滚动吸附（设置 SNAPPABLE 标志）
 * @param obj  对象指针
 * @param en   true: 可吸附；false: 不可吸附 */
void lv_obj_set_snappable(lv_obj_t * obj, bool en);

/** 设置按住后手指滑出是否保持按下状态（设置 PRESS_LOCK 标志）
 * @param obj  对象指针
 * @param en   true: 保持按下；false: 手指滑出即释放 */
void lv_obj_set_press_lock(lv_obj_t * obj, bool en);

/** 设置事件是否冒泡给父对象（设置 EVENT_BUBBLE 标志）
 * @param obj  对象指针
 * @param en   true: 冒泡；false: 不冒泡 */
void lv_obj_set_event_bubble(lv_obj_t * obj, bool en);

/** 设置手势是否冒泡给父对象（设置 GESTURE_BUBBLE 标志）
 * @param obj  对象指针
 * @param en   true: 冒泡；false: 不冒泡 */
void lv_obj_set_gesture_bubble(lv_obj_t * obj, bool en);

/** 设置是否启用高精度命中测试（设置 ADV_HITTEST 标志）
 * @param obj  对象指针
 * @param en   true: 考虑圆角等非矩形区域；false: 按矩形区域测试 */
void lv_obj_set_adv_hittest(lv_obj_t * obj, bool en);

/** 设置是否忽略布局自动定位（设置 IGNORE_LAYOUT 标志）
 * @param obj  对象指针
 * @param en   true: 忽略布局；false: 由布局系统定位 */
void lv_obj_set_ignore_layout(lv_obj_t * obj, bool en);

/** 设置是否浮动（不随父滚动，忽略布局，设置 FLOATING 标志）
 * @param obj  对象指针
 * @param en   true: 浮动；false: 正常 */
void lv_obj_set_floating(lv_obj_t * obj, bool en);

/** 设置是否发送绘制任务事件（设置 SEND_DRAW_TASK_EVENTS 标志）
 * @param obj  对象指针
 * @param en   true: 发送；false: 不发送 */
void lv_obj_set_send_draw_task_events(lv_obj_t * obj, bool en);

/** 设置子对象是否可绘制到父对象边界之外（设置 OVERFLOW_VISIBLE 标志）
 * @param obj  对象指针
 * @param en   true: 溢出可见；false: 裁剪溢出内容 */
void lv_obj_set_overflow_visible(lv_obj_t * obj, bool en);

/** 设置事件是否向下传递给子对象（设置 EVENT_TRICKLE 标志）
 * @param obj  对象指针
 * @param en   true: 向下传递；false: 不传递 */
void lv_obj_set_event_trickle(lv_obj_t * obj, bool en);

/** 设置状态是否向下传递给子对象（设置 STATE_TRICKLE 标志）
 * @param obj  对象指针
 * @param en   true: 向下传递；false: 不传递 */
void lv_obj_set_state_trickle(lv_obj_t * obj, bool en);

/** 设置单选按钮行为：同一父对象下只能有一个 CHECKED 子项
 * @param obj  对象指针
 * @param en   true: 启用单选；false: 禁用 */
void lv_obj_set_radio_button(lv_obj_t * obj, bool en);

/** 设置 Flex 布局中强制换行（在新轨道开始，设置 FLEX_IN_NEW_TRACK 标志）
 * @param obj  对象指针
 * @param en   true: 强制换行；false: 不强制 */
void lv_obj_set_flex_in_new_track(lv_obj_t * obj, bool en);

/**
 * 添加一个或多个状态位
 *
 * 添加状态后，关联了这些状态的样式将自动应用。如果样式配置了过渡动画，
 * 会自动从旧状态平滑过渡到新状态。
 *
 * @param obj    对象指针
 * @param state  要添加的状态（可位或组合），如 LV_STATE_PRESSED | LV_STATE_FOCUSED
 */
void lv_obj_add_state(lv_obj_t * obj, lv_state_t state);

/**
 * 移除一个或多个状态位
 *
 * @param obj    对象指针
 * @param state  要移除的状态（可位或组合）
 */
void lv_obj_remove_state(lv_obj_t * obj, lv_state_t state);

/**
 * 设置或清除一个或多个状态位
 *
 * @param obj    对象指针
 * @param state  要操作的状态（可位或组合）
 * @param v      true: 添加状态；false: 移除状态
 */
void lv_obj_set_state(lv_obj_t * obj, lv_state_t state, bool v);

/**
 * 设置对象的 user_data 字段
 *
 * user_data 是一个 void* 指针，可以存储用户自定义数据，
 * 在事件回调中通过 lv_event_get_user_data() 获取。
 *
 * @param obj       对象指针
 * @param user_data 指向用户数据的指针
 */
void lv_obj_set_user_data(lv_obj_t * obj, void * user_data);

/**
 * 设置用户自定义标志位（共 4 个可用位）
 *
 * @param obj  对象指针
 * @param bit  位索引（0~3）
 * @param v    true: 置位；false: 清除
 */
void lv_obj_set_user_flag(lv_obj_t * obj, uint32_t bit, bool v);

/*=======================
 * Getter functions（获取函数）
 *======================*/

/**
 * 检查对象是否设置了指定标志位（已废弃，建议使用专用 getter）
 *
 * @deprecated 请使用专用 getter，例如 lv_obj_is_hidden(obj)。
 * @param obj   对象指针
 * @param f     要检查的标志（可位或组合）
 * @return      true: 所有标志都已设置；false: 并非全部设置
 */
LV_DEPRECATED("Use the dedicated lv_obj_is_<flag>() functions instead, e.g. lv_obj_is_hidden(obj).")
bool lv_obj_has_flag(const lv_obj_t * obj, lv_obj_flag_t f);

/**
 * 检查对象是否设置了指定标志位中的任意一个（已废弃）
 *
 * @deprecated 请使用专用 getter，例如 lv_obj_is_hidden(obj)。
 * @param obj   对象指针
 * @param f     要检查的标志（可位或组合）
 * @return      true: 至少一个标志已设置；false: 全部未设置
 */
LV_DEPRECATED("Use the dedicated lv_obj_is_<flag>() functions instead, e.g. lv_obj_is_hidden(obj).")
bool lv_obj_has_flag_any(const lv_obj_t * obj, lv_obj_flag_t f);

/** 获取对象是否隐藏
 * @param obj  对象指针
 * @return     true: 隐藏；false: 可见 */
bool lv_obj_is_hidden(const lv_obj_t * obj);

/** 获取对象是否可点击
 * @param obj  对象指针
 * @return     true: 可点击；false: 不可点击 */
bool lv_obj_is_clickable(const lv_obj_t * obj);

/** 获取点击时是否获得焦点
 * @param obj  对象指针
 * @return     true: 点击获焦；false: 点击不获焦 */
bool lv_obj_is_click_focusable(const lv_obj_t * obj);

/** 获取是否切换选中状态
 * @param obj  对象指针
 * @return     true: 可切换；false: 不可切换 */
bool lv_obj_is_checkable(const lv_obj_t * obj);

/** 获取对象是否可滚动
 * @param obj  对象指针
 * @return     true: 可滚动；false: 不可滚动 */
bool lv_obj_is_scrollable(const lv_obj_t * obj);

/** 获取是否启用弹性滚动
 * @param obj  对象指针
 * @return     true: 已启用；false: 未启用 */
bool lv_obj_is_scroll_elastic(const lv_obj_t * obj);

/** 获取是否启用惯性滚动
 * @param obj  对象指针
 * @return     true: 已启用；false: 未启用 */
bool lv_obj_is_scroll_momentum(const lv_obj_t * obj);

/** 获取是否只滚动一个吸附子项
 * @param obj  对象指针
 * @return     true: 是；false: 否 */
bool lv_obj_is_scroll_one(const lv_obj_t * obj);

/** 获取水平滚动链是否启用
 * @param obj  对象指针
 * @return     true: 启用；false: 禁用 */
bool lv_obj_is_scroll_chain_hor(const lv_obj_t * obj);

/** 获取垂直滚动链是否启用
 * @param obj  对象指针
 * @return     true: 启用；false: 禁用 */
bool lv_obj_is_scroll_chain_ver(const lv_obj_t * obj);

/** 获取获得焦点时是否自动滚动到可见区域
 * @param obj  对象指针
 * @return     true: 是；false: 否 */
bool lv_obj_is_scroll_on_focus(const lv_obj_t * obj);

/** 获取方向键是否可滚动已聚焦的对象
 * @param obj  对象指针
 * @return     true: 是；false: 否 */
bool lv_obj_is_scroll_with_arrow(const lv_obj_t * obj);

/** 获取对象是否可被滚动吸附
 * @param obj  对象指针
 * @return     true: 可吸附；false: 不可吸附 */
bool lv_obj_is_snappable(const lv_obj_t * obj);

/** 获取按住锁定是否启用
 * @param obj  对象指针
 * @return     true: 启用；false: 禁用 */
bool lv_obj_is_press_lock(const lv_obj_t * obj);

/** 获取事件是否冒泡给父对象
 * @param obj  对象指针
 * @return     true: 冒泡；false: 不冒泡 */
bool lv_obj_is_event_bubble(const lv_obj_t * obj);

/** 获取手势是否冒泡给父对象
 * @param obj  对象指针
 * @return     true: 冒泡；false: 不冒泡 */
bool lv_obj_is_gesture_bubble(const lv_obj_t * obj);

/** 获取高精度命中测试是否启用
 * @param obj  对象指针
 * @return     true: 启用；false: 禁用 */
bool lv_obj_is_adv_hittest(const lv_obj_t * obj);

/** 获取对象是否忽略布局自动定位
 * @param obj  对象指针
 * @return     true: 忽略；false: 不忽略 */
bool lv_obj_is_ignore_layout(const lv_obj_t * obj);

/** 获取对象是否浮动
 * @param obj  对象指针
 * @return     true: 浮动；false: 正常 */
bool lv_obj_is_floating(const lv_obj_t * obj);

/** 获取是否发送绘制任务事件
 * @param obj  对象指针
 * @return     true: 发送；false: 不发送 */
bool lv_obj_is_send_draw_task_events(const lv_obj_t * obj);

/** 获取子对象是否可溢出到父对象之外
 * @param obj  对象指针
 * @return     true: 可溢出；false: 裁剪 */
bool lv_obj_is_overflow_visible(const lv_obj_t * obj);

/** 获取事件是否向下传递给子对象
 * @param obj  对象指针
 * @return     true: 传递；false: 不传递 */
bool lv_obj_is_event_trickle(const lv_obj_t * obj);

/** 获取状态是否向下传递给子对象
 * @param obj  对象指针
 * @return     true: 传递；false: 不传递 */
bool lv_obj_is_state_trickle(const lv_obj_t * obj);

/** 获取是否为单选按钮模式
 * @param obj  对象指针
 * @return     true: 单选模式；false: 普通模式 */
bool lv_obj_is_radio_button(const lv_obj_t * obj);

/**
 * 获取 Flex 布局中是否强制换行
 * @param obj  对象指针
 * @return     true: 强制换行；false: 不强制
 */
bool lv_obj_is_flex_in_new_track(const lv_obj_t * obj);

/**
 * 获取对象当前状态（位或组合）
 *
 * @param obj  对象指针
 * @return     当前状态的位或组合，如 LV_STATE_PRESSED | LV_STATE_FOCUSED
 */
lv_state_t lv_obj_get_state(const lv_obj_t * obj);

/**
 * 检查对象是否处于指定状态
 *
 * @param obj    对象指针
 * @param state  要检查的状态（可位或组合）
 * @return       true: 对象处于该状态；false: 不处于该状态
 */
bool lv_obj_has_state(const lv_obj_t * obj, lv_state_t state);

/**
 * 获取对象所属的输入设备组
 *
 * @param obj  对象指针
 * @return     对象所属的组指针
 */
lv_group_t * lv_obj_get_group(const lv_obj_t * obj);

/**
 * 获取对象的 user_data 字段
 *
 * @param obj  对象指针
 * @return     user_data 指针
 */
void * lv_obj_get_user_data(lv_obj_t * obj);

/**
 * 获取用户自定义标志位的值
 *
 * @param obj  对象指针
 * @param bit  位索引（0~3）
 * @return     true: 置位；false: 清除
 */
bool lv_obj_get_user_flag(lv_obj_t * obj, uint32_t bit);

/*=======================
 * Other functions（其他函数）
 *======================*/

/**
 * 检查对象的类（类型）是否精确匹配
 *
 * @param obj      对象指针
 * @param class_p  要检查的类（如 lv_slider_class）
 * @return         true: 精确匹配；false: 不匹配
 */
bool lv_obj_check_type(const lv_obj_t * obj, const lv_obj_class_t * class_p);

/**
 * 检查对象是否属于指定类（包含祖先类）
 *
 * 与 lv_obj_check_type 不同，此函数会沿着类继承链向上查找。
 * 例如 lv_slider 对象会匹配 lv_slider_class 和 lv_bar_class。
 *
 * @param obj      对象指针
 * @param class_p  要检查的类
 * @return         true: 对象属于该类或其子类；false: 不属于
 */
bool lv_obj_has_class(const lv_obj_t * obj, const lv_obj_class_t * class_p);

/**
 * 获取对象的类（类型）
 *
 * @param obj  对象指针
 * @return     对象的类指针
 */
const lv_obj_class_t * lv_obj_get_class(const lv_obj_t * obj);

/**
 * 检查对象是否有效（仍然存活）
 *
 * @param obj  对象指针
 * @return     true: 有效；false: 已删除或无效
 */
bool lv_obj_is_valid(const lv_obj_t * obj);

/**
 * 注册自动置空回调：当对象被删除时自动将指定指针置为 NULL
 *
 * 指针必须存储在对象生命周期内不会失效的位置（如 static 变量或堆分配）。
 *
 * @param obj_ptr  指向对象指针的指针（即 lv_obj_t **）
 */
void lv_obj_null_on_delete(lv_obj_t ** obj_ptr);

/**
 * 注册删除回调
 *
 * 当对象被删除时，自动调用指定回调函数并传入 user_data。
 * 这是 LV_EVENT_DELETE 事件的便捷封装。
 *
 * @param obj        对象指针
 * @param cb         删除回调函数
 * @param user_data  回调函数的用户数据参数
 * @return           删除描述符指针，失败返回 NULL
 */
lv_delete_dsc_t * lv_obj_add_delete_cb(lv_obj_t * obj, lv_delete_cb_t cb, void * user_data);

/**
 * 注销删除回调
 *
 * @param dsc  删除描述符指针（由 lv_obj_add_delete_cb 返回）。传入 NULL 则无操作。
 */
void lv_obj_remove_delete_cb(lv_delete_dsc_t * dsc);

/**
 * 注册屏幕加载事件：触发指定事件时切换到指定屏幕
 *
 * @param obj         绑定事件的对象
 * @param trigger     触发事件（如 LV_EVENT_CLICKED）
 * @param screen      要加载的屏幕对象
 * @param anim_type   加载动画类型（lv_screen_load_anim_t 枚举）
 * @param duration    动画时长（毫秒）
 * @param delay       延迟时间（毫秒）
 */
void lv_obj_add_screen_load_event(lv_obj_t * obj, lv_event_code_t trigger, lv_obj_t * screen,
                                  lv_screen_load_anim_t anim_type, uint32_t duration, uint32_t delay);

/**
 * 注册屏幕创建事件：触发指定事件时创建并切换到新屏幕
 *
 * 创建的屏幕在被卸载时自动删除。
 *
 * @param obj               绑定事件的对象
 * @param trigger           触发事件（如 LV_EVENT_CLICKED）
 * @param screen_create_cb  屏幕创建回调函数（返回 lv_obj_t *）
 * @param anim_type         加载动画类型
 * @param duration          动画时长（毫秒）
 * @param delay             延迟时间（毫秒）
 */
void lv_obj_add_screen_create_event(lv_obj_t * obj, lv_event_code_t trigger, lv_screen_create_cb_t screen_create_cb,
                                    lv_screen_load_anim_t anim_type, uint32_t duration, uint32_t delay);


/**
 * 注册时间线动画播放事件：触发指定事件时播放时间线动画
 *
 * @param obj      绑定事件的对象
 * @param trigger  触发事件（如 LV_EVENT_CLICKED）
 * @param at       动画时间线指针
 * @param delay    延迟时间（毫秒）
 * @param reverse  true: 反向播放；false: 正向播放
 */
void lv_obj_add_play_timeline_event(lv_obj_t * obj, lv_event_code_t trigger, lv_anim_timeline_t * at, uint32_t delay,
                                    bool reverse);

#if LV_USE_OBJ_ID
/**
 * 设置对象的 ID
 *
 * @param obj  对象指针
 * @param id   ID 指针
 */
void lv_obj_set_id(lv_obj_t * obj, void * id);

/**
 * 获取对象的 ID
 *
 * @param obj  对象指针
 * @return     ID 指针
 */
void * lv_obj_get_id(const lv_obj_t * obj);

/**
 * 根据 ID 查找子对象（递归搜索，已废弃）
 *
 * @deprecated ID 仅用于打印控件树。查找控件请使用 lv_obj_find_by_name。
 * @param obj  父对象指针
 * @param id   要查找的子对象 ID
 * @return     找到的子对象指针，未找到返回 NULL
 */
LV_DEPRECATED("IDs are used only to print the widget trees. To find a widget use lv_obj_find_by_name")
lv_obj_t * lv_obj_find_by_id(const lv_obj_t * obj, const void * id);

/**
 * 自动分配对象 ID（LV_OBJ_ID_AUTO_ASSIGN 启用时自动调用）
 *
 * 设置 LV_USE_OBJ_ID_BUILTIN 使用内置 ID 生成器，
 * 否则需外部实现 lv_obj_[set|assign|free|stringify]_id 和 lv_obj_id_compare。
 *
 * @param class_p  对象所属的类
 * @param obj      对象指针
 */
void lv_obj_assign_id(const lv_obj_class_t * class_p, lv_obj_t * obj);

/**
 * 释放对象 ID 占用的资源（对象删除时自动调用）
 *
 * @param obj  对象指针
 */
void lv_obj_free_id(lv_obj_t * obj);

/**
 * 比较两个 ID 是否相同
 *
 * 设置 LV_USE_OBJ_ID_BUILTIN 使用内置比较器，否则需外部实现。
 *
 * @param id1  第一个 ID
 * @param id2  第二个 ID
 * @return     0: 相同；非零: 不同
 */
int lv_obj_id_compare(const void * id1, const void * id2);

/**
 * 将对象 ID 格式化为字符串
 *
 * @param obj  对象指针
 * @param buf  输出缓冲区
 * @param len  缓冲区长度
 * @return     格式化后的字符串指针
 */
const char * lv_obj_stringify_id(lv_obj_t * obj, char * buf, uint32_t len);

#if LV_USE_OBJ_ID_BUILTIN
/**
 * 释放内置 ID 生成器占用的资源
 */
void lv_objid_builtin_destroy(void);
#endif

#endif /*LV_USE_OBJ_ID*/

/**********************
 *      MACROS
 **********************/

#if LV_USE_ASSERT_OBJ
/**
 * @deprecated 请使用 LV_CHECK_OBJ(obj, cls, return) 替代。
 *             LV_ASSERT_OBJ 失败时中止程序；LV_CHECK_OBJ 仅记录警告并执行指定操作，更适合生产环境。
 */
#define LV_ASSERT_OBJ(obj_p, obj_class)                                                             \
    do {                                                                                              \
        LV_DEPRECATED_MACRO_WARN("LV_ASSERT_OBJ is deprecated. Use LV_CHECK_OBJ instead.");             \
        LV_ASSERT_MSG(obj_p != NULL, "The object is NULL");                                             \
        LV_ASSERT_MSG(lv_obj_has_class(obj_p, obj_class) == true, "Incompatible object type.");         \
        LV_ASSERT_MSG(lv_obj_is_valid(obj_p)  == true, "The object is invalid, deleted or corrupted?"); \
    } while(0)
# else
/**
 * @deprecated 请使用 LV_CHECK_OBJ(obj, cls, return) 替代。
 */
#define LV_ASSERT_OBJ(obj_p, obj_class) \
    do { \
        LV_DEPRECATED_MACRO_WARN("LV_ASSERT_OBJ is deprecated. Use LV_CHECK_OBJ instead."); \
        LV_ASSERT_NULL(obj_p); \
    } while(0)
#endif

#if LV_USE_LOG && LV_LOG_TRACE_OBJ_CREATE
#  define LV_TRACE_OBJ_CREATE(...) LV_LOG_TRACE(__VA_ARGS__)
#else
#  define LV_TRACE_OBJ_CREATE(...)
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_H*/