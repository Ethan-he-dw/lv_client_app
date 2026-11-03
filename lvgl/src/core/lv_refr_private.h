/**
 * @file lv_refr_private.h
 * 屏幕刷新私有模块：提供屏幕刷新子系统的初始化与区域重绘等内部接口。
 *
 */

#ifndef LV_REFR_PRIVATE_H
#define LV_REFR_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/

#include "../lvgl_public.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Initialize the screen refresh subsystem
 * 初始化屏幕刷新子系统
 */
void lv_refr_init(void);

/**
 * Deinitialize the screen refresh subsystem
 * 反初始化屏幕刷新子系统
 */
void lv_refr_deinit(void);

/**
 * Invalidate an area on display to redraw it
 * 使显示区域无效以便重绘
 * @param area_p pointer to area which should be invalidated (NULL: delete the invalidated areas)
 *               指向需要设为无效的区域的指针(NULL: 清除无效区域)
 * @param disp pointer to display where the area should be invalidated (NULL can be used if there is
 * only one display)
 *             需要设为无效的区域所在的显示设备(只有一个显示设备时可为 NULL)
 * @return LV_RESULT_OK: the area is invalidated; LV_RESULT_INVALID: the area wasn't invalidated.
 *         LV_RESULT_OK: 区域已设为无效; LV_RESULT_INVALID: 区域未被设为无效。
 */
lv_result_t lv_inv_area(lv_display_t * disp, const lv_area_t * area_p);

/**
 * Get the display which is being refreshed
 * 获取正在刷新的显示设备
 * @return the display being refreshed
 *         正在刷新的显示设备
 */
lv_display_t * lv_refr_get_disp_refreshing(void);

/**
 * Set the display which is being refreshed
 * 设置正在刷新的显示设备
 * @param disp the display being refreshed
 *             正在刷新的显示设备
 */
void lv_refr_set_disp_refreshing(lv_display_t * disp);

/**
 * Search the most top object which fully covers an area
 * 查找完全覆盖某个区域的最顶层对象
 * @param area_p pointer to an area
 *                指向区域的指针
 * @param obj the first object to start the searching (typically a screen)
 *            开始搜索的第一个对象(通常是屏幕)
 * @return    找到的对象
 */
lv_obj_t * lv_refr_get_top_obj(const lv_area_t * area_p, lv_obj_t * obj);

/**
 * Render an object to a layer
 * 将对象渲染到图层上
 * @param layer target drawing layer
 *              目标绘制图层
 * @param obj   object to render
 *              要渲染的对象
 */
void lv_obj_refr(lv_layer_t * layer, lv_obj_t * obj);

/**********************
 *      MACROS
 *      宏定义
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_REFR_PRIVATE_H*/
