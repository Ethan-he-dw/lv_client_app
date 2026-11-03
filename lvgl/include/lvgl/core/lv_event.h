/**
 * @file lv_event.h
 *
 */

#ifndef LV_EVENT_H
#define LV_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../lv_types.h"
#include "../config/lv_conf_internal.h"
#include "../misc/lv_array.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef void (*lv_event_cb_t)(lv_event_t * e);
/**
 * 发送给控件的事件类型
 */
typedef enum {
    LV_EVENT_ALL = 0,                          /**< 所有事件（用于注册监听全部事件）*/

    /* 输入设备事件 */
    LV_EVENT_PRESSED,             /**< 控件被按下 */
    LV_EVENT_PRESSING,            /**< 控件正在被按下（按住期间持续发送） */
    LV_EVENT_PRESS_LOST,          /**< 控件仍处于按下状态，但触摸点/光标已滑出控件范围 */
    LV_EVENT_SHORT_CLICKED,       /**< 控件被短时间按下后释放（未发生滚动时发送） */
    LV_EVENT_SINGLE_CLICKED,      /**< 在短距离和短时间内第一次单击时发送 */
    LV_EVENT_DOUBLE_CLICKED,      /**< 在短距离和短时间内第二次单击时发送 */
    LV_EVENT_TRIPLE_CLICKED,      /**< 在短距离和短时间内第三次单击时发送 */
    LV_EVENT_LONG_PRESSED,        /**< 控件被按下至少达到 `long_press_time` 时长（未发生滚动时发送） */
    LV_EVENT_LONG_PRESSED_REPEAT, /**< 长按达到 `long_press_time` 后，每隔 `long_press_repeat_time` 毫秒重复发送（未发生滚动） */
    LV_EVENT_CLICKED,             /**< 释放时发送，前提是未发生滚动（无论是否长按） */
    LV_EVENT_RELEASED,            /**< 控件被释放时始终发送（任何情况） */
    LV_EVENT_SCROLL_BEGIN,        /**< 滚动开始。事件参数为指向滚动动画的指针，可修改 */
    LV_EVENT_SCROLL_THROW_BEGIN,  /**< 惯性滚动开始 */
    LV_EVENT_SCROLL_END,          /**< 滚动结束 */
    LV_EVENT_SCROLL,              /**< 滚动进行中 */
    LV_EVENT_GESTURE,             /**< 检测到手势。通过 `lv_indev_get_gesture_dir(lv_indev_active())` 获取手势方向 */
    LV_EVENT_KEY,                 /**< 按键事件发送给控件。通过 `lv_indev_get_key(lv_indev_active())` 获取键值 */
    LV_EVENT_ROTARY,              /**< 编码器或滚轮旋转。通过 `lv_event_get_rotary_diff(e)` 获取旋转增量 */
    LV_EVENT_FOCUSED,             /**< 控件获得焦点 */
    LV_EVENT_DEFOCUSED,           /**< 控件失去焦点 */
    LV_EVENT_LEAVE,               /**< 控件失去焦点但仍处于选中状态 */
    LV_EVENT_HIT_TEST,            /**< 执行高级命中测试 */
    LV_EVENT_INDEV_RESET,         /**< 输入设备被复位 */
    LV_EVENT_HOVER_OVER,          /**< 输入设备悬停在控件上方 */
    LV_EVENT_HOVER_LEAVE,         /**< 输入设备悬停离开控件 */

    /* 绘制事件 */
    LV_EVENT_COVER_CHECK,         /**< 检查控件是否完全覆盖某个区域。事件参数为 `lv_cover_check_info_t *` 类型 */
    LV_EVENT_REFR_EXT_DRAW_SIZE,  /**< 获取控件所需的额外绘制区域（例如阴影）。事件参数为 `int32_t *` 用于存储大小 */
    LV_EVENT_DRAW_MAIN_BEGIN,     /**< 主绘制阶段开始 */
    LV_EVENT_DRAW_MAIN,           /**< 执行主绘制 */
    LV_EVENT_DRAW_MAIN_END,       /**< 主绘制阶段结束 */
    LV_EVENT_DRAW_POST_BEGIN,     /**< 后绘制阶段开始（所有子控件绘制完毕后） */
    LV_EVENT_DRAW_POST,           /**< 执行后绘制（所有子控件绘制完毕后） */
    LV_EVENT_DRAW_POST_END,       /**< 后绘制阶段结束（所有子控件绘制完毕后） */
    LV_EVENT_DRAW_TASK_ADDED,     /**< 添加绘制任务（需要设置 `LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS` 标志） */

    /* 特殊事件 */
    LV_EVENT_VALUE_CHANGED,       /**< 控件的值发生变化（例如滑块移动） */
    LV_EVENT_INSERT,              /**< 文本被插入到控件中。事件数据为 `char *` 指向插入的字符串 */
    LV_EVENT_REFRESH,             /**< 通知控件刷新其内容（供用户自定义使用） */
    LV_EVENT_READY,               /**< 某个处理流程已完成 */
    LV_EVENT_CANCEL,              /**< 某个处理流程已取消 */
    LV_EVENT_STATE_CHANGED,       /**< 控件的状态发生了变化 */

    /* 其他事件 */
    LV_EVENT_CREATE,              /**< 对象正在被创建 */
    LV_EVENT_DELETE,              /**< 对象正在被删除 */
    LV_EVENT_CHILD_CHANGED,       /**< 子对象被移除、添加，或其尺寸、位置发生改变 */
    LV_EVENT_CHILD_CREATED,       /**< 子对象被创建（始终冒泡到所有父对象） */
    LV_EVENT_CHILD_DELETED,       /**< 子对象被删除（始终冒泡到所有父对象） */
    LV_EVENT_SCREEN_UNLOAD_START, /**< 屏幕卸载开始，在调用 `scr_load` 时立即触发 */
    LV_EVENT_SCREEN_LOAD_START,   /**< 屏幕加载开始，在屏幕切换延迟到期时触发 */
    LV_EVENT_SCREEN_LOADED,       /**< 屏幕已加载完成 */
    LV_EVENT_SCREEN_UNLOADED,     /**< 屏幕已卸载完成 */
    LV_EVENT_SIZE_CHANGED,        /**< 对象的坐标/尺寸已改变 */
    LV_EVENT_STYLE_CHANGED,       /**< 对象的样式已改变 */
    LV_EVENT_LAYOUT_CHANGED,      /**< 由于布局重新计算，子对象的位置发生了改变 */
    LV_EVENT_GET_SELF_SIZE,       /**< 获取控件的内部尺寸 */

    /* LVGL 可选组件的事件 */
    LV_EVENT_INVALIDATE_AREA,     /**< 某区域被标记为无效（需要重绘）。事件参数为 `lv_area_t *`，指向待失效区域的坐标。
                                    *   可以自由修改该区域以适应特殊显示需求（如单色屏需按 8 像素行/列批量失效）。 */
    LV_EVENT_RESOLUTION_CHANGED,  /**< 分辨率发生变化（通过 `lv_display_set_resolution()` 或 `lv_display_set_rotation()` 触发） */
    LV_EVENT_COLOR_FORMAT_CHANGED,/**< 颜色格式发生变化（通过 `lv_display_set_color_format()` 触发） */
    LV_EVENT_REFR_REQUEST,        /**< 发生需要重绘的事件时发送 */
    LV_EVENT_REFR_START,          /**< 刷新周期开始前发送（即使没有内容需要重绘也会发送） */
    LV_EVENT_REFR_READY,          /**< 刷新完成时发送（渲染并调用刷新回调之后，即使没有重绘也会发送） */
    LV_EVENT_RENDER_START,        /**< 即将开始渲染之前发送 */
    LV_EVENT_RENDER_READY,        /**< 渲染完成之后发送 */
    LV_EVENT_FLUSH_START,         /**< 调用刷新回调之前发送 */
    LV_EVENT_FLUSH_FINISH,        /**< 刷新回调返回之后发送 */
    LV_EVENT_FLUSH_WAIT_START,    /**< 调用刷新等待回调之前发送 */
    LV_EVENT_FLUSH_WAIT_FINISH,   /**< 刷新等待回调返回之后发送 */
    LV_EVENT_SYNC_START,          /**< 调用同步回调之前发送 */
    LV_EVENT_SYNC_FINISH,         /**< 同步回调返回之后发送 */
    LV_EVENT_SYNC_WAIT_START,     /**< 调用同步等待回调之前发送 */
    LV_EVENT_SYNC_WAIT_FINISH,    /**< 同步等待回调返回之后发送 */
    LV_EVENT_UPDATE_LAYOUT_COMPLETED, /**< 布局更新完成后发送 */

    LV_EVENT_VSYNC,               /**< 垂直同步事件 */
    LV_EVENT_VSYNC_REQUEST,       /**< 请求垂直同步 */

#if LV_USE_TRANSLATION
    LV_EVENT_TRANSLATION_LANGUAGE_CHANGED, /**< 翻译语言发生变化时发送（需启用 `LV_USE_TRANSLATION`） */
#endif /*LV_USE_TRANSLATION*/

    LV_EVENT_LAST,                 /**< 默认事件数量 */

    /* 标志位（可与其他事件按位或） */
    LV_EVENT_PREPROCESS = 0x8000,   /**< 设置此标志后，该事件将在类的默认事件处理之前被处理 */
    LV_EVENT_MARKED_DELETING = 0x10000, /**< 标记对象正在删除（内部使用） */
} lv_event_code_t;
typedef struct {
    lv_array_t array;
    uint8_t is_traversing: 1;          /**< True: the list is being nested traversed */
    uint8_t has_marked_deleting: 1;    /**< True: the list has marked deleting objects
                                         when some of events are marked as deleting */
} lv_event_list_t;

