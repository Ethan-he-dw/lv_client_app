/**
 * @file lv_slider.c
 * 控件简介:滑块控件,基于进度条(bar)实现的可拖动数值选择控件,支持范围模式
 *
 */

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "lv_slider_private.h"

#if LV_USE_SLIDER

#include "../../misc/lv_area_private.h"
#include "../../core/lv_obj_private.h"
#include "../../core/lv_obj_event_private.h"
#include "../../core/lv_obj_class_private.h"
#include "../../lvgl_public.h"
#include "../../indev/lv_indev_private.h"
#include "../../core/lv_observer_private.h"

/*Check dependencies*/
/*检查依赖条件*/
#if LV_USE_BAR == 0
    #error "lv_slider: lv_bar is required. Enable it in lv_conf.h (LV_USE_BAR 1)"
#endif

/*********************
 *      DEFINES
 *      宏定义
 *********************/
#define MY_CLASS (&lv_slider_class)

#define LV_SLIDER_KNOB_COORD(is_reversed, area) (is_reversed ? area.x1 : area.x2)
#define LV_SLIDER_KNOB_COORD_VERTICAL(is_reversed, area) (is_reversed ? area.y2 : area.y1)

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/**********************
 *  STATIC PROTOTYPES
 *  静态函数声明
 **********************/
static void lv_slider_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_slider_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void position_knob(lv_obj_t * obj, lv_area_t * knob_area, const int32_t knob_size, const bool hor);
static void draw_knob(lv_event_t * e);
static bool is_slider_horizontal(lv_obj_t * obj);
static void drag_start(lv_obj_t * obj);
static void update_knob_pos(lv_obj_t * obj, bool check_drag);

#if LV_USE_OBSERVER
    static void slider_value_changed_event_cb(lv_event_t * e);
    static void slider_value_observer_cb(lv_observer_t * observer, lv_subject_t * subject);
#endif /*LV_USE_OBSERVER，观察者支持结束*/

/**********************
 *  STATIC VARIABLES
 *  静态变量
 **********************/

#if LV_USE_OBJ_PROPERTY
static const lv_property_ops_t lv_slider_properties[] = {
    {
        .id = LV_PROPERTY_SLIDER_VALUE,
        .setter = lv_slider_set_value,
        .getter = lv_slider_get_value,
    },
    {
        .id = LV_PROPERTY_SLIDER_LEFT_VALUE,
        .setter = lv_slider_set_start_value,
        .getter = lv_slider_get_left_value,
    },
    {
        .id = LV_PROPERTY_SLIDER_RANGE,
        .setter = lv_slider_set_range,
        .getter = NULL,
    },
    {
        .id = LV_PROPERTY_SLIDER_MIN_VALUE,
        .setter = lv_slider_set_min_value,
        .getter = lv_slider_get_min_value,
    },
    {
        .id = LV_PROPERTY_SLIDER_MAX_VALUE,
        .setter = lv_slider_set_max_value,
        .getter = lv_slider_get_max_value,
    },
    {
        .id = LV_PROPERTY_SLIDER_MODE,
        .setter = lv_slider_set_mode,
        .getter = lv_slider_get_mode,
    },
    {
        .id = LV_PROPERTY_SLIDER_IS_DRAGGED,
        .setter = NULL,
        .getter = lv_slider_is_dragged,
    },
    {
        .id = LV_PROPERTY_SLIDER_IS_SYMMETRICAL,
        .setter = NULL,
        .getter = lv_slider_is_symmetrical,
    },
};
#endif

const lv_obj_class_t lv_slider_class = {
    .constructor_cb = lv_slider_constructor,
    .event_cb = lv_slider_event,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_slider_t),
    .base_class = &lv_bar_class,
    .name = "lv_slider",
    LV_PROPERTY_CLASS_FIELDS(slider, SLIDER)
};

/**********************
 *      MACROS
 *      宏
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 *  全局函数
 **********************/

/* 创建滑块对象 */
lv_obj_t * lv_slider_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/* 检查滑块是否正在被拖动 */
bool lv_slider_is_dragged(const lv_obj_t * obj)
{
    LV_CHECK_OBJ(obj, MY_CLASS, return false);
    lv_slider_t * slider = (lv_slider_t *)obj;

    return slider->dragging;
}

