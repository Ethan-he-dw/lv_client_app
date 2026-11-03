/**
 * @file lv_gltf.h
 * glTF 3D 模型查看控件头文件：提供 glTF 模型加载、相机控制、渲染与背景设置等功能。
 *
 */

#ifndef LV_GLTF_H
#define LV_GLTF_H

/*********************
 *      INCLUDES
 * 头文件包含
 *********************/

#include "../config/lv_conf_internal.h"

#if LV_USE_GLTF

#include "../3d/lv_gltf_model.h"
#include "../3d/lv_3dmath.h"
#include "../lv_types.h"
#include "../core/lv_area.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 * 宏定义
 *********************/

/**********************
 *      TYPEDEFS
 * 类型定义
 **********************/

typedef enum {
    LV_GLTF_AA_MODE_OFF = 0, /** Anti aliasing off*/
                              /** 关闭抗锯齿*/
    LV_GLTF_AA_MODE_ON = 1,  /** Anti aliasing on*/
                              /** 开启抗锯齿*/
    LV_GLTF_AA_MODE_DYNAMIC = 2, /** Anti aliasing on only when frame has no movement*/
                                  /** 仅在画面无移动时开启抗锯齿*/
} lv_gltf_aa_mode_t;

typedef enum {
    LV_GLTF_BG_MODE_SOLID = 0, /** Solid background. Use `lv_obj_set_style_bg_color` to set the background color*/
                                /** 纯色背景。使用 `lv_obj_set_style_bg_color` 设置背景颜色*/
    LV_GLTF_BG_MODE_ENVIRONMENT = 1, /** Environment background*/
                                      /** 环境背景*/
} lv_gltf_bg_mode_t;


/**********************
 * GLOBAL PROTOTYPES
 * 全局函数声明
 **********************/

/**
 * Create a glTF object
 * 创建一个 glTF 对象
 * @param parent pointer to the parent object
 * @param parent 指向父对象的指针
 * @return pointer to the created glTF object
 * @return 指向创建好的 glTF 对象的指针
 */
lv_obj_t * lv_gltf_create(lv_obj_t * parent);

/**
 * Assign an environment to a glTF object for IBL rendering
 * 为 glTF 对象指定一个环境，用于 IBL(基于图像的光照)渲染
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param environment pointer to the environment to use
 * @param environment 指向要使用的环境的指针
 * @note The environment can be shared across multiple glTF objects
 * @note 该环境可以在多个 glTF 对象之间共享
 * @note If no environment is set before attempting to load a file,
 * @note 如果在尝试加载文件之前未设置环境，
 *       a default one will be created for you
 *       系统会自动为您创建一个默认环境
 */
void lv_gltf_set_environment(lv_obj_t * obj, lv_gltf_environment_t * environment);

/**
 * Load a glTF model from a file into the viewer
 * 从文件加载 glTF 模型到查看器中
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param path file path to the glTF model to load
 * @param path 要加载的 glTF 模型的文件路径
 * @return pointer to the loaded glTF model, or NULL on failure
 * @return 指向加载好的 glTF 模型的指针，失败时返回 NULL
 */
lv_gltf_model_t * lv_gltf_load_model_from_file(lv_obj_t * obj, const char * path);

/**
 * Load a glTF model from a byte array into the viewer
 * 从字节数组加载 glTF 模型到查看器中
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param bytes glTF raw data
 * @param bytes glTF 原始数据
 * @param len glTF raw data length in bytes
 * @param len glTF 原始数据的字节长度
 * @return pointer to the loaded glTF model, or NULL on failure
 * @return 指向加载好的 glTF 模型的指针，失败时返回 NULL
 */
lv_gltf_model_t * lv_gltf_load_model_from_bytes(lv_obj_t * obj, const uint8_t * bytes, size_t len);

/**
 * Add a glTF model to the viewer.
 * 向查看器中添加一个 glTF 模型。
 *
 * Contrary to `lv_gltf_load_model_from_file` and `lv_gltf_load_model_from_bytes`, the model
 * 与 `lv_gltf_load_model_from_file` 和 `lv_gltf_load_model_from_bytes` 不同，该模型
 * is owned by the caller of this function meaning that it's the caller's responsibility
 * 由本函数的调用方所有，即调用方有责任
 * to delete the model when it is no longer needed, that is, the model must outlive the viewer's lifetime.
 * 在不再需要时删除该模型，也就是说，模型的生命周期必须比查看器更长。
 *
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param model glTF model to add to the viewer
 * @param model 要添加到查看器中的 glTF 模型
 * @return LV_RESULT_OK if the model was added to the viewer or LV_RESULT_INVALID on failure
 * @return 模型添加成功返回 LV_RESULT_OK，失败返回 LV_RESULT_INVALID
 */
