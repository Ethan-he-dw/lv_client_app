/**
 * @file lv_obj_event.c
 * 本模块实现 LVGL 对象的事件系统:事件的发送与处理、事件回调的注册与移除、以及各种事件参数的获取。
 *
 */

/*********************
 *      INCLUDES
 *      包含的头文件
 *********************/
#include "../misc/lv_event_private.h"
#include "../lvgl_public.h"
#include "lv_obj_event_private.h"
#include "lv_obj_class_private.h"
#include "lv_obj_private.h"
#include "../indev/lv_indev_private.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/
#define MY_CLASS (&lv_obj_class)

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *      静态函数原型
 **********************/
static lv_result_t event_send_core(lv_event_t * e);
static bool event_is_bubbled(lv_event_t * e);
static bool event_is_trickled(lv_event_t * e);
#if LV_USE_CHECK_ARG
    static bool event_code_in_array(lv_event_code_t code, const lv_event_code_t * arr, uint32_t len);
#endif

/**********************
 *  STATIC VARIABLES
 *      静态变量
 **********************/

/**********************
 *      MACROS
 *      宏
 **********************/
#if LV_USE_LOG && LV_LOG_TRACE_EVENT
    #define LV_TRACE_EVENT(...) LV_LOG_TRACE(__VA_ARGS__)
#else
    #define LV_TRACE_EVENT(...)
#endif

/**********************
 *   GLOBAL FUNCTIONS
 *      全局函数
 **********************/

/*向对象发送一个事件:将事件压入栈后依次分发处理,最后弹出事件栈并返回处理结果。*/
lv_result_t lv_obj_send_event(lv_obj_t * obj, lv_event_code_t event_code, void * param)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);

    lv_event_t e;
    e.current_target = obj;
    e.original_target = obj;
    e.code = event_code;
    e.user_data = NULL;
    e.param = param;
    e.deleted = 0;
    e.stop_bubbling = 0;
    e.stop_processing = 0;
    e.stop_trickling = 0;

    lv_event_push(&e);

    /*Send the event*/
    /*发送事件*/
    lv_result_t res = event_send_core(&e);

    /*Remove this element from the list*/
    /*将该事件从列表中移除*/
    lv_event_pop(&e);

    return res;
}

/*在对象的基类链中查找并调用祖先类的事件处理回调。*/
lv_result_t lv_obj_event_base(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return LV_RESULT_INVALID);
    LV_CHECK_ARG(e->current_target != NULL, return LV_RESULT_INVALID);

    const lv_obj_class_t * base;
    if(class_p == NULL) base = ((lv_obj_t *)e->current_target)->class_p;
    else base = class_p->base_class;

    /*Find a base in which call the ancestor's event handler_cb if set*/
    /*向上查找设置了事件处理回调的基类*/
    while(base && base->event_cb == NULL) base = base->base_class;

    if(base == NULL) return LV_RESULT_OK;
    if(base->event_cb == NULL) return LV_RESULT_OK;

    /*Call the actual event callback*/
    /*调用实际的事件回调*/
    e->user_data = NULL;
    LV_PROFILER_EVENT_BEGIN_TAG(lv_event_code_get_name(e->code));
    base->event_cb(base, e);
    LV_PROFILER_EVENT_END_TAG(lv_event_code_get_name(e->code));

    lv_result_t res = LV_RESULT_OK;
    /*Stop if the object is deleted*/
    /*若对象已被删除则停止*/
    if(e->deleted) res = LV_RESULT_INVALID;

    return res;
}

/*为对象注册一个事件回调,返回事件描述符。*/
lv_event_dsc_t * lv_obj_add_event_cb(lv_obj_t * obj, lv_event_cb_t event_cb, lv_event_code_t filter, void * user_data)
{
    LV_CHECK_ARG(event_cb != NULL, return NULL);
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);

    if(!lv_obj_allocate_spec_attr(obj)) {
        return NULL;
    }

    return lv_event_add(&obj->spec_attr->event_list, event_cb, filter, user_data);
}

/*获取对象已注册的事件回调数量。*/
uint32_t lv_obj_get_event_count(lv_obj_t * obj)
{
    LV_CHECK_ARG(obj != NULL, return 0);
    if(obj->spec_attr == NULL) return 0;
    return lv_event_get_count(&obj->spec_attr->event_list);
}

/*按索引获取对象的事件描述符。*/
lv_event_dsc_t * lv_obj_get_event_dsc(lv_obj_t * obj, uint32_t index)
{
    LV_CHECK_ARG(obj != NULL, return NULL);
    if(obj->spec_attr == NULL) return NULL;
    return lv_event_get_dsc(&obj->spec_attr->event_list, index);
}

