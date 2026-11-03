/**
 * @file lv_label.h
 * 标签(Label)控件：用于显示文本，支持换行、省略号、滚动、文本选择与重着色等特性
 *
 */

#ifndef LV_LABEL_H
#define LV_LABEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_LABEL != 0

#include "../lv_types.h"
#include "../core/lv_obj.h"
#include "../font/lv_font.h"
#include "../font/lv_symbol_def.h"
#include "../font/lv_text.h"
#include "../draw/lv_draw.h"
#include "../core/lv_observer.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/
/*文本过长时显示的省略号个数*/
#define LV_LABEL_DOT_NUM 3
/*标签文本末尾位置标记*/
#define LV_LABEL_POS_LAST 0xFFFF
/*关闭文本选择*/
#define LV_LABEL_TEXT_SELECTION_OFF LV_DRAW_LABEL_NO_TXT_SEL
#if LV_WIDGETS_HAS_DEFAULT_VALUE
/*标签的默认文本*/
#define LV_LABEL_DEFAULT_TEXT "Text"
#else
#define LV_LABEL_DEFAULT_TEXT ""
#endif

LV_EXPORT_CONST_INT(LV_LABEL_DOT_NUM);
LV_EXPORT_CONST_INT(LV_LABEL_POS_LAST);
LV_EXPORT_CONST_INT(LV_LABEL_TEXT_SELECTION_OFF);

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/** Long mode behaviors. Used in 'lv_label_ext_t'*/
/** 长文本模式行为，用于 'lv_label_ext_t'*/
typedef enum {
    /*换行模式：保持对象宽度，超宽的行换行显示，并扩展对象高度*/
    LV_LABEL_LONG_MODE_WRAP,             /**< Keep the object width, wrap lines longer than object width and expand the object height*/
    /*省略号模式：保持尺寸不变，文本过长时在末尾显示省略号*/
    LV_LABEL_LONG_MODE_DOTS,             /**< Keep the size and write dots at the end if the text is too long*/
    /*滚动模式：保持尺寸不变，文本来回滚动显示*/
    LV_LABEL_LONG_MODE_SCROLL,           /**< Keep the size and roll the text back and forth*/
    /*循环滚动模式：保持尺寸不变，文本循环滚动显示*/
    LV_LABEL_LONG_MODE_SCROLL_CIRCULAR,  /**< Keep the size and roll the text circularly*/
    /*裁剪模式：保持尺寸不变，超出范围的文本被裁剪*/
    LV_LABEL_LONG_MODE_CLIP,             /**< Keep the size and clip the text out of it*/
} lv_label_long_mode_t;

#if LV_USE_OBJ_PROPERTY
/*标签对象属性ID枚举*/
enum _lv_property_label_id_t {
    /*标签文本*/
    LV_PROPERTY_ID(LABEL, TEXT,                   LV_PROPERTY_TYPE_TEXT,      0),
    /*长文本模式*/
    LV_PROPERTY_ID(LABEL, LONG_MODE,              LV_PROPERTY_TYPE_INT,       1),
    /*最大显示行数*/
    LV_PROPERTY_ID(LABEL, MAX_LINES,              LV_PROPERTY_TYPE_INT,       2),
    /*文本选择起始索引*/
    LV_PROPERTY_ID(LABEL, TEXT_SELECTION_START,   LV_PROPERTY_TYPE_INT,       3),
    /*文本选择结束索引*/
    LV_PROPERTY_ID(LABEL, TEXT_SELECTION_END,     LV_PROPERTY_TYPE_INT,       4),
    LV_PROPERTY_LABEL_END,
};
#endif

/*标签控件的类对象*/
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_label_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a label object
 * 创建标签对象
 * @param parent    pointer to an object, it will be the parent of the new label.
 * @param parent    指向父对象的指针，它将成为新标签的父对象
 * @return          pointer to the created button
 * @return          指向已创建标签对象的指针
 */
lv_obj_t * lv_label_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set a new text for a label. Memory will be allocated to store the text by the label.
 * 为标签设置新文本，标签会分配内存来存储该文本
 * @param obj           pointer to a label object
 * @param obj           指向标签对象的指针
 * @param text          '\0' terminated character string. NULL to refresh with the current text.
 * @param text          '\0' 结尾的字符串。传 NULL 则使用当前文本刷新显示
 * @note If `LV_USE_ARABIC_PERSIAN_CHARS` is enabled the text will be modified to have the correct Arabic
 * @note 如果启用了 `LV_USE_ARABIC_PERSIAN_CHARS`，文本将被修改为包含正确的阿拉伯字符
 * characters in it.
 * 字符
 */
