/**
 * @file ivs_task_face.h
 * @brief face 任务的转换入口（仅 C++ 适配器使用）
 *
 * 每个 NPU 任务一个这样的文件：把该任务的 IDL 结构体翻译成统一的 ivs_result_t。
 * 新增任务时新增同构文件，不改适配器与 UI。
 */

#ifndef IVS_TASK_FACE_H
#define IVS_TASK_FACE_H

#include <omnibinder/buffer.h>

#include "ivs_types.h"

/** @return true 表示反序列化成功且已填充 out */
bool IvsConvertFace(const omnibinder::Buffer &data, ivs_result_t *out);

#endif /* IVS_TASK_FACE_H */
