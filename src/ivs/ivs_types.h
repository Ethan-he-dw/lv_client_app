/**
 * @file ivs_types.h
 * @brief NPU 结果的统一数据模型（与具体任务无关）
 *
 * 设计要点：
 *  - 定长 POD，可安全跨越 C/C++ 边界（UI 是 C，适配器是 C++）；
 *  - 任务差异由两部分承载：objs[]（几何 + 标签）与 attrs[]（键值对）；
 *  - UI/面板不认识 "cos""det_ms" 这类字段名，只按 key: val 展示，
 *    因此新增 NPU 任务不需要改显示层。
 */

#ifndef IVS_TYPES_H
#define IVS_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===================== 容量上限 ===================== */
#define IVS_MAX_OBJS      16   /* 与板端 kMaxBoxes 对齐，超出截断 */
#define IVS_MAX_KPS       10   /* 5 点 × (x,y) */
#define IVS_LABEL_MAX     32   /* 姓名/类别：字母或编号 */
#define IVS_MAX_ATTRS     10   /* 任务自有标量：fps/det_ms/cos/thr... */
#define IVS_ATTR_KEY_MAX  16
#define IVS_ATTR_VAL_MAX  40
#define IVS_TASK_NAME_MAX 16

/* 检测画布几何（板端 frame_types.h / osd_face.cpp 的契约，编译期常量） */
#define IVS_CANVAS_W    640.0f
#define IVS_CANVAS_H    640.0f
#define IVS_CANVAS_PAD  140.0f
#define IVS_ACTIVE_H    360.0f

/* ===================== 任务类型 ===================== */
typedef enum {
    IVS_TASK_FACE = 0,
    IVS_TASK_PERSON,
    IVS_TASK_VEHICLE,
    IVS_TASK_CUSTOM
} ivs_task_t;

/* ===================== 单个目标 ===================== */
typedef struct {
    float   score;                        /* 置信度 0~1 */
    float   x1, y1, x2, y2;               /* 画布坐标（640×640，含 pad） */
    float   kps[IVS_MAX_KPS];             /* 5 点 ×(x,y)：左眼/右眼/鼻/左嘴角/右嘴角 */
    uint8_t kps_n;                        /* 0 或 10；不足视为缺失 */
    uint8_t is_best;                      /* 主目标（高亮用） */
    char    label[IVS_LABEL_MAX];         /* 姓名或类别；空串=无标签 */
} ivs_object_t;

/* ===================== 任务属性（键值对） ===================== */
typedef struct {
    char key[IVS_ATTR_KEY_MAX];
    char val[IVS_ATTR_VAL_MAX];
} ivs_attr_t;

/* ===================== 一帧结果 ===================== */
typedef struct {
    uint8_t      task;                    /* ivs_task_t */
    char         task_name[IVS_TASK_NAME_MAX];  /* "face"，面板/日志用 */
    uint64_t     seq;                     /* 帧序号（板端单调递增） */
    uint64_t     ts_us;                   /* 板端单调时钟微秒，只能算相对时间 */
    uint16_t     obj_count;               /* 已截断到 IVS_MAX_OBJS */
    ivs_object_t objs[IVS_MAX_OBJS];
    uint8_t      attr_n;
    ivs_attr_t   attrs[IVS_MAX_ATTRS];
} ivs_result_t;

/* ===================== 链路状态 ===================== */
typedef enum {
    IVS_IDLE = 0,       /* 未开始 */
    IVS_CONNECTING,     /* 正在连接/订阅 */
    IVS_SUBSCRIBED,     /* 订阅成功，尚无数据 */
    IVS_STREAMING,      /* 正在收数据 */
    IVS_NODATA,         /* 曾连上但超时无数据（由 state() 惰性推导） */
    IVS_FAILED          /* 重试用尽或不可恢复错误 */
} ivs_state_t;

/* ===================== 小工具 ===================== */

static inline const char *ivs_state_str(ivs_state_t st)
{
    switch (st) {
        case IVS_IDLE:       return "IDLE";
        case IVS_CONNECTING: return "CONNECTING";
        case IVS_SUBSCRIBED: return "SUBSCRIBED";
        case IVS_STREAMING:  return "STREAMING";
        case IVS_NODATA:     return "NODATA";
        case IVS_FAILED:     return "FAILED";
        default:             return "?";
    }
}

static inline const char *ivs_task_str(uint8_t task)
{
    switch (task) {
        case IVS_TASK_FACE:    return "face";
        case IVS_TASK_PERSON:  return "person";
        case IVS_TASK_VEHICLE: return "vehicle";
        default:               return "custom";
    }
}

/** 追加一条属性（满则忽略）；fmt 走 printf 语义 */
static inline void ivs_attr_setf(ivs_result_t *r, const char *key, const char *fmt, ...)
{
    if (r == NULL || key == NULL || fmt == NULL) return;
    if (r->attr_n >= IVS_MAX_ATTRS) return;

    ivs_attr_t *a = &r->attrs[r->attr_n];
    strncpy(a->key, key, IVS_ATTR_KEY_MAX - 1);
    a->key[IVS_ATTR_KEY_MAX - 1] = '\0';

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(a->val, sizeof(a->val), fmt, ap);
    va_end(ap);

    r->attr_n++;
}

/** 安全字符串拷贝（总是以 '\0' 结尾） */
static inline void ivs_strcpy(char *dst, size_t dst_sz, const char *src)
{
    if (dst == NULL || dst_sz == 0) return;
    if (src == NULL) { dst[0] = '\0'; return; }
    strncpy(dst, src, dst_sz - 1);
    dst[dst_sz - 1] = '\0';
}

#ifdef __cplusplus
}
#endif

#endif /* IVS_TYPES_H */