lv_result_t lv_gltf_add_model(lv_obj_t * obj, lv_gltf_model_t * model);

/**
 * Get the number of models loaded in the glTF viewer
 * 获取 glTF 查看器中已加载模型的数量
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return the total number of models in the viewer
 * @return 查看器中模型的总数
 */
size_t lv_gltf_get_model_count(lv_obj_t * obj);

/**
 * Get a specific model by its index
 * 通过索引获取指定模型
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param id index of the model to retrieve (0-based)
 * @param id 要获取的模型的索引(从 0 开始)
 * @return pointer to the model at the specified index, or NULL if index is invalid
 * @return 指向指定索引处模型的指针，如果索引无效则返回 NULL
 */
lv_gltf_model_t * lv_gltf_get_model_by_index(const lv_obj_t * obj, size_t id);

/**
 * Get the primary model from the glTF viewer
 * 从 glTF 查看器获取主模型
 * The primary model is the first model added to the viewer and can be used
 * 主模型是添加到查看器的第一个模型，可用于
 * for camera selection and other primary operations
 * 相机选择和其他主要操作
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return pointer to the primary model, or NULL if no models are loaded
 * @return 指向主模型的指针，如果没有加载任何模型则返回 NULL
 */
lv_gltf_model_t * lv_gltf_get_primary_model(const lv_obj_t * obj);

/**
 * Set the yaw (horizontal rotation) of the camera
 * 设置相机的偏航角(水平旋转)
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param yaw yaw angle in degrees
 * @param yaw 偏航角度(度)
 */
void lv_gltf_set_yaw(lv_obj_t * obj, float yaw);

/**
 * Get the yaw (horizontal rotation) of the camera
 * 获取相机的偏航角(水平旋转)
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return yaw angle in degrees
 * @return 偏航角度(度)
 */
float lv_gltf_get_yaw(const lv_obj_t * obj);

/**
 * Set the pitch (vertical rotation) of the camera
 * 设置相机的俯仰角(垂直旋转)
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param pitch pitch angle in degrees
 * @param pitch 俯仰角度(度)
 */
void lv_gltf_set_pitch(lv_obj_t * obj, float pitch);

/**
 * Get the pitch (vertical rotation) of the camera
 * 获取相机的俯仰角(垂直旋转)
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return pitch angle in degrees
 * @return 俯仰角度(度)
 */
float lv_gltf_get_pitch(const lv_obj_t * obj);

/**
 * Set the camera distance from the focal point
 * 设置相机到焦点的距离
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value distance value
 * @param value 距离值
 */
void lv_gltf_set_distance(lv_obj_t * obj, float value);

/**
 * Get the camera distance scale factor from the focal point
 * 获取相机到焦点的距离缩放因子
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return distance scaling factor value
 * @return 距离缩放因子值
 */
float lv_gltf_get_distance(const lv_obj_t * obj);

/**
 * Get the camera distance from the focal point in world units
 * 获取相机到焦点的距离(以世界单位表示)
 * @param obj pointer to a GLTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return world unit distance value
 * @return 世界单位距离值
 */
float lv_gltf_get_world_distance(const lv_obj_t * obj);

/**********************
 * Viewport Functions
 * 视口函数
 **********************/

/**
 * Set the field of view
 * 设置视野角度
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value vertical FOV in degrees. If zero, the view will be orthographic (non-perspective)
 * @param value 垂直视野角度(度)。如果为零，视图将为正交投影(非透视)
 */
void lv_gltf_set_fov(lv_obj_t * obj, float value);

/**
 * Get the field of view
 * 获取视野角度
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return vertical FOV in degrees
 * @return 垂直视野角度(度)
 */
float lv_gltf_get_fov(const lv_obj_t * obj);

/**********************
 * Focal Point Functions
 * 焦点函数
 **********************/

/**
 * Set the X coordinate of the camera focal point
 * 设置相机焦点的 X 坐标
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value X coordinate
 * @param value X 坐标
 */
void lv_gltf_set_focal_x(lv_obj_t * obj, float value);

/**
 * Get the X coordinate of the camera focal point
 * 获取相机焦点的 X 坐标
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return X coordinate
 * @return X 坐标
 */
float lv_gltf_get_focal_x(const lv_obj_t * obj);

/**
 * Set the Y coordinate of the camera focal point
 * 设置相机焦点的 Y 坐标
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value Y coordinate
 * @param value Y 坐标
 */
void lv_gltf_set_focal_y(lv_obj_t * obj, float value);

/**
 * Get the Y coordinate of the camera focal point
 * 获取相机焦点的 Y 坐标
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return Y coordinate
 * @return Y 坐标
 */
float lv_gltf_get_focal_y(const lv_obj_t * obj);

/**
 * Set the Z coordinate of the camera focal point
 * 设置相机焦点的 Z 坐标
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value Z coordinate
 * @param value Z 坐标
 */
