/**
 * @file lv_spinbox.h
 * @brief 数字输入框(Spinbox)控件,可编辑数字,并支持步进/加减/逐位选择等功能
 *
 */

#ifndef LV_SPINBOX_H
#define LV_SPINBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "lv_textarea.h"

#if LV_USE_SPINBOX

/*********************
 *      DEFINES
 *      宏定义
 *********************/
/** 数字输入框的最大位数 */
#define LV_SPINBOX_MAX_DIGIT_COUNT 10

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/** 数字输入框控件的对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_spinbox_class;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_spinbox_id_t {
    LV_PROPERTY_ID(SPINBOX, VALUE,                LV_PROPERTY_TYPE_INT, 0),
    LV_PROPERTY_ID(SPINBOX, ROLLOVER,             LV_PROPERTY_TYPE_BOOL, 1),
    LV_PROPERTY_ID(SPINBOX, DIGIT_COUNT,          LV_PROPERTY_TYPE_INT, 2),
    LV_PROPERTY_ID(SPINBOX, DEC_POINT_POS,        LV_PROPERTY_TYPE_INT, 3),
    LV_PROPERTY_ID(SPINBOX, STEP,                 LV_PROPERTY_TYPE_INT, 4),
    LV_PROPERTY_ID(SPINBOX, MIN_VALUE,            LV_PROPERTY_TYPE_INT, 5),
    LV_PROPERTY_ID(SPINBOX, MAX_VALUE,            LV_PROPERTY_TYPE_INT, 6),
    LV_PROPERTY_ID(SPINBOX, DIGIT_STEP_DIRECTION, LV_PROPERTY_TYPE_INT, 7),
    LV_PROPERTY_SPINBOX_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a spinbox object
 * 创建一个数字输入框对象
 * @param parent    pointer to an object, it will be the parent of the new spinbox
 * @param parent    指向一个对象,它将是新数字输入框的父对象
 * @return          pointer to the created spinbox
 * @return          指向创建的数字输入框对象的指针
 */
lv_obj_t * lv_spinbox_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set spinbox value
 * 设置数字输入框的数值
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @param v     value to be set
 * @param v     要设置的数值
 */
void lv_spinbox_set_value(lv_obj_t * obj, int32_t v);

/**
 * Set spinbox rollover function
 * 设置数字输入框的循环(回绕)功能
 * @param obj       pointer to spinbox
 * @param obj       指向数字输入框
 * @param rollover  true or false to enable or disable (default)
 * @param rollover  true 或 false 来启用或禁用(默认禁用)
 */
void lv_spinbox_set_rollover(lv_obj_t * obj, bool rollover);

/**
 * Set spinbox digit format (digit count and decimal format)
 * 设置数字输入框的位数格式(位数和小数格式)
 * @param obj           pointer to spinbox
 * @param obj           指向数字输入框
 * @param digit_count   number of digit excluding the decimal separator and the sign
 * @param digit_count   位数,不含小数点分隔符和符号
 * @param sep_pos       number of digit before the decimal point. If 0, decimal point is not
 * @param sep_pos       小数点前面的位数。如果为 0,则不显示小数点
 * shown
 * 显示小数点
 */
void lv_spinbox_set_digit_format(lv_obj_t * obj, uint32_t digit_count, uint32_t sep_pos);

/**
 * Set the number of digits
 * 设置数字位数
 * @param obj           pointer to spinbox
 * @param obj           指向数字输入框
 * @param digit_count   number of digits
 * @param digit_count   位数
 */
void lv_spinbox_set_digit_count(lv_obj_t * obj, uint32_t digit_count);

/**
 * Set the position of the decimal point
 * 设置小数点的位置
 * @param obj           pointer to spinbox
 * @param obj           指向数字输入框
 * @param dec_point_pos 0: there is no separator, 2: two integer digits
 * @param dec_point_pos 0:没有小数点分隔符,2:两个整数位
 */
void lv_spinbox_set_dec_point_pos(lv_obj_t * obj, uint32_t dec_point_pos);

/**
 * Set spinbox step
 * 设置数字输入框的步进值
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @param step  steps on increment/decrement. Can be 1, 10, 100, 1000, etc the digit that will change.
 * @param step  递增/递减的步进值,可以是 1, 10, 100, 1000 等,即每次改变的数字位。
 */
void lv_spinbox_set_step(lv_obj_t * obj, uint32_t step);

/**
 * Set spinbox value range
 * 设置数字输入框的数值范围
 * @param obj       pointer to spinbox
 * @param obj       指向数字输入框
 * @param min_value minimum value, inclusive
 * @param min_value 最小值(含)
 * @param max_value maximum value, inclusive
 * @param max_value 最大值(含)
 */
void lv_spinbox_set_range(lv_obj_t * obj, int32_t min_value, int32_t max_value);

