/**
 * @file lv_obj_tree.c
 * @brief 对象树管理模块 - 负责LVGL对象的创建、删除、父子关系管理、树遍历等功能
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_obj_private.h"
#include "../lvgl_public.h"
#include "lv_obj_class_private.h"
#include "../indev/lv_indev_private.h"
#include "../display/lv_display_private.h"
#include "../misc/lv_anim_private.h"
#include "../core/lv_global.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&lv_obj_class)
#define disp_ll_p &(LV_GLOBAL_DEFAULT()->disp_ll)

#define OBJ_DUMP_STRING_LEN 128
#define LV_OBJ_NAME_MAX_LEN 128

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
/* 异步删除回调函数 */
static void lv_obj_delete_async_cb(void * obj);
/* 核心删除函数，负责递归删除对象及其子对象 */
static void obj_delete_core(lv_obj_t * obj);
/* 核心树遍历函数，递归遍历对象树并调用回调 */
static lv_obj_tree_walk_res_t walk_core(lv_obj_t * obj, lv_obj_tree_walk_cb_t cb, void * user_data);
/* 核心树转储函数，递归打印对象树结构 */
static void dump_tree_core(lv_obj_t * obj, int32_t depth);
/* 获取第一个未处于删除状态的子对象 */
static lv_obj_t * lv_obj_get_first_not_deleting_child(lv_obj_t * obj);
#if LV_USE_OBJ_NAME
    /* 根据名称在父对象的直接子对象中查找 */
    static lv_obj_t * find_by_name_direct(const lv_obj_t * parent, const char * name, size_t len);
#endif /*LV_USE_OBJ_NAME*/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief 删除一个对象（包括其所有子对象）
 * 
 * 该函数会：
 * 1. 使对象区域失效（触发重绘）
 * 2. 调用核心删除函数递归删除
 * 3. 通知父对象子对象已变更/删除
 * 4. 如果删除的是当前活动屏幕，则将活动屏幕指针置NULL
 * 
 * @param obj 要删除的对象
 */
void lv_obj_delete(lv_obj_t * obj)
{
    if(!obj) return;
    if(obj->is_deleting) return;  /* 如果对象已经在删除中，直接返回 */

    LV_LOG_TRACE("begin (delete %p)", (void *)obj);
    /* 使对象区域失效，以便后续重绘 */
    lv_obj_invalidate(obj);

    lv_obj_t * par = lv_obj_get_parent(obj);

    lv_display_t * disp = NULL;
    bool act_screen_del = false;
    if(par == NULL) {
        /* 没有父对象，说明这是一个屏幕对象 */
        disp = lv_obj_get_display(obj);
        if(!disp) return;   /*不应该发生*/
        if(disp->act_scr == obj) act_screen_del = true;  /* 标记正在删除活动屏幕 */
    }

    /* 执行核心删除逻辑 */
    obj_delete_core(obj);

    /* 通知父对象：子对象已变更和已删除 */
    if(par && !par->is_deleting) {
        lv_obj_scrollbar_invalidate(par);
        lv_obj_send_event(par, LV_EVENT_CHILD_CHANGED, NULL);
        lv_obj_send_event(par, LV_EVENT_CHILD_DELETED, NULL);
    }

    /* 如果删除的是活动屏幕，将活动屏幕指针置NULL */
    if(act_screen_del) {
        LV_LOG_WARN("the active screen was deleted");
        disp->act_scr = NULL;
    }

    LV_ASSERT_MEM_INTEGRITY();
    LV_LOG_TRACE("finished (delete %p)", (void *)obj);
}

/**
 * @brief 清空对象的所有子对象
 * 
 * 删除指定对象的所有子对象，重置滚动位置，
 * 并发送 CHILD_CHANGED 和 CHILD_DELETED 事件。
 * 
 * @param obj 要清空子对象的父对象
 */
void lv_obj_clean(lv_obj_t * obj)
{
    LV_LOG_TRACE("begin (clean %p)", (void *)obj);
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    /* 使对象区域失效 */
    lv_obj_invalidate(obj);

    uint32_t cnt = lv_obj_get_child_count(obj);
    /* 循环删除所有未处于删除状态的子对象 */
    lv_obj_t * child = lv_obj_get_first_not_deleting_child(obj);
    while(child) {
        obj_delete_core(child);
        child = lv_obj_get_first_not_deleting_child(obj);
    }
    /* 滚动到原点，移除滚动动画 */
    lv_obj_scroll_to(obj, 0, 0, LV_ANIM_OFF);
    if(obj->spec_attr) {
        obj->spec_attr->scroll.x = 0;
        obj->spec_attr->scroll.y = 0;
    }

    /* 如果确实删除了子对象，发送事件通知 */
    if(lv_obj_get_child_count(obj) < cnt) {
        lv_obj_send_event(obj, LV_EVENT_CHILD_CHANGED, NULL);
        lv_obj_send_event(obj, LV_EVENT_CHILD_DELETED, NULL);
    }

    LV_ASSERT_MEM_INTEGRITY();

    LV_LOG_TRACE("finished (clean %p)", (void *)obj);
}

