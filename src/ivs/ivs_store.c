/**
 * @file ivs_store.c
 * @brief 最新值单槽实现 + 共用单调时钟
 */

#include "ivs_store.h"
#include "ivs_source.h"

#include <string.h>

#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION ivs_lock_t;
#define IVS_LOCK_INIT(l)   InitializeCriticalSection(l)
#define IVS_LOCK_DEL(l)    DeleteCriticalSection(l)
#define IVS_LOCK(l)        EnterCriticalSection(l)
#define IVS_UNLOCK(l)      LeaveCriticalSection(l)
#else
#include <pthread.h>
#include <time.h>
typedef pthread_mutex_t ivs_lock_t;
#define IVS_LOCK_INIT(l)   pthread_mutex_init((l), NULL)
#define IVS_LOCK_DEL(l)    pthread_mutex_destroy(l)
#define IVS_LOCK(l)        pthread_mutex_lock(l)
#define IVS_UNLOCK(l)      pthread_mutex_unlock(l)
#endif

static ivs_result_t s_slot;
static uint32_t     s_ver = 0;
static bool         s_ready = false;
static ivs_lock_t   s_lock;
static bool         s_lock_inited = false;

static void store_ensure_init(void)
{
    if (s_lock_inited) return;
    IVS_LOCK_INIT(&s_lock);
    memset(&s_slot, 0, sizeof(s_slot));
    s_ver = 0;
    s_ready = false;
    s_lock_inited = true;
}

void ivs_store_put(const ivs_result_t *r)
{
    if (r == NULL) return;
    store_ensure_init();

    IVS_LOCK(&s_lock);
    s_slot = *r;                 /* 定长 POD，整体覆盖即"最新值"语义 */
    s_ver++;
    s_ready = true;
    IVS_UNLOCK(&s_lock);
}

bool ivs_store_get(ivs_result_t *out, uint32_t *ver)
{
    if (out == NULL) return false;
    store_ensure_init();

    bool ok;
    IVS_LOCK(&s_lock);
    ok = s_ready;
    if (ok) {
        *out = s_slot;
        if (ver) *ver = s_ver;
    }
    IVS_UNLOCK(&s_lock);
    return ok;
}

bool ivs_store_get_if_new(ivs_result_t *out, uint32_t *last_ver)
{
    if (out == NULL || last_ver == NULL) return false;
    store_ensure_init();

    bool got = false;
    IVS_LOCK(&s_lock);
    if (s_ready && s_ver != *last_ver) {
        *out = s_slot;
        *last_ver = s_ver;
        got = true;
    }
    IVS_UNLOCK(&s_lock);
    return got;
}

uint32_t ivs_store_version(void)
{
    store_ensure_init();
    uint32_t v;
    IVS_LOCK(&s_lock);
    v = s_ver;
    IVS_UNLOCK(&s_lock);
    return v;
}

void ivs_store_reset(void)
{
    store_ensure_init();
    IVS_LOCK(&s_lock);
    memset(&s_slot, 0, sizeof(s_slot));
    s_ver = 0;
    s_ready = false;
    IVS_UNLOCK(&s_lock);
}

/* ===================== 共用单调时钟 ===================== */

uint64_t ivs_now_ms(void)
{
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)(ts.tv_nsec / 1000000);
#endif
}
