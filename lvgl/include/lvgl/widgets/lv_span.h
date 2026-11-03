/**
 * @file lv_span.h
 * @brief 跨段(Span)控件,用于在文本组(Spangroup)中显示带不同样式的文本片段
 *
 */

#ifndef LV_SPAN_H
#define LV_SPAN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"
#include "../core/lv_obj.h"
#include "../core/lv_observer.h"
#include "../core/lv_obj_property.h"

#if LV_USE_SPAN != 0

/*********************
 *      DEFINES
 *      宏定义
 *********************/
/** span 内部使用的栈大小 */
#ifndef LV_SPAN_SNIPPET_STACK_SIZE
#define LV_SPAN_SNIPPET_STACK_SIZE 64
#endif

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/
typedef enum {
    /** 溢出时裁剪,不显示多余内容 */
    LV_SPAN_OVERFLOW_CLIP,
    /** 溢出时以省略号(...)显示 */
    LV_SPAN_OVERFLOW_ELLIPSIS,
    LV_SPAN_OVERFLOW_LAST,  /**< Fence member*/
                            /**< 边界成员 */
} lv_span_overflow_t;

typedef enum {
    LV_SPAN_MODE_FIXED,     /**< fixed the obj size */
                            /**< 固定对象大小 */
    LV_SPAN_MODE_EXPAND,    /**< Expand the object size to the text size */
                            /**< 将对象大小扩展到文本大小 */
    LV_SPAN_MODE_BREAK,     /**< Keep width, break the too long lines and expand height */
                            /**< 保持宽度,过长的行自动换行并增加高度 */
    LV_SPAN_MODE_LAST       /**< Fence member */
                            /**< 边界成员 */
} lv_span_mode_t;

/** Coords of a span */
/** 跨段的坐标 */
typedef struct _lv_span_coords_t {
    lv_area_t heading;  /**< 跨段首部的区域坐标 */
    lv_area_t middle;   /**< 跨段中部的区域坐标 */
    lv_area_t trailing; /**< 跨段尾部的区域坐标 */
} lv_span_coords_t;

/** 文本组(Spangroup)控件的对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_spangroup_class;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_span_id_t {
    LV_PROPERTY_ID(SPAN, ALIGN,     LV_PROPERTY_TYPE_INT, 0),
    LV_PROPERTY_ID(SPAN, OVERFLOW,  LV_PROPERTY_TYPE_INT, 1),
    LV_PROPERTY_ID(SPAN, INDENT,    LV_PROPERTY_TYPE_INT, 2),
    LV_PROPERTY_ID(SPAN, MODE,      LV_PROPERTY_TYPE_INT, 3),
    LV_PROPERTY_ID(SPAN, MAX_LINES, LV_PROPERTY_TYPE_INT, 4),
    LV_PROPERTY_SPAN_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/** 初始化 span 组件内部使用的栈 */
void lv_span_stack_init(void);
/** 释放 span 组件内部使用的栈 */
void lv_span_stack_deinit(void);

/**
 * Create a spangroup object
 * 创建一个文本组(Spangroup)控件对象
 * @param parent    pointer to an object, it will be the parent of the new spangroup
 * @param parent    指向一个对象,它将是新文本组的父对象
 * @return          pointer to the created spangroup
 * @return          指向创建的文本组对象的指针
 */
lv_obj_t * lv_spangroup_create(lv_obj_t * parent);

/**
 * Create a span string descriptor and add to spangroup.
 * 创建一个跨段字符串描述符并添加到文本组中。
 * @param obj       pointer to a spangroup object.
 * @param obj       指向文本组对象。
 * @return          pointer to the created span.
 * @return          指向创建的跨段的指针。
 */
lv_span_t * lv_spangroup_add_span(lv_obj_t * obj);