/**
 * @brief 延迟删除对象
 * 
 * 通过动画机制在指定延迟后删除对象。
 * 创建一个持续时间为1ms、延迟为delay_ms的动画，
 * 动画完成时触发删除回调。
 * 
 * @param obj 要删除的对象
 * @param delay_ms 延迟时间（毫秒）
 */
void lv_obj_delete_delayed(lv_obj_t * obj, uint32_t delay_ms)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, NULL);
    lv_anim_set_duration(&a, 1);             /* 动画持续1ms */
    lv_anim_set_delay(&a, delay_ms);         /* 延迟delay_ms后开始 */
    lv_anim_set_completed_cb(&a, lv_obj_delete_anim_completed_cb);
    lv_anim_start(&a);
}

/**
 * @brief 动画完成后的删除回调
 * 
 * 当延迟删除动画完成时，此回调被调用以执行实际删除。
 * 
 * @param a 动画对象指针
 */
void lv_obj_delete_anim_completed_cb(lv_anim_t * a)
{
    LV_CHECK_ARG(a != NULL, return);

    lv_obj_delete(a->var);
}

/**
 * @brief 异步删除对象
 * 
 * 将删除操作推迟到下一个事件循环周期执行，
 * 适用于在事件回调中安全删除对象。
 * 
 * @param obj 要异步删除的对象
 */
void lv_obj_delete_async(lv_obj_t * obj)
{
    lv_async_call(lv_obj_delete_async_cb, obj);
}

/**
 * @brief 设置对象的新父对象（重新挂载对象）
 * 
 * 将对象从当前父对象移到新的父对象下。
 * 过程包括：
 * 1. 添加到新父对象的子列表
 * 2. 从旧父对象的子列表中移除
 * 3. 通知旧父对象和新父对象
 * 4. 标记布局为脏并使对象区域失效
 * 
 * 注意：屏幕对象（parent==NULL）不能更改父对象
 * 
 * @param obj 要移动的对象
 * @param parent 新的父对象
 */
void lv_obj_set_parent(lv_obj_t * obj, lv_obj_t * parent)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    LV_CHECK_OBJ(parent, MY_CLASS, return);

    /* 屏幕对象不能更改父对象 */
    if(obj->parent == NULL) {
        LV_LOG_WARN("Can't set the parent of a screen");
        return;
    }

    /* 如果新父对象和旧父对象相同，无需操作 */
    if(parent == obj->parent) {
        return;
    }

    /* 确保新父对象有特殊属性（用于存储子对象列表） */
    if(!lv_obj_allocate_spec_attr(parent)) {
        return;
    }

    /* 先将对象添加到新父对象的子列表 */
    if(lv_obj_add_child(parent, obj) != LV_RESULT_OK) {
        LV_LOG_WARN("Failed to attach child to parent");
        return;
    }

    /* 从旧父对象的子列表中移除该对象
     * 这一步不应该失败，所以放在添加到新父对象之后执行 */
    lv_obj_remove_child(obj->parent, obj);

    /* 通知旧父对象：它失去了一个子对象 */
    lv_obj_scrollbar_invalidate(obj->parent);
    lv_obj_send_event(obj->parent, LV_EVENT_CHILD_CHANGED, obj);
    lv_obj_send_event(obj->parent, LV_EVENT_CHILD_DELETED, NULL);
    obj->parent = parent;

    /* 通知新父对象：它获得了一个新子对象 */
    lv_obj_send_event(parent, LV_EVENT_CHILD_CHANGED, obj);
    lv_obj_send_event(parent, LV_EVENT_CHILD_CREATED, NULL);

    /* 标记布局需要重新计算 */
    lv_obj_mark_layout_as_dirty(obj);

    /* 使对象区域失效，触发重绘 */
    lv_obj_invalidate(obj);
}

/**
 * @brief 移动对象在父对象子列表中的位置（改变Z序）
 * 
 * 将对象移动到父对象子列表的指定索引位置。
 * 索引0是最底层（最先绘制），最后一个索引是最顶层（最后绘制）。
 * 支持负索引（-1表示最后一个位置）。
 * 
 * @param obj 要移动的对象
 * @param index 目标索引位置
 */
void lv_obj_move_to_index(lv_obj_t * obj, int32_t index)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    /* 检查父对象有效性 */
    lv_obj_t * parent = lv_obj_get_parent(obj);
    if(!parent) {
        LV_LOG_WARN("parent is NULL");
        return;
    }

    const uint32_t parent_child_count = lv_obj_get_child_count(parent);
    /* old_index 只能 >= 0，因为能到达这里说明父对象不为NULL */
    const int32_t old_index = lv_obj_get_index(obj);
    LV_ASSERT(0 <= old_index);

    /* 处理负索引：转换为正索引 */
    if(index < 0) {
        index += parent_child_count;
    }

    /* 边界检查：索引无效或未变化时直接返回 */
    if((index < 0)
       /* 索引超出子对象数量范围 */
       || (index >= (int32_t) parent_child_count)
       /* 新旧索引相同，无需移动 */
       || (index == old_index)) {

        return;
    }

    /* 通过移位操作重新排列子对象数组 */
    int32_t i = old_index;
    if(index < old_index) {
        /* 向前移动：将目标位置到原位置之间的元素后移 */
        while(i > index)  {
            parent->spec_attr->children[i] = parent->spec_attr->children[i - 1];
            i--;
        }
    }
    else {
        /* 向后移动：将原位置到目标位置之间的元素前移 */
        while(i < index) {
            parent->spec_attr->children[i] = parent->spec_attr->children[i + 1];
            i++;
        }
    }

    /* 将对象放到目标位置 */
    parent->spec_attr->children[index] = obj;
    lv_obj_send_event(parent, LV_EVENT_CHILD_CHANGED, NULL);
    lv_obj_invalidate(parent);
}

