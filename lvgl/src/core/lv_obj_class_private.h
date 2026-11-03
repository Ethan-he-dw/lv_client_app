/**
 * @file lv_obj_class_private.h
 * 对象类私有模块：定义 lv_obj_class_t 对象类结构体及私有接口。
 *
 */

#ifndef LV_OBJ_CLASS_PRIVATE_H
#define LV_OBJ_CLASS_PRIVATE_H

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

/**
 * Describe the common methods of every object.
 * Similar to a C++ class.
 * 描述每个对象共有的方法。
 * 类似于 C++ 中的类。
 */
struct _lv_obj_class_t {
    const lv_obj_class_t * base_class;
    /** class_p is the final class while obj->class_p is the class currently being [de]constructed.
     * class_p 是最终类，而 obj->class_p 是当前正在[析]构造的类。 */
    void (*constructor_cb)(const lv_obj_class_t * class_p, lv_obj_t * obj);
    void (*destructor_cb)(const lv_obj_class_t * class_p, lv_obj_t * obj);

    /** class_p is the class in which event is being processed.
     * class_p 是正在处理事件的类。 */
    void (*event_cb)(const lv_obj_class_t * class_p, lv_event_t * e);  /**< Widget type specific event function
                                                                        * 部件类型特有的事件函数 */

#if LV_USE_OBJ_PROPERTY
    uint32_t prop_index_start;
    uint32_t prop_index_end;
    const lv_property_ops_t * properties;
    uint32_t properties_count;

#if LV_USE_OBJ_PROPERTY_NAME
    /* An array of property ID and name
     * 属性 ID 与名称的数组 */
    const lv_property_name_t * property_names;
    uint32_t names_count;
#endif
#endif

    void * user_data;
    const char * name;
    int32_t width_def;
    int32_t height_def;
    uint32_t editable : 2;             /**< Value from ::lv_obj_class_editable_t
                                        * 值来自 ::lv_obj_class_editable_t */
    uint32_t group_def : 2;            /**< Value from ::lv_obj_class_group_def_t
                                        * 值来自 ::lv_obj_class_group_def_t */
    uint32_t instance_size : 16;
    uint32_t theme_inheritable : 1;    /**< Value from ::lv_obj_class_theme_inheritable_t
                                        * 值来自 ::lv_obj_class_theme_inheritable_t */
};


/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

void lv_obj_destruct(lv_obj_t * obj);   /**< 析构一个对象 */

/**********************
 *      MACROS
 *      宏定义
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_CLASS_PRIVATE_H*/
