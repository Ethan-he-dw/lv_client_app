#ifndef COMM_UDP_H
#define COMM_UDP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int  comm_udp_init(void);
void comm_udp_deinit(void);

int  comm_udp_broadcast_search(void);
int  comm_udp_send_text(const char * ip, const char * text);

void comm_udp_set_rx_cb(void (*cb)(const char * ip, uint32_t dev_id));

#ifdef __cplusplus
}
#endif

#endif