/* 设置滑块当前值 */
void lv_slider_set_value(lv_obj_t * obj, int32_t value, lv_anim_enable_t anim)
{
    lv_bar_set_value(obj, value, anim);
}

/* 设置范围模式下起始值(左旋钮) */
void lv_slider_set_start_value(lv_obj_t * obj, int32_t value, lv_anim_enable_t anim)
{
    lv_bar_set_start_value(obj, value, anim);
}

/* 设置滑块值范围 */
void lv_slider_set_range(lv_obj_t * obj, int32_t min, int32_t max)
{
    lv_bar_set_range(obj, min, max);
}

/* 设置最小值 */
void lv_slider_set_min_value(lv_obj_t * obj, int32_t min)
{
    lv_bar_set_min_value(obj, min);
}

/* 设置最大值 */
void lv_slider_set_max_value(lv_obj_t * obj, int32_t min)
{
    lv_bar_set_max_value(obj, min);
}

/* 设置滑块模式(普通/对称/范围) */
void lv_slider_set_mode(lv_obj_t * obj, lv_slider_mode_t mode)
{
    lv_bar_set_mode(obj, (lv_bar_mode_t)mode);
}

/* 设置滑块方向(水平/垂直) */
void lv_slider_set_orientation(lv_obj_t * obj, lv_slider_orientation_t orientation)
{
    lv_bar_set_orientation(obj, (lv_bar_orientation_t)orientation);
}

/* 获取滑块当前值 */
int32_t lv_slider_get_value(const lv_obj_t * obj)
{
    return lv_bar_get_value(obj);
}

/* 获取起始值(左旋钮值) */
int32_t lv_slider_get_left_value(const lv_obj_t * obj)
{
    return lv_bar_get_start_value(obj);
}

/* 获取最小值 */
int32_t lv_slider_get_min_value(const lv_obj_t * obj)
{
    return lv_bar_get_min_value(obj);
}

/* 获取最大值 */
int32_t lv_slider_get_max_value(const lv_obj_t * obj)
{
    return lv_bar_get_max_value(obj);
}

/* 获取滑块模式 */
lv_slider_mode_t lv_slider_get_mode(lv_obj_t * slider)
{
    lv_bar_mode_t mode = lv_bar_get_mode(slider);
    if(mode == LV_BAR_MODE_SYMMETRICAL) return LV_SLIDER_MODE_SYMMETRICAL;
    else if(mode == LV_BAR_MODE_RANGE) return LV_SLIDER_MODE_RANGE;
    else return LV_SLIDER_MODE_NORMAL;
}

/* 获取滑块方向 */
lv_slider_orientation_t lv_slider_get_orientation(lv_obj_t * slider)
{
    lv_bar_orientation_t ori = lv_bar_get_orientation(slider);
    if(ori == LV_BAR_ORIENTATION_HORIZONTAL) return LV_SLIDER_ORIENTATION_HORIZONTAL;
    else if(ori == LV_BAR_ORIENTATION_VERTICAL) return LV_SLIDER_ORIENTATION_VERTICAL;
    else return LV_SLIDER_ORIENTATION_AUTO;
}

/* 检查滑块是否为对称模式 */
bool lv_slider_is_symmetrical(lv_obj_t * obj)
{
    return lv_bar_is_symmetrical(obj);
}

#if LV_USE_OBSERVER
/* 将滑块的值与主题(subject)绑定,实现双向同步 */
lv_observer_t * lv_slider_bind_value(lv_obj_t * obj, lv_subject_t * subject)
{
    LV_ASSERT_NULL(subject);
    LV_ASSERT_NULL(obj);

    if(subject->type != LV_SUBJECT_TYPE_INT && subject->type != LV_SUBJECT_TYPE_FLOAT) {
        LV_LOG_WARN("Incompatible subject type: %d", subject->type);
        return NULL;
    }

    lv_obj_add_event_cb(obj, slider_value_changed_event_cb, LV_EVENT_VALUE_CHANGED, subject);
    lv_observer_t * observer = lv_subject_add_observer_obj(subject, slider_value_observer_cb, obj, NULL);
    return observer;
}
#endif /*LV_USE_OBSERVER，观察者支持结束*/


/**********************
 *   STATIC FUNCTIONS
 *  静态函数
 **********************/

