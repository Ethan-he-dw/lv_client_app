/**
 * @file lv_3dtexture.c
 * 3D纹理(3dtexture)控件：显示 3D 纹理，支持水平/垂直翻转
 *
 */

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/

#include "lv_3dtexture_private.h"
#include "../../lvgl_public.h"
#if LV_USE_3DTEXTURE

#include "../../core/lv_obj_class_private.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

#define MY_CLASS (&lv_3dtexture_class)

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数声明
 **********************/

static void lv_3dtexture_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_3dtexture_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_3dtexture_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void draw_3dtexture(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 *  静态变量
 **********************/

const lv_obj_class_t lv_3dtexture_class = {
    .constructor_cb = lv_3dtexture_constructor,
    .destructor_cb = lv_3dtexture_destructor,
    .event_cb = lv_3dtexture_event,
    .width_def = LV_DPI_DEF,
    .height_def = LV_DPI_DEF,
    .instance_size = sizeof(lv_3dtexture_t),
    .base_class = &lv_obj_class,
    .name = "3dtexture",
};

/**********************
 *      MACROS
 *      宏
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 *   全局函数
 **********************/

/* 创建 3D 纹理对象 */
lv_obj_t * lv_3dtexture_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/* 设置 3D 纹理 ID */
void lv_3dtexture_set_src(lv_obj_t * obj, lv_3dtexture_id_t id)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    lv_3dtexture_t * tex = (lv_3dtexture_t *)obj;
    tex->id = id;
}

/* 设置水平/垂直翻转 */
void lv_3dtexture_set_flip(lv_obj_t * obj, bool h_flip, bool v_flip)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    lv_3dtexture_t * tex = (lv_3dtexture_t *)obj;

    if(tex->h_flip == h_flip && tex->v_flip == v_flip) return;

    tex->h_flip = h_flip;
    tex->v_flip = v_flip;

    lv_obj_invalidate(obj);
}

/*======================
 * Add/remove functions
 * 增删函数
 *=====================*/

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/*=====================
 * Other functions
 * 其它函数
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 *   静态函数
 **********************/

/* 3D 纹理构造函数 */
static void lv_3dtexture_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_3dtexture_t * tex = (lv_3dtexture_t *)obj;
    tex->id = LV_3DTEXTURE_ID_NULL;
    tex->h_flip = false;
    tex->v_flip = false;

    LV_TRACE_OBJ_CREATE("finished");
}

/* 3D 纹理析构函数 */
static void lv_3dtexture_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_3dtexture_t * tex = (lv_3dtexture_t *)obj;
    LV_UNUSED(tex);
}

/* 3D 纹理事件处理函数 */
static void lv_3dtexture_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    lv_result_t res;

    /*Call the ancestor's event handler*/
    /*调用父类的事件处理函数*/
    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_DRAW_MAIN) {
        draw_3dtexture(e);
    }
}

/* 绘制 3D 纹理 */
static void draw_3dtexture(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_current_target(e);
    lv_3dtexture_t * tex = (lv_3dtexture_t *)obj;
    lv_layer_t * layer = lv_event_get_layer(e);

    lv_draw_3d_dsc_t dsc;
    lv_draw_3d_dsc_init(&dsc);
    dsc.tex_id = tex->id;
    dsc.opa = lv_obj_get_style_opa(obj, LV_PART_MAIN);
    dsc.h_flip = tex->h_flip;
    dsc.v_flip = tex->v_flip;
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);
    lv_draw_3d(layer, &dsc, &coords);
}

#endif /*LV_USE_3DTEXTURE*/
