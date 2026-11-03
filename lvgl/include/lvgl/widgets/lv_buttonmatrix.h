/**
 * @file lv_buttonmatrix.h
 * 按钮矩阵控件:由字符串数组映射生成的多按钮网格
 *
 */

#ifndef LV_BUTTONMATRIX_H
#define LV_BUTTONMATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_BUTTONMATRIX != 0

#include "../core/lv_obj.h"
#include "../core/lv_obj_property.h"

/*********************
 *      DEFINES
 * 宏定义
 *********************/
/** 无有效按钮的 ID */
#define LV_BUTTONMATRIX_BUTTON_NONE 0xFFFF
LV_EXPORT_CONST_INT(LV_BUTTONMATRIX_BUTTON_NONE);

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

/** Type to store button control flags (disabled, hidden etc.)
 *  The least-significant 4 bits are used to store button-width proportions in range [1..15].
 *  用于存储按钮控制标志的类型(禁用、隐藏等)。
 *  最低 4 位用于存储按钮宽度比例,取值范围 [1..15]。 */
typedef enum {
    LV_BUTTONMATRIX_CTRL_NONE         = 0x0000, /**< No extra control, use the default settings*/
    /**< 无额外控制,使用默认设置 */
    LV_BUTTONMATRIX_CTRL_WIDTH_1      = 0x0001, /**< Set the width to 1 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 1 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_2      = 0x0002, /**< Set the width to 2 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 2 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_3      = 0x0003, /**< Set the width to 3 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 3 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_4      = 0x0004, /**< Set the width to 4 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 4 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_5      = 0x0005, /**< Set the width to 5 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 5 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_6      = 0x0006, /**< Set the width to 6 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 6 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_7      = 0x0007, /**< Set the width to 7 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 7 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_8      = 0x0008, /**< Set the width to 8 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 8 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_9      = 0x0009, /**< Set the width to 9 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 9 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_10     = 0x000A, /**< Set the width to 10 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 10 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_11     = 0x000B, /**< Set the width to 11 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 11 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_12     = 0x000C, /**< Set the width to 12 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 12 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_13     = 0x000D, /**< Set the width to 13 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 13 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_14     = 0x000E, /**< Set the width to 14 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 14 倍 */
    LV_BUTTONMATRIX_CTRL_WIDTH_15     = 0x000F, /**< Set the width to 15 relative to the other buttons in the same row */
    /**< 将宽度设置为同排其他按钮的 15 倍 */
    LV_BUTTONMATRIX_CTRL_HIDDEN       = 0x0010, /**< Hides button; it continues to hold its space in layout. */
    /**< 隐藏按钮;按钮仍占据其布局空间。 */
    LV_BUTTONMATRIX_CTRL_NO_REPEAT    = 0x0020, /**< Do not emit LV_EVENT_LONG_PRESSED_REPEAT events while button is long-pressed. */
    /**< 长按时不发送 LV_EVENT_LONG_PRESSED_REPEAT 事件。 */
    LV_BUTTONMATRIX_CTRL_DISABLED     = 0x0040, /**< Disables button like LV_STATE_DISABLED on normal Widgets. */
    /**< 禁用按钮,类似普通控件上的 LV_STATE_DISABLED。 */
    LV_BUTTONMATRIX_CTRL_CHECKABLE    = 0x0080, /**< Enable toggling of LV_STATE_CHECKED when clicked. */
    /**< 点击时启用 LV_STATE_CHECKED 状态的切换。 */
    LV_BUTTONMATRIX_CTRL_CHECKED      = 0x0100, /**< Make the button checked. It will use the :cpp:enumerator:`LV_STATE_CHECKED` styles. */
    /**< 使按钮处于选中状态,将使用 :cpp:enumerator:`LV_STATE_CHECKED` 样式。 */
    LV_BUTTONMATRIX_CTRL_CLICK_TRIG   = 0x0200, /**< 1: Enables sending LV_EVENT_VALUE_CHANGE on CLICK, 0: sends LV_EVENT_VALUE_CHANGE on PRESS. */
    /**< 1:在点击(CLICK)时发送 LV_EVENT_VALUE_CHANGE;0:在按下(PRESS)时发送。 */
    LV_BUTTONMATRIX_CTRL_POPOVER      = 0x0400, /**< Show button text in a pop-over while being pressed. */
    /**< 按下时在弹出层中显示按钮文本。 */
    LV_BUTTONMATRIX_CTRL_RECOLOR      = 0x0800, /**< Enable text recoloring with `#color` */
    /**< 启用使用 `#color` 的文本重新着色。 */
    LV_BUTTONMATRIX_CTRL_RESERVED_1   = 0x1000, /**< Reserved for later use */
    /**< 保留供以后使用 */
    LV_BUTTONMATRIX_CTRL_RESERVED_2   = 0x2000, /**< Reserved for later use */
    /**< 保留供以后使用 */
    LV_BUTTONMATRIX_CTRL_CUSTOM_1     = 0x4000, /**< Custom free-to-use flag */
    /**< 自定义的自由使用标志 */
    LV_BUTTONMATRIX_CTRL_CUSTOM_2     = 0x8000, /**< Custom free-to-use flag */
    /**< 自定义的自由使用标志 */
} lv_buttonmatrix_ctrl_t;

