/**
 * @file lv_event.c
 * @brief 事件系统模块 - 负责LVGL事件的注册、发送、回调管理和事件生命周期处理
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_event_private.h"
#include "../core/lv_global.h"

/*********************
 *      DEFINES
 *********************/

/* 全局事件栈顶指针，用于追踪当前正在处理的事件链 */
#define event_head LV_GLOBAL_DEFAULT()->event_header
/* 全局事件最后注册ID，用于自定义事件ID的分配 */
#define event_last_id LV_GLOBAL_DEFAULT()->event_last_register_id

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/* 遍历事件列表，清理已标记为删除的事件描述符 */
static void cleanup_event_list(lv_event_list_t * list);
/* 清理事件数组的核心实现，释放已标记删除的项并压缩数组 */
static void cleanup_event_list_core(lv_array_t * array);

/* 标记事件描述符为待删除状态 */
static void event_mark_deleting(lv_event_list_t * list, lv_event_dsc_t * dsc);
/* 检查事件描述符是否已被标记为待删除 */
static bool event_is_marked_deleting(lv_event_dsc_t * dsc);
/* 获取事件列表中的事件数量 */
static uint32_t event_array_size(lv_event_list_t * list);
/* 获取事件列表中指定索引位置的事件描述符指针 */
static lv_event_dsc_t ** event_array_at(lv_event_list_t * list, uint32_t index);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/* 事件跟踪日志宏，仅在启用LV_USE_LOG和LV_LOG_TRACE_EVENT时生效 */
#if LV_USE_LOG && LV_LOG_TRACE_EVENT
    #define LV_TRACE_EVENT(...) LV_LOG_TRACE(__VA_ARGS__)
#else
    #define LV_TRACE_EVENT(...)
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

#if LV_USE_EXT_DATA
/**
 * @brief 设置事件描述符的外部数据
 * 
 * 为事件描述符关联外部数据和对应的释放回调函数。
 * 外部数据可用于在事件处理中传递额外的上下文信息。
 * 
 * @param dsc 事件描述符
 * @param data 外部数据指针
 * @param free_cb 释放外部数据的回调函数
 */
void lv_event_desc_set_external_data(lv_event_dsc_t * dsc, void * data, void (* free_cb)(void * data))
{
    LV_CHECK_ARG(dsc != NULL, return);

    dsc->ext_data.data = data;
    dsc->ext_data.free_cb = free_cb;
}
#endif

/**
 * @brief 将事件压入事件栈
 * 
 * 构建一个链表结构来跟踪当前正在处理的事件。
 * 这对于检测嵌套事件中对象是否被删除非常重要。
 * 例如：在一个事件回调中触发了另一个事件，
 * 而该事件又删除了最初触发事件的对象。
 * 
 * @param e 要压入的事件对象
 */
void lv_event_push(lv_event_t * e)
{
    /* 从事件对象构建一个简单的链表
     * 这对于了解对象是否被嵌套事件删除非常重要
     * （嵌套事件是从当前event_cb中调用的）*/
    e->prev = event_head;
    event_head = e;

}

/**
 * @brief 将事件从事件栈中弹出
 * 
 * 将事件栈顶指针移回上一个事件。
 * 
 * @param e 要弹出的事件对象
 */
void lv_event_pop(lv_event_t * e)
{
    event_head = e->prev;
}

/**
 * @brief 构建事件并发送（先预处理，再正常处理）
 * 
 * 创建一个事件对象，将其压入事件栈，然后按两个阶段发送：
 * 1. 先发送给带有 LV_EVENT_PREPROCESS 标志的处理器（预处理阶段）
 * 2. 再发送给不带该标志的处理器（正常处理阶段）
 * 
 * @param event_list 事件列表
 * @param code 事件代码
 * @param original_target 事件的原始目标对象
 * @param param 事件参数
 * @return 处理结果，LV_RESULT_OK表示成功
 */
