/**
 * @file lv_obj_class.c
 * 对象类管理模块：根据类定义创建、初始化与销毁对象
 *
 */

/*********************
 *      INCLUDES
 *      包含的头文件
 *********************/
#include "lv_obj_class_private.h"
#include "../lvgl_public.h"
#include "lv_obj_private.h"
#include "../display/lv_display_private.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/
#define MY_CLASS (&lv_obj_class)

/**********************
 *      TYPEDEFS
 *      自定义类型定义
 **********************/

/**********************
 *  GLOBAL PROTOTYPES
 *  全局函数原型声明
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数原型声明
 **********************/
static void lv_obj_construct(const lv_obj_class_t * class_p, lv_obj_t * obj);
static uint32_t get_instance_size(const lv_obj_class_t * class_p);

/**********************
 *  STATIC VARIABLES
 *  静态变量定义
 **********************/

/**********************
 *      MACROS
 *      宏定义
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 *   全局函数实现
 **********************/

/* 根据类定义创建对象(若父对象为 NULL 则创建屏幕) */
lv_obj_t * lv_obj_class_create_obj(const lv_obj_class_t * class_p, lv_obj_t * parent)
{
    LV_CHECK_ARG(class_p != NULL, return NULL);

    LV_TRACE_OBJ_CREATE("Creating object with %p class on %p parent", (void *)class_p, (void *)parent);
    uint32_t s = get_instance_size(class_p);
    lv_obj_t * obj = lv_malloc_zeroed(s);
    LV_ASSERT_MALLOC(obj);
    if(obj == NULL) return NULL;
    obj->class_p = class_p;
    obj->parent = parent;

    /*Create a screen*/
    /*创建屏幕*/
    if(parent == NULL) {
        LV_TRACE_OBJ_CREATE("creating a screen");
        lv_display_t * disp = lv_display_get_default();
        if(!disp) {
            LV_LOG_WARN("No display created yet. No place to assign the new screen");
            lv_free(obj);
            return NULL;
        }

        if(disp->screens == NULL) {
            disp->screen_cnt = 0;
        }

        lv_obj_t ** screens = lv_realloc(disp->screens, sizeof(lv_obj_t *) * (disp->screen_cnt + 1));
        LV_ASSERT_MALLOC(screens);
        if(screens == NULL) {
            LV_LOG_WARN("Failed to expand memory for screen array");
            lv_free(obj);
            return NULL;
        }

        disp->screen_cnt++;
        disp->screens = screens;
        disp->screens[disp->screen_cnt - 1] = obj;

        /*Set coordinates to full screen size*/
        /*将坐标设置为整个屏幕大小*/
        obj->coords.x1 = 0;
        obj->coords.y1 = 0;
        obj->coords.x2 = lv_display_get_horizontal_resolution(NULL) - 1;
        obj->coords.y2 = lv_display_get_vertical_resolution(NULL) - 1;
    }
    /*Create a normal object*/
    /*创建普通对象*/
    else {
        LV_TRACE_OBJ_CREATE("creating normal object");
        LV_CHECK_OBJ(parent, MY_CLASS, return NULL);

        if(!lv_obj_allocate_spec_attr(parent)) {
            lv_free(obj);
            return NULL;
        }
        if(lv_obj_add_child(parent, obj) != LV_RESULT_OK) {
            lv_free(obj);
            return NULL;
        }
    }

    return obj;
}

/* 初始化对象:应用主题、调用构造函数、刷新样式并通知父对象 */
void lv_obj_class_init_obj(lv_obj_t * obj)
{
    LV_CHECK_ARG(obj != NULL, return);

    lv_obj_mark_layout_as_dirty(obj);
    lv_obj_enable_style_refresh(false);

    lv_theme_apply(obj);
    lv_obj_construct(obj->class_p, obj);

    lv_obj_enable_style_refresh(true);
    lv_obj_refresh_style(obj, LV_PART_ANY, LV_STYLE_PROP_ANY);

    lv_obj_refresh_self_size(obj);

    lv_group_t * def_group = lv_group_get_default();
    if(def_group && lv_obj_is_group_def(obj)) {
        lv_group_add_obj(def_group, obj);
    }

    lv_obj_t * parent = lv_obj_get_parent(obj);
    if(parent) {
        /*Call the ancestor's event handler to the parent to notify it about the new child.
         *Also triggers layout update*/
        /*调用父对象祖先的事件处理器,通知它新增了子对象。
         *同时触发布局更新*/
        lv_obj_send_event(parent, LV_EVENT_CHILD_CHANGED, obj);
        lv_obj_send_event(parent, LV_EVENT_CHILD_CREATED, obj);

        /*Invalidate the area if not screen created*/
        /*若创建的并非屏幕,则使其区域无效*/
        lv_obj_invalidate(obj);
    }
}

