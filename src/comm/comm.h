#ifndef COMM_H
#define COMM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COMM_MAX_IP_LEN      64
#define COMM_MAX_DATA_LEN    1024
#define COMM_QUEUE_SIZE      64
#define COMM_UDP_PORT        8888
#define COMM_SEARCH_MSG      "MYAPP_SEARCH"
#define COMM_DEV_REPLY_FMT   "MYAPP_DEV:%u"

typedef enum {
    COMM_EVT_SEARCH_DONE = 0,
    COMM_EVT_DEVICE_FOUND,
    COMM_EVT_BACKEND_DATA,
    COMM_EVT_LOG,
    COMM_EVT_UPGRADE_PROGRESS,
} comm_event_type_t;

typedef struct {
    comm_event_type_t type;
    char     ip[COMM_MAX_IP_LEN];
    uint32_t dev_id;
    char     data[COMM_MAX_DATA_LEN];
    uint32_t data_len;
    int      progress;
} comm_event_t;

typedef struct {
    char     ip[COMM_MAX_IP_LEN];
    uint32_t dev_id;
} comm_device_t;

void  comm_init(void);
void  comm_deinit(void);

int   comm_search_devices(void);
int   comm_send_backend_cmd(const char * ip, const char * cmd);
int   comm_start_upgrade(const char * ip, const char * file_path);
int   comm_send_debug_cmd(const char * ip, const char * cmd);

bool  comm_pull_event(comm_event_t * out);
void  comm_clear_devices(void);

int   comm_get_device_count(void);
bool  comm_get_device(int idx, comm_device_t * out);

#ifdef __cplusplus
}
#endif

#endif