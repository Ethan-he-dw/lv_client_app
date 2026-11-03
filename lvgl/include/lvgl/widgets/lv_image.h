/**
 * @file lv_image.h
 * 图片(Image)控件：用于显示图片，支持旋转、缩放、对齐、平铺等变换
 *
 */

#ifndef LV_IMAGE_H
#define LV_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *      头文件包含
 *********************/
#include "../config/lv_conf_internal.h"

#if LV_USE_IMAGE != 0

/*Testing of dependencies*/
/*依赖关系检查*/
#if LV_USE_LABEL == 0
#error "lv_img: lv_label is required. Enable it in lv_conf.h (LV_USE_LABEL 1)"
#endif

#include "../core/lv_obj.h"
#include "../fs/lv_fs.h"
#include "../draw/lv_draw.h"
#include "../core/lv_observer.h"

/*********************
 *      DEFINES
 *      宏定义
 *********************/

/**********************
 *      TYPEDEFS
 *      类型定义
 **********************/

/*图片控件的类对象*/
LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_image_class;

/**
 * Image size mode, when image size and object size is different
 * 图片尺寸模式：当图片尺寸与对象尺寸不同时使用
 */
typedef enum {
    /*默认模式：图片按原始尺寸显示在对象左上角*/
    LV_IMAGE_ALIGN_DEFAULT = 0,
    /*对齐到顶部左侧*/
    LV_IMAGE_ALIGN_TOP_LEFT,
    /*对齐到顶部中间*/
    LV_IMAGE_ALIGN_TOP_MID,
    /*对齐到顶部右侧*/
    LV_IMAGE_ALIGN_TOP_RIGHT,
    /*对齐到底部左侧*/
    LV_IMAGE_ALIGN_BOTTOM_LEFT,
    /*对齐到底部中间*/
    LV_IMAGE_ALIGN_BOTTOM_MID,
    /*对齐到底部右侧*/
    LV_IMAGE_ALIGN_BOTTOM_RIGHT,
    /*左侧垂直居中*/
    LV_IMAGE_ALIGN_LEFT_MID,
    /*右侧垂直居中*/
    LV_IMAGE_ALIGN_RIGHT_MID,
    /*居中对齐*/
    LV_IMAGE_ALIGN_CENTER,
    /*标记对图片进行变换的模式起始位置*/
    _LV_IMAGE_ALIGN_AUTO_TRANSFORM, /**< Marks the start of modes that transform the image*/
    /*拉伸模式：设置X和Y缩放比例以填满控件区域*/
    LV_IMAGE_ALIGN_STRETCH, /**< Set X and Y scale to fill the Widget's area. */
    /*平铺模式：平铺图片以填满控件区域，偏移量用于移动平铺位置*/
    LV_IMAGE_ALIGN_TILE,    /**< Tile image to fill Widget's area. Offset is applied to shift the tiling. */
    /*包含模式：保持宽高比，缩放到适合控件区域内的最大尺寸*/
    LV_IMAGE_ALIGN_CONTAIN, /**< The image keeps its aspect ratio, but is resized to the maximum size that fits within the Widget's area. */
    /*包含缩小模式：保持宽高比，仅当图片大于控件时缩放到最大适合尺寸*/
    LV_IMAGE_ALIGN_CONTAIN_DOWNSCALE, /**< The image keeps its aspect ratio, but is resized to the maximum size that fits within the Widget's area if the image is bigger. */
    /*覆盖模式：保持宽高比并填满控件区域*/
    LV_IMAGE_ALIGN_COVER,   /**< The image keeps its aspect ratio and fills the Widget's area. */
} lv_image_align_t;

