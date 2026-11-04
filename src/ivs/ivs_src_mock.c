/**
 * @file ivs_src_mock.c
 * @brief 假数据源：无板状态下把"订阅→模型→显示"整条链路跑通
 *
 * 用途：UI/面板/OSD 的验收全部可以在没有板子时完成。数据形状刻意向真实
 * FaceResult 对齐（1~3 个目标、5 点 kps、键值属性、心跳帧），并周期性注入
 * 边界用例（17 个目标、kps 缺失），用来验证截断与降级。
 *
 * 注意：本文件禁止包含任何 lvgl 头（契约③）。
 */

#include "ivs_source.h"
#include "ivs_store.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#endif

#define MOCK_FPS_PERIOD_MS   40
#define MOCK_HEARTBEAT_MS    5000   /* 每 5s 一帧 obj_count=0 */
#define MOCK_BURST_PERIOD_MS 60000  /* 每 60s 注入一次边界用例 */
#define MOCK_BURST_DUR_MS    1000
#define MOCK_NODATA_MS       3000   /* 与实现约定一致：3s 无帧 → NODATA */

static volatile int s_run = 0;
static ivs_state_t  s_state = IVS_IDLE;
static uint64_t     s_last_rx = 0;
static char         s_err[128] = "mock: 无外部依赖";
static char         s_task[IVS_TASK_NAME_MAX] = "face";
static uint64_t     s_t0 = 0;

#ifdef _WIN32
static HANDLE s_th = NULL;
static DWORD WINAPI mock_thread(LPVOID p);
#else
static pthread_t s_th;
static void *mock_thread(void *p);
#endif

/* ---------------- 造数 ---------------- */

static void fill_object(ivs_object_t *o, int idx, float cx, float cy, uint32_t age_ms, bool with_kps)
{
    const float w = 110.0f, h = 140.0f;

    memset(o, 0, sizeof(*o));
    o->score = 0.90f + 0.08f * (float)((idx * 37) % 10) / 10.0f;
    o->x1 = cx - w * 0.5f;
    o->y1 = cy - h * 0.5f;
    o->x2 = cx + w * 0.5f;
    o->y2 = cy + h * 0.5f;
    o->is_best = (idx == 0) ? 1 : 0;

    if (with_kps) {
        /* 归一化位置（相对框）：左眼/右眼/鼻尖/左嘴角/右嘴角 + 轻微抖动 */
        static const float kx[5] = { 0.32f, 0.68f, 0.50f, 0.36f, 0.64f };
        static const float ky[5] = { 0.36f, 0.36f, 0.50f, 0.72f, 0.72f };
        const float jit = (float)((age_ms / MOCK_FPS_PERIOD_MS + (uint32_t)idx) % 5) - 2.0f;
        for (int k = 0; k < 5; k++) {
            o->kps[k * 2 + 0] = o->x1 + (o->x2 - o->x1) * kx[k] + jit;
            o->kps[k * 2 + 1] = o->y1 + (o->y2 - o->y1) * ky[k];
        }
        o->kps_n = IVS_MAX_KPS;
    }

    /* 第 2、3 个目标故意"未匹配"，验证 unknown 标签 */
    if (idx == 0) {
        ivs_strcpy(o->label, sizeof(o->label), "P001");
    } else if (idx == 1) {
        ivs_strcpy(o->label, sizeof(o->label), "");        /* 未匹配 */
    } else {
        ivs_strcpy(o->label, sizeof(o->label), "P003");
    }
}

static void fill_attrs(ivs_result_t *r, int matched)
{
    ivs_attr_setf(r, "fps",       "%.1f",   24.8f);
    ivs_attr_setf(r, "det",       "%.1fms", 18.2f);
    ivs_attr_setf(r, "mbf",       "%.1fms", 6.1f);
    ivs_attr_setf(r, "dec",       "%.1fms", 4.3f);
    ivs_attr_setf(r, "thr_score", "%.2f",   0.50f);
    ivs_attr_setf(r, "thr_match", "%.2f",   0.55f);
    ivs_attr_setf(r, "cos",       "%.3f",   matched ? 0.732f : 0.418f);
    ivs_attr_setf(r, "matched",   "%d",     matched ? 1 : 0);
    ivs_attr_setf(r, "alts",      "%s",     matched ? "P007 0.71|P012 0.68" : "-");
}

