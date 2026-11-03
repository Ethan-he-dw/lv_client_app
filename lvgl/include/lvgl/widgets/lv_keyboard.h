/**
 * @file lv_keyboard.h
 * 键盘(Keyboard)控件：基于按钮矩阵(Button Matrix)实现，支持多套按键布局和模式
 *
 */

#ifndef LV_KEYBOARD_H
#define LV_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "lv_buttonmatrix.h"

#if LV_USE_KEYBOARD

/*********************
 *      DEFINES
 *      宏定义
 *********************/
/*键盘控制按钮的标志位组合（不重复触发、点击触发、可选中）*/
#define LV_KEYBOARD_CTRL_BUTTON_FLAGS (LV_BUTTONMATRIX_CTRL_NO_REPEAT | LV_BUTTONMATRIX_CTRL_CLICK_TRIG | LV_BUTTONMATRIX_CTRL_CHECKED)

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/** Current keyboard mode.*/
/** 当前的键盘模式。*/
typedef enum {
    /*小写字母文本模式*/
    LV_KEYBOARD_MODE_TEXT_LOWER,
    /*大写字母文本模式*/
    LV_KEYBOARD_MODE_TEXT_UPPER,
    /*特殊字符模式*/
    LV_KEYBOARD_MODE_SPECIAL,
    /*数字模式*/
    LV_KEYBOARD_MODE_NUMBER,
    /*用户自定义模式1*/
    LV_KEYBOARD_MODE_USER_1,
    /*用户自定义模式2*/
    LV_KEYBOARD_MODE_USER_2,
    /*用户自定义模式3*/
    LV_KEYBOARD_MODE_USER_3,
    /*用户自定义模式4*/
    LV_KEYBOARD_MODE_USER_4,
#if LV_USE_ARABIC_PERSIAN_CHARS == 1
    /*阿拉伯文文本模式*/
    LV_KEYBOARD_MODE_TEXT_ARABIC
#endif
} lv_keyboard_mode_t;

#if LV_USE_OBJ_PROPERTY
/*键盘对象属性ID枚举*/
enum _lv_property_keyboard_id_t {
    /*关联的文本框对象*/
    LV_PROPERTY_ID(KEYBOARD, TEXTAREA,            LV_PROPERTY_TYPE_OBJ,   0),
    /*键盘模式*/
    LV_PROPERTY_ID(KEYBOARD, MODE,                LV_PROPERTY_TYPE_INT,   1),
    /*是否启用按键弹出提示*/
    LV_PROPERTY_ID(KEYBOARD, POPOVERS,            LV_PROPERTY_TYPE_INT,   2),
    /*当前选中的按钮索引*/
    LV_PROPERTY_ID(KEYBOARD, SELECTED_BUTTON,     LV_PROPERTY_TYPE_INT,   3),
    LV_PROPERTY_KEYBOARD_END,
};
#endif

/*键盘控件的类对象*/
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_keyboard_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a Keyboard object
 * 创建键盘对象
 * @param parent    pointer to an object, it will be the parent of the new keyboard
 * @param parent    指向父对象的指针，它将成为新键盘的父对象
 * @return          pointer to the created keyboard object
 * @return          指向已创建键盘对象的指针
 */
lv_obj_t * lv_keyboard_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Assign a text area to the keyboard. Pressed characters will be inserted there.
 * 为键盘指定一个文本框，按下的字符将插入到该文本框中
 * @param kb        pointer to a keyboard object
 * @param kb        指向键盘对象的指针
 * @param ta        pointer to a text area object to write into
 * @param ta        指向要写入的文本框对象的指针
 */
void lv_keyboard_set_textarea(lv_obj_t * kb, lv_obj_t * ta);

/**
 * Set a new mode (e.g., text, number, special characters).
 * 设置新的键盘模式（例如：文本、数字、特殊字符）
 * @param kb        pointer to a keyboard object
 * @param kb        指向键盘对象的指针
 * @param mode      the desired mode (see 'lv_keyboard_mode_t')
 * @param mode      期望设置的模式（参见 'lv_keyboard_mode_t'）
 */
void lv_keyboard_set_mode(lv_obj_t * kb, lv_keyboard_mode_t mode);

/**
 * Enable or disable popovers showing button titles on press.
 * 启用或禁用按键弹出提示（按下时显示按钮标题）
 * @param kb        pointer to a keyboard object
 * @param kb        指向键盘对象的指针
 * @param en        true to enable popovers; false to disable
 * @param en        true：启用弹出提示；false：禁用
 */