/** 按钮矩阵按钮的自定义绘制回调函数类型 */
typedef bool (*lv_buttonmatrix_button_draw_cb_t)(lv_obj_t * btnm, uint32_t btn_id, const lv_area_t * draw_area,
                                                 const lv_area_t * clip_area);

/** 按钮矩阵对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_buttonmatrix_class;

#if LV_USE_OBJ_PROPERTY
/** 属性系统:按钮矩阵对象的属性 ID 枚举 */
enum _lv_property_buttonmatrix_id_t {
    LV_PROPERTY_ID(BUTTONMATRIX, SELECTED_BUTTON, LV_PROPERTY_TYPE_INT, 0),
    LV_PROPERTY_ID(BUTTONMATRIX, ONE_CHECKED,     LV_PROPERTY_TYPE_BOOL, 1),
    LV_PROPERTY_BUTTONMATRIX_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a button matrix object
 * 创建按钮矩阵对象
 * @param parent    pointer to an object, it will be the parent of the new button matrix
 * @param parent    指向一个对象的指针,该对象将成为新按钮矩阵的父对象
 * @return          pointer to the created button matrix
 * @return          指向所创建的按钮矩阵对象的指针
 */
lv_obj_t * lv_buttonmatrix_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set a new map. Buttons will be created/deleted according to the map. The
 * button matrix keeps a reference to the map and so the string array must not
 * be deallocated during the life of the matrix.
 * 设置新的映射。将根据映射创建/删除按钮。按钮矩阵会保存映射的引用,
 * 因此在矩阵的整个生命周期内,该字符串数组不得被释放。
 * @param obj       pointer to a button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param map       pointer a string array. The last string has to be: "". Use "\n" to make a line break.
 * @param map       指向字符串数组的指针。最后一个字符串必须是:""。使用 "\n" 进行换行。
 */
void lv_buttonmatrix_set_map(lv_obj_t * obj, const char * const map[]);

/**
 * Set the button control map (hidden, disabled etc.) for a button matrix.
 * 设置按钮矩阵的按钮控制映射(隐藏、禁用等)。
 * The control map array will be copied and so may be deallocated after this
 * function returns.
 * 控制映射数组会被复制,因此本函数返回后可以将其释放。
 * @param obj       pointer to a button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param ctrl_map  pointer to an array of `lv_button_ctrl_t` control bytes. The
 *                  length of the array and position of the elements must match
 *                  the number and order of the individual buttons (i.e. excludes
 *                  newline entries).
 *                  An element of the map should look like e.g.:
 *                 `ctrl_map[0] = width | LV_BUTTONMATRIX_CTRL_NO_REPEAT |  LV_BUTTONMATRIX_CTRL_TGL_ENABLE`
 *                  ctrl_map:指向 `lv_button_ctrl_t` 控制字节数组的指针。数组的长度和
 *                  元素的位置必须与各个按钮的数量和顺序一致(即不包括换行条目)。
 *                  映射元素的形式例如:
 *                  `ctrl_map[0] = width | LV_BUTTONMATRIX_CTRL_NO_REPEAT |  LV_BUTTONMATRIX_CTRL_TGL_ENABLE`
 */
void lv_buttonmatrix_set_ctrl_map(lv_obj_t * obj, const lv_buttonmatrix_ctrl_t ctrl_map[]);

/**
 * Set the selected buttons
 * 设置选中的按钮
 * @param obj        pointer to button matrix object
 * @param obj        指向按钮矩阵对象的指针
 * @param btn_id     0 based index of the button to modify. (Not counting new lines)
 * @param btn_id     要修改的按钮的从 0 开始的索引(不计换行)
 */
void lv_buttonmatrix_set_selected_button(lv_obj_t * obj, uint32_t btn_id);

/**
 * Set the attributes of a button of the button matrix
 * 设置按钮矩阵中某个按钮的属性
 * @param obj       pointer to button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param btn_id    0 based index of the button to modify. (Not counting new lines)
 * @param btn_id    要修改的按钮的从 0 开始的索引(不计换行)
 * @param ctrl      OR-ed attributes. E.g. `LV_BUTTONMATRIX_CTRL_NO_REPEAT | LV_BUTTONMATRIX_CTRL_CHECKABLE`
 * @param ctrl      按位或组合的属性。例如 `LV_BUTTONMATRIX_CTRL_NO_REPEAT | LV_BUTTONMATRIX_CTRL_CHECKABLE`
 */
void lv_buttonmatrix_set_button_ctrl(lv_obj_t * obj, uint32_t btn_id, lv_buttonmatrix_ctrl_t ctrl);

/**
 * Clear the attributes of a button of the button matrix
 * 清除按钮矩阵中某个按钮的属性
 * @param obj       pointer to button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param btn_id    0 based index of the button to modify. (Not counting new lines)
 * @param btn_id    要修改的按钮的从 0 开始的索引(不计换行)
 * @param ctrl      OR-ed attributes. E.g. `LV_BUTTONMATRIX_CTRL_NO_REPEAT | LV_BUTTONMATRIX_CTRL_CHECKABLE`
 * @param ctrl      按位或组合的属性。例如 `LV_BUTTONMATRIX_CTRL_NO_REPEAT | LV_BUTTONMATRIX_CTRL_CHECKABLE`
 */
void lv_buttonmatrix_clear_button_ctrl(lv_obj_t * obj, uint32_t btn_id, lv_buttonmatrix_ctrl_t ctrl);

/**
 * Set attributes of all buttons of a button matrix
 * 设置按钮矩阵中所有按钮的属性
 * @param obj       pointer to a button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param ctrl      attribute(s) to set from `lv_buttonmatrix_ctrl_t`. Values can be ORed.
 * @param ctrl      要设置的属性,取自 `lv_buttonmatrix_ctrl_t`。多个值可以按位或组合。
 */
void lv_buttonmatrix_set_button_ctrl_all(lv_obj_t * obj, lv_buttonmatrix_ctrl_t ctrl);

/**
 * Clear the attributes of all buttons of a button matrix
 * 清除按钮矩阵中所有按钮的属性
 * @param obj       pointer to a button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param ctrl      attribute(s) to set from `lv_buttonmatrix_ctrl_t`. Values can be ORed.
 * @param ctrl      要清除的属性,取自 `lv_buttonmatrix_ctrl_t`。多个值可以按位或组合。
 */
void lv_buttonmatrix_clear_button_ctrl_all(lv_obj_t * obj, lv_buttonmatrix_ctrl_t ctrl);

/**
 * Set a single button's relative width.
 * 设置单个按钮的相对宽度。
 * This method will cause the matrix be regenerated and is a relatively
 * expensive operation. It is recommended that initial width be specified using
 * `lv_buttonmatrix_set_ctrl_map` and this method only be used for dynamic changes.
 * 该方法会导致矩阵重新生成,属于相对开销较大的操作。
 * 建议使用 `lv_buttonmatrix_set_ctrl_map` 指定初始宽度,仅在本方法中做动态修改。
 * @param obj       pointer to button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param btn_id    0 based index of the button to modify.
 * @param btn_id    要修改的按钮的从 0 开始的索引
 * @param width     relative width compared to the buttons in the same row. [1..15]
 * @param width     与同行其他按钮相比的相对宽度 [1..15]
 */
void lv_buttonmatrix_set_button_width(lv_obj_t * obj, uint32_t btn_id, uint32_t width);

/**
 * Make the button matrix like a selector widget (only one button may be checked at a time).
 * 使按钮矩阵像选择器控件一样工作(同一时间只允许一个按钮被选中)。
 * `LV_BUTTONMATRIX_CTRL_CHECKABLE` must be enabled on the buttons to be selected using
 * `lv_buttonmatrix_set_ctrl()` or `lv_buttonmatrix_set_button_ctrl_all()`.
 * 需要使用 `lv_buttonmatrix_set_ctrl()` 或 `lv_buttonmatrix_set_button_ctrl_all()`
 * 在待选按钮上启用 `LV_BUTTONMATRIX_CTRL_CHECKABLE`。
 * @param obj       pointer to a button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param en        whether "one check" mode is enabled
 * @param en        是否启用"单选"模式
 */
void lv_buttonmatrix_set_one_checked(lv_obj_t * obj, bool en);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the current map of a button matrix
 * 获取按钮矩阵当前的映射
 * @param obj       pointer to a button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @return          the current map
 * @return          当前的映射
 */
const char * const * lv_buttonmatrix_get_map(const lv_obj_t * obj);

/**
 * Get the index of the lastly "activated" button by the user (pressed, released, focused etc)
 * 获取用户最后"激活"的按钮索引(按下、释放、聚焦等)
 * Useful in the `event_cb` to get the text of the button, check if hidden etc.
 * 在 `event_cb` 中可用于获取按钮的文本、检查是否隐藏等。
 * @param obj       pointer to button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @return          index of the last released button (LV_BUTTONMATRIX_BUTTON_NONE: if unset)
 * @return          最后释放的按钮索引(未设置时为 LV_BUTTONMATRIX_BUTTON_NONE)
 */
uint32_t lv_buttonmatrix_get_selected_button(const lv_obj_t * obj);

/**
 * Get the button's text
 * 获取按钮的文本
 * @param obj       pointer to button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param btn_id    the index a button not counting new line characters.
 * @param btn_id    按钮的索引,不计换行字符
 * @return          text of btn_index` button
 * @return          btn_index 按钮的文本
 */
const char * lv_buttonmatrix_get_button_text(const lv_obj_t * obj, uint32_t btn_id);

/**
 * Get the whether a control value is enabled or disabled for button of a button matrix
 * 获取按钮矩阵中某个按钮的控制值是否启用
 * @param obj       pointer to a button matrix object
 * @param obj       指向按钮矩阵对象的指针
 * @param btn_id    the index of a button not counting new line characters.
 * @param btn_id    按钮的索引,不计换行字符
 * @param ctrl      control values to check (ORed value can be used)
 * @param ctrl      要检查的控制值(可以使用按位或组合的值)
 * @return          true: the control attribute is enabled false: disabled
 * @return          true:控制属性已启用 false:已禁用
 */
bool lv_buttonmatrix_has_button_ctrl(lv_obj_t * obj, uint32_t btn_id, lv_buttonmatrix_ctrl_t ctrl);

/**
 * Tell whether "one check" mode is enabled or not.
 * 判断"单选"模式是否启用。
 * @param obj       Button matrix object
 * @param obj       按钮矩阵对象
 * @return          true: "one check" mode is enabled; false: disabled
 * @return          true:"单选"模式已启用;false:已禁用
 */
bool lv_buttonmatrix_get_one_checked(const lv_obj_t * obj);

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_BUTTONMATRIX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_BUTTONMATRIX_H*/