lv_result_t lv_event_push_and_send(lv_event_list_t * event_list, lv_event_code_t code, void * original_target,
                                   void * param)
{
    LV_ASSERT_NULL(event_list);
    lv_event_t e;
    lv_memzero(&e, sizeof(e));
    e.code = code;                      /* 设置事件代码 */
    e.current_target = original_target; /* 当前目标（初始为原始目标） */
    e.original_target = original_target;/* 原始目标 */
    e.param = param;                    /* 事件参数 */

    lv_event_push(&e);
    /* 第一阶段：发送给预处理（PREPROCESS）回调 */
    lv_result_t res = lv_event_send(event_list, &e, true);
    if(res != LV_RESULT_OK) goto ret;

    /* 第二阶段：发送给正常（非PREPROCESS）回调 */
    res = lv_event_send(event_list, &e, false);
    if(res != LV_RESULT_OK) goto ret;

ret:
    lv_event_pop(&e);
    return res;
}

/**
 * @brief 向事件列表发送事件
 * 
 * 遍历事件列表中的所有事件描述符，对匹配的回调函数进行调用。
 * 
 * 支持两种模式：
 * - preprocess=true: 只调用带有 LV_EVENT_PREPROCESS 标志的回调
 * - preprocess=false: 只调用不带该标志的回调
 * 
 * 处理嵌套事件删除的安全机制：
 * - 备份数组头部，防止在事件回调中删除对象导致内存泄漏
 * - 使用 is_traversing 标志防止嵌套遍历时重复清理
 * - 跳过已标记为删除的事件描述符
 * 
 * @param list 事件列表
 * @param e 事件对象
 * @param preprocess true=预处理阶段，false=正常处理阶段
 * @return 处理结果
 */
lv_result_t lv_event_send(lv_event_list_t * list, lv_event_t * e, bool preprocess)
{
    if(list == NULL) return LV_RESULT_OK;
    LV_CHECK_ARG(e != NULL, return LV_RESULT_INVALID);
    if(e->deleted) return LV_RESULT_INVALID;

    /* 当对象在自己的事件中被删除时，会导致 list->array 的头部被释放，
     * 但内容仍然存在，从而造成内存泄漏。
     * 因此预先备份头部信息，尽量释放内存并防止使用已释放的内存。 */
    lv_array_t back_array_head = list->array;

    /* 处理嵌套事件删除的问题：标记当前正在遍历 */
    const bool is_traversing = list->is_traversing;
    list->is_traversing = true;

    lv_result_t res = LV_RESULT_OK;
    const uint32_t size = event_array_size(list);
    for(uint32_t i = 0; i < size && !e->deleted; i++) {
        lv_event_dsc_t * dsc = *event_array_at(list, i);
        if(dsc->cb == NULL) continue;                        /* 跳过空回调 */
        if(event_is_marked_deleting(dsc)) continue;          /* 跳过已标记删除的 */
        const bool is_preprocessed = (dsc->filter & LV_EVENT_PREPROCESS) != 0;
        if(is_preprocessed != preprocess) continue;          /* 不匹配当前处理阶段则跳过 */
        lv_event_code_t filter = dsc->filter & ~LV_EVENT_PREPROCESS;
        if(filter == LV_EVENT_ALL || filter == e->code) {
            /* 事件代码匹配（LV_EVENT_ALL匹配所有事件） */
            e->user_data = dsc->user_data;  /* 设置用户数据 */
#if LV_USE_EXT_DATA
            e->ext_data.data = dsc->ext_data.data;  /* 设置外部数据 */
#endif
            dsc->cb(e);                     /* 调用回调函数 */
            if(e->stop_processing) break;   /* 如果设置了停止处理标志，退出循环 */

            /* 如果对象在回调中被删除，停止处理 */
            if(e->deleted) {
                res = LV_RESULT_INVALID;
                break;
            }
        }
    }

    /* 如果是嵌套调用（外层还在遍历），直接返回，不执行清理 */
    if(is_traversing) return res;

    /* 非嵌套调用时执行清理 */
    if(e->deleted) cleanup_event_list_core(&back_array_head);
    else {
        list->is_traversing = false;
        cleanup_event_list(list);
    }

    return res;
}

/**
 * @brief 向事件列表添加一个新的事件处理器
 * 
 * 创建一个新的事件描述符，包含回调函数、事件过滤器和用户数据，
 * 并将其添加到事件列表中。
 * 
 * @param list 事件列表
 * @param cb 事件回调函数
 * @param filter 事件过滤器（可以是事件代码 | LV_EVENT_PREPROCESS）
 * @param user_data 传递给回调函数的用户数据
 * @return 新创建的事件描述符指针
 */