static void mock_emit(uint32_t age_ms, uint64_t seq)
{
    ivs_result_t r;
    memset(&r, 0, sizeof(r));

    r.task = (strcmp(s_task, "person") == 0) ? IVS_TASK_PERSON : IVS_TASK_FACE;
    ivs_strcpy(r.task_name, sizeof(r.task_name), s_task);
    r.seq   = seq;
    r.ts_us = (uint64_t)age_ms * 1000ull;   /* 假时间轴（真实实现用板端 ts_us） */

    const uint32_t phase = age_ms % MOCK_BURST_PERIOD_MS;
    const bool heartbeat = (age_ms % MOCK_HEARTBEAT_MS) < MOCK_FPS_PERIOD_MS;
    bool force_many = (phase >= 10000 && phase < 10000 + MOCK_BURST_DUR_MS);   /* 17 个目标 */
    bool drop_kps   = (phase >= 12000 && phase < 12000 + MOCK_BURST_DUR_MS);   /* kps 缺失 */

    if (heartbeat) {
        /* 轻量帧：无人脸也发布（对接文档 §3.3），用来证明"链路活着" */
        r.obj_count = 0;
        fill_attrs(&r, 0);
    } else if (force_many) {
        r.obj_count = 17;                    /* 故意超过上限，验证截断 */
        for (int i = 0; i < 17; i++) {
            ivs_object_t o;
            fill_object(&o, i, 60.0f + (float)(i % 6) * 95.0f, 90.0f + (float)(i / 6) * 130.0f, age_ms, !drop_kps);
            if (i < IVS_MAX_OBJS) r.objs[i] = o;
        }
        fill_attrs(&r, 1);
    } else {
        const int n = 3;
        r.obj_count = (uint16_t)n;
        for (int i = 0; i < n; i++) {
            const float cx = 320.0f + 150.0f * sinf((float)age_ms / 900.0f + (float)i * 2.1f);
            const float cy = 260.0f +  50.0f * sinf((float)age_ms / 700.0f + (float)i * 1.7f);
            fill_object(&r.objs[i], i, cx, cy, age_ms, !drop_kps);
        }
        fill_attrs(&r, 1);
    }

    ivs_store_put(&r);
    s_last_rx = ivs_now_ms();
    s_state = IVS_STREAMING;
}

static void mock_loop(void)
{
    uint64_t seq = 0;
    s_t0 = ivs_now_ms();
    s_state = IVS_SUBSCRIBED;

    while (s_run) {
        const uint32_t age_ms = (uint32_t)(ivs_now_ms() - s_t0);
        mock_emit(age_ms, ++seq);

#ifdef _WIN32
        Sleep(MOCK_FPS_PERIOD_MS);
#else
        struct timespec ts = { 0, MOCK_FPS_PERIOD_MS * 1000000L };
        nanosleep(&ts, NULL);
#endif
    }
}

#ifdef _WIN32
static DWORD WINAPI mock_thread(LPVOID p) { (void)p; mock_loop(); return 0; }
#else
static void *mock_thread(void *p) { (void)p; mock_loop(); return NULL; }
#endif

/* ---------------- ops 实现 ---------------- */

static void mock_close(void);

static int mock_open(const char *host, int port, const char *task)
{
    (void)host;
    (void)port;

    if (s_run) mock_close();                        /* 单例语义：先关旧会话 */

    ivs_strcpy(s_task, sizeof(s_task), (task && task[0]) ? task : "face");
    ivs_store_reset();
    s_last_rx = ivs_now_ms();
    s_state = IVS_CONNECTING;
    s_run = 1;

#ifdef _WIN32
    s_th = CreateThread(NULL, 0, mock_thread, NULL, 0, NULL);
    if (s_th == NULL) { s_run = 0; s_state = IVS_FAILED; return -1; }
#else
    if (pthread_create(&s_th, NULL, mock_thread, NULL) != 0) {
        s_run = 0; s_state = IVS_FAILED; return -1;
    }
#endif
    return 0;
}

static void mock_close(void)
{
    if (!s_run) { s_state = IVS_IDLE; return; }

    s_run = 0;
#ifdef _WIN32
    if (s_th) { WaitForSingleObject(s_th, 1000); CloseHandle(s_th); s_th = NULL; }
#else
    pthread_join(s_th, NULL);
#endif
    ivs_store_reset();
    s_state = IVS_IDLE;
    s_last_rx = 0;
}

static ivs_state_t mock_state(void)
{
    /* NODATA 惰性推导：省一个看门狗线程 */
    if ((s_state == IVS_STREAMING || s_state == IVS_SUBSCRIBED) &&
        s_last_rx != 0 && (ivs_now_ms() - s_last_rx) > MOCK_NODATA_MS) {
        return IVS_NODATA;
    }
    return s_state;
}

static bool mock_latest(ivs_result_t *out, uint32_t *ver)
{
    return ivs_store_get(out, ver);
}

static const char *mock_last_error(void)
{
    return s_err;
}

const ivs_source_ops_t ivs_src_mock = {
    "mock",
    mock_open,
    mock_close,
    mock_state,
    mock_latest,
    mock_last_error
};
