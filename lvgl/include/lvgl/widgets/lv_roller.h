/**
 * @file lv_roller.h
 * @brief 滚轮(Roller)控件,用于通过滚动来在选项中进行选择
 *
 */

#ifndef LV_ROLLER_H
#define LV_ROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../core/lv_obj.h"

#if LV_USE_ROLLER != 0

/*Testing of dependencies*/
/*测试依赖项*/
#if LV_USE_LABEL == 0
#error "lv_roller: lv_label is required. Enable it in lv_conf.h (LV_USE_ROLLER 1)"
#endif

#include "lv_label.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/** Roller mode. */
/** 滚轮模式。 */
typedef enum {
    LV_ROLLER_MODE_NORMAL,   /**< Normal mode (roller ends at the end of the options). */
                             /**< 普通模式(滚轮滚动到最后一个选项处结束)。 */
    LV_ROLLER_MODE_INFINITE, /**< Infinite mode (roller can be scrolled forever). */
                             /**< 无限模式(滚轮可以无限滚动)。 */
} lv_roller_mode_t;

#if LV_USE_OBJ_PROPERTY
enum _lv_property_roller_id_t {
    LV_PROPERTY_ID2(ROLLER, OPTIONS,            LV_PROPERTY_TYPE_TEXT,  LV_PROPERTY_TYPE_INT,   0),
    LV_PROPERTY_ID2(ROLLER, SELECTED,           LV_PROPERTY_TYPE_INT,   LV_PROPERTY_TYPE_INT, 1),
    LV_PROPERTY_ID(ROLLER, VISIBLE_ROW_COUNT,   LV_PROPERTY_TYPE_INT,   2),
    LV_PROPERTY_ROLLER_END,
};
#endif

/** 滚轮控件的对象类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_roller_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a roller object
 * 创建一个滚轮控件对象
 * @param parent    pointer to an object, it will be the parent of the new roller.
 * @param parent    指向一个对象,它将是新滚轮的父对象。
 * @return          pointer to the created roller
 * @return          指向创建的滚轮对象的指针
 */
lv_obj_t * lv_roller_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the options on a roller
 * 设置滚轮的选项
 * @param obj       pointer to roller object
 * @param obj       指向滚轮对象
 * @param options   a string with '\n' separated options. E.g. "One\nTwo\nThree"
 * @param options   以'\n'分隔选项的字符串,例如 "One\nTwo\nThree"
 * @param mode      `LV_ROLLER_MODE_NORMAL` or `LV_ROLLER_MODE_INFINITE`
 * @param mode      `LV_ROLLER_MODE_NORMAL`(普通模式)或 `LV_ROLLER_MODE_INFINITE`(无限模式)
 */
void lv_roller_set_options(lv_obj_t * obj, const char * options, lv_roller_mode_t mode);

/**
 * Set the selected option
 * 设置选中的选项
 * @param obj       pointer to a roller object
 * @param obj       指向滚轮对象
 * @param sel_opt   index of the selected option (0 ... number of option - 1);
 * @param sel_opt   选中选项的索引(0 ... 选项数 - 1);
 * @param anim   LV_ANIM_ON: set with animation; LV_ANIM_OFF set immediately
 * @param anim   LV_ANIM_ON:带动画设置;LV_ANIM_OFF:立即设置
 */
void lv_roller_set_selected(lv_obj_t * obj, uint32_t sel_opt, lv_anim_enable_t anim);

/**
 * Sets the given string as the selection on the roller. Does not alter the current selection on failure.
 * 将给定的字符串设置为滚轮当前选中的选项。设置失败时不改变当前选中项。
 * @param obj               pointer to roller object
 * @param obj               指向滚轮对象
 * @param sel_opt   pointer to the string you want to set as an option
 * @param sel_opt   指向要设置为选项的字符串的指针
 * @param anim          LV_ANIM_ON: set with animation; LV_ANIM_OFF set immediately
 * @param anim          LV_ANIM_ON:带动画设置;LV_ANIM_OFF:立即设置
 * @return                  `true` if set successfully and `false` if the given string does not exist as an option in the roller
 * @return                  设置成功返回 `true`;若给定字符串在滚轮选项中不存在则返回 `false`
 */
bool lv_roller_set_selected_str(lv_obj_t * obj, const char * sel_opt, lv_anim_enable_t anim);

/**
 * Set the height to show the given number of rows (options)
 * 设置高度以显示给定数量的行(选项)
 * @param obj       pointer to a roller object
 * @param obj       指向滚轮对象
 * @param row_cnt   number of desired visible rows
 * @param row_cnt   期望可见的行数
 */
void lv_roller_set_visible_row_count(lv_obj_t * obj, uint32_t row_cnt);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the index of the selected option
 * 获取选中选项的索引
 * @param obj       pointer to a roller object
 * @param obj       指向滚轮对象
 * @return          index of the selected option (0 ... number of option - 1);
 * @return          选中选项的索引(0 ... 选项数 - 1);
 */
uint32_t lv_roller_get_selected(const lv_obj_t * obj);

/**
 * Get the current selected option as a string.
 * 获取当前选中的选项字符串。
 * @param obj       pointer to roller object
 * @param obj       指向滚轮对象
 * @param buf       pointer to an array to store the string
 * @param buf       指向用于存储字符串的数组的指针
 * @param buf_size  size of `buf` in bytes. 0: to ignore it.
 * @param buf_size  `buf` 的大小(以字节为单位)。0:表示忽略它。
 */
void lv_roller_get_selected_str(const lv_obj_t * obj, char * buf, uint32_t buf_size);

/**
 * Get the options of a roller
 * 获取滚轮的选项
 * @param obj       pointer to roller object
 * @param obj       指向滚轮对象
 * @return          the options separated by '\n'-s (E.g. "Option1\nOption2\nOption3")
 * @return          以'\n'分隔的选项字符串(例如 "Option1\nOption2\nOption3")
 */
const char * lv_roller_get_options(const lv_obj_t * obj);

/**
 * Get the total number of options
 * 获取选项的总数
 * @param obj   pointer to a roller object
 * @param obj   指向滚轮对象
 * @return      the total number of options
 * @return      选项的总数
 */
uint32_t lv_roller_get_option_count(const lv_obj_t * obj);

/**
 * Get an option as a string.
 * 获取指定索引的选项字符串。
 * @param obj       pointer to roller object
 * @param obj       指向滚轮对象
 * @param option    index of chosen option
 * @param option    所选选项的索引
 * @param buf       pointer to an array to store the string
 * @param buf       指向用于存储字符串的数组的指针
 * @param buf_size  size of `buf` in bytes. 0: to ignore it.
 * @param buf_size  `buf` 的大小(以字节为单位)。0:表示忽略它。
 * @return          LV_RESULT_OK if option found
 * @return          如果找到选项则返回 LV_RESULT_OK
 */
lv_result_t lv_roller_get_option_str(const lv_obj_t * obj, uint32_t option, char * buf, uint32_t buf_size);

#if LV_USE_OBSERVER
/**
 * Bind an integer Subject to a Roller's value.
 * 将整型 Subject 绑定到滚轮的数值上。
 * @param obj       pointer to Roller
 * @param obj       指向滚轮对象
 * @param subject   pointer to Subject
 * @param subject   指向 Subject 的指针
 * @return          pointer to newly-created Observer
 * @return          指向新创建的 Observer 的指针
 */
lv_observer_t * lv_roller_bind_value(lv_obj_t * obj, lv_subject_t * subject);
#endif

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_ROLLER*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_ROLLER_H*/