lv_event_dsc_t * lv_event_add(lv_event_list_t * list, lv_event_cb_t cb, lv_event_code_t filter,
                              void * user_data)
{
    LV_CHECK_ARG(list != NULL, return NULL);
    lv_event_dsc_t * dsc = lv_malloc(sizeof(lv_event_dsc_t));
    LV_ASSERT_NULL(dsc);

    dsc->cb = cb;
    dsc->filter = filter;
    dsc->user_data = user_data;
#if LV_USE_EXT_DATA
    dsc->ext_data.free_cb = NULL;
    dsc->ext_data.data = NULL;
#endif

    if(event_array_size(list) == 0) {
        /* 事件列表尚未初始化，初始化数组（初始容量1，元素大小为指针大小） */
        lv_array_init(&list->array, 1, sizeof(lv_event_dsc_t *));
    }

    lv_array_push_back(&list->array, &dsc);
    return dsc;
}

/**
 * @brief 从事件列表中移除指定的事件描述符
 * 
 * 查找并移除指定的事件描述符。如果描述符在当前遍历过程中被移除，
 * 会先标记为待删除，在遍历结束后才真正释放内存。
 * 如果启用了外部数据，会先调用释放回调。
 * 
 * @param list 事件列表
 * @param dsc 要移除的事件描述符
 * @return true=成功移除，false=未找到
 */
bool lv_event_remove_dsc(lv_event_list_t * list, lv_event_dsc_t * dsc)
{
    LV_CHECK_ARG(list != NULL, return false);
    LV_CHECK_ARG(dsc != NULL, return false);

    const uint32_t size = event_array_size(list);
    for(uint32_t i = 0; i < size; i++) {
        lv_event_dsc_t * event = *event_array_at(list, i);
        if(event == dsc) {
#if LV_USE_EXT_DATA
            /* 释放外部数据 */
            if(dsc->ext_data.free_cb) {
                dsc->ext_data.free_cb(dsc->ext_data.data);
                dsc->ext_data.data = NULL;
            }
#endif
            event_mark_deleting(list, event);   /* 标记为待删除 */
            cleanup_event_list(list);           /* 尝试清理（如果不在遍历中） */
            return true;
        }
    }

    return false;
}

/**
 * @brief 获取事件列表中的事件描述符数量
 * 
 * @param list 事件列表
 * @return 事件描述符数量
 */
uint32_t lv_event_get_count(lv_event_list_t * list)
{
    LV_CHECK_ARG(list != NULL, return 0);
    return event_array_size(list);
}

/**
 * @brief 获取事件列表中指定索引的事件描述符
 * 
 * @param list 事件列表
 * @param index 索引位置
 * @return 事件描述符指针，索引无效返回NULL
 */
lv_event_dsc_t * lv_event_get_dsc(lv_event_list_t * list, uint32_t index)
{
    LV_CHECK_ARG(list != NULL, return NULL);
    lv_event_dsc_t ** dsc = event_array_at(list, index);
    return dsc ? *dsc : NULL;
}

/**
 * @brief 获取事件描述符的回调函数
 * 
 * @param dsc 事件描述符
 * @return 回调函数指针
 */
lv_event_cb_t lv_event_dsc_get_cb(lv_event_dsc_t * dsc)
{
    LV_CHECK_ARG(dsc != NULL, return NULL);
    return dsc->cb;
}

/**
 * @brief 获取事件描述符的用户数据
 * 
 * @param dsc 事件描述符
 * @return 用户数据指针
 */
void * lv_event_dsc_get_user_data(lv_event_dsc_t * dsc)
{
    LV_CHECK_ARG(dsc != NULL, return NULL);
    return dsc->user_data;

}

/**
 * @brief 按索引移除事件列表中的事件描述符
 * 
 * 获取指定索引的事件描述符并移除。
 * 如果启用了外部数据，会先调用释放回调。
 * 
 * @param list 事件列表
 * @param index 要移除的事件描述符索引
 * @return true=成功移除，false=索引无效
 */
