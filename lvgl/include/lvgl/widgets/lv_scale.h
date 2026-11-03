/**
 * @file lv_scale.h
 * @brief 刻度(Scale)控件,用于绘制带刻度和指针的刻度盘/标尺
 *
 */

#ifndef LV_SCALE_H
#define LV_SCALE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_SCALE != 0

#include "../core/lv_obj.h"
#include "lv_line.h"
#include "lv_image.h"
#include "../core/lv_observer.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**Default value of total minor ticks. */
/**默认的小刻度总数量。 */
#define LV_SCALE_TOTAL_TICK_COUNT_DEFAULT (11U)
LV_EXPORT_CONST_INT(LV_SCALE_TOTAL_TICK_COUNT_DEFAULT);

/**Default value of major tick every nth ticks. */
/**默认每 n 个刻度绘制一次主刻度的值。 */
#define LV_SCALE_MAJOR_TICK_EVERY_DEFAULT (5U)
LV_EXPORT_CONST_INT(LV_SCALE_MAJOR_TICK_EVERY_DEFAULT);

/**Default value of scale label enabled. */
/**默认启用刻度标签的值。 */
#define LV_SCALE_LABEL_ENABLED_DEFAULT (1U)
LV_EXPORT_CONST_INT(LV_SCALE_LABEL_ENABLED_DEFAULT);

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**
 * Scale mode
 * 刻度模式
 */
typedef enum {
    /** 水平刻度模式,刻度位于顶部 */
    LV_SCALE_MODE_HORIZONTAL_TOP    = 0x00U,
    /** 水平刻度模式,刻度位于底部 */
    LV_SCALE_MODE_HORIZONTAL_BOTTOM = 0x01U,
    /** 垂直刻度模式,刻度位于左侧 */
    LV_SCALE_MODE_VERTICAL_LEFT     = 0x02U,
    /** 垂直刻度模式,刻度位于右侧 */
    LV_SCALE_MODE_VERTICAL_RIGHT    = 0x04U,
    /** 环形刻度模式,刻度位于内侧 */
    LV_SCALE_MODE_ROUND_INNER       = 0x08U,
    /** 环形刻度模式,刻度位于外侧 */
    LV_SCALE_MODE_ROUND_OUTER      = 0x10U,
    LV_SCALE_MODE_LAST
} lv_scale_mode_t;

/** 刻度标签的旋转角度与刻度方向一致 */
#define LV_SCALE_LABEL_ROTATE_MATCH_TICKS  0x100000
LV_EXPORT_CONST_INT(LV_SCALE_LABEL_ROTATE_MATCH_TICKS);

/** 刻度标签始终保持竖直方向 */
#define LV_SCALE_LABEL_ROTATE_KEEP_UPRIGHT 0x80000
LV_EXPORT_CONST_INT(LV_SCALE_LABEL_ROTATE_KEEP_UPRIGHT);

/** 刻度旋转角度的掩码 */
#define LV_SCALE_ROTATION_ANGLE_MASK 0x7FFFF
LV_EXPORT_CONST_INT(LV_SCALE_ROTATION_ANGLE_MASK);

/** 刻度控件的对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_scale_class;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_scale_id_t {
    LV_PROPERTY_ID(SCALE, MODE,               LV_PROPERTY_TYPE_INT,   0),
    LV_PROPERTY_ID(SCALE, TOTAL_TICK_COUNT,   LV_PROPERTY_TYPE_INT,   1),
    LV_PROPERTY_ID(SCALE, MAJOR_TICK_EVERY,   LV_PROPERTY_TYPE_INT,   2),
    LV_PROPERTY_ID(SCALE, LABEL_SHOW,         LV_PROPERTY_TYPE_BOOL,  3),
    LV_PROPERTY_ID(SCALE, ANGLE_RANGE,        LV_PROPERTY_TYPE_INT,   4),
    LV_PROPERTY_ID(SCALE, ROTATION,           LV_PROPERTY_TYPE_INT,   5),
    LV_PROPERTY_ID(SCALE, RANGE_MIN_VALUE,    LV_PROPERTY_TYPE_INT,   6),
    LV_PROPERTY_ID(SCALE, RANGE_MAX_VALUE,    LV_PROPERTY_TYPE_INT,   7),
    LV_PROPERTY_SCALE_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create an scale object
 * 创建一个刻度控件对象
 * @param parent    pointer to an object, it will be the parent of the new scale
 * @param parent    指向一个对象,它将是新刻度的父对象
 * @return          pointer to created Scale Widget
 * @return          指向创建的刻度控件的指针
 */
