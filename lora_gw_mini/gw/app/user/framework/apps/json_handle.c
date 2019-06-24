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
#include "../device/flash_param.h"

#include "json_handle.h"

static json_msg_cb  msg_cb = NULL;
static void *       msg_arg = NULL;

static int  req_id = 0;
static char cur_dev_uuid[CC_MAC_LEN+1];
LOCAL void ICACHE_FLASH_ATTR recv_data_callback(void *arg, char *buffer, int length);
LOCAL int ICACHE_FLASH_ATTR msg_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parse);

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
            if (jsonparse_strcmp_value(parse, "method") == 0)
            {
                int version=0;
                jsonparse_next(parse);
                jsonparse_next(parse);
                os_memset(buffer, 0, sizeof(buffer));
                jsonparse_copy_value(parse, buffer, sizeof(buffer));
                os_printf("method = %s \n", buffer);
            }
            else if(jsonparse_strcmp_value(parse, "req_id") == 0)
            {
                int req_id = 0;
                jsonparse_next(parse);
                jsonparse_next(parse);
                req_id = jsonparse_get_value_as_int(parse);
                if ((10000 == req_id) && (NULL != msg_cb))
                {
                    msg_cb(msg_arg, E_REGISTER_RESP, req_id, 0, NULL);
                    os_printf("register response!\n");
                }
                else if ((10001 == req_id) && (NULL != msg_cb))
                {
                    msg_cb(msg_arg, E_HEART_BEAT_RESP, req_id, 0, NULL);
                    os_printf("heart beat response!\n");
                }
                else if ((10002 == req_id) && (NULL != msg_cb))
                {
                    msg_cb(msg_arg, E_FW_UPGRADE_CMD, req_id, 0, NULL);
                    os_printf("upgrade fw response!\n");
                }
                os_printf("req_id = %d \n", req_id);
            }
            else if(jsonparse_strcmp_value(parse, "cmd") == 0)
            {
                int req_id = 0;
                jsonparse_next(parse);
                jsonparse_next(parse);
                jsonparse_copy_value(parse, buffer, sizeof(buffer));
                os_printf("cmd = %s \n", buffer);

                if (0 == os_strcmp("get_param", buffer) && (NULL != msg_cb))
                {
                    msg_cb(msg_arg, E_GET_SUB_DEV_CMD, req_id, 1, cur_dev_uuid);
                }
                else if ((0 == os_strcmp("set_switch", buffer)) && (NULL != msg_cb))
                {
                    int req_id = 0;
                    jsonparse_next(parse);
                    jsonparse_next(parse);
                    os_memset(buffer, 0, sizeof(buffer));
                    jsonparse_copy_value(parse, buffer, sizeof(buffer));
                    if (os_strcmp("buffer", "on"))
                    {
                        msg_cb(msg_arg, E_SET_SWITCH_CMD, req_id, 1, cur_dev_uuid);
                    }
                    else
                    {
                        msg_cb(msg_arg, E_SET_SWITCH_CMD, req_id, 0, cur_dev_uuid);
                    }
                    os_printf("set_switch %s to %s \n", cur_dev_uuid, buffer);
                }
            }
            else if(jsonparse_strcmp_value(parse, "dev_uuid") == 0)
            {
                jsonparse_next(parse);
                jsonparse_next(parse);
                os_memset(cur_dev_uuid, 0, sizeof(cur_dev_uuid));
                jsonparse_copy_value(parse, cur_dev_uuid, sizeof(cur_dev_uuid));
                os_printf("cur uuid = %s \n", cur_dev_uuid);
            }
        }
    }

    return 0;
}

struct jsontree_callback msg_callback_fn = JSONTREE_CALLBACK(NULL, msg_parse);

JSONTREE_OBJECT(msg_tree_sub,
                JSONTREE_PAIR("dev_uuid",  NULL),
                JSONTREE_PAIR("cmd",  &msg_callback_fn),);

JSONTREE_OBJECT(msg_tree, JSONTREE_PAIR("dev_uuid", NULL),
                          JSONTREE_PAIR("method", NULL),
                          JSONTREE_PAIR("req_id", NULL),
                          JSONTREE_PAIR("attr",  &msg_tree_sub));

/* global function. */
void ICACHE_FLASH_ATTR json_handle_handle_data(char *buffer, int length)
{
    os_printf("receive len:%d msg:%s \n", length, buffer);

    struct jsontree_context js;
    jsontree_setup(&js, (struct jsontree_value *)&msg_tree, json_putchar);
    json_parse(&js, buffer);
}

void ICACHE_FLASH_ATTR json_handle_set_callback(json_msg_cb cb, void * arg)
{
    msg_cb  = cb;
    msg_arg = arg;
}
