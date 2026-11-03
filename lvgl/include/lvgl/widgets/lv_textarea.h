/**
 * @file lv_textarea.h
 * @brief 文本框/文本区域(textarea)控件
 */

#ifndef LV_TEXTAREA_H
#define LV_TEXTAREA_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "lv_label.h"

#if LV_USE_TEXTAREA != 0

/*Testing of dependencies*/
/*检查依赖项*/
#if LV_USE_LABEL == 0
#error "lv_textarea: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1)"
#endif

/*********************
 *      DEFINES
 * 宏定义
 *********************/
#define LV_TEXTAREA_CURSOR_LAST (0x7FFF) /*Put the cursor after the last character*/
/*将光标放在最后一个字符之后*/
LV_EXPORT_CONST_INT(LV_TEXTAREA_CURSOR_LAST);

/*占位符(placeholder)部件(part)*/
#define LV_PART_TEXTAREA_PLACEHOLDER LV_PART_CUSTOM_FIRST
LV_EXPORT_CONST_INT(LV_PART_TEXTAREA_PLACEHOLDER);

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

#if LV_USE_OBJ_PROPERTY
/*文本框对象属性ID枚举*/
enum _lv_property_textarea_id_t {
    LV_PROPERTY_ID(TEXTAREA, TEXT,              LV_PROPERTY_TYPE_TEXT,  0),   /*文本内容*/
    LV_PROPERTY_ID(TEXTAREA, PLACEHOLDER_TEXT,  LV_PROPERTY_TYPE_TEXT,  1),   /*占位符文本*/
    LV_PROPERTY_ID(TEXTAREA, CURSOR_POS,        LV_PROPERTY_TYPE_INT,   2),   /*光标位置*/
    LV_PROPERTY_ID(TEXTAREA, CURSOR_CLICK_POS,  LV_PROPERTY_TYPE_INT,   3),   /*点击定位光标*/
    LV_PROPERTY_ID(TEXTAREA, PASSWORD_MODE,     LV_PROPERTY_TYPE_INT,   4),   /*密码模式*/
    LV_PROPERTY_ID(TEXTAREA, PASSWORD_BULLET,   LV_PROPERTY_TYPE_TEXT,  5),   /*密码显示字符*/
    LV_PROPERTY_ID(TEXTAREA, ONE_LINE,          LV_PROPERTY_TYPE_BOOL,  6),   /*单行模式*/
    LV_PROPERTY_ID(TEXTAREA, ACCEPTED_CHARS,    LV_PROPERTY_TYPE_TEXT,  7),   /*可接受的字符集*/
    LV_PROPERTY_ID(TEXTAREA, MAX_LENGTH,        LV_PROPERTY_TYPE_INT,   8),   /*最大长度*/
    LV_PROPERTY_ID(TEXTAREA, TEXT_SELECTION,    LV_PROPERTY_TYPE_BOOL,  9),   /*文本选择模式*/
    LV_PROPERTY_ID(TEXTAREA, PASSWORD_SHOW_TIME, LV_PROPERTY_TYPE_INT,  10),  /*密码显示时长*/
    LV_PROPERTY_ID(TEXTAREA, LABEL,             LV_PROPERTY_TYPE_OBJ,   11),  /*内部标签(label)对象*/
    LV_PROPERTY_ID(TEXTAREA, TEXT_IS_SELECTED,  LV_PROPERTY_TYPE_INT,   12),  /*文本是否被选中*/
    LV_PROPERTY_ID(TEXTAREA, CURRENT_CHAR,      LV_PROPERTY_TYPE_INT,   13),  /*当前字符*/
    LV_PROPERTY_TEXTAREA_END,
};
#endif

/*文本框控件类(lv_obj_class_t)*/
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_textarea_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a text area object
 * 创建一个文本框对象
 * @param parent    pointer to an object, it will be the parent of the new text area
 *                 指向父对象，新文本框的父对象
 * @return          pointer to the created text area
 *                 指向创建出的文本框对象的指针
 */
lv_obj_t * lv_textarea_create(lv_obj_t * parent);

/*======================
 * Add/remove functions
 * 添加/删除函数
 *=====================*/

/**
 * Insert a character to the current cursor position.
 * 在当前光标位置插入一个字符。
 * To add a wide char, e.g. 'Á' use `lv_text_encoded_conv_wc('Á')`
 * 若要添加宽字符，例如 'Á'，请使用 `lv_text_encoded_conv_wc('Á')`
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param c         a character (e.g. 'a')
 *                 一个字符 (例如 'a')
 */
void lv_textarea_add_char(lv_obj_t * obj, uint32_t c);

/**
 * Insert a text to the current cursor position
 * 在当前光标位置插入一段文本
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param txt       a '\0' terminated string to insert
 *                 要插入的以 '\0' 结尾的字符串
 */
void lv_textarea_add_text(lv_obj_t * obj, const char * txt);

