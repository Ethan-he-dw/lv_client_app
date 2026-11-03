/**
 * @file lv_obj_style.h
 *
 * 对象样式（Style）管理模块。
 * 该模块提供了向对象（Widget）添加、移除、替换样式的接口，
 * 以及查询样式属性、设置本地样式、处理样式刷新等功能的声明。
 */

#ifndef LV_OBJ_STYLE_H
#define LV_OBJ_STYLE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES（头文件包含）
 *********************/
#include "../font/lv_bidi.h"   /* 双向文本（Bidi）支持 */
#include "lv_style.h"          /* 样式核心定义 */
#include "../lv_types.h"       /* LVGL 通用类型定义 */

/*********************
 *      DEFINES（宏定义）
 *********************/

/**********************
 *      TYPEDEFS（类型定义）
 **********************/

/**
 * 控件的可能状态（State）。
 * 不同的状态可通过按位或（OR）组合使用。
 */
typedef enum {
    LV_STATE_DEFAULT     = 0,         /**< 默认状态 */
    LV_STATE_ALT         = 1 << 0,    /**< 备用（Alternate）状态 */
    /*1 reserved（位1保留）*/
    LV_STATE_CHECKED     = 1 << 2,    /**< 已勾选（Checked）状态，例如复选框被选中 */
    LV_STATE_FOCUSED     = 1 << 3,    /**< 已聚焦（Focused）状态，例如可通过键盘/方向键导航到的控件 */
    LV_STATE_FOCUS_KEY   = 1 << 4,    /**< 通过键盘聚焦的状态（仅通过键盘交互时触发） */
    LV_STATE_EDITED      = 1 << 5,    /**< 编辑（Edited）状态，例如文本区域正在被编辑 */
    LV_STATE_HOVERED     = 1 << 6,    /**< 悬停（Hovered）状态，鼠标悬停在控件上时触发 */
    LV_STATE_PRESSED     = 1 << 7,    /**< 按下（Pressed）状态，控件被按压时触发 */
    LV_STATE_SCROLLED    = 1 << 8,    /**< 滚动（Scrolled）状态，控件正在被滚动时触发 */
    LV_STATE_DISABLED    = 1 << 9,    /**< 禁用（Disabled）状态，控件被禁用后不可交互 */
    /*2 reserved（位2保留）*/
    LV_STATE_USER_1      = 1 << 12,   /**< 用户自定义状态 1 */
    LV_STATE_USER_2      = 1 << 13,   /**< 用户自定义状态 2 */
    LV_STATE_USER_3      = 1 << 14,   /**< 用户自定义状态 3 */
    LV_STATE_USER_4      = 1 << 15,   /**< 用户自定义状态 4 */

    LV_STATE_ANY         = 0xFFFF,    /**< 特殊值：在某些函数中用于表示“所有状态” */
} lv_state_t;

/**
 * 控件的可能部件（Part）。
 * 部件可视为控件内部的构建模块。
 * 例如：滑块 = 背景 + 指示器 + 旋钮。
 * 并非所有部件都会被每个控件使用。
 */
typedef enum {
    LV_PART_MAIN         = 0x000000,  /**< 主体（如背景矩形） */
    LV_PART_SCROLLBAR    = 0x010000,  /**< 滚动条 */
    LV_PART_INDICATOR    = 0x020000,  /**< 指示器，例如滑块、进度条、开关的指示部分，或复选框的勾选框 */
    LV_PART_KNOB         = 0x030000,  /**< 手柄，用于拖动调节数值（如滑块上的旋钮） */
    LV_PART_SELECTED     = 0x040000,  /**< 表示当前选中的选项或区域 */
    LV_PART_ITEMS        = 0x050000,  /**< 用于控件包含多个相似元素时（例如表格的单元格） */
    LV_PART_CURSOR       = 0x060000,  /**< 标记特定位置，例如文本区的光标或图表上的游标 */

    LV_PART_CUSTOM_FIRST = 0x080000,  /**< 自定义部件的扩展起点（供自定义控件使用） */

    LV_PART_ANY          = 0x0F0000,  /**< 特殊值，可用于某些函数中代表“所有部件” */
} lv_part_t;

/**
 * 两个状态之间样式比较的结果类型。
 */