#if LV_USE_OBJ_PROPERTY
/*图片对象属性ID枚举*/
enum _lv_property_image_id_t {
    /*图片源：图片描述符、文件路径或符号*/
    LV_PROPERTY_ID(IMAGE, SRC,          LV_PROPERTY_TYPE_IMGSRC,    0),
    /*X轴偏移量*/
    LV_PROPERTY_ID(IMAGE, OFFSET_X,     LV_PROPERTY_TYPE_INT,       1),
    /*Y轴偏移量*/
    LV_PROPERTY_ID(IMAGE, OFFSET_Y,     LV_PROPERTY_TYPE_INT,       2),
    /*旋转角度*/
    LV_PROPERTY_ID(IMAGE, ROTATION,     LV_PROPERTY_TYPE_INT,       3),
    /*旋转中心点*/
    LV_PROPERTY_ID(IMAGE, PIVOT,        LV_PROPERTY_TYPE_POINT,     4),
    /*缩放系数*/
    LV_PROPERTY_ID(IMAGE, SCALE,        LV_PROPERTY_TYPE_INT,       5),
    /*水平缩放系数*/
    LV_PROPERTY_ID(IMAGE, SCALE_X,      LV_PROPERTY_TYPE_INT,       6),
    /*垂直缩放系数*/
    LV_PROPERTY_ID(IMAGE, SCALE_Y,      LV_PROPERTY_TYPE_INT,       7),
    /*混合模式*/
    LV_PROPERTY_ID(IMAGE, BLEND_MODE,   LV_PROPERTY_TYPE_INT,       8),
    /*抗锯齿*/
    LV_PROPERTY_ID(IMAGE, ANTIALIAS,    LV_PROPERTY_TYPE_INT,       9),
    /*内部对齐模式*/
    LV_PROPERTY_ID(IMAGE, INNER_ALIGN,  LV_PROPERTY_TYPE_INT,       10),
    LV_PROPERTY_IMAGE_END,
};
#endif

/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create an image object
 * 创建图片对象
 * @param parent pointer to an object, it will be the parent of the new image
 * @param parent 指向父对象的指针，它将成为新图片的父对象
 * @return pointer to the created image
 * @return 指向已创建图片对象的指针
 */
lv_obj_t * lv_image_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 * 设置函数
 *====================*/

/**
 * Set the image data to display on the object
 * 设置要显示在对象上的图片数据
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param src       1) pointer to an ::lv_image_dsc_t descriptor (converted by LVGL's image converter) (e.g. &my_img) or
 * @param src       1) 指向 ::lv_image_dsc_t 描述符的指针（由LVGL图片转换器转换生成）(例如 &my_img)，或
 *                  2) path to an image file (e.g. "S:/dir/img.bin")or
 *                  2) 图片文件的路径（例如 "S:/dir/img.bin"），或
 *                  3) a SYMBOL (e.g. LV_SYMBOL_OK)
 *                  3) 一个符号（例如 LV_SYMBOL_OK）
 */
void lv_image_set_src(lv_obj_t * obj, const void * src);

/**
 * Set an offset for the source of an image so the image will be displayed from the new origin.
 * 设置图片源的偏移量，使图片从新的原点开始显示
 * @param obj       pointer to an image
 * @param obj       指向图片对象的指针
 * @param x         the new offset along x axis.
 * @param x         X轴方向的新偏移量
 */
void lv_image_set_offset_x(lv_obj_t * obj, int32_t x);

/**
 * Set an offset for the source of an image.
 * 设置图片源的偏移量
 * so the image will be displayed from the new origin.
 * 使图片从新的原点开始显示
 * @param obj       pointer to an image
 * @param obj       指向图片对象的指针
 * @param y         the new offset along y axis.
 * @param y         Y轴方向的新偏移量
 */
void lv_image_set_offset_y(lv_obj_t * obj, int32_t y);

/**
 * Set the rotation angle of the image.
 * 设置图片的旋转角度
 * The image will be rotated around the set pivot set by `lv_image_set_pivot()`
 * 图片将围绕 `lv_image_set_pivot()` 设置的旋转中心进行旋转
 * Note that indexed and alpha only images can't be transformed.
 * 注意：索引色图片和仅含Alpha通道的图片无法进行变换
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param angle     rotation in degree with 0.1 degree resolution (0..3600: clock wise)
 * @param angle     旋转角度，分辨率为0.1度（0..3600：顺时针）
 * @note            if image_align is `LV_IMAGE_ALIGN_STRETCH` or `LV_IMAGE_ALIGN_FIT`
 * @note            如果 image_align 为 `LV_IMAGE_ALIGN_STRETCH` 或 `LV_IMAGE_ALIGN_FIT`
 *                  rotation will be set to 0 automatically.
 *                  旋转角度将自动被设置为0
 *
 */