/* 滑块对象构造函数 */
static void lv_slider_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_slider_t * slider = (lv_slider_t *)obj;

    /*Initialize the allocated 'slider'*/
    /*初始化已分配内存的 'slider' 结构体*/
    slider->value_to_set = NULL;
    slider->dragging = 0U;
    slider->left_knob_focus = 0U;

    lv_obj_set_scroll_chain_hor(obj, false);
    lv_obj_set_scrollable(obj, false);
    lv_obj_set_scroll_on_focus(obj, true);
    lv_obj_set_ext_click_area(obj, LV_DPX(8));
}

/* 滑块事件处理 */
static void lv_slider_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    lv_result_t res;

    /*Call the ancestor's event handler*/
    /*调用祖先类的事件处理函数*/
    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);
    lv_slider_t * slider = (lv_slider_t *)obj;
    lv_slider_mode_t type = lv_slider_get_mode(obj);

    /*Advanced hit testing: react only on dragging the knob(s)*/
    /*高级命中测试:仅响应拖动滑块旋钮*/
    if(code == LV_EVENT_HIT_TEST) {
        lv_hit_test_info_t * info = lv_event_get_param(e);
        int32_t ext_click_area = obj->spec_attr ? obj->spec_attr->ext_click_pad : 0;

        /*Ordinary slider: was the knob area hit?*/
        /*普通滑块:是否命中了旋钮区域?*/
        lv_area_t a;
        lv_area_copy(&a, &slider->right_knob_area);
        lv_area_increase(&a, ext_click_area, ext_click_area);
        info->res = lv_area_is_point_on(&a, info->point, 0);

        /*There's still a chance that there is a hit if there is another knob*/
        /*若存在另一个旋钮,仍有可能被命中*/
        if((info->res == false) && (type == LV_SLIDER_MODE_RANGE)) {
            lv_area_copy(&a, &slider->left_knob_area);
            lv_area_increase(&a, ext_click_area, ext_click_area);
            info->res = lv_area_is_point_on(&a, info->point, 0);
        }
    }
    else if(code == LV_EVENT_PRESSED) {
        /*Save the pressed coordinates*/
        /*保存按下时的坐标*/
        lv_indev_get_point(lv_indev_active(), &slider->pressed_point);
        lv_obj_transform_point(obj, &slider->pressed_point, LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE_RECURSIVE);
    }
    else if(code == LV_EVENT_PRESSING) {
        update_knob_pos(obj, true);
    }
    else if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        update_knob_pos(obj, false);
        slider->dragging = false;
        slider->value_to_set = NULL;

        lv_obj_invalidate(obj);

        /*Leave edit mode if released. (No need to wait for LONG_PRESS)*/
        /*松开时退出编辑模式(无需等待长按)*/
        lv_group_t * g   = lv_obj_get_group(obj);
        bool editing     = lv_group_get_editing(g);
        lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_active());
        if(indev_type == LV_INDEV_TYPE_ENCODER) {
            if(editing) {
                if(lv_slider_get_mode(obj) == LV_SLIDER_MODE_RANGE) {
                    if(slider->left_knob_focus == 0) slider->left_knob_focus = 1;
                    else {
                        slider->left_knob_focus = 0;
                        lv_group_set_editing(g, false);
                    }
                }
                else {
                    lv_group_set_editing(g, false);
                }
            }
        }
        else if(indev_type == LV_INDEV_TYPE_POINTER) {
            if(is_slider_horizontal(obj)) lv_obj_set_scroll_chain_ver(obj, true);
            else  lv_obj_set_scroll_chain_hor(obj, true);
        }
    }
    else if(code == LV_EVENT_FOCUSED) {
        lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_active());
        if(indev_type == LV_INDEV_TYPE_ENCODER || indev_type == LV_INDEV_TYPE_KEYPAD) {
            slider->left_knob_focus = 0;
        }
    }
    else if(code == LV_EVENT_SIZE_CHANGED) {
        if(is_slider_horizontal(obj)) {
            lv_obj_set_scroll_chain_ver(obj, true);
            lv_obj_set_scroll_chain_hor(obj, false);
        }
        else {
            lv_obj_set_scroll_chain_hor(obj, true);
            lv_obj_set_scroll_chain_ver(obj, false);
        }
        lv_obj_refresh_ext_draw_size(obj);
    }
    else if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        int32_t knob_left = lv_obj_get_style_pad_left(obj, LV_PART_KNOB);
        int32_t knob_right = lv_obj_get_style_pad_right(obj, LV_PART_KNOB);
        int32_t knob_top = lv_obj_get_style_pad_top(obj, LV_PART_KNOB);
        int32_t knob_bottom = lv_obj_get_style_pad_bottom(obj, LV_PART_KNOB);

        /*The smaller size is the knob diameter*/
        /*较小尺寸即为旋钮直径*/
        int32_t trans_w = lv_obj_get_style_transform_width(obj, LV_PART_KNOB);
        int32_t trans_h = lv_obj_get_style_transform_height(obj, LV_PART_KNOB);
        int32_t knob_size = LV_MIN(lv_obj_get_width(obj) + 2 * trans_w, lv_obj_get_height(obj) + 2 * trans_h) >> 1;
        knob_size += LV_MAX(LV_MAX(knob_left, knob_right), LV_MAX(knob_bottom, knob_top));
        knob_size += 2;         /*For rounding error，用于消除舍入误差*/
        knob_size += lv_obj_calculate_ext_draw_size(obj, LV_PART_KNOB);

        /*Indic. size is handled by bar*/
        /*指示器尺寸由进度条(bar)处理*/
        int32_t * s = lv_event_get_param(e);
        *s  = LV_MAX(*s, knob_size);

    }
    else if(code == LV_EVENT_KEY) {
        uint32_t c = lv_event_get_key(e);

        if(c == LV_KEY_RIGHT || c == LV_KEY_UP) {
            if(!slider->left_knob_focus) lv_slider_set_value(obj, lv_slider_get_value(obj) + 1, LV_ANIM_ON);
            else lv_slider_set_start_value(obj, lv_slider_get_left_value(obj) + 1, LV_ANIM_ON);
        }
        else if(c == LV_KEY_LEFT || c == LV_KEY_DOWN) {
            if(!slider->left_knob_focus) lv_slider_set_value(obj, lv_slider_get_value(obj) - 1, LV_ANIM_ON);
            else lv_slider_set_start_value(obj, lv_slider_get_left_value(obj) - 1, LV_ANIM_ON);
        }
        else {
            return;
        }

        res = lv_obj_send_event(obj, LV_EVENT_VALUE_CHANGED, NULL);
        if(res != LV_RESULT_OK) return;
    }
    else if(code == LV_EVENT_ROTARY) {
        int32_t r = lv_event_get_rotary_diff(e);

        if(!slider->left_knob_focus) lv_slider_set_value(obj, lv_slider_get_value(obj) + r, LV_ANIM_ON);
        else lv_slider_set_start_value(obj, lv_slider_get_left_value(obj) + 1, LV_ANIM_ON);

        res = lv_obj_send_event(obj, LV_EVENT_VALUE_CHANGED, NULL);
        if(res != LV_RESULT_OK) return;
    }
    else if(code == LV_EVENT_DRAW_MAIN) {
        draw_knob(e);
    }
}

