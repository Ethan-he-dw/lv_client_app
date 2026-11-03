/**
 * @file lv_flex.c
 *
 * Flex 弹性布局的实现文件
 *
 * 本文件实现了类似 CSS Flexbox 的一维布局系统。
 * 核心流程：
 *   1. lv_flex_init()          - 注册布局回调
 *   2. flex_update()           - 布局主入口，遍历 track 并定位子对象
 *   3. find_track_end()        - 确定当前 track（行/列）包含哪些子对象
 *   4. children_repos()        - 将同一 track 内的子对象定位到正确位置
 *   5. place_content()         - 根据对齐方式计算起始位置和间距
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../core/lv_obj_private.h"

#if LV_USE_FLEX

#include "../../core/lv_global.h"
/*********************
 *      DEFINES
 *********************/
/** 获取全局布局列表的快捷宏 */
#define layout_list_def LV_GLOBAL_DEFAULT()->layout_list

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Flex 容器的布局配置信息
 *
 * 从容器对象的样式属性中提取，描述了 Flex 布局的排列方向、
 * 换行、反向以及各轴的对齐方式。
 */
typedef struct {
    lv_flex_align_t main_place;   /**< 主轴对齐方式（行方向为水平，列方向为垂直） */
    lv_flex_align_t cross_place;  /**< 交叉轴对齐方式（垂直于主轴方向） */
    lv_flex_align_t track_place;  /**< 多行/列（track）的对齐方式 */
    uint8_t row : 1;              /**< 1=行方向（水平排列），0=列方向（垂直排列） */
    uint8_t wrap : 1;             /**< 1=启用换行，0=不换行 */
    uint8_t rev : 1;              /**< 1=反向排列，0=正向排列 */
} flex_t;

/**
 * 弹性增长子对象的描述符
 *
 * 记录每个具有 flex grow 属性的子对象的尺寸约束和增长值，
 * 用于在空间分配迭代中计算最终尺寸。
 */
typedef struct {
    lv_obj_t * item;          /**< 指向子对象的指针 */
    int32_t min_size;         /**< 子对象的最小尺寸（对应样式 min_width 或 min_height） */
    int32_t max_size;         /**< 子对象的最大尺寸（对应样式 max_width 或 max_height） */
    int32_t final_size;       /**< 经过弹性分配后计算出的最终尺寸 */
    uint32_t grow_value;      /**< flex grow 值，越大分配的剩余空间越多 */
    uint32_t clamped : 1;     /**< 1=该子对象的尺寸已被 clamp 到 min/max 范围内 */
} grow_dsc_t;

/**
 * Track（行/列）描述符
 *
 * 一个 track 是 Flex 容器中的一行（行方向）或一列（列方向）。
 * 记录该 track 的尺寸信息、包含的子对象数量，
 * 以及弹性增长子对象的分配数据。
 */
typedef struct {
    int32_t track_cross_size;       /**< track 在交叉轴方向的尺寸（行方向为高度，列方向为宽度） */
    int32_t track_main_size;        /**< track 在主轴方向的总尺寸（用于所有子对象） */
    int32_t track_fix_main_size;    /**< 非弹性增长子对象占用的主轴尺寸（固定部分） */
    int32_t track_grow_min_size;    /**< 弹性增长子对象的最小尺寸之和 */
    uint32_t item_cnt;              /**< track 中可见子对象的数量 */
    grow_dsc_t * grow_dsc;          /**< 弹性增长子对象描述符数组 */
    uint32_t grow_item_cnt;         /**< 弹性增长子对象的数量 */
    uint32_t grow_dsc_calc : 1;     /**< 1=需要计算 grow 描述符（第二遍遍历时才设为 1） */
} track_t;

/**********************
 *  GLOBAL PROTOTYPES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/** 计算 Flex 容器在主轴方向的最小尺寸 */
static bool calc_min_size(lv_obj_t * cont, int32_t * req_size, bool width, void * user_data);

/** Flex 布局的主更新函数，遍历所有 track 并定位子对象 */
static void flex_update(lv_obj_t * cont, void * user_data);

/** 查找当前 track 的最后一个子对象，返回下一个 track 的起始子对象 ID */
static int32_t find_track_end(lv_obj_t * cont, flex_t * f, int32_t item_start_id, int32_t max_main_size,
                              int32_t item_gap, track_t * t);

/** 将同一 track 内的子对象定位到正确的位置 */
static void children_repos(lv_obj_t * cont, flex_t * f, int32_t item_first_id, int32_t item_last_id, int32_t abs_x,
                           int32_t abs_y, int32_t max_main_size, int32_t item_gap, track_t * t);

/** 根据对齐方式计算起始位置和间距 */
static void place_content(lv_flex_align_t place, int32_t max_size, int32_t content_size, int32_t item_cnt,
                          int32_t * start_pos, int32_t * gap);