/**
 * Remove the span from the spangroup and free memory.
 * 从文本组中移除跨段并释放内存。
 * @param obj   pointer to a spangroup object.
 * @param obj   指向文本组对象。
 * @param span  pointer to a span.
 * @param span  指向跨段。
 * @note        Note that before calling `lv_spangroup_delete_span`
 * @note        注意:在调用 `lv_spangroup_delete_span` 之前
 *              `lv_observer_remove` needs to be called manually as LVGL can't remove the
 *              需要手动调用 `lv_observer_remove`,因为 LVGL 无法自动移除
 *              binding automatically.
 *              绑定关系。
 */
void lv_spangroup_delete_span(lv_obj_t * obj, lv_span_t * span);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set a new text for a span. Memory will be allocated to store the text by the span.
 * 为跨段设置新的文本。跨段会自动分配内存来保存该文本。
 * As the spangroup is not passed a redraw (invalidation) can't be triggered automatically.
 * 由于未传入文本组,无法自动触发重绘(失效)。
 * Therefore `lv_spangroup_refresh(spangroup)` needs to be called manually,
 * 因此需要手动调用 `lv_spangroup_refresh(spangroup)`,
 * @param span  pointer to a span.
 * @param span  指向跨段。
 * @param text  pointer to a text.
 * @param text  指向文本的指针。
 */
void lv_span_set_text(lv_span_t * span, const char * text);


/**
 * Set a new text for a span using a printf-like formatting string.
 * 使用类似 printf 的格式化字符串为跨段设置新的文本。
 * Memory will be allocated to store the text by the span.
 * 跨段会自动分配内存来保存该文本。
 * As the spangroup is not passed a redraw (invalidation) can't be triggered automatically.
 * 由于未传入文本组,无法自动触发重绘(失效)。
 * Therefore `lv_spangroup_refresh(spangroup)` needs to be called manually,
 * 因此需要手动调用 `lv_spangroup_refresh(spangroup)`,
 * @param span  pointer to a span.
 * @param span  指向跨段。
 * @param fmt   `printf`-like format string
 * @param fmt   `printf` 风格的格式化字符串
 */
void lv_span_set_text_fmt(lv_span_t * span, const char * fmt, ...) LV_FORMAT_ATTRIBUTE(2, 3);

/**
 * Set a static text. It will not be saved by the span so the 'text' variable
 * has to be 'alive' while the span exist.
 * 设置静态文本。跨段不会保存该文本,因此在跨段存在期间 'text' 变量必须始终有效。
 * As the spangroup is not passed a redraw (invalidation) can't be triggered automatically.
 * 由于未传入文本组,无法自动触发重绘(失效)。
 * Therefore `lv_spangroup_refresh(spangroup)` needs to be called manually,
 * 因此需要手动调用 `lv_spangroup_refresh(spangroup)`,
 *
 * @param span  pointer to a span.
 * @param span  指向跨段。
 * @param text  pointer to a text.
 * @param text  指向文本的指针。
 */
void lv_span_set_text_static(lv_span_t * span, const char * text);

/**
 * Set a new text for a span. Memory will be allocated to store the text by the span.
 * 为跨段设置新的文本。跨段会自动分配内存来保存该文本。
 * @param obj   pointer to a spangroup widget.
 * @param obj   指向文本组控件。
 * @param span  pointer to a span.
 * @param span  指向跨段。
 * @param text  pointer to a text.
 * @param text  指向文本的指针。
 */
void lv_spangroup_set_span_text(lv_obj_t * obj, lv_span_t * span, const char * text);

/**
 * Set a new text for a span. Memory will be allocated to store the text by the span.
 * 为跨段设置新的静态文本。跨段会自动分配内存来保存该文本。
 * @param obj   pointer to a spangroup widget.
 * @param obj   指向文本组控件。
 * @param span  pointer to a span.
 * @param span  指向跨段。
 * @param text  pointer to a text.
 * @param text  指向文本的指针。
 */
void lv_spangroup_set_span_text_static(lv_obj_t * obj, lv_span_t * span, const char * text);

/**
 * Set a new text for a span using a printf-like formatting string.
 * 使用类似 printf 的格式化字符串为跨段设置新的文本。
 * Memory will be allocated to store the text by the span.
 * 跨段会自动分配内存来保存该文本。
 * @param obj   pointer to a spangroup widget.
 * @param obj   指向文本组控件。
 * @param span  pointer to a span.
 * @param span  指向跨段。
 * @param fmt   `printf`-like format string
 * @param fmt   `printf` 风格的格式化字符串
 */
