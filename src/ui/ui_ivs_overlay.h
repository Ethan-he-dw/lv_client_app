/**
 * @file ui_ivs_overlay.h
 * @brief 画面叠加层（通用）：框 + 关键点 + 主目标标签
 *
 * 与任务无关：颜色/是否画关键点/空标签文案由样式表按 task 索引，
 * 新增 NPU 任务只需在样式表加一行。
 *
 * 线程约束：本模块只在 LVGL 线程被调用；数据由 ivs_store 提供（已加锁）。
 */

#ifndef UI_IVS_OVERLAY_H
#define UI_IVS_OVERLAY_H

#include "lvgl/lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 创建叠加层（挂到当前 screen；需在播放器创建之后调用，或随后再 set_player 提升层级） */
void ui_ivs_overlay_create(void);
void ui_ivs_overlay_destroy(void);

/** 绑定播放器（几何基准）；传 NULL 解绑并清空 */
void ui_ivs_overlay_set_player(lv_obj_t *player);

void ui_ivs_overlay_set_visible(bool v);
bool ui_ivs_overlay_is_visible(void);

/** 清空所有框/点/标签（停播、切 URL、离开页面时调用） */
void ui_ivs_overlay_clear(void);

/** 30ms 定时器调用：读数据槽并更新几何 */
void ui_ivs_overlay_update(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_IVS_OVERLAY_H */
