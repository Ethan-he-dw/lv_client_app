/**
 * @file lv_grid.c
 *
 * Grid 网格布局的实现文件
 *
 * 本文件实现了类似 CSS Grid 的二维布局系统。
 * 核心流程：
 *   1. lv_grid_init()          - 注册布局回调
 *   2. grid_update()           - 布局主入口，计算网格并定位子对象
 *   3. calc()                  - 计算网格的所有单元格坐标和尺寸
 *   4. calc_cols() / calc_rows() - 分别计算列和行的尺寸（处理 FR、CONTENT、固定值）
 *   5. grid_align()            - 根据对齐方式计算 track 的位置
 *   6. item_repos()            - 将子对象定位到其所属的网格单元格中
 */

/*********************
 *      INCLUDES
 *********************/

#include "../../lvgl_public.h"

#if LV_USE_GRID

#include "../../core/lv_obj_private.h"
#include "../../core/lv_global.h"

/*********************
 *      DEFINES
 *********************/
/** 获取全局布局列表的快捷宏 */
#define layout_list_def LV_GLOBAL_DEFAULT()->layout_list

/**
 * 辅助宏定义
 */
/** 判断是否为 FR（弹性比例）尺寸 */
#define IS_FR(x)       (x >= LV_COORD_MAX - 100)
/** 判断是否为 CONTENT（内容自适应）尺寸 */
#define IS_CONTENT(x)  (x == LV_COORD_MAX - 101)
/** 从特殊尺寸值中提取 FR 比例系数 */
#define GET_FR(x)      (x - (LV_COORD_MAX - 100))

/**********************
 *      TYPEDEFS
 **********************/

/**
 * 子对象重定位提示结构体
 *
 * 用于在遍历子对象时缓存网格的绝对坐标和上一个子对象的行列位置，
 * 避免对每个子对象重复计算网格绝对坐标。
 */
typedef struct {
    uint32_t col;           /**< 上一个子对象的列位置（用于优化查找） */
    uint32_t row;           /**< 上一个子对象的行位置（用于优化查找） */
    lv_point_t grid_abs;    /**< 网格内容区域左上角的绝对坐标 */
} item_repos_hint_t;

/**
 * 网格计算结果结构体
 *
 * 存储计算后的网格布局数据，包括每列/每行的坐标和尺寸。
 */
typedef struct {
    int32_t * x;        /**< 每列的 X 起始坐标数组 */
    int32_t * y;        /**< 每行的 Y 起始坐标数组 */
    int32_t * w;        /**< 每列的宽度数组 */
    int32_t * h;        /**< 每行的高度数组 */
    uint32_t col_num;   /**< 列数 */
    uint32_t row_num;   /**< 行数 */
    int32_t grid_w;     /**< 网格总宽度 */
    int32_t grid_h;     /**< 网格总高度 */
} lv_grid_calc_t;

/**********************
 *  GLOBAL PROTOTYPES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/** Grid 布局的主更新函数，计算网格并定位所有子对象 */
static void grid_update(lv_obj_t * cont, void * user_data);

/** 计算网格中所有单元格的坐标和尺寸 */
static lv_result_t calc(lv_obj_t * obj, lv_grid_calc_t * calc);

/** 释放网格计算结果中动态分配的内存 */
static void calc_free(lv_grid_calc_t * calc);

/** 计算网格中所有列的尺寸 */
static lv_result_t calc_cols(lv_obj_t * cont, lv_grid_calc_t * c);

/** 计算网格中所有行的尺寸 */
static lv_result_t calc_rows(lv_obj_t * cont, lv_grid_calc_t * c);

/** 将子对象定位到其所属的网格单元格中 */
static void item_repos(lv_obj_t * item, lv_grid_calc_t * c, item_repos_hint_t * hint);

/** 根据对齐方式计算 track 的位置，支持正向和反向（RTL）排列 */
static int32_t grid_align(int32_t cont_size, bool auto_size, lv_grid_align_t align, int32_t gap,
                          uint32_t track_num,
                          int32_t * size_array, int32_t * pos_array, bool reverse);

/** 计算描述数组中的 track 数量（遍历到 LV_GRID_TEMPLATE_LAST 为止） */
static uint32_t count_tracks(const int32_t * templ);

/**
 * 获取列描述数组的内联辅助函数
 * @return 指向列描述数组的指针，如果未设置则返回 NULL
 */
static inline const int32_t * get_col_dsc(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_column_dsc_array(obj, LV_PART_MAIN);
}

/**
 * 获取行描述数组的内联辅助函数
 * @return 指向行描述数组的指针，如果未设置则返回 NULL
 */
static inline const int32_t * get_row_dsc(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_row_dsc_array(obj, LV_PART_MAIN);
}

/** 获取子对象在网格中的列位置 */
static inline int32_t get_col_pos(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_cell_column_pos(obj, LV_PART_MAIN);
}

