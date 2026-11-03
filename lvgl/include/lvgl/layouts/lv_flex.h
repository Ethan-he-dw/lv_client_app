/**
 * @file lv_flex.h
 *
 * Flex 弹性布局模块
 *
 * Flex 布局是一种 CSS Flexbox 风格的一维布局系统，用于在一维方向（行或列）上
 * 排列子对象（item），并支持自动换行、反向排列、对齐方式和空间分配等特性。
 *
 * 使用方法：
 *   1. 对父对象调用 lv_obj_set_flex_flow() 设置排列方向（行/列/换行等）。
 *   2. 调用 lv_obj_set_flex_align() 设置主轴对齐、交叉轴对齐和行对齐。
 *   3. 可对子对象调用 lv_obj_set_flex_grow() 设置弹性增长比例。
 *
 * 示例：
 *   lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);           // 水平排列，允许换行
 *   lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START,             // 主轴靠左
 *                              LV_FLEX_ALIGN_CENTER,             // 交叉轴居中
 *                              LV_FLEX_ALIGN_SPACE_EVENLY);      // 行间距均匀分布
 *
 * 注意：父对象必须启用 Flex 布局后，子对象的 flex grow 属性才有效。
 */

#ifndef LV_FLEX_H
#define LV_FLEX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../config/lv_conf_internal.h"
#include "../core/lv_area.h"

#if LV_USE_FLEX

/*********************
 *      DEFINES
 *********************/

/** Flex 排列方向标志位（用于组合构成 lv_flex_flow_t 枚举值） */
#define LV_FLEX_COLUMN        (1 << 0)  /**< 设置为列方向（垂直排列），默认为行方向（水平排列） */
#define LV_FLEX_WRAP       (1 << 2)  /**< 启用换行：当一行/列空间不足时，子对象换到下一行/列 */
#define LV_FLEX_REVERSE    (1 << 3)  /**< 启用反向排列：子对象从右到左（行）或从下到上（列）排列 */

/**********************
 *      TYPEDEFS
 **********************/

/*Can't include lv_obj.h because it includes this header file*/

/**
 * Flex 对齐方式枚举
 *
 * 用于指定子对象在主轴、交叉轴以及多行/列场景下的对齐行为。
 * 类似于 CSS Flexbox 的 justify-content 和 align-items 属性。
 */
typedef enum {
    LV_FLEX_ALIGN_START,          /**< 起始对齐：主轴靠起点排列，交叉轴靠起点对齐 */
    LV_FLEX_ALIGN_END,            /**< 末尾对齐：主轴靠终点排列，交叉轴靠终点对齐 */
    LV_FLEX_ALIGN_CENTER,         /**< 居中对齐：子对象沿轴线居中排列 */
    LV_FLEX_ALIGN_SPACE_EVENLY,   /**< 均匀分布：每个间隔大小相同（包括首尾间隔） */
    LV_FLEX_ALIGN_SPACE_AROUND,   /**< 环绕分布：每个间隔大小相同，首尾间隔为中间间隔的一半 */
    LV_FLEX_ALIGN_SPACE_BETWEEN,  /**< 两端对齐：首尾紧贴边界，中间均匀分布 */
} lv_flex_align_t;

/**
 * Flex 排列方向枚举
 *
 * 通过组合基本标志位（行/列、换行、反向）来定义子对象的排列方式。
 * 对应 CSS Flexbox 的 flex-direction 和 flex-wrap 属性。
 *
 * 位域结构：
 *   - bit 0: 列方向标志 (LV_FLEX_COLUMN)
 *   - bit 2: 换行标志 (LV_FLEX_WRAP)
 *   - bit 3: 反向标志 (LV_FLEX_REVERSE)
 */
typedef enum {
    LV_FLEX_FLOW_ROW                 = 0x00,                              /**< 水平排列（默认），从左到右 */
    LV_FLEX_FLOW_COLUMN              = LV_FLEX_COLUMN,                    /**< 垂直排列，从上到下 */
    LV_FLEX_FLOW_ROW_WRAP            = LV_FLEX_FLOW_ROW | LV_FLEX_WRAP,               /**< 水平排列 + 自动换行 */
    LV_FLEX_FLOW_ROW_REVERSE         = LV_FLEX_FLOW_ROW | LV_FLEX_REVERSE,            /**< 水平排列（反向），从右到左 */
    LV_FLEX_FLOW_ROW_WRAP_REVERSE    = LV_FLEX_FLOW_ROW | LV_FLEX_WRAP | LV_FLEX_REVERSE,    /**< 水平排列 + 换行 + 反向 */
    LV_FLEX_FLOW_COLUMN_WRAP         = LV_FLEX_FLOW_COLUMN | LV_FLEX_WRAP,            /**< 垂直排列 + 自动换行 */
    LV_FLEX_FLOW_COLUMN_REVERSE      = LV_FLEX_FLOW_COLUMN | LV_FLEX_REVERSE,         /**< 垂直排列（反向），从下到上 */
    LV_FLEX_FLOW_COLUMN_WRAP_REVERSE = LV_FLEX_FLOW_COLUMN | LV_FLEX_WRAP | LV_FLEX_REVERSE,  /**< 垂直排列 + 换行 + 反向 */
} lv_flex_flow_t;

