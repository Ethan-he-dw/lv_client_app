/**
 * @file lv_dropdown.h
 * 下拉列表(dropdown)控件头文件：提供选项列表、选中项获取/设置、展开/收起等功能。
 *
 */

#ifndef LV_DROPDOWN_H
#define LV_DROPDOWN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_DROPDOWN != 0

/*Testing of dependencies*/
/** 依赖检查 */

#if LV_USE_LABEL == 0
#error "lv_dropdown: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1)"
#endif

#include "lv_label.h"

/*********************
 *      DEFINES
 * 宏定义
 *********************/
/** 表示列表末尾的特殊位置值 */
#define LV_DROPDOWN_POS_LAST 0xFFFF
LV_EXPORT_CONST_INT(LV_DROPDOWN_POS_LAST);

#if LV_USE_OBJ_PROPERTY
/** 下拉列表对象的属性ID枚举(仅当启用对象属性系统时有效) */
enum _lv_property_dropdown_id_t {
    LV_PROPERTY_ID(DROPDOWN, TEXT,                LV_PROPERTY_TYPE_TEXT,   0),
    LV_PROPERTY_ID(DROPDOWN, OPTIONS,             LV_PROPERTY_TYPE_TEXT,   1),
    LV_PROPERTY_ID(DROPDOWN, OPTION_COUNT,        LV_PROPERTY_TYPE_INT,    2),
    LV_PROPERTY_ID(DROPDOWN, SELECTED,            LV_PROPERTY_TYPE_INT,    3),
    // LV_PROPERTY_ID(DROPDOWN, SELECTED_STR,        LV_PROPERTY_TYPE_TEXT,   4),
    LV_PROPERTY_ID(DROPDOWN, DIR,                 LV_PROPERTY_TYPE_INT,    5),
    LV_PROPERTY_ID(DROPDOWN, SYMBOL,              LV_PROPERTY_TYPE_IMGSRC, 6),
    LV_PROPERTY_ID(DROPDOWN, SELECTED_HIGHLIGHT,  LV_PROPERTY_TYPE_INT,    7),
    LV_PROPERTY_ID(DROPDOWN, LIST,                LV_PROPERTY_TYPE_OBJ,    8),
    LV_PROPERTY_ID(DROPDOWN, IS_OPEN,             LV_PROPERTY_TYPE_BOOL,   9),
    LV_PROPERTY_DROPDOWN_END,
};
#endif

/** 下拉列表控件的类结构体 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_dropdown_class;
/** 下拉列表弹出列表控件的类结构体 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_dropdownlist_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a drop-down list object
 * 创建一个下拉列表对象
 * @param parent pointer to an object, it will be the parent of the new drop-down list
 * @param parent 指向一个对象，它将成为新下拉列表的父对象
 * @return pointer to the created drop-down list
 * @return 指向创建好的下拉列表对象的指针
 */
lv_obj_t * lv_dropdown_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set text of the drop-down list's button.
 * 设置下拉列表按钮的文本。
 * If set to `NULL` the selected option's text will be displayed on the button.
 * 如果设置为 `NULL`，按钮上会显示所选选项的文本。
 * If set to a specific text then that text will be shown regardless of the selected option.
 * 如果设置为特定文本，则无论选中哪个选项，都会显示该文本。
 * @param obj       pointer to a drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @param text      the text as a string (Copy is saved)
 * @param text      作为字符串的文本(会保存副本)
 */
void lv_dropdown_set_text(lv_obj_t * obj, const char * text);

/**
 * Set text of the drop-down list's button.
 * 设置下拉列表按钮的文本。
 * If set to `NULL` the selected option's text will be displayed on the button.
 * 如果设置为 `NULL`，按钮上会显示所选选项的文本。
 * If set to a specific text then that text will be shown regardless of the selected option.
 * 如果设置为特定文本，则无论选中哪个选项，都会显示该文本。
 * @param obj       pointer to a drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @param text      the text as a string (Only its pointer is saved)
 * @param text      作为字符串的文本(仅保存其指针)
 */
void lv_dropdown_set_text_static(lv_obj_t * obj, const char * text);