/* 绘制滑块旋钮 */
static void draw_knob(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_current_target(e);
    lv_slider_t * slider = (lv_slider_t *)obj;
    lv_layer_t * layer = lv_event_get_layer(e);

    const bool is_rtl = LV_BASE_DIR_RTL == lv_obj_get_style_base_dir(obj, LV_PART_MAIN);
    const bool is_horizontal = is_slider_horizontal(obj);
    const bool is_reversed = slider->bar.val_reversed ^ (is_rtl && is_horizontal);

    lv_area_t knob_area;
    int32_t knob_size;
    bool is_symmetrical = lv_slider_is_symmetrical(obj);

    if(is_horizontal) {
        knob_size = lv_obj_get_height(obj);
        if(is_symmetrical &&
           slider->bar.cur_value < 0) knob_area.x1 = LV_SLIDER_KNOB_COORD(!is_reversed, slider->bar.indic_area);
        else knob_area.x1 = LV_SLIDER_KNOB_COORD(is_reversed, slider->bar.indic_area);
    }
    else {
        knob_size = lv_obj_get_width(obj);
        if(is_symmetrical &&
           slider->bar.cur_value < 0) knob_area.y1 =  LV_SLIDER_KNOB_COORD_VERTICAL(!is_reversed, slider->bar.indic_area);
        else knob_area.y1 = LV_SLIDER_KNOB_COORD_VERTICAL(is_reversed, slider->bar.indic_area);
    }
    lv_draw_rect_dsc_t knob_rect_dsc;
    lv_draw_rect_dsc_init(&knob_rect_dsc);
    knob_rect_dsc.base.layer = layer;
    lv_obj_init_draw_rect_dsc(obj, LV_PART_KNOB, &knob_rect_dsc);
    /* Update knob area with knob style */
    /* 使用旋钮样式更新旋钮区域 */
    position_knob(obj, &knob_area, knob_size, is_horizontal);
    /* Update right knob area with calculated knob area */
    /* 将计算得到的旋钮区域保存到右旋钮区域 */
    lv_area_copy(&slider->right_knob_area, &knob_area);

    if(lv_slider_get_mode(obj) != LV_SLIDER_MODE_RANGE) {
        lv_draw_rect(layer, &knob_rect_dsc, &slider->right_knob_area);
    }
    else {
        /*Save the draw part_draw_dsc. because it can be modified in the event*/
        /*保存绘制描述符,因为事件中可能会修改它*/
        lv_draw_rect_dsc_t knob_rect_dsc_tmp;
        lv_memcpy(&knob_rect_dsc_tmp, &knob_rect_dsc, sizeof(lv_draw_rect_dsc_t));
        /* Draw the right knob */
        /* 绘制右旋钮 */
        lv_draw_rect(layer, &knob_rect_dsc, &slider->right_knob_area);

        /*Calculate the second knob area*/
        /*计算第二个旋钮的区域*/
        if(is_horizontal) {
            /*use !is_reversed to get the other knob*/
            /*使用 !is_reversed 获取另一个旋钮*/
            knob_area.x1 = LV_SLIDER_KNOB_COORD(!is_reversed, slider->bar.indic_area);
        }
        else {
            knob_area.y1 = LV_SLIDER_KNOB_COORD_VERTICAL(!is_reversed, slider->bar.indic_area);
        }
        position_knob(obj, &knob_area, knob_size, is_horizontal);
        lv_area_copy(&slider->left_knob_area, &knob_area);

        lv_memcpy(&knob_rect_dsc, &knob_rect_dsc_tmp, sizeof(lv_draw_rect_dsc_t));

        lv_draw_rect(layer, &knob_rect_dsc, &slider->left_knob_area);
    }
}

