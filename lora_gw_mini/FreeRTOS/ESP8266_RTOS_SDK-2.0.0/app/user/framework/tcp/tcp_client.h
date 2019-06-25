#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C" {
#endif


#define TCP_IOT_SERVER_URL              "www.iowin.cn"
#define TCP_REMOTE_SERVER_PORT          8010
#undef  UPGRADE_SSL_ENABLE

//#define TCP_SERVER_REMOTE_IP          "120.55.58.166"
#define TCP_SERVER_REMOTE_IP            "47.105.47.213"
#define TCP_SERVER_REMOTE_PORT          (8010)
#define TCP_CLIENT_LOCAL_PORT           (7668)

typedef void (*tcp_recv_data_callback)(void *arg, char *buffer, unsigned short length);

boolean ICACHE_FLASH_ATTR tcp_client_create(void);
void ICACHE_FLASH_ATTR tcp_client_set_callback(tcp_recv_data_callback cb, void *arg);
void ICACHE_FLASH_ATTR tcp_client_send_msg(uint8 *buffer, uint16 len);
void ICACHE_FLASH_ATTR tcp_client_destroy(void);

#ifdef __cplusplus
}
#endif

#endif // __TCP_CLIENT_H
