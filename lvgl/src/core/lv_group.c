/**
 * @file lv_group.c
 * @brief 组管理模块：管理对象组及其焦点（focus）切换、编辑模式等功能。
 *
 */

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "lv_group_private.h"
#include "../core/lv_obj_private.h"
#include "../core/lv_global.h"
#include "../lvgl_public.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/
#define default_group LV_GLOBAL_DEFAULT()->group_default
#define group_ll_p &(LV_GLOBAL_DEFAULT()->group_ll)

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数原型
 **********************/
static bool focus_next_core(lv_group_t * group, void * (*begin)(const lv_ll_t *),
                            void * (*move)(const lv_ll_t *, const void *));
static void lv_group_refocus(lv_group_t * g);
static lv_indev_t * get_indev(const lv_group_t * g);

/**********************
 *  STATIC VARIABLES
 *  静态变量
 **********************/

/**********************
 *      MACROS
 *      宏
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 *   全局函数
 **********************/

/* 初始化组管理模块 */
void lv_group_init(void)
{
    lv_ll_init(group_ll_p, sizeof(lv_group_t));
}

/* 释放组管理模块占用的资源 */
void lv_group_deinit(void)
{
    lv_ll_clear(group_ll_p);
}

/* 创建一个新的组 */
lv_group_t * lv_group_create(void)
{
    lv_group_t * group = lv_ll_ins_head(group_ll_p);
    LV_ASSERT_MALLOC(group);
    if(group == NULL) return NULL;
    lv_ll_init(&group->obj_ll, sizeof(lv_obj_t *));

    group->obj_focus      = NULL;
    group->frozen         = 0;
    group->focus_cb       = NULL;
    group->edge_cb        = NULL;
    group->editing        = 0;
    group->refocus_policy = LV_GROUP_REFOCUS_POLICY_PREV;
    group->wrap           = 1;
    group->user_data      = NULL;
#if LV_USE_EXT_DATA
    group->ext_data.free_cb = NULL;
    group->ext_data.data = NULL;
#endif

    return group;
}

/* 删除一个组 */
void lv_group_delete(lv_group_t * group)
{
    if(group == NULL) return;

    /*Defocus the currently focused object*/
    /*使当前聚焦的对象失去焦点*/
    if(group->obj_focus != NULL) {
        lv_obj_send_event(*group->obj_focus, LV_EVENT_DEFOCUSED, get_indev(group));
        lv_obj_invalidate(*group->obj_focus);
    }

    /*Remove the objects from the group*/
    /*将组中的对象移除*/
    lv_obj_t ** obj;
    LV_LL_READ(&group->obj_ll, obj) {
        if((*obj)->spec_attr)(*obj)->spec_attr->group_p = NULL;
    }

    /*Remove the group from any indev devices */
    /*将该组从所有输入设备上移除*/
    lv_indev_t * indev = lv_indev_get_next(NULL);
    while(indev) {
        if(lv_indev_get_group(indev) == group) {
            lv_indev_set_group(indev, NULL);
        }
        indev = lv_indev_get_next(indev);
    }

    /*If the group is the default group, set the default group as NULL*/
    /*如果该组是默认组，则将默认组设为 NULL*/
    if(group == lv_group_get_default()) lv_group_set_default(NULL);

    lv_ll_clear(&(group->obj_ll));
    lv_ll_remove(group_ll_p, group);
#if LV_USE_EXT_DATA
    if(group->ext_data.free_cb) {
        group->ext_data.free_cb(group->ext_data.data);
        group->ext_data.data = NULL;
    }
#endif
    lv_free(group);
}

/* 设置默认组 */
void lv_group_set_default(lv_group_t * group)
{
    default_group = group;
}

/* 获取默认组 */
lv_group_t * lv_group_get_default(void)
{
    return default_group;
}