/* 计算并定位旋钮区域 */
static void position_knob(lv_obj_t * obj, lv_area_t * knob_area, const int32_t knob_size, const bool hor)
{
    if(hor) {
        knob_area->x1 -= (knob_size >> 1);
        knob_area->x2 = knob_area->x1 + knob_size - 1;
        knob_area->y1 = obj->coords.y1;
        knob_area->y2 = obj->coords.y2;
    }
    else {
        knob_area->y1 -= (knob_size >> 1);
        knob_area->y2 = knob_area->y1 + knob_size - 1;
        knob_area->x1 = obj->coords.x1;
        knob_area->x2 = obj->coords.x2;
    }

    int32_t knob_left = lv_obj_get_style_pad_left(obj, LV_PART_KNOB);
    int32_t knob_right = lv_obj_get_style_pad_right(obj, LV_PART_KNOB);
    int32_t knob_top = lv_obj_get_style_pad_top(obj, LV_PART_KNOB);
    int32_t knob_bottom = lv_obj_get_style_pad_bottom(obj, LV_PART_KNOB);

    int32_t transf_w = lv_obj_get_style_transform_width(obj, LV_PART_KNOB);
    int32_t transf_h = lv_obj_get_style_transform_height(obj, LV_PART_KNOB);

    /*Apply the paddings on the knob area*/
    /*在旋钮区域上应用内边距*/
    knob_area->x1 -= knob_left + transf_w;
    knob_area->x2 += knob_right + transf_w;
    knob_area->y1 -= knob_top + transf_h;
    knob_area->y2 += knob_bottom + transf_h;
}

/* 判断滑块是否为水平方向 */
static bool is_slider_horizontal(lv_obj_t * obj)
{
    lv_slider_t * slider = (lv_slider_t *)obj;
    if(slider->bar.orientation == LV_BAR_ORIENTATION_AUTO) return lv_obj_get_width(obj) >= lv_obj_get_height(obj);
    else if(slider->bar.orientation == LV_BAR_ORIENTATION_HORIZONTAL) return true;
    else return false;
}

