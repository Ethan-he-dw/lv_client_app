/**
 * @file lv_grid.h
 *
 * Grid 网格布局模块
 *
 * Grid 布局是一种 CSS Grid 风格的二维布局系统，用于在行和列两个方向上
 * 同时排列子对象（item），支持跨行跨列、多种对齐方式和弹性尺寸分配。
 *
 * 使用方法：
 *   1. 对父对象调用 lv_obj_set_grid_dsc_array() 设置列和行的描述数组。
 *      - 描述数组中每个元素定义一个 track（行/列）的尺寸：
 *        * 固定像素值：如 100 表示 100px
 *        * LV_GRID_FR(x)：弹性比例，按比例分配剩余空间
 *        * LV_GRID_CONTENT：根据子对象内容自动确定尺寸
 *        * LV_GRID_TEMPLATE_LAST：数组结束标记
 *   2. 调用 lv_obj_set_grid_align() 设置网格整体的对齐方式。
 *   3. 对子对象调用 lv_obj_set_grid_cell() 指定其在网格中的位置和跨距。
 *
 * 示例：
 *   // 定义 3 列 2 行的网格
 *   static int32_t col_dsc[] = {100, LV_GRID_FR(1), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
 *   static int32_t row_dsc[] = {60, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
 *   lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);
 *
 *   // 将子对象放在第 1 列第 0 行，跨 2 列
 *   lv_obj_set_grid_cell(child, LV_GRID_ALIGN_CENTER, 1, 2,
 *                                LV_GRID_ALIGN_CENTER, 0, 1);
 */

#ifndef LV_GRID_H
#define LV_GRID_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../config/lv_conf_internal.h"
#include "../core/lv_area.h"

#if LV_USE_GRID

/*********************
 *      DEFINES
 *********************/
/**
 * 弹性比例尺寸宏（类似 CSS 的 fr 单位）
 *
 * 用于在网格描述数组中定义 track 的弹性比例尺寸。
 * 所有使用 LV_GRID_FR() 的 track 会按比例分配容器的剩余空间。
 *
 * @param x 比例系数，相对于其他 FR track 的比例份额
 * @return 特殊的 track 尺寸值
 *
 * 例如：LV_GRID_FR(1) 和 LV_GRID_FR(2) 的 track 分别获得 1/3 和 2/3 的剩余空间。
 */
#define LV_GRID_FR(x)          (LV_COORD_MAX - 100 + x)

/**
 * 内容自适应尺寸
 *
 * 用于在网格描述数组中定义 track 的尺寸由子对象内容决定。
 * 该 track 的尺寸等于其包含的最大子对象的尺寸。
 */
#define LV_GRID_CONTENT        (LV_COORD_MAX - 101)
LV_EXPORT_CONST_INT(LV_GRID_CONTENT);

/**
 * 网格描述数组结束标记
 *
 * 必须作为网格描述数组（行/列）的最后一个元素，
 * 用于标识数组的结束。
 */
#define LV_GRID_TEMPLATE_LAST  (LV_COORD_MAX)
LV_EXPORT_CONST_INT(LV_GRID_TEMPLATE_LAST);

/**********************
 *      TYPEDEFS
 **********************/

/*Can't include lv_obj.h because it includes this header file*/

/**
 * Grid 对齐方式枚举
 *
 * 用于控制子对象在其所在网格单元格内的对齐方式，
 * 以及网格整体在容器中的对齐方式。
 *
 * 类似于 CSS Grid 的 justify-items / align-items 属性。
 */
typedef enum {
    LV_GRID_ALIGN_START,          /**< 起始对齐：子对象靠单元格起始边对齐 */
    LV_GRID_ALIGN_CENTER,         /**< 居中对齐：子对象在单元格内居中 */
    LV_GRID_ALIGN_END,            /**< 末尾对齐：子对象靠单元格末尾边对齐 */
    LV_GRID_ALIGN_STRETCH,        /**< 拉伸填充：子对象尺寸扩展以填满整个单元格 */
    LV_GRID_ALIGN_SPACE_EVENLY,   /**< 均匀分布：每个间隔大小相同（包括首尾间隔），
                                       仅用于网格整体对齐，不适用于单元格内对齐 */
    LV_GRID_ALIGN_SPACE_AROUND,   /**< 环绕分布：每个间隔大小相同，首尾间隔为中间间隔的一半，
                                       仅用于网格整体对齐，不适用于单元格内对齐 */
    LV_GRID_ALIGN_SPACE_BETWEEN,  /**< 两端对齐：首尾紧贴边界，中间均匀分布，
                                       仅用于网格整体对齐，不适用于单元格内对齐 */
} lv_grid_align_t;