bool lv_event_remove(lv_event_list_t * list, uint32_t index)
{
    LV_CHECK_ARG(list != NULL, return false);
    lv_event_dsc_t * dsc = lv_event_get_dsc(list, index);
    if(dsc == NULL) return false;
#if LV_USE_EXT_DATA
    /* 释放外部数据 */
    if(dsc->ext_data.free_cb) {
        dsc->ext_data.free_cb(dsc->ext_data.data);
        dsc->ext_data.data = NULL;
    }
#endif
    event_mark_deleting(list, dsc);
    cleanup_event_list(list);
    return true;
}

/**
 * @brief 移除事件列表中的所有事件描述符
 * 
 * 将所有事件描述符标记为待删除，然后执行清理。
 * 如果启用了外部数据，会先为每个描述符调用释放回调。
 * 
 * @param list 事件列表
 */
void lv_event_remove_all(lv_event_list_t * list)
{
    LV_CHECK_ARG(list != NULL, return);
    const uint32_t size = event_array_size(list);
    for(uint32_t i = 0; i < size; i++) {
#if LV_USE_EXT_DATA
        lv_event_dsc_t * dsc = lv_event_get_dsc(list, i);
        if(dsc && dsc->ext_data.free_cb) {
            dsc->ext_data.free_cb(dsc->ext_data.data);
            dsc->ext_data.data = NULL;
        }
#endif
        event_mark_deleting(list, *event_array_at(list, i));
    }

    cleanup_event_list(list);
}

/**
 * @brief 获取事件的当前目标对象
 * 
 * current_target 是事件当前正在被处理的对象。
 * 在事件冒泡过程中，current_target 会逐级变化，
 * 而 original_target 始终是最初触发事件的对象。
 * 
 * @param e 事件对象
 * @return 当前目标对象指针
 */
void * lv_event_get_current_target(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    return e->current_target;
}

/**
 * @brief 获取事件的原始目标对象
 * 
 * original_target 是最初触发事件的对象，不会随冒泡而改变。
 * 
 * @param e 事件对象
 * @return 原始目标对象指针
 */
void * lv_event_get_target(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    return e->original_target;
}

/**
 * @brief 获取事件代码
 * 
 * 返回去掉预处理标志（LV_EVENT_PREPROCESS）后的事件代码。
 * 
 * @param e 事件对象
 * @return 事件代码
 */
lv_event_code_t lv_event_get_code(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return 0);
    return e->code & ~LV_EVENT_PREPROCESS;
}

/**
 * @brief 获取事件的参数
 * 
 * 参数的含义取决于具体的事件类型。
 * 例如：LV_EVENT_VALUE_CHANGED 事件中，参数通常包含新值。
 * 
 * @param e 事件对象
 * @return 事件参数指针
 */
void * lv_event_get_param(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    return e->param;
}

/**
 * @brief 获取事件的用户数据
 * 
 * 用户数据是在注册事件回调时通过 lv_event_add() 设置的。
 * 
 * @param e 事件对象
 * @return 用户数据指针
 */
void * lv_event_get_user_data(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return NULL);
    return e->user_data;
}

/**
 * @brief 停止事件冒泡
 * 
 * 设置此标志后，事件将不再向父对象冒泡传播。
 * 冒泡：事件从原始目标逐级向父对象传播的过程。
 * 
 * @param e 事件对象
 */
void lv_event_stop_bubbling(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return);
    e->stop_bubbling = 1;
}

/**
 * @brief 停止事件滴流（向下传播）
 * 
 * 设置此标志后，事件将不再向子对象滴流传播。
 * 滴流：事件从父对象逐级向子对象传播的过程。
 * 
 * @param e 事件对象
 */
void lv_event_stop_trickling(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return);
    e->stop_trickling = 1;
}

/**
 * @brief 停止事件处理
 * 
 * 设置此标志后，当前事件列表中的后续回调函数将不再被调用。
 * 
 * @param e 事件对象
 */
void lv_event_stop_processing(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return);
    e->stop_processing = 1;
}

/**
 * @brief 释放用户数据的回调函数
 * 
 * 这是一个预定义的回调，用于自动释放事件的user_data指向的内存。
 * 可以在注册事件时作为free_cb使用。
 * 
 * @param e 事件对象
 */
void lv_event_free_user_data_cb(lv_event_t * e)
{
    LV_CHECK_ARG(e != NULL, return);
    void * p = lv_event_get_user_data(e);
    lv_free(p);
}

