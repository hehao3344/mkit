#include <stdio.h>
#include <string.h>

#include "esp_common.h"
#include "../core/core.h"
#include "espconn.h"

#include "../core/platform.h"
#include "../core/version.h"
#include "../tcp/tcp_client.h"
#include "../apps/prj_config.h"
#include "../apps/json_format.h"

#include "../device/flash_param.h"
#include "tcp_client.h"

#define PACKET_SIZE 2048

typedef struct _TCP_CLIENT_OBJECT
{
    int32   count;
    boolean proxy_is_handle;
    boolean proxy_connected;

    char reg_buf[256];

    char cur_dev_uuid[20];
    int8 mac[40];
    int  req_id;
    struct espconn  proxy_svr_conn;
    struct _esp_tcp proxy_user_tcp;

    os_timer_t      dns_find_timer;
    os_timer_t      register_timer; // contain keep alive.
    ip_addr_t       proxy_server_ip;
    boolean         got_dns_ip;

    tcp_recv_data_callback cb;
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

//LOCAL void ICACHE_FLASH_ATTR user_esp_platform_upgrade_rsp(void *arg);
//LOCAL void ICACHE_FLASH_ATTR user_esp_platform_upgrade_begin(struct espconn *pespconn, struct upgrade_server_info *server);
static void ICACHE_FLASH_ATTR recv_data_callback_fn(void *arg, char *buffer, int length);

#define HEAD_BUFFER "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \r\n\
Accept: */*\r\n\
Authorization: token %s\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"

#define UPGRADE_RESPONSE_JSON   "{\
\"method\":\"up_msg\",\
\"cc_uuid\":\"%S\",\
\"req_id\":%d,\
\"code\":%d\
}"

boolean ICACHE_FLASH_ATTR tcp_client_create(void)
{
    TCP_CLIENT_OBJECT *handle = instance();
    if (NULL == handle)
    {
        printf("tcp dev not enough memory \n");
        return FALSE;
    }

    handle->proxy_svr_conn.proto.tcp = &handle->proxy_user_tcp;
    handle->proxy_svr_conn.type      = ESPCONN_TCP;
    handle->proxy_svr_conn.state     = ESPCONN_NONE;

    char mac[24] = {0};
    wifi_get_macaddr(STATION_IF, mac);

  	sprintf(handle->mac, MACSTR, MAC2STR(mac));

    uint8 i, j;

    for (i=0,j=0; i<12;)
    {
        handle->mac[i]   = handle->mac[j];
        handle->mac[i+1] = handle->mac[j+1];
        i += 2;
        j += 3;
    }
    handle->mac[12] = 0;

    sprintf(handle->reg_buf, DEV_REGISTER_MSG, "10", handle->mac, 10000, VER_MAJOR, VER_MINOR);

    printf("get mac %s %s\n", handle->mac, handle->reg_buf);

    uint32 ip = ipaddr_addr(TCP_SERVER_REMOTE_IP);
    // const char tcp_server_addr[4] = {120, 55, 58, 166};
    //memcpy(handle->proxy_svr_conn.proto.tcp->remote_ip, tcp_server_addr, 4);
    memcpy(handle->proxy_svr_conn.proto.tcp->remote_ip, &ip, 4);

    os_timer_disarm(&handle->register_timer);
    os_timer_setfn(&handle->register_timer, (os_timer_func_t *)register_center, handle);
    os_timer_arm(&handle->register_timer, 1000, 1);

    printf("[tcp client] create ok \n");
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
        printf("user_dns_found NULL \r\n");
        return;
    }

    // dns got ip
    printf("user_dns_found %d.%d.%d.%d \r\n",
                *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
                    *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

    if ((0 == handle->proxy_server_ip.addr) && (0 != ipaddr->addr))
    {
        // dns succeed, create tcp connection
        os_timer_disarm(&handle->dns_find_timer);
        handle->proxy_server_ip.addr = ipaddr->addr;
        memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);    // remote ip of tcp server which get by dns
        pespconn->proto.tcp->remote_port = TCP_REMOTE_SERVER_PORT;      // remote port of tcp server
        pespconn->proto.tcp->local_port  = espconn_port();               // local port of ESP8266

        // connect_proxy_server();

        printf("start tcp connect to ... %d.%d.%d.%d \r\n",
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

    printf("dns_check\n");
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
    os_timer_setfn(&handle->dns_find_timer, (os_timer_func_t *)user_dns_check_cb, &handle->proxy_svr_conn);
    os_timer_arm(&handle->dns_find_timer, 1000, 0);
}

void ICACHE_FLASH_ATTR tcp_client_set_callback(tcp_recv_data_callback cb, void *arg)
{
    TCP_CLIENT_OBJECT *handle = instance();

    handle->cb  = cb;
    handle->arg = arg;
}

void ICACHE_FLASH_ATTR tcp_client_send_msg(uint8 *buffer, uint16 len)
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
        free(handle);
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
        memset(handle, 0, sizeof(TCP_CLIENT_OBJECT));
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
    printf("connectting ... to tcp server \n");
}

////////////////////////////////////////////////////////////////////////////////
LOCAL void ICACHE_FLASH_ATTR proxy_sent_cb(void *arg)
{
    struct espconn *pespconn = arg;
    //printf("proxy_sent_cb\n");
}

LOCAL void ICACHE_FLASH_ATTR proxy_connect_cb(void *arg)
{
    TCP_CLIENT_OBJECT *handle = instance();
    struct espconn * pespconn = arg;
    printf("proxy connect success \n");
    espconn_regist_recvcb(pespconn, proxy_recv);
    espconn_regist_sentcb(pespconn, proxy_sent_cb);
    //espconn_sent(pespconn, (uint8 *)handle->inform_buf, strlen(handle->inform_buf));

    handle->proxy_connected = TRUE;
}

LOCAL void ICACHE_FLASH_ATTR proxy_discon_cb(void *arg)
{
    TCP_CLIENT_OBJECT *handle = instance();

    printf("proxy disconnect \n");
    handle->proxy_is_handle  = FALSE;
    handle->proxy_connected  = FALSE;
    handle->count = 0;
}

LOCAL void ICACHE_FLASH_ATTR proxy_recon_cb(void *arg, sint8 err)
{
    printf("proxy reconn \n");
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
    //if ((status != STATION_GOT_IP) || (0 == handle->got_dns_ip))
    if (status != STATION_GOT_IP)
    {
        printf("waitting for ... \n");
        handle->proxy_is_handle  = FALSE;
        handle->proxy_connected  = FALSE;
        return;
    }

    if (!handle->proxy_is_handle)
    {
        printf("register_center: connecting ... \n");
        handle->proxy_is_handle = TRUE;
        connect_proxy_server();
    }

    if (handle->proxy_connected)
    {
        if (0 == handle->count%15)
        {
            if (strlen(handle->reg_buf) > 0)
            {
                printf("update param %d \n", handle->count);
                espconn_sent(&handle->proxy_svr_conn, (uint8 *)handle->reg_buf, strlen(handle->reg_buf));
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
        printf("tcp recv msg [%s] len %d \n", buffer, length);
        handle->cb(handle->arg, buffer, length);

        //struct upgrade_server_info *server = NULL;
        //        server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
        //        memcpy(server->upgrade_version, pstr + 12, 16);
        //        server->upgrade_version[15] = '\0';
        //        sprintf(server->pre_version,"%s%d.%d.%dt%d(%s)",VERSION_TYPE,IOT_VERSION_MAJOR,\
        //            	IOT_VERSION_MINOR,IOT_VERSION_REVISION,device_type,UPGRADE_FALG);
        //        user_esp_platform_upgrade_begin(pespconn, server);

    }
}
#if 0
#if 0
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
    // memcpy(devkey, esp_param.devkey, 40);

    pbuf = (char *)os_zalloc(PACKET_SIZE);  // 2k

    if (server->upgrade_flag == true)
    {
        printf("user_esp_platform_upgarde_successfully\n");

        //action = "device_upgrade_success";
        //sprintf(pbuf, UPGRADE_FRAME, devkey, action, server->pre_version, server->upgrade_version);
        //ESP_DBG("%s\n",pbuf);

#ifdef CLIENT_SSL_ENABLE
        espconn_secure_sent(pespconn, pbuf, os_strlen(pbuf));
#else
        espconn_sent(pespconn, pbuf, os_strlen(pbuf));
#endif

        if (pbuf != NULL)
        {
            free(pbuf);
            pbuf = NULL;
        }
    }
    else
    {
        printf("user_esp_platform_upgrade_failed\n");

        //action = "device_upgrade_failed";
        //sprintf(pbuf, UPGRADE_FRAME, devkey, action,server->pre_version, server->upgrade_version);
        //ESP_DBG("%s\n",pbuf);

#ifdef CLIENT_SSL_ENABLE
        espconn_secure_sent(pespconn, pbuf, os_strlen(pbuf));
#else
        espconn_sent(pespconn, pbuf, os_strlen(pbuf));
#endif

        if (pbuf != NULL)
        {
            free(pbuf);
            pbuf = NULL;
        }
    }

    free(server->url);
    server->url = NULL;
    free(server);
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
    memcpy(server->ip, pespconn->proto.tcp->remote_ip, 4);

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
        memcpy(user_bin, "user2.bin", 10);
    }
    else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    {
        memcpy(user_bin, "user1.bin", 10);
    }

    sprintf(server->url, "GET /%sfilename=%s HTTP/1.0\r\nHost: "IPSTR":%d\r\n"HEAD_BUFFER"",
               server->upgrade_version, user_bin, IP2STR(server->ip),
               server->port);
    printf("%s\n", server->url);

#ifdef UPGRADE_SSL_ENABLE
    if (system_upgrade_start_ssl(server) == false)
    {
#else
    if (system_upgrade_start(server) == false)
    {
#endif
        printf("upgrade is already started\n");
    }
}


/***************************************************************************************************
* static function.
***************************************************************************************************/
LOCAL int ICACHE_FLASH_ATTR msg_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parse)
{
    int type;
    char buffer[16] = {0};
    TCP_CLIENT_OBJECT *handle = instance();
    if (NULL == handle)
    {
        printf("invalid param \n");
        return -1;
    }
    while ((type = jsonparse_next(parse)) != 0)
    {
        if (type == JSON_TYPE_PAIR_NAME)
        {
            if (jsonparse_strcmp_value(parse,"method") == 0)
            {
                int version=0;
                jsonparse_next(parse);
                jsonparse_next(parse);
                memset(buffer, 0, sizeof(buffer));
                jsonparse_copy_value(parse, buffer, sizeof(buffer));
                printf("method = %s \n", buffer);
            }
            else if(jsonparse_strcmp_value(parse,"req_id") == 0)
            {
                int req_id = 0;
                jsonparse_next(parse);
                jsonparse_next(parse);
                req_id = jsonparse_get_value_as_int(parse);
                handle->req_id = req_id;
                printf("req_id = %d \n", req_id);
            }
            else if(jsonparse_strcmp_value(parse, "cmd") == 0)
            {
                int req_id = 0;
                jsonparse_next(parse);
                jsonparse_next(parse);
                jsonparse_copy_value(parse, buffer, sizeof(buffer));
                printf("cmd = %s \n", buffer);

                if (0 == os_strcmp("fw_upgrade", buffer))
                {
                    // user_esp_platform_upgrade_begin(struct espconn *pespconn, struct upgrade_server_info *server)
                }

            }
            else if(jsonparse_strcmp_value(parse, "version") == 0)
            {
                // 当版本号不一致时启动升级
                struct upgrade_server_info *server = NULL;
                server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
                memcpy(server->upgrade_version, "V2.00", 4);
                server->upgrade_version[15] = '\0';

                //sprintf(server->pre_version,"%s%d.%d.%dt%d(%s)", VERSION_TYPE,IOT_VERSION_MAJOR,
                //    	     IOT_VERSION_MINOR, IOT_VERSION_REVISION, device_type, UPGRADE_FALG);
                user_esp_platform_upgrade_begin(&handle->proxy_svr_conn, server);
            }

            else if(jsonparse_strcmp_value(parse,"dev_uuid") == 0)
            {
                jsonparse_next(parse);
                jsonparse_next(parse);
                memset(handle->cur_dev_uuid, 0, sizeof(handle->cur_dev_uuid));
                jsonparse_copy_value(parse, handle->cur_dev_uuid, sizeof(handle->cur_dev_uuid));
                printf("cur uuid = %s \n", handle->cur_dev_uuid);
            }
        }
    }

    return 0;
}

struct jsontree_callback msg_callback_tcp = JSONTREE_CALLBACK(NULL, msg_parse);

JSONTREE_OBJECT(msg_tree_sub,
                JSONTREE_PAIR("cmd",  &msg_callback_tcp),);

JSONTREE_OBJECT(msg_tree, JSONTREE_PAIR("dev_uuid", NULL),
                          JSONTREE_PAIR("method", NULL),
                          JSONTREE_PAIR("req_id", NULL),
                          JSONTREE_PAIR("attr",  &msg_tree_sub));

static void ICACHE_FLASH_ATTR recv_data_callback_fn(void *arg, char *buffer, int length)
{
    TCP_CLIENT_OBJECT * handle = (TCP_CLIENT_OBJECT *)arg;
    if (NULL == handle)
    {
        printf("invalid param \n");
        return;
    }

    printf("receive len:%d msg:%s \n", length, buffer);

    struct jsontree_context js;
    jsontree_setup(&js, (struct jsontree_value *)&msg_tree, json_putchar);
    json_parse(&js, buffer);
}
#endif
#endif
