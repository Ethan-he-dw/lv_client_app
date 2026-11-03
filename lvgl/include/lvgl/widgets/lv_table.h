/**
 * @file lv_table.h
 * @brief 表格(table)控件
 */

#ifndef LV_TABLE_H
#define LV_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/

#include "lv_label.h"
#include "../core/lv_obj_property.h"

#if LV_USE_TABLE != 0

/*Testing of dependencies*/
/*检查依赖项*/
#if LV_USE_LABEL == 0
#error "lv_table: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1)"
#endif

/*********************
 *      DEFINES
 * 宏定义
 *********************/
/*未选中的单元格(无单元格)标记值*/
#define LV_TABLE_CELL_NONE 0XFFFF
LV_EXPORT_CONST_INT(LV_TABLE_CELL_NONE);

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

/*单元格控制位(control bits)枚举*/
typedef enum {
    LV_TABLE_CELL_CTRL_NONE        = 0 << 0,   /*无任何控制位*/
    LV_TABLE_CELL_CTRL_MERGE_RIGHT = 1 << 0,   /*与右侧单元格合并*/
    LV_TABLE_CELL_CTRL_TEXT_CROP   = 1 << 1,   /*裁剪文本(不换行)*/
    LV_TABLE_CELL_CTRL_CUSTOM_1    = 1 << 4,   /*用户自定义控制位1*/
    LV_TABLE_CELL_CTRL_CUSTOM_2    = 1 << 5,   /*用户自定义控制位2*/
    LV_TABLE_CELL_CTRL_CUSTOM_3    = 1 << 6,   /*用户自定义控制位3*/
    LV_TABLE_CELL_CTRL_CUSTOM_4    = 1 << 7,   /*用户自定义控制位4*/
} lv_table_cell_ctrl_t;

/*表格控件类(lv_obj_class_t)*/
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_table_class;