void lv_image_set_rotation(lv_obj_t * obj, int32_t angle);

/**
 * Set the rotation center of the image.
 * 设置图片的旋转中心
 * The image will be rotated around this point.
 * 图片将围绕该点进行旋转
 * x, y can be set with value of LV_PCT, lv_image_get_pivot will return the true pixel coordinate of pivot in this case.
 * x、y 可以设置为 LV_PCT 百分比值，此时 lv_image_get_pivot 将返回旋转中心的真实像素坐标
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param x         rotation center x of the image
 * @param x         图片旋转中心的X坐标
 * @param y         rotation center y of the image
 * @param y         图片旋转中心的Y坐标
 */
void lv_image_set_pivot(lv_obj_t * obj, int32_t x, int32_t y);

/**
 * Set the rotation horizontal center of the image.
 * 设置图片旋转中心的水平坐标
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param x         rotation center x of the image, or lv_pct()
 * @param x         图片旋转中心的X坐标，或 lv_pct() 百分比值
 */
void lv_image_set_pivot_x(lv_obj_t * obj, int32_t x);

/**
 * Set the rotation vertical center of the image.
 * 设置图片旋转中心的垂直坐标
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param y         rotation center y of the image, or lv_pct()
 * @param y         图片旋转中心的Y坐标，或 lv_pct() 百分比值
 */
void lv_image_set_pivot_y(lv_obj_t * obj, int32_t y);

/**
 * Set the zoom factor of the image.
 * 设置图片的缩放系数
 * Note that indexed and alpha only images can't be transformed.
 * 注意：索引色图片和仅含Alpha通道的图片无法进行变换
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param zoom      the zoom factor.  Example values:
 * @param zoom      缩放系数，示例值：
 *                      - 256 or LV_SCALE_NONE:  no zoom
 *                      - 256 或 LV_SCALE_NONE：不缩放
 *                      - <256:  scale down
 *                      - <256：缩小
 *                      - >256:  scale up
 *                      - >256：放大
 *                      - 128:  half size
 *                      - 128：一半大小
 *                      - 512:  double size
 *                      - 512：两倍大小
 */
void lv_image_set_scale(lv_obj_t * obj, uint32_t zoom);

/**
 * Set the horizontal zoom factor of the image.
 * 设置图片的水平缩放系数
 * Note that indexed and alpha only images can't be transformed.
 * 注意：索引色图片和仅含Alpha通道的图片无法进行变换
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param zoom      the zoom factor.  Example values:
 * @param zoom      缩放系数，示例值：
 *                      - 256 or LV_SCALE_NONE:  no zoom
 *                      - 256 或 LV_SCALE_NONE：不缩放
 *                      - <256:  scale down
 *                      - <256：缩小
 *                      - >256:  scale up
 *                      - >256：放大
 *                      - 128:  half size
 *                      - 128：一半大小
 *                      - 512:  double size
 *                      - 512：两倍大小
 */
void lv_image_set_scale_x(lv_obj_t * obj, uint32_t zoom);

/**
 * Set the vertical zoom factor of the image.
 * 设置图片的垂直缩放系数
 * Note that indexed and alpha only images can't be transformed.
 * 注意：索引色图片和仅含Alpha通道的图片无法进行变换
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param zoom      the zoom factor.  Example values:
 * @param zoom      缩放系数，示例值：
 *                      - 256 or LV_SCALE_NONE:  no zoom
 *                      - 256 或 LV_SCALE_NONE：不缩放
 *                      - <256:  scale down
 *                      - <256：缩小
 *                      - >256:  scale up
 *                      - >256：放大
 *                      - 128:  half size
 *                      - 128：一半大小
 *                      - 512:  double size
 *                      - 512：两倍大小
 */
void lv_image_set_scale_y(lv_obj_t * obj, uint32_t zoom);

