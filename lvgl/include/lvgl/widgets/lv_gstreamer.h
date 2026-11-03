/**
 * @file lv_gstreamer.h
 * GStreamer 视频控件头文件：提供基于 GStreamer 的媒体源设置、播放控制、音量/速率调整等功能。
 *
 */

#ifndef LV_GSTREAMER_H
#define LV_GSTREAMER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_GSTREAMER
#include "../core/lv_obj.h"


/*********************
 *      DEFINES
 * 宏定义
 *********************/


/* Using the `URI` "factory", we can specify various URI schemes as media sources including
 * 使用 `URI` "factory"，我们可以指定各种 URI 方案作为媒体源，包括
 * - local files (file://)
 * - 本地文件(file://)
 * - web streams (http://, https://)
 * - 网络流(http://、https://)
 * - RTSP streams (rtsp://)
 * - RTSP 流(rtsp://)
 * - UDP streams (udp://)
 * - UDP 流(udp://)
 * and many others.
 * 以及其他许多方案。
 * GStreamer's uridecodebin automatically selects the appropriate
 * GStreamer 的 uridecodebin 会根据 URI 方案和媒体格式自动选择
 * source element and decoder based on the URI scheme and media format. */

 * */
#define LV_GSTREAMER_FACTORY_URI_DECODE      "uridecodebin"
#define LV_GSTREAMER_PROPERTY_URI_DECODE      "uri"

/** 本地文件源工厂与属性 */
#define LV_GSTREAMER_FACTORY_FILE            "filesrc"
#define LV_GSTREAMER_PROPERTY_FILE            "location"

/** HTTP 源工厂与属性 */
#define LV_GSTREAMER_FACTORY_HTTP            "souphttpsrc"
#define LV_GSTREAMER_PROPERTY_HTTP            "location"

/** HTTPS 源工厂与属性 */
#define LV_GSTREAMER_FACTORY_HTTPS           "souphttpsrc"
#define LV_GSTREAMER_PROPERTY_HTTPS           "location"

/** V4L2 摄像头源工厂与属性 */
#define LV_GSTREAMER_FACTORY_V4L2_CAMERA     "v4l2src"
#define LV_GSTREAMER_PROPERTY_V4L2_CAMERA     "device"

/** ALSA 音频源工厂与属性 */
#define LV_GSTREAMER_FACTORY_ALSA_AUDIO      "alsasrc"
#define LV_GSTREAMER_PROPERTY_ALSA_AUDIO      "device"

/** PulseAudio 音频源工厂与属性 */
#define LV_GSTREAMER_FACTORY_PULSE_AUDIO     "pulsesrc"
#define LV_GSTREAMER_PROPERTY_PULSE_AUDIO     "device"

/** 测试音频源工厂与属性 */
#define LV_GSTREAMER_FACTORY_TEST_AUDIO      "audiotestsrc"
#define LV_GSTREAMER_PROPERTY_TEST_AUDIO      NULL

/** 测试视频源工厂与属性 */
#define LV_GSTREAMER_FACTORY_TEST_VIDEO      "videotestsrc"
#define LV_GSTREAMER_PROPERTY_TEST_VIDEO      NULL

/** 应用源工厂与属性 */
#define LV_GSTREAMER_FACTORY_APP             "appsrc"
#define LV_GSTREAMER_PROPERTY_APP             NULL

/** WebRTC 源工厂与属性 */
#define LV_GSTREAMER_FACTORY_WEBRTCSRC       "webrtcsrc"
#define LV_GSTREAMER_PROPERTY_WEBRTCSRC      "signaller::uri"

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

typedef enum {
    /** 空状态 */
    LV_GSTREAMER_STATE_NULL,
    /** 就绪状态 */
    LV_GSTREAMER_STATE_READY,
    /** 暂停状态 */
    LV_GSTREAMER_STATE_PAUSED,
    /** 播放状态 */
    LV_GSTREAMER_STATE_PLAYING
} lv_gstreamer_state_t;

typedef enum {
    /** 流启动 */
    LV_GSTREAMER_STREAM_STATE_START,
    /** 流播放 */
    LV_GSTREAMER_STREAM_STATE_PLAY,
    /** 流暂停 */
    LV_GSTREAMER_STREAM_STATE_PAUSE,
    /** 流停止 */
    LV_GSTREAMER_STREAM_STATE_STOP,
    /** 流结束 */
    LV_GSTREAMER_STREAM_STATE_END
} lv_gstreamer_stream_state_t;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a gstreamer object
 * 创建一个 gstreamer 对象
 * @param parent    pointer to an object, it will be the parent of the new gstreamer
 * @param parent    指向一个对象，它将成为新 gstreamer 的父对象
 * @return          pointer to the created gstreamer
 * @return          指向创建好的 gstreamer 对象的指针
 */
lv_obj_t * lv_gstreamer_create(lv_obj_t * parent);