/* 开始拖动:确定拖动的是哪个旋钮 */
static void drag_start(lv_obj_t * obj)
{
    lv_slider_t * slider = (lv_slider_t *)obj;
    lv_slider_mode_t mode = lv_slider_get_mode(obj);
    lv_point_t p;
    slider->dragging = true;
    if(mode == LV_SLIDER_MODE_NORMAL || mode == LV_SLIDER_MODE_SYMMETRICAL) {
        slider->value_to_set = &slider->bar.cur_value;
    }
    else if(mode == LV_SLIDER_MODE_RANGE) {
        lv_indev_get_point(lv_indev_active(), &p);
        lv_obj_transform_point(obj, &p, LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE_RECURSIVE);
        const bool is_rtl = LV_BASE_DIR_RTL == lv_obj_get_style_base_dir(obj, LV_PART_MAIN);
        const bool is_horizontal = is_slider_horizontal(obj);
        const bool is_reversed = slider->bar.val_reversed ^ (is_rtl && is_horizontal);
        int32_t dist_left, dist_right;
        if(is_horizontal) {
            if((!is_reversed && p.x > slider->right_knob_area.x2) || (is_reversed && p.x < slider->right_knob_area.x1)) {
                slider->value_to_set = &slider->bar.cur_value;
            }
            else if((!is_reversed && p.x < slider->left_knob_area.x1) || (is_reversed && p.x > slider->left_knob_area.x2)) {
                slider->value_to_set = &slider->bar.start_value;
            }
            else {
                /*Calculate the distance from each knob*/
                /*计算与每个旋钮的距离*/
                dist_left = LV_ABS((slider->left_knob_area.x1 + (slider->left_knob_area.x2 - slider->left_knob_area.x1) / 2) - p.x);
                dist_right = LV_ABS((slider->right_knob_area.x1 + (slider->right_knob_area.x2 - slider->right_knob_area.x1) / 2) - p.x);
                /*Use whichever one is closer*/
                /*使用距离更近的旋钮*/
                if(dist_right < dist_left) {
                    slider->value_to_set = &slider->bar.cur_value;
                    slider->left_knob_focus = 0;
                }
                else {
                    slider->value_to_set = &slider->bar.start_value;
                    slider->left_knob_focus = 1;
                }
            }
        }
        else {
            if((!is_reversed && p.y < slider->right_knob_area.y1) || (is_reversed && p.y > slider->right_knob_area.y2)) {
                slider->value_to_set = &slider->bar.cur_value;
            }
            else if((!is_reversed && p.y > slider->left_knob_area.y2) || (is_reversed && p.y < slider->left_knob_area.y1)) {
                slider->value_to_set = &slider->bar.start_value;
            }
            else {
                /*Calculate the distance from each knob*/
                /*计算与每个旋钮的距离*/
                dist_left = LV_ABS((slider->left_knob_area.y1 + (slider->left_knob_area.y2 - slider->left_knob_area.y1) / 2) - p.y);
                dist_right = LV_ABS((slider->right_knob_area.y1 + (slider->right_knob_area.y2 - slider->right_knob_area.y1) / 2) - p.y);

                /*Use whichever one is closer*/
                /*使用距离更近的旋钮*/
                if(dist_right < dist_left) {
                    slider->value_to_set = &slider->bar.cur_value;
                    slider->left_knob_focus = 0;
                }
                else {
                    slider->value_to_set = &slider->bar.start_value;
                    slider->left_knob_focus = 1;
                }
            }
        }
    }
}

