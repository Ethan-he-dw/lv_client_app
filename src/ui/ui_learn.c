#include "ui_learn.h"
static lv_obj_t * g_slider_label; // 用于显示数值的标签
static lv_obj_t *g_slider_label_move;
/**
 * @brief 将屏幕绝对坐标转为相对于某个父对象的坐标
 */
static void screen_to_parent(lv_obj_t *parent, lv_point_t *screen_pos)
{
    lv_area_t parent_content;
    lv_obj_get_content_coords(parent, &parent_content);

    screen_pos->x -= parent_content.x1;
    screen_pos->y -= parent_content.y1;
}
static void get_knob_coords(lv_obj_t *slider, lv_point_t *knob_center)
{
    lv_coord_t knob_size = lv_obj_get_style_width(slider, LV_PART_KNOB);
    // 注意： knob 通常是正方形，获取宽度即可

    lv_area_t slider_coords;
    lv_obj_get_content_coords(slider, &slider_coords);
    lv_coord_t content_w = lv_area_get_width(&slider_coords);

    int32_t val = lv_slider_get_value(slider);
    int32_t min = lv_slider_get_min_value(slider);
    int32_t max = lv_slider_get_max_value(slider);
    int32_t range = max - min;
    if (range == 0) range = 1;

    // 计算 knob 中心 X（考虑 knob 自身大小）
    lv_coord_t knob_x = (content_w - knob_size) * (val - min) / range + knob_size / 2;
    lv_coord_t knob_y = lv_area_get_height(&slider_coords) / 2;

    knob_center->x = slider_coords.x1 + knob_x;
    knob_center->y = slider_coords.y1 + knob_y;

}
static lv_coord_t get_knob_offset_x(lv_obj_t *slider)
{
    lv_coord_t knob_size = lv_obj_get_style_width(slider, LV_PART_KNOB);
    // 注意： knob 通常是正方形，获取宽度即可

    lv_area_t slider_coords;
    lv_obj_get_content_coords(slider, &slider_coords);
    lv_coord_t content_w = lv_area_get_width(&slider_coords);

    int32_t val = lv_slider_get_value(slider);
    int32_t min = lv_slider_get_min_value(slider);
    int32_t max = lv_slider_get_max_value(slider);
    int32_t range = max - min;
    if (range == 0) range = 1;

    // 计算 knob 中心 X（考虑 knob 自身大小）
    lv_coord_t knob_offset_x = (content_w - knob_size) * (val - min) / range + knob_size / 2;
    return knob_offset_x;


}
static void obj_event_cb_scr_set_Shadow(lv_event_t * e){
    LV_LOG_WARN("[obj_event_cb_scr_set_Shadow]");
    lv_obj_t *sub = lv_event_get_target_obj(e);
    if (sub == NULL){
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scrollED]\tget_target_obj false");
    }
    lv_obj_t * parent = lv_obj_get_parent(sub);
    
    
    // 子屏幕默认没有边框，需要手动设置
lv_obj_set_style_border_width(sub, 12, 0);       // 要显式设宽度
lv_obj_set_style_border_color(sub, lv_color_hex(0x333333), 0);
lv_obj_set_style_border_opa(sub, LV_OPA_COVER, 0);
lv_obj_set_style_radius(sub, 10, 0);            // 圆角也要手动设

// 霓虹发光效果

lv_obj_set_style_shadow_width(sub, 30, 0);
lv_obj_set_style_shadow_color(sub, lv_color_hex(0x00FF00), 0);
lv_obj_set_style_shadow_opa(sub, LV_OPA_60, 0);
lv_obj_set_style_shadow_spread(sub, 10, 0);

