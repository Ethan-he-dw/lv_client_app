/**
 * @file lv_arclabel.h
 * 圆弧标签控件:文本沿圆弧曲线排列显示
 *
 */

#ifndef LV_ARCLABEL_H
#define LV_ARCLABEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_ARCLABEL != 0

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/** 圆弧标签省略号(点)的数量 */
#define LV_ARCLABEL_DOT_NUM 3
/** 圆弧标签的默认文本(根据配置选择) */
#if LV_WIDGETS_HAS_DEFAULT_VALUE
#define LV_ARCLABEL_DEFAULT_TEXT "Arced Text"
#else
#define LV_ARCLABEL_DEFAULT_TEXT ""
#endif

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

/** 圆弧标签文字方向 */
typedef enum {
    /** 顺时针方向 */
    LV_ARCLABEL_DIR_CLOCKWISE = 0,
    /** 逆时针方向 */
    LV_ARCLABEL_DIR_COUNTER_CLOCKWISE = 1
} lv_arclabel_dir_t;

/** 圆弧标签文字对齐方式 */
typedef enum {
    /** 默认对齐 */
    LV_ARCLABEL_TEXT_ALIGN_DEFAULT = 0,
    /** 前缘对齐 */
    LV_ARCLABEL_TEXT_ALIGN_LEADING = 1,
    /** 居中对齐 */
    LV_ARCLABEL_TEXT_ALIGN_CENTER = 2,
    /** 尾缘对齐 */
    LV_ARCLABEL_TEXT_ALIGN_TRAILING = 3,
} lv_arclabel_text_align_t;

/** 圆弧标签文本溢出处理方式 */
typedef enum {
    LV_ARCLABEL_OVERFLOW_VISIBLE,  /**< Show full text, may overflow object area */
    /**< 显示完整文本,可能超出对象区域 */
    LV_ARCLABEL_OVERFLOW_ELLIPSIS, /**< Show ellipsis (...) when text overflows */
    /**< 文本溢出时显示省略号(...) */
    LV_ARCLABEL_OVERFLOW_CLIP      /**< Clip text at arc boundary */
    /**< 在圆弧边界处裁剪文本 */
} lv_arclabel_overflow_t;

/** 圆弧标签对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_arclabel_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create an arc label object
 * 创建圆弧标签对象
 * @param parent    pointer to an object, it will be the parent of the new arc label
 * @param parent    指向一个对象的指针,该对象将成为新圆弧标签的父对象
 * @return          pointer to the created arc label
 * @return          指向所创建的圆弧标签对象的指针
 */
lv_obj_t * lv_arclabel_create(lv_obj_t * parent);

/*======================
 * Add/remove functions
 * 添加/删除函数
 *=====================*/

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the text of the arc label.
 * 设置圆弧标签的文本。
 *
 * This function sets the text displayed by an arc label object.
 * 该函数设置圆弧标签对象所显示的文本。
 *
 * @param obj       Pointer to the arc label object.
 * @param obj       指向圆弧标签对象的指针。
 * @param text      Pointer to a null-terminated string containing the new text for the label.
 * @param text      指向以 '\0' 结尾的字符串的指针,包含标签的新文本。
 */
void lv_arclabel_set_text(lv_obj_t * obj, const char * text);

/**
 * Set the formatted text of an arc label object.
 * 设置圆弧标签对象的格式化文本。
 *
 * This function sets the text of an arc label object with support for
 * variable arguments formatting, similar to `printf`.
 * 该函数设置圆弧标签对象的文本,支持类似 `printf` 的可变参数格式化。
 *
 * @param obj       The arc label object to set the text for.
 * @param obj       要设置文本的圆弧标签对象。
 * @param fmt       A format string that specifies how subsequent arguments are converted to text.
 * @param fmt       格式字符串,指定后续参数如何转换为文本。
 * @param ...       Arguments following the format string that are used to replace format specifiers in the format string.
 * @param ...       格式字符串之后的参数,用于替换格式字符串中的格式说明符。
 */
void lv_arclabel_set_text_fmt(lv_obj_t * obj, const char * fmt, ...) LV_FORMAT_ATTRIBUTE(2, 3);