/**
 * @brief Event callback.
 * Events are used to notify the user of some action being taken on Widget.
 * For details, see ::lv_event_t.
 */

lv_result_t lv_event_send(lv_event_list_t * list, lv_event_t * e, bool preprocess);

lv_event_dsc_t * lv_event_add(lv_event_list_t * list, lv_event_cb_t cb, lv_event_code_t filter, void * user_data);
bool lv_event_remove_dsc(lv_event_list_t * list, lv_event_dsc_t * dsc);

uint32_t lv_event_get_count(lv_event_list_t * list);

lv_event_dsc_t * lv_event_get_dsc(lv_event_list_t * list, uint32_t index);

lv_event_cb_t lv_event_dsc_get_cb(lv_event_dsc_t * dsc);

void * lv_event_dsc_get_user_data(lv_event_dsc_t * dsc);

bool lv_event_remove(lv_event_list_t * list, uint32_t index);

void lv_event_remove_all(lv_event_list_t * list);

/**
 * Get Widget originally targeted by the event. It's the same even if event was bubbled.
 * @param e     pointer to the event descriptor
 * @return      the target of the event_code
 */
void * lv_event_get_target(lv_event_t * e);

/**
 * Get current target of the event. It's the Widget for which the event handler being called.
 * If the event is not bubbled it's the same as "normal" target.
 * @param e     pointer to the event descriptor
 * @return      pointer to the current target of the event_code
 */
