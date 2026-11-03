/**
 * @file lv_chart.h
 * 图表(chart)控件头文件：提供折线、曲线、柱状、散点等数据图表功能。
 *
 */

#ifndef LV_CHART_H
#define LV_CHART_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"
#include "../core/lv_obj.h"

#if LV_USE_CHART != 0

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/**Default value of points. Can be used to not draw a point*/
/**点的默认值。可用于不绘制某个点*/
#define LV_CHART_POINT_NONE     (INT32_MAX)
LV_EXPORT_CONST_INT(LV_CHART_POINT_NONE);

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

/**
 * Chart types
 * 图表类型
 */
typedef enum {
    LV_CHART_TYPE_NONE,     /**< Don't draw the series*/
                            /**< 不绘制该系列*/
    LV_CHART_TYPE_LINE,     /**< Connect the points with lines*/
                            /**< 用直线连接各数据点*/
    LV_CHART_TYPE_CURVE,    /**< Connect the points with curves*/
                            /**< 用曲线连接各数据点*/
    LV_CHART_TYPE_BAR,      /**< Draw bars for each series*/
                            /**< 为每个系列绘制柱状条*/
    LV_CHART_TYPE_STACKED,  /**< Draw a single stacked bar for each data point. Supports only positive values*/
                            /**< 为每个数据点绘制一个堆叠柱状条。仅支持正值*/
    LV_CHART_TYPE_SCATTER,  /**< Draw points and lines in 2D (x,y coordinates)*/
                            /**< 以二维(x,y 坐标)方式绘制点和线*/
} lv_chart_type_t;

/**
 * Chart update mode for `lv_chart_set_next`
 * `lv_chart_set_next` 使用的图表更新模式
 */
typedef enum {
    LV_CHART_UPDATE_MODE_SHIFT,     /**< Shift old data to the left and add the new one the right*/
                                    /**< 将旧数据向左移动，并在右侧添加新数据*/
    LV_CHART_UPDATE_MODE_CIRCULAR,  /**< Add the new data in a circular way*/
                                    /**< 以循环方式添加新数据*/
} lv_chart_update_mode_t;

/**
 * Enumeration of the axis'
 * 坐标轴的枚举
 */
typedef enum {
    /** 主Y轴 */
    LV_CHART_AXIS_PRIMARY_Y     = 0x00,
    /** 次Y轴 */
    LV_CHART_AXIS_SECONDARY_Y   = 0x01,
    /** 主X轴 */
    LV_CHART_AXIS_PRIMARY_X     = 0x02,
    /** 次X轴 */
    LV_CHART_AXIS_SECONDARY_X   = 0x04,
    /** 最后一个轴标记，用于遍历 */
    LV_CHART_AXIS_LAST
} lv_chart_axis_t;

