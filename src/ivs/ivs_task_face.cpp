/**
 * @file ivs_task_face.cpp
 * @brief mediaipc::FaceResult → ivs_result_t
 *
 * 唯一一处"认识人脸字段"的代码。业务字段（cos/thr/perf/alts）统一塞进 attrs，
 * 显示层按 key:val 展示，因此加新任务不需要动面板与叠加层。
 */

#include "ivs_task_face.h"

#include "face_service.bidl.h"

#include <cstdio>
#include <cstring>

namespace {

/** 备选人员拼成一串：`P007 0.71|P012 0.68`（面板直接显示，无需二次解析） */
void FormatAlts(const mediaipc::FaceItem &best, char *out, size_t out_sz)
{
    out[0] = '\0';
    size_t used = 0;

    const size_t n = best.alts.size();
    for (size_t i = 0; i < n && i < 4; i++) {
        const mediaipc::FaceCandidate &c = best.alts[i];
        const int w = snprintf(out + used, out_sz - used, "%s%s %.2f",
                               (i == 0) ? "" : "|", c.name.c_str(), (double)c.cosine);
        if (w < 0 || (size_t)w >= out_sz - used) break;
        used += (size_t)w;
    }

    if (out[0] == '\0') snprintf(out, out_sz, "-");
}

}  // namespace

bool IvsConvertFace(const omnibinder::Buffer &data, ivs_result_t *out)
{
    if (out == NULL) return false;

    mediaipc::FaceResult msg;
    omnibinder::Buffer buf(data.data(), data.size());
    if (!msg.deserialize(buf)) return false;

    memset(out, 0, sizeof(*out));
    out->task = IVS_TASK_FACE;
    ivs_strcpy(out->task_name, sizeof(out->task_name), "face");
    out->seq   = msg.seq;
    out->ts_us = msg.ts_us;

    /* 目标数：det_count / dets.size() / 上限 三者取最小 */
    uint32_t n = msg.det_count;
    if (n > msg.dets.size()) n = (uint32_t)msg.dets.size();
    if (n > IVS_MAX_OBJS)    n = IVS_MAX_OBJS;

    for (uint32_t i = 0; i < n; i++) {
        const mediaipc::FaceDet &d = msg.dets[i];
        ivs_object_t *o = &out->objs[i];

        o->score = d.score;
        o->x1 = d.x1;
        o->y1 = d.y1;
        o->x2 = d.x2;
        o->y2 = d.y2;
        o->is_best = (msg.best_index == (int32_t)i) ? 1 : 0;

        if (d.kps.size() >= IVS_MAX_KPS) {
            for (int k = 0; k < IVS_MAX_KPS; k++) o->kps[k] = d.kps[k];
            o->kps_n = IVS_MAX_KPS;
        }

        /* 只有 best 带姓名，且仅命中时填；未命中留给 OSD 显示 unknown */
        if (o->is_best && msg.best.matched && !msg.best.name.empty()) {
            ivs_strcpy(o->label, sizeof(o->label), msg.best.name.c_str());
        }
    }
    out->obj_count = (uint16_t)n;

    /* 任务属性（显示层不认这些名字，只按 key: val 排版） */
    ivs_attr_setf(out, "fps",       "%.1f", (double)msg.perf.fps_link);
    ivs_attr_setf(out, "det",       "%.1fms", (double)msg.perf.exec_det_ms);
    ivs_attr_setf(out, "mbf",       "%.1fms", (double)msg.perf.exec_mbf_ms);
    ivs_attr_setf(out, "dec",       "%.1fms", (double)msg.perf.decode_ms);
    ivs_attr_setf(out, "thr_score", "%.2f", (double)msg.score_thr);
    ivs_attr_setf(out, "thr_match", "%.2f", (double)msg.match_thr);
    ivs_attr_setf(out, "cos",       "%.3f", (double)msg.best.cosine);
    ivs_attr_setf(out, "matched",   "%d",   msg.best.matched ? 1 : 0);

    char alts[IVS_ATTR_VAL_MAX];
    FormatAlts(msg.best, alts, sizeof(alts));
    ivs_attr_setf(out, "alts", "%s", alts);

    return true;
}