/**
 * Set the minimum value
 * 设置最小值
 * @param obj       pointer to spinbox
 * @param obj       指向数字输入框
 * @param min_value the minimum value
 * @param min_value 最小值
 */
void lv_spinbox_set_min_value(lv_obj_t * obj, int32_t min_value);

/**
 * Set the maximum value
 * 设置最大值
 * @param obj       pointer to spinbox
 * @param obj       指向数字输入框
 * @param max_value the maximum value
 * @param max_value 最大值
 */
void lv_spinbox_set_max_value(lv_obj_t * obj, int32_t max_value);

/**
 * Set cursor position to a specific digit for edition
 * 将光标位置设置到指定的数字位以便编辑
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @param pos   selected position in spinbox
 * @param pos   选中的位置(数字位)
 */
void lv_spinbox_set_cursor_pos(lv_obj_t * obj, uint32_t pos);

/**
 * Set direction of digit step when clicking an encoder button while in editing mode
 * 设置在编辑模式下点击编码器按键时数字步进的方向
 * @param obj           pointer to spinbox
 * @param obj           指向数字输入框
 * @param direction     the direction (LV_DIR_RIGHT or LV_DIR_LEFT)
 * @param direction     方向(LV_DIR_RIGHT 或 LV_DIR_LEFT)
 */
void lv_spinbox_set_digit_step_direction(lv_obj_t * obj, lv_dir_t direction);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get spinbox rollover function status
 * 获取数字输入框循环(回绕)功能的状态
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 */
bool lv_spinbox_get_rollover(lv_obj_t * obj);

/**
 * Get the spinbox numeral value (user has to convert to float according to its digit format)
 * 获取数字输入框的数值(用户需根据自己的位数格式将其转换为浮点数)
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @return      value integer value of the spinbox
 * @return      数字输入框的整型数值
 */
int32_t lv_spinbox_get_value(lv_obj_t * obj);

/**
 * Get the spinbox step value (user has to convert to float according to its digit format)
 * 获取数字输入框的步进值(用户需根据自己的位数格式将其转换为浮点数)
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @return      value integer step value of the spinbox
 * @return      数字输入框的整型步进值
 */
int32_t lv_spinbox_get_step(lv_obj_t * obj);

/**
 * Get the spinbox digit count
 * 获取数字输入框的位数
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @return      number of digits
 * @return      位数
 */
uint32_t lv_spinbox_get_digit_count(lv_obj_t * obj);

/**
 * Get the decimal point position
 * 获取小数点的位置
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @return      decimal point position
 * @return      小数点的位置
 */
uint32_t lv_spinbox_get_dec_point_pos(lv_obj_t * obj);

/**
 * Get the spinbox minimum value
 * 获取数字输入框的最小值
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @return      minimum value
 * @return      最小值
 */
int32_t lv_spinbox_get_min_value(lv_obj_t * obj);

/**
 * Get the spinbox maximum value
 * 获取数字输入框的最大值
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @return      maximum value
 * @return      最大值
 */
int32_t lv_spinbox_get_max_value(lv_obj_t * obj);

/**
 * Get the digit step direction
 * 获取数字步进方向
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 * @return      direction (LV_DIR_RIGHT or LV_DIR_LEFT)
 * @return      方向(LV_DIR_RIGHT 或 LV_DIR_LEFT)
 */
lv_dir_t lv_spinbox_get_digit_step_direction(lv_obj_t * obj);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**
 * Select next lower digit for edition by dividing the step by 10
 * 通过将步进值除以 10 来选择下一个更低的数字位进行编辑
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 */
void lv_spinbox_step_next(lv_obj_t * obj);

/**
 * Select next higher digit for edition by multiplying the step by 10
 * 通过将步进值乘以 10 来选择下一个更高的数字位进行编辑
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 */
void lv_spinbox_step_prev(lv_obj_t * obj);

/**
 * Increment spinbox value by one step
 * 将数字输入框的数值增加一个步进
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 */
void lv_spinbox_increment(lv_obj_t * obj);

/**
 * Decrement spinbox value by one step
 * 将数字输入框的数值减少一个步进
 * @param obj   pointer to spinbox
 * @param obj   指向数字输入框
 */
void lv_spinbox_decrement(lv_obj_t * obj);



#if LV_USE_OBSERVER
/**
 * Bind an integer subject to a Spinbox's value.
 * 将整型 subject 绑定到数字输入框的数值上。
 * @param obj       pointer to Spinbox
 * @param obj       指向数字输入框
 * @param subject   pointer to Subject
 * @param subject   指向 Subject 的指针
 * @return          pointer to newly-created Observer
 * @return          指向新创建的 Observer 的指针
 */
lv_observer_t * lv_spinbox_bind_value(lv_obj_t * obj, lv_subject_t * subject);
#endif

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_SPINBOX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif /*LV_SPINBOX_H*/