/**
 * Delete a the left character from the current cursor position
 * 从当前光标位置删除左侧的一个字符
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 */
void lv_textarea_delete_char(lv_obj_t * obj);

/**
 * Delete the right character from the current cursor position
 * 从当前光标位置删除右侧的一个字符
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 */
void lv_textarea_delete_char_forward(lv_obj_t * obj);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the text of a text area
 * 设置文本框的文本
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param txt       pointer to the text
 *                 指向文本的指针
 */
void lv_textarea_set_text(lv_obj_t * obj, const char * txt);

/**
 * Set the placeholder text of a text area
 * 设置文本框的占位符文本
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param txt       pointer to the text
 *                 指向文本的指针
 */
void lv_textarea_set_placeholder_text(lv_obj_t * obj, const char * txt);

/**
 * Set the cursor position
 * 设置光标位置
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param pos       the new cursor position in character index
 *                 新的光标位置(字符索引)
 *                  < 0 : index from the end of the text
 *                  < 0 : 从文本末尾开始计数的索引
 *                  LV_TEXTAREA_CURSOR_LAST: go after the last character
 *                  LV_TEXTAREA_CURSOR_LAST: 将光标移到最后一个字符之后
 */
void lv_textarea_set_cursor_pos(lv_obj_t * obj, int32_t pos);

/**
 * Enable/Disable the positioning of the cursor by clicking the text on the text area.
 * 启用/禁用通过点击文本框中的文本定位光标。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param en        true: enable click positions; false: disable
 *                 true: 启用点击定位；false: 禁用
 */
void lv_textarea_set_cursor_click_pos(lv_obj_t * obj, bool en);

/**
 * Enable/Disable password mode
 * 启用/禁用密码模式
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param en        true: enable, false: disable
 *                 true: 启用，false: 禁用
 */
void lv_textarea_set_password_mode(lv_obj_t * obj, bool en);

/**
 * Set the replacement characters to show in password mode
 * 设置密码模式下显示的替代字符
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param bullet    pointer to the replacement text
 *                 指向替代文本的指针
 */
void lv_textarea_set_password_bullet(lv_obj_t * obj, const char * bullet);

/**
 * Configure the text area to one line or back to normal
 * 将文本框配置为单行或恢复正常模式
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param en        true: one line, false: normal
 *                 true: 单行，false: 正常(多行)
 */
void lv_textarea_set_one_line(lv_obj_t * obj, bool en);

/**
 * Set a list of characters. Only these characters will be accepted by the text area
 * 设置可接受的字符列表。文本框只接受这些字符
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param list      list of characters. A copy is saved. Example: "+-.,0123456789"
 *                 字符列表。列表会被拷贝保存。例如: "+-.,0123456789"
 */
void lv_textarea_set_accepted_chars(lv_obj_t * obj, const char * list);

/**
 * Set a list of characters. Only these characters will be accepted by the text area
 * 设置可接受的字符列表。文本框只接受这些字符
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param list      list of characters. Only the pointer is saved. Example: "+-.,0123456789"
 *                 字符列表。只保存指针。例如: "+-.,0123456789"
 */
void lv_textarea_set_accepted_chars_static(lv_obj_t * obj, const char * list);

/**
 * Set max length of a Text Area.
 * 设置文本框的最大长度。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param num       the maximal number of characters can be added (`lv_textarea_set_text` ignores it)
 *                 可添加的最大字符数 (`lv_textarea_set_text` 忽略该限制)
 */
void lv_textarea_set_max_length(lv_obj_t * obj, uint32_t num);

/**
 * In `LV_EVENT_INSERT` the text which planned to be inserted can be replaced by another text.
 * 在 `LV_EVENT_INSERT` 事件中，计划插入的文本可以被另一段文本替换。
 * It can be used to add automatic formatting to the text area.
 * 它可用于为文本框添加自动格式化功能。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param txt       pointer to a new string to insert. If `""` no text will be added.
 *                 指向要插入的新字符串。若为 `""` 则不会添加任何文本。
 *                  The variable must be live after the `event_cb` exists. (Should be `global` or `static`)
 *                  该变量在 `event_cb` 退出后必须仍然有效(应为 `global` 或 `static`)
 */
void lv_textarea_set_insert_replace(lv_obj_t * obj, const char * txt);

/**
 * Enable/disable selection mode.
 * 启用/禁用选择模式。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param en        true or false to enable/disable selection mode
 *                 true 或 false，用于启用/禁用选择模式
 */
void lv_textarea_set_text_selection(lv_obj_t * obj, bool en);

/**
 * Set how long show the password before changing it to '*'
 * 设置在密码变为 '*' 之前的显示时长
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param time      show time in milliseconds. 0: hide immediately.
 *                 显示时长(毫秒)。0: 立即隐藏。
 */
void lv_textarea_set_password_show_time(lv_obj_t * obj, uint32_t time);