void * lv_event_get_current_target(lv_event_t * e);

/**
 * Get event code of an event.
 * @param e     pointer to the event descriptor
 * @return      the event code. (E.g. `LV_EVENT_CLICKED`, `LV_EVENT_FOCUSED`, etc)
 */
lv_event_code_t lv_event_get_code(lv_event_t * e);

/**
 * Get parameter passed when event was sent.
 * @param e     pointer to the event descriptor
 * @return      pointer to the parameter
 */
void * lv_event_get_param(lv_event_t * e);

/**
 * Get user_data passed when event was registered on Widget.
 * @param e     pointer to the event descriptor
 * @return      pointer to the user_data
 */
void * lv_event_get_user_data(lv_event_t * e);

/**
 * Stop event from bubbling.
 * This is only valid when called in the middle of an event processing chain.
 * @param e     pointer to the event descriptor
 */
void lv_event_stop_bubbling(lv_event_t * e);

/**
 * Stop event from trickling down to children.
 * This is only valid when called in the middle of an event processing chain.
 * @param e     pointer to the event descriptor
 */
void lv_event_stop_trickling(lv_event_t * e);

/**
 * Stop processing this event.
 * This is only valid when called in the middle of an event processing chain.
 * @param e     pointer to the event descriptor
 */
void lv_event_stop_processing(lv_event_t * e);

/**
 * Helper function typically used in LV_EVENT_DELETE
 * to free the event's user_data
 * @param e     pointer to an event descriptor
 */
void lv_event_free_user_data_cb(lv_event_t * e);


/**
 * Register a new, custom event ID.
 * It can be used the same way as e.g. `LV_EVENT_CLICKED` to send custom events
 * @return      the new event id
 *
 * Example:
 * @code
 * uint32_t LV_EVENT_MINE = 0;
 * ...
 * e = lv_event_register_id();
 * ...
 * lv_obj_send_event(obj, LV_EVENT_MINE, &some_data);
 * @endcode
 */
uint32_t lv_event_register_id(void);

/**
 * Get the name of an event code.
 * @param code  the event code
 * @return      the name of the event code as a string
 */
const char * lv_event_code_get_name(lv_event_code_t code);

#if LV_USE_EXT_DATA
/**
 * Set external data and its destructor for an event descriptor.
 * This allows associating custom data with an event callback that will be automatically cleaned up
 * when the event descriptor is removed or destroyed.
 * @param dsc         pointer to an event descriptor (from lv_obj_add_event_cb)
 * @param data        pointer to the external data to associate with the event descriptor
 * @param free_cb     function pointer to a destructor that will be called to clean up the external data.
 *                    The destructor will receive the data pointer as its parameter.
 */
void lv_event_desc_set_external_data(lv_event_dsc_t * dsc, void * data, void (* free_cb)(void * data));
#endif

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_EVENT_H */