/** 获取子对象在网格中的行位置 */
static inline int32_t get_row_pos(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_cell_row_pos(obj, LV_PART_MAIN);
}

/** 获取子对象的列跨距（占据的列数） */
static inline int32_t get_col_span(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_cell_column_span(obj, LV_PART_MAIN);
}

/** 获取子对象的行跨距（占据的行数） */
static inline int32_t get_row_span(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_cell_row_span(obj, LV_PART_MAIN);
}

/** 获取子对象在单元格内的列方向（水平）对齐方式 */
static inline lv_grid_align_t get_cell_col_align(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_cell_x_align(obj, LV_PART_MAIN);
}

/** 获取子对象在单元格内的行方向（垂直）对齐方式 */
static inline lv_grid_align_t get_cell_row_align(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_cell_y_align(obj, LV_PART_MAIN);
}

/** 获取网格整体的列方向对齐方式 */
static inline lv_grid_align_t get_grid_col_align(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_column_align(obj, LV_PART_MAIN);
}

/** 获取网格整体的行方向对齐方式 */
static inline lv_grid_align_t get_grid_row_align(lv_obj_t * obj)
{
    return lv_obj_get_style_grid_row_align(obj, LV_PART_MAIN);
}

/** 获取对象的水平 margin 之和（左 + 右） */
static inline int32_t get_margin_hor(lv_obj_t * obj)
{
    return lv_obj_get_style_margin_left(obj, LV_PART_MAIN)
           + lv_obj_get_style_margin_right(obj, LV_PART_MAIN);
}

/** 获取对象的垂直 margin 之和（上 + 下） */
static inline int32_t get_margin_ver(lv_obj_t * obj)
{
    return lv_obj_get_style_margin_top(obj, LV_PART_MAIN)
           + lv_obj_get_style_margin_bottom(obj, LV_PART_MAIN);
}

/**
 * 四舍五入的整数除法
 * @param dividend 被除数
 * @param divisor  除数
 * @return 四舍五入后的商
 */
static inline int32_t lv_div_round_closest(int32_t dividend, int32_t divisor)
{
    return (dividend + divisor / 2) / divisor;
}

/**********************
 *  GLOBAL VARIABLES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/
/** 布局跟踪日志宏，仅在启用日志跟踪时输出 */
#if LV_USE_LOG && LV_LOG_TRACE_LAYOUT
    #define LV_TRACE_LAYOUT(...) LV_LOG_TRACE(__VA_ARGS__)
#else
    #define LV_TRACE_LAYOUT(...)
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * 初始化 Grid 布局模块
 *
 * 注册 Grid 布局的更新回调到全局布局列表中。
 * 在 LVGL 初始化时由 lv_init() 内部调用。
 */
void lv_grid_init(void)
{
    layout_list_def[LV_LAYOUT_GRID].callbacks.layout_update_cb = grid_update;
    layout_list_def[LV_LAYOUT_GRID].callbacks.get_min_size_cb = NULL;
    layout_list_def[LV_LAYOUT_GRID].user_data = NULL;
}

/**
 * 设置网格容器的列和行描述数组
 *
 * 通过设置样式属性来指定列和行的描述数组，并将对象标记为使用 Grid 布局。
 */
