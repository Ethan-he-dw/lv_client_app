/**
 * @file lv_observer_private.h
 * 观察者私有模块：定义订阅主体(subject)变更的观察者描述符等内部数据结构。
 *
 */

#ifndef LV_OBSERVER_PRIVATE_H
#define LV_OBSERVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/

#include "../lvgl_public.h"

#if LV_USE_OBSERVER

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**
 * The observer object: a descriptor returned when subscribing LVGL widgets to subjects
 * 观察者对象：将 LVGL 部件订阅到主体时返回的描述符
 */
struct _lv_observer_t {
    lv_subject_t * subject;             /**< Observed subject
                                         * 被观察的主体 */
    lv_observer_cb_t cb;                /**< Callback that notifies when value changes
                                         * 值变化时发出通知的回调 */
    void * target;                      /**< A target for the observer, e.g. a widget or any pointer
                                         * 观察者的目标，例如一个部件或任意指针 */
    void * user_data;                   /**< Additional parameter supplied when subscribing
                                         * 订阅时提供的附加参数 */
    uint32_t auto_free_user_data : 1;   /**< Automatically free user data when observer is removed
                                         * 观察者被移除时自动释放用户数据 */
    uint32_t notified : 1;              /**< Was observer already notified?
                                         * 观察者是否已被通知？ */
    uint32_t for_obj : 1;               /**< Is `target` a pointer to a Widget (`lv_obj_t *`)?
                                         * `target` 是否是指向部件(`lv_obj_t *`)的指针？ */
};

/**
 * Descriptor created by `lv_obj_add_subject_increment_event()`
 * 由 `lv_obj_add_subject_increment_event()` 创建的描述符
 */
struct _lv_subject_increment_dsc_t {
    lv_subject_t * subject; /**< The subject to adjust
                             * 需要调整的主体 */
    int32_t step;           /**< The step add to the subject
                             * 累加到主体的步长 */
    bool rollover;          /**< Where to start over from the other end when one end is exceeded
                             * 超出任一端时是否从另一端重新开始 */
    int32_t min_value;      /**< Don't set a value smaller than this
                             * 不允许设置小于此值的数值 */
    int32_t max_value;      /**< Don't set a value larger than this
                             * 不允许设置大于此值的数值 */
};

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**********************
 *      MACROS
 *      宏定义
 **********************/

#endif /* LV_USE_OBSERVER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBSERVER_PRIVATE_H*/