lv_obj_t * lv_scale_create(lv_obj_t * parent);

/*======================
 * Add/remove functions
 * 添加/移除函数
 *=====================*/

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set scale mode. See lv_scale_mode_t.
 * 设置刻度模式,参见 lv_scale_mode_t。
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @param mode      the new scale mode
 * @param mode      新的刻度模式
 */
void lv_scale_set_mode(lv_obj_t * obj, lv_scale_mode_t mode);

/**
 * Set scale total tick count (including minor and major ticks).
 * 设置刻度的总刻度数(包括小刻度和大刻度)。
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @param total_tick_count    New total tick count
 * @param total_tick_count    新的总刻度数
 */
void lv_scale_set_total_tick_count(lv_obj_t * obj, uint32_t total_tick_count);

/**
 * Sets how often major ticks are drawn.
 * 设置每隔多少个刻度绘制一次主刻度。
 * @param obj                 pointer to Scale Widget
 * @param obj                 指向刻度控件
 * @param major_tick_every    the new count for major tick drawing
 * @param major_tick_every    新的主刻度绘制间隔数
 */
void lv_scale_set_major_tick_every(lv_obj_t * obj, uint32_t major_tick_every);

/**
 * Sets label visibility.
 * 设置刻度标签的可见性。
 * @param obj           pointer to Scale Widget
 * @param obj           指向刻度控件
 * @param show_label    true/false to enable tick label
 * @param show_label    true/false 是否启用刻度标签
 */
void lv_scale_set_label_show(lv_obj_t * obj, bool show_label);

/**
 * Set minimum and maximum values on Scale.
 * 设置刻度的最小值和最大值。
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @param min       minimum value of Scale
 * @param min       刻度的最小值
 * @param max       maximum value of Scale
 * @param max       刻度的最大值
 */
void lv_scale_set_range(lv_obj_t * obj, int32_t min, int32_t max);

/**
 * Set minimum values on Scale.
 * 设置刻度的最小值。
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @param min       minimum value of Scale
 * @param min       刻度的最小值
 */
void lv_scale_set_min_value(lv_obj_t * obj, int32_t min);

/**
 * Set maximum values on Scale.
 * 设置刻度的最大值。
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @param min       minimum value of Scale
 * @param min       刻度的最小值
 */
void lv_scale_set_max_value(lv_obj_t * obj, int32_t max);

/**
 * Set angle between the low end and the high end of the Scale.
 * (Applies only to round Scales.)
 * 设置刻度低端与高端之间的角度范围。(仅适用于环形刻度。)
 * @param obj         pointer to Scale Widget
 * @param obj         指向刻度控件
 * @param max_angle   angle in degrees from Scale minimum where top end of Scale will be drawn
 * @param max_angle   以度为单位的角度,即从刻度最小值到刻度顶端绘制的角度
 */
void lv_scale_set_angle_range(lv_obj_t * obj, uint32_t angle_range);

/**
 * Set angular offset from the 3-o'clock position of the low end of the Scale.
 * (Applies only to round Scales.)
 * 设置刻度低端相对 3 点钟方向的角偏移。(仅适用于环形刻度。)
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @param rotation  clockwise angular offset (in degrees) from the 3-o'clock position
 * @param rotation  相对 3 点钟方向顺时针旋转的角度偏移(以度为单位)
 *                  of the low end of the scale; negative and >360 values are first normalized
 *                  刻度低端的偏移角度;负值和大于 360 的值会先被归一化
 *                  to range [0..360].
 *                  到 [0..360] 范围内。
 *                  Examples:
 *                  示例:
 *                      -   0 = 3 o'clock (right side)
 *                      -   0 = 3 点钟方向(右侧)
 *                      -  30 = 4 o'clock
 *                      -  30 = 4 点钟方向
 *                      -  60 = 5 o'clock
 *                      -  60 = 5 点钟方向
 *                      -  90 = 6 o'clock
 *                      -  90 = 6 点钟方向
 *                      - 135 = midway between 7 and 8 o'clock (default)
 *                      - 135 = 7 点和 8 点钟方向之间(默认值)
 *                      - 180 = 9 o'clock
 *                      - 180 = 9 点钟方向
 *                      - 270 = 12 o'clock
 *                      - 270 = 12 点钟方向
 *                      - 300 = 1 o'clock
 *                      - 300 = 1 点钟方向
 *                      - 330 = 2 o'clock
 *                      - 330 = 2 点钟方向
 *                      - -30 = 2 o'clock
 *                      - -30 = 2 点钟方向
 *                      - 390 = 4 o'clock
 *                      - 390 = 4 点钟方向
 */