void lv_obj_set_grid_dsc_array(lv_obj_t * obj, const int32_t col_dsc[], const int32_t row_dsc[])
{
    lv_obj_set_style_grid_column_dsc_array(obj, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(obj, row_dsc, 0);
    lv_obj_set_style_layout(obj, LV_LAYOUT_GRID, 0);
}

/**
 * 设置网格的整体对齐方式
 *
 * 控制列和行 track 在容器内容区域中的对齐和分布方式。
 */
void lv_obj_set_grid_align(lv_obj_t * obj, lv_grid_align_t column_align, lv_grid_align_t row_align)
{
    lv_obj_set_style_grid_column_align(obj, column_align, 0);
    lv_obj_set_style_grid_row_align(obj, row_align, 0);

}

/**
 * 设置子对象在网格中的单元格位置和跨距
 *
 * 指定子对象在父容器网格中的位置、跨距和对齐方式，
 * 并标记父对象的布局为脏，触发重新布局。
 */
void lv_obj_set_grid_cell(lv_obj_t * obj, lv_grid_align_t x_align, int32_t col_pos, int32_t col_span,
                          lv_grid_align_t y_align, int32_t row_pos, int32_t row_span)

{
    lv_obj_set_style_grid_cell_column_pos(obj, col_pos, 0);
    lv_obj_set_style_grid_cell_row_pos(obj, row_pos, 0);
    lv_obj_set_style_grid_cell_x_align(obj, x_align, 0);
    lv_obj_set_style_grid_cell_column_span(obj, col_span, 0);
    lv_obj_set_style_grid_cell_row_span(obj, row_span, 0);
    lv_obj_set_style_grid_cell_y_align(obj, y_align, 0);

    lv_obj_mark_layout_as_dirty(lv_obj_get_parent(obj));
}

/**
 * LV_GRID_FR() 宏的函数包装（用于语言绑定）
 */
int32_t lv_grid_fr(uint8_t x)
{
    return LV_GRID_FR(x);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Grid 布局的主更新函数
 *
 * 这是 Grid 布局的核心入口，由 LVGL 布局引擎调用。
 * 主要流程：
 *   1. 调用 calc() 计算所有单元格的坐标和尺寸
 *   2. 遍历所有子对象，调用 item_repos() 将其定位到正确的单元格
 *   3. 如果容器尺寸为内容自适应，刷新容器尺寸
 *   4. 发送布局变更事件
 *
 * @param cont      Grid 容器对象
 * @param user_data 未使用
 */
static void grid_update(lv_obj_t * cont, void * user_data)
{
    LV_LOG_INFO("update %p container", (void *)cont);
    LV_UNUSED(user_data);

    lv_grid_calc_t c;
    lv_result_t res = calc(cont, &c);
    if(res != LV_RESULT_OK) {
        calc_free(&c);
        return;
    }

    item_repos_hint_t hint;
    lv_memzero(&hint, sizeof(hint));

    /* 计算网格的绝对坐标，用于子对象定位时的偏移计算。
     * 预先计算一次，避免对每个子对象重复计算。 */
    int32_t pad_left = lv_obj_get_style_space_left(cont, LV_PART_MAIN);
    int32_t pad_top = lv_obj_get_style_space_top(cont, LV_PART_MAIN);
    hint.grid_abs.x = pad_left + cont->coords.x1 - lv_obj_get_scroll_x(cont);
    hint.grid_abs.y = pad_top + cont->coords.y1 - lv_obj_get_scroll_y(cont);

    /* 遍历所有子对象，将其定位到各自的网格单元格中 */
    uint32_t i;
    for(i = 0; i < cont->spec_attr->child_cnt; i++) {
        lv_obj_t * item = cont->spec_attr->children[i];
        item_repos(item, &c, &hint);
    }
    calc_free(&c);

    /* 如果容器尺寸为内容自适应，刷新容器尺寸 */
    int32_t w_set = lv_obj_get_style_width(cont, LV_PART_MAIN);
    int32_t h_set = lv_obj_get_style_height(cont, LV_PART_MAIN);
    if(w_set == LV_SIZE_CONTENT || h_set == LV_SIZE_CONTENT) {
        lv_obj_refr_size(cont);
    }

    /* 发送布局变更事件 */
    lv_obj_send_event(cont, LV_EVENT_LAYOUT_CHANGED, NULL);

    LV_TRACE_LAYOUT("finished");
}

/**
 * 计算网格中所有单元格的坐标和尺寸
 *
 * 分别调用 calc_rows() 和 calc_cols() 计算行和列的尺寸，
 * 然后通过 grid_align() 计算各 track 的位置。
 *
 * @param cont      网格容器对象
 * @param calc_out  输出参数，存储计算结果（行列的坐标和尺寸）
 * @return LV_RESULT_OK=计算成功，LV_RESULT_INVALID=计算失败（缺少描述数组等）
 *
 * @note 使用完毕后需要调用 calc_free() 释放动态分配的内存
 */
static lv_result_t calc(lv_obj_t * cont, lv_grid_calc_t * calc_out)
{
    lv_memzero(calc_out, sizeof(lv_grid_calc_t));
    if(lv_obj_get_child(cont, 0) == NULL) {
        return LV_RESULT_INVALID;
    }

    /* 计算行的尺寸 */
    if(calc_rows(cont, calc_out) == LV_RESULT_INVALID) {
        return LV_RESULT_INVALID;
    }
    /* 计算列的尺寸 */
    if(calc_cols(cont, calc_out) == LV_RESULT_INVALID) {
        return LV_RESULT_INVALID;
    }

    /* 获取行列间距 */
    int32_t col_gap = lv_obj_get_style_pad_column(cont, LV_PART_MAIN);
    int32_t row_gap = lv_obj_get_style_pad_row(cont, LV_PART_MAIN);

    /* RTL（从右到左）布局支持 */
    bool rev = lv_obj_get_style_base_dir(cont, LV_PART_MAIN) == LV_BASE_DIR_RTL;

    /* 计算列方向的 track 位置 */
    int32_t w_set = lv_obj_get_style_width(cont, LV_PART_MAIN);
    int32_t h_set = lv_obj_get_style_height(cont, LV_PART_MAIN);
    bool auto_w = w_set == LV_SIZE_CONTENT && !cont->w_layout;
    int32_t cont_w = lv_obj_get_content_width(cont);
    calc_out->grid_w = grid_align(cont_w, auto_w, get_grid_col_align(cont), col_gap, calc_out->col_num, calc_out->w,
                                  calc_out->x, rev);

    /* 计算行方向的 track 位置 */
    bool auto_h = h_set == LV_SIZE_CONTENT && !cont->h_layout;
    int32_t cont_h = lv_obj_get_content_height(cont);
    calc_out->grid_h = grid_align(cont_h, auto_h, get_grid_row_align(cont), row_gap, calc_out->row_num, calc_out->h,
                                  calc_out->y, false);

    LV_ASSERT_MEM_INTEGRITY();
    return LV_RESULT_OK;
}

/**
 * 释放网格计算结果中动态分配的内存
 *
 * @param calc 指向网格计算结果的指针
 */
static void calc_free(lv_grid_calc_t * calc)
{
    lv_free(calc->x);
    lv_free(calc->y);
    lv_free(calc->w);
    lv_free(calc->h);
}

/**
 * 计算网格中所有列的尺寸
 *
 * 处理三种尺寸类型：
 *   - 固定像素值：直接使用该值
 *   - LV_GRID_FR(x)：先统计所有 FR 的总系数，再按比例分配剩余空间
 *   - LV_GRID_CONTENT：遍历该列中的子对象，取最大宽度
 *
 * 支持子网格（subgrid）：如果当前对象没有列描述数组，
 * 则从父对象继承列描述（取父对象网格中对应位置的列定义）。
 *
 * @param cont 网格容器对象
 * @param c    网格计算结果，输出列的宽度和数量
 * @return LV_RESULT_OK=成功，LV_RESULT_INVALID=失败
 */
static lv_result_t calc_cols(lv_obj_t * cont, lv_grid_calc_t * c)
{

    const int32_t * col_templ;
    col_templ = get_col_dsc(cont);

    /* 如果没有列描述数组，检查是否为子网格（subgrid） */
    bool subgrid = false;
    if(col_templ == NULL) {
        lv_obj_t * parent = lv_obj_get_parent(cont);
        if(parent == NULL) {
            LV_LOG_WARN("No column descriptor, and there is no parent for a screen to process subgrid");
            return LV_RESULT_INVALID;
        }

        col_templ = get_col_dsc(parent);
        if(col_templ == NULL) {
            LV_LOG_WARN("No column descriptor found even on the parent");
            return LV_RESULT_INVALID;
        }

        /* 从父对象的列描述中提取对应位置和跨距的列定义 */
        int32_t pos = get_col_pos(cont);
        int32_t span = get_col_span(cont);

        int32_t * col_templ_sub = lv_malloc(sizeof(int32_t) * (span + 1));
        lv_memcpy(col_templ_sub, &col_templ[pos], sizeof(int32_t) * span);
        col_templ_sub[span] = LV_GRID_TEMPLATE_LAST;
        col_templ = col_templ_sub;
        subgrid = true;
    }

    int32_t cont_w = lv_obj_get_content_width(cont);

    c->col_num = count_tracks(col_templ);
    c->x = lv_malloc(sizeof(int32_t) * c->col_num);
    c->w = lv_malloc(sizeof(int32_t) * c->col_num);

    /* 第一步：处理 CONTENT 类型的列，根据子对象内容确定尺寸 */
    uint32_t i;
    for(i = 0; i < c->col_num; i++) {
        int32_t size = LV_COORD_MIN;
        if(IS_CONTENT(col_templ[i])) {
            /* 遍历该列中的子对象（span=1 且 pos 匹配），取最大宽度 */
            uint32_t ci;
            for(ci = 0; ci < lv_obj_get_child_count(cont); ci++) {
                lv_obj_t * item = lv_obj_get_child(cont, ci);
                if((lv_obj_is_ignore_layout(item) || lv_obj_is_hidden(item) || lv_obj_is_floating(item))) continue;
                uint32_t col_span = get_col_span(item);
                if(col_span != 1) continue;

                uint32_t col_pos = get_col_pos(item);
                if(col_pos != i) continue;

                size = LV_MAX(size, lv_obj_get_width(item));
            }
            if(size >= 0) c->w[i] = size;
            else c->w[i] = 0;
        }
    }

    /* 第二步：统计 FR 总系数，计算固定尺寸列的总宽度 */
    uint32_t col_fr_cnt = 0;
    int32_t grid_w = 0;

    for(i = 0; i < c->col_num; i++) {
        int32_t x = col_templ[i];
        if(IS_FR(x)) {
            col_fr_cnt += GET_FR(x);
        }
        else if(IS_CONTENT(x)) {
            grid_w += c->w[i];
        }
        else {
            /* 固定像素值 */
            c->w[i] = x;
            grid_w += x;
        }
    }

    /* 第三步：按 FR 比例分配剩余空间 */
    int32_t col_gap = lv_obj_get_style_pad_column(cont, LV_PART_MAIN);
    cont_w -= col_gap * (c->col_num - 1);
    int32_t free_w = cont_w - grid_w;
    if(free_w < 0) free_w = 0;

    for(i = 0; i < c->col_num && col_fr_cnt; i++) {
        int32_t x = col_templ[i];
        if(IS_FR(x)) {
            int32_t f = GET_FR(x);
            c->w[i] = lv_div_round_closest(free_w * f, col_fr_cnt);
            /* 通过更新剩余的 FR 系数和空间，确保最后一次迭代时
             * f == col_fr_cnt，从而避免舍入误差并用尽所有剩余空间 */
            col_fr_cnt -= f;
            free_w -= c->w[i];
        }
    }

    if(subgrid) {
        lv_free((void *)col_templ);
    }
    return LV_RESULT_OK;
}

/**
 * 计算网格中所有行的尺寸
 *
 * 逻辑与 calc_cols() 完全对称，处理三种尺寸类型：
 *   - 固定像素值：直接使用该值
 *   - LV_GRID_FR(x)：按比例分配剩余空间
 *   - LV_GRID_CONTENT：遍历该行中的子对象，取最大高度
 *
 * 支持子网格（subgrid）：如果当前对象没有行描述数组，
 * 则从父对象继承行描述。
 *
 * @param cont 网格容器对象
 * @param c    网格计算结果，输出行的高度和数量
 * @return LV_RESULT_OK=成功，LV_RESULT_INVALID=失败
 */
static lv_result_t calc_rows(lv_obj_t * cont, lv_grid_calc_t * c)
{
    const int32_t * row_templ;
    row_templ = get_row_dsc(cont);

    /* 如果没有行描述数组，检查是否为子网格（subgrid） */
    bool subgrid = false;
    if(row_templ == NULL) {
        lv_obj_t * parent = lv_obj_get_parent(cont);
        if(parent == NULL) {
            LV_LOG_WARN("No row descriptor, and there is no parent for a screen to process subgrid");
            return LV_RESULT_INVALID;
        }

        row_templ = get_row_dsc(parent);
        if(row_templ == NULL) {
            LV_LOG_WARN("No row descriptor found even on the parent");
            return LV_RESULT_INVALID;
        }

        /* 从父对象的行描述中提取对应位置和跨距的行定义 */
        int32_t pos = get_row_pos(cont);
        int32_t span = get_row_span(cont);

        int32_t * row_templ_sub = lv_malloc(sizeof(int32_t) * (span + 1));
        lv_memcpy(row_templ_sub, &row_templ[pos], sizeof(int32_t) * span);
        row_templ_sub[span] = LV_GRID_TEMPLATE_LAST;
        row_templ = row_templ_sub;
        subgrid = true;
    }

    c->row_num = count_tracks(row_templ);
    c->y = lv_malloc(sizeof(int32_t) * c->row_num);
    c->h = lv_malloc(sizeof(int32_t) * c->row_num);

    /* 第一步：处理 CONTENT 类型的行，根据子对象内容确定尺寸 */
    uint32_t i;
    for(i = 0; i < c->row_num; i++) {
        int32_t size = LV_COORD_MIN;
        if(IS_CONTENT(row_templ[i])) {
            /* 遍历该行中的子对象（span=1 且 pos 匹配），取最大高度 */
            uint32_t ci;
            for(ci = 0; ci < lv_obj_get_child_count(cont); ci++) {
                lv_obj_t * item = lv_obj_get_child(cont, ci);
                if((lv_obj_is_ignore_layout(item) || lv_obj_is_hidden(item) || lv_obj_is_floating(item))) continue;
                uint32_t row_span = get_row_span(item);
                if(row_span != 1) continue;

                uint32_t row_pos = get_row_pos(item);
                if(row_pos != i) continue;

                size = LV_MAX(size, lv_obj_get_height(item));
            }
            if(size >= 0) c->h[i] = size;
            else c->h[i] = 0;
        }
    }

    /* 第二步：统计 FR 总系数，计算固定尺寸行的总高度 */
    uint32_t row_fr_cnt = 0;
    int32_t grid_h = 0;

    for(i = 0; i < c->row_num; i++) {
        int32_t x = row_templ[i];
        if(IS_FR(x)) {
            row_fr_cnt += GET_FR(x);
        }
        else if(IS_CONTENT(x)) {
            grid_h += c->h[i];
        }
        else {
            /* 固定像素值 */
            c->h[i] = x;
            grid_h += x;
        }
    }

    /* 第三步：按 FR 比例分配剩余空间 */
    int32_t row_gap = lv_obj_get_style_pad_row(cont, LV_PART_MAIN);
    int32_t cont_h = lv_obj_get_content_height(cont) - row_gap * (c->row_num - 1);
    int32_t free_h = cont_h - grid_h;
    if(free_h < 0) free_h = 0;

    for(i = 0; i < c->row_num && row_fr_cnt; i++) {
        int32_t x = row_templ[i];
        if(IS_FR(x)) {
            int32_t f = GET_FR(x);
            c->h[i] = lv_div_round_closest(free_h * f, row_fr_cnt);
            /* 通过更新剩余的 FR 系数和空间，确保最后一次迭代时
             * f == row_fr_cnt，从而避免舍入误差并用尽所有剩余空间 */
            row_fr_cnt -= f;
            free_h -= c->h[i];
        }
    }

    if(subgrid) {
        lv_free((void *)row_templ);
    }
    return LV_RESULT_OK;
}

/**
 * 将子对象定位到其所属的网格单元格中
 *
 * 主要流程：
 *   1. 获取子对象的列/行位置和跨距，进行边界检查
 *   2. 计算子对象所属单元格的坐标和尺寸（支持跨行列）
 *   3. 根据对齐方式（START/END/CENTER/STRETCH）计算子对象在单元格内的位置
 *   4. 处理 RTL 布局和 translate 样式的百分比值
 *   5. 移动子对象到计算出的位置
 *
 * @param item  要定位的子对象
 * @param c     网格计算结果（行列的坐标和尺寸数组）
 * @param hint  提示信息（网格绝对坐标，用于优化计算）
 */
static void item_repos(lv_obj_t * item, lv_grid_calc_t * c, item_repos_hint_t * hint)
{
    /* 跳过不可见、被忽略或浮动的子对象 */
    if(lv_obj_is_ignore_layout(item) || lv_obj_is_hidden(item) || lv_obj_is_floating(item)) return;

    /* 获取并验证列跨距 */
    int32_t col_span = get_col_span(item);
    if(col_span <= 0) {
        LV_LOG_WARN("Column span was %" LV_PRId32 ", setting it to 1", col_span);
        col_span = 1;
    }

    /* 获取并验证行跨距 */
    int32_t row_span = get_row_span(item);
    if(row_span <= 0) {
        LV_LOG_WARN("Row span was %" LV_PRId32 ", setting it to 1", row_span);
        row_span = 1;
    }

    /* RTL（从右到左）布局支持 */
    bool rev = lv_obj_get_style_base_dir(lv_obj_get_parent(item), LV_PART_MAIN) == LV_BASE_DIR_RTL;

    /* 获取并验证列位置 */
    int32_t col_pos = get_col_pos(item);
    if(col_pos < 0) {
        LV_LOG_WARN("Column position was %" LV_PRId32 ", setting it to 0", col_pos);
        col_pos = 0;
    }

    if(col_pos >= (int32_t)c->col_num) {
        LV_LOG_WARN("Column position was %" LV_PRId32 ", setting to %" LV_PRId32, col_pos, c->col_num - 1);
        col_pos = c->col_num - 1;
    }

    /* 获取并验证行位置 */
    int32_t row_pos = get_row_pos(item);
    if(row_pos < 0) {
        LV_LOG_WARN("Row position was %" LV_PRId32 ", setting it to 0", row_pos);
        row_pos = 0;
    }

    if(row_pos >= (int32_t)c->row_num) {
        LV_LOG_WARN("Row position was %" LV_PRId32 ", setting to %" LV_PRId32, row_pos, c->row_num - 1);
        row_pos = c->row_num - 1;
    }

    /* 验证列跨距不超出网格范围 */
    if(col_pos + col_span > (int32_t)c->col_num) {
        col_span = c->col_num - col_pos;
        LV_LOG_WARN("Column span is too large, limiting it to %" LV_PRId32, col_span);
    }

    /* 验证行跨距不超出网格范围 */
    if(row_pos + row_span > (int32_t)c->row_num) {
        row_span = c->row_num - row_pos;
        LV_LOG_WARN("Row span is too large, limiting it to %" LV_PRId32, row_span);
    }

    /* 获取子对象在单元格内的对齐方式 */
    lv_grid_align_t col_align = get_cell_col_align(item);
    lv_grid_align_t row_align = get_cell_row_align(item);

    /* 计算子对象所属单元格的列方向范围 */
    int32_t col_x1 = 0;
    int32_t col_x2 = 0;
    int32_t col_w = 0;

    if(rev && col_span > 1) {
        /* RTL 模式下多列跨距的坐标计算（从右向左） */
        col_x1 = c->x[col_pos + col_span - 1];
        col_x2 = c->x[col_pos] + c->w[col_pos];
        col_w = col_x2 - col_x1;
    }
    else {
        col_x1 = c->x[col_pos];
        col_x2 = c->x[col_pos + col_span - 1] + c->w[col_pos + col_span - 1];
        col_w = col_x2 - col_x1;
    }

    /* 计算子对象所属单元格的行方向范围 */
    int32_t row_y1 = c->y[row_pos];
    int32_t row_y2 = c->y[row_pos + row_span - 1] + c->h[row_pos + row_span - 1];
    int32_t row_h = row_y2 - row_y1;

    /* 如果子对象自身设置了 RTL 基础方向，翻转 START/END 对齐 */
    if(lv_obj_get_style_base_dir(item, LV_PART_MAIN) == LV_BASE_DIR_RTL) {
        if(col_align == LV_GRID_ALIGN_START) col_align = LV_GRID_ALIGN_END;
        else if(col_align == LV_GRID_ALIGN_END) col_align = LV_GRID_ALIGN_START;
    }

    int32_t x;
    int32_t y;
    int32_t item_w = lv_area_get_width(&item->coords);
    int32_t item_h = lv_area_get_height(&item->coords);

    col_pos = rev && col_span > 1 ? col_pos + col_span - 1 : col_pos;

    /* 根据列方向对齐方式计算 X 坐标 */
    switch(col_align) {
        default:
        case LV_GRID_ALIGN_START:
            /* 起始对齐：紧贴单元格左边缘 + 左 margin */
            x = c->x[col_pos] + lv_obj_get_style_margin_left(item, LV_PART_MAIN);
            item->w_layout = 0;
            break;
        case LV_GRID_ALIGN_STRETCH:
            /* 拉伸填充：宽度扩展到单元格宽度（减去左右 margin） */
            x = c->x[col_pos] + lv_obj_get_style_margin_left(item, LV_PART_MAIN);
            item_w = col_w - get_margin_hor(item);
            item->w_layout = 1;
            break;
        case LV_GRID_ALIGN_CENTER:
            /* 居中对齐：在单元格内水平居中（考虑左右 margin 的差值） */
            x = c->x[col_pos] + (col_w - item_w) / 2 + (lv_obj_get_style_margin_left(item, LV_PART_MAIN) -
                                                        lv_obj_get_style_margin_right(item, LV_PART_MAIN)) / 2;
            item->w_layout = 0;
            break;
        case LV_GRID_ALIGN_END:
            /* 末尾对齐：紧贴单元格右边缘 - 右 margin */
            x = c->x[col_pos] + col_w - lv_obj_get_width(item) - lv_obj_get_style_margin_right(item, LV_PART_MAIN);
            item->w_layout = 0;
            break;
    }

    /* 根据行方向对齐方式计算 Y 坐标 */
    switch(row_align) {
        default:
        case LV_GRID_ALIGN_START:
            /* 起始对齐：紧贴单元格上边缘 + 上 margin */
            y = c->y[row_pos] + lv_obj_get_style_margin_top(item, LV_PART_MAIN);
            item->h_layout = 0;
            break;
        case LV_GRID_ALIGN_STRETCH:
            /* 拉伸填充：高度扩展到单元格高度（减去上下 margin） */
            y = c->y[row_pos] + lv_obj_get_style_margin_top(item, LV_PART_MAIN);
            item_h = row_h - get_margin_ver(item);
            item->h_layout = 1;
            break;
        case LV_GRID_ALIGN_CENTER:
            /* 居中对齐：在单元格内垂直居中（考虑上下 margin 的差值） */
            y = c->y[row_pos] + (row_h - item_h) / 2 + (lv_obj_get_style_margin_top(item, LV_PART_MAIN) -
                                                        lv_obj_get_style_margin_bottom(item, LV_PART_MAIN)) / 2;
            item->h_layout = 0;
            break;
        case LV_GRID_ALIGN_END:
            /* 末尾对齐：紧贴单元格下边缘 - 下 margin */
            y = c->y[row_pos] + row_h - lv_obj_get_height(item) - lv_obj_get_style_margin_bottom(item, LV_PART_MAIN);
            item->h_layout = 0;
            break;
    }

    /* 如果子对象的尺寸需要更新（STRETCH 模式下可能改变），发送尺寸变更事件 */
    if(lv_obj_get_width(item) != item_w || lv_obj_get_height(item) != item_h) {
        lv_area_t old_coords;
        lv_area_copy(&old_coords, &item->coords);
        lv_obj_invalidate(item);
        lv_area_set_width(&item->coords, item_w);
        lv_area_set_height(&item->coords, item_h);
        lv_obj_invalidate(item);
        lv_obj_send_event(item, LV_EVENT_SIZE_CHANGED, &old_coords);
        lv_obj_send_event(lv_obj_get_parent(item), LV_EVENT_CHILD_CHANGED, item);

    }

    /* 处理 translate 样式的百分比值 */
    int32_t tr_x = lv_obj_get_style_translate_x(item, LV_PART_MAIN);
    int32_t tr_y = lv_obj_get_style_translate_y(item, LV_PART_MAIN);
    int32_t w = lv_obj_get_width(item);
    int32_t h = lv_obj_get_height(item);
    if(LV_COORD_IS_PCT(tr_x)) tr_x = (w * LV_COORD_GET_PCT(tr_x)) / 100;
    if(LV_COORD_IS_PCT(tr_y)) tr_y = (h * LV_COORD_GET_PCT(tr_y)) / 100;

    x += tr_x;
    y += tr_y;

    /* 计算子对象需要移动的偏移量 */
    int32_t diff_x = hint->grid_abs.x + x - item->coords.x1;
    int32_t diff_y = hint->grid_abs.y + y - item->coords.y1;
    if(diff_x || diff_y) {
        /* 移动子对象及其所有子节点 */
        lv_obj_invalidate(item);
        item->coords.x1 += diff_x;
        item->coords.x2 += diff_x;
        item->coords.y1 += diff_y;
        item->coords.y2 += diff_y;
        lv_obj_invalidate(item);
        lv_obj_move_children_by(item, diff_x, diff_y, false);
    }
}

/**
 * 根据对齐方式计算 track 的位置
 *
 * 保持 track 的尺寸不变，根据对齐方式设置每个 track 的起始位置。
 * 同时处理间距（gap）的计算：
 *   - START/CENTER/END：使用原始 gap
 *   - SPACE_BETWEEN：首尾紧贴边界，中间均匀分布
 *   - SPACE_AROUND：每个间隔相同，首尾间隔为中间的一半
 *   - SPACE_EVENLY：所有间隔相同（包括首尾）
 *
 * @param cont_size   容器内容区域的大小（宽度或高度）
 * @param auto_size   true=容器在当前方向为自动尺寸（内容自适应）
 * @param align       对齐方式
 * @param gap         网格间距（会被空间分布模式覆盖）
 * @param track_num   track 数量
 * @param size_array  每个 track 的尺寸数组
 * @param pos_array   输出参数：每个 track 的起始位置数组
 * @param reverse     true=反向排列（RTL 模式）
 * @return 网格的总尺寸
 */
static int32_t grid_align(int32_t cont_size,  bool auto_size, lv_grid_align_t align, int32_t gap,
                          uint32_t track_num,
                          int32_t * size_array, int32_t * pos_array, bool reverse)
{
    int32_t grid_size = 0;
    uint32_t i;

    if(auto_size) {
        /* 自动尺寸模式：第一个 track 从位置 0 开始 */
        pos_array[0] = 0;
    }
    else {
        /* 空间分布对齐模式下，gap 由剩余空间计算得出 */
        if(align == LV_GRID_ALIGN_SPACE_AROUND || align == LV_GRID_ALIGN_SPACE_BETWEEN || align == LV_GRID_ALIGN_SPACE_EVENLY) {
            gap = 0;
            /* 只有一个 track 时，空间分布退化为居中 */
            if(track_num == 1) align = LV_GRID_ALIGN_CENTER;
        }

        /* 计算包含间距在内的网格总尺寸 */
        for(i = 0; i < track_num; i++) {
            grid_size += size_array[i] + gap;
        }
        grid_size -= gap;

        /* 根据对齐方式计算第一个 track 的起始位置 */
        switch(align) {
            case LV_GRID_ALIGN_START:
                pos_array[0] = 0;
                break;
            case LV_GRID_ALIGN_CENTER:
                pos_array[0] = (cont_size - grid_size) / 2;
                break;
            case LV_GRID_ALIGN_END:
                pos_array[0] = cont_size - grid_size;
                break;
            case LV_GRID_ALIGN_SPACE_BETWEEN:
                pos_array[0] = 0;
                gap = (int32_t)(cont_size - grid_size) / (int32_t)(track_num - 1);
                break;
            case LV_GRID_ALIGN_SPACE_AROUND:
                gap = (int32_t)(cont_size - grid_size) / (int32_t)(track_num);
                pos_array[0] = gap / 2;
                break;
            case LV_GRID_ALIGN_SPACE_EVENLY:
                gap = (int32_t)(cont_size - grid_size) / (int32_t)(track_num + 1);
                pos_array[0] = gap;
                break;
            default:
                break;
        }
    }

    /* 根据起始位置、间距和 track 尺寸，计算所有 track 的位置 */
    for(i = 0; i < track_num - 1; i++) {
        pos_array[i + 1] = pos_array[i] + size_array[i] + gap;
    }

    int32_t total_gird_size = pos_array[track_num - 1] + size_array[track_num - 1] - pos_array[0];

    /* RTL 反向排列：将所有 track 的位置翻转 */
    if(reverse) {
        for(i = 0; i < track_num; i++) {
            pos_array[i] = cont_size - pos_array[i] - size_array[i];
        }

    }

    /* 返回网格的总尺寸 */
    return total_gird_size;
}

/**
 * 计算描述数组中的 track 数量
 *
 * 遍历描述数组，直到遇到 LV_GRID_TEMPLATE_LAST 结束标记。
 *
 * @param templ 描述数组（列或行）
 * @return track 的数量
 */
static uint32_t count_tracks(const int32_t * templ)
{
    uint32_t i;
    for(i = 0; templ[i] != LV_GRID_TEMPLATE_LAST; i++);

    return i;
}

#endif /*LV_USE_GRID*/