#include "comm_udp.h"
#include "comm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

static SOCKET s_sock = INVALID_SOCKET;
static HANDLE s_rx_thread = NULL;
static volatile bool s_run = false;
static void (*s_rx_cb)(const char * ip, uint32_t dev_id) = NULL;

static DWORD WINAPI rx_thread_fn(LPVOID p)
{
    (void)p;
    char buf[512];
    struct sockaddr_in from;
    int from_len = sizeof(from);

    while(s_run) {
        int n = recvfrom(s_sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &from_len);
        if(n <= 0) continue;
        buf[n] = '\0';

        if(strncmp(buf, "MYAPP_DEV:", 10) == 0) {
            uint32_t dev_id = 0;
            if(sscanf(buf + 10, "%u", &dev_id) == 1 && s_rx_cb) {
                const char * ip = inet_ntoa(from.sin_addr);
                s_rx_cb(ip, dev_id);
            }
        }
    }
    return 0;
}

int comm_udp_init(void)
{
    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;

    s_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(s_sock == INVALID_SOCKET) return -1;

    BOOL b = TRUE;
    setsockopt(s_sock, SOL_SOCKET, SO_BROADCAST, (const char *)&b, sizeof(b));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(COMM_UDP_PORT);
    bind(s_sock, (struct sockaddr *)&addr, sizeof(addr));

    s_run = true;
    s_rx_thread = CreateThread(NULL, 0, rx_thread_fn, NULL, 0, NULL);
    return 0;
}

void comm_udp_deinit(void)
{
    s_run = false;
    if(s_sock != INVALID_SOCKET) {
        closesocket(s_sock);
        s_sock = INVALID_SOCKET;
    }
    if(s_rx_thread) {
        WaitForSingleObject(s_rx_thread, 1000);
        CloseHandle(s_rx_thread);
        s_rx_thread = NULL;
    }
    WSACleanup();
}

int comm_udp_broadcast_search(void)
{
    if(s_sock == INVALID_SOCKET) return -1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    addr.sin_port = htons(COMM_UDP_PORT);
    return sendto(s_sock, COMM_SEARCH_MSG, (int)strlen(COMM_SEARCH_MSG), 0,
                  (struct sockaddr *)&addr, sizeof(addr));
}

int comm_udp_send_text(const char * ip, const char * text)
{
    if(s_sock == INVALID_SOCKET || ip == NULL || text == NULL) return -1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(COMM_UDP_PORT);
    return sendto(s_sock, text, (int)strlen(text), 0,
                  (struct sockaddr *)&addr, sizeof(addr));
}

void comm_udp_set_rx_cb(void (*cb)(const char * ip, uint32_t dev_id))
{
    s_rx_cb = cb;
}

#else

int comm_udp_init(void) { return 0; }
void comm_udp_deinit(void) {}
int comm_udp_broadcast_search(void) { return 0; }
int comm_udp_send_text(const char * ip, const char * text) { (void)ip; (void)text; return 0; }
void comm_udp_set_rx_cb(void (*cb)(const char * ip, uint32_t dev_id)) { (void)cb; }

#endif