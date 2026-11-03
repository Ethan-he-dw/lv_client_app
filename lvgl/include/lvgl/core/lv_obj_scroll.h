/**
 * @file lv_obj_scroll.h
 *
 * LVGL 对象滚动（Scroll）管理模块
 *
 * 本头文件定义了 LVGL 中与滚动相关的所有类型和函数接口，包括：
 *
 * 1. 滚动条显示模式（scrollbar mode）——控制滚动条何时可见
 * 2. 滚动吸附（snap）——滚动停止时子对象对齐方式
 * 3. 滚动方向控制——限定允许滚动的方向
 * 4. 滚动位置查询——获取当前滚动偏移量和剩余可滚动距离
 * 5. 主动滚动——通过代码以动画或立即方式滚动到指定位置
 * 6. 辅助功能——检测滚动状态、获取滚动条区域、刷新吸附等
 *
 * 滚动相关的标志位（如 LV_OBJ_FLAG_SCROLLABLE、LV_OBJ_FLAG_SCROLL_ELASTIC 等）
 * 定义在 lv_obj.h 的 lv_obj_flag_t 枚举中。
 *
 * 常见用法示例：
 * @code
 *   // 1. 使对象可滚动（lv_obj 默认已是可滚动的）
 *   // lv_obj 默认带有 LV_OBJ_FLAG_SCROLLABLE 标志
 *
 *   // 2. 设置滚动条显示模式
 *   lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_AUTO);
 *
 *   // 3. 只允许垂直滚动（禁止水平滚动）
 *   lv_obj_set_scroll_dir(cont, LV_DIR_BOTTOM);
 *
 *   // 4. 启用垂直滚动吸附到顶部
 *   lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_START);
 *
 *   // 5. 通过代码滚动到 Y=200 处（带动画）
 *   lv_obj_scroll_to_y(cont, 200, LV_ANIM_ON);
 *
 *   // 6. 使某个子对象自动滚动进入可视区域
 *   lv_obj_scroll_to_view(child_btn, LV_ANIM_ON);
 * @endcode
 *
 * 注意：滚动行为还受到 lv_obj_flag_t 中以下标志的影响：
 *   - LV_OBJ_FLAG_SCROLLABLE       —— 是否允许滚动（基础开关）
 *   - LV_OBJ_FLAG_SCROLL_ELASTIC   —— 是否允许弹性越界回弹
 *   - LV_OBJ_FLAG_SCROLL_MOMENTUM  —— 是否允许惯性滚动
 *   - LV_OBJ_FLAG_SCROLL_CHAIN     —— 是否允许滚动事件传递给父对象
 *   - LV_OBJ_FLAG_SCROLL_ON_FOCUS  —— 聚焦时是否自动滚动到可视区域
 *   - LV_OBJ_FLAG_SCROLL_ONE       —— 是否只滚动一个吸附单元
 */

#ifndef LV_OBJ_SCROLL_H
#define LV_OBJ_SCROLL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_area.h"
#include "lv_anim.h"
#include "../lv_types.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Can't include lv_obj.h because it includes this header file*/

/**
 * 滚动条显示模式枚举
 *
 * 控制滚动条何时可见。滚动条是对象边缘的指示条，提示用户该方向还有更多内容可滚动。
 * 类似于 CSS 中 overflow 的 scrollbar 行为。
 */
typedef enum {
    LV_SCROLLBAR_MODE_OFF,      /**< 永不显示滚动条：即使内容溢出也不显示 */
    LV_SCROLLBAR_MODE_ON,       /**< 始终显示滚动条：即使内容未溢出也显示 */
    LV_SCROLLBAR_MODE_ACTIVE,   /**< 仅在滚动进行中显示：用户正在拖动/滚动时才显示，停止后隐藏 */
    LV_SCROLLBAR_MODE_AUTO,     /**< 自动判断：内容溢出（可以滚动）时才显示，否则隐藏 */
} lv_scrollbar_mode_t;