void lv_scale_set_rotation(lv_obj_t * obj, int32_t rotation);

/**
 * Point line needle to specified value.
 * 将线段指针指向指定的数值。
 * @param obj              pointer to Scale Widget
 * @param obj              指向刻度控件
 * @param needle_line      needle_line of the Scale. The line points will be allocated and
 * @param needle_line      刻度的线段指针。线段点数组将由刻度控件分配和管理
 *                         managed by the Scale unless the line point array was previously set
 *                         除非之前已通过 `lv_line_set_points_mutable` 设置了线段点数组
 *                         using `lv_line_set_points_mutable`.
 * @param needle_length    length of the needle
 * @param needle_length    指针的长度
 *                         - needle_length>0: needle_length=needle_length;
 *                         - needle_length>0:指针长度 = needle_length;
 *                         - needle_length<0: needle_length=radius-|needle_length|;
 *                         - needle_length<0:指针长度 = 半径 - |needle_length|;
 * @param value            Scale value needle will point to
 * @param value            指针将指向的刻度数值
 */
void lv_scale_set_line_needle_value(lv_obj_t * obj, lv_obj_t * needle_line, int32_t needle_length,
                                    int32_t value);

/**
 * Point image needle to specified value;
   image must point to the right. E.g. -O------>
 * 将图片指针指向指定的数值;图片必须指向右侧,例如 -O------>
 * @param obj              pointer to Scale Widget
 * @param obj              指向刻度控件
 * @param needle_img       pointer to needle's Image
 * @param needle_img       指向指针图片的指针
 * @param value            Scale value needle will point to
 * @param value            指针将指向的刻度数值
 */
void lv_scale_set_image_needle_value(lv_obj_t * obj, lv_obj_t * needle_img, int32_t value);

/**
 * Set custom text source for major ticks labels.
 * 为主刻度标签设置自定义文本源。
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @param txt_src   pointer to an array of strings which will be display at major ticks;
 * @param txt_src   指向将在主刻度处显示的字符串数组的指针;
 *                  last element must be a NULL pointer.
 *                  最后一个元素必须为 NULL 指针。
 */
void lv_scale_set_text_src(lv_obj_t * obj, const char * txt_src[]);

/**
 * Draw Scale after all its children are drawn.
 * 在所有子对象绘制完成后再绘制刻度控件。
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @param en        true: enable post draw
 * @param en        true:启用后置绘制
 */
void lv_scale_set_post_draw(lv_obj_t * obj, bool en);

/**
 * Draw Scale ticks on top of all other parts.
 * 将刻度绘制在其他所有部分之上。
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @param en        true: enable draw ticks on top of all parts
 * @param en        true:启用将刻度绘制在所有部分之上
 */
void lv_scale_set_draw_ticks_on_top(lv_obj_t * obj, bool en);

/**
 * Add a Section to specified Scale.  Section will not be drawn until
 * a valid range is set for it using `lv_scale_set_section_range()`.
 * 向指定的刻度控件添加一个分段(Section)。在通过 `lv_scale_set_section_range()` 为其设置
 * 有效范围之前,该分段不会被绘制。
 * @param obj       pointer to Scale Widget
 * @param obj       指向刻度控件
 * @return          pointer to new Section
 * @return          指向新分段的指针
 */
lv_scale_section_t * lv_scale_add_section(lv_obj_t * obj);

/**
 * DEPRECATED, use lv_scale_set_section_range instead.
 * 已弃用,请改用 lv_scale_set_section_range。
 * Set range for specified Scale Section
 * 为指定的刻度分段设置范围
 * @param section       pointer to Section
 * @param section       指向分段
 * @param range_min     Section new minimum value
 * @param range_min     分段新的最小值
 * @param range_max     Section new maximum value
 * @param range_max     分段新的最大值
 */
void lv_scale_section_set_range(lv_scale_section_t * section, int32_t min, int32_t max);