void lv_spangroup_set_span_text_fmt(lv_obj_t * obj, lv_span_t * span, const char * fmt, ...) LV_FORMAT_ATTRIBUTE(3, 4);

/**
 * Set a static text. It will not be saved by the span so the 'text' variable
 * has to be 'alive' while the span exist.
 * 设置静态文本。跨段不会保存该文本,因此在跨段存在期间 'text' 变量必须始终有效。
 * @param span  pointer to a span.
 * @param span  指向跨段。
 * @param text  pointer to a text.
 * @param text  指向文本的指针。
 */
void lv_span_set_text_static(lv_span_t * span, const char * text);

/**
 * Copy all style properties of style to the bbuilt-in static style of the span.
 * 将 style 的所有样式属性复制到跨段的内置静态样式中。
 * @param obj       pointer_to a spangroup
 * @param obj       指向文本组
 * @param span      pointer to a span.
 * @param span      指向跨段。
 * @param style     pointer to a style to copy into the span's built-in style
 * @param style     指向要复制到跨段内置样式中的样式
 */
void lv_spangroup_set_span_style(lv_obj_t * obj, lv_span_t * span, const lv_style_t * style);

/**
 * DEPRECATED. Use the text_align style property instead
 * 已弃用,请改用 text_align 样式属性。
 * Set the align of the spangroup.
 * 设置文本组的对齐方式。
 * @param obj   pointer to a spangroup object.
 * @param obj   指向文本组对象。
 * @param align see lv_text_align_t for details.
 * @param align 详见 lv_text_align_t。
 */
LV_DEPRECATED("Use text_align style property instead")
void lv_spangroup_set_align(lv_obj_t * obj, lv_text_align_t align);

/**
 * Set the overflow of the spangroup.
 * 设置文本组的溢出处理方式。
 * @param obj       pointer to a spangroup object.
 * @param obj       指向文本组对象。
 * @param overflow  see lv_span_overflow_t for details.
 * @param overflow  详见 lv_span_overflow_t。
 */
void lv_spangroup_set_overflow(lv_obj_t * obj, lv_span_overflow_t overflow);

/**
 * Set the indent of the spangroup.
 * 设置文本组的缩进。
 * @param obj       pointer to a spangroup object.
 * @param obj       指向文本组对象。
 * @param indent    the first line indentation
 * @param indent    第一行的缩进量
 */
void lv_spangroup_set_indent(lv_obj_t * obj, int32_t indent);

/**
 * DEPRECATED, set the width to LV_SIZE_CONTENT or fixed value to control expanding/wrapping"
 * 已弃用,请通过将宽度设置为 LV_SIZE_CONTENT 或固定值来控制扩展/换行。
 * Set the mode of the spangroup.
 * 设置文本组的模式。
 * @param obj       pointer to a spangroup object.
 * @param obj       指向文本组对象。
 * @param mode      see lv_span_mode_t for details.
 * @param mode      详见 lv_span_mode_t。
 */
LV_DEPRECATED("set the width to LV_SIZE_CONTENT or fixed value to control expanding/wrapping")
void lv_spangroup_set_mode(lv_obj_t * obj, lv_span_mode_t mode);

/**
 * Set maximum lines of the spangroup.
 * 设置文本组的最大行数。
 * @param obj       pointer to a spangroup object.
 * @param obj       指向文本组对象。
 * @param lines     max lines that can be displayed in LV_SPAN_MODE_BREAK mode. < 0 means no limit.
 * @param lines     在 LV_SPAN_MODE_BREAK 模式下可显示的最大行数。< 0 表示不限制。
 */
