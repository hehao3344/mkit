#include <stdio.h>
#include <string.h>

#include "../core/core.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"
#include "c_types.h"

#include "../core/platform.h"
#include "../core/version.h"
#include "../device/flash_param.h"
#include "tcp_client.h"

typedef struct _TCP_CLIENT_OBJECT
{
    int32   count;
    boolean proxy_is_handle;
    boolean proxy_connected;

    int8 inform_buf[64];
    int8 mac[40];
    struct espconn  proxy_svr_conn;
    struct _esp_tcp proxy_user_tcp;

    os_timer_t      register_timer; // contain keep alive.
    ip_addr_t       proxy_server_ip;

    recv_data_callback cb;
    void *arg;
} TCP_CLIENT_OBJECT;

static TCP_CLIENT_OBJECT * instance(void);
LOCAL void ICACHE_FLASH_ATTR proxy_recv(void *arg, char *buffer, unsigned short length);
LOCAL void ICACHE_FLASH_ATTR register_center(void *arg);
LOCAL void ICACHE_FLASH_ATTR proxy_sent_cb(void *arg);
LOCAL void ICACHE_FLASH_ATTR proxy_connect_cb(void *arg);
LOCAL void ICACHE_FLASH_ATTR proxy_discon_cb(void *arg);
LOCAL void ICACHE_FLASH_ATTR proxy_recon_cb(void *arg, sint8 err);
LOCAL void ICACHE_FLASH_ATTR connect_proxy_server(void);

boolean ICACHE_FLASH_ATTR tcp_client_create(void)
{
    TCP_CLIENT_OBJECT *handle = instance();
    if (NULL == handle)
    {
        os_printf("tcp dev not enough memory \n");
        return FALSE;
    }

    handle->proxy_svr_conn.proto.tcp = &handle->proxy_user_tcp;
    handle->proxy_svr_conn.type      = ESPCONN_TCP;
    handle->proxy_svr_conn.state     = ESPCONN_NONE;

    char mac[24] = {0};
    wifi_get_macaddr(STATION_IF, mac);

  	os_sprintf(handle->mac, MACSTR, MAC2STR(mac));

    uint8 i, j;

    for (i=0,j=0; i<12;)
    {
        handle->mac[i]   = handle->mac[j];
        handle->mac[i+1] = handle->mac[j+1];
        i += 2;
        j += 3;
    }
    handle->mac[12] = 0;

#if 0
    handle->mac[2] = handle->mac[3];
    handle->mac[3] = handle->mac[4];
    handle->mac[4] = handle->mac[6];
    handle->mac[5] = handle->mac[7];
    handle->mac[6] = handle->mac[9];
    handle->mac[7] = handle->mac[10];
    handle->mac[8] = handle->mac[12];
    handle->mac[9] = handle->mac[13];
    handle->mac[10] = handle->mac[15];
    handle->mac[11] = handle->mac[16];
    handle->mac[12] = 0;
#endif


    os_sprintf(handle->inform_buf,  "{\"device\":\"%s\"}\n", handle->mac);

    os_printf("get mac %s %s\n", handle->mac, handle->inform_buf);

    uint32 ip = ipaddr_addr(TCP_SERVER_REMOTE_IP);
    // const char tcp_server_addr[4] = {120, 55, 58, 166};
    //os_memcpy(handle->proxy_svr_conn.proto.tcp->remote_ip, tcp_server_addr, 4);
    os_memcpy(handle->proxy_svr_conn.proto.tcp->remote_ip, &ip, 4);

    os_timer_disarm(&handle->register_timer);
    os_timer_setfn(&handle->register_timer, (os_timer_func_t *)register_center, handle);
    os_timer_arm(&handle->register_timer, 2000, 1);

    os_printf("[tcp client] create ok \n");
    return TRUE;
}

void ICACHE_FLASH_ATTR tcp_client_set_callback(recv_data_callback cb, void *arg)
{
    TCP_CLIENT_OBJECT *handle = instance();

    handle->cb  = cb;
    handle->arg = arg;
}

void tcp_client_send_msg(uint8 *buffer, uint16 len)
{
    TCP_CLIENT_OBJECT *handle = instance();
    if (handle->proxy_connected)
    {
        espconn_sent(&handle->proxy_svr_conn, buffer, len);
    }
}