void lv_gltf_set_focal_z(lv_obj_t * obj, float value);

/**
 * Get the Z coordinate of the camera focal point
 * 获取相机焦点的 Z 坐标
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return Z coordinate
 * @return Z 坐标
 */
float lv_gltf_get_focal_z(const lv_obj_t * obj);

/**
 * Set the focal coordinates to the center point of the model object
 * 将焦点坐标设置为模型对象的中心点
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param model a model attached to this viewer or NULL for the first model
 * @param model 挂载到该查看器上的模型，传入 NULL 表示使用第一个模型
 */
void lv_gltf_recenter(lv_obj_t * obj, lv_gltf_model_t * model);

/**********************
 * Scene Control Functions
 * 场景控制函数
 **********************/

/**
 * Set the active camera index
 * 设置当前激活的相机索引
 * The camera is selected from the first glTF model added to the viewer
 * 相机从添加到查看器的第一个 glTF 模型中选择
 *
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value camera index (0 for default camera, 1+ for scene camera index)
 * @param value 相机索引(0 为默认相机，1+ 为场景相机索引)
 * @note Values higher than the scene's camera count will be clamped to the maximum available camera index
 * @note 高于场景相机数量的值将被限制为可用的最大相机索引
 */
void lv_gltf_set_camera(lv_obj_t * obj, uint32_t value);

/**
 * Get the active camera index
 * 获取当前激活的相机索引
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return active camera index
 * @return 当前激活的相机索引
 */
uint32_t lv_gltf_get_camera(const lv_obj_t * obj);

/**
 * Get the number of cameras in the first glTF model added to the viewer
 * 获取添加到查看器的第一个 glTF 模型中的相机数量
 * This count represents the valid range for the camera index parameter
 * 该数量表示 lv_gltf_set_camera() 中相机索引参数的有效范围
 * used with lv_gltf_set_camera()
 *
 * To get the camera count of other models, call
 * 要获取其他模型的相机数量，请
 * lv_gltf_model_get_camera_count(model) directly with the specific model
 * 直接对指定模型调用 lv_gltf_model_get_camera_count(model)
 *
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return number of available cameras
 * @return 可用的相机数量
 */
uint32_t lv_gltf_get_camera_count(const lv_obj_t * obj);

/**
 * DEPRECATED. See `lv_gltf_model_set_animation_speed`
 * 已弃用。请参见 `lv_gltf_model_set_animation_speed`
 *
 * Set the animation speed ratio
 * 设置动画速度比例
 *
 * The actual ratio is the value parameter / LV_GLTF_ANIM_SPEED_NORMAL
 * 实际比例为 value 参数 / LV_GLTF_ANIM_SPEED_NORMAL
 * Values greater than LV_GLTF_ANIM_SPEED_NORMAL will speed-up the animation
 * 大于 LV_GLTF_ANIM_SPEED_NORMAL 的值会加快动画速度
 * Values less than LV_GLTF_ANIM_SPEED_NORMAL will slow down the animation
 * 小于 LV_GLTF_ANIM_SPEED_NORMAL 的值会减慢动画速度
 *
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value speed-up ratio of the animation
 * @param value 动画的加速比例
 */
void lv_gltf_set_animation_speed(lv_obj_t * obj, uint32_t value);

/**
 * DEPRECATED. See `lv_gltf_model_get_animation_speed`
 * 已弃用。请参见 `lv_gltf_model_get_animation_speed`
 *
 * Get the animation speed ratio
 * 获取动画速度比例
 *
 * The actual ratio is the return value / LV_GLTF_ANIM_SPEED_NORMAL
 * 实际比例为返回值 / LV_GLTF_ANIM_SPEED_NORMAL
 *
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 */
uint32_t lv_gltf_get_animation_speed(const lv_obj_t * obj);

/**********************
 * Visual Settings Functions
 * 可视化设置函数
 **********************/

/**
 * Set the background mode
 * 设置背景模式
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value background mode
 * @param value 背景模式
 */
void lv_gltf_set_background_mode(lv_obj_t * obj, lv_gltf_bg_mode_t value);

/**
 * Get the background mode
 * 获取背景模式
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return background mode
 * @return 背景模式
 */
lv_gltf_bg_mode_t lv_gltf_get_background_mode(const lv_obj_t * obj);

/**
 * Set the background blur amount
 * 设置背景模糊程度
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value blur amount between 0 and 100
 * @param value 模糊程度，取值范围 0 到 100
 */
void lv_gltf_set_background_blur(lv_obj_t * obj, uint32_t value);

/**
 * Get the background blur amount
 * 获取背景模糊程度
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return blur amount between 0 and 100
 * @return 模糊程度，取值范围 0 到 100
 */