void lv_label_set_text(lv_obj_t * obj, const char * text);

/**
 * Set a new formatted text for a label. Memory will be allocated to store the text by the label.
 * 为标签设置新的格式化文本，标签会分配内存来存储该文本
 * @param obj           pointer to a label object
 * @param obj           指向标签对象的指针
 * @param fmt           `printf`-like format string
 * @param fmt           `printf` 风格的格式字符串
 * Example:
 * 示例：
 * @code
 * lv_label_set_text_fmt(label1, "%d user", user_num);
 * @endcode
 * @note If `LV_USE_ARABIC_PERSIAN_CHARS` is enabled the text will be modified to have the correct Arabic characters in it.
 * @note 如果启用了 `LV_USE_ARABIC_PERSIAN_CHARS`，文本将被修改为包含正确的阿拉伯字符
 */
void lv_label_set_text_fmt(lv_obj_t * obj, const char * fmt, ...) LV_FORMAT_ATTRIBUTE(2, 3);

/**
 * Set a new formatted text for a label. Memory will be allocated to store the text by the label.
 * 为标签设置新的格式化文本（使用可变参数列表），标签会分配内存来存储该文本
 * @param obj           pointer to a label object
 * @param obj           指向标签对象的指针
 * @param fmt           `printf`-like format string
 * @param fmt           `printf` 风格的格式字符串
 * @param args          variadic arguments list
 * @param args          可变参数列表
 *
 * Example:
 * 示例：
 * @code
 * va_list args;
 * va_start(args, fmt);
 * lv_label_set_text_vfmt(label1, fmt, args);
 * va_end(args);
 * @endcode
 * @note It ignores `LV_USE_ARABIC_PERSIAN_CHARS`
 * @note 此函数会忽略 `LV_USE_ARABIC_PERSIAN_CHARS` 设置
 */
void lv_label_set_text_vfmt(lv_obj_t * obj, const char * fmt, va_list args);

/**
 * Set a static text. It will not be saved by the label so the 'text' variable
 * 设置静态文本。标签不会保存该文本，因此 'text' 变量
 * has to be 'alive' while the label exists.
 * 在标签存续期间必须保持有效
 * @param obj           pointer to a label object
 * @param obj           指向标签对象的指针
 * @param text          pointer to a text. NULL to refresh with the current text.
 * @param text          指向文本的指针。传 NULL 则使用当前文本刷新显示
 * @note It ignores `LV_USE_ARABIC_PERSIAN_CHARS`
 * @note 此函数会忽略 `LV_USE_ARABIC_PERSIAN_CHARS` 设置
 */
void lv_label_set_text_static(lv_obj_t * obj, const char * text);

/**
 * Set the behavior of the label with text longer than the object size
 * 设置文本长度超过对象尺寸时标签的行为
 * @param obj           pointer to a label object
 * @param obj           指向标签对象的指针
 * @param long_mode     the new mode from 'lv_label_long_mode' enum.
 * @param long_mode     来自 'lv_label_long_mode' 枚举的新模式
 *                      In LV_LONG_WRAP/DOT/SCROLL/SCROLL_CIRC the size of the label should be set AFTER this function
 *                      在 LV_LONG_WRAP/DOT/SCROLL/SCROLL_CIRC 模式下，标签的尺寸应在此函数调用之后设置
 */
void lv_label_set_long_mode(lv_obj_t * obj, lv_label_long_mode_t long_mode);

/**
 * Set the maximum number of lines that the label should display in
 * 设置标签在以下模式中应显示的最大行数
 * `LV_LABEL_LONG_MODE_WRAP` and `LV_LABEL_LONG_MODE_DOTS` mode.
 * `LV_LABEL_LONG_MODE_WRAP` 和 `LV_LABEL_LONG_MODE_DOTS` 模式
 * @param obj     pointer to a label object
 * @param obj     指向标签对象的指针
 * @param lines   number of lines to display (unlimited if not positive)
 * @param lines   要显示的行数（非正数表示不限制）
 */