/**
 * 滚动吸附对齐方式枚举
 *
 * 当父对象启用了滚动吸附（snap）功能后，滚动停止时子对象会自动对齐到指定位置。
 * 类似于手机桌面图标的翻页对齐效果。
 *
 * 吸附功能通过 lv_obj_set_scroll_snap_x() 和 lv_obj_set_scroll_snap_y() 启用。
 * 子对象需要设置 LV_OBJ_FLAG_SNAPPABLE 标志才会参与吸附。
 */
typedef enum {
    LV_SCROLL_SNAP_NONE,    /**< 不吸附：滚动停止后保持当前位置 */
    LV_SCROLL_SNAP_START,   /**< 吸附到起始位置：水平滚动时靠左，垂直滚动时靠上 */
    LV_SCROLL_SNAP_END,     /**< 吸附到末尾位置：水平滚动时靠右，垂直滚动时靠下 */
    LV_SCROLL_SNAP_CENTER   /**< 吸附到中心位置：子对象在可视区域中居中 */
} lv_scroll_snap_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*=====================
 * Setter functions（设置函数）
 *====================*/

/**
 * 设置滚动条的显示模式
 *
 * @param obj   指向控件对象的指针
 * @param mode  滚动条模式，取值为 lv_scrollbar_mode_t 枚举中的任意一项
 *
 * @code
 * // 示例：内容溢出时自动显示滚动条
 * lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_AUTO);
 * @endcode
 */
void lv_obj_set_scrollbar_mode(lv_obj_t * obj, lv_scrollbar_mode_t mode);

/**
 * 设置对象允许的滚动方向
 *
 * 通过位或（|）组合多个方向，限制用户只能在指定方向上滚动。
 * 默认情况下，对象允许所有方向滚动（LV_DIR_ALL）。
 *
 * @param obj   指向控件对象的指针
 * @param dir   滚动方向，取值为 lv_dir_t 枚举值的位或组合
 *              - LV_DIR_TOP    —— 允许向上滚动
 *              - LV_DIR_BOTTOM —— 允许向下滚动
 *              - LV_DIR_LEFT   —— 允许向左滚动
 *              - LV_DIR_RIGHT  —— 允许向右滚动
 *              - LV_DIR_HOR    —— 允许水平方向滚动（LEFT | RIGHT）
 *              - LV_DIR_VER    —— 允许垂直方向滚动（TOP | BOTTOM）
 *              - LV_DIR_ALL    —— 允许所有方向滚动
 *
 * @code
 * // 示例：只允许垂直滚动
 * lv_obj_set_scroll_dir(cont, LV_DIR_VER);
 *
 * // 示例：只允许向下滚动（可向上拖看到更多底部内容）
 * lv_obj_set_scroll_dir(cont, LV_DIR_BOTTOM);
 * @endcode
 */
void lv_obj_set_scroll_dir(lv_obj_t * obj, lv_dir_t dir);

/**
 * 设置水平滚动停止时的吸附对齐方式
 *
 * @param obj   指向控件对象的指针
 * @param align 吸附对齐方式，取值为 lv_scroll_snap_t 枚举中的任意一项
 *
 * @code
 * // 示例：水平滚动吸附到左边
 * lv_obj_set_scroll_snap_x(cont, LV_SCROLL_SNAP_START);
 * @endcode
 */
void lv_obj_set_scroll_snap_x(lv_obj_t * obj, lv_scroll_snap_t align);

/**
 * 设置垂直滚动停止时的吸附对齐方式
 *
 * @param obj   指向控件对象的指针
 * @param align 吸附对齐方式，取值为 lv_scroll_snap_t 枚举中的任意一项
 *
 * @code
 * // 示例：垂直滚动吸附到顶部
 * lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_START);
 * @endcode
 */
void lv_obj_set_scroll_snap_y(lv_obj_t * obj, lv_scroll_snap_t align);