/**
 * @brief 交换两个对象的位置和父对象
 * 
 * 交换两个对象在各自父对象子列表中的位置。
 * 如果两个对象的父对象不同，还会交换它们的父对象。
 * 交换后会通知相关父对象并触发重绘。
 * 
 * @param obj1 第一个对象
 * @param obj2 第二个对象
 */
void lv_obj_swap(lv_obj_t * obj1, lv_obj_t * obj2)
{
    LV_CHECK_OBJ(obj1, MY_CLASS, return);
    LV_CHECK_OBJ(obj2, MY_CLASS, return);

    lv_obj_t * parent = lv_obj_get_parent(obj1);
    lv_obj_t * parent2 = lv_obj_get_parent(obj2);

    uint_fast32_t index1 = lv_obj_get_index(obj1);
    uint_fast32_t index2 = lv_obj_get_index(obj2);

    /* 通知父对象子对象即将被删除（临时） */
    lv_obj_send_event(parent2, LV_EVENT_CHILD_DELETED, obj2);
    lv_obj_send_event(parent, LV_EVENT_CHILD_DELETED, obj1);

    /* 在父对象的子列表中交换两个对象 */
    parent->spec_attr->children[index1] = obj2;
    obj2->parent = parent;

    parent2->spec_attr->children[index2] = obj1;
    obj1->parent = parent2;

    /* 通知父对象子对象已变更和已创建 */
    lv_obj_send_event(parent, LV_EVENT_CHILD_CHANGED, obj2);
    lv_obj_send_event(parent, LV_EVENT_CHILD_CREATED, obj2);
    lv_obj_send_event(parent2, LV_EVENT_CHILD_CHANGED, obj1);
    lv_obj_send_event(parent2, LV_EVENT_CHILD_CREATED, obj1);

    lv_obj_invalidate(parent);

    if(parent != parent2) {
        lv_obj_invalidate(parent2);
    }
    /* 同时交换输入组中的对象 */
    lv_group_swap_obj(obj1, obj2);
}

/**
 * @brief 获取对象所在的屏幕
 * 
 * 沿父对象链向上查找，直到找到没有父对象的屏幕对象。
 * 
 * @param obj 要查找的对象
 * @return 屏幕对象指针
 */
lv_obj_t * lv_obj_get_screen(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);

    const lv_obj_t * par = obj;
    const lv_obj_t * act_par;

    /* 沿父对象链向上遍历，直到找到屏幕（parent==NULL） */
    do {
        act_par = par;
        par = lv_obj_get_parent(act_par);
    } while(par != NULL);

    return (lv_obj_t *)act_par;
}

/**
 * @brief 获取对象所在的显示器
 * 
 * 首先获取对象所在的屏幕，然后在显示器链表中查找
 * 哪个显示器包含该屏幕。
 * 
 * @param obj 要查找的对象
 * @return 显示器指针，未找到返回NULL
 */
lv_display_t * lv_obj_get_display(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);

    const lv_obj_t * scr;

    if(obj->parent == NULL) scr = obj;  /* obj本身就是一个屏幕 */
    else scr = lv_obj_get_screen(obj);  /* 获取obj所在的屏幕 */

    /* 遍历所有显示器，查找包含该屏幕的显示器 */
    lv_display_t * d;
    lv_ll_t * disp_head = disp_ll_p;
    LV_LL_READ(disp_head, d) {
        uint32_t i;
        for(i = 0; i < d->screen_cnt; i++) {
            if(d->screens[i] == scr) return d;
        }
    }

    LV_LOG_WARN("No screen found");
    return NULL;
}

/**
 * @brief 获取对象的父对象
 * 
 * @param obj 要查询的对象
 * @return 父对象指针，如果是屏幕则返回NULL
 */
lv_obj_t * lv_obj_get_parent(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);

    return obj->parent;
}

/**
 * @brief 根据索引获取子对象
 * 
 * 获取指定索引位置的子对象。
 * 支持负索引：-1表示最后一个子对象，-2表示倒数第二个，以此类推。
 * 
 * @param obj 父对象
 * @param idx 子对象索引（支持负数）
 * @return 子对象指针，索引无效返回NULL
 */
lv_obj_t * lv_obj_get_child(const lv_obj_t * obj, int32_t idx)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);

    if(obj->spec_attr == NULL) return NULL;

    /* 处理负索引：转换为正索引 */
    if(idx < 0) {
        idx = obj->spec_attr->child_cnt + idx;
        if(idx < 0) return NULL;
    }

    uint32_t idu = (uint32_t) idx;

    if(idu < obj->spec_attr->child_cnt) return obj->spec_attr->children[idu];
    else return NULL;
}