/**
 * Set the range of a scale section
 * 设置刻度分段的范围
 * @param scale         pointer to scale
 * @param scale         指向刻度控件
 * @param section       pointer to section
 * @param section       指向分段
 * @param range_min     the section's new minimum value
 * @param range_min     分段新的最小值
 * @param range_max     the section's new maximum value
 * @param range_max     分段新的最大值
 */
void lv_scale_set_section_range(lv_obj_t * scale, lv_scale_section_t * section, int32_t min, int32_t max);

/**
 * Set the minimum value of a scale section
 * 设置刻度分段的最小值
 * @param scale         pointer to scale
 * @param scale         指向刻度控件
 * @param section       pointer to section
 * @param section       指向分段
 * @param min           the section's new minimum value
 * @param min           分段新的最小值
 */
void lv_scale_set_section_min_value(lv_obj_t * scale, lv_scale_section_t * section, int32_t min);

/**
 * Set the maximum value of a scale section
 * 设置刻度分段的最大值
 * @param scale         pointer to scale
 * @param scale         指向刻度控件
 * @param section       pointer to section
 * @param section       指向分段
 * @param max           the section's new maximum value
 * @param max           分段新的最大值
 */
void lv_scale_set_section_max_value(lv_obj_t * scale, lv_scale_section_t * section, int32_t max);

/**
 * DEPRECATED, use lv_scale_set_section_style_main/indicator/items instead.
 * 已弃用,请改用 lv_scale_set_section_style_main/indicator/items。
 * Set style for specified part of Section.
 * 为分段的指定部分设置样式。
 * @param section             pointer to Section
 * @param section             指向分段
 * @param part                the part of the Scale the style will apply to, e.g. LV_PART_INDICATOR
 * @param part                样式将应用于刻度的哪个部分,例如 LV_PART_INDICATOR
 * @param section_part_style  pointer to style to apply
 * @param section_part_style  指向要应用的样式的指针
 */
LV_DEPRECATED("use lv_scale_set_section_style_main/indicator/items instead")
void lv_scale_section_set_style(lv_scale_section_t * section, lv_part_t part, lv_style_t * section_part_style);

/**
 * Set the style of the line on a section.
 * 设置分段上线条的样式。
 * @param scale         pointer to scale
 * @param scale         指向刻度控件
 * @param section       pointer to section
 * @param section       指向分段
 * @param style         point to a style
 * @param style         指向一个样式
 */
void lv_scale_set_section_style_main(lv_obj_t * scale, lv_scale_section_t * section, const lv_style_t * style);

/**
 * Set the style of the major ticks and label on a section.
 * 设置分段上主刻度及其标签的样式。
 * @param scale         pointer to scale
 * @param scale         指向刻度控件
 * @param section       pointer to section
 * @param section       指向分段
 * @param style         point to a style
 * @param style         指向一个样式
 */
void lv_scale_set_section_style_indicator(lv_obj_t * scale, lv_scale_section_t * section, const lv_style_t * style);

/**
 * Set the style of the minor ticks on a section.
 * 设置分段上小刻度的样式。
 * @param scale         pointer to scale
 * @param scale         指向刻度控件
 * @param section       pointer to section
 * @param section       指向分段
 * @param style         point to a style
 * @param style         指向一个样式
 */
void lv_scale_set_section_style_items(lv_obj_t * scale, lv_scale_section_t * section, const lv_style_t * style);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get scale mode. See lv_scale_mode_t
 * 获取刻度模式,参见 lv_scale_mode_t
 * @param obj   pointer to Scale Widget
 * @param obj   指向刻度控件
 * @return      Scale mode
 * @return      刻度模式
 */
lv_scale_mode_t lv_scale_get_mode(lv_obj_t * obj);

/**
 * Get scale total tick count (including minor and major ticks)
 * 获取刻度的总刻度数(包括小刻度和大刻度)
 * @param obj   pointer to Scale Widget
 * @param obj   指向刻度控件
 * @return      Scale total tick count
 * @return      刻度的总刻度数
 */
int32_t lv_scale_get_total_tick_count(lv_obj_t * obj);

/**
 * Get how often the major tick will be drawn
 * 获取每隔多少个刻度绘制一次主刻度
 * @param obj   pointer to Scale Widget
 * @param obj   指向刻度控件
 * @return      Scale major tick every count
 * @return      主刻度绘制间隔数
 */