void lv_label_set_max_lines(lv_obj_t * obj, int32_t lines);

/**
 * Set where text selection should start
 * 设置文本选择的起始位置
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @param index     character index from where selection should start. `LV_LABEL_TEXT_SELECTION_OFF` for no selection
 * @param index     选择起始的字符索引，`LV_LABEL_TEXT_SELECTION_OFF` 表示无选择
 */
void lv_label_set_text_selection_start(lv_obj_t * obj, uint32_t index);

/**
 * Set where text selection should end
 * 设置文本选择的结束位置
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @param index     character index where selection should end. `LV_LABEL_TEXT_SELECTION_OFF` for no selection
 * @param index     选择结束的字符索引，`LV_LABEL_TEXT_SELECTION_OFF` 表示无选择
 */
void lv_label_set_text_selection_end(lv_obj_t * obj, uint32_t index);

/**
 * Enable the recoloring by in-line commands
 * 启用通过行内命令在文本中重设颜色
 * @param obj           pointer to a label object
 * @param obj           指向标签对象的指针
 * @param en            true: enable recoloring, false: disable
 * @param en            true：启用重新着色；false：禁用
 * Example: "This is a #ff0000 red# word"
 * 示例："This is a #ff0000 red# word"（#颜色值 文本# 的格式）
 */
void lv_label_set_recolor(lv_obj_t * obj, bool en);

#if LV_USE_TRANSLATION

/**
 * Assign a translation tag for this label. Memory will be allocated to store the tag by the label.
 * 为标签分配一个翻译标签，标签会分配内存来存储该标签
 * The label text will automatically update when the language is changed via `lv_translation_set_language`.
 * 当通过 `lv_translation_set_language` 切换语言时，标签文本会自动更新
 * @param obj           pointer to a label object
 * @param obj           指向标签对象的指针
 * @param tag          '\0' terminated character string.
 * @param tag           '\0' 结尾的字符串
 */
void lv_label_set_translation_tag(lv_obj_t * obj, const char * tag);

#endif /*LV_USE_TRANSLATION*/


/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the text of a label
 * 获取标签的文本
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @return          the text of the label
 * @return          标签的文本
 */
char * lv_label_get_text(const lv_obj_t * obj);

/**
 * Get the long mode of a label
 * 获取标签的长文本模式
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @return          the current long mode
 * @return          当前的长文本模式
 */
lv_label_long_mode_t lv_label_get_long_mode(const lv_obj_t * obj);

/**
 * Get the maximum number of lines that a label should display
 * 获取标签应显示的最大行数
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @return          the maximum number of lines, if positive
 * @return          最大行数（若为正数）
 */
int32_t lv_label_get_max_lines(const lv_obj_t * obj);

/**
 * Get the relative x and y coordinates of a letter
 * 获取某个字符的相对X和Y坐标
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @param char_id   index of the character [0 ... text length - 1].
 * @param char_id   字符索引 [0 ... 文本长度 - 1]
 *                  Expressed in character index, not byte index (different in UTF-8)
 *                  使用字符索引而非字节索引表示（在UTF-8中两者不同）
 * @param pos       store the result here (E.g. index = 0 gives 0;0 coordinates if the text if aligned to the left)
 * @param pos       在此处存储结果（例如：文本左对齐时，索引0对应坐标0;0）
 */
void lv_label_get_letter_pos(const lv_obj_t * obj, uint32_t char_id, lv_point_t * pos);

/**
 * Get the index of letter on a relative point of a label.
 * 获取标签上某个相对位置处字符的索引
 * @param obj       pointer to label object
 * @param obj       指向标签对象的指针
 * @param pos_in    pointer to point with coordinates on a the label
 * @param pos_in    指向标签上一个坐标点的指针
 * @param bidi      whether to use bidi processed
 * @param bidi      是否使用双向文本处理
 * @return          The index of the letter on the 'pos_p' point (E.g. on 0;0 is the 0. letter if aligned to the left)
 * @return          'pos_p' 点处字符的索引（例如：左对齐时，0;0处是第0个字符）
 *                  Expressed in character index and not byte index (different in UTF-8)
 *                  使用字符索引而非字节索引表示（在UTF-8中两者不同）
 */