#if LV_USE_OBJ_PROPERTY
/*表格对象属性ID枚举*/
enum _lv_property_table_id_t {
    LV_PROPERTY_ID(TABLE, ROW_COUNT,    LV_PROPERTY_TYPE_INT, 0),   /*行数*/
    LV_PROPERTY_ID(TABLE, COLUMN_COUNT, LV_PROPERTY_TYPE_INT, 1),   /*列数*/
    LV_PROPERTY_TABLE_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a table object
 * 创建一个表格对象
 * @param parent        pointer to an object, it will be the parent of the new table
 *                     指向父对象，新表格的父对象
 * @return              pointer to the created table
 *                      指向创建出的表格对象的指针
 */
lv_obj_t * lv_table_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the value of a cell.
 * 设置单元格的值。
 * @param obj           pointer to a Table object
 *                     指向表格对象的指针
 * @param row           id of the row [0 .. row_cnt -1]
 *                      行号 [0 .. 行数-1]
 * @param col           id of the column [0 .. col_cnt -1]
 *                      列号 [0 .. 列数-1]
 * @param txt           text to display in the cell. It will be copied and saved so this variable is not required after this function call.
 *                      要在单元格中显示的文本。该文本会被拷贝保存，因此在函数调用后该变量不再需要。
 * @note                New roes/columns are added automatically if required
 * @note                如需要，会自动添加新的行/列
 */
void lv_table_set_cell_value(lv_obj_t * obj, uint32_t row, uint32_t col, const char * txt);

/**
 * Set the value of a cell.  Memory will be allocated to store the text by the table.
 * 设置单元格的值。表格将分配内存来保存该文本。
 * @param obj           pointer to a Table object
 *                     指向表格对象的指针
 * @param row           id of the row [0 .. row_cnt -1]
 *                      行号 [0 .. 行数-1]
 * @param col           id of the column [0 .. col_cnt -1]
 *                      列号 [0 .. 列数-1]
 * @param fmt           `printf`-like format
 *                      `printf` 风格的格式字符串
 * @note                New roes/columns are added automatically if required
 * @note                如需要，会自动添加新的行/列
 */
void lv_table_set_cell_value_fmt(lv_obj_t * obj, uint32_t row, uint32_t col, const char * fmt,
                                 ...) LV_FORMAT_ATTRIBUTE(4, 5);

/**
 * Set the number of rows
 * 设置行数
 * @param obj           table pointer to a Table object
 *                      指向表格对象的指针
 * @param row_cnt       number of rows
 *                      行数
 */
void lv_table_set_row_count(lv_obj_t * obj, uint32_t row_cnt);

/**
 * Set the number of columns
 * 设置列数
 * @param obj       table pointer to a Table object
 *                 指向表格对象的指针
 * @param col_cnt   number of columns.
 *                 列数
 */
void lv_table_set_column_count(lv_obj_t * obj, uint32_t col_cnt);

/**
 * Set the width of a column
 * 设置某一列的宽度
 * @param obj       table pointer to a Table object
 *                 指向表格对象的指针
 * @param col_id    id of the column [0 .. LV_TABLE_COL_MAX -1]
 *                 列号 [0 .. LV_TABLE_COL_MAX -1]
 * @param w         width of the column
 *                 列的宽度
 */
void lv_table_set_column_width(lv_obj_t * obj, uint32_t col_id, int32_t w);

/**
 * Add control bits to the cell.
 * 为单元格添加控制位。
 * @param obj       pointer to a Table object
 *                 指向表格对象的指针
 * @param row       id of the row [0 .. row_cnt -1]
 *                 行号 [0 .. 行数-1]
 * @param col       id of the column [0 .. col_cnt -1]
 *                 列号 [0 .. 列数-1]
 * @param ctrl      OR-ed values from ::lv_table_cell_ctrl_t
 *                 来自 ::lv_table_cell_ctrl_t 的按位或组合值
 */
void lv_table_set_cell_ctrl(lv_obj_t * obj, uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl);

/**
 * Clear control bits of the cell.
 * 清除单元格的控制位。
 * @param obj       pointer to a Table object
 *                 指向表格对象的指针
 * @param row       id of the row [0 .. row_cnt -1]
 *                 行号 [0 .. 行数-1]
 * @param col       id of the column [0 .. col_cnt -1]
 *                 列号 [0 .. 列数-1]
 * @param ctrl      OR-ed values from ::lv_table_cell_ctrl_t
 *                 来自 ::lv_table_cell_ctrl_t 的按位或组合值
 */
void lv_table_clear_cell_ctrl(lv_obj_t * obj, uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl);

/**
 * Add custom user data to the cell.
 * 为单元格添加自定义用户数据。
 * @param obj       pointer to a Table object
 *                 指向表格对象的指针
 * @param row       id of the row [0 .. row_cnt -1]
 *                 行号 [0 .. 行数-1]
 * @param col       id of the column [0 .. col_cnt -1]
 *                 列号 [0 .. 列数-1]
 * @param user_data pointer to the new user_data.
 *                 指向新的用户数据的指针。
 *                  Should be allocated by `lv_malloc`,
 *                  应通过 `lv_malloc` 分配，
 *                  and it will be freed automatically when the table is deleted or
 *                  when the cell is dropped due to lower row or column count.
 *                  当表格被删除，或当单元格因行/列数减少而被移除时，该内存会被自动释放。
 */
void lv_table_set_cell_user_data(lv_obj_t * obj, uint16_t row, uint16_t col, void * user_data);

/**
 * Set the selected cell
 * 设置选中的单元格
 * @param obj       pointer to a table object
 *                 指向表格对象的指针
 * @param row       id of the cell row to select
 *                 要选中的单元格行号
 * @param col       id of the cell column to select
 *                 要选中的单元格列号
 */
void lv_table_set_selected_cell(lv_obj_t * obj, uint16_t row, uint16_t col);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the value of a cell.
 * 获取单元格的值。
 * @param obj       pointer to a Table object
 *                 指向表格对象的指针
 * @param row       id of the row [0 .. row_cnt -1]
 *                 行号 [0 .. 行数-1]
 * @param col       id of the column [0 .. col_cnt -1]
 *                 列号 [0 .. 列数-1]
 * @return          text in the cell
 *                 单元格中的文本
 */
const char * lv_table_get_cell_value(lv_obj_t * obj, uint32_t row, uint32_t col);

/**
 * Get the number of rows.
 * 获取行数。
 * @param obj       table pointer to a Table object
 *                 指向表格对象的指针
 * @return          number of rows.
 *                 行数
 */
uint32_t lv_table_get_row_count(lv_obj_t * obj);

/**
 * Get the number of columns.
 * 获取列数。
 * @param obj       table pointer to a Table object
 *                 指向表格对象的指针
 * @return          number of columns.
 *                 列数
 */
uint32_t lv_table_get_column_count(lv_obj_t * obj);

/**
 * Get the width of a column
 * 获取某一列的宽度
 * @param obj       table pointer to a Table object
 *                 指向表格对象的指针
 * @param col       id of the column [0 .. LV_TABLE_COL_MAX -1]
 *                 列号 [0 .. LV_TABLE_COL_MAX -1]
 * @return          width of the column
 *                 列的宽度
 */
int32_t lv_table_get_column_width(lv_obj_t * obj, uint32_t col);

/**
 * Get whether a cell has the control bits
 * 获取单元格是否具有指定的控制位
 * @param obj       pointer to a Table object
 *                 指向表格对象的指针
 * @param row       id of the row [0 .. row_cnt -1]
 *                 行号 [0 .. 行数-1]
 * @param col       id of the column [0 .. col_cnt -1]
 *                 列号 [0 .. 列数-1]
 * @param ctrl      OR-ed values from ::lv_table_cell_ctrl_t
 *                 来自 ::lv_table_cell_ctrl_t 的按位或组合值
 * @return          true: all control bits are set; false: not all control bits are set
 *                 true: 所有控制位均已设置；false: 并非所有控制位都已设置
 */
bool lv_table_has_cell_ctrl(lv_obj_t * obj, uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl);

/**
 * Get the selected cell (pressed and or focused)
 * 获取当前选中的单元格(按下或聚焦)
 * @param obj       pointer to a table object
 *                 指向表格对象的指针
 * @param row       pointer to variable to store the selected row (LV_TABLE_CELL_NONE: if no cell selected)
 *                 用于保存选中行号的变量指针 (LV_TABLE_CELL_NONE: 表示没有单元格被选中)
 * @param col       pointer to variable to store the selected column  (LV_TABLE_CELL_NONE: if no cell selected)
 *                 用于保存选中列号的变量指针 (LV_TABLE_CELL_NONE: 表示没有单元格被选中)
 */
void lv_table_get_selected_cell(lv_obj_t * obj, uint32_t * row, uint32_t * col);

/**
 * Get custom user data to the cell.
 * 获取单元格的自定义用户数据。
 * @param obj       pointer to a Table object
 *                 指向表格对象的指针
 * @param row       id of the row [0 .. row_cnt -1]
 *                 行号 [0 .. 行数-1]
 * @param col       id of the column [0 .. col_cnt -1]
 *                 列号 [0 .. 列数-1]
 */
void * lv_table_get_cell_user_data(lv_obj_t * obj, uint16_t row, uint16_t col);

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_TABLE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TABLE_H*/