lv_refr_now(NULL);  // 参数传 NULL 刷新所有屏幕
LV_LOG_WARN("[obj_event_cb_scr_set_Shadow]\t   set!!!!!!!!!! \n");

}
static void obj_event_cb_scr_reset_Shadow(lv_event_t * e){
    lv_obj_t *sub = lv_event_get_target_obj(e);
    if (sub == NULL){
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scrollED]\tget_target_obj false");
    }
    lv_obj_t * parent = lv_obj_get_parent(sub);
    lv_obj_set_style_shadow_width(sub, 0, 0);
    lv_refr_now(NULL);  // 参数传 NULL 刷新所有屏幕
    LV_LOG_WARN("[obj_event_cb_scr_reset_Shadow]\t   reset @@@@@@@@@\n");
}
static void obj_event_cb_btn_set_red(lv_event_t * e){
    lv_obj_t *sub = lv_event_get_target_obj(e);
    if (sub == NULL){
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scrollED]\tget_target_obj false");
    }
    lv_obj_set_style_bg_color(sub,lv_palette_main(LV_PALETTE_RED),LV_PART_MAIN);
}
static void obj_event_cb_btn_set_blue(lv_event_t * e){
    lv_obj_t *sub = lv_event_get_target_obj(e);
    if (sub == NULL){
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scrollED]\tget_target_obj false");
    }
    lv_obj_set_style_bg_color(sub,lv_palette_main(LV_PALETTE_BLUE),LV_PART_MAIN);
}
static void obj_event_cb_scr_sub0_scroll(lv_event_t * e)
{
    LV_LOG_INFO("[obj_event_cb_scr_sub0_scroll]");
    lv_obj_t *sub = lv_event_get_target_obj(e);
    if (sub == NULL){
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scroll]\tget_target_obj false");
    }
    {
        lv_obj_set_style_border_width(sub, 5, 0);       // 要显式设宽度
        lv_obj_set_style_border_color(sub, lv_color_hex(0x333333), 0);
        lv_obj_set_style_border_opa(sub, LV_OPA_COVER, 0);
    }
    lv_obj_t * parent = lv_obj_get_parent(sub);
    if (parent == NULL){
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scroll]\tget btn parent false");
    }
    lv_obj_send_event(parent,LV_EVENT_HOVER_OVER,NULL);
    if (1){
            // 子屏幕默认没有边框，需要手动设置
        lv_obj_set_style_border_width(parent, 12, 0);       // 要显式设宽度
        lv_obj_set_style_border_color(parent, lv_color_hex(0x333333), 0);
        lv_obj_set_style_border_opa(parent, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(parent, 10, 0);            // 圆角也要手动设

        // 霓虹发光效果

        lv_obj_set_style_shadow_width(parent, 30, 0);
        lv_obj_set_style_shadow_color(parent, lv_color_hex(0x00FF00), 0);
        lv_obj_set_style_shadow_opa(parent, LV_OPA_60, 0);
        lv_obj_set_style_shadow_spread(parent, 10, 0);

        lv_refr_now(NULL);  // 参数传 NULL 刷新所有屏幕
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scroll]\t   set!!!!!!!!!! \n");
    }
    int32_t child_cnt = lv_obj_get_child_cnt(parent);
    for(int32_t index = 0;index < child_cnt;index++)
    {
        lv_obj_t *child_obj = lv_obj_get_child(parent,index);
        if (child_obj == NULL){
            LV_LOG_WARN("[obj_event_cb_scr_sub0_scroll]\tget btn bro false");
        }
        //lv_obj_set_style_bg_color(child_obj,lv_palette_main(LV_PALETTE_RED),LV_PART_MAIN);
       // lv_event_send(child_obj, LV_EVENT_HOVER_OVER, NULL);
       lv_obj_send_event(child_obj, LV_EVENT_FOCUSED, NULL);
    }
    
}

