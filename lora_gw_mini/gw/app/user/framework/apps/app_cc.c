#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "user_interface.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"
#include "user_json.h"
#include "../tcp/tcp_server.h"
#include "../device/flash_param.h"

#include "app_cc.h"

#define MAX_SUB_DEV_COUNT   4

typedef struct _SUB_DEV_PARAM
{
    char dev_uuid[16];
    int  status;        /* online-1 offline-0 */
    int  on_off;        /* on-1 off-0 */
} SUB_DEV_PARAM;

typedef struct _APP_CC_OBJECT
{
    int req_id;
    SUB_DEV_PARAM dev_param[MAX_SUB_DEV_COUNT];
    char cur_dev_uuid[16];
} APP_CC_OBJECT;

#define UP_MSG_RESP "{\
\"method\":\"up_msg\",\
\"dev_uuid\":\"%s\",\
\"req_id\":%d,\
\"code\":0\
}"

#define GET_DEV_LIST_RESP "{\
\"method\":\"up_msg\",\
\"req_id\":%d,\
\"code\":0,\
\"attribute\":\
{\
\"dev1\":\
{\
\"dev_uuid\":\"%s\",\
\"status\":\"%s\",\
\"switch\":\"%s\",\
}\
\"dev2\":\
{\
\"dev_uuid\":\"%s\",\
\"status\":\"%s\",\
\"switch\":\"%s\",\
}\
\"dev3\":\
{\
\"dev_uuid\":\"%s\",\
\"status\":\"%s\",\
\"switch\":\"%s\",\
}\
\"dev4\":\
{\
\"dev_uuid\":\"%s\",\
\"status\":\"%s\",\
\"switch\":\"%s\",\
}\
}\
}"

LOCAL APP_CC_OBJECT * instance(void);
LOCAL void ICACHE_FLASH_ATTR recv_data_callback(void *arg, char *buffer, int length);
LOCAL int ICACHE_FLASH_ATTR msg_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parse);

int ICACHE_FLASH_ATTR app_cc_create(void)
{
    int i;
    APP_CC_OBJECT * handle = instance();

    for (i=0; i<MAX_SUB_DEV_COUNT; i++)
    {
        os_memset(handle->dev_param[i].dev_uuid, 0, sizeof(handle->dev_param[i].dev_uuid));
        flash_param_get_dev_uuid(i, handle->dev_param[i].dev_uuid, sizeof(handle->dev_param[i].dev_uuid));
        handle->dev_param[i].status = 0;
        handle->dev_param[i].on_off = 0;
    }
    os_printf("[app_cc] init ok \n");

    if (0 != tcp_server_create())
    {
        os_printf("tcp svr failed \n");
        return -1;
    }
    tcp_server_set_callback((void *)handle, recv_data_callback);

    return 0;
}

void ICACHE_FLASH_ATTR app_cc_set_param(char *dev_uuid, int status, int on_off)
{
    int i;
    APP_CC_OBJECT * handle = instance();
    if (NULL == handle)
    {
        os_printf("invalid param \n");
        return;
    }
    
    for (i=0; i<MAX_SUB_DEV_COUNT; i++)
    {
        if (0 == os_strcmp(dev_uuid, handle->dev_param[i].dev_uuid))
        {
            handle->dev_param[i].status = status;
            handle->dev_param[i].on_off = on_off;
            break;
        }
    }
}

void ICACHE_FLASH_ATTR app_cc_destroy(void)
{
    APP_CC_OBJECT * handle = instance();
    if (NULL != handle)
    {
        tcp_client_destroy();
        os_free(handle);
    }
}

//////////////////////////////////////////////////////////////////////////////////
// static function.
//////////////////////////////////////////////////////////////////////////////////
LOCAL APP_CC_OBJECT * instance( void )
{
    static APP_CC_OBJECT *handle = NULL;
    if (NULL == handle)
    {
        handle = (APP_CC_OBJECT *)os_zalloc(sizeof(APP_CC_OBJECT));
    }

    return handle;
}