/**
 * Sets a new static text for the arc label or refreshes it with the current text.
 * 为圆弧标签设置新的静态文本,或用当前文本刷新标签。
 * The 'text' must remain valid in memory; the arc label does not manage its lifecycle.
 * 'text' 必须始终在内存中有效;圆弧标签不负责管理其生命周期。
 *
 * @param obj       Pointer to the arc label object.
 * @param obj       指向圆弧标签对象的指针。
 * @param text      Pointer to the new text. If NULL, the label is refreshed with its current text.
 * @param text      指向新文本的指针。若为 NULL,则用当前文本刷新标签。
 */
void lv_arclabel_set_text_static(lv_obj_t * obj, const char * text);

/**
 * Set the start angle of an arc. 0 deg: right, 90 bottom, etc.
 * 设置圆弧的起始角度。0度:右侧,90度:底部等。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param start     the start angle. (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param start     起始角度(若启用 `LV_USE_FLOAT`,可为小数)
 */
void lv_arclabel_set_angle_start(lv_obj_t * obj, lv_value_precise_t start);

/**
 * Set the end angle of an arc. 0 deg: right, 90 bottom, etc.
 * 设置圆弧的角度大小。0度:右侧,90度:底部等。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param size      the angle size (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @param size      角度大小(若启用 `LV_USE_FLOAT`,可为小数)
 */
void lv_arclabel_set_angle_size(lv_obj_t * obj, lv_value_precise_t size);

/**
 * Set the rotation for the whole arc
 * 设置整个圆弧的旋转角度
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param offset    rotation angle
 * @param offset    旋转角度
 */
void lv_arclabel_set_offset(lv_obj_t * obj, int32_t offset);

/**
 * Set the type of arc.
 * 设置圆弧标签的文字方向。
 * @param obj       pointer to and arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param dir      arc label's direction
 * @param dir       圆弧标签的文字方向
 */
void lv_arclabel_set_dir(lv_obj_t * obj, lv_arclabel_dir_t dir);

/**
 * Enable the recoloring by in-line commands
 * 启用通过行内命令实现的重新着色功能
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param en        true: enable recoloring, false: disable
 * @param en        true:启用重新着色,false:禁用
 * Example: "This is a #ff0000 red# word"
 * 示例:"This is a #ff0000 red# word"
 */
void lv_arclabel_set_recolor(lv_obj_t * obj, bool en);

/**
 * Set the radius for an arc label object.
 * 设置圆弧标签对象的半径。
 *
 * @param obj       pointer to the arc label object.
 * @param obj       指向圆弧标签对象的指针。
 * @param radius    The radius value to set for the label's curvature, in pixels.
 * @param radius    要设置的标签曲率半径值,单位:像素。
 */
void lv_arclabel_set_radius(lv_obj_t * obj, uint32_t radius);

/**
 * Set the center offset x for an arc label object.
 * 设置圆弧标签对象的圆心 X 偏移。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param x         the x offset
 * @param x         X 方向偏移
 */
void lv_arclabel_set_center_offset_x(lv_obj_t * obj, uint32_t x);

/**
 * Set the center offset y for an arc label object.
 * 设置圆弧标签对象的圆心 Y 偏移。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param y         the y offset
 * @param y         Y 方向偏移
 */
void lv_arclabel_set_center_offset_y(lv_obj_t * obj, uint32_t y);

/**
 * Set the text vertical alignment for an arc label object.
 * 设置圆弧标签对象的文本垂直对齐方式。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param align     the vertical alignment
 * @param align     垂直对齐方式
 */
void lv_arclabel_set_text_vertical_align(lv_obj_t * obj, lv_arclabel_text_align_t align);

/**
 * Set the text horizontal alignment for an arc label object.
 * 设置圆弧标签对象的文本水平对齐方式。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param align     the horizontal alignment
 * @param align     水平对齐方式
 */
void lv_arclabel_set_text_horizontal_align(lv_obj_t * obj, lv_arclabel_text_align_t align);

/**
 * Set the overflow behavior for an arc label object.
 * 设置圆弧标签对象的溢出处理行为。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param overflow  the overflow mode (visible, ellipsis, clip)
 * @param overflow  溢出模式(可见、省略号、裁剪)
 */
void lv_arclabel_set_overflow(lv_obj_t * obj, lv_arclabel_overflow_t overflow);

/**
 * Set the end overlap behavior for an arc label object.
 * 设置圆弧标签对象的端点重叠处理行为。
 * This controls how text is handled when it would overlap at the end of a 360-degree arc.
 * 该选项控制当文本在 360 度圆弧末端会发生重叠时的处理方式。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @param overlap   set the arc label's end overlap behavior
 * @param overlap   设置圆弧标签的端点重叠行为
 */
