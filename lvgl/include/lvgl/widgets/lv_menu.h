/**
 * @file lv_menu.h
 * 菜单控件头文件：可创建带侧边栏、页面跳转和返回按钮的菜单
 *
 */

#ifndef LV_MENU_H
#define LV_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../core/lv_obj.h"
#include "../core/lv_obj_property.h"

#if LV_USE_MENU

#if LV_USE_FLEX == 0
#error "lv_menu: lv_flex is required. Enable it in lv_conf.h (LV_USE_FLEX 1)"
#endif

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/
/** 菜单头部模式枚举 */
typedef enum {
    LV_MENU_HEADER_TOP_FIXED,   /**< Header is positioned at the top */
    /** 头部固定在顶部 */
    LV_MENU_HEADER_TOP_UNFIXED, /**< Header is positioned at the top and can be scrolled out of view*/
    /** 头部位于顶部，且可随滚动移出视野 */
    LV_MENU_HEADER_BOTTOM_FIXED /**< Header is positioned at the bottom */
    /** 头部固定在底部 */
} lv_menu_mode_header_t;

/** 菜单根级返回按钮模式枚举 */
typedef enum {
    /** 禁用根级返回按钮 */
    LV_MENU_ROOT_BACK_BUTTON_DISABLED,
    /** 启用根级返回按钮 */
    LV_MENU_ROOT_BACK_BUTTON_ENABLED
} lv_menu_mode_root_back_button_t;

/** 菜单控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_menu_class;
/** 菜单页面控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_menu_page_class;
/** 菜单容器控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_menu_cont_class;
/** 菜单区块控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_menu_section_class;
/** 菜单分隔线控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_menu_separator_class;
/** 菜单侧边栏容器控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_menu_sidebar_cont_class;
/** 菜单主区域容器控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_menu_main_cont_class;
/** 菜单侧边栏头部容器控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_menu_sidebar_header_cont_class;
/** 菜单主区域头部容器控件类 */
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_menu_main_header_cont_class;