/*=====================
 * Getter functions（获取函数）
 *====================*/

/**
 * 获取当前滚动条显示模式
 *
 * @param obj   指向控件对象的指针
 * @return      当前的滚动条模式（lv_scrollbar_mode_t 枚举值）
 */
lv_scrollbar_mode_t lv_obj_get_scrollbar_mode(const lv_obj_t * obj);

/**
 * 获取对象当前允许的滚动方向
 *
 * @param obj   指向控件对象的指针
 * @return      当前滚动方向的位标志（lv_dir_t 的位或组合）
 */
lv_dir_t lv_obj_get_scroll_dir(const lv_obj_t * obj);

/**
 * 获取水平滚动的吸附对齐方式
 *
 * @param obj   指向控件对象的指针
 * @return      当前水平吸附方式（lv_scroll_snap_t 枚举值）
 */
lv_scroll_snap_t lv_obj_get_scroll_snap_x(const lv_obj_t * obj);

/**
 * 获取垂直滚动的吸附对齐方式
 *
 * @param obj   指向控件对象的指针
 * @return      当前垂直吸附方式（lv_scroll_snap_t 枚举值）
 */
lv_scroll_snap_t lv_obj_get_scroll_snap_y(const lv_obj_t * obj);

/**
 * 获取当前水平滚动偏移量（X 方向）
 *
 * 等价于 lv_obj_get_scroll_left()。表示对象从初始位置向右滚动了多少像素。
 *
 * @param obj   指向可滚动容器的指针
 * @return      当前 X 方向滚动偏移量
 *              - 未滚动时返回 0
 *              - 向右滚动后返回 > 0
 *              - 弹性滚动越界时返回 < 0
 */
int32_t lv_obj_get_scroll_x(const lv_obj_t * obj);

/**
 * 获取当前垂直滚动偏移量（Y 方向）
 *
 * 等价于 lv_obj_get_scroll_top()。表示对象从初始位置向下滚动了多少像素。
 *
 * @param obj   指向可滚动容器的指针
 * @return      当前 Y 方向滚动偏移量
 *              - 未滚动时返回 0
 *              - 向下滚动后返回 > 0
 *              - 弹性滚动越界时返回 < 0
 */
int32_t lv_obj_get_scroll_y(const lv_obj_t * obj);

/**
 * 获取从当前位置到顶部边缘的剩余可向下滚动像素数
 *
 * 表示还可以向下滚动多少像素才能看到内容区域的顶部。
 * 当 LV_OBJ_FLAG_SCROLL_ELASTIC 开启时，弹性越界期间可能返回负值。
 *
 * @param obj   指向可滚动容器的指针
 * @return      距离顶部边缘的剩余可向下滚动像素数
 */
int32_t lv_obj_get_scroll_top(const lv_obj_t * obj);

/**
 * 获取从当前位置到底部边缘的剩余可向上滚动像素数
 *
 * 表示还可以向上滚动多少像素才能看到内容区域的底部。
 * 当 LV_OBJ_FLAG_SCROLL_ELASTIC 开启时，弹性越界期间可能返回负值。
 *
 * @param obj   指向可滚动容器的指针
 * @return      距离底部边缘的剩余可向上滚动像素数
 */
int32_t lv_obj_get_scroll_bottom(const lv_obj_t * obj);

/**
 * 获取从当前位置到左侧边缘的剩余可向右滚动像素数
 *
 * 表示还可以向右滚动多少像素才能看到内容区域的左侧。
 * 当 LV_OBJ_FLAG_SCROLL_ELASTIC 开启时，弹性越界期间可能返回负值。
 *
 * @param obj   指向可滚动容器的指针
 * @return      距离左侧边缘的剩余可向右滚动像素数
 */
int32_t lv_obj_get_scroll_left(const lv_obj_t * obj);

