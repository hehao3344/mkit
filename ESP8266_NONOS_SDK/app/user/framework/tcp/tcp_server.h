#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif
 
#define TCP_BIND_PORT           (12080)

typedef void (* recv_callback)(void *arg, char *buffer, int length);

int  ICACHE_FLASH_ATTR tcp_server_create(void);
void ICACHE_FLASH_ATTR tcp_server_set_callback(recv_callback cb, void *arg);
void ICACHE_FLASH_ATTR tcp_server_send_msg(char * buffer, int len);
void ICACHE_FLASH_ATTR tcp_server_destroy(void);

#ifdef __cplusplus
}
#endif

#endif // __TCP_SERVER_H