/**********************
 * GLOBAL VARIABLES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * 初始化 Flex 布局模块
 *
 * 在 LVGL 内部初始化流程中调用，注册 Flex 布局的计算回调函数。
 * 用户一般不需要手动调用此函数。
 */
void lv_flex_init(void);

/**
 * 设置 Flex 容器的排列方向（flow）
 *
 * 设置子对象的流动方向（行/列），以及是否换行和是否反向排列。
 * 调用此函数后，对象自动成为 Flex 容器，其子对象将按指定方式排列。
 *
 * @param obj   指向 Flex 容器对象的指针（即父对象）。如果父对象未使用 Flex 布局，则此函数无效果。
 * @param flow  排列方向，取值为 lv_flex_flow_t 枚举中的任意一项。
 *
 * @code
 * // 示例：水平排列并允许换行
 * lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);
 *
 * // 示例：垂直反向排列
 * lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN_REVERSE);
 * @endcode
 */
void lv_obj_set_flex_flow(lv_obj_t * obj, lv_flex_flow_t flow);

/**
 * 设置 Flex 容器的对齐方式
 *
 * 控制子对象在容器内的对齐行为，包括三个维度：
 *   - main_place   （主轴对齐）：控制子对象在行/列方向上的对齐方式
 *   - cross_place  （交叉轴对齐）：控制子对象在垂直于行/列方向上的对齐方式
 *   - track_cross_place（多行对齐）：当有多个行/列（track）时，控制各 track 在交叉轴方向的分布方式
 *
 * @param obj               指向 Flex 容器对象的指针。如果父对象未使用 Flex 布局，则此函数无效果。
 * @param main_place        主轴对齐方式，取值为 lv_flex_align_t 枚举中的任意一项。
 * @param cross_place       交叉轴对齐方式，通常使用 LV_FLEX_ALIGN_START / END / CENTER。
 * @param track_cross_place 多行/列场景下的行对齐方式，取值为 lv_flex_align_t 枚举中的任意一项。
 *
 * @code
 * // 示例：主轴居中，交叉轴居中，多行均匀分布
 * lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER,
 *                             LV_FLEX_ALIGN_CENTER,
 *                             LV_FLEX_ALIGN_SPACE_EVENLY);
 * @endcode
 */
void lv_obj_set_flex_align(lv_obj_t * obj, lv_flex_align_t main_place, lv_flex_align_t cross_place,
                           lv_flex_align_t track_cross_place);

/**
 * 设置子对象的弹性增长比例（flex grow）
 *
 * 指定子对象在主轴方向上按比例分配容器的剩余空间。
 * 类似于 CSS 的 flex-grow 属性。所有 grow 值相同的子对象平均分配剩余空间，
 * grow 值为 0 的子对象不分配剩余空间（保持其原始尺寸）。
 *
 * 例如：三个子对象的 grow 值分别为 1、2、1，则它们分别获得剩余空间的 1/4、2/4、1/4。
 *
 * @param obj  指向 Flex 子对象的指针。该对象的父对象必须使用了 Flex 布局，否则此函数无效果。
 * @param grow 弹性增长比例值。0 表示不增长；大于 0 则按比例分配剩余空间。
 *
 * @code
 * // 示例：设置子对象的 flex grow 为 1（按比例填充剩余空间）
 * lv_obj_set_flex_grow(child, 1);
 *
 * // 示例：一个子对象 grow=1，另一个 grow=2，后者获得两倍于前者的空间
 * lv_obj_set_flex_grow(child1, 1);
 * lv_obj_set_flex_grow(child2, 2);
 * @endcode
 */
void lv_obj_set_flex_grow(lv_obj_t * obj, uint8_t grow);

/**********************
 *      MACROS
 **********************/

#endif  /*LV_USE_FLEX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_FLEX_H*/