int32_t lv_scale_get_major_tick_every(lv_obj_t * obj);

/**
 * Get angular location of low end of Scale.
 * 获取刻度低端的角位置。
 * @param obj   pointer to Scale Widget
 * @param obj   指向刻度控件
 * @return      Scale low end angular location
 * @return      刻度低端的角位置
 */
int32_t lv_scale_get_rotation(lv_obj_t * obj);

/**
 * Gets label visibility
 * 获取刻度标签的可见性
 * @param obj   pointer to Scale Widget
 * @param obj   指向刻度控件
 * @return      true if tick label is enabled, false otherwise
 * @return      若刻度标签启用则返回 true,否则返回 false
 */
bool lv_scale_get_label_show(lv_obj_t * obj);

/**
 * Get Scale's range in degrees
 * 获取刻度的角度范围(以度为单位)
 * @param obj   pointer to Scale Widget
 * @param obj   指向刻度控件
 * @return      Scale's angle_range
 * @return      刻度的角度范围
 */
uint32_t lv_scale_get_angle_range(lv_obj_t * obj);

/**
 * Get minimum value for Scale
 * 获取刻度的最小值
 * @param obj   pointer to Scale Widget
 * @param obj   指向刻度控件
 * @return      Scale's minimum value
 * @return      刻度的最小值
 */
int32_t lv_scale_get_range_min_value(lv_obj_t * obj);

/**
 * Get maximum value for Scale
 * 获取刻度的最大值
 * @param obj   pointer to Scale Widget
 * @param obj   指向刻度控件
 * @return      Scale's maximum value
 * @return      刻度的最大值
 */
int32_t lv_scale_get_range_max_value(lv_obj_t * obj);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

#if LV_USE_OBSERVER

/**
 * Bind an integer subject to a scales section minimum value
 * 将整型 subject 绑定到刻度分段的最小值上
 * @param obj       pointer to a Scale
 * @param obj       指向刻度控件
 * @param section   pointer to a Scale section
 * @param section   指向刻度分段
 * @param subject   pointer to a Subject
 * @param subject   指向 Subject 的指针
 * @return          pointer to newly-created Observer
 * @return          指向新创建的 Observer 的指针
 */
lv_observer_t * lv_scale_bind_section_min_value(lv_obj_t * obj, lv_scale_section_t * section, lv_subject_t * subject);

/**
 * Bind an integer subject to a scales section maximum value
 * 将整型 subject 绑定到刻度分段的最大值上
 * @param obj       pointer to an Scale
 * @param obj       指向刻度控件
 * @param section   pointer to a Scale section
 * @param section   指向刻度分段
 * @param subject   pointer to a Subject
 * @param subject   指向 Subject 的指针
 * @return          pointer to newly-created Observer
 * @return          指向新创建的 Observer 的指针
 */
lv_observer_t * lv_scale_bind_section_max_value(lv_obj_t * obj, lv_scale_section_t * section, lv_subject_t * subject);

/**
 * Bind an integer subject to a scale's line needle value
 * 将整型 subject 绑定到刻度的线段指针数值上
 * @param obj           pointer to a Scale
 * @param obj           指向刻度控件
 * @param needle_line   pointer to a line needle
 * @param needle_line   指向线段指针
 * @param needle_length length of the needle
 * @param needle_length 指针的长度
 * @param subject       pointer to a Subject
 * @param subject       指向 Subject 的指针
 * @return              pointer to newly-created Observer
 * @return              指向新创建的 Observer 的指针
 */
lv_observer_t * lv_scale_bind_line_needle_value(lv_obj_t * obj, lv_obj_t * needle_line, int32_t needle_length,
                                                lv_subject_t * subject);

/**
 * Bind an integer subject to a scale's image needle value
 * 将整型 subject 绑定到刻度的图片指针数值上
 * @param obj           pointer to a Scale
 * @param obj           指向刻度控件
 * @param needle_img    pointer to an image needle
 * @param needle_img    指向图片指针
 * @param subject       pointer to a Subject
 * @param subject       指向 Subject 的指针
 * @return              pointer to newly-created Observer
 * @return              指向新创建的 Observer 的指针
 */
lv_observer_t * lv_scale_bind_image_needle_value(lv_obj_t * obj, lv_obj_t * needle_img, lv_subject_t * subject);

#endif

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_SCALE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCALE_H*/