/**
 * @brief 根据索引和类型获取子对象
 * 
 * 在父对象的子对象中，只计算指定类型的子对象，
 * 返回第idx个该类型的子对象。
 * 
 * @param obj 父对象
 * @param idx 类型内索引（支持负数）
 * @param class_p 要筛选的类型
 * @return 符合条件的子对象指针，未找到返回NULL
 */
lv_obj_t * lv_obj_get_child_by_type(const lv_obj_t * obj, int32_t idx, const lv_obj_class_t * class_p)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);
    LV_CHECK_ARG(class_p != NULL, return NULL);

    if(obj->spec_attr == NULL) return NULL;

    int32_t i;
    int32_t cnt = (int32_t)obj->spec_attr->child_cnt;
    if(idx >= 0) {
        /* 正向查找：从第一个子对象开始 */
        for(i = 0; i < cnt; i++) {
            if(obj->spec_attr->children[i]->class_p == class_p) {
                if(idx == 0) return obj->spec_attr->children[i];
                else idx--;
            }
        }
    }
    else {
        /* 负索引：-1表示第一个匹配的子对象（反向查找） */
        idx++;   /*-1 means the first child*/
        for(i = cnt - 1; i >= 0; i--) {
            if(obj->spec_attr->children[i]->class_p == class_p) {
                if(idx == 0) return obj->spec_attr->children[i];
                else idx++;
            }
        }
    }
    return NULL;
}

/**
 * @brief 获取兄弟对象（相对于当前对象的偏移位置）
 * 
 * 根据当前对象在父对象子列表中的位置，加上偏移量idx，
 * 返回对应的兄弟对象。
 * 
 * @param obj 当前对象
 * @param idx 偏移量（正数向后，负数向前）
 * @return 兄弟对象指针，无效则返回NULL
 */
lv_obj_t * lv_obj_get_sibling(const lv_obj_t * obj, int32_t idx)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);
    LV_CHECK_ARG(obj->parent != NULL, return NULL);

    lv_obj_t * parent = lv_obj_get_parent(obj);
    int32_t sibling_idx = (int32_t)lv_obj_get_index(obj) + idx;
    if(sibling_idx < 0) return NULL;

    return lv_obj_get_child(parent, sibling_idx);
}

/**
 * @brief 获取同类型的兄弟对象
 * 
 * 类似于lv_obj_get_sibling，但只计算指定类型的兄弟对象。
 * 
 * @param obj 当前对象
 * @param idx 类型内偏移量
 * @param class_p 要筛选的类型
 * @return 同类型的兄弟对象指针
 */
lv_obj_t * lv_obj_get_sibling_by_type(const lv_obj_t * obj, int32_t idx, const lv_obj_class_t * class_p)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);
    LV_CHECK_ARG(obj->parent != NULL, return NULL);
    LV_CHECK_ARG(class_p != NULL, return NULL);

    lv_obj_t * parent = lv_obj_get_parent(obj);
    int32_t sibling_idx = (int32_t)lv_obj_get_index_by_type(obj, class_p) + idx;
    if(sibling_idx < 0) return NULL;

    return lv_obj_get_child_by_type(parent, sibling_idx, class_p);
}

/**
 * @brief 获取子对象数量
 * 
 * @param obj 父对象
 * @return 子对象数量
 */
uint32_t lv_obj_get_child_count(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);
    if(obj->spec_attr == NULL) return 0;
    return obj->spec_attr->child_cnt;
}

/**
 * @brief 获取指定类型的子对象数量
 * 
 * 遍历所有子对象，统计属于指定类型的数量。
 * 
 * @param obj 父对象
 * @param class_p 要统计的类型
 * @return 指定类型的子对象数量
 */
uint32_t lv_obj_get_child_count_by_type(const lv_obj_t * obj, const lv_obj_class_t * class_p)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return 0);
    LV_CHECK_ARG(class_p != NULL, return 0);

    if(obj->spec_attr == NULL) return 0;

    uint32_t i;
    uint32_t cnt = 0;
    for(i = 0; i < obj->spec_attr->child_cnt; i++) {
        if(obj->spec_attr->children[i]->class_p == class_p) cnt++;
    }
    return cnt;
}

#if LV_USE_OBJ_NAME

/**
 * @brief 设置对象的名称（动态分配内存）
 * 
 * 为对象设置一个名称，名称字符串会被复制到新分配的内存中。
 * 如果之前有动态分配的名称，会先释放旧名称的内存。
 * 传入NULL会清除名称。
 * 
 * @param obj 要设置名称的对象
 * @param name 名称字符串，NULL表示清除名称
 */
void lv_obj_set_name(lv_obj_t * obj, const char * name)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);

    if(!lv_obj_allocate_spec_attr(obj)) {
        return;
    }

    /* 如果之前的名称是动态分配的，释放旧内存 */
    if(!obj->spec_attr->name_static && obj->spec_attr->name) lv_free((void *)obj->spec_attr->name);

    if(name == NULL) {
        obj->spec_attr->name = NULL;
        obj->spec_attr->name_static = 1;
    }
    else {
        obj->spec_attr->name = lv_strdup(name);  /* 复制名称字符串 */
        obj->spec_attr->name_static = 0;
    }
}