/**
 * @deprecated Use the normal text_align style property instead
 * @deprecated 请改用普通的 text_align 样式属性
 * Set the label's alignment.
 * 设置内部标签(label)的对齐方式。
 * It sets where the label is aligned (in one line mode it can be smaller than the text area)
 * 它设置标签的对齐位置(在单行模式下，标签可能比文本框小)
 * and how the lines of the area align in case of multiline text area
 * 以及多行文本框时文本行的对齐方式
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @param align     the align mode from ::lv_text_align_t
 *                 来自 ::lv_text_align_t 的对齐模式
 */
LV_DEPRECATED("Use text_align style property instead")
void lv_textarea_set_align(lv_obj_t * obj, lv_text_align_t align);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the text of a text area. In password mode it gives the real text (not '*'s).
 * 获取文本框的文本。在密码模式下返回真实文本(而不是 '*' 字符)。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          pointer to the text
 *                 指向文本的指针
 */
const char * lv_textarea_get_text(const lv_obj_t * obj);

/**
 * Get the placeholder text of a text area
 * 获取文本框的占位符文本
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          pointer to the text
 *                 指向文本的指针
 */
const char * lv_textarea_get_placeholder_text(lv_obj_t * obj);

/**
 * Get the label of a text area
 * 获取文本框的内部标签(label)对象
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          pointer to the label object
 *                 指向标签对象的指针
 */
lv_obj_t * lv_textarea_get_label(const lv_obj_t * obj);

/**
 * Get the current cursor position in character index
 * 获取当前光标位置(字符索引)
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          the cursor position
 *                 光标位置
 */
uint32_t lv_textarea_get_cursor_pos(const lv_obj_t * obj);

/**
 * Get whether the cursor click positioning is enabled or not.
 * 获取点击定位光标是否已启用。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          true: enable click positions; false: disable
 *                 true: 已启用点击定位；false: 已禁用
 */
bool lv_textarea_get_cursor_click_pos(lv_obj_t * obj);

/**
 * Get the password mode attribute
 * 获取密码模式属性
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          true: password mode is enabled, false: disabled
 *                 true: 密码模式已启用，false: 已禁用
 */
bool lv_textarea_get_password_mode(const lv_obj_t * obj);

/**
 * Get the replacement characters to show in password mode
 * 获取密码模式下显示的替代字符
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          pointer to the replacement text
 *                 指向替代文本的指针
 */
const char * lv_textarea_get_password_bullet(lv_obj_t * obj);

/**
 * Get the one line configuration attribute
 * 获取单行配置属性
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          true: one line configuration is enabled, false: disabled
 *                 true: 单行配置已启用，false: 已禁用
 */
bool lv_textarea_get_one_line(const lv_obj_t * obj);

/**
 * Get a list of accepted characters.
 * 获取可接受的字符列表。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          list of accented characters.
 *                 可接受的字符列表。
 */
const char * lv_textarea_get_accepted_chars(lv_obj_t * obj);

/**
 * Get max length of a Text Area.
 * 获取文本框的最大长度。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          the maximal number of characters to be add
 *                 可添加的最大字符数
 */
uint32_t lv_textarea_get_max_length(lv_obj_t * obj);

/**
 * Find whether text is selected or not.
 * 查看文本是否被选中。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          whether text is selected or not
 *                 文本是否被选中
 */
bool lv_textarea_text_is_selected(const lv_obj_t * obj);

/**
 * Find whether selection mode is enabled.
 * 查看选择模式是否启用。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          true: selection mode is enabled, false: disabled
 *                 true: 选择模式已启用，false: 已禁用
 */
bool lv_textarea_get_text_selection(lv_obj_t * obj);

/**
 * Set how long show the password before changing it to '*'
 * 获取密码变为 '*' 之前的显示时长
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          show time in milliseconds. 0: hide immediately.
 *                 显示时长(毫秒)。0: 立即隐藏。
 */
uint32_t lv_textarea_get_password_show_time(lv_obj_t * obj);

/**
 * Get a the character from the current cursor position
 * 获取当前光标位置的字符
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 * @return          a the character or 0
 *                 该字符或 0
 */
uint32_t lv_textarea_get_current_char(lv_obj_t * obj);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**
 * Clear the selection on the text area.
 * 清除文本框中的选中内容。
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 */
void lv_textarea_clear_selection(lv_obj_t * obj);

/**
 * Move the cursor one character right
 * 将光标向右移动一个字符
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 */
void lv_textarea_cursor_right(lv_obj_t * obj);

/**
 * Move the cursor one character left
 * 将光标向左移动一个字符
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 */
void lv_textarea_cursor_left(lv_obj_t * obj);

/**
 * Move the cursor one line down
 * 将光标向下移动一行
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 */
void lv_textarea_cursor_down(lv_obj_t * obj);

/**
 * Move the cursor one line up
 * 将光标向上移动一行
 * @param obj       pointer to a text area object
 *                 指向文本框对象的指针
 */
void lv_textarea_cursor_up(lv_obj_t * obj);

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_TEXTAREA_H*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEXTAREA_H*/