typedef enum {
    LV_STYLE_STATE_CMP_SAME,           /**< 两个状态中的样式属性完全相同 */
    LV_STYLE_STATE_CMP_DIFF_REDRAW,    /**< 差异可通过简单的重绘来表现 */
    LV_STYLE_STATE_CMP_DIFF_DRAW_PAD,  /**< 差异可通过简单的重绘来表现（涉及绘制内边距） */
    LV_STYLE_STATE_CMP_DIFF_LAYOUT,    /**< 差异会影响到布局，需要重新布局 */
} lv_style_state_cmp_t;

/**
 * `lv_part_t` 和 `lv_state_t` 的联合类型。示例取值：
 * - `0`                              ：表示 `LV_PART_MAIN | LV_STATE_DEFAULT`
 * - `LV_STATE_PRESSED`               ：表示在按下状态下
 * - `LV_PART_KNOB`                   ：表示旋钮部件
 * - `LV_PART_KNOB | LV_STATE_PRESSED | LV_STATE_CHECKED`：表示按下且勾选状态下的旋钮部件
 *
 * 该类型用于为样式指定“作用于哪个部件、在哪个状态下生效”。
 */
typedef uint32_t lv_style_selector_t;

/**********************
 * GLOBAL PROTOTYPES（全局函数声明）
 **********************/

/**
 * 向对象添加一个样式。
 * @param obj       指向对象（Object）的指针
 * @param style     指向要添加的样式的指针
 * @param selector  部件和状态的按位或（OR-ed）组合值，用于指定该样式在哪些部件/状态下生效
 *
 * 示例：
 * @code
 * lv_obj_add_style(btn, &style_btn, 0); // 按钮默认样式
 *
 * lv_obj_add_style(btn, &btn_red, LV_STATE_PRESSED); // 按下时仅将部分颜色覆盖为红色
 * @endcode
 */
void lv_obj_add_style(lv_obj_t * obj, const lv_style_t * style, lv_style_selector_t selector);

/**
 * 替换对象的某个样式，同时保留样式栈中的顺序（本地样式和过渡动画会被忽略）。
 * @param obj           指向对象的指针
 * @param old_style     指向要被替换的样式的指针
 * @param new_style     指向用于替换旧样式的新样式的指针
 * @param selector      状态和部件的按位或（OR-ed）组合值，仅替换具有匹配选择器的样式。
 *                      可使用 LV_STATE_ANY 和 LV_PART_ANY
 *
 * 示例：
 * @code
 * lv_obj_replace_style(obj, &yellow_style, &blue_style, LV_PART_ANY | LV_STATE_ANY); // 替换某个特定样式
 *
 * lv_obj_replace_style(obj, &yellow_style, &blue_style, LV_PART_MAIN | LV_STATE_PRESSED); // 替换在按下状态下分配给主部件的特定样式
 * @endcode
 */
bool lv_obj_replace_style(lv_obj_t * obj, const lv_style_t * old_style, const lv_style_t * new_style,
                          lv_style_selector_t selector);

/**
 * 从对象中移除一个样式。
 * @param obj       指向对象的指针
 * @param style     指向要移除的样式的指针。可为 NULL，此时仅根据 selector 进行匹配
 * @param selector  状态和部件的按位或（OR-ed）组合值，仅移除具有匹配选择器的样式。
 *                  可使用 LV_STATE_ANY 和 LV_PART_ANY
 *
 * 示例：
 * @code
 * lv_obj_remove_style(obj, &style, LV_PART_ANY | LV_STATE_ANY); // 移除某个特定样式
 *
 * lv_obj_remove_style(obj, NULL, LV_PART_MAIN | LV_STATE_ANY); // 移除主部件上的所有样式
 *
 * lv_obj_remove_style(obj, NULL, LV_PART_ANY | LV_STATE_ANY); // 移除所有样式
 * @endcode
 */
void lv_obj_remove_style(lv_obj_t * obj, const lv_style_t * style, lv_style_selector_t selector);


/**
 * 移除由主题（Theme）添加到控件上的所有样式。
 * @param obj       指向控件的指针
 * @param selector  状态和部件的按位或（OR-ed）组合值，仅移除具有匹配选择器的样式。
 *                  可使用 LV_STATE_ANY 和 LV_PART_ANY
 */
void lv_obj_remove_theme(lv_obj_t * obj, lv_style_selector_t selector);

/**
 * 移除对象上的所有样式。
 * @param obj       指向对象的指针
 */
void lv_obj_remove_style_all(lv_obj_t * obj);

/**
 * 当某个样式被修改时，通知所有相关对象。
 * @param style     指向被修改的样式的指针。只有使用该样式的对象会被通知
 *                  （传 NULL 则通知所有对象）
 */
void lv_obj_report_style_change(lv_style_t * style);