/* 将对象添加到组中 */
void lv_group_add_obj(lv_group_t * group, lv_obj_t * obj)
{
    LV_CHECK_ARG(group != NULL, return);
    LV_CHECK_ARG(obj != NULL, return);

    LV_LOG_TRACE("begin");

    if(!lv_obj_allocate_spec_attr(obj)) {
        return;
    }

    LV_ASSERT_NULL(obj->spec_attr);

    /*Be sure the object is removed from its current group*/
    /*确保对象已从其当前组中移除*/
    lv_group_remove_obj(obj);
    obj->spec_attr->group_p = group;

    lv_obj_t ** next = lv_ll_ins_tail(&group->obj_ll);
    LV_ASSERT_MALLOC(next);
    if(next == NULL) return;
    *next = obj;

    /*If the head and the tail is equal then there is only one object in the linked list.
     *In this case automatically activate it
     *如果链表头和尾相等，说明链表中只有一个对象，此时自动激活它*/
    if(lv_ll_get_head(&group->obj_ll) == next) {
        lv_group_refocus(group);
    }

    LV_LOG_TRACE("finished");
}

/* 交换组中两个对象的位置及焦点 */
void lv_group_swap_obj(lv_obj_t * obj1, lv_obj_t * obj2)
{
    LV_CHECK_ARG(obj1 != NULL, return);
    LV_CHECK_ARG(obj2 != NULL, return);

    lv_group_t * g1 = lv_obj_get_group(obj1);
    lv_group_t * g2 = lv_obj_get_group(obj2);

    if(g1 != g2) return;
    if(g1 == NULL) return;

    /*Do not add the object twice*/
    /*不要重复添加对象*/
    lv_obj_t ** obj_i;
    LV_LL_READ(&g1->obj_ll, obj_i) {
        if((*obj_i) == obj1)(*obj_i) = obj2;
        else if((*obj_i) == obj2)(*obj_i) = obj1;
    }

    /* Swap the focus as well. */
    /* 同时交换焦点。 */
    lv_obj_t * focused = lv_group_get_focused(g1);
    if(focused == obj1) lv_group_focus_obj(obj2);
    else if(focused == obj2) lv_group_focus_obj(obj1);
}

/* 将对象从其所属的组中移除 */
void lv_group_remove_obj(lv_obj_t * obj)
{
    LV_CHECK_ARG(obj != NULL, return);

    lv_group_t * g = lv_obj_get_group(obj);
    if(g == NULL) return;

    LV_LOG_TRACE("begin");

    /* If we are removing the focused object */
    /* 如果正在移除的是聚焦对象 */
    if(g->obj_focus && *g->obj_focus == obj) {
        if(g->frozen) g->frozen = 0;

        /*If there are more objects in the group, try to move focus to the next/prev object*/
        /*如果组中还有其它对象，尝试将焦点移到下一个/上一个对象*/
        if(lv_ll_get_head(&g->obj_ll) != g->obj_focus || lv_ll_get_tail(&g->obj_ll) != g->obj_focus) {
            lv_group_refocus(g);
        }
    }

    /*If the focused object is still the one being removed (either it was the only object,
     *or refocus couldn't find a focusable candidate), defocus it and clear the focus.
     *或者重新聚焦找不到可聚焦的候选对象），则使其失焦并清除焦点。*/
    if(g->obj_focus && *g->obj_focus == obj) {
        lv_obj_send_event(*g->obj_focus, LV_EVENT_DEFOCUSED, get_indev(g));
        g->obj_focus = NULL;
    }

    /*Search the object and remove it from its group*/
    /*搜索该对象并将其从所属组中移除*/
    lv_obj_t ** i;
    LV_LL_READ(&g->obj_ll, i) {
        if(*i == obj) {
            lv_ll_remove(&g->obj_ll, i);
            lv_free(i);
            if(obj->spec_attr) obj->spec_attr->group_p = NULL;
            break;
        }
    }
    LV_LOG_TRACE("finished");
}

