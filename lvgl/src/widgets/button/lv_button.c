/**
 * @file lv_button.c
 * 按钮控件：基于基础对象实现的简单可点击按钮。
 *
 */

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/

#include "lv_button_private.h"
#include "../../core/lv_obj_class_private.h"

#if LV_USE_BUTTON

/*********************
 *      DEFINES
 *      宏定义
 *********************/
#define MY_CLASS (&lv_button_class)

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数声明
 **********************/
static void lv_button_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);

/**********************
 *  STATIC VARIABLES
 *  静态变量
 **********************/
const lv_obj_class_t lv_button_class  = {
    .constructor_cb = lv_button_constructor,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_button_t),
    .base_class = &lv_obj_class,
    .name = "lv_button",
};

/**********************
 *      MACROS
 *      宏
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 *   全局函数
 **********************/

/*创建按钮对象，parent 为父对象*/
lv_obj_t * lv_button_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/**********************
 *   STATIC FUNCTIONS
 *   静态函数
 **********************/

/*构造函数：初始化按钮的默认属性*/
static void lv_button_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_obj_set_scrollable(obj, false);
    lv_obj_set_scroll_on_focus(obj, true);

    LV_TRACE_OBJ_CREATE("finished");
}

#endif
