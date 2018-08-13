#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user_interface.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"
#include "driver/uart.h"
#include "../core/mem_mgr.h"
#include "../device/flash_param.h"

#include "app_cc.h"

#define MAX_SUB_DEV_COUNT   4

typedef struct _SUB_DEV_PARAM
{
    char dev_uuid[16];
    int  status;        /* online-1 offline-0 */
    int  switch;        /* on-1 off-0 */
} SUB_DEV_PARAM;

typedef struct _APP_CC_OBJECT
{
    int req_id;
    SUB_DEV_PARAM dev_param[MAX_SUB_DEV_COUNT];
} APP_CC_OBJECT;

#define GET_DEV_LIST_RESP "{\
\"method\":\"up_msg\",\
\"req_id\":%d,\
\"code\":0,\
\"attribute\":
{\
\"dev1\":\
{
\"dev_uuid\":\"%s",\
\"status\":\"%s\",\
\"switch\":\"%s\",\
}\
\"dev2\":\
{\
\"dev_uuid\":\"%s\",\
\"status\":\"%s\",\
\"switch\":\"on\",\
}\
\"dev3\":\
{\
\"dev_uuid\":\"%s\", \
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
}


static APP_CC_OBJECT * instance(void);
static void ICACHE_FLASH_ATTR recv_data_callback(void *arg, char *buffer, unsigned short length);

int ICACHE_FLASH_ATTR app_cc_create(void)
{
    int i;
    APP_CC_OBJECT * handle = instance();

    for (i=0; i<MAX_SUB_DEV_COUNT; i++)
    {
        // read from flash.
        // handle->dev_uuid[i];
    }
    os_printf("[app_cc] init ok \n");

    return TRUE;
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
static APP_CC_OBJECT * instance( void )
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
                jsonparse_copy_value(parser, buffer, sizeof(buffer));
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
                jsonparse_copy_value(parser, buffer, sizeof(buffer));
                os_printf("cmd = %s \n", buffer);

                char * resp_buf = (char *)os_malloc(512);
                if (NULL != resp_buf)
                {
                    os_sprintf(resp_buf, GET_DEV_LIST_RESP, handle->req_id,
                                            dev_param[0].dev_uuid,
                                                (1 == dev_param[0].dev_uuid) ? "online" : "offline",
                                                    (1 == dev_param[0].switch) ? "on" : "off",
                                            dev_param[1].dev_uuid,
                                                (1 == dev_param[1].dev_uuid) ? "online" : "offline",
                                                    (1 == dev_param[1].switch) ? "on" : "off",
                                            dev_param[2].dev_uuid,
                                                (1 == dev_param[2].dev_uuid) ? "online" : "offline",
                                                    (1 == dev_param[2].switch) ? "on" : "off",
                                            dev_param[3].dev_uuid,
                                                (1 == dev_param[3].dev_uuid) ? "online" : "offline",
                                                    (1 == dev_param[3].switch) ? "on" : "off");
                    /* 发送到客户端 */
                    os_printf("get resp %s len %d \n", resp_buf, strlen(resp_buf));
                    os_free(resp_buf);
                }
            }
        }
    }
    return 0;
}

struct jsontree_callback msg_callback = JSONTREE_CALLBACK(NULL, msg_parse);

JSONTREE_OBJECT(msg_tree_sub,
                JSONTREE_PAIR("cmd",  &msg_callback),);

JSONTREE_OBJECT(msg_tree, JSONTREE_PAIR("method", NULL),
                          JSONTREE_PAIR("req_id", NULL),
                          JSONTREE_PAIR("attr",  &msg_tree_sub));

static void ICACHE_FLASH_ATTR recv_data_callback(void *arg, char *buffer, unsigned short length)
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