/* 移除组中的所有对象 */
void lv_group_remove_all_objs(lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return);

    /*Defocus the currently focused object*/
    /*使当前聚焦的对象失去焦点*/
    if(group->obj_focus != NULL) {
        lv_obj_send_event(*group->obj_focus, LV_EVENT_DEFOCUSED, get_indev(group));
        lv_obj_invalidate(*group->obj_focus);
        group->obj_focus = NULL;
    }

    /*Remove the objects from the group*/
    /*将组中的对象移除*/
    lv_obj_t ** obj;
    LV_LL_READ(&group->obj_ll, obj) {
        if((*obj)->spec_attr)(*obj)->spec_attr->group_p = NULL;
    }

    lv_ll_clear(&(group->obj_ll));
}

/* 聚焦组中的指定对象 */
void lv_group_focus_obj(lv_obj_t * obj)
{
    LV_CHECK_ARG(obj != NULL, return);

    lv_group_t * g = lv_obj_get_group(obj);

    if(g == NULL) return;
    if(g->frozen) return;

    /*On defocus edit mode must be leaved*/
    /*失去焦点时必须退出编辑模式*/
    lv_group_set_editing(g, false);

    lv_obj_t ** i;
    LV_LL_READ(&g->obj_ll, i) {
        if(*i == obj) {
            if(g->obj_focus != NULL) {
                if(*g->obj_focus == obj) break;  // Refocusing same object：重新聚焦同一个对象
                lv_result_t res = lv_obj_send_event(*g->obj_focus, LV_EVENT_DEFOCUSED, get_indev(g));
                if(res != LV_RESULT_OK) return;
                lv_obj_invalidate(*g->obj_focus);
            }

            g->obj_focus = i;

            if(g->focus_cb) g->focus_cb(g);
            lv_result_t res = lv_obj_send_event(*g->obj_focus, LV_EVENT_FOCUSED, get_indev(g));
            if(res != LV_RESULT_OK) return;
            lv_obj_invalidate(*g->obj_focus);
            break;
        }
    }
}

/* 将焦点移到组中的下一个对象 */
void lv_group_focus_next(lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return);

    bool focus_changed = focus_next_core(group, lv_ll_get_head, lv_ll_get_next);

    if(!focus_changed && group->edge_cb) {
        group->edge_cb(group, true);
    }
}

/* 将焦点移到组中的上一个对象 */
void lv_group_focus_prev(lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return);

    bool focus_changed = focus_next_core(group, lv_ll_get_tail, lv_ll_get_prev);

    if(!focus_changed && group->edge_cb) {
        group->edge_cb(group, false);
    }
}

/* 冻结或解冻组的焦点 */
void lv_group_focus_freeze(lv_group_t * group, bool en)
{
    LV_CHECK_ARG(group != NULL, return);

    if(en == false) group->frozen = 0;
    else group->frozen = 1;
}

/* 向当前聚焦的对象发送数据（按键值） */
lv_result_t lv_group_send_data(lv_group_t * group, uint32_t c)
{
    LV_CHECK_ARG(group != NULL, return LV_RESULT_INVALID);

    lv_obj_t * act = lv_group_get_focused(group);
    if(act == NULL) return LV_RESULT_OK;

    if(lv_obj_has_state(act, LV_STATE_DISABLED)) return LV_RESULT_OK;

    return lv_obj_send_event(act, LV_EVENT_KEY, &c);
}

/* 设置组的焦点改变回调函数 */
void lv_group_set_focus_cb(lv_group_t * group, lv_group_focus_cb_t focus_cb)
{
    LV_CHECK_ARG(group != NULL, return);

    group->focus_cb = focus_cb;
}

/* 设置组到达边界时的回调函数 */
void lv_group_set_edge_cb(lv_group_t * group, lv_group_edge_cb_t edge_cb)
{
    LV_CHECK_ARG(group != NULL, return);

    group->edge_cb = edge_cb;
}

/* 设置组的编辑模式 */
void lv_group_set_editing(lv_group_t * group, bool edit)
{
    LV_CHECK_ARG(group != NULL, return);
    uint8_t en_val = edit ? 1 : 0;

    if(en_val == group->editing) return; /*Do not set the same mode again，不要重复设置相同的模式*/

    group->editing     = en_val;
    lv_obj_t * focused = lv_group_get_focused(group);

    if(focused) {
        lv_result_t res = lv_obj_send_event(*group->obj_focus, LV_EVENT_FOCUSED, get_indev(group));
        if(res != LV_RESULT_OK) return;

        lv_obj_invalidate(focused);
    }
}