void ICACHE_FLASH_ATTR tcp_client_destroy(void)
{
    TCP_CLIENT_OBJECT *handle = instance();
    if (NULL != handle)
    {
        os_free(handle);
    }
}

////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
static TCP_CLIENT_OBJECT * instance(void)
{
    static TCP_CLIENT_OBJECT * handle = NULL;
    if (NULL == handle)
    {
        handle = (TCP_CLIENT_OBJECT *)os_zalloc(sizeof(TCP_CLIENT_OBJECT));
        os_memset(handle, 0, sizeof(TCP_CLIENT_OBJECT));
    }

    return handle;
}

LOCAL void ICACHE_FLASH_ATTR connect_proxy_server(void)
{
    TCP_CLIENT_OBJECT *handle = instance();

    handle->proxy_svr_conn.proto.tcp->local_port  = TCP_CLIENT_LOCAL_PORT;
    handle->proxy_svr_conn.proto.tcp->remote_port = TCP_SERVER_REMOTE_PORT;
    espconn_regist_connectcb(&handle->proxy_svr_conn, proxy_connect_cb);
    espconn_regist_disconcb(&handle->proxy_svr_conn,  proxy_discon_cb);
    espconn_regist_reconcb(&handle->proxy_svr_conn,   proxy_recon_cb);
    espconn_connect(&handle->proxy_svr_conn);
    os_printf("connectting ... to tcp server \n");
}

////////////////////////////////////////////////////////////////////////////////
LOCAL void ICACHE_FLASH_ATTR proxy_sent_cb(void *arg)
{
    struct espconn *pespconn = arg;
    //os_printf("proxy_sent_cb\n");
}

LOCAL void ICACHE_FLASH_ATTR proxy_connect_cb(void *arg)
{
    TCP_CLIENT_OBJECT *handle = instance();
    struct espconn * pespconn = arg;
    os_printf("proxy connect success \n");
    espconn_regist_recvcb(pespconn, proxy_recv);
    espconn_regist_sentcb(pespconn, proxy_sent_cb);
    espconn_sent(pespconn, (uint8 *)handle->inform_buf, strlen(handle->inform_buf));

    handle->proxy_connected = TRUE;
}

LOCAL void ICACHE_FLASH_ATTR proxy_discon_cb(void *arg)
{
    TCP_CLIENT_OBJECT *handle = instance();

    os_printf("proxy disconnect \n");
    handle->proxy_is_handle  = FALSE;
    handle->proxy_connected  = FALSE;
    handle->count = 0;
}

LOCAL void ICACHE_FLASH_ATTR proxy_recon_cb(void *arg, sint8 err)
{
    os_printf("proxy reconn \n");
    TCP_CLIENT_OBJECT *handle = instance();

    handle->proxy_is_handle  = FALSE;
    handle->proxy_connected  = FALSE;
    handle->count = 0;
}

static void ICACHE_FLASH_ATTR register_center(void *arg)
{
    TCP_CLIENT_OBJECT * handle = (TCP_CLIENT_OBJECT *)arg;

    struct ip_info ipconfig;

    wifi_get_ip_info(STATION_IF, &ipconfig);
    int status = wifi_station_get_connect_status();
    if (status != STATION_GOT_IP)
    {
        os_printf("waitting for ... \n");
        handle->proxy_is_handle  = FALSE;
        handle->proxy_connected  = FALSE;
        return;
    }

    if (!handle->proxy_is_handle)
    {
        os_printf("register_center: connecting ... \n");
        handle->proxy_is_handle = TRUE;
        connect_proxy_server();
    }

    if (handle->proxy_connected)
    {
        if (0 == handle->count%5)
        {
            if (strlen(handle->inform_buf) > 0)
            {
                os_printf("update param %d \n", handle->count);
                espconn_sent(&handle->proxy_svr_conn, (uint8 *)handle->inform_buf, strlen(handle->inform_buf));
            }
        }
        handle->count++;
    }
}

////////////////////////////////////////////////////////////////////////////////
LOCAL void ICACHE_FLASH_ATTR proxy_recv(void *arg, char *buffer, unsigned short length)
{
    TCP_CLIENT_OBJECT *handle = instance();
    if (NULL != handle->cb)
    {
        os_printf("tcp recv msg [%s] len %d \n", buffer, length);
        handle->cb(handle->arg, buffer, length);
    }
}
