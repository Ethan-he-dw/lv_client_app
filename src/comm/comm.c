#include "comm.h"
#include "comm_udp.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>

typedef CRITICAL_SECTION comm_lock_t;
#define COMM_LOCK_INIT(l) InitializeCriticalSection(l)
#define COMM_LOCK_DEL(l) DeleteCriticalSection(l)
#define COMM_LOCK(l) EnterCriticalSection(l)
#define COMM_UNLOCK(l) LeaveCriticalSection(l)
#define COMM_SLEEP(ms) Sleep(ms)
#else
#include <pthread.h>
#include <unistd.h>

typedef pthread_mutex_t comm_lock_t;
#define COMM_LOCK_INIT(l) pthread_mutex_init(l, NULL)
#define COMM_LOCK_DEL(l) pthread_mutex_destroy(l)
#define COMM_LOCK(l) pthread_mutex_lock(l)
#define COMM_UNLOCK(l) pthread_mutex_unlock(l)
#define COMM_SLEEP(ms) usleep((ms) * 1000)
#endif

typedef struct {
    comm_event_t buf[COMM_QUEUE_SIZE];
    int head;
    int tail;
    int count;
    comm_lock_t lock;
} evt_queue_t;

typedef struct {
    char ip[COMM_MAX_IP_LEN];
    uint32_t dev_id;
} dev_entry_t;

#define MAX_DEVICES 32

static evt_queue_t s_q;
static dev_entry_t s_devs[MAX_DEVICES];
static int s_dev_cnt = 0;
static comm_lock_t s_dev_lock;
static bool s_inited = false;
static bool s_searching = false;
static bool s_backend_active = false;
static bool s_upgrading = false;
static int  s_upg_progress = 0;
static uint32_t s_fake_seq = 0;

#ifdef _WIN32
static HANDLE s_sim_thread = NULL;
static DWORD WINAPI sim_thread_fn(LPVOID p);
#else
static void * sim_thread_fn(void * p);
#endif

static void queue_push(const comm_event_t * evt)
{
    COMM_LOCK(&s_q.lock);
    if(s_q.count < COMM_QUEUE_SIZE) {
        s_q.buf[s_q.tail] = *evt;
        s_q.tail = (s_q.tail + 1) % COMM_QUEUE_SIZE;
        s_q.count++;
    }
    COMM_UNLOCK(&s_q.lock);
}

static void on_udp_packet(const char * ip, uint32_t dev_id)
{
    bool known = false;
    COMM_LOCK(&s_dev_lock);
    for(int i = 0; i < s_dev_cnt; i++) {
        if(s_devs[i].dev_id == dev_id && strcmp(s_devs[i].ip, ip) == 0) {
            known = true;
            break;
        }
    }
    if(!known && s_dev_cnt < MAX_DEVICES) {
        strncpy(s_devs[s_dev_cnt].ip, ip, COMM_MAX_IP_LEN - 1);
        s_devs[s_dev_cnt].ip[COMM_MAX_IP_LEN - 1] = '\0';
        s_devs[s_dev_cnt].dev_id = dev_id;
        s_dev_cnt++;
    }
    COMM_UNLOCK(&s_dev_lock);

    comm_event_t evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = COMM_EVT_DEVICE_FOUND;
    strncpy(evt.ip, ip, COMM_MAX_IP_LEN - 1);
    evt.dev_id = dev_id;
    queue_push(&evt);
}

static void sim_thread_fn_impl(void)
{
    while(1) {
#ifdef _WIN32
        if(s_sim_thread == NULL) break;
#endif
        {
            comm_event_t evt;

            if(s_searching) {
                s_searching = false;
                memset(&evt, 0, sizeof(evt));
                evt.type = COMM_EVT_SEARCH_DONE;
                evt.progress = s_dev_cnt;
                queue_push(&evt);
            }

            if(s_upgrading) {
                s_upg_progress += 8;
                if(s_upg_progress >= 100) {
                    s_upg_progress = 100;
                    s_upgrading = false;
                }
                memset(&evt, 0, sizeof(evt));
                evt.type = COMM_EVT_UPGRADE_PROGRESS;
                evt.progress = s_upg_progress;
                queue_push(&evt);
            }

            if(s_backend_active) {
                memset(&evt, 0, sizeof(evt));
                evt.type = COMM_EVT_BACKEND_DATA;
                s_fake_seq++;
                snprintf(evt.data, sizeof(evt.data),
                         "{\"seq\":%u,\"type\":\"sample\",\"value\":%u}",
                         s_fake_seq, (s_fake_seq * 7) % 100);
                evt.data_len = (uint32_t)strlen(evt.data);
                queue_push(&evt);
            }

            memset(&evt, 0, sizeof(evt));
            evt.type = COMM_EVT_LOG;
            snprintf(evt.data, sizeof(evt.data),
                     "[sim] uptime=%us devices=%d upgrading=%d",
                     (unsigned)(s_fake_seq), s_dev_cnt, s_upgrading ? 1 : 0);
            evt.data_len = (uint32_t)strlen(evt.data);
            queue_push(&evt);
        }
        COMM_SLEEP(1000);
    }
}