/* 设置组的重新聚焦策略 */
void lv_group_set_refocus_policy(lv_group_t * group, lv_group_refocus_policy_t policy)
{
    LV_CHECK_ARG(group != NULL, return);
    group->refocus_policy = policy & 0x01;
}

/* 设置组在焦点到达边界时是否循环 */
void lv_group_set_wrap(lv_group_t * group, bool en)
{
    LV_CHECK_ARG(group != NULL, return);
    group->wrap = en ? 1 : 0;
}

/* 获取组当前聚焦的对象 */
lv_obj_t * lv_group_get_focused(const lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return NULL);
    if(group->obj_focus == NULL) return NULL;

    return *group->obj_focus;
}

/* 获取组的焦点改变回调函数 */
lv_group_focus_cb_t lv_group_get_focus_cb(const lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return NULL);
    return group->focus_cb;
}

/* 获取组到达边界时的回调函数 */
lv_group_edge_cb_t lv_group_get_edge_cb(const lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return NULL);
    return group->edge_cb;
}

/* 获取组当前的编辑模式状态 */
bool lv_group_get_editing(const lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return false);
    return group->editing;
}

/* 获取组在焦点到达边界时是否循环 */
bool lv_group_get_wrap(lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return false);
    return group->wrap;
}

/* 获取组中对象的数量 */
uint32_t lv_group_get_obj_count(lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return 0);
    return lv_ll_get_len(&group->obj_ll);
}

/* 按索引获取组中的对象 */
lv_obj_t * lv_group_get_obj_by_index(lv_group_t * group, uint32_t index)
{
    LV_CHECK_ARG(group != NULL, return NULL);
    uint32_t len = 0;
    lv_obj_t ** obj;

    LV_LL_READ(&group->obj_ll, obj) {
        if(len == index) {
            return *obj;
        }
        len++;
    }
    return NULL;
}

/* 获取系统中组的数量 */
uint32_t lv_group_get_count(void)
{
    return lv_ll_get_len(group_ll_p);
}

/* 按索引获取系统中的组 */
lv_group_t  * lv_group_by_index(uint32_t index)
{
    uint32_t len = 0;
    lv_group_t * group;

    LV_LL_READ_BACK(group_ll_p, group) {
        if(len == index) {
            return group;
        }
        len++;
    }

    return NULL;
}

#if LV_USE_EXT_DATA
void lv_group_set_external_data(lv_group_t * group, void * data, void (* free_cb)(void * data))
{
    LV_CHECK_ARG(group != NULL, return);

    group->ext_data.data = data;
    group->ext_data.free_cb = free_cb;
}
#endif

/* 设置组的用户数据 */
void lv_group_set_user_data(lv_group_t * group, void * user_data)
{
    LV_CHECK_ARG(group != NULL, return);
    group->user_data = user_data;
}

/* 获取组的用户数据 */
void * lv_group_get_user_data(const lv_group_t * group)
{
    LV_CHECK_ARG(group != NULL, return NULL);
    return group->user_data;
}

/**********************
 *   STATIC FUNCTIONS
 *   静态函数
 **********************/

static void lv_group_refocus(lv_group_t * g)
{
    /*Refocus must temporarily allow wrapping to work correctly*/
    /*重新聚焦时必须临时允许循环，以便正确工作*/
    uint8_t temp_wrap = g->wrap;
    g->wrap           = 1;

    if(g->refocus_policy == LV_GROUP_REFOCUS_POLICY_NEXT)
        lv_group_focus_next(g);
    else if(g->refocus_policy == LV_GROUP_REFOCUS_POLICY_PREV)
        lv_group_focus_prev(g);
    /*Restore wrap property*/
    /*恢复 wrap 属性*/
    g->wrap = temp_wrap;
}