/**
 * @brief 设置对象的名称（静态字符串，不分配内存）
 * 
 * 为对象设置一个静态字符串名称，不会复制字符串。
 * 调用者需要确保字符串的生命周期覆盖对象的使用期。
 * 
 * @param obj 要设置名称的对象
 * @param name 静态名称字符串，不能为NULL
 */
void lv_obj_set_name_static(lv_obj_t * obj, const char * name)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    LV_CHECK_ARG(name != NULL, return);

    if(!lv_obj_allocate_spec_attr(obj)) {
        return;
    }
    /* 如果之前的名称是动态分配的，释放旧内存 */
    if(!obj->spec_attr->name_static && obj->spec_attr->name) lv_free((void *)obj->spec_attr->name);

    obj->spec_attr->name = name;
    obj->spec_attr->name_static = 1;
}

/**
 * @brief 获取对象的名称
 * 
 * @param obj 要查询的对象
 * @return 名称字符串指针，未设置名称返回NULL
 */
const char * lv_obj_get_name(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return NULL);

    if(obj->spec_attr == NULL) return NULL;
    else return obj->spec_attr->name;
}

/**
 * @brief 获取对象的解析后名称（处理自动索引）
 * 
 * 如果对象名称以'#'结尾，会自动为同名兄弟对象生成索引后缀。
 * 例如：名称为"btn#"的多个兄弟对象会解析为"btn0"、"btn1"、"btn2"等。
 * 如果名称以"class_name_#"形式存在，则使用类名进行匹配。
 * 
 * @param obj 要查询的对象
 * @param buf 输出缓冲区
 * @param buf_size 缓冲区大小
 */
void lv_obj_get_name_resolved(const lv_obj_t * obj, char buf[], size_t buf_size)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return);
    LV_CHECK_ARG(buf != NULL && buf_size > 0, return);

    const char * name = lv_obj_get_name(obj);
    /* 使用默认名称（带自动索引） */
    char name_buf[LV_OBJ_NAME_MAX_LEN];
    if(name == NULL) {
        lv_snprintf(name_buf, sizeof(name_buf), "%s_#", obj->class_p->name);
        name = name_buf;
    }

    size_t name_len = lv_strlen(name);
    lv_obj_t * parent = lv_obj_get_parent(obj);

    /* 如果名称以'#'结尾，自动为同名兄弟对象生成索引 */
    if(parent && name_len > 0 && name[name_len - 1] == '#') {
        uint32_t child_cnt = lv_obj_get_child_count(parent);
        uint32_t cnt = 0;
        uint32_t i;
        for(i = 0; i < child_cnt; i++) {
            lv_obj_t * child = lv_obj_get_child(parent, i);
            /* 找到当前对象，生成带索引的名称 */
            if(child == obj) {
                char num_buf[8];
                size_t num_len;
                num_len = lv_snprintf(num_buf, sizeof(num_buf), "%d", cnt);
                /* 检查缓冲区是否足够容纳名称和索引 */
                if(buf_size > name_len + num_len) {
                    /* 例如：buf = "some_name_"，去掉末尾的#并拼接数字 */
                    lv_strncpy(buf, name, name_len - 1);
                    lv_strcpy(&buf[name_len - 1], num_buf);
                }
                else {
                    /* 缓冲区不足时直接使用原始名称 */
                    lv_strlcpy(buf, obj->spec_attr->name, buf_size);
                }
                break;
            }
            /* 检查更早的兄弟对象，如果同名则计数递增 */
            else {
                const char * child_name = lv_obj_get_name(child);
                if(child_name == NULL) {
                    /* 名称格式为 <class_name># 时，用类名匹配 */
                    size_t class_name_len = lv_strlen(child->class_p->name);
                    if(name_len > 3 && class_name_len == name_len - 2 &&
                       lv_strncmp(child->class_p->name, name, class_name_len) == 0) {
                        cnt++;
                    }
                }
                /* 名称格式为 <some_name># 时，直接比较名称 */
                else {
                    if(lv_strcmp(child->spec_attr->name, name) == 0) {
                        cnt++;
                    }
                }
            }
        }
    }
    else {
        /* 名称不以'#'结尾，直接使用原始名称 */
        lv_strlcpy(buf, obj->spec_attr->name, buf_size);
    }
}

/**
 * @brief 根据路径查找子对象
 * 
 * 使用'/'分隔的路径在对象树中查找子对象。
 * 路径中的每个段需要与子对象的解析名称精确匹配。
 * 例如："panel/button" 表示查找名为"panel"的子对象下的"button"子对象。
 * 
 * @param parent 起始父对象
 * @param path '/'分隔的路径字符串
 * @return 找到的对象指针，未找到返回NULL
 */