/**
 * @brief 注册一个新的自定义事件ID
 * 
 * 返回一个全局唯一递增的事件ID，用于自定义事件。
 * 自定义事件ID从 LV_EVENT_LAST 之后开始分配。
 * 
 * @return 新的事件ID
 */
uint32_t lv_event_register_id(void)
{
    event_last_id ++;
    return event_last_id;
}

/**
 * @brief 标记指定目标对象相关的事件为已删除
 * 
 * 遍历当前事件栈，将所有 original_target 或 current_target
 * 匹配的事件标记为已删除（deleted=1）。
 * 这用于在对象被删除时，通知正在处理的事件停止进一步操作。
 * 
 * @param target 要标记为已删除的目标对象
 */
void lv_event_mark_deleted(void * target)
{
    lv_event_t * e = event_head;

    while(e) {
        if(e->original_target == target || e->current_target == target) e->deleted = 1;
        e = e->prev;
    }
}

/**
 * @brief 获取事件代码的名称字符串
 * 
 * 将事件代码转换为可读的字符串名称，用于调试和日志输出。
 * 例如：LV_EVENT_PRESSED -> "EVENT_PRESSED"
 * 
 * @param code 事件代码
 * @return 事件名称字符串，未知事件返回"EVENT_UNKNOWN"
 */