/**
 * Set the blend mode of an image.
 * 设置图片的混合模式
 * @param obj           pointer to an image object
 * @param obj           指向图片对象的指针
 * @param blend_mode    the new blend mode
 * @param blend_mode    新的混合模式
 */
void lv_image_set_blend_mode(lv_obj_t * obj, lv_blend_mode_t blend_mode);

/**
 * Enable/disable anti-aliasing for the transformations (rotate, zoom) or not.
 * 启用/禁用图片变换（旋转、缩放）时的抗锯齿功能
 * The quality is better with anti-aliasing looks better but slower.
 * 开启抗锯齿后显示效果更好，但性能会降低
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param antialias true: anti-aliased; false: not anti-aliased
 * @param antialias true：启用抗锯齿；false：不启用
 */
void lv_image_set_antialias(lv_obj_t * obj, bool antialias);

/**
 * Set the image object size mode.
 * 设置图片对象的大小模式
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param align     the new align mode.
 * @param align     新的对齐模式
 * @note            if image_align is `LV_IMAGE_ALIGN_STRETCH` or `LV_IMAGE_ALIGN_FIT`
 * @note            如果 image_align 为 `LV_IMAGE_ALIGN_STRETCH` 或 `LV_IMAGE_ALIGN_FIT`
 *                  rotation, scale and pivot will be overwritten and controlled internally.
 *                  旋转、缩放和旋转中心将被覆盖并由系统内部控制
 */
void lv_image_set_inner_align(lv_obj_t * obj, lv_image_align_t align);

/**
 * Set an A8 bitmap mask for the image.
 * 为图片设置一个A8格式的位图蒙版
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param src       an lv_image_dsc_t bitmap mask source.
 * @param src       一个 lv_image_dsc_t 类型的位图蒙版源
 */
void lv_image_set_bitmap_map_src(lv_obj_t * obj, const lv_image_dsc_t * src);

/*=====================
 * Getter functions
 * 获取函数
 *====================*/

/**
 * Get the source of the image
 * 获取图片的源
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @return          the image source (symbol, file name or ::lv-img_dsc_t for C arrays)
 * @return          图片源（符号、文件名或C数组的 ::lv_image_dsc_t 描述符）
 */
const void * lv_image_get_src(lv_obj_t * obj);

/**
 * Get the offset's x attribute of the image object.
 * 获取图片对象的X轴偏移属性
 * @param obj       pointer to an image
 * @param obj       指向图片对象的指针
 * @return          offset X value.
 * @return          X轴偏移值
 */
int32_t lv_image_get_offset_x(lv_obj_t * obj);

/**
 * Get the offset's y attribute of the image object.
 * 获取图片对象的Y轴偏移属性
 * @param obj       pointer to an image
 * @param obj       指向图片对象的指针
 * @return          offset Y value.
 * @return          Y轴偏移值
 */
int32_t lv_image_get_offset_y(lv_obj_t * obj);

/**
 * Get the rotation of the image.
 * 获取图片的旋转角度
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @return          rotation in 0.1 degrees (0..3600)
 * @return          旋转角度，单位0.1度（0..3600）
 * @note            if image_align is `LV_IMAGE_ALIGN_STRETCH` or  `LV_IMAGE_ALIGN_FIT`
 * @note            如果 image_align 为 `LV_IMAGE_ALIGN_STRETCH` 或 `LV_IMAGE_ALIGN_FIT`
 *                  rotation will be set to 0 automatically.
 *                  旋转角度将自动被设置为0
 */
int32_t lv_image_get_rotation(lv_obj_t * obj);

/**
 * Get the pivot (rotation center) of the image.
 * 获取图片的旋转中心
 * If pivot is set with LV_PCT, convert it to px before return.
 * 如果旋转中心是用 LV_PCT 设置的，返回前会将其转换为像素值
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @param pivot     store the rotation center here
 * @param pivot     在此处存储旋转中心
 */
void lv_image_get_pivot(lv_obj_t * obj, lv_point_t * pivot);

