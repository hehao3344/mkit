#include <stdio.h>
#include <string.h>

#include "user_interface.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"
#include "c_types.h"

#include "tcp_server.h"

typedef struct _TCP_SERVER_OBJECT
{    
    struct espconn  commu_conn;
    struct _esp_tcp commu_user_tcp;
  
    recv_callback cb;
    void *arg;

} TCP_SERVER_OBJECT;

static TCP_SERVER_OBJECT * instance(void);
LOCAL void ICACHE_FLASH_ATTR commu_listen(void *arg);
LOCAL void ICACHE_FLASH_ATTR commu_discon(void *arg);
LOCAL void ICACHE_FLASH_ATTR commu_recon(void *arg, sint8 err);
LOCAL void ICACHE_FLASH_ATTR commu_recv(void *arg, char *buffer, unsigned short length);

int ICACHE_FLASH_ATTR tcp_server_create(void)
{
    TCP_SERVER_OBJECT *handle = instance();
    if (NULL == handle)
    {
        os_printf("tcp svr failed \n");
        return -1;
    }

    /* 作为TCP服务器 接收客户端的请求 */
    handle->commu_conn.proto.tcp = &handle->commu_user_tcp;
    handle->commu_conn.type  = ESPCONN_TCP;
    handle->commu_conn.state = ESPCONN_NONE;
    handle->commu_conn.proto.tcp->local_port = TCP_BIND_PORT;
    espconn_regist_connectcb(&handle->commu_conn, commu_listen);
    espconn_accept &handle->commu_conn);

    os_printf("[tcp svr] create ok \n");

    return 0;
}

void ICACHE_FLASH_ATTR tcp_server_set_callback(recv_callback cb, void *arg)
{
    TCP_SERVER_OBJECT *handle = instance();

    handle->cb  = cb;
    handle->arg = arg;
}

void tcp_server_send_msg(char * buffer, int len)
{
    TCP_SERVER_OBJECT *handle = instance();
    
    espconn_sent(&handle->commu_conn, buffer, len);
    //os_printf("remote: %d.%d.%d.%d:%d\n",
    //            handle->commu_conn.proto.tcp->remote_ip[0],
    //    		  handle->commu_conn.proto.tcp->remote_ip[1],
    //    		  handle->commu_conn.proto.tcp->remote_ip[2],
    //    		  handle->commu_conn.proto.tcp->remote_ip[3],
    //    		  handle->commu_conn.proto.tcp->remote_port);
}

void ICACHE_FLASH_ATTR tcp_server_destroy(void)
{
    TCP_SERVER_OBJECT *handle = instance();
    if (NULL != handle)
    {
        os_free(handle);
    }
}

////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
static TCP_SERVER_OBJECT * instance(void)
{
    static TCP_SERVER_OBJECT * handle = NULL;
    if (NULL == handle)
    {
        handle = (TCP_SERVER_OBJECT *)os_zalloc(sizeof(TCP_SERVER_OBJECT));
        os_memset(handle, 0, sizeof(TCP_SERVER_OBJECT));
    }

    return handle;
}

////////////////////////////////////////////////////////////////////////////////
LOCAL void ICACHE_FLASH_ATTR commu_discon(void *arg)
{
    TCP_SERVER_OBJECT *handle = instance();
    os_printf("commu disconnect \n");
}

LOCAL void ICACHE_FLASH_ATTR commu_recon(void *arg, sint8 err)
{
    os_printf("commu reconn \n");
}

LOCAL void ICACHE_FLASH_ATTR commu_listen(void *arg)
{
    struct espconn * pesp_conn = arg;

    os_printf("new connection commu \n");
    espconn_regist_recvcb(pesp_conn,   commu_recv);
    espconn_regist_reconcb(pesp_conn,  commu_recon);
    espconn_regist_disconcb(pesp_conn, commu_discon);
}

LOCAL void ICACHE_FLASH_ATTR commu_recv(void *arg, char *buffer, int length)
{
    TCP_SERVER_OBJECT *handle = instance();

    handle->cb(handle->arg, buffer, length);               
}
