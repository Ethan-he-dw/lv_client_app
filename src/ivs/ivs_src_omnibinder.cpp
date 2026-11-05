/**
 * @file ivs_src_omnibinder.cpp
 * @brief 真机订阅源：OmniBinder 原生客户端
 *
 * 线程模型（受框架约束，别改）：
 *   ctl 线程：init → 起 drv → lookupService → subscribeTopic（都是同步阻塞 API，
 *             必须有人在泵 event-loop 才能返回，所以先起 drv）
 *   drv 线程：rt->run() 阻塞，直到 close() 里 rt->stop()
 *   TopicCallback 在 drv 线程执行：只做「转换 + 写单槽 + 记时间」，禁止任何阻塞/IO/lv_*
 *
 * 生命周期要点：
 *   · OmniRuntime 不可复用（stop() 是终态、init() 只能成功一次）→ 每次重试 new 一个
 *   · subscribeTopic 用 4 参版本以拿到 on_err（ERR_IDL_MISMATCH 可见）；
 *     生成的 SubscribeFaceResult 把 on_err 写死 nullptr，会退化成"永远没数据"
 */

#include "ivs_source.h"
#include "ivs_store.h"
#include "ivs_task_face.h"

#include <omnibinder/omnibinder.h>

#include "face_service.bidl.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>

namespace {

constexpr int kMaxAttempts = 10;
constexpr int kBackoffSec[kMaxAttempts] = { 1, 2, 4, 8, 16, 16, 16, 16, 16, 16 };
constexpr int64_t kNoDataMs = 3000;    /* 3s 无帧 → NODATA（与 mock 一致） */

/* ---------------- 任务绑定表：新增任务只加一行 ---------------- */

using ConvertFn = bool (*)(const omnibinder::Buffer &, ivs_result_t *);

struct Binding {
    const char *task;      /* 客户端选的类型名，如 "face" */
    const char *service;   /* 板端服务名 */
    const char *topic;     /* 板端 topic 名 */
    uint32_t    idl_hash;  /* 生成代码里的常量，改 IDL 会自动变 */
    ConvertFn   convert;
};

const Binding kBindings[] = {
    { "face", "FaceService", "FaceResult", mediaipc::FaceResult::TOPIC_IDL_HASH, IvsConvertFace },
    /* 新任务：{ "person", "PersonService", "PersonResult", PersonResult::TOPIC_IDL_HASH, IvsConvertPerson }, */
};

const Binding *FindBinding(const char *task)
{
    if (task == NULL || task[0] == '\0') task = "face";
    for (const Binding &b : kBindings) {
        if (strcmp(b.task, task) == 0) return &b;
    }
    return NULL;
}

/* ---------------- 会话 ---------------- */

struct Session {
    std::thread  ctl;
    std::thread  drv;
    std::atomic<bool>    stop{false};
    std::atomic<int>     state{IVS_IDLE};
    std::atomic<int64_t> last_rx_ms{0};
    std::atomic<int64_t> t_sub_ms{0};     /* 进入 SUBSCRIBED 的时刻（用于"从未收到数据"的超时判定） */
    std::atomic<int>     last_err_code{0};

    omnibinder::OmniRuntime *rt{nullptr};
    const Binding           *bind{nullptr};

    std::mutex mu;
    char       err[160]{0};
    char       host[64]{0};
    int        port{9900};
};

Session *g_sess = nullptr;   /* 单例 */

/* 前向声明（ob_open 内部会先关旧会话） */
int  ob_open(const char *host, int port, const char *task);
void ob_close(void);

/* ---------------- 错误记录（加锁） ---------------- */

void set_err(Session *s, int code, const char *txt)
{
    std::lock_guard<std::mutex> lk(s->mu);
    s->last_err_code.store(code);
    snprintf(s->err, sizeof(s->err), "%s", (txt != NULL) ? txt : "");
}

void set_errf(Session *s, int code, const char *fmt, int arg)
{
    std::lock_guard<std::mutex> lk(s->mu);
    s->last_err_code.store(code);
    snprintf(s->err, sizeof(s->err), fmt, arg);
}

void clear_err(Session *s)
{
    std::lock_guard<std::mutex> lk(s->mu);
    s->last_err_code.store(0);
    s->err[0] = '\0';
}

bool sleep_interruptible(Session *s, int ms)
{
    const int step = 100;
    for (int t = 0; t < ms; t += step) {
        if (s->stop.load()) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(step));
    }
    return !s->stop.load();
}