/**
 * 获取从当前位置到右侧边缘的剩余可向左滚动像素数
 *
 * 表示还可以向左滚动多少像素才能看到内容区域的右侧。
 * 当 LV_OBJ_FLAG_SCROLL_ELASTIC 开启时，弹性越界期间可能返回负值。
 *
 * @param obj   指向可滚动容器的指针
 * @return      距离右侧边缘的剩余可向左滚动像素数
 */
int32_t lv_obj_get_scroll_right(const lv_obj_t * obj);

/**
 * 获取滚动动画结束时的最终 X/Y 坐标
 *
 * 如果当前正在播放滚动动画，返回动画结束后的目标位置；
 * 如果没有动画在进行，返回当前位置。
 *
 * @param obj   指向可滚动控件的指针
 * @param end   指向 lv_point_t 结构体的指针，用于存储结果（end->x 和 end->y）
 */
void lv_obj_get_scroll_end(lv_obj_t * obj, lv_point_t * end);

/*=====================
 * Other functions（其他函数）
 *====================*/

/**
 * 按指定像素量滚动
 *
 * 以相对方式滚动对象。dx 和 dy 可以是任意值，包括超出内容边界。
 * 正值 dx 表示向右滚动（看到更多右侧内容），正值 dy 表示向下滚动（看到更多下方内容）。
 *
 * @param obj       指向可滚动控件的指针
 * @param dx        水平方向滚动的像素量
 * @param dy        垂直方向滚动的像素量
 * @param anim_en   LV_ANIM_ON: 带动画滚动；LV_ANIM_OFF: 立即跳到目标位置
 *
 * @note dy = -20 表示向下滚动 20 像素（内容上移），dy = 20 表示向上滚动 20 像素（内容下移）
 */
void lv_obj_scroll_by(lv_obj_t * obj, int32_t dx, int32_t dy, lv_anim_enable_t anim_en);

/**
 * 按指定像素量滚动（有边界限制）
 *
 * 与 lv_obj_scroll_by() 类似，但 dx 和 dy 会被限制在内容区域内，
 * 不会滚动超出内容边界。
 *
 * @param obj       指向可滚动控件的指针
 * @param dx        水平方向滚动的像素量
 * @param dy        垂直方向滚动的像素量
 * @param anim_en   LV_ANIM_ON: 带动画滚动；LV_ANIM_OFF: 立即跳到目标位置
 */
void lv_obj_scroll_by_bounded(lv_obj_t * obj, int32_t dx, int32_t dy, lv_anim_enable_t anim_en);

/**
 * 滚动到指定绝对坐标
 *
 * 将对象滚动到指定的 (x, y) 位置，x 和 y 会被限制在有效的内容区域内。
 *
 * @param obj       指向可滚动控件的指针
 * @param x         目标水平滚动位置
 * @param y         目标垂直滚动位置
 * @param anim_en   LV_ANIM_ON: 带动画滚动；LV_ANIM_OFF: 立即跳到目标位置
 */
void lv_obj_scroll_to(lv_obj_t * obj, int32_t x, int32_t y, lv_anim_enable_t anim_en);

/**
 * 滚动到指定 X 坐标
 *
 * @param obj       指向可滚动控件的指针
 * @param x         目标水平滚动位置
 * @param anim_en   LV_ANIM_ON: 带动画滚动；LV_ANIM_OFF: 立即跳到目标位置
 */
void lv_obj_scroll_to_x(lv_obj_t * obj, int32_t x, lv_anim_enable_t anim_en);

/**
 * 滚动到指定 Y 坐标
 *
 * @param obj       指向可滚动控件的指针
 * @param y         目标垂直滚动位置
 * @param anim_en   LV_ANIM_ON: 带动画滚动；LV_ANIM_OFF: 立即跳到目标位置
 */
void lv_obj_scroll_to_y(lv_obj_t * obj, int32_t y, lv_anim_enable_t anim_en);

