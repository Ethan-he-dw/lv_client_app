/**
 * @file ivs_source.h
 * @brief 订阅源抽象层：UI 只依赖本文件，不依赖任何框架
 *
 * 分层意图：
 *   UI ──► ivs_source（本文） ──► [mock | omnibinder 实现]
 * 换框架只改实现；加 NPU 任务只改绑定（见 ivs_src_omnibinder.cpp 的 kBindings）。
 *
 * 实现必须遵守的契约：
 *  1. open() 立即返回，连接/订阅在实现内部线程完成；
 *  2. state()/latest() 可在任意线程调用，内部加锁；
 *  3. 实现内部禁止调用任何 lv_*（LVGL 非线程安全）；
 *  4. close() 幂等，且必须能打断实现内部的阻塞等待；
 *  5. ver 只在新帧到达时递增（同一帧重复读不增）；
 *  6. 单例：open() 时若已有会话，先内部 close()。
 */

#ifndef IVS_SOURCE_H
#define IVS_SOURCE_H

#include "ivs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ivs_source_ops {
    const char *name;                                            /* "mock" / "omnibinder" */
    int  (*open)(const char *host, int port, const char *task);   /* 立即返回；0=已受理 */
    void (*close)(void);                                          /* 幂等 */
    ivs_state_t (*state)(void);                                   /* 线程安全；NODATA 惰性推导 */
    bool (*latest)(ivs_result_t *out, uint32_t *ver);
    const char *(*last_error)(void);
} ivs_source_ops_t;

/* 各实现导出的 ops 表 */
extern const ivs_source_ops_t ivs_src_mock;
#ifdef IVS_SRC_OMNIBINDER
extern const ivs_source_ops_t ivs_src_omnibinder;
#endif

/** 当前生效的实现（编译期选定；无框架环境自动回落到 mock） */
static inline const ivs_source_ops_t *ivs_source_active(void)
{
#ifdef IVS_SRC_OMNIBINDER
    return &ivs_src_omnibinder;
#else
    return &ivs_src_mock;
#endif
}

/* ===================== UI 侧便捷接口 ===================== */

static inline int ivs_open(const char *host, int port, const char *task)
{
    return ivs_source_active()->open(host, port, task);
}

static inline void ivs_close(void)
{
    ivs_source_active()->close();
}

static inline ivs_state_t ivs_state(void)
{
    return ivs_source_active()->state();
}

static inline bool ivs_latest(ivs_result_t *out, uint32_t *ver)
{
    return ivs_source_active()->latest(out, ver);
}

static inline const char *ivs_last_error(void)
{
    return ivs_source_active()->last_error();
}

/* ===================== 实现公用工具 ===================== */

/** 单调时钟毫秒（实现见 ivs_store.c）；只用于算相对时间/超时 */
uint64_t ivs_now_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* IVS_SOURCE_H */
