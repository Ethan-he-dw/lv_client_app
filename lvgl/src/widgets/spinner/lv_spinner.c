/**
 * @file lv_spinner.c
 * 加载指示器控件：基于弧形(arc)控件实现的旋转加载动画。
 *
 */

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/

#include "lv_spinner_private.h"
#include "../../lvgl_public.h"
#if LV_USE_SPINNER

#include "../../misc/lv_anim_private.h"
#include "../../core/lv_obj_class_private.h"

/*Check dependencies*/
/*检查依赖关系*/
#if LV_USE_ARC == 0
    #error "lv_spinner: lv_arc is required. Enable it in lv_conf.h (LV_USE_ARC  1) "
#endif

/*Check dependencies*/
/*检查依赖关系*/
#if LV_USE_ARC == 0
    #error "lv_spinner: lv_arc is required. Enable it in lv_conf.h (LV_USE_ARC  1) "
#endif

/*********************
 *      DEFINES
 *      宏定义
 *********************/
#define DEF_ARC_ANGLE   200
#define DEF_TIME        1000
#define MY_CLASS        (&lv_spinner_class)

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数声明
 **********************/
static void lv_spinner_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void arc_anim_angles(void * obj, int32_t v);

/**********************
 *  STATIC VARIABLES
 *  静态变量
 **********************/

#if LV_USE_OBJ_PROPERTY
static const lv_property_ops_t lv_spinner_properties[] = {
    {
        .id = LV_PROPERTY_SPINNER_ANIM_DURATION,
        .setter = lv_spinner_set_anim_duration,
        .getter = lv_spinner_get_anim_duration,
    },
    {
        .id = LV_PROPERTY_SPINNER_ARC_SWEEP,
        .setter = lv_spinner_set_arc_sweep,
        .getter = lv_spinner_get_arc_sweep,
    },
};
#endif

const lv_obj_class_t lv_spinner_class = {
    .base_class = &lv_arc_class,
    .constructor_cb = lv_spinner_constructor,
    .name = "lv_spinner",
    .instance_size = sizeof(lv_spinner_t),
    LV_PROPERTY_CLASS_FIELDS(spinner, SPINNER)
};

/**********************
 *      MACROS
 *      宏
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 *   全局函数
 **********************/

/*创建加载指示器(spinner)对象，parent 为父对象*/
lv_obj_t * lv_spinner_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*设置动画时长 t 与弧线扫过的角度 angle*/
void lv_spinner_set_anim_params(lv_obj_t * obj, uint32_t t, uint32_t angle)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_spinner_t * spinner = (lv_spinner_t *)obj;

    spinner->duration = t;
    spinner->angle = angle;

    /*Delete the current animation*/
    /*删除当前动画*/
    lv_anim_delete(obj, NULL);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, arc_anim_angles);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_duration(&a, t);
    lv_anim_set_values(&a, 0, LV_BEZIER_VAL_MAX);
    lv_anim_start(&a);

    lv_arc_set_bg_angles(obj, 0, 360);
    lv_arc_set_rotation(obj, 270);
}

/*设置动画时长*/
void lv_spinner_set_anim_duration(lv_obj_t * obj, uint32_t t)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_spinner_t * spinner = (lv_spinner_t *)obj;

    lv_spinner_set_anim_params(obj, t, spinner->angle);
}

/*设置弧线扫过的角度*/
void lv_spinner_set_arc_sweep(lv_obj_t * obj, uint32_t angle)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    lv_spinner_t * spinner = (lv_spinner_t *)obj;

    lv_spinner_set_anim_params(obj, spinner->duration, angle);
}

/*获取动画时长*/
uint32_t lv_spinner_get_anim_duration(lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);
    lv_spinner_t * spinner = (lv_spinner_t *)obj;
    return spinner->duration;
}

/*获取弧线扫过的角度*/
uint32_t lv_spinner_get_arc_sweep(lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);
    lv_spinner_t * spinner = (lv_spinner_t *)obj;
    return spinner->angle;
}

/**********************
 *   STATIC FUNCTIONS
 *   静态函数
 **********************/

/*构造函数：初始化加载指示器的默认动画参数*/
static void lv_spinner_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_TRACE_OBJ_CREATE("begin");

    LV_UNUSED(class_p);

    lv_obj_set_clickable(obj, false);

    lv_spinner_set_anim_params(obj, DEF_TIME, DEF_ARC_ANGLE);
}

/*动画回调：根据动画进度 v 更新弧线的起始与结束角度*/
static void arc_anim_angles(void * obj, int32_t v)
{
    lv_spinner_t * spinner = (lv_spinner_t *)obj;

    /*Start angle (bezier)*/
    /*起始角度（贝塞尔曲线）*/
    int32_t step = lv_cubic_bezier(v, LV_BEZIER_VAL_FLOAT(0.42f), LV_BEZIER_VAL_FLOAT(0.58f),
                                   LV_BEZIER_VAL_FLOAT(0.0f), LV_BEZIER_VAL_FLOAT(1.0f));
    lv_value_precise_t start = (lv_value_precise_t)((step * 360) >> LV_BEZIER_VAL_SHIFT);

    /*End angle (linear)*/
    /*结束角度（线性）*/
    lv_value_precise_t end = spinner->angle + ((v * 360) >> LV_BEZIER_VAL_SHIFT);

    lv_arc_set_angles(obj, start, end);
}

#endif /*LV_USE_SPINNER，结束 LV_USE_SPINNER 条件编译*/