/** 图表控件的类结构体 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_chart_class;

#if LV_USE_OBJ_PROPERTY
/** 图表对象的属性ID枚举(仅当启用对象属性系统时有效) */
enum _lv_property_chart_id_t {
    LV_PROPERTY_ID(CHART, TYPE,               LV_PROPERTY_TYPE_INT,   0),
    LV_PROPERTY_ID(CHART, POINT_COUNT,        LV_PROPERTY_TYPE_INT,   1),
    LV_PROPERTY_ID(CHART, UPDATE_MODE,        LV_PROPERTY_TYPE_INT,   2),
    LV_PROPERTY_ID(CHART, HOR_DIV_LINE_COUNT, LV_PROPERTY_TYPE_INT,   3),
    LV_PROPERTY_ID(CHART, VER_DIV_LINE_COUNT, LV_PROPERTY_TYPE_INT,   4),
    LV_PROPERTY_CHART_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a chart object
 * 创建一个图表对象
 * @param parent    pointer to an object, it will be the parent of the new chart
 * @param parent    指向一个对象，它将成为新图表的父对象
 * @return          pointer to the created chart
 * @return          指向创建好的图表对象的指针
 */
lv_obj_t * lv_chart_create(lv_obj_t * parent);

/**
 * Set a new type for a chart
 * 为图表设置新的类型
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param type      new type of the chart (from 'lv_chart_type_t' enum)
 * @param type      图表的新类型(取自 'lv_chart_type_t' 枚举)
 */
void lv_chart_set_type(lv_obj_t * obj, lv_chart_type_t type);
/**
 * Set the number of points on a data line on a chart
 * 设置图表中一条数据线上的数据点个数
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param cnt       new number of points on the data lines
 * @param cnt       数据线上新的数据点个数
 */
void lv_chart_set_point_count(lv_obj_t * obj, uint32_t cnt);

/**
 * Set the minimal and maximal y values on an axis
 * 设置坐标轴上的 Y 值最小值和最大值
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param axis      `LV_CHART_AXIS_PRIMARY_Y` or `LV_CHART_AXIS_SECONDARY_Y`
 * @param axis      `LV_CHART_AXIS_PRIMARY_Y` 或 `LV_CHART_AXIS_SECONDARY_Y`
 * @param min       minimum value of the y axis
 * @param min       Y 轴的最小值
 * @param max       maximum value of the y axis
 * @param max       Y 轴的最大值
 */
void lv_chart_set_axis_range(lv_obj_t * obj, lv_chart_axis_t axis, int32_t min, int32_t max);

/**
 * Set the minimal values on an axis
 * 设置坐标轴上的最小值
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param axis      `LV_CHART_AXIS_PRIMARY_Y` or `LV_CHART_AXIS_SECONDARY_Y`
 * @param axis      `LV_CHART_AXIS_PRIMARY_Y` 或 `LV_CHART_AXIS_SECONDARY_Y`
 * @param min       minimal value of the y axis
 * @param min       Y 轴的最小值
 */
void lv_chart_set_axis_min_value(lv_obj_t * obj, lv_chart_axis_t axis, int32_t min);

/**
 * Set the maximal y values on an axis
 * 设置坐标轴上的最大 Y 值
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param axis      `LV_CHART_AXIS_PRIMARY_Y` or `LV_CHART_AXIS_SECONDARY_Y`
 * @param axis      `LV_CHART_AXIS_PRIMARY_Y` 或 `LV_CHART_AXIS_SECONDARY_Y`
 * @param max       maximum value of the y axis
 * @param max       Y 轴的最大值
 */
void lv_chart_set_axis_max_value(lv_obj_t * obj, lv_chart_axis_t axis, int32_t max);


/**
 * Set update mode of the chart object. Affects
 * 设置图表对象的更新模式。影响
 * @param obj              pointer to a chart object
 * @param obj              指向图表对象的指针
 * @param update_mode      the update mode
 * @param update_mode      更新模式
 */
void lv_chart_set_update_mode(lv_obj_t * obj, lv_chart_update_mode_t update_mode);

/**
 * Set the number of horizontal and vertical division lines
 * 设置水平与垂直分隔线的数量
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param hdiv      number of horizontal division lines
 * @param hdiv      水平分隔线的数量
 * @param vdiv      number of vertical division lines
 * @param vdiv      垂直分隔线的数量
 */
void lv_chart_set_div_line_count(lv_obj_t * obj, uint32_t hdiv, uint32_t vdiv);

/**
 * Set the number of horizontal division lines
 * 设置水平分隔线的数量
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param cnt       number of horizontal division lines
 * @param cnt       水平分隔线的数量
 */
void lv_chart_set_hor_div_line_count(lv_obj_t * obj, uint32_t cnt);

/**
 * Set the number of vertical division lines
 * 设置垂直分隔线的数量
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param cnt       number of vertical division lines
 * @param cnt       垂直分隔线的数量
 */
void lv_chart_set_ver_div_line_count(lv_obj_t * obj, uint32_t cnt);

/**
 * Get the type of a chart
 * 获取图表的类型
 * @param obj       pointer to chart object
 * @param obj       指向图表对象的指针
 * @return          type of the chart (from 'lv_chart_t' enum)
 * @return          图表的类型(取自 'lv_chart_t' 枚举)
 */
lv_chart_type_t lv_chart_get_type(const lv_obj_t * obj);

/**
 * Get the data point number per data line on chart
 * 获取图表中每条数据线上的数据点个数
 * @param obj       pointer to chart object
 * @param obj       指向图表对象的指针
 * @return          point number on each data line
 * @return          每条数据线上的数据点个数
 */
uint32_t lv_chart_get_point_count(const lv_obj_t * obj);

/**
 * Get the update mode of a chart
 * 获取图表的更新模式
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @return          the update mode
 * @return          更新模式
 */
lv_chart_update_mode_t lv_chart_get_update_mode(const lv_obj_t * obj);

/**
 * Get the number of horizontal division lines
 * 获取水平分隔线的数量
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @return          the number of horizontal division lines
 * @return          水平分隔线的数量
 */
uint32_t lv_chart_get_hor_div_line_count(const lv_obj_t * obj);

/**
 * Get the number of vertical division lines
 * 获取垂直分隔线的数量
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @return          the number of vertical division lines
 * @return          垂直分隔线的数量
 */
uint32_t lv_chart_get_ver_div_line_count(const lv_obj_t * obj);

/**
 * Get the current index of the x-axis start point in the data array
 * 获取数据数组中当前 X 轴起始点的索引
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param ser       pointer to a data series on 'chart'
 * @param ser       指向 'chart' 上某个数据系列的指针
 * @return          the index of the current x start point in the data array
 * @return          数据数组中当前 X 轴起始点的索引
 */
uint32_t lv_chart_get_x_start_point(const lv_obj_t * obj, lv_chart_series_t * ser);

/**
 * Get the position of a point to the chart.
 * 获取某个数据点在图表中的位置。
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param ser       pointer to series
 * @param ser       指向数据系列的指针
 * @param id        the index.
 * @param id        索引。
 * @param p_out     store the result position here
 * @param p_out     在此处存放计算出的位置结果
 */
void lv_chart_get_point_pos_by_id(lv_obj_t * obj, lv_chart_series_t * ser, uint32_t id, lv_point_t * p_out);

/**
 * Refresh a chart if its data line has changed
 * 当数据线发生变化时刷新图表
 * @param   obj   pointer to chart object
 * @param   obj   指向图表对象的指针
 */
void lv_chart_refresh(lv_obj_t * obj);

/*======================
 * Series
 * 系列
 *=====================*/

/**
 * Allocate and add a data series to the chart
 * 分配并添加一个数据系列到图表中
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param color     color of the data series
 * @param color     数据系列的颜色
 * @param axis      the y axis to which the series should be attached (::LV_CHART_AXIS_PRIMARY_Y or ::LV_CHART_AXIS_SECONDARY_Y)
 * @param axis      该系列应挂载的 Y 轴(::LV_CHART_AXIS_PRIMARY_Y 或 ::LV_CHART_AXIS_SECONDARY_Y)
 * @return          pointer to the allocated data series or NULL on failure
 * @return          指向分配好的数据系列的指针，失败时返回 NULL
 */
lv_chart_series_t * lv_chart_add_series(lv_obj_t * obj, lv_color_t color, lv_chart_axis_t axis);

/**
 * Deallocate and remove a data series from a chart
 * 释放并从图表中移除一个数据系列
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param series    pointer to a data series on 'chart'
 * @param series    指向 'chart' 上某个数据系列的指针
 */
void lv_chart_remove_series(lv_obj_t * obj, lv_chart_series_t * series);

/**
 * Hide/Unhide a single series of a chart.
 * 隐藏/显示图表中的单个系列。
 * @param chart     pointer to a chart object.
 * @param chart     指向图表对象的指针。
 * @param series    pointer to a series object
 * @param series    指向系列对象的指针
 * @param hide      true: hide the series
 * @param hide      true：隐藏该系列
 */
void lv_chart_hide_series(lv_obj_t * chart, lv_chart_series_t * series, bool hide);

/**
 * Change the color of a series
 * 修改系列的颜色
 * @param chart     pointer to a chart object.
 * @param chart     指向图表对象的指针。
 * @param series    pointer to a series object
 * @param series    指向系列对象的指针
 * @param color     the new color of the series
 * @param color     系列的新颜色
 */
void lv_chart_set_series_color(lv_obj_t * chart, lv_chart_series_t * series, lv_color_t color);

/**
 * Get the color of a series
 * 获取系列的颜色
 * @param chart     pointer to a chart object.
 * @param chart     指向图表对象的指针。
 * @param series    pointer to a series object
 * @param series    指向系列对象的指针
 * @return          the color of the series
 * @return          系列的颜色
 */
lv_color_t lv_chart_get_series_color(lv_obj_t * chart, const lv_chart_series_t * series);

/**
 * Set the index of the x-axis start point in the data array.
 * 设置数据数组中 X 轴起始点的索引。
 * This point will be considers the first (left) point and the other points will be drawn after it.
 * 该点将被视为第一个(最左侧)点，其余点将依次在其后绘制。
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param ser       pointer to a data series on 'chart'
 * @param ser       指向 'chart' 上某个数据系列的指针
 * @param id        the index of the x point in the data array
 * @param id        数据数组中 X 点的索引
 */
void lv_chart_set_x_start_point(lv_obj_t * obj, lv_chart_series_t * ser, uint32_t id);

/**
 * Get the next series.
 * 获取下一个系列。
 * @param chart     pointer to a chart
 * @param chart     指向图表的指针
 * @param ser      the previous series or NULL to get the first
 * @param ser      上一个系列，传入 NULL 以获取第一个系列
 * @return          the next series or NULL if there is no more.
 * @return          下一个系列，如果没有更多系列则返回 NULL。
 */
lv_chart_series_t * lv_chart_get_series_next(const lv_obj_t * chart, const lv_chart_series_t * ser);

/*=====================
 * Cursor
 * 游标
 *====================*/

/**
 * Add a cursor with a given color
 * 添加一个具有指定颜色的游标
 * @param obj       pointer to chart object
 * @param obj       指向图表对象的指针
 * @param color     color of the cursor
 * @param color     游标的颜色
 * @param dir       direction of the cursor. `LV_DIR_RIGHT/LEFT/TOP/DOWN/HOR/VER/ALL`. OR-ed values are possible
 * @param dir       游标的方向。`LV_DIR_RIGHT/LEFT/TOP/DOWN/HOR/VER/ALL`。各值可进行按位或组合
 * @return          pointer to the created cursor
 * @return          指向创建好的游标的指针
 */
lv_chart_cursor_t  * lv_chart_add_cursor(lv_obj_t * obj, lv_color_t color, lv_dir_t dir);

/**
 * Remove a cursor
 * 移除一个游标
 * @param obj       pointer to chart object
 * @param obj       指向图表对象的指针
 * @param cursor    pointer to the cursor
 * @param cursor    指向游标的指针
 */
void lv_chart_remove_cursor(lv_obj_t * obj, lv_chart_cursor_t * cursor);

/**
 * Set the coordinate of the cursor with respect to the paddings
 * 设置游标相对于内边距的坐标
 * @param chart     pointer to a chart object
 * @param chart     指向图表对象的指针
 * @param cursor    pointer to the cursor
 * @param cursor    指向游标的指针
 * @param pos       the new coordinate of cursor relative to the chart
 * @param pos       游标相对于图表的新坐标
 */
void lv_chart_set_cursor_pos(lv_obj_t * chart, lv_chart_cursor_t * cursor, lv_point_t * pos);

/**
 * Set the X coordinate of the cursor with respect to the paddings
 * 设置游标相对于内边距的 X 坐标
 * @param chart     pointer to a chart object
 * @param chart     指向图表对象的指针
 * @param cursor    pointer to the cursor
 * @param cursor    指向游标的指针
 * @param x         the new X coordinate of cursor relative to the chart
 * @param x         游标相对于图表的新 X 坐标
 */
void lv_chart_set_cursor_pos_x(lv_obj_t * chart, lv_chart_cursor_t * cursor, int32_t x);

/**
 * Set the coordinate of the cursor with respect to the paddings
 * 设置游标相对于内边距的 Y 坐标
 * @param chart     pointer to a chart object
 * @param chart     指向图表对象的指针
 * @param cursor    pointer to the cursor
 * @param cursor    指向游标的指针
 * @param y         the new Y coordinate of cursor relative to the chart
 * @param y         游标相对于图表的新 Y 坐标
 */
void lv_chart_set_cursor_pos_y(lv_obj_t * chart, lv_chart_cursor_t * cursor, int32_t y);

/**
 * Stick the cursor to a point
 * 将游标吸附到一个数据点上
 * @param chart     pointer to a chart object
 * @param chart     指向图表对象的指针
 * @param cursor    pointer to the cursor
 * @param cursor    指向游标的指针
 * @param ser       pointer to a series
 * @param ser       指向一个系列的指针
 * @param point_id  the point's index or `LV_CHART_POINT_NONE` to not assign to any points.
 * @param point_id  数据点的索引，或使用 `LV_CHART_POINT_NONE` 不吸附到任何点。
 */
void lv_chart_set_cursor_point(lv_obj_t * chart, lv_chart_cursor_t * cursor, lv_chart_series_t * ser,
                               uint32_t point_id);

/**
 * Get the coordinate of the cursor with respect to the paddings
 * 获取游标相对于内边距的坐标
 * @param chart     pointer to a chart object
 * @param chart     指向图表对象的指针
 * @param cursor    pointer to cursor
 * @param cursor    指向游标的指针
 * @return          coordinate of the cursor as lv_point_t
 * @return          游标的坐标(lv_point_t 类型)
 */
lv_point_t lv_chart_get_cursor_point(lv_obj_t * chart, lv_chart_cursor_t * cursor);

/*=====================
 * Set/Get value(s)
 * 设置/获取数值
 *====================*/

/**
 * Initialize all data points of a series with a value
 * 用一个值初始化系列的所有数据点
 * @param obj       pointer to chart object
 * @param obj       指向图表对象的指针
 * @param ser       pointer to a data series on 'chart'
 * @param ser       指向 'chart' 上某个数据系列的指针
 * @param value     the new value for all points. `LV_CHART_POINT_NONE` can be used to hide the points.
 * @param value     所有点的新值。可使用 `LV_CHART_POINT_NONE` 隐藏这些点。
 */
void lv_chart_set_all_values(lv_obj_t * obj, lv_chart_series_t * ser, int32_t value);

/**
 * Set the next point's Y value according to the update mode policy.
 * 根据更新模式策略设置下一个数据点的 Y 值。
 * @param obj       pointer to chart object
 * @param obj       指向图表对象的指针
 * @param ser       pointer to a data series on 'chart'
 * @param ser       指向 'chart' 上某个数据系列的指针
 * @param value     the new value of the next data
 * @param value     下一个数据点的新值
 */
void lv_chart_set_next_value(lv_obj_t * obj, lv_chart_series_t * ser, int32_t value);

/**
 * Set the next point's X and Y value according to the update mode policy.
 * 根据更新模式策略设置下一个数据点的 X 和 Y 值。
 * @param obj       pointer to chart object
 * @param obj       指向图表对象的指针
 * @param ser       pointer to a data series on 'chart'
 * @param ser       指向 'chart' 上某个数据系列的指针
 * @param x_value   the new X value of the next data
 * @param x_value   下一个数据点的新 X 值
 * @param y_value   the new Y value of the next data
 * @param y_value   下一个数据点的新 Y 值
 */
void lv_chart_set_next_value2(lv_obj_t * obj, lv_chart_series_t * ser, int32_t x_value, int32_t y_value);

/**
 * Same as `lv_chart_set_next_value` but set the values from an array
 * 与 `lv_chart_set_next_value` 相同，但从数组中设置数值
 * @param obj           pointer to chart object
 * @param obj           指向图表对象的指针
 * @param ser           pointer to a data series on 'chart'
 * @param ser           指向 'chart' 上某个数据系列的指针
 * @param values        the new values to set
 * @param values        要设置的新数值
 * @param values_cnt    number of items in `values`
 * @param values_cnt    `values` 中元素的数量
 */
void lv_chart_set_series_values(lv_obj_t * obj, lv_chart_series_t * ser, const int32_t values[], size_t values_cnt);

/**
 * Same as `lv_chart_set_next_value2` but set the values from an array
 * 与 `lv_chart_set_next_value2` 相同，但从数组中设置数值
 * @param obj           pointer to chart object
 * @param obj           指向图表对象的指针
 * @param ser           pointer to a data series on 'chart'
 * @param ser           指向 'chart' 上某个数据系列的指针
 * @param x_values      the new values to set on the X axis
 * @param x_values      要在 X 轴上设置的新数值
 * @param y_values      the new values to set o nthe Y axis
 * @param y_values      要在 Y 轴上设置的新数值
 * @param values_cnt    number of items in `x_values` and `y_values`
 * @param values_cnt    `x_values` 和 `y_values` 中元素的数量
 */
void lv_chart_set_series_values2(lv_obj_t * obj, lv_chart_series_t * ser, const int32_t x_values[],
                                 const int32_t y_values[], size_t values_cnt);

/**
 * Set an individual point's y value of a chart's series directly based on its index
 * 直接根据索引设置图表系列中某个数据点的 Y 值
 * @param obj     pointer to a chart object
 * @param obj     指向图表对象的指针
 * @param ser     pointer to a data series on 'chart'
 * @param ser     指向 'chart' 上某个数据系列的指针
 * @param id      the index of the x point in the array
 * @param id      数组中数据点的索引
 * @param value   value to assign to array point
 * @param value   要赋给该数据点的值
 */
void lv_chart_set_series_value_by_id(lv_obj_t * obj, lv_chart_series_t * ser, uint32_t id, int32_t value);

/**
 * Set an individual point's x and y value of a chart's series directly based on its index
 * 直接根据索引设置图表系列中某个数据点的 X 和 Y 值
 * Can be used only with `LV_CHART_TYPE_SCATTER`.
 * 仅可用于 `LV_CHART_TYPE_SCATTER`(散点图)类型。
 * @param obj       pointer to chart object
 * @param obj       指向图表对象的指针
 * @param ser       pointer to a data series on 'chart'
 * @param ser       指向 'chart' 上某个数据系列的指针
 * @param id        the index of the x point in the array
 * @param id        数组中数据点的索引
 * @param x_value   the new X value of the next data
 * @param x_value   该数据点的新 X 值
 * @param y_value   the new Y value of the next data
 * @param y_value   该数据点的新 Y 值
 */
void lv_chart_set_series_value_by_id2(lv_obj_t * obj, lv_chart_series_t * ser, uint32_t id, int32_t x_value,
                                      int32_t y_value);

/**
 * Set an external array for the y data points to use for the chart
 * 为图表设置用于 Y 数据点的外部数组
 * NOTE: It is the users responsibility to make sure the `point_cnt` matches the external array size.
 * 注意：用户有责任确保 `point_cnt`(数据点个数)与外部数组大小一致。
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param ser       pointer to a data series on 'chart'
 * @param ser       指向 'chart' 上某个数据系列的指针
 * @param array     external array of points for chart
 * @param array     图表使用的外部数据点数组
 */
void lv_chart_set_series_ext_y_array(lv_obj_t * obj, lv_chart_series_t * ser, int32_t array[]);

/**
 * Set an external array for the x data points to use for the chart
 * 为图表设置用于 X 数据点的外部数组
 * NOTE: It is the users responsibility to make sure the `point_cnt` matches the external array size.
 * 注意：用户有责任确保 `point_cnt`(数据点个数)与外部数组大小一致。
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @param ser       pointer to a data series on 'chart'
 * @param ser       指向 'chart' 上某个数据系列的指针
 * @param array     external array of points for chart
 * @param array     图表使用的外部数据点数组
 */
void lv_chart_set_series_ext_x_array(lv_obj_t * obj, lv_chart_series_t * ser, int32_t array[]);

/**
 * Get the array of y values of a series
 * 获取系列的 Y 值数组
 * @param obj   pointer to a chart object
 * @param obj   指向图表对象的指针
 * @param ser   pointer to a data series on 'chart'
 * @param ser   指向 'chart' 上某个数据系列的指针
 * @return      the array of values with 'point_count' elements
 * @return      包含 'point_count' 个元素的数值数组
 */
int32_t * lv_chart_get_series_y_array(const lv_obj_t * obj, lv_chart_series_t * ser);

/**
 * Get the array of x values of a series
 * 获取系列的 X 值数组
 * @param obj   pointer to a chart object
 * @param obj   指向图表对象的指针
 * @param ser   pointer to a data series on 'chart'
 * @param ser   指向 'chart' 上某个数据系列的指针
 * @return      the array of values with 'point_count' elements
 * @return      包含 'point_count' 个元素的数值数组
 */
int32_t * lv_chart_get_series_x_array(const lv_obj_t * obj, lv_chart_series_t * ser);

/**
 * Get the index of the currently pressed point. It's the same for every series.
 * 获取当前被按下的数据点的索引。该索引对所有系列都是相同的。
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @return          the index of the point [0 .. point count] or LV_CHART_POINT_ID_NONE if no point is being pressed
 * @return          数据点的索引[0 .. 数据点个数]，若没有点被按下则返回 LV_CHART_POINT_ID_NONE
 */
uint32_t lv_chart_get_pressed_point(const lv_obj_t * obj);

/**
 * Get the overall offset from the chart's side to the center of the first point.
 * 获取从图表边缘到第一个数据点中心的整体偏移量。
 * In case of a bar chart it will be the center of the first column group
 * 对于柱状图，则为第一个柱状组的中心
 * @param obj       pointer to a chart object
 * @param obj       指向图表对象的指针
 * @return          the offset of the center
 * @return          中心点的偏移量
 */
int32_t lv_chart_get_first_point_center_offset(lv_obj_t * obj);

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_CHART*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CHART_H*/