/* 析构对象:依次调用本类与基类的析构回调 */
void lv_obj_destruct(lv_obj_t * obj)
{
    LV_CHECK_ARG(obj != NULL, return);
    LV_CHECK_ARG(obj->class_p != NULL, return);

#if LV_USE_EXT_DATA
    if(obj->ext_data.free_cb) {
        obj->ext_data.free_cb(obj->ext_data.data);
        obj->ext_data.data = NULL;
    }
#endif

    if(obj->class_p->destructor_cb) obj->class_p->destructor_cb(obj->class_p, obj);

    if(obj->class_p->base_class) {
        /*Don't let the descendant methods run during destructing the ancestor type*/
        /*析构祖先类型时,不让派生类的方法参与运行*/
        obj->class_p = obj->class_p->base_class;

        /*Call the base class's destructor too*/
        /*同时调用基类的析构函数*/
        lv_obj_destruct(obj);
    }
}

/* 判断对象是否可编辑 */
bool lv_obj_is_editable(const lv_obj_t * obj)
{
    LV_CHECK_ARG(obj != NULL, return false);

    const lv_obj_class_t * class_p = obj->class_p;

    /*Find a base in which editable is set*/
    /*查找设置了 editable 属性的基类*/
    while(class_p && class_p->editable == LV_OBJ_CLASS_EDITABLE_INHERIT) class_p = class_p->base_class;

    if(class_p == NULL) return false;

    return class_p->editable == LV_OBJ_CLASS_EDITABLE_TRUE;
}

/* 判断对象是否为组的默认对象 */
bool lv_obj_is_group_def(const lv_obj_t * obj)
{
    LV_CHECK_ARG(obj != NULL, return false);

    const lv_obj_class_t * class_p = obj->class_p;

    /*Find a base in which group_def is set*/
    /*查找设置了 group_def 属性的基类*/
    while(class_p && class_p->group_def == LV_OBJ_CLASS_GROUP_DEF_INHERIT) class_p = class_p->base_class;

    if(class_p == NULL) return false;

    return class_p->group_def == LV_OBJ_CLASS_GROUP_DEF_TRUE;
}

#if LV_USE_EXT_DATA
/* 为对象附加外部用户数据及其释放回调 */
void lv_obj_set_external_data(lv_obj_t * obj, void * data, void (* free_cb)(void * data))
{
    LV_CHECK_ARG(obj != NULL, return, "Can't attach external user data and destructor callback to a NULL object");

    obj->ext_data.data = data;
    obj->ext_data.free_cb = free_cb;
}
#endif

/**********************
 *   STATIC FUNCTIONS
 *   静态函数实现
 **********************/

/* 对象的构造函数:先构造基类,再调用本类构造函数回调 */
static void lv_obj_construct(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_ASSERT_NULL(class_p);
    LV_ASSERT_NULL(obj);
    LV_ASSERT_NULL(obj->class_p);

#if LV_USE_OBJ_NAME
    LV_ASSERT_NULL(class_p->name);
#endif

#if LV_USE_EXT_DATA
    obj->ext_data.free_cb = NULL;
    obj->ext_data.data = NULL;
#endif

    if(obj->class_p->base_class) {
        const lv_obj_class_t * original_class_p = obj->class_p;

        /*Don't let the descendant methods run during constructing the ancestor type*/
        /*构造祖先类型时,不让派生类的方法参与运行*/
        obj->class_p = obj->class_p->base_class;

        /*Construct the base first*/
        /*先构造基类*/
        lv_obj_construct(class_p, obj);

        /*Restore the original class*/
        /*恢复原来的类*/
        obj->class_p = original_class_p;
    }

    if(obj->class_p->constructor_cb) obj->class_p->constructor_cb(class_p, obj);
}

/* 获取对象的实例大小(沿基类链查找已设置实例大小的类) */
static uint32_t get_instance_size(const lv_obj_class_t * class_p)
{
    /*Find a base in which instance size is set*/
    /*查找设置了实例大小的基类*/
    while(class_p && class_p->instance_size == 0) class_p = class_p->base_class;

    LV_ASSERT(class_p != NULL); /*Never happens: set at least in `lv_obj` class, 不会发生:至少在 lv_obj 类中已设置*/

    return class_p->instance_size;
}