/** 获取下一个子对象（根据正向/反向遍历方向） */
static lv_obj_t * get_next_item(lv_obj_t * cont, bool rev, int32_t * item_id);

/** 获取对象宽度（包含左右 margin） */
static int32_t lv_obj_get_width_with_margin(const lv_obj_t * obj);

/** 获取对象高度（包含上下 margin） */
static int32_t lv_obj_get_height_with_margin(const lv_obj_t * obj);

/**
 * 四舍五入的整数除法
 * @param dividend 被除数
 * @param divisor  除数
 * @return 四舍五入后的商
 */
static inline int32_t div_round_closest(int32_t dividend, int32_t divisor)
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

/*=====================
 * Setter functions
 *====================*/

/**
 * 初始化 Flex 布局模块
 *
 * 注册 Flex 布局的更新回调和最小尺寸计算回调到全局布局列表中。
 * 在 LVGL 初始化时由 lv_init() 内部调用。
 */
void lv_flex_init(void)
{
    layout_list_def[LV_LAYOUT_FLEX].callbacks.layout_update_cb = flex_update;
    layout_list_def[LV_LAYOUT_FLEX].callbacks.get_min_size_cb = calc_min_size;
    layout_list_def[LV_LAYOUT_FLEX].user_data = NULL;
}

/**
 * 设置 Flex 容器的排列方向
 *
 * 通过设置样式属性来指定子对象的流动方向，并将对象标记为使用 Flex 布局。
 */
void lv_obj_set_flex_flow(lv_obj_t * obj, lv_flex_flow_t flow)
{
    lv_obj_set_style_flex_flow(obj, flow, 0);
    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, 0);
}

/**
 * 设置 Flex 容器的对齐方式
 *
 * 同时设置主轴、交叉轴和多行/列的对齐方式，并将对象标记为使用 Flex 布局。
 */
void lv_obj_set_flex_align(lv_obj_t * obj, lv_flex_align_t main_place, lv_flex_align_t cross_place,
                           lv_flex_align_t track_place)
{
    lv_obj_set_style_flex_main_place(obj, main_place, 0);
    lv_obj_set_style_flex_cross_place(obj, cross_place, 0);
    lv_obj_set_style_flex_track_place(obj, track_place, 0);
    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, 0);
}

/**
 * 设置子对象的弹性增长比例（flex grow）
 *
 * 设置后会标记父对象的布局为脏，触发重新布局。
 */