/**********************
 * GLOBAL VARIABLES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * 初始化 Grid 布局模块
 *
 * 在 LVGL 内部初始化流程中调用，注册 Grid 布局的计算回调函数。
 * 用户一般不需要手动调用此函数。
 */
void lv_grid_init(void);

/**
 * 设置网格容器的列和行描述数组
 *
 * 调用此函数后，对象自动成为网格容器，其子对象将按网格方式排列。
 * 列和行描述数组定义了每个 track（列/行）的尺寸。
 *
 * @param obj    指向网格容器对象的指针（即父对象）。
 * @param col_dsc 列描述数组，以 LV_GRID_TEMPLATE_LAST 结尾。
 *                每个元素可以是：固定像素值、LV_GRID_FR(x) 或 LV_GRID_CONTENT。
 * @param row_dsc 行描述数组，以 LV_GRID_TEMPLATE_LAST 结尾。
 *                格式同 col_dsc。
 *
 * @code
 * // 示例：定义 3 列（100px, 1fr, 2fr）和 2 行（50px, 自适应）
 * static int32_t col_dsc[] = {100, LV_GRID_FR(1), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
 * static int32_t row_dsc[] = {50, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
 * lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);
 * @endcode
 */
void lv_obj_set_grid_dsc_array(lv_obj_t * obj, const int32_t col_dsc[], const int32_t row_dsc[]);

/**
 * 设置网格的整体对齐方式
 *
 * 控制所有列/行 track 在容器内容区域中的对齐和分布方式。
 * 仅当 track 总尺寸小于容器内容区域时，对齐效果才明显。
 *
 * @param obj           指向网格容器对象的指针。
 * @param column_align  列方向的对齐方式，取值为 lv_grid_align_t 枚举。
 * @param row_align     行方向的对齐方式，取值为 lv_grid_align_t 枚举。
 *
 * @code
 * // 示例：列居中对齐，行起始对齐
 * lv_obj_set_grid_align(cont, LV_GRID_ALIGN_CENTER, LV_GRID_ALIGN_START);
 * @endcode
 */
void lv_obj_set_grid_align(lv_obj_t * obj, lv_grid_align_t column_align, lv_grid_align_t row_align);

/**
 * 设置子对象在网格中的单元格位置和跨距
 *
 * 指定子对象在父容器网格中的位置（行列坐标）、跨距（占据多少行列）
 * 以及在单元格内的对齐方式。
 * 父对象必须已通过 lv_obj_set_grid_dsc_array() 设置为网格容器。
 *
 * @param obj          指向子对象的指针。该对象的父对象必须使用了网格布局。
 * @param column_align 子对象在单元格内的列方向（水平）对齐方式。
 *                     取值为 LV_GRID_ALIGN_START / END / CENTER / STRETCH。
 * @param col_pos      列位置索引（从 0 开始）。
 * @param col_span     列跨距，表示子对象占据的列数（>= 1，默认为 1）。
 * @param row_align    子对象在单元格内的行方向（垂直）对齐方式。
 *                     取值为 LV_GRID_ALIGN_START / END / CENTER / STRETCH。
 * @param row_pos      行位置索引（从 0 开始）。
 * @param row_span     行跨距，表示子对象占据的行数（>= 1，默认为 1）。
 *
 * @code
 * // 示例：将子对象放在第 1 列第 0 行，跨 2 列 1 行，居中对齐
 * lv_obj_set_grid_cell(child, LV_GRID_ALIGN_CENTER, 1, 2,
 *                              LV_GRID_ALIGN_CENTER, 0, 1);
 *
 * // 示例：将子对象放在第 0 列第 1 行，跨 1 列 1 行，拉伸填充
 * lv_obj_set_grid_cell(child, LV_GRID_ALIGN_STRETCH, 0, 1,
 *                              LV_GRID_ALIGN_STRETCH, 1, 1);
 * @endcode
 */
void lv_obj_set_grid_cell(lv_obj_t * obj, lv_grid_align_t column_align, int32_t col_pos, int32_t col_span,
                          lv_grid_align_t row_align, int32_t row_pos, int32_t row_span);

/**
 * LV_GRID_FR() 宏的函数包装（用于语言绑定）
 *
 * 提供与 LV_GRID_FR(x) 宏相同功能的函数接口，
 * 便于在不支持宏的语言绑定中使用。
 *
 * @param x 比例系数
 * @return 对应的 FR 尺寸值
 */
int32_t lv_grid_fr(uint8_t x);

/**********************
 * GLOBAL VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GRID*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GRID_H*/