static bool focus_next_core(lv_group_t * group, void * (*begin)(const lv_ll_t *),
                            void * (*move)(const lv_ll_t *, const void *))
{
    if(group->frozen) return false;

    bool focus_changed = false;

    lv_obj_t ** obj_next     = group->obj_focus;
    lv_obj_t ** obj_sentinel = NULL;
    bool can_move            = true;
    bool can_begin           = true;

    for(;;) {
        if(obj_next == NULL) {
            if(group->wrap || obj_sentinel == NULL) {
                if(!can_begin) return focus_changed;
                obj_next  = begin(&group->obj_ll);
                can_move  = false;
                can_begin = false;
            }
            else {
                /*Currently focused object is the last/first in the group, keep it that way*/
                /*当前聚焦的对象已是组中的最后一个/第一个，保持现状*/
                return focus_changed;
            }
        }

        if(obj_sentinel == NULL) {
            obj_sentinel = obj_next;
            if(obj_sentinel == NULL) return focus_changed; /*Group is empty，组为空*/
        }

        if(can_move) {
            obj_next = move(&group->obj_ll, obj_next);

            /*Give up if we walked the entire list and haven't found another visible object*/
            /*如果遍历了整个链表都没有找到另一个可见对象，则放弃*/
            if(obj_next == obj_sentinel) return focus_changed;
        }

        can_move = true;

        if(obj_next == NULL) continue;
        if(lv_obj_get_state(*obj_next) & LV_STATE_DISABLED) continue;

        /*Hidden objects don't receive focus.
         *If any parent is hidden, the object is also hidden)
         *如果任一父对象被隐藏，则该对象也被隐藏）*/
        lv_obj_t * parent = *obj_next;
        while(parent) {
            if(lv_obj_is_hidden(parent)) break;
            parent = lv_obj_get_parent(parent);
        }

        if(parent && lv_obj_is_hidden(parent)) continue;

        /*If we got her a good candidate is found*/
        /*走到这里说明找到了合适的候选对象*/
        break;
    }

    if(obj_next == group->obj_focus) return focus_changed; /*There's only one visible object and it's already focused，只有一个可见对象且已经聚焦*/

    if(group->obj_focus) {
        lv_result_t res = lv_obj_send_event(*group->obj_focus, LV_EVENT_DEFOCUSED, get_indev(group));
        if(res != LV_RESULT_OK) return focus_changed;
        lv_obj_invalidate(*group->obj_focus);
    }

    group->obj_focus = obj_next;

    lv_result_t res = lv_obj_send_event(*group->obj_focus, LV_EVENT_FOCUSED, get_indev(group));
    if(res != LV_RESULT_OK) return focus_changed;

    lv_obj_invalidate(*group->obj_focus);

    if(group->focus_cb) group->focus_cb(group);
    focus_changed = true;
    return focus_changed;
}

/**
 * Find an indev preferably with POINTER type (because it's the most generic) that uses the given group.
 * 查找一个使用给定组的输入设备，优先选择 POINTER 类型（因为它最通用）。
 * In other words, find an indev, that is related to the given group.
 * 换句话说，就是查找一个与该组相关的输入设备。
 * In the worst case simply return the latest indev
 * 最坏的情况下直接返回最新的输入设备。
 * @param g     a group the find in the indevs
 * @param g     要在输入设备中查找的组
 * @return      the suggested indev
 * @return      推荐的输入设备
 */
static lv_indev_t * get_indev(const lv_group_t * g)
{
    lv_indev_t * indev_guess = NULL;
    lv_indev_t * indev = lv_indev_get_next(NULL);

    while(indev) {
        lv_indev_type_t indev_type = lv_indev_get_type(indev);
        /*Prefer POINTER*/
        /*优先选择 POINTER 类型*/
        if(indev_type == LV_INDEV_TYPE_POINTER) return indev;
        if(lv_indev_get_group(indev) == g) {
            indev_guess = indev;
        }
        indev = lv_indev_get_next(indev);
    }

    return indev_guess;
}