void lv_spangroup_set_max_lines(lv_obj_t * obj, int32_t lines);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get a pointer to the style of a span's built-in style.
 * 获取跨段内置样式的指针。
 * Any lv_style_set_... functions can be applied on the returned style.
 * 所有 lv_style_set_... 函数都可作用于返回的样式。
 * @param span  pointer to the span
 * @param span  指向跨段
 * @return      pointer to the style. (valid as long as the span is valid)
 * @return      指向样式的指针(只要跨段有效则该指针有效)
 */
lv_style_t * lv_span_get_style(lv_span_t * span);

/**
 * Get a pointer to the text of a span
 * 获取跨段文本的指针
 * @param span  pointer to the span
 * @param span  指向跨段
 * @return      pointer to the text
 * @return      指向文本的指针
*/
const char * lv_span_get_text(lv_span_t * span);

/**
 * Get a spangroup child by its index.
 * 通过索引获取文本组的子跨段。
 *
 * @param obj   The spangroup object
 * @param obj   文本组对象
 * @param id    the index of the child.
 * @param id    子跨段的索引。
 *              0: the oldest (firstly created) child
 *              0:最旧的(最先创建的)子跨段
 *              1: the second oldest
 *              1:第二旧的子跨段
 *              child count-1: the youngest
 *              child count-1:最新的子跨段
 *              -1: the youngest
 *              -1:最新的子跨段
 *              -2: the second youngest
 *              -2:第二新的子跨段
 * @return      The child span at index `id`, or NULL if the ID does not exist
 * @return      索引 `id` 对应的子跨段,如果该索引不存在则返回 NULL
 */
lv_span_t * lv_spangroup_get_child(const lv_obj_t * obj, int32_t id);

/**
 * Get number of spans
 * 获取跨段的数量
 * @param obj   the spangroup object to get the child count of.
 * @param obj   要获取子项数量的文本组对象。
 * @return      the span count of the spangroup.
 * @return      文本组中跨段的数量。
 */
uint32_t lv_spangroup_get_span_count(const lv_obj_t * obj);

/**
 * Get the align of the spangroup.
 * 获取文本组的对齐方式。
 * @param obj   pointer to a spangroup object.
 * @param obj   指向文本组对象。
 * @return      the align value.
 * @return      对齐值。
 */
lv_text_align_t lv_spangroup_get_align(lv_obj_t * obj);

/**
 * Get the overflow of the spangroup.
 * 获取文本组的溢出处理方式。
 * @param obj   pointer to a spangroup object.
 * @param obj   指向文本组对象。
 * @return      the overflow value.
 * @return      溢出处理方式值。
 */
lv_span_overflow_t lv_spangroup_get_overflow(lv_obj_t * obj);

/**
 * Get the indent of the spangroup.
 * 获取文本组的缩进。
 * @param obj   pointer to a spangroup object.
 * @param obj   指向文本组对象。
 * @return      the indent value.
 * @return      缩进值。
 */
int32_t lv_spangroup_get_indent(lv_obj_t * obj);

/**
 * Get the mode of the spangroup.
 * 获取文本组的模式。
 * @param obj   pointer to a spangroup object.
 * @param obj   指向文本组对象。
 */
lv_span_mode_t lv_spangroup_get_mode(lv_obj_t * obj);

/**
 * Get maximum lines of the spangroup.
 * 获取文本组的最大行数。
 * @param obj   pointer to a spangroup object.
 * @param obj   指向文本组对象。
 * @return      the max lines value.
 * @return      最大行数值。
 */
int32_t lv_spangroup_get_max_lines(lv_obj_t * obj);

/**
 * Get max line height of all span in the spangroup.
 * 获取文本组中所有跨段的最大行高。
 * @param obj   pointer to a spangroup object.
 * @param obj   指向文本组对象。
 */
int32_t lv_spangroup_get_max_line_height(lv_obj_t * obj);

/**
 * Get the text content width when all span of spangroup on a line.
 * 获取文本组所有跨段排在一行时的文本内容宽度。
 * @param obj       pointer to a spangroup object.
 * @param obj       指向文本组对象。
 * @param max_width if text content width >= max_width, return max_width
 * @param max_width 如果文本内容宽度 >= max_width,则返回 max_width
 * to reduce computation, if max_width == 0, returns the text content width.
 * 以减少计算量;如果 max_width == 0,则返回文本内容宽度。
 * @return text     content width or max_width.
 * @return text     返回文本内容宽度或 max_width。
 */