/**
 * 通知对象及其子对象：“该对象的样式已被修改”。
 * @param obj       指向对象的指针
 * @param part      样式被修改的部件。例如 `LV_PART_ANY`、`LV_PART_MAIN`
 * @param prop      `LV_STYLE_PROP_ANY` 或某个 `LV_STYLE_...` 属性。
 *                  用于优化需要刷新的内容。
 *                  传 `LV_STYLE_PROP_INV` 则仅执行样式缓存更新
 */
void lv_obj_refresh_style(lv_obj_t * obj, lv_part_t part, lv_style_prop_t prop);

/**
 * 临时禁用一个选择器对应的样式，效果等同于该样式从未被添加。
 * @param obj       指向对象的指针
 * @param style     指向样式的指针
 * @param selector  样式的选择器（例如 LV_STATE_PRESSED | LV_PART_KNOB）
 * @param dis       true: 禁用该样式；false: 启用该样式
 */
void lv_obj_style_set_disabled(lv_obj_t * obj, const lv_style_t * style, lv_style_selector_t selector, bool dis);

/**
 * 查询某个样式在对象上是否被禁用。
 * @param obj       指向对象的指针
 * @param style     指向样式的指针
 * @param selector  样式的选择器（例如 LV_STATE_PRESSED | LV_PART_KNOB）
 * @return          true: 该样式已被禁用；false: 该样式未被禁用（处于启用状态）
 */
bool lv_obj_style_get_disabled(lv_obj_t * obj, const lv_style_t * style, lv_style_selector_t selector);

/**
 * 启用或禁用自动样式刷新功能。
 * 当向对象添加/移除新样式，或发生其他任何样式变化时，是否自动刷新。
 * @param en        true: 启用刷新；false: 禁用刷新
 */
void lv_obj_enable_style_refresh(bool en);

/**
 * 获取样式属性的值。会考虑对象当前的状态。
 * 可继承的属性会进行继承。
 * 如果属性未被设置，则返回默认值。
 * @param obj       指向对象的指针
 * @param part      要获取属性的部件
 * @param prop      要获取的属性
 * @return          属性的值。
 *                  应根据属性的类型，从 `lv_style_value_t` 中相应的字段读取
 */
lv_style_value_t lv_obj_get_style_prop(const lv_obj_t * obj, lv_part_t part, lv_style_prop_t prop);

/**
 * 检查对象在给定的样式选择器下，是否设置了指定的样式属性。
 * @param obj       指向对象的指针
 * @param selector  要检查的样式选择器，用于限定所检查样式的生效范围
 * @param prop      要检查的属性
 * @return          true: 对象在该选择器下设置了此属性；false: 未设置
 */
bool lv_obj_has_style_prop(const lv_obj_t * obj, lv_style_selector_t selector, lv_style_prop_t prop);

/**
 * 在对象的某个部件和状态下，设置本地样式属性（Local Style Property）。
 * @param obj       指向对象的指针
 * @param prop      属性
 * @param value     属性的值。应根据属性的类型，在联合体中设置正确的成员
 * @param selector  部件和状态的按位或（OR-ed）组合值，用于指定样式在哪些部件/状态下生效
 */
void lv_obj_set_local_style_prop(lv_obj_t * obj, lv_style_prop_t prop, lv_style_value_t value,
                                 lv_style_selector_t selector);

/**
 * 获取对象在某个部件和状态下的本地样式属性（Local Style Property）。
 * @param obj       指向对象的指针
 * @param prop      属性
 * @param value     用于存储属性值的输出参数
 * @param selector  部件和状态的按位或（OR-ed）组合值，用于指定样式的生效范围
 * @return          查询结果状态（`lv_style_res_t`）
 */
lv_style_res_t lv_obj_get_local_style_prop(lv_obj_t * obj, lv_style_prop_t prop, lv_style_value_t * value,
                                           lv_style_selector_t selector);

/**
 * 从对象某个部件、某个状态上移除一个本地样式属性。
 * @param obj       指向对象的指针
 * @param prop      要移除的样式属性
 * @param selector  部件和状态的按位或（OR-ed）组合值，用于指定样式的生效范围
 * @return true     该属性存在并被成功移除；false: 未找到该属性
 */
bool lv_obj_remove_local_style_prop(lv_obj_t * obj, lv_style_prop_t prop, lv_style_selector_t selector);

/**
 * 内部使用：用于颜色过滤（Color Filter）处理。
 */