/**
 * Set the options in a drop-down list from a string.
 * 通过字符串设置下拉列表中的选项。
 * The options will be copied and saved in the object so the `options` can be destroyed after calling this function
 * 选项会被复制并保存在对象中，因此调用本函数后可以销毁 `options`
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @param options   a string with '\n' separated options. E.g. "One\nTwo\nThree"
 * @param options   以 '\n' 分隔选项的字符串。例如 "One\nTwo\nThree"
 */
void lv_dropdown_set_options(lv_obj_t * obj, const char * options);

/**
 * Set the options in a drop-down list from a static string (global, static or dynamically allocated).
 * 通过静态字符串(全局、静态或动态分配的字符串)设置下拉列表中的选项。
 * Only the pointer of the option string will be saved.
 * 仅保存选项字符串的指针。
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @param options   a static string with '\n' separated options. E.g. "One\nTwo\nThree"
 * @param options   以 '\n' 分隔选项的静态字符串。例如 "One\nTwo\nThree"
 */
void lv_dropdown_set_options_static(lv_obj_t * obj, const char * options);

/**
 * Add an options to a drop-down list from a string.  Only works for non-static options.
 * 通过字符串向下拉列表中添加一个选项。仅适用于非静态选项。
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @param option    a string without '\n'. E.g. "Four"
 * @param option    不含 '\n' 的字符串。例如 "Four"
 * @param pos       the insert position, indexed from 0, LV_DROPDOWN_POS_LAST = end of string
 * @param pos       插入位置，从 0 开始索引，LV_DROPDOWN_POS_LAST 表示字符串末尾
 */
void lv_dropdown_add_option(lv_obj_t * obj, const char * option, uint32_t pos);

/**
 * Clear all options in a drop-down list.  Works with both static and dynamic options.
 * 清空下拉列表中的所有选项。对静态和动态选项均有效。
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 */
void lv_dropdown_clear_options(lv_obj_t * obj);

/**
 * Set the selected option
 * 设置选中的选项
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @param sel_opt   id of the selected option (0 ... number of option - 1);
 * @param sel_opt   选中选项的 ID(0 ... 选项个数 - 1)；
 */
void lv_dropdown_set_selected(lv_obj_t * obj, uint32_t sel_opt);

/**
 * Set the direction of the a drop-down list
 * 设置下拉列表的展开方向
 * @param obj       pointer to a drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @param dir       LV_DIR_LEFT/RIGHT/TOP/BOTTOM
 * @param dir       LV_DIR_LEFT/RIGHT/TOP/BOTTOM
 */
void lv_dropdown_set_dir(lv_obj_t * obj, lv_dir_t dir);

/**
 * Set an arrow or other symbol to display when on drop-down list's button. Typically a down caret or arrow.
 * 设置显示在下拉列表按钮上的箭头或其他符号。通常是向下的小箭头或箭头。
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @param symbol    a text like `LV_SYMBOL_DOWN`, an image (pointer or path) or NULL to not draw symbol icon
 * @param symbol    文本如 `LV_SYMBOL_DOWN`、图像(指针或路径)，或 NULL 表示不绘制符号图标
 * @note angle and zoom transformation can be applied if the symbol is an image.
 * @note 如果符号是图像，则可以应用角度和缩放变换。
 * E.g. when drop down is checked (opened) rotate the symbol by 180 degree
 * 例如，当下拉列表被展开时，可将符号旋转 180 度
 */
void lv_dropdown_set_symbol(lv_obj_t * obj, const void * symbol);

/**
 * Set whether the selected option in the list should be highlighted or not
 * 设置列表中的选中选项是否需要高亮显示
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @param en        true: highlight enabled; false: disabled
 * @param en        true：启用高亮；false：禁用
 */
void lv_dropdown_set_selected_highlight(lv_obj_t * obj, bool en);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the list of a drop-down to allow styling or other modifications
 * 获取下拉列表的列表对象，以便进行样式设置或其他修改
 * @param obj       pointer to a drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @return          pointer to the list of the drop-down
 * @return          指向下拉列表的列表对象的指针
 */
lv_obj_t * lv_dropdown_get_list(lv_obj_t * obj);

/**
 * Get text of the drop-down list's button.
 * 获取下拉列表按钮的文本。
 * @param obj   pointer to a drop-down list object
 * @param obj   指向下拉列表对象的指针
 * @return      the text as string, `NULL` if no text
 * @return      作为字符串的文本，如果没有文本则返回 `NULL`
 */
const char * lv_dropdown_get_text(lv_obj_t * obj);