static void obj_event_cb_scr_sub0_scrollED(lv_event_t * e)
{

    lv_obj_t *sub = lv_event_get_target_obj(e);
    if (sub == NULL){
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scrollED]\tget_target_obj false");
    }
    lv_obj_t * parent = lv_obj_get_parent(sub);
    if (parent == NULL){
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scrollED]\tget btn parent false");
    }
    {
        lv_obj_set_style_border_width(sub, 0, 0);    

    }
    lv_obj_send_event(parent,LV_EVENT_HOVER_LEAVE,NULL);
    {
        //取消边框
        lv_obj_set_style_border_width(parent, 0, 0);
        //取消阴影  
        lv_obj_set_style_shadow_width(parent, 0, 0);
        lv_obj_set_style_border_color(parent, lv_color_hex(0x000000), 0);
        lv_obj_set_style_pad_all(parent,0,0);

        lv_refr_now(NULL);  // 参数传 NULL 刷新所有屏幕
        LV_LOG_WARN("[obj_event_cb_scr_sub0_scrollED]\t   reset @@@@@@@@@\n");
    }
    int32_t child_cnt = lv_obj_get_child_cnt(parent);
    for(int32_t index = 0;index < child_cnt;index++)
    {
        lv_obj_t *child_obj = lv_obj_get_child(parent,index);
        if (child_obj == NULL){
            LV_LOG_WARN("[obj_event_cb_scr_sub0_scrollED]\tget btn bro false");
        }
        //lv_obj_set_style_bg_color(child_obj,lv_palette_main(LV_PALETTE_BLUE),LV_PART_MAIN);
       // lv_event_send(child_obj, LV_EVENT_HOVER_OVER, NULL);
       lv_obj_send_event(child_obj, LV_EVENT_DEFOCUSED, NULL);
    }
       

}
/* 滑块事件回调 */
static void slider_event_cb_mv(lv_event_t * e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    int32_t value = lv_slider_get_value(slider);


    // 1. 更新标签文字
    lv_label_set_text_fmt(g_slider_label_move, "%" LV_PRId32, value);

    // 必须调用此函数，让标签刷新尺寸（以便后面获取宽度时是准确的）
    lv_obj_update_layout(g_slider_label_move);

    // 2. 获取滑块的全局坐标和宽度
    lv_area_t slider_area;
    lv_obj_get_coords(slider, &slider_area);
    int32_t slider_left = slider_area.x1;
    int32_t slider_top  = slider_area.y1;
    int32_t slider_w    = lv_area_get_width(&slider_area);

    // 3. 获取滑块范围
    int32_t min_val, max_val;
    min_val = lv_slider_get_min_value(slider);
    max_val = lv_slider_get_max_value(slider);
    float progress = (float)(value - min_val) / (max_val - min_val);
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    // 4. 获取内边距，计算拇指中心 X 坐标（全局）
    int32_t pad_left  = lv_obj_get_style_pad_left(slider, LV_PART_MAIN);
    int32_t pad_right = lv_obj_get_style_pad_right(slider, LV_PART_MAIN);
    int32_t track_w = slider_w - pad_left - pad_right;
    int32_t thumb_x = slider_left + pad_left + (int32_t)(progress * track_w);

    // 5. 【核心修正】计算标签的 Y 坐标（位于滑块正上方 15 像素处）
    int32_t label_y = slider_top - 15;

    // 6. 【防消失处理】如果滑块太靠上，强制标签不跑出屏幕顶部
    if (label_y < 0) {
        label_y = 0;  // 贴在屏幕最上方
    }

    // 7. 【居中处理】让标签的中心对准拇指的中心（而不是左上角对齐）
    lv_coord_t label_w = lv_obj_get_width(g_slider_label_move);
    int32_t label_x = thumb_x - label_w / 2;

    // 8. 防止标签左半部分跑出屏幕（左边界保护）
    if (label_x < 0) {
        label_x = 0;
    }

    // 9. 设置标签位置（全局坐标）
    //lv_obj_set_pos(g_slider_label_move, label_x, label_y);


}
/* 滑块的事件回调函数 */
static void btn_sub_event_cb(lv_event_t * e)
{
    lv_obj_t * btn_sub = lv_event_get_target_obj(e);  // 获取触发事件的对象[reference:2]
    lv_obj_t *btn_sub_label =  lv_obj_get_child(btn_sub,0);
    lv_label_set_text(btn_sub_label,"btn_sub focused!!!");
}
/* 滑块的事件回调函数 */
static void btn_sub_reset_event_cb(lv_event_t * e)
{
    lv_obj_t * btn_sub = lv_event_get_target_obj(e);  // 获取触发事件的对象[reference:2]
    lv_obj_t *btn_sub_label =  lv_obj_get_child(btn_sub,0);
    lv_label_set_text(btn_sub_label,"subButton!!!");
}
/* 滑块的事件回调函数 */
static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target_obj(e);  // 获取触发事件的滑块对象[reference:2]

    // 使用 lv_label_set_text_fmt 格式化并更新标签文本[reference:3]
    // LV_PRId32 是 LVGL 为 int32_t 提供的跨平台格式化宏
    lv_label_set_text_fmt(g_slider_label, "%" LV_PRId32, lv_slider_get_value(slider));

    // 重新将标签对齐到滑块的上方中间[reference:4]
    // 参数：要对齐的对象, 参考对象, 对齐方式, X偏移, Y偏移
    lv_obj_align_to(g_slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);
    int32_t slider_label_value = lv_slider_get_value(slider);
    int32_t slider_label_max_value = lv_slider_get_max_value(slider);

    /* 重新定位 */
    lv_point_t knob_pos;
    lv_coord_t knob_x_offset;
    get_knob_coords(slider, &knob_pos);
    knob_x_offset = get_knob_offset_x(slider);


    int32_t label_w = lv_obj_get_width(g_slider_label);
    lv_coord_t knob_w = lv_obj_get_style_width(slider, LV_PART_KNOB);
    lv_obj_align_to(g_slider_label, slider, LV_ALIGN_LEFT_MID, knob_x_offset + knob_w - label_w/2, lv_pct(5));


    if(slider_label_value == slider_label_max_value){
        lv_obj_del(slider);
        //lv_obj_delete_async(g_slider_label);
        lv_obj_delete_delayed(g_slider_label,1000);
    }
}

/* 进度条的事件回调函数 */
static void bar_event_cb(lv_event_t * e)
{
    lv_obj_t * slider  = lv_event_get_target_obj(e);
    // 2. 获取通过 user_data 传入的关联对象 (这里是 label)

    lv_obj_t * associated_bar = (lv_obj_t *) lv_event_get_user_data(e);
    int32_t value = lv_slider_get_value(slider);
    lv_bar_set_value(associated_bar,value,LV_ANIM_ON);
}