/* 根据指针位置更新旋钮的值 */
static void update_knob_pos(lv_obj_t * obj, bool check_drag)
{
    lv_slider_t * slider = (lv_slider_t *)obj;
    lv_indev_t * indev = lv_indev_active();
    if(lv_indev_get_type(indev) != LV_INDEV_TYPE_POINTER)
        return;
    if(lv_indev_get_scroll_obj(indev) != NULL)
        return;

    lv_point_t p;
    lv_indev_get_point(indev, &p);
    lv_obj_transform_point(obj, &p, LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE_RECURSIVE);

    bool is_hor = is_slider_horizontal(obj);

    if(check_drag && !slider->dragging) {
        int32_t ofs = is_hor ? (p.x - slider->pressed_point.x) : (p.y - slider->pressed_point.y);

        /*Stop processing when offset is below scroll_limit*/
        /*当偏移量小于滚动阈值时停止处理*/
        if(LV_ABS(ofs) < indev->scroll_limit) {
            return;
        }
    }

    if(!slider->value_to_set) {
        /*Ready to start drag*/
        /*准备开始拖动*/
        drag_start(obj);
    }

    int32_t new_value = 0;
    const int32_t range = slider->bar.max_value - slider->bar.min_value;
    const bool is_rtl = LV_BASE_DIR_RTL == lv_obj_get_style_base_dir(obj, LV_PART_MAIN);
    const bool is_horizontal = is_slider_horizontal(obj);
    const bool is_reversed = slider->bar.val_reversed ^ (is_rtl && is_horizontal);

    if(is_hor) {
        const int32_t bg_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
        const int32_t bg_right = lv_obj_get_style_pad_right(obj, LV_PART_MAIN);
        const int32_t w = lv_obj_get_width(obj);
        const int32_t indic_w = w - bg_left - bg_right;

        if(is_reversed) {
            /*Make the point relative to the indicator*/
            /*使坐标点相对于指示器*/
            new_value = (obj->coords.x2 - bg_right) - p.x;
        }
        else {
            /*Make the point relative to the indicator*/
            /*使坐标点相对于指示器*/
            new_value = p.x - (obj->coords.x1 + bg_left);
        }
        if(indic_w) {
            new_value = (int32_t)(((int64_t)new_value * range + indic_w / 2) / indic_w);
            new_value += slider->bar.min_value;
        }
    }
    else {
        const int32_t bg_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
        const int32_t bg_bottom = lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN);
        const int32_t h = lv_obj_get_height(obj);
        const int32_t indic_h = h - bg_bottom - bg_top;

        if(is_reversed) {
            /*Make the point relative to the indicator*/
            /*使坐标点相对于指示器*/
            new_value = p.y - (obj->coords.y1 + bg_top);
        }
        else {
            /*Make the point relative to the indicator*/
            /*使坐标点相对于指示器*/
            new_value = p.y - (obj->coords.y2 + bg_bottom);
            new_value = -new_value;
        }
        new_value = (int32_t)(((int64_t)new_value * range + indic_h / 2) / indic_h);
        new_value += slider->bar.min_value;
    }

    int32_t real_max_value = slider->bar.max_value;
    int32_t real_min_value = slider->bar.min_value;
    /*Figure out the min. and max. for this mode*/
    /*计算该模式下的最小与最大值*/
    if(slider->value_to_set == &slider->bar.start_value) {
        real_max_value = slider->bar.cur_value;
    }
    else {
        real_min_value = slider->bar.start_value;
    }

    new_value = LV_CLAMP(real_min_value, new_value, real_max_value);
    if(*slider->value_to_set != new_value) {
        *slider->value_to_set = new_value;
        if(is_hor)
            lv_obj_set_scroll_chain_ver(obj, false);
        else
            lv_obj_set_scroll_chain_hor(obj, false);

        lv_obj_invalidate(obj);
        lv_result_t res = lv_obj_send_event(obj, LV_EVENT_VALUE_CHANGED, NULL);
        if(res != LV_RESULT_OK)
            return;
    }
}


#if LV_USE_OBSERVER

/* 值变化事件回调:同步到主题 */
static void slider_value_changed_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_current_target(e);
    lv_subject_t * subject = lv_event_get_user_data(e);

    if(subject->type == LV_SUBJECT_TYPE_INT) {
        lv_subject_set_int(subject, lv_slider_get_value(slider));
    }
#if LV_USE_FLOAT
    else {
        lv_subject_set_float(subject, (float)lv_slider_get_value(slider));
    }
#endif
}

/* 主题变化观察回调:更新滑块值 */
static void slider_value_observer_cb(lv_observer_t * observer, lv_subject_t * subject)
{
    lv_obj_t * obj = lv_observer_get_target_obj(observer);
    /*If the slider is not rendered yet show the new state immediately*/
    /*如果滑块尚未渲染,则立即显示新状态*/
    lv_anim_enable_t anim_on = obj->rendered ? LV_ANIM_ON : LV_ANIM_OFF;
    if(subject->type == LV_SUBJECT_TYPE_INT) {
        lv_slider_set_value(observer->target, subject->value.num, anim_on);
    }
#if LV_USE_FLOAT
    else {
        lv_slider_set_value(observer->target, (int32_t)subject->value.float_v, anim_on);
    }
#endif
}

#endif /*LV_USE_OBSERVER，观察者支持结束*/


#endif