/***************************************************************************************************
* static function.
***************************************************************************************************/
LOCAL int ICACHE_FLASH_ATTR msg_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parse)
{
    int type;
    char buffer[16] = {0};
    APP_CC_OBJECT * handle = instance();
    if (NULL == handle)
    {
        os_printf("invalid param \n");
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
                os_memset(buffer, 0, sizeof(buffer));
                jsonparse_copy_value(parse, buffer, sizeof(buffer));
                os_printf("method = %s \n", buffer);
            }
            else if(jsonparse_strcmp_value(parse,"req_id") == 0)
            {
                int req_id = 0;
                jsonparse_next(parse);
                jsonparse_next(parse);
                req_id = jsonparse_get_value_as_int(parse);
                handle->req_id = req_id;
                os_printf("req_id = %d \n", req_id);
            }
            else if(jsonparse_strcmp_value(parse,"cmd") == 0)
            {
                int req_id = 0;
                jsonparse_next(parse);
                jsonparse_next(parse);
                jsonparse_copy_value(parse, buffer, sizeof(buffer));
                os_printf("cmd = %s \n", buffer);
                
                if (0 == os_strcmp("get_dev_list", buffer))
                {
                    char * resp_buf = (char *)os_malloc(512);
                    if (NULL != resp_buf)
                    {
                        os_sprintf(resp_buf, GET_DEV_LIST_RESP, handle->req_id,
                                                handle->dev_param[0].dev_uuid,
                                                    (1 == handle->dev_param[0].status) ? "online" : "offline",
                                                        (1 == handle->dev_param[0].on_off) ? "on" : "off",
                                                handle->dev_param[1].dev_uuid,
                                                    (1 == handle->dev_param[1].status) ? "online" : "offline",
                                                        (1 == handle->dev_param[1].on_off) ? "on" : "off",
                                                handle->dev_param[2].dev_uuid,
                                                    (1 == handle->dev_param[2].status) ? "online" : "offline",
                                                        (1 == handle->dev_param[2].on_off) ? "on" : "off",
                                                handle->dev_param[3].dev_uuid,
                                                    (1 == handle->dev_param[3].status) ? "online" : "offline",
                                                        (1 == handle->dev_param[3].on_off) ? "on" : "off");
                        tcp_server_send_msg(resp_buf, strlen(resp_buf));
                        /* 发送到客户端 */
                        os_printf("get resp %s len %d \n", resp_buf, strlen(resp_buf));
                        os_free(resp_buf);
                    }
                }
                else if ((0 == os_strcmp("set_switch", buffer)) || (0 == os_strcmp("get_all_property", buffer)))
                {
                    /* 找到对应的dev 将消息发送到该设备 */   
                                     
                    char * resp_buf = (char *)os_malloc(128);
                    if (NULL != resp_buf)
                    {
                        os_sprintf(resp_buf, UP_MSG_RESP, handle->cur_dev_uuid, handle->req_id);
                        tcp_server_send_msg(resp_buf, strlen(resp_buf));
                        os_free(resp_buf);
                    }
                }                
            }

            else if(jsonparse_strcmp_value(parse,"dev_uuid") == 0)
            {
                jsonparse_next(parse);
                jsonparse_next(parse);
                os_memset(handle->cur_dev_uuid, 0, sizeof(handle->cur_dev_uuid));
                jsonparse_copy_value(parse, handle->cur_dev_uuid, sizeof(handle->cur_dev_uuid));
                os_printf("cur uuid = %s \n", handle->cur_dev_uuid);
            }
        }
    }
    
    return 0;
}

struct jsontree_callback msg_callback_fn = JSONTREE_CALLBACK(NULL, msg_parse);

JSONTREE_OBJECT(msg_tree_sub,
                JSONTREE_PAIR("cmd",  &msg_callback_fn),);

JSONTREE_OBJECT(msg_tree, JSONTREE_PAIR("dev_uuid", NULL),
                          JSONTREE_PAIR("method", NULL),
                          JSONTREE_PAIR("req_id", NULL),
                          JSONTREE_PAIR("attr",  &msg_tree_sub));

static void ICACHE_FLASH_ATTR recv_data_callback(void *arg, char *buffer, int length)
{
    APP_CC_OBJECT * handle = (APP_CC_OBJECT *)arg;
    if (NULL == handle)
    {
        os_printf("invalid param \n");
        return;
    }

    os_printf("receive len:%d msg:%s \n", length, buffer);

    struct jsontree_context js;
    jsontree_setup(&js, (struct jsontree_value *)&msg_tree, json_putchar);
    json_parse(&js, buffer);
}
