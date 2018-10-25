#include <stdio.h>
#include <string.h>

#include "../core/core.h"
#include "upgrade.h"
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

    os_timer_t      dns_find_timer;
    os_timer_t      register_timer; // contain keep alive.
    ip_addr_t       proxy_server_ip;
    boolean         got_dns_ip;

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
LOCAL void ICACHE_FLASH_ATTR user_dns_found(const char *name, ip_addr_t *ipaddr, void *arg);
LOCAL void ICACHE_FLASH_ATTR user_dns_check_cb(void *arg);
LOCAL void ICACHE_FLASH_ATTR dns_to_ip_start(void);
LOCAL void ICACHE_FLASH_ATTR user_esp_platform_upgrade_rsp(void *arg);
LOCAL void ICACHE_FLASH_ATTR user_esp_platform_upgrade_begin(struct espconn *pespconn, struct upgrade_server_info *server);


#define HEAD_BUFFER "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \r\n\
Accept: */*\r\n\
Authorization: token %s\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"

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


/******************************************************************************
 * FunctionName : user_dns_found
 * Description  : dns found callback
 * Parameters   : name -- pointer to the name that was looked up.
 *                ipaddr -- pointer to an ip_addr_t containing the IP address of
 *                the hostname, or NULL if the name could not be found (or on any
 *                other error).
 *                callback_arg -- a user-specified callback argument passed to
 *                dns_gethostbyname
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR user_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    TCP_CLIENT_OBJECT *handle = instance();

    struct espconn *pespconn = (struct espconn *)arg;

    if (NULL == ipaddr)
    {
        os_printf("user_dns_found NULL \r\n");
        return;
    }

    // dns got ip
    os_printf("user_dns_found %d.%d.%d.%d \r\n",
                *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
                    *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

    if ((0 == handle->proxy_server_ip.addr) && (0 != ipaddr->addr))
    {
        // dns succeed, create tcp connection
        os_timer_disarm(&handle->dns_find_timer);
        handle->proxy_server_ip.addr = ipaddr->addr;
        os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);    // remote ip of tcp server which get by dns
        pespconn->proto.tcp->remote_port = TCP_REMOTE_SERVER_PORT;      // remote port of tcp server
        pespconn->proto.tcp->local_port  = espconn_port();               // local port of ESP8266

        // connect_proxy_server();

        os_printf("start tcp connect to ... %d.%d.%d.%d \r\n",
                    *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
                        *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));
        handle->got_dns_ip = 1;
    }
}

/*******************************************************************************
 * FunctionName : user_esp_platform_dns_check_cb
 * Description  : 1s time callback to check dns found
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR user_dns_check_cb(void *arg)
{
    struct espconn *pespconn = arg;

    TCP_CLIENT_OBJECT *handle = instance();

    espconn_gethostbyname(pespconn, TCP_IOT_SERVER_URL, &handle->proxy_server_ip, user_dns_found); // recall DNS function

    os_printf("dns_check\n");
    os_timer_arm(&handle->dns_find_timer, 1000, 0);
}


LOCAL void ICACHE_FLASH_ATTR dns_to_ip_start(void)
{
    TCP_CLIENT_OBJECT *handle = instance();

    // Connect to tcp server as TCP_IOT_SERVER_URL
    handle->proxy_svr_conn.proto.tcp = &handle->proxy_user_tcp;
    handle->proxy_svr_conn.type   = ESPCONN_TCP;
    handle->proxy_svr_conn.state  = ESPCONN_NONE;

    handle->proxy_server_ip.addr  = 0;

    espconn_gethostbyname(&handle->proxy_svr_conn, TCP_IOT_SERVER_URL, &handle->proxy_server_ip, user_dns_found); // DNS function

    os_timer_disarm(&handle->dns_find_timer);
    os_timer_setfn(&handle->dns_find_timer, (os_timer_func_t *)user_dns_check_cb, handle->proxy_svr_conn);
    os_timer_arm(&handle->dns_find_timer, 1000, 0);
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

    handle->proxy_svr_conn.proto.tcp->local_port  = espconn_port();               // local port of ESP8266
    handle->proxy_svr_conn.proto.tcp->remote_port = TCP_REMOTE_SERVER_PORT;

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
    if ((status != STATION_GOT_IP) || (0 == handle->got_dns_ip))
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

        //struct upgrade_server_info *server = NULL;

        //        server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
        //        os_memcpy(server->upgrade_version, pstr + 12, 16);
        //        server->upgrade_version[15] = '\0';
        //        os_sprintf(server->pre_version,"%s%d.%d.%dt%d(%s)",VERSION_TYPE,IOT_VERSION_MAJOR,\
        //            	IOT_VERSION_MINOR,IOT_VERSION_REVISION,device_type,UPGRADE_FALG);
        //        user_esp_platform_upgrade_begin(pespconn, server);

    }
}


/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_cb
 * Description  : Processing the downloaded data from the server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR user_esp_platform_upgrade_rsp(void *arg)
{
    struct upgrade_server_info *server = arg;
    struct espconn *pespconn = server->pespconn;
    // uint8 devkey[41] = {0};
    uint8 *pbuf = NULL;
    char *action = NULL;
    // os_memcpy(devkey, esp_param.devkey, 40);

    pbuf = (char *)os_zalloc(packet_size);  // 2k

    if (server->upgrade_flag == true)
    {
        os_printf("user_esp_platform_upgarde_successfully\n");

        //action = "device_upgrade_success";
        //os_sprintf(pbuf, UPGRADE_FRAME, devkey, action, server->pre_version, server->upgrade_version);
        //ESP_DBG("%s\n",pbuf);

#ifdef CLIENT_SSL_ENABLE
        espconn_secure_sent(pespconn, pbuf, os_strlen(pbuf));
#else
        espconn_sent(pespconn, pbuf, os_strlen(pbuf));
#endif

        if (pbuf != NULL)
        {
            os_free(pbuf);
            pbuf = NULL;
        }
    }
    else
    {
        os_printf("user_esp_platform_upgrade_failed\n");

        //action = "device_upgrade_failed";
        //os_sprintf(pbuf, UPGRADE_FRAME, devkey, action,server->pre_version, server->upgrade_version);
        //ESP_DBG("%s\n",pbuf);

#ifdef CLIENT_SSL_ENABLE
        espconn_secure_sent(pespconn, pbuf, os_strlen(pbuf));
#else
        espconn_sent(pespconn, pbuf, os_strlen(pbuf));
#endif

        if (pbuf != NULL)
        {
            os_free(pbuf);
            pbuf = NULL;
        }
    }

    os_free(server->url);
    server->url = NULL;
    os_free(server);
    server = NULL;
}

/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_begin
 * Description  : Processing the received data from the server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 *                server -- upgrade param
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR user_esp_platform_upgrade_begin(struct espconn *pespconn, struct upgrade_server_info *server)
{
    uint8 user_bin[10] = {0};
    server->pespconn = pespconn;
    os_memcpy(server->ip, pespconn->proto.tcp->remote_ip, 4);

#ifdef UPGRADE_SSL_ENABLE
    server->port = 443;
#else
    server->port = 80;
#endif

    server->check_cb    = user_esp_platform_upgrade_rsp;
    server->check_times = 120000;

    if (server->url == NULL)
    {
        server->url = (uint8 *)os_zalloc(512);
    }

    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
    {
        os_memcpy(user_bin, "user2.bin", 10);
    }
    else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    {
        os_memcpy(user_bin, "user1.bin", 10);
    }

    os_sprintf(server->url, "GET /%sfilename=%s HTTP/1.0\r\nHost: "IPSTR":%d\r\n"HEAD_BUFFER"",
               server->upgrade_version, user_bin, IP2STR(server->ip),
               server->port);
    os_printf("%s\n", server->url);

#ifdef UPGRADE_SSL_ENABLE
    if (system_upgrade_start_ssl(server) == false)
    {
#else
    if (system_upgrade_start(server) == false)
    {
#endif
        os_printf("upgrade is already started\n");
    }
}
#endif

