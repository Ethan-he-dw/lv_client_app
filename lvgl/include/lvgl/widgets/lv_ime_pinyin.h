/**
 * @file lv_ime_pinyin.h
 * 拼音输入法(IME Pinyin)控件：支持26键和9键两种拼音输入模式
 *
 */
#ifndef LV_IME_PINYIN_H
#define LV_IME_PINYIN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"
#include "../core/lv_obj.h"

#if LV_USE_IME_PINYIN != 0

/*********************
 *      DEFINES
 *      宏定义
 *********************/
/*K9模式下允许输入的最大拼音字符数*/
#define LV_IME_PINYIN_K9_MAX_INPUT  7

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/*拼音输入法的工作模式*/
typedef enum {
    /*26键(全键盘)拼音输入模式*/
    LV_IME_PINYIN_MODE_K26,
    /*9键(T9)拼音输入模式*/
    LV_IME_PINYIN_MODE_K9,
    /*9键数字输入模式*/
    LV_IME_PINYIN_MODE_K9_NUMBER,
} lv_ime_pinyin_mode_t;

/*Data of pinyin_dict*/
/*拼音字典的数据结构*/
typedef struct {
    /*拼音字符串*/
    const char * const py;
    /*拼音对应的候选字/词列表*/
    const char * const py_mb;
} lv_pinyin_dict_t;

/*Data of 9-key input(k9) mode*/
/*9键输入（k9）模式的数据结构*/
typedef struct {
    /*拼音字符串缓冲区*/
    char py_str[7];
} ime_pinyin_k9_py_str_t;

/***********************
 * GLOBAL VARIABLES
 * 全局变量
 ***********************/

/*拼音输入法控件的类对象*/
extern const lv_obj_class_t lv_ime_pinyin_class;

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/
/*创建拼音输入法对象*/
lv_obj_t * lv_ime_pinyin_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the keyboard of Pinyin input method.
 * 设置拼音输入法所使用的键盘
 * @param obj  pointer to a Pinyin input method object
 * @param obj  指向拼音输入法对象的指针
 * @param kb pointer to a Pinyin input method keyboard
 * @param kb 指向拼音输入法键盘的指针
 */
void lv_ime_pinyin_set_keyboard(lv_obj_t * obj, lv_obj_t * kb);

/**
 * Set the dictionary of Pinyin input method.
 * 设置拼音输入法所使用的字典
 * @param obj  pointer to a Pinyin input method object
 * @param obj  指向拼音输入法对象的指针
 * @param dict pointer to a Pinyin input method dictionary
 * @param dict 指向拼音输入法字典的指针
 */
void lv_ime_pinyin_set_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict);

/**
 * Set mode, 26-key input(k26) or 9-key input(k9).
 * 设置输入模式：26键输入（k26）或9键输入（k9）
 * @param obj  pointer to a Pinyin input method object
 * @param obj  指向拼音输入法对象的指针
 * @param mode   the mode from 'lv_ime_pinyin_mode_t'
 * @param mode   来自 'lv_ime_pinyin_mode_t' 的模式
 */
void lv_ime_pinyin_set_mode(lv_obj_t * obj, lv_ime_pinyin_mode_t mode);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Set the dictionary of Pinyin input method.
 * 获取拼音输入法所使用的键盘
 * @param obj  pointer to a Pinyin IME object
 * @param obj  指向拼音输入法对象的指针
 * @return     pointer to the Pinyin IME keyboard
 * @return     指向拼音输入法键盘的指针
 */
lv_obj_t * lv_ime_pinyin_get_kb(lv_obj_t * obj);

/**
 * Set the dictionary of Pinyin input method.
 * 获取拼音输入法的候选面板
 * @param obj  pointer to a Pinyin input method object
 * @param obj  指向拼音输入法对象的指针
 * @return     pointer to the Pinyin input method candidate panel
 * @return     指向拼音输入法候选面板的指针
 */
lv_obj_t * lv_ime_pinyin_get_cand_panel(lv_obj_t * obj);

/**
 * Set the dictionary of Pinyin input method.
 * 获取拼音输入法所使用的字典
 * @param obj  pointer to a Pinyin input method object
 * @param obj  指向拼音输入法对象的指针
 * @return     pointer to the Pinyin input method dictionary
 * @return     指向拼音输入法字典的指针
 */
const lv_pinyin_dict_t * lv_ime_pinyin_get_dict(lv_obj_t * obj);

/*=====================
 * Other functions
 * 其他函数
 *====================*/

/**********************
 *      MACROS
 *      宏
 **********************/

#endif  /*LV_IME_PINYIN*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_IME_PINYIN_H*/