uint32_t lv_spangroup_get_expand_width(lv_obj_t * obj, uint32_t max_width);

/**
 * Get the text content height with width fixed.
 * 获取在宽度固定情况下的文本内容高度。
 * @param obj       pointer to a spangroup object.
 * @param obj       指向文本组对象。
 * @param width     the width of the span group.
 * @param width     文本组的宽度。

 */
int32_t lv_spangroup_get_expand_height(lv_obj_t * obj, int32_t width);

/**
 * Get the span's coords in the spangroup.
 * 获取跨段在文本组中的坐标。
 * @note Before calling this function, please make sure that the layout of span group has been updated.
 * @note 在调用此函数之前,请确保文本组的布局已经更新。
 * Like calling lv_obj_update_layout() like function.
 * 例如调用 lv_obj_update_layout() 之类的函数。
 *
 *     +--------+
 *     |Heading +--->------------------+
 *     |  Pos   |   |     Heading      |
 *     +--------+---+------------------+
 *     |                               |
 *     |                               |
 *     |                               |
 *     |            Middle   +--------+|
 *     |                     |Trailing||
 *     |                   +-|  Pos   ||
 *     |                   | +--------+|
 *     +-------------------v-----------+
 *     |     Trailing      |
 *     +-------------------+
 * @param obj       pointer to a spangroup object.
 * @param obj       指向文本组对象。
 * @param span      pointer to a span.
 * @param span      指向跨段。
 * @return the span's coords in the spangroup.
 * @return 返回跨段在文本组中的坐标。
 */
lv_span_coords_t lv_spangroup_get_span_coords(lv_obj_t * obj, const lv_span_t * span);

/**
 * Get the span object by point.
 * 通过坐标点获取跨段对象。
 * @param obj       pointer to a spangroup object.
 * @param obj       指向文本组对象。
 * @param point     pointer to point containing absolute coordinates
 * @param point     包含绝对坐标的点的指针
 * @return          pointer to the span under the point or `NULL` if not found.
 * @return          指向该点所在跨段的指针,如果未找到则返回 `NULL`。
 */
lv_span_t * lv_spangroup_get_span_by_point(lv_obj_t * obj, const lv_point_t * point);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**
 * Update the mode of the spangroup.
 * 更新文本组的模式(重算跨段布局)。
 * @param obj   pointer to a spangroup object.
 * @param obj   指向文本组对象。
 */
void lv_spangroup_refresh(lv_obj_t * obj);

#if LV_USE_OBSERVER

/**
 * Bind an integer, string, or pointer Subject to a Spangroup's Span.
 * 将整型、字符串或指针类型的 Subject 绑定到文本组的跨段上。
 * @param obj       pointer to Spangroup
 * @param obj       指向文本组
 * @param span      pointer to Span
 * @param span      指向跨段
 * @param subject   pointer to Subject
 * @param subject   指向 Subject 的指针
 * @param fmt       optional printf-like format string with 1 format specifier (e.g. "%d °C")
 * @param fmt       可选,包含 1 个格式说明符的 printf 风格格式化字符串(例如 "%d °C")
 *                  or NULL to bind to the value directly.
 *                  或传 NULL 以直接绑定到数值。
 * @return          pointer to newly-created Observer
 * @return          指向新创建的 Observer 的指针
 * @note            If `fmt == NULL` strings and pointers (`\0` terminated string) will be shown
 * @note            如果 `fmt == NULL`,字符串和指针(`\0` 结尾的字符串)会按原样显示
 *                  as text as they are, integers as %d, floats as %0.1f
 *                  整数按 %d 显示,浮点数按 %0.1f 显示
 */
lv_observer_t * lv_spangroup_bind_span_text(lv_obj_t * obj, lv_span_t * span, lv_subject_t * subject, const char * fmt);

#endif

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_SPAN*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_SPAN_H*/