/**
 * Get the zoom factor of the image.
 * 获取图片的缩放系数
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @return          zoom factor (256: no zoom)
 * @return          缩放系数（256：不缩放）
 */
int32_t lv_image_get_scale(lv_obj_t * obj);

/**
 * Get the horizontal zoom factor of the image.
 * 获取图片的水平缩放系数
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @return          zoom factor (256: no zoom)
 * @return          缩放系数（256：不缩放）
 */
int32_t lv_image_get_scale_x(lv_obj_t * obj);

/**
 * Get the vertical zoom factor of the image.
 * 获取图片的垂直缩放系数
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @return          zoom factor (256: no zoom)
 * @return          缩放系数（256：不缩放）
 */
int32_t lv_image_get_scale_y(lv_obj_t * obj);

/**
 * Get the width of an image before any transformations.
 * 获取图片在任何变换之前的宽度
 * @param obj Pointer to an image object.
 * @param obj 指向图片对象的指针
 * @return The width of the image.
 * @return 图片的宽度
 */
int32_t lv_image_get_src_width(lv_obj_t * obj);

/**
 * Get the height of an image before any transformations.
 * 获取图片在任何变换之前的高度
 * @param obj Pointer to an image object.
 * @param obj 指向图片对象的指针
 * @return The height of the image.
 * @return 图片的高度
 */
int32_t lv_image_get_src_height(lv_obj_t * obj);

/**
 * Get the transformed width of an image object.
 * 获取图片对象变换后的宽度
 * @param obj Pointer to an image object.
 * @param obj 指向图片对象的指针
 * @return The transformed width of the image.
 * @return 图片变换后的宽度
 */
int32_t lv_image_get_transformed_width(lv_obj_t * obj);

/**
 * Get the transformed height of an image object.
 * 获取图片对象变换后的高度
 * @param obj Pointer to an image object.
 * @param obj 指向图片对象的指针
 * @return The transformed height of the image.
 * @return 图片变换后的高度
 */
int32_t lv_image_get_transformed_height(lv_obj_t * obj);

/**
 * Get the current blend mode of the image
 * 获取图片当前的混合模式
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @return          the current blend mode
 * @return          当前的混合模式
 */
lv_blend_mode_t lv_image_get_blend_mode(lv_obj_t * obj);

/**
 * Get whether the transformations (rotate, zoom) are anti-aliased or not
 * 获取图片变换（旋转、缩放）是否启用抗锯齿
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @return          true: anti-aliased; false: not anti-aliased
 * @return          true：启用抗锯齿；false：未启用
 */
bool lv_image_get_antialias(lv_obj_t * obj);

/**
 * Get the size mode of the image
 * 获取图片的大小模式
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @return          element of `lv_image_align_t`
 * @return          `lv_image_align_t` 类型中的元素
 */
lv_image_align_t lv_image_get_inner_align(lv_obj_t * obj);

/**
 * Get the bitmap mask source.
 * 获取位图蒙版源
 * @param obj       pointer to an image object
 * @param obj       指向图片对象的指针
 * @return          an lv_image_dsc_t bitmap mask source.
 * @return          一个 lv_image_dsc_t 类型的位图蒙版源
 */
const lv_image_dsc_t * lv_image_get_bitmap_map_src(lv_obj_t * obj);


#if LV_USE_OBSERVER
/**
 * Bind a pointer Subject to an Image's source.
 * 将一个指针类型的Subject(可观察对象)绑定到图片的源
 * @param obj       pointer to Image
 * @param obj       指向图片对象的指针
 * @param subject   pointer to Subject
 * @param subject   指向Subject(可观察对象)的指针
 * @return          pointer to newly-created Observer
 * @return          指向新创建的Observer的指针
 */
lv_observer_t * lv_image_bind_src(lv_obj_t * obj, lv_subject_t * subject);
#endif

/**********************
 *      MACROS
 *      宏
 **********************/

/** Use this macro to declare an image in a C file*/
/** 使用此宏在C文件中声明一张图片*/
#define LV_IMAGE_DECLARE(var_name) extern const lv_image_dsc_t var_name

#endif /*LV_USE_IMAGE*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_IMAGE_H*/