/*按索引移除对象的事件回调。*/
bool lv_obj_remove_event(lv_obj_t * obj, uint32_t index)
{
    LV_CHECK_ARG(obj != NULL, return false);
    if(obj->spec_attr == NULL) return false;
    return lv_event_remove(&obj->spec_attr->event_list, index);
}

/*按描述符移除对象的事件回调。*/
bool lv_obj_remove_event_dsc(lv_obj_t * obj, lv_event_dsc_t * dsc)
{
    LV_CHECK_ARG(obj != NULL, return false);
    LV_CHECK_ARG(dsc != NULL, return false);

    if(obj->spec_attr == NULL) return false;
    return lv_event_remove_dsc(&obj->spec_attr->event_list, dsc);
}

/*移除对象上与给定回调函数匹配的全部事件回调,返回移除的数量。*/
uint32_t lv_obj_remove_event_cb(lv_obj_t * obj, lv_event_cb_t event_cb)
{
    LV_CHECK_ARG(obj != NULL, return 0);

    uint32_t event_cnt = lv_obj_get_event_count(obj);
    uint32_t removed_count = 0;
    int32_t i;

    if(event_cnt == 0) return 0;

    for(i = event_cnt - 1; i >= 0; i--) {
        lv_event_dsc_t * dsc = lv_obj_get_event_dsc(obj, i);
        if(dsc && (event_cb == NULL || dsc->cb == event_cb)) {
            lv_obj_remove_event(obj, i);
            removed_count++;
        }
    }

    return removed_count;
}

/*移除回调函数与用户数据均匹配的事件回调,返回移除的数量。*/
uint32_t lv_obj_remove_event_cb_with_user_data(lv_obj_t * obj, lv_event_cb_t event_cb, void * user_data)
{
    LV_CHECK_ARG(obj != NULL, return 0);

    uint32_t event_cnt = lv_obj_get_event_count(obj);
    uint32_t removed_count = 0;
    int32_t i;

    if(event_cnt == 0) return 0;

    for(i = event_cnt - 1; i >= 0; i--) {
        lv_event_dsc_t * dsc = lv_obj_get_event_dsc(obj, i);
        if(dsc && (event_cb == NULL || dsc->cb == event_cb) && dsc->user_data == user_data) {
            lv_obj_remove_event(obj, i);
            removed_count ++;
        }
    }

    return removed_count;
}

/*获取事件的当前目标对象。*/
lv_obj_t * lv_event_get_current_target_obj(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    return lv_event_get_current_target(e);
}

/*获取事件的原始目标对象。*/
lv_obj_t * lv_event_get_target_obj(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    return lv_event_get_target(e);
}

/*获取触发该事件的输入设备。*/
lv_indev_t * lv_event_get_indev(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
#if LV_USE_CHECK_ARG
    static const lv_event_code_t indev_codes[] = {
        LV_EVENT_PRESSED,           LV_EVENT_PRESSING,          LV_EVENT_PRESS_LOST,
        LV_EVENT_SHORT_CLICKED,     LV_EVENT_LONG_PRESSED,      LV_EVENT_LONG_PRESSED_REPEAT,
        LV_EVENT_CLICKED,           LV_EVENT_RELEASED,
        LV_EVENT_SCROLL_BEGIN,      LV_EVENT_SCROLL_END,        LV_EVENT_SCROLL,
        LV_EVENT_GESTURE,           LV_EVENT_KEY,
        LV_EVENT_FOCUSED,           LV_EVENT_DEFOCUSED,         LV_EVENT_LEAVE,
        LV_EVENT_HOVER_OVER,        LV_EVENT_HOVER_LEAVE,
    };
#endif
    LV_CHECK_ARG(event_code_in_array(e->code, indev_codes, sizeof(indev_codes) / sizeof(indev_codes[0])),
                 return NULL, "invalid event code %" LV_PRId32, (int32_t)e->code);
    return lv_event_get_param(e);
}

/*获取绘制事件所属的图层。*/
lv_layer_t * lv_event_get_layer(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
#if LV_USE_CHECK_ARG
    static const lv_event_code_t draw_codes[] = {
        LV_EVENT_DRAW_MAIN,     LV_EVENT_DRAW_MAIN_BEGIN,   LV_EVENT_DRAW_MAIN_END,
        LV_EVENT_DRAW_POST,     LV_EVENT_DRAW_POST_BEGIN,   LV_EVENT_DRAW_POST_END,
    };
#endif
    LV_CHECK_ARG(event_code_in_array(e->code, draw_codes, sizeof(draw_codes) / sizeof(draw_codes[0])),
                 return NULL, "invalid event code %" LV_PRId32, (int32_t)e->code);
    return lv_event_get_param(e);
}