lv_style_value_t lv_obj_style_apply_color_filter(const lv_obj_t * obj, lv_part_t part, lv_style_value_t v);

/**
 * 淡入（Fade in）一个对象及其所有子对象。
 * @param obj       要淡入的对象
 * @param time      淡入动画持续时间
 * @param delay     动画开始前的延迟时间
 */
void lv_obj_fade_in(lv_obj_t * obj, uint32_t time, uint32_t delay);

/**
 * 淡出（Fade out）一个对象及其所有子对象。
 * @param obj       要淡出的对象
 * @param time      淡出动画持续时间
 * @param delay     动画开始前的延迟时间
 */
void lv_obj_fade_out(lv_obj_t * obj, uint32_t time, uint32_t delay);

/**
 * 从样式选择器中提取状态部分。
 * @param selector  样式选择器（包含部件和状态的组合值）
 * @return          选择器中的状态（低 16 位）
 */
static inline lv_state_t lv_obj_style_get_selector_state(lv_style_selector_t selector)
{
    return (lv_state_t)(selector & 0xFFFF);
}

/**
 * 从样式选择器中提取部件部分。
 * @param selector  样式选择器（包含部件和状态的组合值）
 * @return          选择器中的部件（高字节部分，位于 0xFF0000 掩码内）
 */
static inline lv_part_t lv_obj_style_get_selector_part(lv_style_selector_t selector)
{
    return (lv_part_t)(selector & 0xFF0000);
}

#include "lv_obj_style_gen.h"   /* 包含由生成器生成的样式设置/获取函数 */

/**
 * 一次性设置对象四边（左、右、上、下）的内边距（Padding）为相同的值。
 * @param obj       指向对象的指针
 * @param value     四边统一的内边距值
 * @param selector  部件和状态的按位或（OR-ed）组合值
 */
static inline void lv_obj_set_style_pad_all(lv_obj_t * obj, int32_t value, lv_style_selector_t selector)
{
    lv_obj_set_style_pad_left(obj, value, selector);
    lv_obj_set_style_pad_right(obj, value, selector);
    lv_obj_set_style_pad_top(obj, value, selector);
    lv_obj_set_style_pad_bottom(obj, value, selector);
}

/**
 * 一次性设置对象水平方向（左、右）的内边距为相同的值。
 * @param obj       指向对象的指针
 * @param value     水平方向统一的内边距值
 * @param selector  部件和状态的按位或（OR-ed）组合值
 */
static inline void lv_obj_set_style_pad_hor(lv_obj_t * obj, int32_t value, lv_style_selector_t selector)
{
    lv_obj_set_style_pad_left(obj, value, selector);
    lv_obj_set_style_pad_right(obj, value, selector);
}

/**
 * 一次性设置对象垂直方向（上、下）的内边距为相同的值。
 * @param obj       指向对象的指针
 * @param value     垂直方向统一的内边距值
 * @param selector  部件和状态的按位或（OR-ed）组合值
 */
static inline void lv_obj_set_style_pad_ver(lv_obj_t * obj, int32_t value, lv_style_selector_t selector)
{
    lv_obj_set_style_pad_top(obj, value, selector);
    lv_obj_set_style_pad_bottom(obj, value, selector);
}

/**
 * 一次性设置对象四边的外边距（Margin）为相同的值。
 * @param obj       指向对象的指针
 * @param value     四边统一的外边距值
 * @param selector  部件和状态的按位或（OR-ed）组合值
 */
static inline void lv_obj_set_style_margin_all(lv_obj_t * obj, int32_t value, lv_style_selector_t selector)
{
    lv_obj_set_style_margin_left(obj, value, selector);
    lv_obj_set_style_margin_right(obj, value, selector);
    lv_obj_set_style_margin_top(obj, value, selector);
    lv_obj_set_style_margin_bottom(obj, value, selector);
}

/**
 * 一次性设置对象水平方向（左、右）的外边距为相同的值。
 * @param obj       指向对象的指针
 * @param value     水平方向统一的外边距值
 * @param selector  部件和状态的按位或（OR-ed）组合值
 */
static inline void lv_obj_set_style_margin_hor(lv_obj_t * obj, int32_t value, lv_style_selector_t selector)
{
    lv_obj_set_style_margin_left(obj, value, selector);
    lv_obj_set_style_margin_right(obj, value, selector);
}

/**
 * 一次性设置对象垂直方向（上、下）的外边距为相同的值。
 * @param obj       指向对象的指针
 * @param value     垂直方向统一的外边距值
 * @param selector  部件和状态的按位或（OR-ed）组合值
 */
