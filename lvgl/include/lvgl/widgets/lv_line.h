/**
 * @file lv_line.h
 * 线条控件头文件：用于绘制连接一系列点的直线
 *
 */

#ifndef LV_LINE_H
#define LV_LINE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../core/lv_obj.h"
#if LV_USE_LINE != 0

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/** 线条控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_line_class;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_line_id_t {
    LV_PROPERTY_ID(LINE, Y_INVERT, LV_PROPERTY_TYPE_BOOL, 0),
    LV_PROPERTY_LINE_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a line object
 * 创建一个线条对象
 * @param parent pointer to an object, it will be the parent of the new line
 * @param parent 指向一个对象，它将成为新线条的父对象
 * @return pointer to the created line
 * @return 指向创建好的线条对象的指针
 */
lv_obj_t * lv_line_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set an array of points. The line object will connect these points.
 * 设置点数组，线条对象将依次连接这些点
 * @param obj           pointer to a line object
 * @param obj           指向线条对象
 * @param points        an array of points. Only the address is saved, so the array needs to be alive while the line exists
 * @param points        点数组。仅保存其地址，因此线条存在期间该数组必须保持有效
 * @param point_num     number of points in 'point_a'
 * @param point_num     点数组中点的数量
 */
void lv_line_set_points(lv_obj_t * obj, const lv_point_precise_t points[], uint32_t point_num);

/**
 * Set a non-const array of points. Identical to `lv_line_set_points` except the array may be retrieved by `lv_line_get_points_mutable`.
 * 设置一个非常量点数组。与 `lv_line_set_points` 相同，区别在于该数组可通过 `lv_line_get_points_mutable` 获取
 * @param obj           pointer to a line object
 * @param obj           指向线条对象
 * @param points        a non-const array of points. Only the address is saved, so the array needs to be alive while the line exists.
 * @param points        一个非常量点数组。仅保存其地址，因此线条存在期间该数组必须保持有效
 * @param point_num     number of points in 'point_a'
 * @param point_num     点数组中点的数量
 */
void lv_line_set_points_mutable(lv_obj_t * obj, lv_point_precise_t points[], uint32_t point_num);

/**
 * Enable (or disable) the y coordinate inversion.
 * 启用(或禁用)y 坐标反转
 * If enabled then y will be subtracted from the height of the object,
 * 若启用，则 y 将从对象高度中减去，
 * therefore the y = 0 coordinate will be on the bottom.
 * 因此 y = 0 的坐标将位于对象底部。
 * @param obj       pointer to a line object
 * @param obj       指向线条对象
 * @param en        true: enable the y inversion, false:disable the y inversion
 * @param en        true: 启用 y 反转，false: 禁用 y 反转
 */
void lv_line_set_y_invert(lv_obj_t * obj, bool en);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the pointer to the array of points.
 * 获取点数组的指针
 * @param obj           pointer to a line object
 * @param obj           指向线条对象
 * @return              const pointer to the array of points
 * @return              指向点数组的常量指针
 */
const lv_point_precise_t * lv_line_get_points(lv_obj_t * obj);

/**
 * Get the number of points in the array of points.
 * 获取点数组中点的数量
 * @param obj           pointer to a line object
 * @param obj           指向线条对象
 * @return              number of points in array of points
 * @return              点数组中点的数量
 */
uint32_t lv_line_get_point_count(lv_obj_t * obj);

/**
 * Check the mutability of the stored point array pointer.
 * 检查存储的点数组指针是否可修改
 * @param obj           pointer to a line object
 * @param obj           指向线条对象
 * @return              true: the point array pointer is mutable, false: constant
 * @return              true: 点数组指针可修改，false: 为常量
 */
bool lv_line_is_point_array_mutable(lv_obj_t * obj);

/**
 * Get a pointer to the mutable array of points or NULL if it is not mutable
 * 获取可修改的点数组指针，若不可修改则返回 NULL
 * @param obj           pointer to a line object
 * @param obj           指向线条对象
 * @return              pointer to the array of points. NULL if not mutable.
 * @return              指向点数组的指针，若不可修改则为 NULL
 */
lv_point_precise_t * lv_line_get_points_mutable(lv_obj_t * obj);

/**
 * Get the y inversion attribute
 * 获取 y 反转属性
 * @param obj       pointer to a line object
 * @param obj       指向线条对象
 * @return          true: y inversion is enabled, false: disabled
 * @return          true: 已启用 y 反转，false: 已禁用
 */
bool lv_line_get_y_invert(const lv_obj_t * obj);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_LINE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LINE_H*/
