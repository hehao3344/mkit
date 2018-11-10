#include <string.h>

#include "user_interface.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"
#include "c_types.h"

#include "discovery.h"

#define MAX_RECV_BUF_LEN    (256)

typedef struct _DISCOVERY_OBJECT
{
    struct espconn udp_conn;
    char  dev_uuid[DEV_UUID_LEN];  
    int   port;       
    char *out_buf;
} DISCOVERY_OBJECT;

LOCAL DISCOVERY_OBJECT * instance(void);
LOCAL void ICACHE_FLASH_ATTR discovery_recv(void *arg, char *pusrdata, unsigned short length);

int ICACHE_FLASH_ATTR discovery_create(DISCOVER_ENV *env)
{
    DISCOVERY_OBJECT * handle = instance();
    if (NULL == handle)
    {
        os_printf("[dis] create error \n");
        return -1;
    }

    if (os_strlen(env->dev_uuid) > 0)
    {
        os_strcpy((char*)handle->dev_uuid, (char*)env->dev_uuid);
    }

    handle->port = env->port;
    handle->out_buf = (int8*)os_zalloc(MAX_RECV_BUF_LEN);
    if (NULL == handle->out_buf)
    {
        return -1;
    }
    
    handle->udp_conn.type = ESPCONN_UDP;
    handle->udp_conn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    if (NULL == handle->udp_conn.proto.udp)
    {
        return -1;
    }
    handle->udp_conn.proto.udp->local_port  = BROADCASE_PORT;
    espconn_regist_recvcb(&handle->udp_conn, discovery_recv);
    espconn_create(&handle->udp_conn);
    os_printf("[dis] create ok \n");

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// confirm is discovery packet.
////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR discovery_destroy(void)
{
    DISCOVERY_OBJECT *handle = instance();
    if (NULL != handle)
    {
        os_free(handle);
    }
}

///////////////////////////////////////////////////////////////////////////////////////
// static function
///////////////////////////////////////////////////////////////////////////////////////
static DISCOVERY_OBJECT * ICACHE_FLASH_ATTR instance(void)
{
    static DISCOVERY_OBJECT *handle = NULL;
    if (NULL == handle)
    {
        handle = (DISCOVERY_OBJECT *)os_malloc(sizeof(DISCOVERY_OBJECT));
        memset(handle, 0, sizeof(DISCOVERY_OBJECT));
    }

    return handle;
}

/******************************************************************************
 * FunctionName : user_devicefind_recv
 * Description  : Processing the received data from the host
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR discovery_recv(void *arg, char *pusrdata, unsigned short length)
{
    int packet_len;
    int8 ip[16]  = { 0 };    
    struct ip_info ipconfig;    
    DISCOVERY_OBJECT *handle = instance();
    if ((NULL == handle) || (NULL == pusrdata))
    {
         return;
    }
    
    /* 收到的消息格式 没有ts  
    {
        "method":"down_msg",
        "req_id":123456789,
        "cmd":"dev_search"
    }
    */
    if (NULL == os_strstr(pusrdata, "dev_search"))
    {
        os_printf("wrong msg format %s \n", pusrdata);
        return;   
    }
    
    /*    
    {
        "method":"up_msg",
        "req_id":123456789,
        "code":0,
        "attribute":
        {
		    "dev_uuid":"01001122334455",
	        "ip":"192.168.10.10",
            "port":60018	
        }
    }
    */
    
#define JSON_DISCOVERY_RESPONSE "{\
\"method\":\"up_msg\",\
\"req_id\":%d,\
\"code\":0,\
\"attribute\":\
{\
\"dev_uuid\":\"%s\",\
\"ip\":\"%s\",\
\"port\":%d\
}\
}"

    // send to local network(staticon).
    memset(&ipconfig, 0, sizeof(struct ip_info));
    wifi_get_ip_info(STATION_IF, &ipconfig);

    memset(ip, 0, sizeof(ip));
    os_sprintf(ip, "%d.%d.%d.%d", IP2STR(&ipconfig.ip));

    //os_printf("discovery: ip \n");
    //os_printf(ip);
    
    if (os_strlen(ip) > 0)
    {
        os_memset(handle->out_buf, 0, MAX_RECV_BUF_LEN);
        os_sprintf(handle->out_buf, JSON_DISCOVERY_RESPONSE, (int)rand(), handle->dev_uuid, ip, handle->port);
        
        /* 不需要加密 */
        packet_len = strlen(handle->out_buf);
        if (packet_len > 0)
        {
            espconn_sent(&handle->udp_conn, handle->out_buf, packet_len);
        }
        
    }

    // send to local network(ap mode).
    memset(&ipconfig, 0, sizeof(struct ip_info));
    wifi_get_ip_info(SOFTAP_IF, &ipconfig); //

    memset(ip, 0, sizeof(ip));
    os_sprintf(ip, "%d.%d.%d.%d", IP2STR(&ipconfig.ip));

    if (os_strlen(ip) > 0)
    {
        memset(handle->out_buf, 0, MAX_RECV_BUF_LEN);
        os_sprintf(handle->out_buf, JSON_DISCOVERY_RESPONSE, (int)rand(), handle->dev_uuid, ip, handle->port);
        
        /* 不需要加密 */
        packet_len = strlen(handle->out_buf);
        if (packet_len > 0)
        {
            espconn_sent(&handle->udp_conn, handle->out_buf, packet_len);
        }
    }
}
