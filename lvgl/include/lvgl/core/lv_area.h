/**
 * @file lv_area.h
 * @brief LVGL 核心头文件：区域(Area)与坐标(Point)相关类型和函数的定义
 *
 * 本文件定义了 LVGL 中最基础的两个几何概念：
 *   - lv_point_t        : 屏幕上的一个点（x, y 坐标）
 *   - lv_area_t         : 屏幕上的一个矩形区域（由左上角 x1,y1 和右下角 x2,y2 组成）
 *
 * 同时提供了一整套操作这些点/区域的 API，
 * 以及 LVGL 独有的"特殊坐标"编码机制（像素 / 百分比 / LV_SIZE_CONTENT）。
 */

#ifndef LV_AREA_H
#define LV_AREA_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../config/lv_conf_internal.h"   /* 自动生成/引用 lv_conf.h 配置 */
#include "../lv_types.h"                  /* LVGL 公共类型定义（如 lv_value_precise_t 等） */
#include "../misc/lv_math.h"              /* LVGL 数学工具（LV_MIN / LV_MAX / LV_ABS 等宏） */

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**
 * 屏幕上的一个整型坐标点。
 */
typedef struct {
    int32_t x;  /**< 水平坐标（像素） */
    int32_t y;  /**< 垂直坐标（像素） */
} lv_point_t;

/**
 * 高精度坐标点：使用 lv_value_precise_t（可能是 float 或 int32，取决于 LV_USE_FLOAT 配置）存储坐标。
 * 用于需要亚像素精度计算的场景，例如变换/动画插值等。
 */
typedef struct {
    lv_value_precise_t x;  /**< 水平坐标（高精度） */
    lv_value_precise_t y;  /**< 垂直坐标（高精度） */
} lv_point_precise_t;

/**
 * 屏幕上的一个矩形区域。
 * 由左上角 (x1, y1) 和右下角 (x2, y2) 两个对角点确定（均含边界，闭区间）。
 * 注意约定：x1 <= x2, y1 <= y2，即 x2/x1 是"右下"而非"宽/高 + 偏移"。
 */
typedef struct {
    int32_t x1;  /**< 区域左边界（左上角 x） */
    int32_t y1;  /**< 区域上边界（左上角 y） */
    int32_t x2;  /**< 区域右边界（右下角 x） */
    int32_t y2;  /**< 区域下边界（右下角 y） */
} lv_area_t;

/** 对齐方式枚举：用于把对象/区域相对另一个对象/区域进行定位 */

typedef enum {
    LV_ALIGN_DEFAULT = 0,    /**< 默认对齐（通常等价于 TOP_LEFT，保持当前位置） */

    /* ---------- 基准区域内部对齐 ---------- */
    LV_ALIGN_TOP_LEFT,       /**< 左上角对齐 */
    LV_ALIGN_TOP_MID,        /**< 上边中点对齐 */
    LV_ALIGN_TOP_RIGHT,      /**< 右上角对齐 */
    LV_ALIGN_BOTTOM_LEFT,    /**< 左下角对齐 */
    LV_ALIGN_BOTTOM_MID,     /**< 下边中点对齐 */
    LV_ALIGN_BOTTOM_RIGHT,   /**< 右下角对齐 */
    LV_ALIGN_LEFT_MID,       /**< 左边中点对齐 */
    LV_ALIGN_RIGHT_MID,      /**< 右边中点对齐 */
    LV_ALIGN_CENTER,         /**< 正中心对齐 */

    /* ---------- 基准区域外部对齐（紧贴外侧） ---------- */
    LV_ALIGN_OUT_TOP_LEFT,       /**< 贴在基准区域上边外部、左对齐 */
    LV_ALIGN_OUT_TOP_MID,        /**< 贴在基准区域上边外部、水平居中 */
    LV_ALIGN_OUT_TOP_RIGHT,      /**< 贴在基准区域上边外部、右对齐 */
    LV_ALIGN_OUT_BOTTOM_LEFT,    /**< 贴在基准区域下边外部、左对齐 */
    LV_ALIGN_OUT_BOTTOM_MID,     /**< 贴在基准区域下边外部、水平居中 */
    LV_ALIGN_OUT_BOTTOM_RIGHT,   /**< 贴在基准区域下边外部、右对齐 */
    LV_ALIGN_OUT_LEFT_TOP,       /**< 贴在基准区域左边外部、上对齐 */
    LV_ALIGN_OUT_LEFT_MID,       /**< 贴在基准区域左边外部、垂直居中 */
    LV_ALIGN_OUT_LEFT_BOTTOM,    /**< 贴在基准区域左边外部、下对齐 */
    LV_ALIGN_OUT_RIGHT_TOP,      /**< 贴在基准区域右边外部、上对齐 */
    LV_ALIGN_OUT_RIGHT_MID,      /**< 贴在基准区域右边外部、垂直居中 */
    LV_ALIGN_OUT_RIGHT_BOTTOM,   /**< 贴在基准区域右边外部、下对齐 */
} lv_align_t;