uint32_t lv_label_get_letter_on(const lv_obj_t * obj, lv_point_t * pos_in, bool bidi);

/**
 * Check if a character is drawn under a point.
 * 检查某个点的下方是否绘制了字符
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @param pos       Point to check for character under
 * @param pos       要检查其下方是否有字符的点
 * @return          whether a character is drawn under the point
 * @return          该点下方是否绘制了字符
 */
bool lv_label_is_char_under_pos(const lv_obj_t * obj, lv_point_t * pos);

/**
 * @brief Get the selection start index.
 * @brief 获取文本选择的起始索引
 * @param obj       pointer to a label object.
 * @param obj       指向标签对象的指针
 * @return          selection start index. `LV_LABEL_TEXT_SELECTION_OFF` if nothing is selected.
 * @return          选择起始索引，未选择任何内容时返回 `LV_LABEL_TEXT_SELECTION_OFF`
 */
uint32_t lv_label_get_text_selection_start(const lv_obj_t * obj);

/**
 * @brief Get the selection end index.
 * @brief 获取文本选择的结束索引
 * @param obj       pointer to a label object.
 * @param obj       指向标签对象的指针
 * @return          selection end index. `LV_LABEL_TXT_SEL_OFF` if nothing is selected.
 * @return          选择结束索引，未选择任何内容时返回 `LV_LABEL_TXT_SEL_OFF`
 */
uint32_t lv_label_get_text_selection_end(const lv_obj_t * obj);

/**
 * @brief Get the recoloring attribute
 * @brief 获取重新着色属性
 * @param obj       pointer to a label object.
 * @param obj       指向标签对象的指针
 * @return          true: recoloring is enabled, false: recoloring is disabled
 * @return          true：已启用重新着色；false：已禁用
 */
bool lv_label_get_recolor(const lv_obj_t * obj);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

#if LV_USE_OBSERVER
/**
 * Bind an integer, string, or pointer Subject to a Label.
 * 将整数、字符串或指针类型的Subject(可观察对象)绑定到标签
 * @param obj       pointer to Label
 * @param obj       指向标签对象的指针
 * @param subject   pointer to Subject
 * @param subject   指向Subject(可观察对象)的指针
 * @param fmt       optional printf-like format string with 1 format specifier (e.g. "%d °C")
 * @param fmt       可选的 printf 风格格式字符串，含1个格式说明符（例如 "%d °C"）
 *                  or NULL to bind to the value directly.
 *                  或传 NULL 直接绑定到值
 * @return          pointer to newly-created Observer
 * @return          指向新创建的Observer的指针
 * @note            If `fmt == NULL` strings and pointers (`\0` terminated string) will be shown
 * @note            如果 `fmt == NULL`，字符串和指针（`\0` 结尾的字符串）将按原样作为文本显示，
 *                  as text as they are, integers as %d, floats as %0.1f
 *                  整数按 %d 显示，浮点数按 %0.1f 显示
 */
lv_observer_t * lv_label_bind_text(lv_obj_t * obj, lv_subject_t * subject, const char * fmt);
#endif


/**
 * Insert a text to a label. The label text cannot be static.
 * 向标签中插入文本。标签文本不能是静态的
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @param pos       character index to insert. Expressed in character index and not byte index.
 * @param pos       要插入的字符索引，使用字符索引而非字节索引
 *                  0: before first char. LV_LABEL_POS_LAST: after last char.
 *                  0：第一个字符之前；LV_LABEL_POS_LAST：最后一个字符之后
 * @param txt       pointer to the text to insert
 * @param txt       指向要插入文本的指针
 */
void lv_label_ins_text(lv_obj_t * obj, uint32_t pos, const char * txt);

/**
 * Delete characters from a label. The label text cannot be static.
 * 从标签中删除字符。标签文本不能是静态的
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @param pos       character index from where to cut. Expressed in character index and not byte index.
 * @param pos       开始删除的字符索引，使用字符索引而非字节索引
 *                  0: start in front of the first character
 *                  0：从第一个字符之前开始
 * @param cnt       number of characters to cut
 * @param cnt       要删除的字符数量
 */
void lv_label_cut_text(lv_obj_t * obj, uint32_t pos, uint32_t cnt);



/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_LABEL*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LABEL_H*/
