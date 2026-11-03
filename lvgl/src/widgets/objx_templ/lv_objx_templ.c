/**
 * @file lv_objx_templ.c
 * 对象控件模板：新建控件时的参考模板
 *
 */

/**
 * TODO Remove these instructions
 * TODO 使用完成后移除以下说明
 * Search and replace: templ -> object short name with lower case(e.g. btn, label etc)
 *                    TEMPL -> object short name with upper case (e.g. BTN, LABEL etc.)
 * 搜索并替换: templ -> 对象短名称(小写, 如 btn、label 等)
 *                    TEMPL -> 对象短名称(大写, 如 BTN、LABEL 等)
 *
 * You can remove the defined() clause from the #if statement below. This exists because
 * LV_USE_TEMPL is not in lv_conf.h or lv_conf_template.h by default.
 * 你可以移除下方 #if 语句中的 defined() 条件;之所以保留,是因为默认情况下
 * LV_USE_TEMPL 未定义在 lv_conf.h 或 lv_conf_template.h 中。
 */

/*********************
 *      INCLUDES
 *      包含头文件
 *********************/
//#include "lv_objx_templ.h" /*TODO uncomment this：TODO 取消注释*/

#if defined(LV_USE_TEMPL) && LV_USE_TEMPL != 0

/*********************
 *      DEFINES
 *      宏定义
 *********************/
#define MY_CLASS (&lv_templ_class)

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数声明
 **********************/
static void lv_templ_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_templ_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_templ_event(const lv_obj_class_t * class_p, lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 *  静态变量
 **********************/
const lv_obj_class_t lv_templ_class = {
    .constructor_cb = lv_templ_constructor,
    .destructor_cb = lv_templ_destructor,
    .event_cb = lv_templ_event,
    .width_def = LV_DPI_DEF,
    .height_def = LV_DPI_DEF,
    .instance_size = sizeof(lv_templ_t),
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .base_class = &lv_templ_class,
    .name = "lv_templ",
};

/**********************
 *      MACROS
 *      宏
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 *   全局函数
 **********************/

/* 创建模板控件对象并返回 */
lv_obj_t * lv_templ_create(lv_obj_t * parent)
{

    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*======================
 * Add/remove functions
 * 添加/移除函数
 *=====================*/

/*
 * New object specific "add" or "remove" functions come here
 * 新对象专属的 "添加" 或 "移除" 函数放在这里
 */

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/*
 * New object specific "set" functions come here
 * 新对象专属的 "设置" 函数放在这里
 */

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/*
 * New object specific "get" functions come here
 * 新对象专属的 "获取" 函数放在这里
 */

/*=====================
 * Other functions
 * 其它函数
 *====================*/

/*
 * New object specific "other" functions come here
 * 新对象专属的 "其它" 函数放在这里
 */

/**********************
 *   STATIC FUNCTIONS
 *   静态函数
 **********************/

/* 模板控件构造函数 */
static void lv_templ_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_templ_t * templ = (lv_templ_t *)obj;
    /*Initialize the widget's data：初始化控件的数据*/

    LV_TRACE_OBJ_CREATE("finished");
}

/* 模板控件析构函数 */
static void lv_templ_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    lv_templ_t * templ = (lv_templ_t *)obj;
    /*Free the widget specific data：释放控件专属数据*/
}

/* 模板控件事件处理函数 */
static void lv_templ_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    lv_result_t res;

    /*Call the ancestor's event handler：调用父类的事件处理器*/
    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;

    /*Add the widget specific event handling here：在此添加控件专属的事件处理*/
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic：该空 typedef 仅用于消除 -Wpedantic 告警。*/
typedef int keep_pedantic_happy;
#endif