/** 方向位掩码枚举：表示四个基本方向，可组合使用（如 HOR = LEFT | RIGHT） */
typedef enum {
    LV_DIR_NONE     = 0x00,                 /**< 无方向 */
    LV_DIR_LEFT     = (1 << 0),             /**< 左 */
    LV_DIR_RIGHT    = (1 << 1),             /**< 右 */
    LV_DIR_TOP      = (1 << 2),             /**< 上 */
    LV_DIR_BOTTOM   = (1 << 3),             /**< 下 */
    LV_DIR_HOR      = LV_DIR_LEFT | LV_DIR_RIGHT,   /**< 水平方向组合 */
    LV_DIR_VER      = LV_DIR_TOP | LV_DIR_BOTTOM,   /**< 垂直方向组合 */
    LV_DIR_ALL      = LV_DIR_HOR | LV_DIR_VER,      /**< 全部方向组合 */
} lv_dir_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * 用四个边界坐标初始化一个区域。
 * @param area_p 指向要初始化的区域
 * @param x1 区域左边界
 * @param y1 区域上边界
 * @param x2 区域右边界
 * @param y2 区域下边界
 */
void lv_area_set(lv_area_t * area_p, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/**
 * 拷贝一个区域（逐字段复制）。
 * @param dest 目标区域指针
 * @param src  源区域指针
 */
inline static void lv_area_copy(lv_area_t * dest, const lv_area_t * src)
{
    dest->x1 = src->x1;
    dest->y1 = src->y1;
    dest->x2 = src->x2;
    dest->y2 = src->y2;
}

/**
 * 获取区域的宽度（像素）。注意：当 x1 == x2 时，宽度为 1（闭区间约定）。
 * @param area_p 指向一个区域
 * @return 区域的宽度
 */
int32_t lv_area_get_width(const lv_area_t * area_p);

/**
 * 获取区域的高度（像素）。注意：当 y1 == y2 时，高度为 1（闭区间约定）。
 * @param area_p 指向一个区域
 * @return 区域的高度
 */
int32_t lv_area_get_height(const lv_area_t * area_p);

/**
 * 设置区域的宽度（重新计算 x2）。注意：w == 1 时会使 x1 == x2。
 * @param area_p 指向要修改的区域
 * @param w 新的宽度（像素）
 */
void lv_area_set_width(lv_area_t * area_p, int32_t w);

/**
 * 设置区域的高度（重新计算 y2）。注意：h == 1 时会使 y1 == y2。
 * @param area_p 指向要修改的区域
 * @param h 新的高度（像素）
 */
void lv_area_set_height(lv_area_t * area_p, int32_t h);

/**
 * 计算区域的面积（宽 × 高）。
 * @param area_p 指向一个区域
 * @return 区域的面积（像素²）
 */
uint32_t lv_area_get_size(const lv_area_t * area_p);

/**
 * 扩大区域：四周各增加 w_extra 水平像素和 h_extra 垂直像素。
 * 注意：数值为负时表示缩小区域。
 * @param area 指向要修改的区域
 * @param w_extra 水平方向额外增加（左右各加）
 * @param h_extra 垂直方向额外增加（上下各加）
 */
void lv_area_increase(lv_area_t * area, int32_t w_extra, int32_t h_extra);

/**
 * 平移区域：整体移动指定的偏移量。
 * @param area 指向要修改的区域
 * @param x_ofs 水平偏移量（像素）
 * @param y_ofs 垂直偏移量（像素）
 */
void lv_area_move(lv_area_t * area, int32_t x_ofs, int32_t y_ofs);

/**
 * 将一个区域对齐到另一个区域上（类似对象对齐）。
 * @param base 基准区域（作为对齐参照）
 * @param to_align 要被对齐的区域（会被修改）
 * @param align 对齐方式（LV_ALIGN_...）
 * @param ofs_x 对齐后的 X 方向附加偏移
 * @param ofs_y 对齐后的 Y 方向附加偏移
 */
void lv_area_align(const lv_area_t * base, lv_area_t * to_align, lv_align_t align, int32_t ofs_x, int32_t ofs_y);

/**
 * 变换（旋转 + 缩放）一个点。
 * @param point  指向要变换的点
 * @param angle  旋转角度，分辨率为 0.1°（例如 123 表示 12.3°）
 * @param scale_x 水平缩放，256 表示 100%
 * @param scale_y 垂直缩放，256 表示 100%
 * @param pivot  变换的旋转/缩放中心点
 * @param zoom_first true: 先缩放后旋转；false: 先旋转后缩放
 */
void lv_point_transform(lv_point_t * point, int32_t angle, int32_t scale_x, int32_t scale_y, const lv_point_t * pivot,
                        bool zoom_first);

/**
 * 变换（旋转 + 缩放）一个点数组。
 * @param points 指向点数组
 * @param count  数组中点的个数
 * @param angle  旋转角度，分辨率为 0.1°（例如 123 表示 12.3°）
 * @param scale_x 水平缩放，256 表示 100%
 * @param scale_y 垂直缩放，256 表示 100%
 * @param pivot  变换的旋转/缩放中心点
 * @param zoom_first true: 先缩放后旋转；false: 先旋转后缩放
 */
void lv_point_array_transform(lv_point_t * points, size_t count, int32_t angle, int32_t scale_x, int32_t scale_y,
                              const lv_point_t * pivot,
                              bool zoom_first);

/**
 * 将高精度点转换为普通整型点（浮点坐标会取整）。
 * @param p 指向高精度点
 * @return 转换后的整型点
 */
lv_point_t lv_point_from_precise(const lv_point_precise_t * p);

/**
 * 将普通整型点转换为高精度点。
 * @param p 指向普通整型点
 * @return 转换后的高精度点
 */
lv_point_precise_t lv_point_to_precise(const lv_point_t * p);

/**
 * 设置一个点的坐标。
 * @param p 指向要设置的点
 * @param x 新的 X 坐标
 * @param y 新的 Y 坐标
 */
void lv_point_set(lv_point_t * p, int32_t x, int32_t y);

/**
 * 设置一个高精度点的坐标。
 * @param p 指向要设置的高精度点
 * @param x 新的 X 坐标（高精度值）
 * @param y 新的 Y 坐标（高精度值）
 */
void lv_point_precise_set(lv_point_precise_t * p, lv_value_precise_t x, lv_value_precise_t y);

/**
 * 交换两个点的坐标值。
 * @param p1 第一个点
 * @param p2 第二个点
 */
void lv_point_swap(lv_point_t * p1, lv_point_t * p2);

/**
 * 交换两个高精度点的坐标值。
 * @param p1 第一个高精度点
 * @param p2 第二个高精度点
 */
void lv_point_precise_swap(lv_point_precise_t * p1, lv_point_precise_t * p2);

/**********************
 *      MACROS
 **********************/

/*
 * ============================================================
 *  LVGL 特殊坐标编码机制（重点理解）
 * ============================================================
 *  LVGL 用一个 int32_t 同时编码三种坐标形式：
 *    1) 普通像素值 (PX)
 *    2) 普通像素负值 (PX_NEG)  —— 为了区分大正数和大负数
 *    3) 特殊值 (SPEC)：包括 LV_SIZE_CONTENT 和 百分比(LV_PCT)
 *
 *  高 3 位（bit29~31）作为"类型标签"，低 29 位作为"数值/载荷"。
 *  ------------------------------------------------------------
 *  比如：LV_PCT(50) 会在 int32 里编码出"50%"，等到布局计算时
 *  再通过 lv_pct_to_px() 结合父对象尺寸换算成真实像素。
 * ============================================================
 */

#define LV_COORD_TYPE_SHIFT    (29U)                /**< 类型标签占用的最高 3 位（29,30,31） */

#define LV_COORD_TYPE_MASK     (3 << LV_COORD_TYPE_SHIFT)  /**< 类型标签位掩码 */
#define LV_COORD_TYPE(x)       ((x) & LV_COORD_TYPE_MASK)  /**< 提取类型标签 */
#define LV_COORD_PLAIN(x)      ((x) & ~LV_COORD_TYPE_MASK) /**< 去掉类型标签，取出纯数值部分 */

#define LV_COORD_TYPE_PX       (0 << LV_COORD_TYPE_SHIFT)  /**< 类型：普通正像素值（正数，最高位为0） */
#define LV_COORD_TYPE_SPEC     (1 << LV_COORD_TYPE_SHIFT)  /**< 类型：特殊值（LV_SIZE_CONTENT / LV_PCT 百分比） */
#define LV_COORD_TYPE_PX_NEG   (3 << LV_COORD_TYPE_SHIFT)  /**< 类型：普通负像素值（负数，最高两位为1） */

#define LV_COORD_IS_PX(x)       (LV_COORD_TYPE(x) == LV_COORD_TYPE_PX || LV_COORD_TYPE(x) == LV_COORD_TYPE_PX_NEG)
                                                                                /**< 判断是否为普通像素值（正或负） */
#define LV_COORD_IS_SPEC(x)     (LV_COORD_TYPE(x) == LV_COORD_TYPE_SPEC)
                                                                                /**< 判断是否为特殊值（百分比 / SIZE_CONTENT） */

#define LV_COORD_SET_SPEC(x)    ((x) | LV_COORD_TYPE_SPEC)
                                                                                /**< 把一个数值标记为特殊值 */

/** 坐标最大值（低 29 位全 1），也是可用像素数值的上限 */
#define LV_COORD_MAX            ((1 << LV_COORD_TYPE_SHIFT) - 1)
/** 坐标最小值（负的 LV_COORD_MAX） */
#define LV_COORD_MIN            (-LV_COORD_MAX)

/* ------------- 特殊坐标值定义 ------------- */

/** 特殊值：表示"内容自适应尺寸"（对象尺寸根据内容自动伸缩） */
#define LV_SIZE_CONTENT         LV_COORD_SET_SPEC(LV_COORD_MAX)

/** 百分比可存储的最大原始值（LV_COORD_MAX - 1，必须为偶数，因为要分成正负两半） */
#define LV_PCT_STORED_MAX       (LV_COORD_MAX - 1)
#if LV_PCT_STORED_MAX % 2 != 0
#error LV_PCT_STORED_MAX should be an even number      /* 编译期校验：必须为偶数 */
#endif

/** 百分比正/负方向各可用的最大值 */
#define LV_PCT_POS_MAX          (LV_PCT_STORED_MAX / 2)

/**
 * 把整数百分比编码为特殊坐标值。
 * 例如：LV_PCT(50) 表示父对象宽度的 50%。
 * 正百分比如 LV_PCT(100)，负百分比如 LV_PCT(-50)。
 */
#define LV_PCT(x)               (LV_COORD_SET_SPEC(((x) < 0 ? (LV_PCT_POS_MAX - LV_MAX((x), -LV_PCT_POS_MAX)) : LV_MIN((x), LV_PCT_POS_MAX))))

/** 判断是否为百分比特殊值（SPEC 类型且载荷不超过最大可存百分比量） */
#define LV_COORD_IS_PCT(x)      ((LV_COORD_IS_SPEC(x) && LV_COORD_PLAIN(x) <= LV_PCT_STORED_MAX))

/** 从编码后的百分比中解出原始百分比数值（含正负号） */
#define LV_COORD_GET_PCT(x)     (LV_COORD_PLAIN(x) > LV_PCT_POS_MAX ? LV_PCT_POS_MAX - LV_COORD_PLAIN(x) : LV_COORD_PLAIN(x))

/* 导出常量到脚本语言（micropython/lua 绑定用），C 代码可忽略 */
LV_EXPORT_CONST_INT(LV_COORD_MAX);
LV_EXPORT_CONST_INT(LV_COORD_MIN);
LV_EXPORT_CONST_INT(LV_SIZE_CONTENT);

/**
 * 将百分比值转换为特殊编码的坐标值（等效于 LV_PCT 宏）。
 * 百分比值的取值范围是 0..1000（内部特殊范围存储）。
 * @param x 百分比数值（0~1000）
 * @return 编码了百分比信息的坐标值
 */
int32_t lv_pct(int32_t x);

/**
 * 将编码后的百分比坐标换算为真正的像素值。
 * @param v    特殊编码的坐标（如 LV_PCT(50) 的结果）
 * @param base 百分比换算的基准尺寸（通常是父对象宽/高）
 * @return 换算后的像素值
 */
int32_t lv_pct_to_px(int32_t v, int32_t base);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