const char * lv_event_code_get_name(lv_event_code_t code)
{
    /* 去除预处理标志 */
    code &= ~LV_EVENT_PREPROCESS;

/* 宏：将事件代码转换为 case 语句和名称字符串 */
#define ENUM_CASE(x) case LV_##x: return #x

    switch(code) {
            ENUM_CASE(EVENT_ALL);

            /** 输入设备事件 */
            ENUM_CASE(EVENT_PRESSED);              /* 按下事件 */
            ENUM_CASE(EVENT_PRESSING);             /* 按住事件（持续触发） */
            ENUM_CASE(EVENT_PRESS_LOST);           /* 按下丢失事件（按下后移出对象区域） */
            ENUM_CASE(EVENT_SHORT_CLICKED);        /* 短按事件 */
            ENUM_CASE(EVENT_SINGLE_CLICKED);       /* 单击事件 */
            ENUM_CASE(EVENT_DOUBLE_CLICKED);       /* 双击事件 */
            ENUM_CASE(EVENT_TRIPLE_CLICKED);       /* 三击事件 */
            ENUM_CASE(EVENT_LONG_PRESSED);         /* 长按事件 */
            ENUM_CASE(EVENT_LONG_PRESSED_REPEAT);  /* 长按重复事件 */
            ENUM_CASE(EVENT_CLICKED);              /* 点击事件（按下并在同一对象上释放） */
            ENUM_CASE(EVENT_RELEASED);             /* 释放事件 */
            ENUM_CASE(EVENT_SCROLL_BEGIN);         /* 滚动开始 */
            ENUM_CASE(EVENT_SCROLL_THROW_BEGIN);   /* 惯性滚动开始 */
            ENUM_CASE(EVENT_SCROLL_END);           /* 滚动结束 */
            ENUM_CASE(EVENT_SCROLL);               /* 滚动中 */
            ENUM_CASE(EVENT_GESTURE);              /* 手势事件 */
            ENUM_CASE(EVENT_KEY);                  /* 按键事件 */
            ENUM_CASE(EVENT_ROTARY);               /* 旋转编码器事件 */
            ENUM_CASE(EVENT_FOCUSED);              /* 获得焦点 */
            ENUM_CASE(EVENT_DEFOCUSED);            /* 失去焦点 */
            ENUM_CASE(EVENT_LEAVE);                /* 离开对象 */
            ENUM_CASE(EVENT_HIT_TEST);             /* 命中测试 */
            ENUM_CASE(EVENT_INDEV_RESET);          /* 输入设备重置 */
            ENUM_CASE(EVENT_HOVER_OVER);           /* 鼠标悬停进入 */
            ENUM_CASE(EVENT_HOVER_LEAVE);          /* 鼠标悬停离开 */

            /** 绘制事件 */
            ENUM_CASE(EVENT_COVER_CHECK);          /* 覆盖检查（判断对象是否完全覆盖其区域） */
            ENUM_CASE(EVENT_REFR_EXT_DRAW_SIZE);   /* 刷新外部绘制大小 */
            ENUM_CASE(EVENT_DRAW_MAIN_BEGIN);      /* 主绘制开始 */
            ENUM_CASE(EVENT_DRAW_MAIN);            /* 主绘制 */
            ENUM_CASE(EVENT_DRAW_MAIN_END);        /* 主绘制结束 */
            ENUM_CASE(EVENT_DRAW_POST_BEGIN);      /* 后置绘制开始 */
            ENUM_CASE(EVENT_DRAW_POST);            /* 后置绘制 */
            ENUM_CASE(EVENT_DRAW_POST_END);        /* 后置绘制结束 */
            ENUM_CASE(EVENT_DRAW_TASK_ADDED);      /* 绘制任务已添加 */

            /** 特殊事件 */
            ENUM_CASE(EVENT_VALUE_CHANGED);        /* 值变化 */
            ENUM_CASE(EVENT_INSERT);               /* 文本插入 */
            ENUM_CASE(EVENT_REFRESH);              /* 刷新 */
            ENUM_CASE(EVENT_READY);                /* 就绪 */
            ENUM_CASE(EVENT_CANCEL);               /* 取消 */
            ENUM_CASE(EVENT_STATE_CHANGED);        /* 状态变化 */

            /** 其他事件 */
            ENUM_CASE(EVENT_CREATE);               /* 对象创建 */
            ENUM_CASE(EVENT_DELETE);               /* 对象删除 */
            ENUM_CASE(EVENT_CHILD_CHANGED);        /* 子对象变更 */
            ENUM_CASE(EVENT_CHILD_CREATED);        /* 子对象创建 */
            ENUM_CASE(EVENT_CHILD_DELETED);        /* 子对象删除 */
            ENUM_CASE(EVENT_SCREEN_UNLOAD_START);  /* 屏幕开始卸载 */
            ENUM_CASE(EVENT_SCREEN_LOAD_START);    /* 屏幕开始加载 */
            ENUM_CASE(EVENT_SCREEN_LOADED);        /* 屏幕加载完成 */
            ENUM_CASE(EVENT_SCREEN_UNLOADED);      /* 屏幕卸载完成 */
            ENUM_CASE(EVENT_SIZE_CHANGED);         /* 大小变化 */
            ENUM_CASE(EVENT_STYLE_CHANGED);        /* 样式变化 */
            ENUM_CASE(EVENT_LAYOUT_CHANGED);       /* 布局变化 */
            ENUM_CASE(EVENT_GET_SELF_SIZE);        /* 获取自身大小 */
            ENUM_CASE(EVENT_UPDATE_LAYOUT_COMPLETED); /* 布局更新完成 */

            /** 可选LVGL组件的事件 */
            ENUM_CASE(EVENT_INVALIDATE_AREA);      /* 使区域失效 */
            ENUM_CASE(EVENT_RESOLUTION_CHANGED);   /* 分辨率变化 */
            ENUM_CASE(EVENT_COLOR_FORMAT_CHANGED); /* 颜色格式变化 */
            ENUM_CASE(EVENT_REFR_REQUEST);         /* 刷新请求 */
            ENUM_CASE(EVENT_REFR_START);           /* 刷新开始 */
            ENUM_CASE(EVENT_REFR_READY);           /* 刷新就绪 */
            ENUM_CASE(EVENT_RENDER_START);         /* 渲染开始 */
            ENUM_CASE(EVENT_RENDER_READY);         /* 渲染就绪 */
            ENUM_CASE(EVENT_FLUSH_START);          /* 缓冲区刷新开始 */
            ENUM_CASE(EVENT_FLUSH_FINISH);         /* 缓冲区刷新完成 */
            ENUM_CASE(EVENT_FLUSH_WAIT_START);     /* 等待刷新开始 */
            ENUM_CASE(EVENT_FLUSH_WAIT_FINISH);    /* 等待刷新完成 */
            ENUM_CASE(EVENT_SYNC_START);           /* 同步开始 */
            ENUM_CASE(EVENT_SYNC_FINISH);          /* 同步完成 */
            ENUM_CASE(EVENT_SYNC_WAIT_START);      /* 等待同步开始 */
            ENUM_CASE(EVENT_SYNC_WAIT_FINISH);     /* 等待同步完成 */

            ENUM_CASE(EVENT_VSYNC);                /* 垂直同步 */
            ENUM_CASE(EVENT_VSYNC_REQUEST);        /* 垂直同步请求 */

#if LV_USE_TRANSLATION
            ENUM_CASE(EVENT_TRANSLATION_LANGUAGE_CHANGED); /* 翻译语言变化 */
#endif /*LV_USE_TRANSLATION*/

        /* 特殊事件标志（不是真正的事件代码，不应被发送） */
        case LV_EVENT_LAST:              /* 事件代码最大值标记 */
        case LV_EVENT_PREPROCESS:        /* 预处理标志 */
        case LV_EVENT_MARKED_DELETING:   /* 标记删除标志 */
            break;

            /* 注意：这里不添加default分支，这样当添加新的事件代码时，
             * 如果忘记添加case，编译器会自动报出警告。
             */
    }

#undef ENUM_CASE

    return "EVENT_UNKNOWN";
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief 清理事件数组的核心实现
 * 
 * 遍历事件数组，释放所有已标记为删除的事件描述符的内存，
 * 将未删除的元素紧凑排列到数组前面，然后调整数组大小。
 * 如果所有元素都被删除，则反初始化数组。
 * 
 * @param array 事件描述符指针数组
 */
static void cleanup_event_list_core(lv_array_t * array)
{
    const uint32_t size = lv_array_size(array);
    uint32_t kept_count = 0;  /* 保留的元素计数 */
    for(uint32_t i = 0; i < size; i++) {
        lv_event_dsc_t ** dsc_i = lv_array_at(array, i);
        lv_event_dsc_t ** dsc_kept = lv_array_at(array, kept_count);
        if(event_is_marked_deleting(*dsc_i)) lv_free(*dsc_i);  /* 释放已标记删除的 */
        else {
            *dsc_kept = *dsc_i;  /* 将保留的元素前移 */
            kept_count++;
        }
    }

    if(kept_count == 0) lv_array_deinit(array);  /* 全部删除，反初始化数组 */
    else lv_array_resize(array, kept_count);     /* 调整数组大小 */
}

/**
 * @brief 清理事件列表中的已标记删除项
 * 
 * 如果当前正在遍历事件列表，则延迟清理（避免在遍历中修改列表）。
 * 如果没有已标记删除的项，直接返回。
 * 
 * @param list 事件列表
 */
static void cleanup_event_list(lv_event_list_t * list)
{
    if(list->is_traversing) return;             /* 正在遍历，延迟清理 */
    if(list->has_marked_deleting == false) return;  /* 没有需要清理的 */

    cleanup_event_list_core(&list->array);

    list->has_marked_deleting = false;
}

/**
 * @brief 标记事件描述符为待删除状态
 * 
 * 在事件描述符的filter字段中设置 LV_EVENT_MARKED_DELETING 标志位，
 * 并标记列表中有待删除项。
 * 使用延迟删除策略，避免在遍历事件列表时修改列表导致崩溃。
 * 
 * @param list 事件列表
 * @param dsc 要标记的事件描述符
 */
static void event_mark_deleting(lv_event_list_t * list, lv_event_dsc_t * dsc)
{
    list->has_marked_deleting = true;
    dsc->filter |= LV_EVENT_MARKED_DELETING;
}

/**
 * @brief 检查事件描述符是否已被标记为待删除
 * 
 * @param dsc 事件描述符
 * @return true=已标记删除，false=未标记
 */
static bool event_is_marked_deleting(lv_event_dsc_t * dsc)
{
    return (dsc->filter & LV_EVENT_MARKED_DELETING) != 0;
}

/**
 * @brief 获取事件列表中的事件数量
 * 
 * @param list 事件列表
 * @return 事件描述符数量
 */
static uint32_t event_array_size(lv_event_list_t * list)
{
    return lv_array_size(&list->array);
}

/**
 * @brief 获取事件列表中指定索引的事件描述符指针
 * 
 * @param list 事件列表
 * @param index 索引位置
 * @return 事件描述符的二级指针
 */
static lv_event_dsc_t ** event_array_at(lv_event_list_t * list, uint32_t index)
{
    return lv_array_at(&list->array, index);
}