lv_obj_t * lv_obj_get_child_by_name(const lv_obj_t * parent, const char * path)
{
    LV_CHECK_OBJ(parent, MY_CLASS, return NULL);
    LV_CHECK_ARG(path != NULL, return NULL);
    LV_CHECK_ARG(parent->spec_attr != NULL, return NULL);

    while(*path) {
        const char * segment = path;
        uint32_t len = 0;

        /* 计算当前路径段的长度（到下一个'/'或字符串结尾） */
        while(path[len] && path[len] != '/')
            len++;

        /* 在当前父对象的直接子对象中查找匹配的子对象 */
        lv_obj_t * child = find_by_name_direct(parent, segment, len);
        if(!child) return NULL; /* 未找到匹配的段 */

        /* 移动到下一个路径段 */
        path += len;
        if(*path == '/') path++; /* 跳过'/'分隔符 */

        /* 如果没有更多路径段，已找到目标对象 */
        if(*path == '\0') return child;

        /* 继续在子对象中查找下一段 */
        parent = child;
    }

    return NULL;

}

/**
 * @brief 在对象树中按名称递归查找子对象
 * 
 * 先在直接子对象中查找，如果未找到则递归搜索所有子对象的子对象。
 * 如果parent为NULL，则从当前活动屏幕开始搜索。
 * 
 * @param parent 起始搜索对象（NULL表示活动屏幕）
 * @param name 要查找的名称
 * @return 找到的对象指针，未找到返回NULL
 */
lv_obj_t * lv_obj_find_by_name(const lv_obj_t * parent, const char * name)
{
    LV_CHECK_ARG(name != NULL, return NULL);

    /* 如果未指定父对象，使用当前活动屏幕 */
    if(parent == NULL) parent = lv_display_get_screen_active(NULL);
    if(parent == NULL) return NULL;

    /* 先在直接子对象中查找 */
    lv_obj_t * child = find_by_name_direct(parent, name, UINT16_MAX);
    if(child) return child;

    /* 未找到，递归搜索所有子对象 */
    uint32_t child_cnt = lv_obj_get_child_count(parent);
    uint32_t i;
    for(i = 0; i < child_cnt; i++) {
        child = parent->spec_attr->children[i];
        lv_obj_t * found = lv_obj_find_by_name(child, name);
        if(found != NULL) return found;
    }

    return NULL;
}

#endif /*LV_USE_OBJ_NAME*/

/**
 * @brief 获取对象在父对象子列表中的索引
 * 
 * 遍历父对象的子列表，返回当前对象的索引位置。
 * 
 * @param obj 要查询的对象
 * @return 索引值（>=0），如果是屏幕则返回-1
 */
int32_t lv_obj_get_index(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return -1);

    lv_obj_t * parent = lv_obj_get_parent(obj);
    if(parent == NULL) return -1;  /* 屏幕对象没有索引 */

    uint32_t i = 0;
    for(i = 0; i < parent->spec_attr->child_cnt; i++) {
        if(parent->spec_attr->children[i] == obj) return (int32_t)i;
    }

    /* 正常情况下不应该执行到这里 */
    LV_ASSERT(0);
    return -1;
}

/**
 * @brief 获取对象在父对象子列表中指定类型内的索引
 * 
 * 只计算与指定类型相同的子对象，返回当前对象在这些同类型子对象中的索引。
 * 
 * @param obj 要查询的对象
 * @param class_p 要筛选的类型
 * @return 类型内索引值，未找到返回-1
 */
int32_t lv_obj_get_index_by_type(const lv_obj_t * obj, const lv_obj_class_t * class_p)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return -1);
    LV_CHECK_ARG(class_p != NULL, return -1);

    lv_obj_t * parent = lv_obj_get_parent(obj);
    if(parent == NULL) return 0xFFFFFFFF;

    uint32_t i = 0;
    uint32_t idx = 0;
    for(i = 0; i < parent->spec_attr->child_cnt; i++) {
        lv_obj_t * child = parent->spec_attr->children[i];
        if(child->class_p == class_p) {
            if(child == obj) return idx;
            idx++;
        }
    }

    /* 如果没有找到指定类型的子对象 */
    return -1;
}

/**
 * @brief 遍历对象树并对每个对象调用回调函数
 * 
 * 从指定对象开始，深度优先遍历对象树。
 * 回调函数可以控制遍历行为：继续、跳过子节点、或终止遍历。
 * 
 * @param start_obj 起始对象（NULL表示遍历所有屏幕）
 * @param cb 回调函数
 * @param user_data 传递给回调函数的用户数据
 */
void lv_obj_tree_walk(lv_obj_t * start_obj, lv_obj_tree_walk_cb_t cb, void * user_data)
{
    LV_CHECK_ARG(cb != NULL, return);

    walk_core(start_obj, cb, user_data);
}

/**
 * @brief 打印对象树结构（用于调试）
 * 
 * 以树形结构打印对象信息，包括对象地址和ID。
 * 如果start_obj为NULL，打印所有显示器的所有屏幕。
 * 
 * @param start_obj 起始对象（NULL表示打印所有屏幕）
 */
