/**
 * @file lv_obj_id_builtin.c
 * 内置对象 ID 管理模块：为对象分配、设置、释放并格式化 ID
 *
 */

/*********************
 *      INCLUDES
 *      包含的头文件
 *********************/
#include "lv_obj_class_private.h"
#include "lv_obj_private.h"
#include "lv_global.h"
#include "../lvgl_public.h"
#include "../osal/lv_os_private.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      自定义类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数原型声明
 **********************/

typedef struct _class_info_t {
    const lv_obj_class_t * class_p;
    uint32_t obj_count;
} class_info_t;

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

#if LV_USE_OBJ_ID && LV_USE_OBJ_ID_BUILTIN

/* 为对象分配内置 ID(按类统计序号) */
void lv_obj_assign_id(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_CHECK_ARG(obj != NULL, return);
    LV_CHECK_ARG(class_p != NULL, return);

    uint32_t i;
    uint32_t id = 0;
    lv_global_t * global = LV_GLOBAL_DEFAULT();
    class_info_t * info = NULL;

    LV_ASSERT_NULL(global);

    obj->id = NULL;

    for(i = 0; i < global->objid_count; i ++) {
        info = ((class_info_t *)global->objid_array) + i;
        if(class_p == info->class_p) break;
    }

    /*Resize array*/
    /*重新调整数组大小*/
    if(i == global->objid_count) {
        void * array = lv_realloc(global->objid_array, sizeof(class_info_t) * (global->objid_count + 1));
        LV_ASSERT_MALLOC(array);
        if(array == NULL) return;
        global->objid_array = array;
        global->objid_count ++;
        info = ((class_info_t *)global->objid_array) + i;
        info->obj_count = 0;
        info->class_p = class_p;
    }

    id = ++info->obj_count;

    obj->id = (void *)(lv_uintptr_t)id;
}

/* 设置对象的 ID(若已有 ID 则先释放) */
void lv_obj_set_id(lv_obj_t * obj, void * id)
{
    LV_CHECK_ARG(obj != NULL, return);
    if(obj->id) lv_obj_free_id(obj);
    obj->id = id;
}

/* 释放(清空)对象的 ID */
void lv_obj_free_id(lv_obj_t * obj)
{
    LV_CHECK_ARG(obj != NULL, return);
    obj->id = NULL;
}

/* 将对象的 ID 格式化为字符串(类名 + 序号) */
const char * lv_obj_stringify_id(lv_obj_t * obj, char * buf, uint32_t len)
{
    LV_CHECK_ARG(obj != NULL, return NULL);
    LV_CHECK_ARG(obj->class_p != NULL, return NULL);
    LV_CHECK_ARG(buf != NULL, return NULL);
    LV_CHECK_ARG(len > 0, return NULL);

    const char * name = obj->class_p->name;
    if(name == NULL) name = "nameless";

    lv_snprintf(buf, len, "%s%" LV_PRIu32 "", name, (uint32_t)(lv_uintptr_t)obj->id);
    return buf;
}

/* 销毁内置对象 ID 子系统并释放 ID 数组 */
void lv_objid_builtin_destroy(void)
{
    lv_global_t * global = LV_GLOBAL_DEFAULT();
    if(global == NULL) return;

    lv_free(global->objid_array);
    global->objid_count = 0;
}

/* 比较两个对象 ID 是否相等 */
int lv_obj_id_compare(const void * id1, const void * id2)
{
    return id1 == id2 ? 0 : 1;
}

#endif /*LV_USE_OBJ_ID_BUILTIN, LV_USE_OBJ_ID_BUILTIN 条件编译结束*/
