/**
 * @file ui_ivs_panel.h
 * @brief NPU 结果的文本面板（状态 / 属性 / 对象三区）
 *
 * 与任务无关：属性区按 attrs 的 key:val 逐行显示，不认具体字段名；
 * 因此新增 NPU 任务不需要改本模块。
 *
 * 刷新策略：由外部 5Hz 定时器调用 refresh()，内部只在文本变化时才 set_text，
 * 避免 25fps 下持续触发文本重排与整块重绘。
 */

#ifndef UI_IVS_PANEL_H
#define UI_IVS_PANEL_H

#include "lvgl/lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 创建面板（挂到 Play 页容器上）；w/h 为面板尺寸 */
void ui_ivs_panel_create(lv_obj_t *parent, lv_coord_t w, lv_coord_t h);

void ui_ivs_panel_destroy(void);

/** 显示/隐藏切换，返回切换后的可见性 */
bool ui_ivs_panel_toggle(void);
bool ui_ivs_panel_is_visible(void);
void ui_ivs_panel_set_visible(bool v);

/** 5Hz 调用：读状态与数据槽，更新文本 */
void ui_ivs_panel_refresh(void);

/** FAILED 态下"重试"按钮的回调（为 NULL 则不显示按钮） */
void ui_ivs_panel_set_retry_cb(void (*cb)(void));

#ifdef __cplusplus
}
#endif

#endif /* UI_IVS_PANEL_H */