bool wait_running(Session *s, int timeout_ms)
{
    for (int t = 0; t < timeout_ms; t += 10) {
        if (s->stop.load()) return false;
        if (s->rt != NULL && s->rt->isRunning()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return (s->rt != NULL) && s->rt->isRunning();
}

/** 拆掉本轮 runtime：停 loop → join drv → 释放（顺序不能反） */
void teardown_rt(Session *s)
{
    if (s->drv.joinable()) {
        if (s->rt != NULL) s->rt->stop();
        s->drv.join();
    }
    delete s->rt;
    s->rt = nullptr;
}

/* ---------------- 回调（在 drv 线程执行，只做轻活） ---------------- */

void on_msg(Session *s, const omnibinder::Buffer &data)
{
    if (s->stop.load()) return;

    ivs_result_t r;
    if (!s->bind->convert(data, &r)) return;   /* 解析失败静默丢弃 */

    ivs_store_put(&r);
    s->last_rx_ms.store((int64_t)ivs_now_ms());
    if (s->state.load() != IVS_STREAMING) s->state.store(IVS_STREAMING);
    if (s->last_err_code.load() != 0) clear_err(s);
}

void on_err(Session *s, int code)
{
    if (code == (int)omnibinder::ErrorCode::ERR_IDL_MISMATCH) {
        set_err(s, code, "IDL 版本不匹配（板端 IDL 与 PC 生成代码不一致）");
        s->state.store(IVS_FAILED);
    } else {
        set_errf(s, code, "topic 错误 code=%d", code);
    }
}

/* ---------------- 线程主体 ---------------- */

void drv_main(Session *s)
{
    if (s->rt != NULL) s->rt->run();
}

void ctl_main(Session *s)
{
    for (int attempt = 0; attempt < kMaxAttempts; attempt++) {
        if (s->stop.load()) return;

        if (attempt > 0) {
            if (!sleep_interruptible(s, kBackoffSec[attempt - 1] * 1000)) return;
        }

        s->state.store(IVS_CONNECTING);
        clear_err(s);

        /* 每次尝试都是全新的 runtime（不可复用） */
        try {
            s->rt = new omnibinder::OmniRuntime();
        } catch (...) {
            set_err(s, -1, "OmniRuntime 创建失败");
            continue;
        }

        const int rc_init = s->rt->init(std::string(s->host), (uint16_t)s->port);
        if (rc_init != 0) {
            set_errf(s, rc_init, "连接 service_manager 失败 rc=%d", rc_init);
            teardown_rt(s);
            continue;
        }

        /* 先起驱动，再调同步 API：否则没人泵 event-loop，lookup/subscribe 会超时 */
        s->drv = std::thread(drv_main, s);
        if (!wait_running(s, 800)) {
            set_err(s, -1, "event-loop 启动失败");
            teardown_rt(s);
            continue;
        }

        omnibinder::ServiceInfo info;
        const int rc_lookup = s->rt->lookupService(std::string(s->bind->service), info);
        if (rc_lookup != 0) {
            set_errf(s, rc_lookup, "服务未注册（板端未起） rc=%d", rc_lookup);
            teardown_rt(s);
            continue;
        }

        const int rc_sub = s->rt->subscribeTopic(
            std::string(s->bind->topic),
            s->bind->idl_hash,
            [s](uint32_t, const omnibinder::Buffer &data) { on_msg(s, data); },
            [s](uint32_t, omnibinder::ErrorCode err, const omnibinder::Buffer &) { on_err(s, (int)err); });
        if (rc_sub != 0) {
            set_errf(s, rc_sub, "订阅失败 rc=%d", rc_sub);
            teardown_rt(s);
            continue;
        }

        s->t_sub_ms.store((int64_t)ivs_now_ms());
        s->state.store(IVS_SUBSCRIBED);
        return;                     /* ctl 使命完成，drv 继续泵事件 */
    }

    if (!s->stop.load()) s->state.store(IVS_FAILED);
}

/* ---------------- ops ---------------- */

int ob_open(const char *host, int port, const char *task)
{
    try {
        if (host == NULL || host[0] == '\0') return -1;

        const Binding *b = FindBinding(task);
        if (b == NULL) return -2;

        ob_close();                 /* 单例：先关旧会话 */

        Session *s = new Session();
        s->bind = b;
        s->port = (port > 0) ? port : 9900;
        ivs_strcpy(s->host, sizeof(s->host), host);

        ivs_store_reset();
        s->state.store(IVS_CONNECTING);
        s->last_rx_ms.store(0);
        clear_err(s);

        g_sess = s;
        s->ctl = std::thread(ctl_main, s);
        return 0;
    } catch (...) {
        return -3;
    }
}

void ob_close(void)
{
    Session *s = g_sess;
    if (s == NULL) return;

    s->stop.store(true);

    /* 先 join ctl（它可能正卡在同步 API 上，超时后会自行退出），
     * 再由我们的线程 stop() 打断 drv，最后 join —— 顺序不能反。 */
    if (s->ctl.joinable()) s->ctl.join();
    if (s->drv.joinable()) {
        if (s->rt != NULL) s->rt->stop();
        s->drv.join();
    }
    delete s->rt;
    s->rt = nullptr;

    delete s;
    g_sess = nullptr;

    ivs_store_reset();
}

ivs_state_t ob_state(void)
{
    Session *s = g_sess;
    if (s == NULL) return IVS_IDLE;

    const ivs_state_t st = (ivs_state_t)s->state.load();
    const int64_t now = (int64_t)ivs_now_ms();
    if (st == IVS_STREAMING) {
        const int64_t lr = s->last_rx_ms.load();
        if (lr != 0 && (now - lr) > kNoDataMs) return IVS_NODATA;
    }
    else if (st == IVS_SUBSCRIBED) {
        /* 订阅成功但从未收到数据：同样算 NODATA（否则会永远停在"订阅中"） */
        const int64_t ts = s->t_sub_ms.load();
        if (ts != 0 && (now - ts) > kNoDataMs) return IVS_NODATA;
    }
    return st;
}

bool ob_latest(ivs_result_t *out, uint32_t *ver)
{
    return ivs_store_get(out, ver);
}

const char *ob_last_error(void)
{
    static char buf[160];
    Session *s = g_sess;
    if (s == NULL) return "";

    std::lock_guard<std::mutex> lk(s->mu);
    snprintf(buf, sizeof(buf), "%s", s->err);
    return buf;
}

}  // namespace

extern "C" const ivs_source_ops_t ivs_src_omnibinder = {
    "omnibinder",
    ob_open,
    ob_close,
    ob_state,
    ob_latest,
    ob_last_error
};