static inline void lv_obj_set_style_margin_ver(lv_obj_t * obj, int32_t value, lv_style_selector_t selector)
{
    lv_obj_set_style_margin_top(obj, value, selector);
    lv_obj_set_style_margin_bottom(obj, value, selector);
}

/**
 * 一次性设置对象的行间距和列间距（Gap）为相同的值。
 * @param obj       指向对象的指针
 * @param value     统一的行/列间距值
 * @param selector  部件和状态的按位或（OR-ed）组合值
 */
static inline void lv_obj_set_style_pad_gap(lv_obj_t * obj, int32_t value, lv_style_selector_t selector)
{
    lv_obj_set_style_pad_row(obj, value, selector);
    lv_obj_set_style_pad_column(obj, value, selector);
}

/**
 * 一次性设置对象的宽度和高度。
 * @param obj       指向对象的指针
 * @param width     宽度值
 * @param height    高度值
 * @param selector  部件和状态的按位或（OR-ed）组合值
 */
static inline void lv_obj_set_style_size(lv_obj_t * obj, int32_t width, int32_t height,
                                         lv_style_selector_t selector)
{
    lv_obj_set_style_width(obj, width, selector);
    lv_obj_set_style_height(obj, height, selector);
}

/**
 * 一次性设置对象在 X 和 Y 方向上的变换缩放比例（Transform Scale）为相同的值。
 * @param obj       指向对象的指针
 * @param value     统一的比例值，例如 256 表示 100%（原始大小）
 * @param selector  部件和状态的按位或（OR-ed）组合值
 */
static inline void lv_obj_set_style_transform_scale(lv_obj_t * obj, int32_t value,
                                                    lv_style_selector_t selector)
{
    lv_obj_set_style_transform_scale_x(obj, value, selector);
    lv_obj_set_style_transform_scale_y(obj, value, selector);
}

/**
 * 获取对象左侧的实际可用空间（内边距 + 左边框宽度，如果左边框存在的话）。
 * @param obj       指向对象的指针
 * @param part      要获取空间的部件
 * @return          左侧可用空间值（像素）
 */
static inline int32_t lv_obj_get_style_space_left(const lv_obj_t * obj, lv_part_t part)
{
    int32_t padding = lv_obj_get_style_pad_left(obj, part);
    int32_t border_width = lv_obj_get_style_border_width(obj, part);
    lv_border_side_t border_side = lv_obj_get_style_border_side(obj, part);
    return (border_side & LV_BORDER_SIDE_LEFT) ? padding + border_width : padding;
}

/**
 * 获取对象右侧的实际可用空间（内边距 + 右边框宽度，如果右边框存在的话）。
 * @param obj       指向对象的指针
 * @param part      要获取空间的部件
 * @return          右侧可用空间值（像素）
 */
static inline int32_t lv_obj_get_style_space_right(const lv_obj_t * obj, lv_part_t part)
{
    int32_t padding = lv_obj_get_style_pad_right(obj, part);
    int32_t border_width = lv_obj_get_style_border_width(obj, part);
    lv_border_side_t border_side = lv_obj_get_style_border_side(obj, part);
    return (border_side & LV_BORDER_SIDE_RIGHT) ? padding + border_width : padding;
}

/**
 * 获取对象上侧的实际可用空间（内边距 + 上边框宽度，如果上边框存在的话）。
 * @param obj       指向对象的指针
 * @param part      要获取空间的部件
 * @return          上侧可用空间值（像素）
 */
static inline int32_t lv_obj_get_style_space_top(const lv_obj_t * obj, lv_part_t part)
{
    int32_t padding = lv_obj_get_style_pad_top(obj, part);
    int32_t border_width = lv_obj_get_style_border_width(obj, part);
    lv_border_side_t border_side = lv_obj_get_style_border_side(obj, part);
    return (border_side & LV_BORDER_SIDE_TOP) ? padding + border_width : padding;
}

/**
 * 获取对象下侧的实际可用空间（内边距 + 下边框宽度，如果下边框存在的话）。
 * @param obj       指向对象的指针
 * @param part      要获取空间的部件
 * @return          下侧可用空间值（像素）
 */
static inline int32_t lv_obj_get_style_space_bottom(const lv_obj_t * obj, lv_part_t part)
{
    int32_t padding = lv_obj_get_style_pad_bottom(obj, part);
    int32_t border_width = lv_obj_get_style_border_width(obj, part);
    lv_border_side_t border_side = lv_obj_get_style_border_side(obj, part);
    return (border_side & LV_BORDER_SIDE_BOTTOM) ? padding + border_width : padding;
}