void lv_obj_set_flex_grow(lv_obj_t * obj, uint8_t grow)
{
    lv_obj_set_style_flex_grow(obj, grow, 0);
    lv_obj_t * parent = lv_obj_get_parent(obj);
    if(parent) lv_obj_mark_layout_as_dirty(parent);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * 从容器对象的样式属性中提取 Flex 布局配置
 *
 * @param cont 容器对象
 * @param f    输出参数，用于存储提取的配置信息
 */
static void get_flex_info(lv_obj_t * cont, flex_t * f)
{
    lv_flex_flow_t flow = lv_obj_get_style_flex_flow(cont, LV_PART_MAIN);
    f->row = flow & LV_FLEX_COLUMN ? 0 : 1;   /* 判断是行方向还是列方向 */
    f->wrap = flow & LV_FLEX_WRAP ? 1 : 0;     /* 判断是否启用换行 */
    f->rev = flow & LV_FLEX_REVERSE ? 1 : 0;   /* 判断是否反向排列 */
    f->main_place = lv_obj_get_style_flex_main_place(cont, LV_PART_MAIN);
    f->cross_place = lv_obj_get_style_flex_cross_place(cont, LV_PART_MAIN);
    f->track_place = lv_obj_get_style_flex_track_place(cont, LV_PART_MAIN);
}

/**
 * 计算 Flex 容器在主轴方向所需的最小尺寸
 *
 * 遍历所有 track，计算每个 track 的固定尺寸和弹性增长子对象的最小尺寸之和，
 * 取所有 track 中最大的作为容器的最小尺寸。
 *
 * @param cont      容器对象
 * @param req_size  输出参数，存储计算出的最小尺寸
 * @param width     true=计算宽度方向的最小尺寸，false=计算高度方向
 * @param user_data 未使用
 * @return true=已计算出最小尺寸，false=该方向不适用 Flex 最小尺寸计算
 */
static bool calc_min_size(lv_obj_t * cont, int32_t * req_size, bool width, void * user_data)
{
    LV_UNUSED(user_data);

    if(cont->spec_attr == NULL)
        return false;

    flex_t f;
    get_flex_info(cont, &f);

    /* 只在主轴方向计算最小尺寸 */
    if(f.row != width) {
        return false;
    }

    /* 内容自适应尺寸时不能换行 */
    f.wrap = false;

    *req_size = 0;

    /* 获取子对象之间的间距 */
    int32_t item_gap =
        f.row ? lv_obj_get_style_pad_column(cont, LV_PART_MAIN) : lv_obj_get_style_pad_row(cont, LV_PART_MAIN);

    /* 获取容器的起始和结束空间（padding） */
    int32_t cont_space_start =
        (f.row ? lv_obj_get_style_space_left(cont, LV_PART_MAIN) : lv_obj_get_style_space_top(cont, LV_PART_MAIN));
    int32_t cont_space_end =
        (f.row ? lv_obj_get_style_space_right(cont, LV_PART_MAIN) : lv_obj_get_style_space_bottom(cont, LV_PART_MAIN));

    int32_t track_first_item = f.rev ? cont->spec_attr->child_cnt - 1 : 0;
    int32_t next_track_first_item;

    /* 遍历所有 track，找出最大的 track 尺寸作为最小尺寸 */
    while(track_first_item < (int32_t)cont->spec_attr->child_cnt && track_first_item >= 0) {
        /* 查找下一个 track 的起始子对象 */
        track_t t;
        t.grow_dsc_calc = 0;
        next_track_first_item = find_track_end(cont, &f, track_first_item, 0, item_gap, &t);

        /* track 最小尺寸 = 固定尺寸 + 弹性增长子对象的最小尺寸 */
        int32_t req_track_size = t.track_fix_main_size + t.track_grow_min_size;

        track_first_item = next_track_first_item;

        *req_size = LV_MAX(*req_size, req_track_size);
    }

    /* 加上容器首尾的 padding 空间 */
    *req_size += (cont_space_start + cont_space_end);

    return true;
}

/**
 * Flex 布局的主更新函数
 *
 * 这是 Flex 布局的核心入口，由 LVGL 布局引擎调用。
 * 主要流程：
 *   1. 获取容器的 Flex 配置（方向、换行、对齐等）
 *   2. 计算主轴可用空间、间距等参数
 *   3. 如果需要 track 级别对齐，先遍历一遍计算所有 track 的总尺寸
 *   4. 遍历每个 track，调用 find_track_end() 确定 track 范围，
 *      再调用 children_repos() 定位子对象
 *   5. 如果容器尺寸为内容自适应，刷新容器尺寸
 *   6. 发送布局变更事件
 *
 * @param cont      Flex 容器对象
 * @param user_data 未使用
 */
static void flex_update(lv_obj_t * cont, void * user_data)
{
    LV_LOG_INFO("update %p container", (void *)cont);
    LV_UNUSED(user_data);

    flex_t f;
    get_flex_info(cont, &f);

    /* RTL（从右到左）布局支持 */
    bool rtl = lv_obj_get_style_base_dir(cont, LV_PART_MAIN) == LV_BASE_DIR_RTL;

    /* track 间距（行方向时为行间距，列方向时为列间距） */
    int32_t track_gap =
        !f.row ? lv_obj_get_style_pad_column(cont, LV_PART_MAIN) : lv_obj_get_style_pad_row(cont, LV_PART_MAIN);
    /* 子对象间距（行方向时为列间距，列方向时为行间距） */
    int32_t item_gap =
        f.row ? lv_obj_get_style_pad_column(cont, LV_PART_MAIN) : lv_obj_get_style_pad_row(cont, LV_PART_MAIN);
    /* 主轴方向的可用内容区域大小 */
    int32_t max_main_size = (f.row ? lv_obj_get_content_width(cont) : lv_obj_get_content_height(cont));
    /* 容器内容区域的绝对起始坐标（考虑 padding 和滚动偏移） */
    int32_t abs_y = cont->coords.y1 + lv_obj_get_style_space_top(cont, LV_PART_MAIN) - lv_obj_get_scroll_y(cont);
    int32_t abs_x = cont->coords.x1 + lv_obj_get_style_space_left(cont, LV_PART_MAIN) - lv_obj_get_scroll_x(cont);

    lv_flex_align_t track_cross_place = f.track_place;
    /* cross_pos 指向交叉轴方向的位置指针 */
    int32_t * cross_pos = (f.row ? &abs_y : &abs_x);

    int32_t w_set = lv_obj_get_style_width(cont, LV_PART_MAIN);
    int32_t h_set = lv_obj_get_style_height(cont, LV_PART_MAIN);

    /* 如果容器在交叉轴方向是内容自适应（LV_SIZE_CONTENT），
     * 则子对象之间的间距会被压缩，对齐效果等同于 START */
    if((f.row && h_set == LV_SIZE_CONTENT && cont->h_layout == 0) ||
       (!f.row && w_set == LV_SIZE_CONTENT && cont->w_layout == 0)) {
        track_cross_place = LV_FLEX_ALIGN_START;
    }

    /* RTL 模式下，列方向的 START/END 对齐需要翻转 */
    if(rtl && !f.row) {
        if(track_cross_place == LV_FLEX_ALIGN_START)
            track_cross_place = LV_FLEX_ALIGN_END;
        else if(track_cross_place == LV_FLEX_ALIGN_END)
            track_cross_place = LV_FLEX_ALIGN_START;
    }

    int32_t total_track_cross_size = 0;  /* 所有 track 在交叉轴方向的总尺寸 */
    int32_t gap = 0;                      /* track 之间的额外间距 */
    uint32_t track_cnt = 0;               /* track 总数 */
    int32_t track_first_item;             /* 当前 track 的第一个子对象 ID */
    int32_t next_track_first_item;        /* 下一个 track 的第一个子对象 ID */

    /* 第一遍遍历：如果 track 级别对齐不是 START，则需要先计算所有 track 的总尺寸
     * 以便通过 place_content() 确定起始位置和间距 */
    if(track_cross_place != LV_FLEX_ALIGN_START) {
        track_first_item = f.rev ? cont->spec_attr->child_cnt - 1 : 0;
        track_t t;
        while(track_first_item < (int32_t)cont->spec_attr->child_cnt && track_first_item >= 0) {
            /* 查找下一个 track 的起始子对象 */
            t.grow_dsc_calc = 0;
            next_track_first_item = find_track_end(cont, &f, track_first_item, max_main_size, item_gap, &t);
            total_track_cross_size += t.track_cross_size + track_gap;
            track_cnt++;
            track_first_item = next_track_first_item;
        }

        if(track_cnt)
            total_track_cross_size -= track_gap; /* 最后一个 track 不需要间距 */

        /* 根据对齐方式计算起始位置和间距 */
        int32_t max_cross_size = (f.row ? lv_obj_get_content_height(cont) : lv_obj_get_content_width(cont));
        place_content(track_cross_place, max_cross_size, total_track_cross_size, track_cnt, cross_pos, &gap);
    }

    /* 第二遍遍历：定位每个 track 中的子对象 */
    track_first_item = f.rev ? cont->spec_attr->child_cnt - 1 : 0;

    /* RTL 模式下列方向需要从末尾开始计算位置 */
    if(rtl && !f.row) {
        *cross_pos += total_track_cross_size;
    }

    while(track_first_item < (int32_t)cont->spec_attr->child_cnt && track_first_item >= 0) {
        track_t t;
        t.grow_dsc_calc = 1;  /* 第二遍遍历需要计算 grow 描述符 */
        /* 查找当前 track 的范围 */
        next_track_first_item = find_track_end(cont, &f, track_first_item, max_main_size, item_gap, &t);

        /* RTL 列方向：向前移动交叉轴位置 */
        if(rtl && !f.row) {
            *cross_pos -= t.track_cross_size;
        }
        /* 定位当前 track 内的所有子对象 */
        children_repos(cont, &f, track_first_item, next_track_first_item, abs_x, abs_y, max_main_size, item_gap, &t);
        track_first_item = next_track_first_item;
        lv_free(t.grow_dsc);
        t.grow_dsc = NULL;
        /* 移动到下一个 track 的起始位置 */
        if(rtl && !f.row) {
            *cross_pos -= gap + track_gap;
        }
        else {
            *cross_pos += t.track_cross_size + gap + track_gap;
        }
    }
    LV_ASSERT_MEM_INTEGRITY();

    /* 如果容器尺寸为内容自适应，刷新容器尺寸 */
    if(w_set == LV_SIZE_CONTENT || h_set == LV_SIZE_CONTENT) {
        lv_obj_refr_size(cont);
    }

    /* 发送布局变更事件 */
    lv_obj_send_event(cont, LV_EVENT_LAYOUT_CHANGED, NULL);

    LV_TRACE_LAYOUT("finished");
}

/**
 * 查找当前 track（行/列）的范围
 *
 * 从 item_start_id 开始，沿着主轴方向遍历子对象，直到：
 *   - 主轴空间用完（需要换行时）
 *   - 遇到要求在新 track 中显示的子对象（lv_obj_is_flex_in_new_track）
 *   - 没有更多的子对象
 *
 * 同时统计 track 中固定尺寸子对象的总尺寸和弹性增长子对象的最小尺寸。
 *
 * @param cont          Flex 容器对象
 * @param f             Flex 配置信息
 * @param item_start_id 当前 track 的起始子对象 ID
 * @param max_main_size 主轴方向的最大可用空间
 * @param item_gap      子对象之间的间距
 * @param t             输出参数，存储 track 的统计信息
 * @return 下一个 track 的起始子对象 ID
 */
static int32_t find_track_end(lv_obj_t * cont, flex_t * f, int32_t item_start_id, int32_t max_main_size,
                              int32_t item_gap, track_t * t)
{
    int32_t w_set = lv_obj_get_style_width(cont, LV_PART_MAIN);
    int32_t h_set = lv_obj_get_style_height(cont, LV_PART_MAIN);

    /* 如果容器是另一个 Flex 容器的子对象且有 grow 值，
     * 则其主轴尺寸已由父容器确定，可以忽略 LV_SIZE_CONTENT 的限制 */
    lv_obj_t * parent = lv_obj_get_parent(cont);
    bool ignore_size_content = false;
    if(parent != NULL) {
        bool parent_is_flex = lv_obj_get_style_layout(parent, LV_PART_MAIN) == LV_LAYOUT_FLEX;
        uint8_t grow_value = lv_obj_get_style_flex_grow(cont, LV_PART_MAIN);
        ignore_size_content = parent_is_flex && (grow_value > 0);
    }

    /* 如果容器主轴尺寸为内容自适应，则不能换行（除非被上面的逻辑覆盖） */
    if(f->wrap && ((f->row && w_set == LV_SIZE_CONTENT) || (!f->row && h_set == LV_SIZE_CONTENT)) &&
       !ignore_size_content) {
        f->wrap = false;
    }

    /* 根据方向选择获取主轴/交叉轴尺寸的函数指针 */
    int32_t (*get_main_size)(const lv_obj_t *) = (f->row ? lv_obj_get_width_with_margin : lv_obj_get_height_with_margin);
    int32_t (*get_cross_size)(const lv_obj_t *) =
        (!f->row ? lv_obj_get_width_with_margin : lv_obj_get_height_with_margin);

    /* 初始化 track 统计信息 */
    t->track_main_size = 0;
    t->track_fix_main_size = 0;
    t->track_grow_min_size = 0;
    t->grow_item_cnt = 0;
    t->track_cross_size = 0;
    t->item_cnt = 0;
    t->grow_dsc = NULL;

    int32_t item_id = item_start_id;
    lv_obj_t * item = lv_obj_get_child(cont, item_id);
    bool first_item = true;
    while(item) {
        /* 跳过不可见、被忽略或浮动的子对象 */
        if(!(lv_obj_is_ignore_layout(item) || lv_obj_is_hidden(item) || lv_obj_is_floating(item))) {
            /* 如果子对象要求在新 track 中显示，则结束当前 track */
            if(!first_item && lv_obj_is_flex_in_new_track(item))
                break;

            uint8_t grow_value = lv_obj_get_style_flex_grow(item, LV_PART_MAIN);
            if(grow_value) {
                /* 弹性增长子对象：记录最小尺寸，用于空间分配 */
                int32_t min_size = f->row ? lv_obj_calc_dynamic_width(item, LV_STYLE_MIN_WIDTH)
                                   : lv_obj_calc_dynamic_height(item, LV_STYLE_MIN_HEIGHT);
                int32_t req_size = min_size;
                if(!first_item) {
                    req_size += item_gap; /* 第一个子对象前面不需要间距 */
                }

                /* 如果加上这个子对象后超出主轴空间，则换行 */
                if(f->wrap && t->track_fix_main_size + t->track_grow_min_size + req_size > max_main_size)
                    break;

                t->track_grow_min_size += min_size;
                if(!first_item) {
                    t->track_fix_main_size += item_gap; /* 间距始终从固定空间中扣除 */
                }

                t->grow_item_cnt++;

                /* 第二遍遍历时，分配并填充 grow 描述符 */
                if(t->grow_dsc_calc) {
                    grow_dsc_t * new_dsc = lv_realloc(t->grow_dsc, sizeof(grow_dsc_t) * (t->grow_item_cnt));
                    LV_ASSERT_MALLOC(new_dsc);
                    if(new_dsc == NULL)
                        return item_id;

                    int32_t max_size = f->row ? lv_obj_calc_dynamic_width(item, LV_STYLE_MAX_WIDTH)
                                       : lv_obj_calc_dynamic_height(item, LV_STYLE_MAX_HEIGHT);

                    new_dsc[t->grow_item_cnt - 1].item = item;
                    new_dsc[t->grow_item_cnt - 1].min_size = min_size;
                    new_dsc[t->grow_item_cnt - 1].max_size = max_size;
                    new_dsc[t->grow_item_cnt - 1].grow_value = grow_value;
                    new_dsc[t->grow_item_cnt - 1].clamped = 0;

                    t->grow_dsc = new_dsc;
                }
            }
            else {
                /* 固定尺寸子对象：直接使用当前尺寸 */
                int32_t item_size = get_main_size(item);
                int32_t req_size = item_size;
                if(!first_item)
                    req_size += item_gap; /* 第一个子对象前面不需要间距 */
                /* 如果加上这个子对象后超出主轴空间，则换行 */
                if(f->wrap && t->track_fix_main_size + t->track_grow_min_size + req_size > max_main_size)
                    break;
                t->track_fix_main_size += req_size;
            }

            first_item = false;
            /* 更新 track 的交叉轴尺寸（取所有子对象中的最大值） */
            t->track_cross_size = LV_MAX(get_cross_size(item), t->track_cross_size);
            t->item_cnt++;
        }

        /* 移动到下一个子对象（根据正向/反向） */
        item_id += f->rev ? -1 : +1;
        if(item_id < 0)
            break;
        item = lv_obj_get_child(cont, item_id);
    }

    /* 如果至少有一个弹性增长子对象，track 占满整个主轴空间；
     * 否则使用固定尺寸之和 */
    t->track_main_size = t->grow_item_cnt ? max_main_size : t->track_fix_main_size;

    /* 确保每个 track 至少包含一个子对象（避免空 track 或首项过大时死循环） */
    if(item && item_id == item_start_id) {
        item = cont->spec_attr->children[item_id];
        get_next_item(cont, f->rev, &item_id);
        if(item) {
            t->track_cross_size = get_cross_size(item);
            t->track_main_size = get_main_size(item);
            t->item_cnt = 1;
        }
    }

    return item_id;
}

/**
 * 将同一 track 内的子对象定位到正确的位置
 *
 * 主要流程：
 *   1. 计算弹性增长子对象的最终尺寸（考虑 min/max 约束的迭代分配）
 *   2. 通过 place_content() 计算主轴起始位置和间距
 *   3. 遍历 track 中的每个子对象，根据对齐方式计算交叉轴位置
 *   4. 移动子对象到计算出的位置，处理 RTL 和 translate 样式
 *
 * @param cont          Flex 容器对象
 * @param f             Flex 配置信息
 * @param item_first_id track 中第一个子对象的 ID
 * @param item_last_id  track 中最后一个子对象之后的 ID（不包含）
 * @param abs_x         容器内容区域的绝对 X 起始坐标
 * @param abs_y         容器内容区域的绝对 Y 起始坐标
 * @param max_main_size 主轴方向的最大可用空间
 * @param item_gap      子对象之间的间距
 * @param t             track 的统计信息（包含 grow 描述符）
 */
static void children_repos(lv_obj_t * cont, flex_t * f, int32_t item_first_id, int32_t item_last_id, int32_t abs_x,
                           int32_t abs_y, int32_t max_main_size, int32_t item_gap, track_t * t)
{
    /* 根据方向选择区域尺寸操作函数 */
    void (*area_set_main_size)(lv_area_t *, int32_t) = (f->row ? lv_area_set_width : lv_area_set_height);
    int32_t (*area_get_main_size)(const lv_area_t *) = (f->row ? lv_area_get_width : lv_area_get_height);
    int32_t (*area_get_cross_size)(const lv_area_t *) = (!f->row ? lv_area_get_width : lv_area_get_height);

    /* 根据方向选择 margin 获取函数 */
    typedef int32_t (*margin_func_t)(const lv_obj_t *, lv_part_t);
    margin_func_t get_margin_main_start = (f->row ? lv_obj_get_style_margin_left : lv_obj_get_style_margin_top);
    margin_func_t get_margin_main_end = (f->row ? lv_obj_get_style_margin_right : lv_obj_get_style_margin_bottom);
    margin_func_t get_margin_cross_start = (!f->row ? lv_obj_get_style_margin_left : lv_obj_get_style_margin_top);
    margin_func_t get_margin_cross_end = (!f->row ? lv_obj_get_style_margin_right : lv_obj_get_style_margin_bottom);

    /*===== 第一步：计算弹性增长子对象的最终尺寸 =====*/
    uint32_t i;
    bool grow_reiterate = true;
    /* 使用迭代算法：每次将超出 min/max 范围的子对象标记为 clamped，
     * 然后重新分配剩余空间，直到所有子对象都在范围内 */
    while(grow_reiterate && t->grow_item_cnt) {
        grow_reiterate = false;
        int32_t grow_value_sum = 0;
        int32_t grow_max_size = t->track_main_size - t->track_fix_main_size;
        /* 统计未被 clamp 的子对象的 grow 值总和 */
        for(i = 0; i < t->grow_item_cnt; i++) {
            if(t->grow_dsc[i].clamped == 0) {
                grow_value_sum += t->grow_dsc[i].grow_value;
            }
            else {
                grow_max_size -= t->grow_dsc[i].final_size;
            }
        }

        /* 按比例分配空间，并检查是否需要 clamp */
        for(i = 0; i < t->grow_item_cnt; i++) {
            if(t->grow_dsc[i].clamped == 0) {
                LV_ASSERT(grow_value_sum != 0);
                int32_t size = div_round_closest(grow_max_size * t->grow_dsc[i].grow_value, grow_value_sum);
                int32_t size_clamp = LV_CLAMP(t->grow_dsc[i].min_size, size, t->grow_dsc[i].max_size);

                /* 如果尺寸被 clamp，则标记并重新迭代 */
                if(size_clamp != size) {
                    t->grow_dsc[i].clamped = 1;
                    grow_reiterate = true;
                }
                t->grow_dsc[i].final_size = size_clamp;
                grow_value_sum -= t->grow_dsc[i].grow_value;
                grow_max_size -= t->grow_dsc[i].final_size;
            }
        }
    }

    bool rtl = lv_obj_get_style_base_dir(cont, LV_PART_MAIN) == LV_BASE_DIR_RTL;

    int32_t main_pos = 0;

    /*===== 第二步：计算主轴起始位置和间距 =====*/
    int32_t place_gap = 0;
    place_content(f->main_place, max_main_size, t->track_main_size, t->item_cnt, &main_pos, &place_gap);
    if(f->row && rtl) main_pos = max_main_size - main_pos;

    /*===== 第三步：遍历子对象并定位 =====*/
    lv_obj_t * item = lv_obj_get_child(cont, item_first_id);
    while(item && item_first_id != item_last_id) {
        /* 跳过不可见、被忽略或浮动的子对象 */
        if((lv_obj_is_ignore_layout(item) || lv_obj_is_hidden(item) || lv_obj_is_floating(item))) {
            item = get_next_item(cont, f->rev, &item_first_id);
            continue;
        }

        /* 保存原始的布局标志 */
        uint16_t item_w_layout = item->w_layout;
        uint16_t item_h_layout = item->h_layout;

        int32_t grow_size = lv_obj_get_style_flex_grow(item, LV_PART_MAIN);
        if(grow_size) {
            /* 弹性增长子对象：从 grow 描述符中获取计算好的最终尺寸 */
            int32_t s = 0;
            for(i = 0; i < t->grow_item_cnt; i++) {
                if(t->grow_dsc[i].item == item) {
                    s = t->grow_dsc[i].final_size;
                    break;
                }
            }

            /* 设置布局标志：主轴方向由布局控制，交叉轴方向不控制 */
            if(f->row) {
                item->w_layout = 1;
                item->h_layout = 0;
            }
            else {
                item->h_layout = 1;
                item->w_layout = 0;
            }

            /* 如果计算出的尺寸与当前尺寸不同，更新子对象尺寸 */
            if(s != area_get_main_size(&item->coords)) {
                lv_obj_invalidate(item);

                lv_area_t old_coords;
                lv_area_copy(&old_coords, &item->coords);
                area_set_main_size(&item->coords, s);
                lv_obj_send_event(item, LV_EVENT_SIZE_CHANGED, &old_coords);
                lv_obj_send_event(lv_obj_get_parent(item), LV_EVENT_CHILD_CHANGED, item);
                lv_obj_invalidate(item);
            }
        }
        else {
            /* 固定尺寸子对象：布局不控制其尺寸 */
            item->w_layout = 0;
            item->h_layout = 0;
        }

        /* 如果布局标志发生变化，标记子对象需要重新布局 */
        if(item->w_layout != item_w_layout || item->h_layout != item_h_layout) {
            lv_obj_mark_layout_as_dirty(item);
        }

        /* 计算交叉轴方向的位置（根据对齐方式） */
        int32_t cross_pos = 0;
        switch(f->cross_place) {
            case LV_FLEX_ALIGN_CENTER:
                /* 居中对齐：交叉轴尺寸向上取整避免除 2 时的舍入误差 */
                cross_pos = (((t->track_cross_size + 1) & (~1)) - area_get_cross_size(&item->coords)) / 2;
                cross_pos += (get_margin_cross_start(item, LV_PART_MAIN) - get_margin_cross_end(item, LV_PART_MAIN)) / 2;
                break;
            case LV_FLEX_ALIGN_END:
                cross_pos = t->track_cross_size - area_get_cross_size(&item->coords);
                cross_pos -= get_margin_cross_end(item, LV_PART_MAIN);
                break;
            default:
                cross_pos += get_margin_cross_start(item, LV_PART_MAIN);
                break;
        }

        /* RTL 行方向：主轴位置需要从右向左移动 */
        if(f->row && rtl)
            main_pos -= area_get_main_size(&item->coords);

        /* 处理 translate 样式的百分比值 */
        int32_t tr_x = lv_obj_get_style_translate_x(item, LV_PART_MAIN);
        int32_t tr_y = lv_obj_get_style_translate_y(item, LV_PART_MAIN);
        int32_t w = lv_obj_get_width(item);
        int32_t h = lv_obj_get_height(item);
        if(LV_COORD_IS_PCT(tr_x))
            tr_x = (w * LV_COORD_GET_PCT(tr_x)) / 100;
        if(LV_COORD_IS_PCT(tr_y))
            tr_y = (h * LV_COORD_GET_PCT(tr_y)) / 100;

        /* 计算子对象需要移动的偏移量 */
        int32_t diff_x = abs_x - item->coords.x1 + tr_x;
        int32_t diff_y = abs_y - item->coords.y1 + tr_y;
        diff_x += f->row ? main_pos + get_margin_main_start(item, LV_PART_MAIN) : cross_pos;
        diff_y += f->row ? cross_pos : main_pos + get_margin_main_start(item, LV_PART_MAIN);

        /* 移动子对象及其所有子节点 */
        if(diff_x || diff_y) {
            lv_obj_invalidate(item);
            item->coords.x1 += diff_x;
            item->coords.x2 += diff_x;
            item->coords.y1 += diff_y;
            item->coords.y2 += diff_y;
            lv_obj_invalidate(item);
            lv_obj_move_children_by(item, diff_x, diff_y, false);
        }

        /* 更新主轴位置，为下一个子对象做准备 */
        if(!(f->row && rtl))
            main_pos += area_get_main_size(&item->coords) + item_gap + place_gap +
                        get_margin_main_start(item, LV_PART_MAIN) + get_margin_main_end(item, LV_PART_MAIN);
        else
            main_pos -= item_gap + place_gap;

        item = get_next_item(cont, f->rev, &item_first_id);
    }
}

/**
 * 根据对齐方式计算起始位置和间距
 *
 * 支持以下对齐方式：
 *   - START:          起始对齐，间距为 0
 *   - END:            末尾对齐，间距为 0
 *   - CENTER:         居中对齐，间距为 0
 *   - SPACE_BETWEEN:  首尾紧贴边界，中间均匀分布
 *   - SPACE_AROUND:   每个间隔相同，首尾间隔为中间的一半
 *   - SPACE_EVENLY:   所有间隔相同（包括首尾）
 *
 * @param place        对齐方式
 * @param max_size     可用空间大小
 * @param content_size 内容总大小
 * @param item_cnt     子对象数量
 * @param start_pos    输入/输出参数，起始位置会被累加
 * @param gap          输出参数，计算出的额外间距
 */
static void place_content(lv_flex_align_t place, int32_t max_size, int32_t content_size, int32_t item_cnt,
                          int32_t * start_pos, int32_t * gap)
{
    /* 当只有一个子对象时，SPACE_AROUND 和 SPACE_EVENLY 退化为 CENTER */
    if(item_cnt <= 1) {
        switch(place) {
            case LV_FLEX_ALIGN_SPACE_AROUND:
            case LV_FLEX_ALIGN_SPACE_EVENLY:
                place = LV_FLEX_ALIGN_CENTER;
                break;
            default:
                break;
        }
    }

    switch(place) {
        case LV_FLEX_ALIGN_CENTER:
            *gap = 0;
            *start_pos += (max_size - content_size) / 2;
            break;
        case LV_FLEX_ALIGN_END:
            *gap = 0;
            *start_pos += max_size - content_size;
            break;
        case LV_FLEX_ALIGN_SPACE_BETWEEN:
            if(item_cnt > 1) *gap = (int32_t)(max_size - content_size) / (int32_t)(item_cnt - 1);
            break;
        case LV_FLEX_ALIGN_SPACE_AROUND:
            *gap += (int32_t)(max_size - content_size) / (int32_t)(item_cnt);
            *start_pos += *gap / 2;
            break;
        case LV_FLEX_ALIGN_SPACE_EVENLY:
            *gap = (int32_t)(max_size - content_size) / (int32_t)(item_cnt + 1);
            *start_pos += *gap;
            break;
        default:
            *gap = 0;
    }
}

/**
 * 获取下一个子对象
 *
 * 根据遍历方向（正向或反向）获取容器中的下一个子对象。
 *
 * @param cont    容器对象
 * @param rev     true=反向遍历，false=正向遍历
 * @param item_id 输入/输出参数，当前子对象 ID，调用后会更新为下一个
 * @return 下一个子对象的指针，如果没有更多子对象则返回 NULL
 */
static lv_obj_t * get_next_item(lv_obj_t * cont, bool rev, int32_t * item_id)
{
    if(rev) {
        (*item_id)--;
        if(*item_id >= 0) return cont->spec_attr->children[*item_id];
        else return NULL;
    }
    else {
        (*item_id)++;
        if((*item_id) < (int32_t)cont->spec_attr->child_cnt) return cont->spec_attr->children[*item_id];
        else return NULL;
    }
}

/**
 * 获取对象宽度（包含左右 margin）
 *
 * 在 Flex 布局中，子对象的 margin 也被计入主轴/交叉轴的占用空间。
 */
static int32_t lv_obj_get_width_with_margin(const lv_obj_t * obj)
{
    return lv_obj_get_style_margin_left(obj, LV_PART_MAIN)
           + lv_obj_get_width(obj)
           + lv_obj_get_style_margin_right(obj, LV_PART_MAIN);
}

/**
 * 获取对象高度（包含上下 margin）
 */
static int32_t lv_obj_get_height_with_margin(const lv_obj_t * obj)
{
    return lv_obj_get_style_margin_top(obj, LV_PART_MAIN)
           + lv_obj_get_height(obj)
           + lv_obj_get_style_margin_bottom(obj, LV_PART_MAIN);
}

#endif /*LV_USE_FLEX*/