/**
 * 将指定对象滚动到父对象的可视区域内
 *
 * 自动滚动 obj 的直接父对象，使 obj 可见。
 * 如果 obj 已经在可视区域内则不做任何操作。
 *
 * @param obj       指向需要被滚动进入可视区域的控件的指针
 * @param anim_en   LV_ANIM_ON: 带动画滚动；LV_ANIM_OFF: 立即跳到目标位置
 *
 * @code
 * // 示例：点击按钮后，确保某个远端子对象可见
 * lv_obj_scroll_to_view(target_item, LV_ANIM_ON);
 * @endcode
 */
void lv_obj_scroll_to_view(lv_obj_t * obj, lv_anim_enable_t anim_en);

/**
 * 递归将指定对象滚动到所有父对象的可视区域内
 *
 * 与 lv_obj_scroll_to_view() 不同，此函数会递归地将所有嵌套的可滚动父对象
 * 都滚动到合适位置，确保 obj 最终在屏幕上可见。
 * 适用于多层嵌套滚动容器的场景（如：页面 > 列表 > 面板 > 按钮）。
 *
 * @param obj       指向需要被滚动进入可视区域的控件的指针
 * @param anim_en   LV_ANIM_ON: 带动画滚动；LV_ANIM_OFF: 立即跳到目标位置
 */
void lv_obj_scroll_to_view_recursive(lv_obj_t * obj, lv_anim_enable_t anim_en);

/**
 * 判断对象当前是否正在滚动
 *
 * @param obj   指向控件对象的指针
 * @return      true: 对象正在被用户拖动/滚动；false: 对象静止
 */
bool lv_obj_is_scrolling(const lv_obj_t * obj);

/**
 * 停止当前对象的滚动动画
 *
 * 立即终止正在进行的滚动动画，对象将停在当前位置。
 *
 * @param obj   指向正在滚动的控件对象的指针
 */
void lv_obj_stop_scroll_anim(const lv_obj_t * obj);

/**
 * 检查并执行滚动吸附
 *
 * 检查 obj 的所有子对象，并根据滚动吸附（snap）设置将 obj 滚动到合适的位置，
 * 使某个子对象吸附到指定对齐位置（起始/末尾/中心）。
 *
 * @param obj       需要检查子对象并执行吸附的控件
 * @param anim_en   LV_ANIM_ON: 带动画吸附；LV_ANIM_OFF: 立即吸附
 */
void lv_obj_update_snap(lv_obj_t * obj, lv_anim_enable_t anim_en);

/**
 * 获取滚动条的绘制区域
 *
 * 获取水平和垂直滚动条在屏幕上的矩形区域，通常用于自定义绘制或碰撞检测。
 *
 * @param obj   指向控件对象的指针
 * @param hor   指向 lv_area_t 的指针，用于存储水平滚动条的区域
 * @param ver   指向 lv_area_t 的指针，用于存储垂直滚动条的区域
 */
void lv_obj_get_scrollbar_area(lv_obj_t * obj, lv_area_t * hor, lv_area_t * ver);

/**
 * 使滚动条区域失效（触发重绘）
 *
 * 标记滚动条所在的区域为需要重绘，通常在滚动条样式或状态改变后调用。
 *
 * @param obj   指向控件对象的指针
 */
void lv_obj_scrollbar_invalidate(lv_obj_t * obj);

/**
 * 修正滚动位置到有效范围
 *
 * 检查内容是否被滚动到了越界位置（如弹性滚动后），并将其调整回正常的有效范围。
 * 通常在内容大小变化后调用，确保滚动位置仍然合法。
 *
 * @param obj       指向控件对象的指针
 * @param anim_en   LV_ANIM_ON: 带动画修正；LV_ANIM_OFF: 立即修正
 */
void lv_obj_readjust_scroll(lv_obj_t * obj, lv_anim_enable_t anim_en);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_OBJ_SCROLL_H*/