#ifdef _WIN32
static DWORD WINAPI sim_thread_fn(LPVOID p)
{
    (void)p;
    sim_thread_fn_impl();
    return 0;
}
#else
static void * sim_thread_fn(void * p)
{
    (void)p;
    sim_thread_fn_impl();
    return NULL;
}
#endif

void comm_init(void)
{
    if(s_inited) return;
    memset(&s_q, 0, sizeof(s_q));
    COMM_LOCK_INIT(&s_q.lock);
    COMM_LOCK_INIT(&s_dev_lock);
    s_dev_cnt = 0;
    s_inited = true;

    comm_udp_set_rx_cb(on_udp_packet);
    comm_udp_init();

#ifdef _WIN32
    s_sim_thread = CreateThread(NULL, 0, sim_thread_fn, NULL, 0, NULL);
#else
    pthread_t tid;
    pthread_create(&tid, NULL, sim_thread_fn, NULL);
    pthread_detach(tid);
#endif
}

void comm_deinit(void)
{
    if(!s_inited) return;
    s_inited = false;
#ifdef _WIN32
    if(s_sim_thread) {
        TerminateThread(s_sim_thread, 0);
        CloseHandle(s_sim_thread);
        s_sim_thread = NULL;
    }
#endif
    comm_udp_deinit();
    COMM_LOCK_DEL(&s_q.lock);
    COMM_LOCK_DEL(&s_dev_lock);
}

int comm_search_devices(void)
{
    COMM_LOCK(&s_dev_lock);
    s_dev_cnt = 0;
    COMM_UNLOCK(&s_dev_lock);
    s_searching = true;
    return comm_udp_broadcast_search();
}

void comm_clear_devices(void)
{
    COMM_LOCK(&s_dev_lock);
    s_dev_cnt = 0;
    COMM_UNLOCK(&s_dev_lock);
}

int comm_send_backend_cmd(const char * ip, const char * cmd)
{
    (void)ip;
    (void)cmd;
    s_backend_active = true;
    return 0;
}

int comm_start_upgrade(const char * ip, const char * file_path)
{
    if(s_upgrading) return -1;
    char buf[COMM_MAX_DATA_LEN];
    snprintf(buf, sizeof(buf), "MYAPP_UPGRADE:%s", file_path);
    comm_udp_send_text(ip, buf);
    s_upg_progress = 0;
    s_upgrading = true;
    return 0;
}

int comm_send_debug_cmd(const char * ip, const char * cmd)
{
    comm_udp_send_text(ip, cmd);

    comm_event_t evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = COMM_EVT_LOG;
    snprintf(evt.data, sizeof(evt.data), ">> %s -> %s", cmd, ip);
    evt.data_len = (uint32_t)strlen(evt.data);
    queue_push(&evt);
    return 0;
}

bool comm_pull_event(comm_event_t * out)
{
    bool got = false;
    COMM_LOCK(&s_q.lock);
    if(s_q.count > 0) {
        *out = s_q.buf[s_q.head];
        s_q.head = (s_q.head + 1) % COMM_QUEUE_SIZE;
        s_q.count--;
        got = true;
    }
    COMM_UNLOCK(&s_q.lock);
    return got;
}

int comm_get_device_count(void)
{
    int n = 0;
    COMM_LOCK(&s_dev_lock);
    n = s_dev_cnt;
    COMM_UNLOCK(&s_dev_lock);
    return n;
}

bool comm_get_device(int idx, comm_device_t * out)
{
    bool ok = false;
    COMM_LOCK(&s_dev_lock);
    if(idx >= 0 && idx < s_dev_cnt) {
        strncpy(out->ip, s_devs[idx].ip, COMM_MAX_IP_LEN - 1);
        out->ip[COMM_MAX_IP_LEN - 1] = '\0';
        out->dev_id = s_devs[idx].dev_id;
        ok = true;
    }
    COMM_UNLOCK(&s_dev_lock);
    return ok;
}