void lv_arclabel_set_end_overlap(lv_obj_t * obj, bool overlap);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the start angle of an arc label.
 * 获取圆弧标签的起始角度。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the start angle [0..360]  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @return          起始角度 [0..360](若启用 `LV_USE_FLOAT`,可为小数)
 */
lv_value_precise_t lv_arclabel_get_angle_start(lv_obj_t * obj);

/**
 * Get the angle size of an arc label.
 * 获取圆弧标签的角度大小。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the end angle [0..360]  (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @return          角度大小 [0..360](若启用 `LV_USE_FLOAT`,可为小数)
 */
lv_value_precise_t lv_arclabel_get_angle_size(lv_obj_t * obj);

/**
 * Get whether the arc label is type or not.
 * 获取圆弧标签的方向。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          arc label's direction
 * @return          圆弧标签的方向
 */
lv_arclabel_dir_t lv_arclabel_get_dir(const lv_obj_t * obj);

/**
 * Enable the recoloring by in-line commands
 * 启用通过行内命令实现的重新着色功能
 *
 * @see lv_arclabel_set_recolor
 *
 * @param obj       pointer to a label object
 * @param obj       指向标签对象的指针
 * @return          true: enable recoloring, false: disable
 * @return          true:启用重新着色,false:禁用
 */
bool lv_arclabel_get_recolor(lv_obj_t * obj);

/**
 * Get the text of the arc label.
 * 获取圆弧标签的半径。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the radius of the arc label
 * @return          圆弧标签的半径
 */
uint32_t lv_arclabel_get_radius(lv_obj_t * obj);

/**
 * Get the center offset x for an arc label object.
 * 获取圆弧标签对象的圆心 X 偏移。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the x offset
 * @return          X 方向偏移
 */
uint32_t lv_arclabel_get_center_offset_x(lv_obj_t * obj);

/**
 * Get the center offset y for an arc label object.
 * 获取圆弧标签对象的圆心 Y 偏移。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the y offset
 * @return          Y 方向偏移
 */
uint32_t lv_arclabel_get_center_offset_y(lv_obj_t * obj);

/**
 * Get the text vertical alignment for an arc label object.
 * 获取圆弧标签对象的文本垂直对齐方式。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the vertical alignment
 * @return          垂直对齐方式
 */
lv_arclabel_text_align_t lv_arclabel_get_text_vertical_align(lv_obj_t * obj);

/**
 * Get the text horizontal alignment for an arc label object.
 * 获取圆弧标签对象的文本水平对齐方式。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the horizontal alignment
 * @return          水平对齐方式
 */
lv_arclabel_text_align_t lv_arclabel_get_text_horizontal_align(lv_obj_t * obj);

/**
 * Get the overflow behavior for an arc label object.
 * 获取圆弧标签对象的溢出处理行为。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the overflow mode
 * @return          溢出模式
 */
lv_arclabel_overflow_t lv_arclabel_get_overflow(lv_obj_t * obj);

/**
 * Get the end overlap behavior for an arc label object.
 * 获取圆弧标签对象的端点重叠处理行为。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the end overlap mode
 * @return          端点重叠模式
 */
bool lv_arclabel_get_end_overlap(lv_obj_t * obj);

/**
 * Get the text angle for an arc label object.
 * 获取圆弧标签对象的文本角度。
 * @note            The text angle is calculated at runtime. You can get the updated value
 *                  after the arclabel's size has been updated.
 *                  Returns the real rendered text angle in degrees except in
 *                  `LV_ARCLABEL_OVERFLOW_VISIBLE` mode.
 *                  @note 文本角度在运行时计算。在圆弧标签的尺寸更新后,
 *                  才能获取到更新后的值。除 `LV_ARCLABEL_OVERFLOW_VISIBLE`
 *                  模式外,返回实际渲染的文本角度(度)。
 * @param obj       pointer to an arc label object
 * @param obj       指向圆弧标签对象的指针
 * @return          the text angle (if `LV_USE_FLOAT` is enabled it can be fractional too.)
 * @return          文本角度(若启用 `LV_USE_FLOAT`,可为小数)
 */
lv_value_precise_t lv_arclabel_get_text_angle(lv_obj_t * obj);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_ARCLABEL*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_ARCLABEL_H*/
