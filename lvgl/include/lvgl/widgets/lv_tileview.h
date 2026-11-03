/**
 * @file lv_tileview.h
 *
 * 平铺视图控件
 */

#ifndef LV_TILEVIEW_H
#define LV_TILEVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../core/lv_obj.h"

#if LV_USE_TILEVIEW

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/** 平铺视图控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_tileview_class;
/** 平铺视图中的平铺控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_tileview_tile_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a tileview object
 * 创建一个平铺视图对象
 * @param parent      pointer to an object, it will be the parent of the new tileview
 *                    指向一个对象，它将成为新tileview的父对象
 * @return            pointer to the created tileview
 *                    指向创建的tileview的指针
 */
lv_obj_t * lv_tileview_create(lv_obj_t * parent);

/**
 * Add a tile to the tileview
 * 向平铺视图中添加一个平铺
 * @param tv          pointer to the tileview object
 *                    指向平铺视图对象
 * @param col_id      column id of the tile
 *                    平铺的列ID
 * @param row_id      row id of the tile
 *                    平铺的行ID
 * @param dir         direction to move to the next tile
 *                    切换到下一个平铺的方向
 * @return            pointer to the added tile object
 *                    指向添加的平铺对象的指针
 */
lv_obj_t * lv_tileview_add_tile(lv_obj_t * tv, uint8_t col_id, uint8_t row_id, lv_dir_t dir);

/**
 * Set the active tile in the tileview.
 * 设置平铺视图中活动的平铺。
 * @param parent      pointer to the tileview object
 *                    指向平铺视图对象
 * @param tile_obj    pointer to the tile object to be set as active
 *                    指向要设为活动的平铺对象
 * @param anim_en     animation enable flag (LV_ANIM_ON or LV_ANIM_OFF)
 *                    动画使能标志（LV_ANIM_ON或LV_ANIM_OFF）
 */
void lv_tileview_set_tile(lv_obj_t * tv, lv_obj_t * tile_obj, lv_anim_enable_t anim_en);

/**
 * Set the active tile by index in the tileview
 * 通过索引设置平铺视图中活动的平铺
 * @param tv          pointer to the tileview object
 *                    指向平铺视图对象
 * @param col_id      column id of the tile to be set as active
 *                    要设为活动的平铺的列ID
 * @param row_id      row id of the tile to be set as active
 *                    要设为活动的平铺的行ID
 * @param anim_en     animation enable flag (LV_ANIM_ON or LV_ANIM_OFF)
 *                    动画使能标志（LV_ANIM_ON或LV_ANIM_OFF）
 */
void lv_tileview_set_tile_by_index(lv_obj_t * tv, uint32_t col_id, uint32_t row_id, lv_anim_enable_t anim_en);

/**
 * Get the currently active tile in the tileview
 * 获取平铺视图中当前活动的平铺
 * @param obj         pointer to the tileview object
 *                    指向平铺视图对象
 * @return            pointer to the currently active tile object
 *                    指向当前活动的平铺对象的指针
 */
lv_obj_t * lv_tileview_get_tile_active(lv_obj_t * obj);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_TILEVIEW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TILEVIEW_H*/
