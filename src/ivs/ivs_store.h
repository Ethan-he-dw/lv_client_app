/**
 * @file ivs_store.h
 * @brief 最新值单槽：写侧覆盖、读侧按 version 判断是否有新帧
 *
 * 为什么不用队列：本场景明确"不需要与画面同步"，25fps 在 50ms 轮询里必然堆积；
 * 队列满会丢帧且增加延迟，而覆盖式单槽天然跳到最新，UI 卡顿后无损恢复。
 */

#ifndef IVS_STORE_H
#define IVS_STORE_H

#include "ivs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 覆盖写入（任意线程调用；内部加锁，version++） */
void ivs_store_put(const ivs_result_t *r);

/** 无条件读取当前值；返回 false 表示还没有过数据 */
bool ivs_store_get(ivs_result_t *out, uint32_t *ver);

/** 仅当 version 与 last_ver 不同才拷贝并更新 last_ver（UI 主路径） */
bool ivs_store_get_if_new(ivs_result_t *out, uint32_t *last_ver);

/** 当前 version（0 = 尚无数据） */
uint32_t ivs_store_version(void);

/** 清空（切 URL / 关会话时调用，version 归零） */
void ivs_store_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* IVS_STORE_H */
