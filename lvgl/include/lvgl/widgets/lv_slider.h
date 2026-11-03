/**
 * @file lv_slider.h
 * @brief 滑块(Slider)控件,用于通过拖动滑块来调整数值
 *
 */

#ifndef LV_SLIDER_H
#define LV_SLIDER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "lv_bar.h"

#if LV_USE_SLIDER

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/
typedef enum {
    /** 普通模式,滑块从最小值到最大值 */
    LV_SLIDER_MODE_NORMAL = LV_BAR_MODE_NORMAL,
    /** 对称模式,滑块以中心点向两侧对称取值 */
    LV_SLIDER_MODE_SYMMETRICAL = LV_BAR_MODE_SYMMETRICAL,
    /** 范围模式,滑块可有两个手柄(左/右)表示范围 */
    LV_SLIDER_MODE_RANGE = LV_BAR_MODE_RANGE
} lv_slider_mode_t;

typedef enum {
    /** 自动方向,根据布局自动选择 */
    LV_SLIDER_ORIENTATION_AUTO = LV_BAR_ORIENTATION_AUTO,
    /** 水平方向 */
    LV_SLIDER_ORIENTATION_HORIZONTAL = LV_BAR_ORIENTATION_HORIZONTAL,
    /** 垂直方向 */
    LV_SLIDER_ORIENTATION_VERTICAL = LV_BAR_ORIENTATION_VERTICAL
} lv_slider_orientation_t;

/** 滑块控件的对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_slider_class;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_slider_id_t {
    LV_PROPERTY_ID2(SLIDER, VALUE,          LV_PROPERTY_TYPE_INT,   LV_PROPERTY_TYPE_BOOL,  0),
    LV_PROPERTY_ID2(SLIDER, LEFT_VALUE,     LV_PROPERTY_TYPE_INT,   LV_PROPERTY_TYPE_BOOL,  1),
    LV_PROPERTY_ID2(SLIDER, RANGE,          LV_PROPERTY_TYPE_INT,   LV_PROPERTY_TYPE_INT,   2),
    LV_PROPERTY_ID(SLIDER, MIN_VALUE,       LV_PROPERTY_TYPE_INT,       4),
    LV_PROPERTY_ID(SLIDER, MAX_VALUE,       LV_PROPERTY_TYPE_INT,       5),
    LV_PROPERTY_ID(SLIDER, MODE,            LV_PROPERTY_TYPE_INT,       6),
    LV_PROPERTY_ID(SLIDER, IS_DRAGGED,      LV_PROPERTY_TYPE_BOOL,      7),
    LV_PROPERTY_ID(SLIDER, IS_SYMMETRICAL,  LV_PROPERTY_TYPE_BOOL,      8),
    LV_PROPERTY_SLIDER_END,
};
#endif
/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a slider object
 * 创建一个滑块控件对象
 * @param parent    pointer to an object, it will be the parent of the new slider.
 * @param parent    指向一个对象,它将是新滑块的父对象。
 * @return          pointer to the created slider
 * @return          指向创建的滑块对象的指针
 */
lv_obj_t * lv_slider_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set a new value on the slider
 * 为滑块设置新的数值
 * @param obj       pointer to a slider object
 * @param obj       指向滑块对象
 * @param value     the new value
 * @param value     新的数值
 * @param anim      LV_ANIM_ON: set the value with an animation; LV_ANIM_OFF: change the value immediately
 * @param anim      LV_ANIM_ON:带动画设置数值;LV_ANIM_OFF:立即改变数值
 */
void lv_slider_set_value(lv_obj_t * obj, int32_t value, lv_anim_enable_t anim);

/**
 * Set a new value for the left knob of a slider
 * 为滑块的左侧手柄设置新的数值
 * @param obj       pointer to a slider object
 * @param obj       指向滑块对象
 * @param value     new value
 * @param value     新的数值
 * @param anim      LV_ANIM_ON: set the value with an animation; LV_ANIM_OFF: change the value immediately
 * @param anim      LV_ANIM_ON:带动画设置数值;LV_ANIM_OFF:立即改变数值
 */
void lv_slider_set_start_value(lv_obj_t * obj, int32_t value, lv_anim_enable_t anim);

/**
 * Set the minimum and the maximum values of a bar
 * 设置滑块的最小值和最大值
 * @param obj       pointer to the slider object
 * @param obj       指向滑块对象
 * @param min       minimum value
 * @param min       最小值
 * @param max       maximum value
 * @param max       最大值
 */
void lv_slider_set_range(lv_obj_t * obj, int32_t min, int32_t max);