void lv_keyboard_set_popovers(lv_obj_t * kb, bool en);

/**
 * Set a custom button map for the keyboard.
 * 为键盘设置自定义的按键映射表
 * @param kb        pointer to a keyboard object
 * @param kb        指向键盘对象的指针
 * @param mode      the mode to assign the new map to (see 'lv_keyboard_mode_t')
 * @param mode      要应用新映射表的模式（参见 'lv_keyboard_mode_t'）
 * @param map       pointer to a string array describing the button map
 * @param map       指向描述按键映射表的字符串数组的指针
 *                  see 'lv_buttonmatrix_set_map()' for more details
 *                  更多细节参见 'lv_buttonmatrix_set_map()'
 * @param ctrl_map  pointer to the control map. See 'lv_buttonmatrix_set_ctrl_map()'
 * @param ctrl_map  指向控制映射表的指针。参见 'lv_buttonmatrix_set_ctrl_map()'

 */
void lv_keyboard_set_map(lv_obj_t * kb, lv_keyboard_mode_t mode, const char * const map[],
                         const lv_buttonmatrix_ctrl_t ctrl_map[]);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the text area currently assigned to the keyboard.
 * 获取当前分配给键盘的文本框
 * @param kb        pointer to a keyboard object
 * @param kb        指向键盘对象的指针
 * @return          pointer to the assigned text area object
 * @return          指向已分配文本框对象的指针
 */
lv_obj_t * lv_keyboard_get_textarea(const lv_obj_t * kb);

/**
 * Get the current mode of the keyboard.
 * 获取键盘当前的模式
 * @param kb        pointer to a keyboard object
 * @param kb        指向键盘对象的指针
 * @return          the current mode (see 'lv_keyboard_mode_t')
 * @return          当前的模式（参见 'lv_keyboard_mode_t'）
 */
lv_keyboard_mode_t lv_keyboard_get_mode(const lv_obj_t * kb);

/**
 * Check whether popovers are enabled on the keyboard.
 * 检查键盘是否启用了按键弹出提示
 * @param obj       pointer to a keyboard object
 * @param obj       指向键盘对象的指针
 * @return          true if popovers are enabled; false otherwise
 * @return          true：启用了；false：未启用
 */
bool lv_keyboard_get_popovers(const lv_obj_t * obj);

/**
 * Get the current button map of the keyboard.
 * 获取键盘当前的按键映射表
 * @param kb        pointer to a keyboard object
 * @param kb        指向键盘对象的指针
 * @return          pointer to the map array
 * @return          指向映射表数组的指针
 */
const char * const * lv_keyboard_get_map_array(const lv_obj_t * kb);

/**
 * Get the index of the last selected button (pressed, released, focused, etc.).
 * 获取最近一次被选中按钮的索引（按下、释放、聚焦等）
 * Useful in the `event_cb` to retrieve button text or properties.
 * 在 `event_cb` 中可用于获取按钮的文本或属性
 * @param obj       pointer to a keyboard object
 * @param obj       指向键盘对象的指针
 * @return          index of the last interacted button
 * @return          最近一次交互按钮的索引
 *                  returns LV_BUTTONMATRIX_BUTTON_NONE if not set
 *                  未设置时返回 LV_BUTTONMATRIX_BUTTON_NONE
 */
uint32_t lv_keyboard_get_selected_button(const lv_obj_t * obj);

/**
 * Get the text of a button by index.
 * 根据索引获取按钮的文本
 * @param obj       pointer to a keyboard object
 * @param obj       指向键盘对象的指针
 * @param btn_id    index of the button (excluding newline characters)
 * @param btn_id    按钮的索引（不含换行字符）
 * @return          pointer to the text of the button
 * @return          指向按钮文本的指针
 */
const char * lv_keyboard_get_button_text(const lv_obj_t * obj, uint32_t btn_id);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**
 * Default keyboard event callback to handle button presses.
 * 默认的键盘事件回调函数，用于处理按键按下事件
 * Adds characters to the text area and switches map if needed.
 * 将字符添加到文本框，并在需要时切换按键映射表
 * If a custom `event_cb` is used, this function can be called within it.
 * 如果使用了自定义的 `event_cb`，可以在其中调用此函数
 * @param e         the triggering event
 * @param e         触发的事件
 */
void lv_keyboard_def_event_cb(lv_event_t * e);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif  /*LV_USE_KEYBOARD*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_KEYBOARD_H*/