void lv_obj_dump_tree(lv_obj_t * start_obj)
{
    if(start_obj == NULL) {
        /* 遍历所有显示器的所有屏幕 */
        lv_display_t * disp = lv_display_get_next(NULL);
        while(disp) {
            uint32_t i;
            for(i = 0; i < disp->screen_cnt; i++) {
                dump_tree_core(disp->screens[i], 0);
            }
            disp = lv_display_get_next(disp);
        }
    }
    else {
        dump_tree_core(start_obj, 0);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief 异步删除的回调函数
 * 
 * @param obj 要删除的对象
 */
static void lv_obj_delete_async_cb(void * obj)
{
    lv_obj_delete(obj);
}

/**
 * @brief 重置输入设备状态
 * 
 * 当删除的对象正在被输入设备使用时，
 * 等待输入设备释放后重置，避免误触发其他对象的点击事件。
 * 
 * @param indev 输入设备
 * @param obj 要删除的对象
 */
static void obj_indev_reset(lv_indev_t * indev, lv_obj_t * obj)
{
    /* 如果输入设备当前处于按下状态，
     * 需要等待释放，避免误触发其他对象的点击事件 */
    if(lv_indev_get_state(indev) != LV_INDEV_STATE_RELEASED) {
        lv_indev_wait_release(indev);
    }

    /* 重置输入设备 */
    lv_indev_reset(indev, obj);
}

/**
 * @brief 对象删除的核心实现
 * 
 * 执行对象删除的实际操作：
 * 1. 标记对象为"正在删除"状态
 * 2. 发送LV_EVENT_DELETE事件，让用户释放自定义资源
 * 3. 清除所有注册的事件回调
 * 4. 递归删除所有子对象
 * 5. 重置引用该对象的所有输入设备
 * 6. 取消所有待执行的异步删除
 * 7. 调用对象析构函数
 * 8. 从屏幕列表或父对象子列表中移除
 * 9. 释放对象内存
 * 
 * @param obj 要删除的对象
 */
static void obj_delete_core(lv_obj_t * obj)
{
    if(obj->is_deleting)
        return;

    obj->is_deleting = true;

    /* 发送DELETE事件，让用户有机会释放自定义资源 */
    lv_result_t res = lv_obj_send_event(obj, LV_EVENT_DELETE, NULL);
    if(res == LV_RESULT_INVALID) {
        obj->is_deleting = false;
        return;  /* 事件处理返回无效，取消删除 */
    }

    /* 清除所有注册的事件回调 */
    if(obj->spec_attr) lv_event_remove_all(&(obj->spec_attr->event_list));

    /* 递归删除所有子对象 */
    lv_obj_t * child = lv_obj_get_child(obj, 0);
    while(child) {
        obj_delete_core(child);
        child = lv_obj_get_child(obj, 0);
    }

    lv_group_t * group = lv_obj_get_group(obj);

    /* 重置所有引用该对象的输入设备 */
    lv_indev_t * indev = lv_indev_get_next(NULL);
    while(indev) {
        lv_indev_type_t indev_type = lv_indev_get_type(indev);
        if(indev_type == LV_INDEV_TYPE_POINTER || indev_type == LV_INDEV_TYPE_BUTTON) {
            /* 如果输入设备的活动对象或滚动对象是当前删除的对象，重置 */
            if(indev->pointer.act_obj == obj || indev->pointer.scroll_obj == obj) {
                obj_indev_reset(indev, obj);
            }
            /* 清除最后按下/悬停的引用 */
            if(indev->pointer.last_pressed == obj) {
                indev->pointer.last_pressed = NULL;
            }
            if(indev->pointer.last_hovered == obj) {
                indev->pointer.last_hovered = NULL;
            }
        }

        /* 如果输入设备所属组包含该对象，也需要重置 */
        if(indev->group == group && obj == lv_indev_get_active_obj()) {
            obj_indev_reset(indev, obj);
        }
        indev = lv_indev_get_next(indev);
    }

    /* 取消所有待执行的异步删除操作 */
    lv_result_t async_cancel_res = LV_RESULT_OK;
    while(async_cancel_res == LV_RESULT_OK) {
        async_cancel_res = lv_async_call_cancel(lv_obj_delete_async_cb, obj);
    }

    /* 所有子对象已删除，清理对象特定数据（调用析构函数） */
    lv_obj_destruct(obj);

    /* 从屏幕列表中移除（如果对象是屏幕） */
    if(obj->parent == NULL) {
        lv_display_t * disp = lv_obj_get_display(obj);
        uint32_t i;
        /* 在屏幕数组中查找该屏幕 */
        for(i = 0; i < disp->screen_cnt; i++) {
            if(disp->screens[i] == obj) break;
        }

        /* 将后面的屏幕前移，覆盖被删除的位置 */
        uint32_t id = i;
        for(i = id; i < disp->screen_cnt - 1; i++) {
            disp->screens[i] = disp->screens[i + 1];
        }
        disp->screen_cnt--;
        /* 重新分配屏幕数组内存 */
        disp->screens = lv_realloc(disp->screens, disp->screen_cnt * sizeof(lv_obj_t *));
    }
    /* 从父对象的子列表中移除（如果对象不是屏幕） */
    else {
        lv_obj_remove_child(obj->parent, obj);
    }

    /* 释放对象本身的内存 */
    lv_free(obj);
}

/**
 * @brief 树遍历的核心递归实现
 * 
 * 深度优先遍历对象树，对每个对象调用回调函数。
 * 回调函数返回值决定遍历行为：
 * - LV_OBJ_TREE_WALK_NEXT: 继续遍历下一个兄弟节点
 * - LV_OBJ_TREE_WALK_SKIP_CHILDREN: 跳过当前节点的子节点
 * - LV_OBJ_TREE_WALK_END: 终止整个遍历
 * 
 * @param obj 当前遍历的对象（NULL表示遍历所有屏幕）
 * @param cb 回调函数
 * @param user_data 用户数据
 * @return 遍历结果
 */
static lv_obj_tree_walk_res_t walk_core(lv_obj_t * obj, lv_obj_tree_walk_cb_t cb, void * user_data)
{
    lv_obj_tree_walk_res_t res = LV_OBJ_TREE_WALK_NEXT;

    if(obj == NULL) {
        /* 遍历所有显示器的所有屏幕 */
        lv_display_t * disp = lv_display_get_next(NULL);
        while(disp) {
            uint32_t i;
            for(i = 0; i < disp->screen_cnt; i++) {
                walk_core(disp->screens[i], cb, user_data);
            }
            disp = lv_display_get_next(disp);
        }
        return LV_OBJ_TREE_WALK_END;    /* 返回值不重要，因为不是递归调用 */
    }

    /* 对当前对象调用回调 */
    res = cb(obj, user_data);

    if(res == LV_OBJ_TREE_WALK_END) return LV_OBJ_TREE_WALK_END;

    /* 如果不需要跳过子节点，递归遍历所有子对象 */
    if(res != LV_OBJ_TREE_WALK_SKIP_CHILDREN) {
        uint32_t i;
        for(i = 0; i < lv_obj_get_child_count(obj); i++) {
            res = walk_core(lv_obj_get_child(obj, i), cb, user_data);
            if(res == LV_OBJ_TREE_WALK_END) return LV_OBJ_TREE_WALK_END;
        }
    }
    return LV_OBJ_TREE_WALK_NEXT;
}

/**
 * @brief 树转储的核心递归实现
 * 
 * 递归打印对象树结构，使用缩进表示层级关系。
 * 每个对象打印其地址和ID（如果启用了对象ID）。
 * 
 * @param obj 当前转储的对象
 * @param depth 当前深度（用于计算缩进）
 */
static void dump_tree_core(lv_obj_t * obj, int32_t depth)
{
#if LV_USE_LOG
    const char * id;

#if LV_USE_OBJ_ID
    char buf[OBJ_DUMP_STRING_LEN];
    id = lv_obj_stringify_id(obj, buf, sizeof(buf));
    if(id == NULL) id = "obj0";
#else
    id = "obj0";
#endif

    /* obj0 的ID对于内建ID来说是无效的 */
    LV_LOG_USER("%*sobj:%p, id:%s;", (int)(2 * depth), "", (void *)obj, id);
#endif /*LV_USE_LOG*/

    /* 递归打印所有子对象 */
    if(obj && obj->spec_attr && obj->spec_attr->child_cnt) {
        for(uint32_t i = 0; i < obj->spec_attr->child_cnt; i++) {
            dump_tree_core(lv_obj_get_child(obj, i), depth + 1);
        }
    }
}

/**
 * @brief 获取第一个未处于删除状态的子对象
 * 
 * 遍历父对象的子列表，返回第一个is_deleting标志为false的子对象。
 * 用于在清空操作中迭代删除子对象。
 * 
 * @param obj 父对象
 * @return 第一个未删除的子对象，全部删除则返回NULL
 */
static lv_obj_t * lv_obj_get_first_not_deleting_child(lv_obj_t * obj)
{
    LV_ASSERT(obj != NULL);

    if(obj->spec_attr == NULL) return NULL;

    int32_t i;
    int32_t cnt = (int32_t)obj->spec_attr->child_cnt;
    for(i = 0; i < cnt; i++) {
        if(!obj->spec_attr->children[i]->is_deleting) {
            return obj->spec_attr->children[i];
        }
    }

    return NULL;
}

#if LV_USE_OBJ_NAME

/**
 * @brief 在父对象的直接子对象中按名称查找
 * 
 * 遍历父对象的所有直接子对象（不递归），
 * 比较每个子对象的解析名称与目标名称。
 * 
 * @param parent 父对象
 * @param name 要查找的名称
 * @param len 名称长度（UINT16_MAX表示使用完整字符串比较）
 * @return 找到的子对象指针，未找到返回NULL
 */
static lv_obj_t * find_by_name_direct(const lv_obj_t * parent, const char * name, size_t len)
{
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_count(parent);
    for(i = 0; i < child_cnt; i++) {
        lv_obj_t * child = parent->spec_attr->children[i];

        /* 获取子对象的解析名称（处理自动索引） */
        char child_name_resolved[LV_OBJ_NAME_MAX_LEN];
        lv_obj_get_name_resolved(child, child_name_resolved, sizeof(child_name_resolved));

        if(len == UINT16_MAX) {
            /* 完整字符串比较 */
            if(lv_strcmp(child_name_resolved, name) == 0) return child;
        }
        else {
            /* 按长度比较，同时确保完全匹配（不多不少） */
            if(lv_strncmp(child_name_resolved, name, len) == 0 &&
               child_name_resolved[len] == '\0') {
                return child;
            }
        }
    }

    return NULL;
}

#endif /*LV_USE_OBJ_NAME*/