void ui_learn_lvgl_init(void){

    //1.获取当前硬件的屏幕（硬件（SDL）初始化时得到，）
    lv_obj_t * scr = lv_screen_active();
    //或者创建一个屏幕 参数为创建框的父类（上一级屏幕）,
    lv_obj_t * scr_sub = lv_obj_create(scr);
    lv_obj_set_style_pad_all(scr,0,0);
    lv_obj_set_style_pad_all(scr_sub,0,0);
    lv_obj_set_size(scr_sub,lv_pct(90),lv_pct(80));
    lv_obj_set_pos(scr_sub,lv_pct(5),lv_pct(20));
    //创建一个组件 ，参数为父类
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_t *btn_label = lv_label_create(scr);
    //lv_obj_set_align(btn,LV_ALIGN_TOP_MID);
    //lv_obj_set_pos(btn,120,20);
    /* X 坐标偏移父对象内容区域宽度的 10%，Y 坐标偏移父对象内容区域高度的 10% */
    lv_obj_set_pos(btn, lv_pct(30), lv_pct(5));
    //lv_obj_set_size(btn,100,50);
    lv_obj_set_size(btn,lv_pct(50),lv_pct(5));

    lv_obj_set_pos(btn_label, lv_pct(30), lv_pct(5));
    lv_obj_set_size(btn_label,lv_pct(50),lv_pct(5));
    lv_label_set_text_fmt(btn_label, "main_top_btn");
    lv_arclabel_set_text_horizontal_align(btn_label,LV_ARCLABEL_TEXT_ALIGN_CENTER);
    lv_arclabel_set_text_vertical_align(btn_label,LV_ARCLABEL_TEXT_ALIGN_CENTER);
    /* 1. 创建并初始化样式 用样式去设置*/
    static lv_style_t btn_label_style;
    lv_style_init(&btn_label_style);
    lv_style_set_bg_color(&btn_label_style, lv_color_hex(0xFFC0CB));  // 粉红色
    lv_style_set_bg_opa(&btn_label_style, LV_OPA_COVER);             // 不透明，否则颜色不可见
    //lv_obj_add_style(btn_label, &btn_label_style, LV_PART_MAIN);
    // 直接设置背景颜色（不创建 style 对象）
    lv_obj_set_style_bg_color(btn_label, lv_palette_main(LV_PALETTE_PINK), 0);  // 0 = LV_PART_MAIN | LV_STATE_DEFAULT  lv_palette_main(LV_PALETTE_RED)
    lv_obj_set_style_bg_opa(btn_label, LV_OPA_50, 0);
    /* 设置文字居中对齐 LVGL 标签没有文字垂直居中属性，要让文字垂直居中需要padding,而在btn_sub中，btn_sub_label是创建在btn_sub上的，直接设置居中即可。*/
    lv_obj_set_style_text_align(btn_label, LV_TEXT_ALIGN_CENTER, 0);
    //设置标签圆角
    lv_obj_set_style_radius(btn_label, 20, 0);  // 20 = 圆角半径，单位像素

    lv_obj_t *btn_sub = lv_btn_create(scr_sub);
    lv_obj_set_align(btn_sub,LV_ALIGN_OUT_BOTTOM_MID);
    lv_obj_set_size(btn_sub,lv_pct(40),lv_pct(5));
    lv_obj_set_pos(btn_sub,lv_pct(5),lv_pct(5));

    lv_obj_set_style_bg_color(btn,lv_palette_main(LV_PALETTE_RED),LV_PART_MAIN|LV_STATE_PRESSED);
    lv_obj_t *btn_sub_label = lv_label_create(btn_sub);
    lv_label_set_text(btn_sub_label, "subButton");
    lv_obj_center(btn_sub_label);

    lv_obj_add_event_cb(btn_sub,btn_sub_reset_event_cb,LV_EVENT_HOVER_LEAVE,NULL);
    lv_obj_add_event_cb(btn_sub,btn_sub_event_cb,LV_EVENT_HOVER_OVER,NULL);



    lv_obj_t *slider = lv_slider_create(scr_sub);
    lv_slider_set_range(slider,0,255);
    lv_slider_set_value(slider,0,LV_ANIM_ON);
    lv_obj_set_size(slider,lv_pct(40),lv_pct(5));
    lv_obj_set_pos(slider,lv_pct(5),100);
    lv_obj_set_style_pad_all(slider, 10, LV_PART_KNOB);    // 方法一

        /* 2. 为滑块添加事件回调 */
    // 当滑块的值发生改变时（LV_EVENT_VALUE_CHANGED），调用 slider_event_cb[reference:8]
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* 3. 创建用于显示数值的标签 */
    g_slider_label = lv_label_create(scr_sub); // 创建标签[reference:9]
    lv_label_set_text(g_slider_label, "0");               // 初始显示为 "0"[reference:10]
    lv_obj_set_style_radius(g_slider_label, LV_RADIUS_CIRCLE, LV_STATE_DEFAULT); // 或者直接设为 50
    // 设置文本颜色为蓝色 (0x5084db)
    lv_obj_set_style_text_color(g_slider_label, lv_color_hex(0x5084db), LV_STATE_DEFAULT);
    // 使用 30 像素大小的 Montserrat 字体
    lv_obj_set_style_text_font(g_slider_label, &lv_font_montserrat_30, LV_STATE_DEFAULT);
    // 设置背景颜色为淡红色，并设置透明度为不透明 (100)
    lv_obj_set_style_bg_color(g_slider_label, lv_color_hex(0xffe1d4), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_slider_label, LV_OPA_COVER, LV_STATE_DEFAULT); // LV_OPA_COVER 即 100% 不透明
    // 将标签对齐到滑块的上方中间，Y轴偏移 -15 像素[reference:11]
    lv_obj_align_to(g_slider_label, slider, LV_ALIGN_RIGHT_MID, 0, lv_pct(5));



    lv_obj_t *slider_btn = lv_slider_create(scr_sub);
    lv_obj_set_style_bg_color(slider_btn,lv_palette_main(LV_PALETTE_GREY),LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider_btn,lv_palette_main(LV_PALETTE_RED),LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_btn,lv_palette_main(LV_PALETTE_BLUE),LV_PART_KNOB);
    lv_slider_set_range(slider_btn,0,255);
    lv_slider_set_value(slider_btn,0,LV_ANIM_ON);
    lv_obj_set_size(slider_btn,lv_pct(40),lv_pct(5));
    //lv_obj_set_pos(slider_btn,20,180);

    lv_obj_align_to(slider_btn, slider,     LV_ALIGN_OUT_RIGHT_TOP,   lv_pct(15),0);

    // 创建标签（作为滑块的子对象，这样可以相对于滑块定位）
    g_slider_label_move = lv_label_create(slider_btn);
    lv_label_set_text(g_slider_label_move, "50");
    lv_obj_set_style_text_color(g_slider_label_move, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_align_to(g_slider_label_move,slider_btn,LV_ALIGN_RIGHT_MID,-10, lv_pct(5));
    // 为滑块添加事件回调
    lv_obj_add_event_cb(slider_btn, slider_event_cb_mv, LV_EVENT_VALUE_CHANGED, NULL);
    // 手动触发一次，显示初始值
    lv_obj_send_event(slider_btn, LV_EVENT_VALUE_CHANGED, NULL);

    /* 缩小圆形 knob */
    lv_obj_set_style_pad_all(slider_btn, 5, LV_PART_KNOB);    // 方法一

    lv_obj_t *bar = lv_bar_create(scr_sub);

    lv_obj_set_size(bar,lv_pct(40),lv_pct(5));
    //lv_obj_set_pos(slider_btn,20,180);
    lv_bar_set_orientation(bar,LV_BAR_ORIENTATION_AUTO);

    lv_obj_align_to(bar, slider,   LV_ALIGN_OUT_BOTTOM_MID,0,   lv_pct(50));
    lv_bar_set_max_value(bar,256);
    lv_bar_set_min_value(bar,0);
    lv_bar_set_start_value(bar,0,LV_ANIM_ON);
    lv_bar_set_mode(bar,LV_BAR_MODE_NORMAL);

    lv_obj_add_event_cb(slider_btn, bar_event_cb, LV_EVENT_VALUE_CHANGED, bar);



}
void ui_learn_layout_init(void){
    lv_obj_t * scr = lv_screen_active();

    /* 禁止主屏幕滚动：scr 默认是可滚动的，如果不禁止，
     * 拖动 scr_sub 之外的区域时 scr 也会跟着动。
     * 只让 scr_sub 内部滚动即可。 */
    //lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
    //lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLL_ELASTIC);
    //lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    //lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLL_CHAIN);
    //lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    //主屏幕第一块空间
    lv_obj_t * scr_sub0 = lv_obj_create(scr);
    lv_obj_set_style_pad_all(scr_sub0,0,0);
    lv_obj_set_size(scr_sub0,lv_pct(80),lv_pct(20));
    lv_obj_set_pos(scr_sub0,lv_pct(5),lv_pct(0));
    lv_obj_set_flex_flow(scr_sub0, LV_FLEX_FLOW_ROW_WRAP);
    
    /**
     * 改在通过内部组件的事件控制
    lv_obj_add_event_cb(scr_sub0,obj_event_cb_scr_set_Shadow,LV_EVENT_HOVER_OVER,NULL);
    lv_obj_add_event_cb(scr_sub0,obj_event_cb_scr_reset_Shadow,LV_EVENT_HOVER_LEAVE,NULL);
    */
    for (size_t i = 0; i < 11; i++)
    {
        lv_obj_t * sub_btn = lv_btn_create(scr_sub0);
        lv_obj_set_size(sub_btn,lv_pct(25),lv_pct(35));
        lv_obj_set_pos(sub_btn,lv_pct(i*5),lv_pct(i*5));
        lv_obj_add_event_cb(sub_btn,obj_event_cb_btn_set_blue,LV_EVENT_DEFOCUSED,NULL);
        lv_obj_add_event_cb(sub_btn,obj_event_cb_btn_set_red,LV_EVENT_FOCUSED,NULL);
        lv_obj_add_event_cb(sub_btn,obj_event_cb_scr_sub0_scrollED,LV_EVENT_HOVER_LEAVE,NULL);
        lv_obj_add_event_cb(sub_btn,obj_event_cb_scr_sub0_scroll,LV_EVENT_HOVER_OVER,NULL);
    }
    LV_LOG_INFO("[main]");
    //lv_obj_add_event_cb(scr_sub0,obj_event_cb_scr_sub0_scroll,LV_EVENT_SCROLL,NULL);
    //lv_obj_add_event_cb(scr_sub0,obj_event_cb_scr_sub0_scrollED,LV_EVENT_SCROLL_END,NULL);
    
    //lv_obj_t * main_btn = lv_btn_create(scr);
    //lv_obj_set_size(main_btn,lv_pct(10),lv_pct(5));
    //lv_obj_set_pos(main_btn,lv_pct(1),lv_pct(1));

    lv_obj_t * scr_sub = lv_obj_create(scr);
    lv_obj_set_size(scr_sub,lv_pct(90),lv_pct(60));
    lv_obj_set_pos(scr_sub,lv_pct(5),lv_pct(25));
    lv_obj_set_flex_flow(scr_sub, LV_FLEX_FLOW_ROW);

    lv_obj_t * scr_sub_btn = lv_obj_create(scr_sub);
    lv_obj_set_style_pad_all(scr_sub_btn,5,0);
    lv_obj_set_flex_flow(scr_sub_btn, LV_FLEX_FLOW_ROW_WRAP);
    
    
/**
 * lv_obj_set_flex_align 参数
 * 参数2 定义子元素在主轴方向上的对齐方式。 例如。 将 LV_FLEX_FLOW_ROW_WRAP 上的元素向右刷新。 （它在 CSS 中称为 justify-content ）

 * 参数3 定义子元素在交叉轴的对齐方式。 例如。 如果元素具有不同的高度，则将它们放置在轨道的底部。 （在 CSS 中称为 align-items ）

 * 参数4 定义多行元素在交叉轴的对齐方式（在 CSS 中称为 align-content ）
 */

    lv_obj_set_flex_align(scr_sub_btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    //lv_obj_set_size(scr_sub_btn,lv_pct(40),lv_pct(40));
    lv_obj_set_flex_grow(scr_sub_btn, 1);
    //lv_obj_set_pos(scr_sub_btn,lv_pct(0),lv_pct(0));
        for (size_t i = 0; i < 10; i++)
        {
            lv_obj_t * sub_btn = lv_btn_create(scr_sub_btn);
            lv_obj_set_size(sub_btn,lv_pct(20),lv_pct(10));
            
            /***
             * 在 LVGL 中，只要父对象启用了 Flex 布局，子对象的 lv_obj_set_pos() 
             * 设置的位置就会被 Flex 引擎完全覆盖和忽略
             */
//            lv_obj_set_pos(sub_btn,lv_pct(i*5),lv_pct(i*5));


        }
    lv_obj_t * scr_sub_slider  = lv_obj_create(scr_sub);

    lv_obj_set_style_pad_all(scr_sub_slider,10,0);

    
    //lv_obj_set_size(scr_sub_slider,lv_pct(50),lv_pct(60));
    lv_obj_set_flex_grow(scr_sub_slider, 2);
    //lv_obj_set_pos(scr_sub_slider,lv_pct(0),lv_pct(0));
    lv_obj_set_flex_flow(scr_sub_slider, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(scr_sub_slider, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        for (size_t i = 0; i < 10; i++)
        {
            lv_obj_t * sub_slider = lv_slider_create(scr_sub_slider);
            lv_obj_set_style_pad_all(sub_slider, 10, LV_PART_KNOB); 
            lv_obj_set_size(sub_slider,lv_pct(15),lv_pct(5));
            lv_obj_set_pos(sub_slider,lv_pct(i*5),lv_pct(i*5));
        }
    ////
    lv_obj_t * scr_sub_label = lv_obj_create(scr_sub);
    lv_obj_set_style_pad_all(scr_sub_label,5,0);
    //lv_obj_set_size(scr_sub_label,lv_pct(50),lv_pct(40));
    lv_obj_set_flex_grow(scr_sub_label, 3);
    //lv_obj_set_pos(scr_sub_slider,lv_pct(0),lv_pct(0));
    lv_obj_set_flex_flow(scr_sub_label, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(scr_sub_label, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
        for (size_t i = 0; i < 10; i++)
        {
            lv_obj_t * sub_label= lv_label_create(scr_sub_label);
            lv_obj_set_style_pad_all(sub_label, 15, LV_PART_KNOB); 
            lv_obj_set_size(sub_label,lv_pct(25),lv_pct(15));
            lv_obj_set_pos(sub_label,lv_pct(i*5),lv_pct(i*5));
           
            lv_label_set_text_fmt(sub_label, "text%" LV_PRId32, i);
        }
    //主屏幕而第三块空间
    lv_obj_t * scr_sub2 = lv_obj_create(scr);
    lv_obj_set_size(scr_sub2,lv_pct(90),lv_pct(70));
    lv_obj_set_pos(scr_sub2,lv_pct(5),lv_pct(90));

    static int32_t column_dsc[] =   {LV_GRID_FR(1),LV_GRID_FR(2),LV_GRID_FR(3),LV_GRID_FR(4) , LV_GRID_TEMPLATE_LAST};
    static int32_t ros_dsc[]    =   {LV_GRID_FR(1),LV_GRID_FR(2),LV_GRID_FR(3),LV_GRID_FR(4),  LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(scr_sub2,column_dsc,ros_dsc);
   // 在 LVGL 中，一个容器只能使用一种布局。
    //lv_obj_set_flex_flow(scr_sub2, LV_FLEX_FLOW_ROW_WRAP);



    for (size_t i = 0; i < 16; i++)
    {   
        /***
         * 可能出现重叠说明：
         * 第一二行重叠的原因：
         * `lv_btn_create()` 创建的按钮有默认最小高度，
         * 而容器 `scr_sub2` 使用 `lv_obj_create()` 创建（自带默认 padding），
         * 加上行描述符 `LV_GRID_FR(1)` 和 `LV_GRID_FR(2)` 分配给第一、第二行的实际像素高度过小，
         * 小于按钮的最小高度限制，导致按钮溢出单元格边界，视觉上产生重叠。
         * 
         */
        lv_obj_t * sub_btn = lv_btn_create(scr_sub2);
        lv_obj_set_style_bg_color(sub_btn, lv_color_hex(i * 0x1111), 0);
        lv_obj_set_grid_cell(sub_btn,LV_GRID_ALIGN_CENTER,i%4,1,LV_GRID_ALIGN_CENTER,i/4,1);
    }

}
void ui_learn_otherwidgets_init(void){
    lv_obj_t * scr = lv_screen_active();
    //主屏幕第一块空间
    lv_obj_t * scr_sub0 = lv_obj_create(scr);
    lv_obj_set_style_pad_all(scr_sub0,0,0);
    lv_obj_set_size(scr_sub0,lv_pct(90),lv_pct(50));
    lv_obj_set_pos(scr_sub0,lv_pct(5),lv_pct(0));
    lv_obj_set_flex_flow(scr_sub0, LV_FLEX_FLOW_ROW_WRAP);
    {
        lv_obj_t * sub_btn = lv_btn_create(scr_sub0);
        lv_obj_set_size(sub_btn,lv_pct(20),lv_pct(30));
        lv_obj_t * sub_arc = lv_arc_create(scr_sub0);
        lv_obj_set_size(sub_arc,lv_pct(35),lv_pct(35));
        lv_obj_center(sub_arc);

        // 设置弧度范围
        lv_arc_set_bg_angles(sub_arc,  135, 45);       // 背景弧范围
        lv_arc_set_angles(sub_arc,   135, 45);          // 指示弧范围
        lv_arc_set_value(sub_arc, 0);               // 值 0~100
    
        // === 设置粗细 ===
        lv_obj_set_style_arc_width(sub_arc, 12, LV_PART_MAIN);       // 背景弧粗 12px
        lv_obj_set_style_arc_width(sub_arc, 12, LV_PART_INDICATOR);  // 指示弧粗 12px
    
        // === 设置颜色 ===
        lv_obj_set_style_arc_color(sub_arc, lv_palette_main(LV_PALETTE_NONE), LV_PART_MAIN);       // 背景灰色
        lv_obj_set_style_arc_color(sub_arc, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);   // 指示绿色
    
        // === 设置圆角端点 ===
        lv_obj_set_style_arc_rounded(sub_arc, true, LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(sub_arc, true, LV_PART_INDICATOR);
        
        // === 设置旋钮 === 旋钮用的是 bg_color 而不是 arc_color，因为旋钮本质上是一个圆形背景对象，不是弧线。
        lv_obj_set_style_arc_width(sub_arc, 0, LV_PART_KNOB);        // 旋钮大小
        lv_obj_set_style_bg_color(sub_arc, lv_palette_main(LV_PALETTE_RED), LV_PART_KNOB);
        lv_obj_set_style_bg_opa(sub_arc, LV_OPA_COVER, LV_PART_KNOB);
        
    }
    
    //主屏幕而第二块空间
    lv_obj_t * scr_sub1 = lv_obj_create(scr);
    lv_obj_set_size(scr_sub1,lv_pct(90),lv_pct(60));
    lv_obj_set_pos(scr_sub1,lv_pct(5),lv_pct(55));

    static int32_t column_dsc[] =   {LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1) , LV_GRID_TEMPLATE_LAST};
    static int32_t ros_dsc[]    =   {LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1),  LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(scr_sub1,column_dsc,ros_dsc);
    {
        lv_obj_t * sub_btn = lv_btn_create(scr_sub1);
        lv_obj_set_style_bg_color(sub_btn, lv_color_hex( 0x1111), 0);
        lv_obj_set_grid_cell(sub_btn,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_CENTER,0,1);
        static const char * btnm_map[] = {"1", "2", "3", "4", "5", "\n",
                                  "6", "7", "8", "9", "0", "\n",
                                  "Action1", "Action2", ""
                                 };


        lv_obj_t * btnm1 = lv_buttonmatrix_create(scr_sub1);
        lv_buttonmatrix_set_map(btnm1, btnm_map);
        lv_buttonmatrix_set_button_width(btnm1, 10, 2);        /*Make "Action1" twice as wide as "Action2"*/
        lv_buttonmatrix_set_button_ctrl(btnm1, 10, LV_BUTTONMATRIX_CTRL_CHECKABLE);
        lv_buttonmatrix_set_button_ctrl(btnm1, 11, LV_BUTTONMATRIX_CTRL_CHECKED);
        lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_grid_cell(btnm1,LV_GRID_ALIGN_START,1,1,LV_GRID_ALIGN_START,1,1);

    }

}
#define CANVAS_WIDTH  1920
#define CANVAS_HEIGHT  1080
lv_obj_t * g_canvas = NULL;

void ui_learn_canvas_init(void){

    /*Create a buffer for the canvas*/
    static uint8_t cbuf[LV_CANVAS_BUF_SIZE(CANVAS_WIDTH, CANVAS_HEIGHT, 32, LV_DRAW_BUF_STRIDE_ALIGN)];
    
    /*Create a canvas and initialize its palette*/
    g_canvas = lv_canvas_create(lv_screen_active());
    lv_canvas_set_buffer(g_canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_ARGB8888);
    lv_canvas_fill_bg(g_canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
    lv_obj_center(g_canvas);

    lv_layer_t layer;
    lv_canvas_init_layer(g_canvas, &layer);
    /* 3. 重点：不声明 img_star，而是直接写本地文件路径 */
    lv_draw_image_dsc_t dsc;
    lv_draw_image_dsc_init(&dsc);
    /* 1. 定义一个足够大的数组（建议比实际长度大一些，防止溢出） */
    char file_path[64]; 
    int my_number = 25700;
    
    /* 2. 拼接 "xxx_" 和数字，存进数组 */
    lv_snprintf(file_path, sizeof(file_path), "G:/TXcloud/20250727/1080_1//1000000%d.png", my_number);
    
    /* 【关键点】指向本地图片 */
    /* 如果 LV_FS_STDIO_PATH 定义了 "D:/"，那这里写 "S:test.png" 就会去读取 D:/test.png */
    //dsc.src = "G:/TXcloud/20250727/1080_1//100000025700.png";  
    dsc.src = file_path;
    // 或者如果你没有改 LV_FS_STDIO_PATH，直接写绝对路径：
    // dsc.src = "D:/my_images/star.png"; 

    /* 4. 设置显示坐标 */
    /* 注意：要确保图片本身存在，且大小不要超 canvas 画布，否则会裁切 */
    lv_area_t coords = {0, 0, CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1}; // 假设图片大小为 100x100
//    for (uint32_t index = 0 ;index < 1000; index++){
//    }
            /* 5. 执行绘制 */
            lv_draw_image(&layer, &dsc, &coords);
        
            lv_canvas_finish_layer(g_canvas, &layer);
            lv_snprintf(file_path, sizeof(file_path), "G:/TXcloud/20250727/1080_1//1000000%d.png", my_number++);
            dsc.src = file_path;
            lv_delay_ms(33);


}