/**
 * 计算对象在给定部件上的实际文本对齐方式（考虑文字的书写方向等）。
 * @param obj       指向对象的指针
 * @param part      要计算的部件
 * @param txt       相关的文本内容（可能影响对齐方式，例如双向文本）
 * @return          计算后的文本对齐方式（`lv_text_align_t`）
 */
lv_text_align_t lv_obj_calculate_style_text_align(const lv_obj_t * obj, lv_part_t part, const char * txt);

/**
 * 安全地获取 X 方向变换缩放比例。
 * 如果获取到的比例值 <= 0，则返回 1（表示 100% 原始大小），避免出现无效值。
 * @param obj       指向对象的指针
 * @param part      要获取的部件
 * @return          有效的 X 方向缩放比例（> 0）
 */
static inline int32_t lv_obj_get_style_transform_scale_x_safe(const lv_obj_t * obj, lv_part_t part)
{
    int32_t scale = lv_obj_get_style_transform_scale_x(obj, part);
    return scale > 0 ? scale : 1;
}

/**
 * 安全地获取 Y 方向变换缩放比例。
 * 如果获取到的比例值 <= 0，则返回 1（表示 100% 原始大小），避免出现无效值。
 * @param obj       指向对象的指针
 * @param part      要获取的部件
 * @return          有效的 Y 方向缩放比例（> 0）
 */
static inline int32_t lv_obj_get_style_transform_scale_y_safe(const lv_obj_t * obj, lv_part_t part)
{
    int32_t scale = lv_obj_get_style_transform_scale_y(obj, part);
    return scale > 0 ? scale : 1;
}

/**
 * 从所有父对象中获取 `opa`（不透明度）样式属性，并将它们相乘后右移 8 位。
 * @param obj       要获取不透明度的对象
 * @param part      要获取不透明度的部件。非 MAIN 部件也会同时考虑 MAIN 部件的不透明度
 * @return          最终的不透明度值（已考虑父对象的不透明度）
 */
lv_opa_t lv_obj_get_style_opa_recursive(const lv_obj_t * obj, lv_part_t part);


/**
 * 根据对象的样式属性，对输入颜色应用重新着色（Recolor）效果。
 * @param obj       包含重新着色样式属性的目标对象
 * @param part      用于获取重新着色样式的部件
 * @param color     要修改的原始颜色
 * @return          应用重新着色和不透明度后的混合颜色
 */
lv_color32_t lv_obj_style_apply_recolor(const lv_obj_t * obj, lv_part_t part, lv_color32_t color);

/**
 * 从所有父对象中获取 `recolor`（重新着色）样式属性，并对它们进行递归混合。
 * @param obj       要获取重新着色值的对象
 * @param part      要检查的目标部件。非 MAIN 部件在计算时也会考虑
 *                  MAIN 部件的 `recolor` 值
 * @return          综合所有父对象重新着色值后得到的最终混合重新着色值
 */
lv_color32_t lv_obj_get_style_recolor_recursive(const lv_obj_t * obj, lv_part_t part);

#if LV_USE_OBSERVER
/**
 * 当主题（Subject）的值不等于参考值时，禁用某个样式。
 * @param obj        指向控件的指针（Widget）
 * @param style      指向样式的指针
 * @param selector   指向选择器的值（部件与状态的组合）
 * @param subject    指向主题（Subject）的指针
 * @param ref_value  用于与主题值比较的参考值
 * @return           指向新创建的观察者（Observer）的指针
 */
lv_observer_t * lv_obj_bind_style(lv_obj_t * obj, const lv_style_t * style, lv_style_selector_t selector,
                                  lv_subject_t * subject, int32_t ref_value);

/**
 * 将主题（Subject）的值绑定到控件的某个样式属性上。
 * @param obj       指向控件的指针（Widget）
 * @param prop      样式属性
 * @param selector  用于指定该属性生效范围的选择器，例如 `LV_PART_KNOB | LV_STATE_PRESSED`
 * @param subject   指向主题（Subject）的指针，属性的值将绑定到该主题的值上
 * @return          指向新创建的观察者（Observer）的指针
 */
lv_observer_t * lv_obj_bind_style_prop(lv_obj_t * obj, lv_style_prop_t prop, lv_style_selector_t selector,
                                       lv_subject_t * subject);

#endif

/**********************
 *      MACROS（宏定义）
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_STYLE_H*/