/**
 * Add a source to this gstreamer object
 * 为 gstreamer 对象添加一个媒体源
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 * @param factory_name  the factory name for the source of this gstreamer object.
 * @param factory_name  该 gstreamer 对象媒体源的工厂名称。
 *                      for common factory names, check `LV_GSTREAMER_FACTORY_XXX` defines
 *                      常见工厂名称请参见 `LV_GSTREAMER_FACTORY_XXX` 宏定义
 * @param property      the property name for the gstreamer source object
 * @param property      gstreamer 源对象的属性名
 *                      for common properties, see `LV_GSTREAMER_PROPERTY_XXX` defines
 *                      常见属性请参见 `LV_GSTREAMER_PROPERTY_XXX` 宏定义
 *                      Passing NULL will create the source object but not set its source
 *                      传入 NULL 将创建源对象但不设置其媒体源
 * @param source        the property value for the gstreamer source object
 * @param source        gstreamer 源对象的属性值
 *                      Passing NULL will create the source object but not set its source
 *                      传入 NULL 将创建源对象但不设置其媒体源
 * @return LV_RESULT_OK if the source was correctly set else LV_RESULT_INVALID
 * @return 媒体源设置正确时返回 LV_RESULT_OK，否则返回 LV_RESULT_INVALID
 */
lv_result_t lv_gstreamer_set_src(lv_obj_t * gstreamer, const char * factory_name, const char * property,
                                 const char * source);

/**
 * Play this gstreamer
 * 播放该 gstreamer
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 */
void lv_gstreamer_play(lv_obj_t * gstreamer);

/**
 * Pause this gstreamer
 * 暂停该 gstreamer
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 */
void lv_gstreamer_pause(lv_obj_t * gstreamer);

/**
 * Stop this gstreamer
 * 停止该 gstreamer
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 */
void lv_gstreamer_stop(lv_obj_t * gstreamer);

/**
 * Seek a position in this gstreamer
 * 在该 gstreamer 中跳转到指定位置
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 * @param position      position to seek to
 * @param position      要跳转到的位置
 */
void lv_gstreamer_set_position(lv_obj_t * gstreamer, uint32_t position);

/**
 * Get the duration of this gstreamer
 * 获取该 gstreamer 的时长
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 * @return              the duration (in ms) of the gstreamer object
 * @return              gstreamer 对象的时长(毫秒)
 */
uint32_t lv_gstreamer_get_duration(lv_obj_t * gstreamer);

/**
 * Get the position of this gstreamer
 * 获取该 gstreamer 的播放位置
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 * @return              the position (in ms) of the gstreamer object
 * @return              gstreamer 对象的播放位置(毫秒)
 */
uint32_t lv_gstreamer_get_position(lv_obj_t * gstreamer);

/**
 * Get the state of this gstreamer
 * 获取该 gstreamer 的状态
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 */
lv_gstreamer_state_t lv_gstreamer_get_state(lv_obj_t * gstreamer);

/**
 * Set the volume of this gstreamer
 * 设置该 gstreamer 的音量
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 * @param volume         the value to set in the range [0..100]. Higher values are clamped
 * @param volume         要设置的音量值，范围 [0..100]。更高的值会被截断
 */
void lv_gstreamer_set_volume(lv_obj_t * gstreamer, uint8_t volume);

/**
 * Get the volume of this gstreamer
 * 获取该 gstreamer 的音量
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 * @return      the volume for this gstreamer
 * @return      该 gstreamer 的音量
 */
uint8_t lv_gstreamer_get_volume(lv_obj_t * gstreamer);

/**
 * Set the speed rate of this gstreamer
 * 设置该 gstreamer 的播放速率
 * @param gstreamer     pointer to a gstreamer object
 * @param gstreamer     指向 gstreamer 对象的指针
 * @param rate      the rate factor.  Example values:
 * @param rate      速率倍数。示例值：
 *                      - 256:   1x
 *                      - 256：  1 倍速
 *                      - <256:  slow down
 *                      - <256： 放慢
 *                      - >256:  speed up
 *                      - >256： 加快
 *                      - 128:   0.5x
 *                      - 128：  0.5 倍速
 *                      - 512:   2x
 *                      - 512：  2 倍速
 */
void lv_gstreamer_set_rate(lv_obj_t * gstreamer, uint32_t rate);

/**
 * Retrieve the stream state from a STATE_CHANGED event callback
 * 从 STATE_CHANGED(状态改变)事件回调中获取流状态
 * @param e     pointer to the event
 * @param e     指向事件的指针
 * @return the stream state or -1 if `e` is invalid (i.e. NULL or does not match expected event)
 * @return 流状态；如果 `e` 无效(即 NULL 或与预期事件不匹配)则返回 -1
 */
lv_gstreamer_stream_state_t lv_gstreamer_get_stream_state(lv_event_t * e);


/**********************
 *      MACROS
 * 宏
 **********************/

#endif /*LV_USE_GSTREAMER*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GSTREAMER_H*/