#if LV_USE_OBJ_PROPERTY
enum __lv_property_menu_id_t {
    LV_PROPERTY_ID(MENU, MODE_HEADER,           LV_PROPERTY_TYPE_INT, 0),
    LV_PROPERTY_ID(MENU, MODE_ROOT_BACK_BUTTON, LV_PROPERTY_TYPE_INT, 1),
    LV_PROPERTY_MENU_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a menu object
 * 创建一个菜单对象
 * @param parent    pointer to an object, it will be the parent of the new menu
 * @param parent    指向一个对象，它将成为新菜单的父对象
 * @return          pointer to the created menu
 * @return          指向创建好的菜单对象的指针
 */
lv_obj_t * lv_menu_create(lv_obj_t * parent);

/**
 * Create a menu page object.
 * 创建一个菜单页面对象。
 *
 * This call inserts the new page under menu->storage as its parent, which is itself a
 * child of the menu, so the resulting object hierarchy is: menu => storage => new_page
 * where `storage` is a Base Widget.
 * 此调用会将新页面插入到 menu->storage 下，以其作为父对象；而 storage 本身是菜单的子对象，
 * 因此最终的对象层级结构为: menu => storage => new_page，其中 `storage` 是一个基础控件。
 * @param menu      pointer to menu object.
 * @param menu      指向菜单对象
 * @param title     pointer to text for title in header (NULL to not display title)
 * @param title     指向头部标题文本的指针(NULL 表示不显示标题)
 * @return          pointer to the created menu page
 * @return          指向创建好的菜单页面对象的指针
 */
lv_obj_t * lv_menu_page_create(lv_obj_t * menu, char const * const title);

/**
 * Create a menu cont object
 * 创建一个菜单容器对象
 * @param parent    pointer to a menu page or menu section object, it will be the parent of the new menu cont object
 * @param parent    指向菜单页面或菜单区块对象，它将成为新菜单容器的父对象
 * @return          pointer to the created menu cont
 * @return          指向创建好的菜单容器对象的指针
 */
lv_obj_t * lv_menu_cont_create(lv_obj_t * parent);

/**
 * Create a menu section object
 * 创建一个菜单区块对象
 * @param parent    pointer to a menu page object, it will be the parent of the new menu section object
 * @param parent    指向菜单页面对象，它将成为新菜单区块的父对象
 * @return          pointer to the created menu section
 * @return          指向创建好的菜单区块对象的指针
 */
lv_obj_t * lv_menu_section_create(lv_obj_t * parent);

/**
 * Create a menu separator object
 * 创建一个菜单分隔线对象
 * @param parent    pointer to a menu page object, it will be the parent of the new menu separator object
 * @param parent    指向菜单页面对象，它将成为新菜单分隔线的父对象
 * @return          pointer to the created menu separator
 * @return          指向创建好的菜单分隔线对象的指针
 */
lv_obj_t * lv_menu_separator_create(lv_obj_t * parent);
/*=====================
 * Setter functions
 * 设置函数
 *====================*/
/**
 * Set menu page to display in main
 * 设置主区域要显示的菜单页面
 * @param obj       pointer to the menu
 * @param obj       指向菜单
 * @param page      pointer to the menu page to set (NULL to clear main and clear menu history)
 * @param page      指向要设置的菜单页面(NULL 表示清空主区域并清空菜单历史)
 */
void lv_menu_set_page(lv_obj_t * obj, lv_obj_t * page);

/**
 * Set menu page title
 * 设置菜单页面标题
 * @param page      pointer to the menu page
 * @param page      指向菜单页面
 * @param title     pointer to text for title in header (NULL to not display title)
 * @param title     指向头部标题文本的指针(NULL 表示不显示标题)
 */
void lv_menu_set_page_title(lv_obj_t * page, char const * const title);

/**
 * Set menu page title with a static text. It will not be saved by the label so the 'text' variable
 * has to be 'alive' while the page exists.
 * 使用静态文本设置菜单页面标题。该文本不会被标签保存，因此页面存在期间 'text' 变量必须保持有效。
 * @param page      pointer to the menu page
 * @param page      指向菜单页面
 * @param title     pointer to text for title in header (NULL to not display title)
 * @param title     指向头部标题文本的指针(NULL 表示不显示标题)
 */
void lv_menu_set_page_title_static(lv_obj_t * page, char const * const title);

/**
 * Set menu page to display in sidebar
 * 设置侧边栏要显示的菜单页面
 * @param obj       pointer to the menu
 * @param obj       指向菜单
 * @param page      pointer to the menu page to set (NULL to clear sidebar)
 * @param page      指向要设置的菜单页面(NULL 表示清空侧边栏)
 */
void lv_menu_set_sidebar_page(lv_obj_t * obj, lv_obj_t * page);

/**
 * Set the how the header should behave and its position
 * 设置头部的行为方式及其位置
 * @param obj       pointer to a menu
 * @param obj       指向菜单
 * @param mode      LV_MENU_HEADER_TOP_FIXED/TOP_UNFIXED/BOTTOM_FIXED
 * @param mode      头部模式: LV_MENU_HEADER_TOP_FIXED/TOP_UNFIXED/BOTTOM_FIXED
 */
void lv_menu_set_mode_header(lv_obj_t * obj, lv_menu_mode_header_t mode);

/**
 * Set whether back button should appear at root
 * 设置在根级是否显示返回按钮
 * @param obj       pointer to a menu
 * @param obj       指向菜单
 * @param mode      LV_MENU_ROOT_BACK_BUTTON_DISABLED/ENABLED
 * @param mode      根级返回按钮模式: LV_MENU_ROOT_BACK_BUTTON_DISABLED/ENABLED
 */
void lv_menu_set_mode_root_back_button(lv_obj_t * obj, lv_menu_mode_root_back_button_t mode);

/**
 * Add menu to the menu item
 * 为菜单项添加菜单加载事件
 * @param menu      pointer to the menu
 * @param menu      指向菜单
 * @param obj       pointer to the obj
 * @param obj       指向对象
 * @param page      pointer to the page to load when obj is clicked
 * @param page      指向当 obj 被点击时要加载的页面
 */
void lv_menu_set_load_page_event(lv_obj_t * menu, lv_obj_t * obj, lv_obj_t * page);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/
/**
* Get a pointer to menu page that is currently displayed in main
* 获取主区域当前显示的菜单页面指针
* @param obj        pointer to the menu
* @param obj        指向菜单
* @return           pointer to current page
* @return           指向当前页面的指针
*/
lv_obj_t * lv_menu_get_cur_main_page(lv_obj_t * obj);

/**
* Get a pointer to menu page that is currently displayed in sidebar
* 获取侧边栏当前显示的菜单页面指针
* @param obj        pointer to the menu
* @param obj        指向菜单
* @return           pointer to current page
* @return           指向当前页面的指针
*/
lv_obj_t * lv_menu_get_cur_sidebar_page(lv_obj_t * obj);

/**
* Get a pointer to main header obj
* 获取主区域头部对象指针
* @param obj        pointer to the menu
* @param obj        指向菜单
* @return           pointer to main header obj
* @return           指向主区域头部对象的指针
*/
lv_obj_t * lv_menu_get_main_header(lv_obj_t * obj);

/**
* Get a pointer to main header back btn obj
* 获取主区域头部返回按钮对象指针
* @param obj        pointer to the menu
* @param obj        指向菜单
* @return           pointer to main header back btn obj
* @return           指向主区域头部返回按钮对象的指针
*/
lv_obj_t * lv_menu_get_main_header_back_button(lv_obj_t * obj);

/**
* Get a pointer to sidebar header obj
* 获取侧边栏头部对象指针
* @param obj        pointer to the menu
* @param obj        指向菜单
* @return           pointer to sidebar header obj
* @return           指向侧边栏头部对象的指针
*/
lv_obj_t * lv_menu_get_sidebar_header(lv_obj_t * obj);

/**
* Get a pointer to sidebar header obj
* 获取侧边栏头部返回按钮对象指针
* @param obj        pointer to the menu
* @param obj        指向菜单
* @return           pointer to sidebar header back btn obj
* @return           指向侧边栏头部返回按钮对象的指针
*/
lv_obj_t * lv_menu_get_sidebar_header_back_button(lv_obj_t * obj);

/**
 * Check if an obj is a root back btn
 * 检查一个对象是否为根级返回按钮
 * @param menu      pointer to the menu
 * @param menu      指向菜单
 * @param obj       pointer to the back button
 * @param obj       指向返回按钮
 * @return          true if it is a root back btn
 * @return          若为根级返回按钮则返回 true
 */
bool lv_menu_back_button_is_root(lv_obj_t * menu, lv_obj_t * obj);

/**
 * Get the header mode of the menu
 * 获取菜单的头部模式
 * @param obj       pointer to a menu
 * @param obj       指向菜单
 * @return          LV_MENU_HEADER_TOP_FIXED/TOP_UNFIXED/BOTTOM_FIXED
 * @return          头部模式: LV_MENU_HEADER_TOP_FIXED/TOP_UNFIXED/BOTTOM_FIXED
 */
lv_menu_mode_header_t lv_menu_get_mode_header(lv_obj_t * obj);

/**
 * Get the root back button mode of the menu
 * 获取菜单的根级返回按钮模式
 * @param obj       pointer to a menu
 * @param obj       指向菜单
 * @return          LV_MENU_ROOT_BACK_BUTTON_DISABLED/ENABLED
 * @return          根级返回按钮模式: LV_MENU_ROOT_BACK_BUTTON_DISABLED/ENABLED
 */
lv_menu_mode_root_back_button_t lv_menu_get_mode_root_back_button(lv_obj_t * obj);

/**
 * Clear menu history
 * 清空菜单历史记录
 * @param obj       pointer to the menu
 * @param obj       指向菜单
 */
void lv_menu_clear_history(lv_obj_t * obj);

/**********************
 *      MACROS
 *      宏
 **********************/

#endif /*LV_USE_MENU*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_MENU_H*/