/**
 * Get the options of a drop-down list
 * 获取下拉列表的选项
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @return          the options separated by '\n'-s (E.g. "Option1\nOption2\nOption3")
 * @return          以 '\n' 分隔的选项字符串(例如 "Option1\nOption2\nOption3")
 */
const char * lv_dropdown_get_options(const lv_obj_t * obj);

/**
 * Get the index of the selected option
 * 获取选中选项的索引
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @return          index of the selected option (0 ... number of option - 1);
 * @return          选中选项的索引(0 ... 选项个数 - 1)；
 */
uint32_t lv_dropdown_get_selected(const lv_obj_t * obj);

/**
 * Get the total number of options
 * 获取选项的总数
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @return          the total number of options in the list
 * @return          列表中选项的总数
 */
uint32_t lv_dropdown_get_option_count(const lv_obj_t * obj);

/**
 * Get the current selected option as a string
 * 获取当前选中选项的字符串
 * @param obj       pointer to drop-down object
 * @param obj       指向下拉列表对象的指针
 * @param buf       pointer to an array to store the string
 * @param buf       指向用于存储字符串的数组的指针
 * @param buf_size  size of `buf` in bytes. 0: to ignore it.
 * @param buf_size  `buf` 的大小(字节)。0：表示忽略该参数。
 */
void lv_dropdown_get_selected_str(const lv_obj_t * obj, char * buf, uint32_t buf_size);

/**
 * Get the index of an option.
 * 获取某个选项的索引。
 * @param obj       pointer to drop-down object
 * @param obj       指向下拉列表对象的指针
 * @param option    an option as string
 * @param option    作为字符串的选项
 * @return          index of `option` in the list of all options. -1 if not found.
 * @return          `option` 在所有选项列表中的索引。未找到时返回 -1。
 */
int32_t lv_dropdown_get_option_index(lv_obj_t * obj, const char * option);

/**
 * Get the symbol on the drop-down list. Typically a down caret or arrow.
 * 获取下拉列表上的符号。通常是向下的小箭头或箭头。
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @return          the symbol or NULL if not enabled
 * @return          符号，如果未启用则返回 NULL
 */
const char * lv_dropdown_get_symbol(lv_obj_t * obj);

/**
 * Get whether the selected option in the list should be highlighted or not
 * 获取列表中的选中选项是否需要高亮显示
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @return          true: highlight enabled; false: disabled
 * @return          true：启用高亮；false：禁用
 */
bool lv_dropdown_get_selected_highlight(lv_obj_t * obj);

/**
 * Get the direction of the drop-down list
 * 获取下拉列表的展开方向
 * @param obj       pointer to a drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @return          LV_DIR_LEF/RIGHT/TOP/BOTTOM
 * @return          LV_DIR_LEF/RIGHT/TOP/BOTTOM
 */
lv_dir_t lv_dropdown_get_dir(const lv_obj_t * obj);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**
 * Open the drop.down list
 * 打开下拉列表
 * @param dropdown_obj       pointer to drop-down list object
 * @param dropdown_obj       指向下拉列表对象的指针
 */
void lv_dropdown_open(lv_obj_t * dropdown_obj);

/**
 * Close (Collapse) the drop-down list
 * 关闭(收起)下拉列表
 * @param obj       pointer to drop-down list object
 * @param obj       指向下拉列表对象的指针
 */
void lv_dropdown_close(lv_obj_t * obj);

/**
 * Tells whether the list is opened or not
 * 判断列表是否处于打开状态
 * @param obj       pointer to a drop-down list object
 * @param obj       指向下拉列表对象的指针
 * @return          true if the list os opened
 * @return          如果列表已打开则返回 true
 */
bool lv_dropdown_is_open(lv_obj_t * obj);


#if LV_USE_OBSERVER
/**
 * Bind an integer Subject to a Dropdown's value.
 * 将一个整数 Subject 绑定到下拉列表的值上。
 * @param obj       pointer to Dropdown
 * @param obj       指向下拉列表对象的指针
 * @param subject   pointer to Subject
 * @param subject   指向 Subject 的指针
 * @return          pointer to newly-created Observer
 * @return          指向新建 Observer 的指针
 */
lv_observer_t * lv_dropdown_bind_value(lv_obj_t * obj, lv_subject_t * subject);
#endif

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_DROPDOWN*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DROPDOWN_H*/
