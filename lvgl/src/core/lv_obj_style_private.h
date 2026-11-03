/**
 * @file lv_obj_style_private.h
 * 对象样式私有模块：定义对象样式与样式过渡的私有数据结构及接口。
 *
 */

#ifndef LV_OBJ_STYLE_PRIVATE_H
#define LV_OBJ_STYLE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/

#include "../lvgl_public.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

struct _lv_obj_style_t {
    const lv_style_t * style;   /**< 样式指针 */
    uint32_t selector : 24;     /**< 样式选择器 */
    uint32_t is_local : 1;      /**< 是否为本地样式 */
    uint32_t is_trans : 1;      /**< 是否为过渡样式 */
    uint32_t is_disabled : 1;   /**< 样式是否被禁用 */
    uint32_t is_theme : 1;  /**< The style is added by a theme
                             * 该样式由主题添加 */
};

struct _lv_obj_style_transition_dsc_t {
    uint16_t time;              /**< 过渡时间 */
    uint16_t delay;             /**< 过渡延时 */
    lv_style_selector_t selector;   /**< 过渡生效的选择器 */
    lv_style_prop_t prop;           /**< 过渡的样式属性 */
    lv_anim_path_cb_t path_cb;      /**< 过渡动画路径回调 */
    void * user_data;               /**< 用户数据 */
};


/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Initialize the object related style manager module.
 * Called by LVGL in `lv_init()`
 * 初始化对象相关的样式管理模块。
 * 由 LVGL 在 `lv_init()` 中调用。
 */
void lv_obj_style_init(void);

/**
 * Deinitialize the object related style manager module.
 * Called by LVGL in `lv_deinit()`
 * 反初始化对象相关的样式管理模块。
 * 由 LVGL 在 `lv_deinit()` 中调用。
 */
void lv_obj_style_deinit(void);

/**
 * Used internally to create a style transition
 * 内部使用：创建样式过渡
 * @param obj   对象
 * @param part  部件
 * @param prev_state  之前的状态
 * @param new_state   新的状态
 * @param tr    过渡描述符
 */
void lv_obj_style_create_transition(lv_obj_t * obj, lv_part_t part, lv_state_t prev_state,
                                    lv_state_t new_state, const lv_obj_style_transition_dsc_t * tr);

/**
 * Used internally to compare the appearance of an object in 2 states
 * 内部使用：比较对象在两种状态下的外观
 * @param obj    对象
 * @param state1 状态1
 * @param state2 状态2
 * @return       比较结果
 */
lv_style_state_cmp_t lv_obj_style_state_compare(lv_obj_t * obj, lv_state_t state1, lv_state_t state2);

/**
 * Update the layer type of a widget bayed on its current styles.
 * The result will be stored in `obj->spec_attr->layer_type`
 * 根据部件当前的样式更新其图层类型。
 * 结果将存储在 `obj->spec_attr->layer_type` 中。
 * @param obj       the object whose layer should be updated
 *                  需要更新图层的对象
 */
void lv_obj_update_layer_type(lv_obj_t * obj);

/**********************
 *      MACROS
 *      宏定义
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_STYLE_PRIVATE_H*/