/*获取尺寸变更事件中对象之前的尺寸。*/
const lv_area_t * lv_event_get_old_size(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    LV_CHECK_ARG(e->code == LV_EVENT_SIZE_CHANGED, return NULL,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    return lv_event_get_param(e);
}

/*获取按键事件中的键值。*/
uint32_t lv_event_get_key(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return 0);
    LV_CHECK_ARG(e->code == LV_EVENT_KEY, return 0,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    uint32_t * k = lv_event_get_param(e);
    return k ? *k : 0;
}

/*获取编码器(旋转)事件中的增量值。*/
int32_t lv_event_get_rotary_diff(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return 0);
    LV_CHECK_ARG(e->code == LV_EVENT_ROTARY, return 0,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    int32_t * r = lv_event_get_param(e);
    return r ? *r : 0;
}

/*获取滚动开始事件中的滚动动画。*/
lv_anim_t * lv_event_get_scroll_anim(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    LV_CHECK_ARG(e->code == LV_EVENT_SCROLL_BEGIN, return NULL,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    return lv_event_get_param(e);
}

/*设置扩展绘制区域事件中的尺寸(取较大值)。*/
void lv_event_set_ext_draw_size(lv_event_t * e, int32_t size)
{
    LV_CHECK_ARG(e != NULL, return);
    LV_CHECK_ARG(e->code == LV_EVENT_REFR_EXT_DRAW_SIZE, return,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    int32_t * cur_size = lv_event_get_param(e);
    *cur_size = LV_MAX(*cur_size, size);
}

/*获取自尺寸计算事件的信息。*/
lv_point_t * lv_event_get_self_size_info(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    LV_CHECK_ARG(e->code == LV_EVENT_GET_SELF_SIZE, return NULL,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    return lv_event_get_param(e);
}

/*获取命中测试事件的信息。*/
lv_hit_test_info_t * lv_event_get_hit_test_info(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    LV_CHECK_ARG(e->code == LV_EVENT_HIT_TEST, return NULL,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    return lv_event_get_param(e);
}

/*获取遮挡检查事件中的区域。*/
const lv_area_t * lv_event_get_cover_area(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    LV_CHECK_ARG(e->code == LV_EVENT_COVER_CHECK, return NULL,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    lv_cover_check_info_t * p = lv_event_get_param(e);
    return p->area;
}

/*设置遮挡检查事件的结果(仅保存"更强"的结果)。*/
void lv_event_set_cover_res(lv_event_t * e, lv_cover_res_t res)
{
    LV_CHECK_ARG(e != NULL, return);
    LV_CHECK_ARG(e->code == LV_EVENT_COVER_CHECK, return,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    lv_cover_check_info_t * p = lv_event_get_param(e);
    if(res > p->res) p->res = res;  /*Save only "stronger" results,仅保存"更强"的结果*/
}

/*获取绘制任务添加事件中的绘制任务。*/
lv_draw_task_t * lv_event_get_draw_task(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    LV_CHECK_ARG(e->code == LV_EVENT_DRAW_TASK_ADDED, return NULL,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    return lv_event_get_param(e);
}

/*获取状态变更事件中对象之前的状态。*/
lv_state_t lv_event_get_prev_state(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return 0);
    LV_CHECK_ARG(e->code == LV_EVENT_STATE_CHANGED, return 0,
                 "invalid event code %" LV_PRId32, (int32_t)e->code);
    lv_state_t * state = lv_event_get_param(e);
    return state ? *state : 0;
}

/**********************
 *   STATIC FUNCTIONS
 *      静态函数
 **********************/

/*事件发送的核心函数:依次处理事件列表、基类回调,并按需向上冒泡或向下滴传。*/
static lv_result_t event_send_core(lv_event_t * e)
{
    LV_TRACE_EVENT("Sending event %d to %p with %p param", e->code, (void *)e->original_target, e->param);

    lv_indev_t * indev_act = lv_indev_active();
    if(indev_act) {
        if(e->stop_processing) return LV_RESULT_OK;
        if(e->deleted) return LV_RESULT_INVALID;
    }

    lv_obj_t * target = e->current_target;
    lv_result_t res = LV_RESULT_OK;
    lv_event_list_t * list = target->spec_attr ?  &target->spec_attr->event_list : NULL;

    res = lv_event_send(list, e, true);
    if(res != LV_RESULT_OK || e->stop_processing) return res;

    res = lv_obj_event_base(NULL, e);
    if(res != LV_RESULT_OK || e->stop_processing) return res;

    res = lv_event_send(list, e, false);
    if(res != LV_RESULT_OK || e->stop_processing) return res;

    lv_obj_t * parent = lv_obj_get_parent(e->current_target);
    if(parent && event_is_bubbled(e)) {
        e->current_target = parent;
        res = event_send_core(e);
    }
    if(res != LV_RESULT_OK) return res;

    /*Trickle down to children if enabled*/
    /*若启用下滴则向下滴传到子对象*/
    if(event_is_trickled(e)) {
        uint32_t child_count = lv_obj_get_child_count(target);

        /* we don't want the event to bubble up again when trickling down */
        /* 向下滴传时不希望事件再次向上冒泡 */
        e->stop_bubbling = 1;

        for(uint32_t i = 0; i < child_count && res == LV_RESULT_OK && !e->stop_processing; i++) {
            lv_obj_t * child = lv_obj_get_child(target, i);
            if(child) {
                e->current_target = child;
                res = event_send_core(e);
                if(res != LV_RESULT_OK) {
                    LV_LOG_WARN("Trickle down event %d to child %p failed", e->code, (void *)child);
                    break;
                }
            }
        }
    }

    return res;
}

/*判断事件是否需要向上冒泡到父对象。*/
static bool event_is_bubbled(lv_event_t * e)
{
    if(e->stop_bubbling) return false;

    /*Event codes that always bubble*/
    /*始终会冒泡的事件码*/
    switch(e->code) {
        case LV_EVENT_CHILD_CREATED:
        case LV_EVENT_CHILD_DELETED:
            return true;
        default:
            break;
    }

    /*Check other codes only if bubbling is enabled*/
    /*仅在启用冒泡时检查其它事件码*/
    if(lv_obj_is_event_bubble(e->current_target) == false) return false;

    switch(e->code) {
        case LV_EVENT_HIT_TEST:
        case LV_EVENT_COVER_CHECK:
        case LV_EVENT_REFR_EXT_DRAW_SIZE:
        case LV_EVENT_DRAW_MAIN_BEGIN:
        case LV_EVENT_DRAW_MAIN:
        case LV_EVENT_DRAW_MAIN_END:
        case LV_EVENT_DRAW_POST_BEGIN:
        case LV_EVENT_DRAW_POST:
        case LV_EVENT_DRAW_POST_END:
        case LV_EVENT_DRAW_TASK_ADDED:
        case LV_EVENT_REFRESH:
        case LV_EVENT_DELETE:
        case LV_EVENT_CHILD_CREATED:
        case LV_EVENT_CHILD_DELETED:
        case LV_EVENT_CHILD_CHANGED:
        case LV_EVENT_SIZE_CHANGED:
        case LV_EVENT_STYLE_CHANGED:
        case LV_EVENT_GET_SELF_SIZE:
            return false;
        default:
            return true;
    }
}

/*判断事件是否需要向下滴传到子对象。*/
static bool event_is_trickled(lv_event_t * e)
{
    if(e->stop_trickling) return false;

    /*Check other codes only if trickle is enabled*/
    /*仅在启用下滴时检查其它事件码*/
    if(lv_obj_is_event_trickle(e->current_target) == false) return false;

    switch(e->code) {
        case LV_EVENT_HIT_TEST:
        case LV_EVENT_COVER_CHECK:
        case LV_EVENT_REFR_EXT_DRAW_SIZE:
        case LV_EVENT_DRAW_MAIN_BEGIN:
        case LV_EVENT_DRAW_MAIN:
        case LV_EVENT_DRAW_MAIN_END:
        case LV_EVENT_DRAW_POST_BEGIN:
        case LV_EVENT_DRAW_POST:
        case LV_EVENT_DRAW_POST_END:
        case LV_EVENT_DRAW_TASK_ADDED:
        case LV_EVENT_REFRESH:
        case LV_EVENT_DELETE:
        case LV_EVENT_CHILD_CREATED:
        case LV_EVENT_CHILD_DELETED:
        case LV_EVENT_CHILD_CHANGED:
        case LV_EVENT_SIZE_CHANGED:
        case LV_EVENT_STYLE_CHANGED:
        case LV_EVENT_GET_SELF_SIZE:
            return false;
        default:
            return true;
    }
}

#if LV_USE_CHECK_ARG
static bool event_code_in_array(lv_event_code_t code, const lv_event_code_t * arr, uint32_t len)
{
    uint32_t i;
    for(i = 0; i < len; i++) {
        if(code == arr[i]) return true;
    }
    return false;
}
#endif