/**
 * Set the minimum values of a bar
 * 设置滑块的最小值
 * @param obj       pointer to the slider object
 * @param obj       指向滑块对象
 * @param min       minimum value
 * @param min       最小值
 */
void lv_slider_set_min_value(lv_obj_t * obj, int32_t min);

/**
 * Set the maximum values of a bar
 * 设置滑块的最大值
 * @param obj       pointer to the slider object
 * @param obj       指向滑块对象
 * @param max       maximum value
 * @param max       最大值
 */
void lv_slider_set_max_value(lv_obj_t * obj, int32_t max);

/**
 * Set the mode of slider.
 * 设置滑块的模式。
 * @param obj       pointer to a slider object
 * @param obj       指向滑块对象
 * @param mode      the mode of the slider. See `lv_slider_mode_t`
 * @param mode      滑块的模式,参见 `lv_slider_mode_t`
 */
void lv_slider_set_mode(lv_obj_t * obj, lv_slider_mode_t mode);

/**
 * Set the orientation of slider.
 * 设置滑块的方向。
 * @param obj           pointer to a slider object
 * @param obj           指向滑块对象
 * @param orientation   slider  orientation from `lv_slider_orientation_t`
 * @param orientation   滑块的方向,来自 `lv_slider_orientation_t`
 */
void lv_slider_set_orientation(lv_obj_t * obj, lv_slider_orientation_t orientation);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the value of the main knob of a slider
 * 获取滑块主手柄的数值
 * @param obj       pointer to a slider object
 * @param obj       指向滑块对象
 * @return          the value of the main knob of the slider
 * @return          滑块主手柄的数值
 */
int32_t lv_slider_get_value(const lv_obj_t * obj);

/**
 * Get the value of the left knob of a slider
 * 获取滑块左侧手柄的数值
 * @param obj       pointer to a slider object
 * @param obj       指向滑块对象
 * @return          the value of the left knob of the slider
 * @return          滑块左侧手柄的数值
 */
int32_t lv_slider_get_left_value(const lv_obj_t * obj);

/**
 * Get the minimum value of a slider
 * 获取滑块的最小值
 * @param obj       pointer to a slider object
 * @param obj       指向滑块对象
 * @return          the minimum value of the slider
 * @return          滑块的最小值
 */
int32_t lv_slider_get_min_value(const lv_obj_t * obj);

/**
 * Get the maximum value of a slider
 * 获取滑块的最大值
 * @param obj       pointer to a slider object
 * @param obj       指向滑块对象
 * @return          the maximum value of the slider
 * @return          滑块的最大值
 */
int32_t lv_slider_get_max_value(const lv_obj_t * obj);

/**
 * Give the slider is being dragged or not
 * 获取滑块是否正在被拖拽
 * @param obj       pointer to a slider object
 * @param obj       指向滑块对象
 * @return          true: drag in progress false: not dragged
 * @return          true:正在拖拽 false:未被拖拽
 */
bool lv_slider_is_dragged(const lv_obj_t * obj);

/**
 * Get the mode of the slider.
 * 获取滑块的模式。
 * @param slider       pointer to a slider object
 * @param slider       指向滑块对象
 * @return          see `lv_slider_mode_t`
 * @return          参见 `lv_slider_mode_t`
 */
lv_slider_mode_t lv_slider_get_mode(lv_obj_t * slider);

/**
 * Get the orientation of slider.
 * 获取滑块的方向。
 * @param obj       pointer to a slider object
 * @param obj       指向滑块对象
 * @return          slider orientation from `lv_slider_orientation_t`
 * @return          滑块的方向,来自 `lv_slider_orientation_t`
 */
lv_slider_orientation_t lv_slider_get_orientation(lv_obj_t * slider);

/**
 * Give the slider is in symmetrical mode or not
 * 获取滑块是否处于对称模式
 * @param obj       pointer to slider object
 * @param obj       指向滑块对象
 * @return          true: in symmetrical mode false : not in
 * @return          true:处于对称模式 false:不处于对称模式
*/
bool lv_slider_is_symmetrical(lv_obj_t * obj);


#if LV_USE_OBSERVER
/**
 * Bind an integer or float Subject to a Slider's value.
 * 将整型或浮点型 Subject 绑定到滑块的数值上。
 * @param obj       pointer to Slider
 * @param obj       指向滑块对象
 * @param subject   pointer to Subject
 * @param subject   指向 Subject 的指针
 * @return          pointer to newly-created Observer
 * @return          指向新创建的 Observer 的指针
 */
lv_observer_t * lv_slider_bind_value(lv_obj_t * obj, lv_subject_t * subject);
#endif

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_SLIDER*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SLIDER_H*/
