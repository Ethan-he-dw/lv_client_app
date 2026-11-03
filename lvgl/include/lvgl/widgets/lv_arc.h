/**
 * @file lv_arc.h
 * 圆弧控件:可拖动的弧形指示器(如旋钮、进度环)
 *
 */

#ifndef LV_ARC_H
#define LV_ARC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_ARC != 0

#include "../core/lv_obj.h"
#include "../core/lv_observer.h"

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

/**
 * In which direction the indicator should grow.
 * 指示器增长的方向模式。
 */
typedef enum {
    LV_ARC_MODE_NORMAL,      /**< Clock-wise */
    /**< 顺时针 */
    LV_ARC_MODE_SYMMETRICAL, /**< Left/right from the midpoint */
    /**< 从中点向左/右增长 */
    LV_ARC_MODE_REVERSE      /**< Counterclock-wise */
    /**< 逆时针 */
} lv_arc_mode_t;

/** 圆弧对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_arc_class;

#if LV_USE_OBJ_PROPERTY
/** 属性系统:圆弧对象的属性 ID 枚举 */
enum _lv_property_arc_id_t {
    LV_PROPERTY_ID(ARC, START_ANGLE,        LV_PROPERTY_TYPE_PRECISE,   0),
    LV_PROPERTY_ID(ARC, END_ANGLE,          LV_PROPERTY_TYPE_PRECISE,   1),
    LV_PROPERTY_ID(ARC, BG_START_ANGLE,     LV_PROPERTY_TYPE_PRECISE,   2),
    LV_PROPERTY_ID(ARC, BG_END_ANGLE,       LV_PROPERTY_TYPE_PRECISE,   3),
    LV_PROPERTY_ID(ARC, ROTATION,           LV_PROPERTY_TYPE_INT,       4),
    LV_PROPERTY_ID(ARC, MODE,               LV_PROPERTY_TYPE_INT,       5),
    LV_PROPERTY_ID(ARC, VALUE,              LV_PROPERTY_TYPE_INT,       6),
    LV_PROPERTY_ID(ARC, MIN_VALUE,          LV_PROPERTY_TYPE_INT,       7),
    LV_PROPERTY_ID(ARC, MAX_VALUE,          LV_PROPERTY_TYPE_INT,       8),
    LV_PROPERTY_ID(ARC, CHANGE_RATE,        LV_PROPERTY_TYPE_INT,       9),
    LV_PROPERTY_ID(ARC, KNOB_OFFSET,        LV_PROPERTY_TYPE_INT,       10),
    LV_PROPERTY_ARC_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create an arc object
 * 创建圆弧对象
 * @param parent    pointer to an object, it will be the parent of the new arc
 * @param parent    指向一个对象的指针,该对象将成为新圆弧的父对象
 * @return          pointer to the created arc
 * @return          指向所创建的圆弧对象的指针
 */
lv_obj_t * lv_arc_create(lv_obj_t * parent);

/*======================
 * Add/remove functions
 * 添加/删除函数
 *=====================*/

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the start angle of an arc. 0 deg: right, 90 bottom, etc.
 * 设置圆弧的起始角度。0度:右侧,90度:底部等。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @param start     the start angle. (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param start     起始角度(若启用 `LV_USE_FLOAT`,可为小数)
 */
void lv_arc_set_start_angle(lv_obj_t * obj, lv_value_precise_t start);

/**
 * Set the end angle of an arc. 0 deg: right, 90 bottom, etc.
 * 设置圆弧的结束角度。0度:右侧,90度:底部等。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @param end       the end angle  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param end       结束角度(若启用 `LV_USE_FLOAT`,可为小数)
 */
void lv_arc_set_end_angle(lv_obj_t * obj, lv_value_precise_t end);

/**
 * Set the start and end angles
 * 同时设置圆弧的起始角度和结束角度
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @param start     the start angle  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param start     起始角度(若启用 `LV_USE_FLOAT`,可为小数)
 * @param end       the end angle  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param end       结束角度(若启用 `LV_USE_FLOAT`,可为小数)
 */
void lv_arc_set_angles(lv_obj_t * obj, lv_value_precise_t start, lv_value_precise_t end);

/**
 * Set the start angle of an arc background. 0 deg: right, 90 bottom, etc.
 * 设置圆弧背景的起始角度。0度:右侧,90度:底部等。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @param start     the start angle  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param start     起始角度(若启用 `LV_USE_FLOAT`,可为小数)
 */
void lv_arc_set_bg_start_angle(lv_obj_t * obj, lv_value_precise_t start);

/**
 * Set the start angle of an arc background. 0 deg: right, 90 bottom etc.
 * 设置圆弧背景的结束角度。0度:右侧,90度:底部等。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @param end       the end angle  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param end       结束角度(若启用 `LV_USE_FLOAT`,可为小数)
 */
void lv_arc_set_bg_end_angle(lv_obj_t * obj, lv_value_precise_t end);

/**
 * Set the start and end angles of the arc background
 * 同时设置圆弧背景的起始角度和结束角度
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @param start     the start angle  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param start     起始角度(若启用 `LV_USE_FLOAT`,可为小数)
 * @param end       the end angle  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param end       结束角度(若启用 `LV_USE_FLOAT`,可为小数)
 */
void lv_arc_set_bg_angles(lv_obj_t * obj, lv_value_precise_t start, lv_value_precise_t end);

/**
 * Set the rotation for the whole arc
 * 设置整个圆弧的旋转角度
 * @param obj           pointer to an arc object
 * @param obj           指向圆弧对象的指针
 * @param rotation      rotation angle
 * @param rotation      旋转角度
 */
void lv_arc_set_rotation(lv_obj_t * obj, int32_t rotation);

/**
 * Set in which direction the indicator should grow.
 * 设置指示器增长的方向模式。
 * @param obj       pointer to arc object
 * @param obj       指向圆弧对象的指针
 * @param type      arc's mode
 * @param type      圆弧的模式
 */
void lv_arc_set_mode(lv_obj_t * obj, lv_arc_mode_t type);

/**
 * Set a new value on the arc
 * 设置圆弧的新值
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @param value     new value
 * @param value     新值
 */
void lv_arc_set_value(lv_obj_t * obj, int32_t value);

/**
 * Set minimum and the maximum values of an arc
 * 设置圆弧的最小值和最大值
 * @param obj       pointer to the arc object
 * @param obj       指向圆弧对象的指针
 * @param min       minimum value
 * @param min       最小值
 * @param max       maximum value
 * @param max       最大值
 */
void lv_arc_set_range(lv_obj_t * obj, int32_t min, int32_t max);

/**
 * Set the minimum values of an arc
 * 设置圆弧的最小值
 * @param obj       pointer to the arc object
 * @param obj       指向圆弧对象的指针
 * @param min       minimum value
 * @param min       最小值
 */
void lv_arc_set_min_value(lv_obj_t * obj, int32_t min);

/**
 * Set the maximum values of an arc
 * 设置圆弧的最大值
 * @param obj       pointer to the arc object
 * @param obj       指向圆弧对象的指针
 * @param max       maximum value
 * @param max       最大值
 */
void lv_arc_set_max_value(lv_obj_t * obj, int32_t max);

/**
 * Set a change rate to limit the speed how fast the arc should reach the pressed point.
 * 设置变化速率,用于限制圆弧到达按压点的速度。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @param rate      the change rate
 * @param rate      变化速率
 */
void lv_arc_set_change_rate(lv_obj_t * obj, uint32_t rate);

/**
 * Set an offset angle for the knob
 * 设置旋钮的偏移角度
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @param offset    knob offset from main arc in degrees
 * @param offset    旋钮相对于主圆弧的偏移角度(度)
 */
void lv_arc_set_knob_offset(lv_obj_t * obj, int32_t offset);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the start angle of an arc.
 * 获取圆弧的起始角度。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          the start angle [0..360]  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @return          起始角度 [0..360](若启用 `LV_USE_FLOAT`,可为小数)
 */
lv_value_precise_t lv_arc_get_angle_start(lv_obj_t * obj);

/**
 * Get the end angle of an arc.
 * 获取圆弧的结束角度。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          the end angle [0..360]  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @return          结束角度 [0..360](若启用 `LV_USE_FLOAT`,可为小数)
 */
lv_value_precise_t lv_arc_get_angle_end(lv_obj_t * obj);

/**
 * Get the start angle of an arc background.
 * 获取圆弧背景的起始角度。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          the  start angle [0..360]  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @return          起始角度 [0..360](若启用 `LV_USE_FLOAT`,可为小数)
 */
lv_value_precise_t lv_arc_get_bg_angle_start(lv_obj_t * obj);

/**
 * Get the end angle of an arc background.
 * 获取圆弧背景的结束角度。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          the end angle [0..360]  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @return          结束角度 [0..360](若启用 `LV_USE_FLOAT`,可为小数)
 */
lv_value_precise_t lv_arc_get_bg_angle_end(lv_obj_t * obj);

/**
 * Get the value of an arc
 * 获取圆弧的值
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          the value of the arc
 * @return          圆弧的值
 */
int32_t lv_arc_get_value(const lv_obj_t * obj);

/**
 * Get the minimum value of an arc
 * 获取圆弧的最小值
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          the minimum value of the arc
 * @return          圆弧的最小值
 */
int32_t lv_arc_get_min_value(const lv_obj_t * obj);

/**
 * Get the maximum value of an arc
 * 获取圆弧的最大值
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          the maximum value of the arc
 * @return          圆弧的最大值
 */
int32_t lv_arc_get_max_value(const lv_obj_t * obj);

/**
 * Get whether the arc is type or not.
 * 获取圆弧的模式。
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          arc's mode
 * @return          圆弧的模式
 */
lv_arc_mode_t lv_arc_get_mode(const lv_obj_t * obj);

/**
 * Get the rotation for the whole arc
 * 获取整个圆弧的旋转角度
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          arc's current rotation
 * @return          圆弧当前的旋转角度
 */
int32_t lv_arc_get_rotation(const lv_obj_t * obj);

/**
 * Get the current knob angle offset
 * 获取当前的旋钮角度偏移
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          arc's current knob offset
 * @return          圆弧当前的旋钮偏移角度
 */
int32_t lv_arc_get_knob_offset(const lv_obj_t * obj);

/**
 * Get the change rate of an arc
 * 获取圆弧的变化速率
 * @param obj       pointer to an arc object
 * @param obj       指向圆弧对象的指针
 * @return          the change rate
 * @return          变化速率
 */
uint32_t lv_arc_get_change_rate(lv_obj_t * obj);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

#if LV_USE_OBSERVER
/**
 * Bind an integer subject to an Arc's value.
 * 将整数主题(Subject)绑定到圆弧的值上。
 * @param obj       pointer to Arc
 * @param obj       指向圆弧的指针
 * @param subject   pointer to Subject
 * @param subject   指向主题(Subject)的指针
 * @return          pointer to newly-created Observer
 * @return          指向新创建的观察者(Observer)的指针
 */
lv_observer_t * lv_arc_bind_value(lv_obj_t * obj, lv_subject_t * subject);
#endif


/**
 * Align an object to the current position of the arc (knob)
 * 将对象对齐到圆弧(旋钮)的当前位置
 * @param obj           pointer to an arc object
 * @param obj           指向圆弧对象的指针
 * @param obj_to_align  pointer to an object to align
 * @param obj_to_align  指向要对齐的对象的指针
 * @param r_offset      consider the radius larger with this value (< 0: for smaller radius)
 * @param r_offset      半径偏移量(< 0:使用更小的半径)
 */
void lv_arc_align_obj_to_angle(const lv_obj_t * obj, lv_obj_t * obj_to_align, int32_t r_offset);

/**
 * Rotate an object to the current position of the arc (knob)
 * 将对象旋转到与圆弧(旋钮)当前位置对应的角度
 * @param obj            pointer to an arc object
 * @param obj            指向圆弧对象的指针
 * @param obj_to_rotate  pointer to an object to rotate
 * @param obj_to_rotate  指向要旋转的对象的指针
 * @param r_offset       consider the radius larger with this value (< 0: for smaller radius)
 * @param r_offset       半径偏移量(< 0:使用更小的半径)
 */
void lv_arc_rotate_obj_to_angle(const lv_obj_t * obj, lv_obj_t * obj_to_rotate, int32_t r_offset);

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_ARC*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_ARC_H*/