uint32_t lv_gltf_get_background_blur(const lv_obj_t * obj);

/**
 * Set the environmental brightness/power
 * 设置环境亮度/强度
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value brightness multiplier
 * @param value 亮度倍数
 */
void lv_gltf_set_env_brightness(lv_obj_t * obj, uint32_t value);

/**
 * Get the environmental brightness/power
 * 获取环境亮度/强度
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return brightness multiplier
 * @return 亮度倍数
 */
uint32_t lv_gltf_get_env_brightness(const lv_obj_t * obj);

/**
 * Set the image exposure level
 * 设置图像曝光级别
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value exposure level (1.0 is default)
 * @param value 曝光级别(默认值为 1.0)
 */
void lv_gltf_set_image_exposure(lv_obj_t * obj, float value);

/**
 * Get the image exposure level
 * 获取图像曝光级别
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return exposure level
 * @return 曝光级别
 */
float lv_gltf_get_image_exposure(const lv_obj_t * obj);

/**********************
 * Rendering Functions
 * 渲染函数
 **********************/

/**
 * Set the anti-aliasing mode
 * 设置抗锯齿模式
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param value anti-aliasing mode
 * @param value 抗锯齿模式
 */
void lv_gltf_set_antialiasing_mode(lv_obj_t * obj, lv_gltf_aa_mode_t value);

/**
 * Get the anti-aliasing mode
 * 获取抗锯齿模式
 * @param obj pointer to a glTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @return anti-aliasing mode
 * @return 抗锯齿模式
 */
lv_gltf_aa_mode_t lv_gltf_get_antialiasing_mode(const lv_obj_t * obj);

/***********************
 * Raycasting Functions
 * 射线投射函数
 ***********************/

/**
 * Get the point that a given ray intersects with a specified plane at, if any
 * 获取给定射线与指定平面相交的点(如果存在)
 * @param ray the intersection test ray
 * @param ray 用于相交测试的射线
 * @param screen_y the plane to test ray intersection with
 * @param screen_y 用于与射线进行相交测试的平面
 * @param collision_point output lv_3dpoint_t holder, values are only valid if true is the return value
 * @param collision_point 输出 lv_3dpoint_t 的存放容器，仅当返回值为 true 时其中的数值才有效
 * @return LV_RESULT_OK if intersection, LV_RESULT_INVALID if no intersection
 * @return 有交点返回 LV_RESULT_OK，无交点返回 LV_RESULT_INVALID
 */
lv_result_t lv_intersect_ray_with_plane(const lv_3dray_t * ray, const lv_3dplane_t * plane,
                                        lv_3dpoint_t * collision_point);

/**
 * Get a plane that faces the current view camera, centered some units in front of it
 * 获取一个面向当前观察相机、并位于其前方若干单位的平面
 * @param obj pointer to a GLTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param distance distance in front of the camera to set the plane, in world units. see lv_gltf_get_world_distance to get the auto-distance
 * @param distance 平面距相机前方的距离，以世界单位表示。参见 lv_gltf_get_world_distance 获取自动距离
 * @return camera facing plane
 * @return 面向相机的平面
 */
lv_3dplane_t lv_gltf_get_current_view_plane(lv_obj_t * obj, float distance);

/**
 * Calculates a ray originating from the camera and passing through the specified mouse position on the screen.
 * 计算一条从相机发出并穿过屏幕上指定鼠标位置的射线。
 * @param obj pointer to a GLTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param screen_pos screen coordinate, in pixels
 * @param screen_pos 屏幕坐标，单位为像素
 * @return mouse point ray
 * @return 经过鼠标点的射线
 */
lv_3dray_t lv_gltf_get_ray_from_2d_coordinate(lv_obj_t * obj, const lv_point_t * screen_pos);


/**
 * Get the screen position of a 3d point
 * 获取一个 3D 点的屏幕位置
 * @param obj pointer to a GLTF viewer object
 * @param obj 指向 glTF 查看器对象的指针
 * @param world_pos world position to convert
 * @param world_pos 要转换的世界坐标位置
 * @param lv_point_t the resulting point, in pixels. only valid if return value is true
 * @param lv_point_t 计算结果点，单位为像素。仅当返回值为 true 时有效
 * @return LV_RESULT_OK if conversion valid, LV_RESULT_INVALID if no valid conversion
 * @return 转换有效返回 LV_RESULT_OK，无法进行有效转换则返回 LV_RESULT_INVALID
 */
lv_result_t lv_gltf_world_to_screen(lv_obj_t * obj, const lv_3dpoint_t world_pos, lv_point_t * screen_pos);


/**********************
 *      MACROS
 * 宏
 **********************/

#ifdef __cplusplus
}
#endif

#endif /*LV_USE_GLTF*/

#endif /*LV_GLTF_H*/
