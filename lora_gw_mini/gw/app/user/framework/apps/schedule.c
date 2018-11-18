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
#include "user_json.h"
#include "gw_io.h"

#include "../core/mem_mgr.h"
#include "../device/flash_param.h"
#include "../device/sx1276.h"
#include "../device/sx1276_hal.h"
#include "../network/wifi.h"
#include "../network/discovery.h"
#include "../crypto/crypto_api.h"
#include "../crypto/packet.h"
#include "../tcp/tcp_server.h"
#include "delay.h"
#include "schedule.h"

#define MAX_DEV_COUNT   4
#define DEV_UUID_LEN    16 /* */


typedef struct _DEV_PARAM
{
    char dev_uuid[DEV_UUID_LEN];
    int  status;        /* online-1 offline-0 */
    int  on_off;        /* on-1 off-0 */
    int  alive_sec;     /* */
} DEV_PARAM;

typedef struct _SCHEDULE_OBJECT
{
    // system timer.
    uint32     sys_sec;
    os_timer_t sys_timer;

    int8 mac[16];

    char cur_dev_uuid[DEV_UUID_LEN];
    DEV_PARAM dev_param[MAX_DEV_COUNT];

    char dev_uuid[MAX_DEV_COUNT][16];

    // 3 secs off - 100 ms on - ok
    // 100 ms off - 100 ms on - auth failed.
    os_timer_t net_led_timer;

    uint8 net_led_status;
    uint8 net_led_status_count;
    uint8 net_wifi_status;          // 0 auth failed, 1 success.
    uint8 switch_level;
    uint8 off_count;

    char send_buf[256];
    char tmp_buf[264];

    // key press.
    struct keys_param keys;
    struct single_key_param *single_key[PLUG_KEY_NUM];

} SCHEDULE_OBJECT;

static SCHEDULE_OBJECT * instance(void);
static void ICACHE_FLASH_ATTR schedule_proc_task(os_event_t *events);
static void ICACHE_FLASH_ATTR system_timer_center(void *arg);
static void ICACHE_FLASH_ATTR net_led_center(void *arg);
static void ICACHE_FLASH_ATTR key_long_press(void);
static void ICACHE_FLASH_ATTR key_short_press(void);
static void ICACHE_FLASH_ATTR tcp_recv_data_callback(void *arg, char *buffer, unsigned short length);
static void ICACHE_FLASH_ATTR recv_data_fn(char *buffer, unsigned short len);


boolean ICACHE_FLASH_ATTR schedule_create(uint16 smart_config)
{
    SCHEDULE_OBJECT * handle = instance();

    gw_io_init();

    // gw_io_status_output(1);

    handle->single_key[0] = key_init_single(GW_KEY_0_IO_NUM,
                                            GW_KEY_0_IO_MUX,
                                            GW_KEY_0_IO_FUNC,
                                            key_long_press,
                                            key_short_press);
    handle->keys.key_num    = PLUG_KEY_NUM;
    handle->keys.single_key = handle->single_key;
    key_init(&handle->keys);
    os_printf("key_init config .... \n");

    gw_io_sx1278_rst_output(1);
    os_printf("gw_io_sx1278_rst_output config .... \n");

    sx1276_hal_set_recv_cb(recv_data_fn);

    sx1276_hal_register_rf_func();

    sx1276_hal_reset();

    sx1276_hal_lora_init();

    os_printf("finish config .... \n");

    int i;
    for (i=0; i<MAX_DEV_COUNT; i++)
    {
        os_memset(handle->dev_param[i].dev_uuid, 0, sizeof(handle->dev_param[i].dev_uuid));
        flash_param_get_dev_uuid(i, handle->dev_param[i].dev_uuid, sizeof(handle->dev_param[i].dev_uuid));
    }

    crypto_api_cbc_set_key(KEY_PASSWORD, strlen(KEY_PASSWORD));
    //sx1276_hal_set_recv_cb(recv_data_fn);

    os_timer_disarm(&handle->sys_timer);
    os_timer_setfn(&handle->sys_timer, (os_timer_func_t *)system_timer_center, handle);
    os_timer_arm(&handle->sys_timer, 3000, 1); // 0 at once, 1 restart auto.

    return 0;

#if 0
    os_timer_disarm(&handle->net_led_timer);
    os_timer_setfn(&handle->net_led_timer, (os_timer_func_t *)net_led_center, handle);
    os_timer_arm(&handle->net_led_timer, 100, 1);
#endif

    //tcp_client_create();
    //tcp_client_set_callback(tcp_recv_data_callback, handle);

    char mac[24] = {0};
    wifi_get_macaddr(STATION_IF, mac);
    memset(handle->mac, 0, sizeof(handle->mac));
  	os_sprintf(handle->mac, MACSTR, MAC2STR(mac));

    int j;
    for (i=0,j=0; i<12;)
    {
        handle->mac[i]   = handle->mac[j];
        handle->mac[i+1] = handle->mac[j+1];
        i += 2;
        j += 3;
    }
    handle->mac[12] = 0;

#if 0
    DISCOVER_ENV dis_env;
    dis_env.port = TCP_BIND_PORT;
    os_memset(&dis_env, 0, sizeof(DISCOVER_ENV));
    os_sprintf(dis_env.dev_uuid, "01%s", handle->mac); /* 01琛ㄧず涓帶 */
    if (0 != discovery_create(&dis_env))
    {
        os_printf("dis failed \n");
        return -1;
    }
#endif

    os_printf("[sch] init ok \n");

    return TRUE;
}

void ICACHE_FLASH_ATTR schedule_destroy(void)
{
    SCHEDULE_OBJECT * handle = instance();
    if (NULL != handle)
    {
        tcp_client_destroy();
        os_free(handle);
    }
}

//////////////////////////////////////////////////////////////////////////////////
// static function.
//////////////////////////////////////////////////////////////////////////////////
static SCHEDULE_OBJECT * instance( void )
{
    static SCHEDULE_OBJECT *handle = NULL;
    if (NULL == handle)
    {
        handle = (SCHEDULE_OBJECT *)os_zalloc(sizeof(SCHEDULE_OBJECT));
    }

    return handle;
}

static void ICACHE_FLASH_ATTR tcp_recv_data_callback(void *arg, char *buffer, unsigned short length)
{
    SCHEDULE_OBJECT * handle = (SCHEDULE_OBJECT *)arg;
    if (NULL == handle)
    {
        os_printf("invalid param \n");
        return;
    }

    os_printf("receive len:%d msg:%s \n", length, buffer);
}

#define SYNC_TIME "{\
\"method\":\"down_msg\",\
\"dev_uuid\":\"%s\",\
\"req_id\":%d,\
\"attr\":\
{\
\"cmd\":\"set_time\":\
{\
\"ts\":%d\
}\
}\
}"

static void system_timer_center( void *arg )
{
    SCHEDULE_OBJECT * handle = ( SCHEDULE_OBJECT * )arg;

    // os_printf("get count %d \n", handle->sys_sec);
    handle->sys_sec++;

    static int flags = 0;

    // gw_io_wifi_output(flags);

    flags = (0 == flags) ? 1 : 0;

    char buffer[8] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02};

    sx1276_hal_rf_send_packet(buffer, sizeof(buffer));

    // sx1276_hal_rx_mode();   // 设置为接收模式

    int i;
    for(i=0; i<100; i++)
    {

    }

    // sx1278_recv_handle();

    return;


    if (0 == handle->sys_sec++ % 60)
    {
        int i;
        int req_id = (int)rand();
        for (i=0; i<MAX_DEV_COUNT; i++)
        {
            os_memset(handle->send_buf, 0, sizeof(handle->send_buf));
            os_sprintf(handle->send_buf, SYNC_TIME, handle->dev_param[i].dev_uuid, req_id, handle->sys_sec);

            int out_len = sizeof(handle->tmp_buf);
            if (0 == packet_enc(handle->send_buf, strlen(handle->send_buf), handle->tmp_buf, &out_len))
            {
                crypto_api_encrypt_buffer(handle->tmp_buf, out_len);
                sx1276_hal_rf_send_packet(handle->tmp_buf, (unsigned char)out_len);
            }

            if (handle->dev_param[i].alive_sec++ > 120)
            {
                /* 寮�鍏崇姸鎬�-1 鏈煡 */
                app_cc_set_param(handle->dev_param[i].dev_uuid, 0, -1);
            }
        }
    }

    // check wifi status.
    static uint32 wifi_check = 0;
    if (wifi_check++ >= 2)
    {
        uint8 wifi_status = wifi_station_get_connect_status();
        if (STATION_GOT_IP == wifi_status)
        {
            handle->net_wifi_status = 1;
            wifi_check = 0;
        }
        else
        {
            handle->net_wifi_status = 0;
            wifi_check = 2;
        }
    }
}

static void net_led_center(void *arg)
{
    SCHEDULE_OBJECT * handle = (SCHEDULE_OBJECT *)arg;
    if (0 == handle->net_wifi_status)
    {
        handle->net_led_status = (0 == handle->net_led_status) ? 1 : 0;
        // set led on/off here.
        gw_io_wifi_output(handle->net_led_status);
    }
    else
    {
        if (handle->net_led_status_count++ < 30)
        {
            handle->net_led_status = 1;
        }
        else
        {
            handle->net_led_status_count = 0;
            handle->net_led_status = 0;
        }
        // set led on/off here.
        gw_io_wifi_output(handle->net_led_status);
    }
}

/******************************************************************************
 * FunctionName : key_short_press
 * Description  : key's short press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
#define SET_MATCH_MSG  "{\
\"method\":\"down_msg\",\
\"dev_uuid\":\"FFFFFFFFFFFF\",\
\"req_id\":%d,\
\"ts\":%d,\
\"attr\":\
{\
\"cmd\":\"set_match\":\
{\
}\
}\
}"

static void ICACHE_FLASH_ATTR key_short_press( void )
{
    int i;
    SCHEDULE_OBJECT * handle = instance();
    if (NULL == handle)
    {
        os_printf("invalid param (not enough memory) \n");
        return;
    }

    os_printf("short press \n");

    static int flags = 0;

    // gw_io_status_output(flags);

    flags = (0 == flags) ? 1 : 0;

    return;

    int req_id = (int)rand();
    for (i=0; i<MAX_DEV_COUNT; i++)
    {
        os_memset(handle->send_buf, 0, sizeof(handle->send_buf));
        os_sprintf(handle->send_buf, SET_MATCH_MSG, handle->dev_uuid[i], req_id, handle->sys_sec);

        int out_len = sizeof(handle->tmp_buf);

        if (0 == packet_enc(handle->send_buf, sizeof(handle->send_buf), handle->tmp_buf, &out_len))
        {
            crypto_api_encrypt_buffer(handle->tmp_buf, out_len);

            sx1276_hal_rf_send_packet(handle->tmp_buf, (unsigned char)out_len);
        }
    }
}

/******************************************************************************
 * FunctionName : key_long_press
 * Description  : key's long press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
static void ICACHE_FLASH_ATTR key_long_press( void )
{
    os_printf("long press \n");

    //flash_param_default_restore();
    //user_esp_platform_set_active(0);
    //flash_param_set_id(CONFIG_RESET_ID);

    //system_restore();
    //system_restart();
}


/***************************************************************************************************
* static function.
***************************************************************************************************/
LOCAL int ICACHE_FLASH_ATTR msg_parse(struct jsontree_context *js_ctx, struct jsonparse_state *parse)
{
    int  type;
    char buffer[16] = {0};
    SCHEDULE_OBJECT * handle = instance();
    if (NULL == handle)
    {
        os_printf("invalid param \n");
        return -1;
    }
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
            else if(jsonparse_strcmp_value(parse, "dev_uuid") == 0)
            {
                jsonparse_next(parse);
                jsonparse_next(parse);

                os_memset(buffer, 0, sizeof(buffer));
                jsonparse_copy_value(parse, buffer, sizeof(handle->cur_dev_uuid));

                int i;
                for (i=0; i<MAX_DEV_COUNT; i++)
                {
                    if (0 == os_strcmp(buffer, handle->dev_param[i].dev_uuid))
                    {
                        handle->dev_param[i].status = 1;
                        handle->dev_param[i].alive_sec = 0;
                        os_memset(handle->cur_dev_uuid, 0, sizeof(handle->cur_dev_uuid));
                        os_memcpy(handle->cur_dev_uuid, handle->dev_param[i].dev_uuid, sizeof(handle->cur_dev_uuid));
                        break;
                    }
                }
            }
            else if(jsonparse_strcmp_value(parse, "switch") == 0)
            {
                jsonparse_next(parse);
                jsonparse_next(parse);
                os_memset(buffer, 0, sizeof(buffer));
                jsonparse_copy_value(parse, buffer, sizeof(buffer));

                int i;
                for (i=0; i<MAX_DEV_COUNT; i++)
                {
                    if (0 == os_strcmp(handle->cur_dev_uuid, handle->dev_param[i].dev_uuid))
                    {
                        if (0 == os_strcmp(buffer, "on"))
                        {
                            handle->dev_param[i].on_off = 1;
                        }
                        else
                        {
                            handle->dev_param[i].on_off = 0;
                        }
                        app_cc_set_param(handle->dev_param[i].dev_uuid, 1, handle->dev_param[i].on_off);
                        break;
                    }
                }
            }
        }
    }

    return 0;
}

struct jsontree_callback msg_callback = JSONTREE_CALLBACK(NULL, msg_parse);

JSONTREE_OBJECT(msg_tree_sub,
                JSONTREE_PAIR("switch",  &msg_callback),);

JSONTREE_OBJECT(msg_tree, JSONTREE_PAIR("dev_uuid", NULL),
                          JSONTREE_PAIR("method", NULL),
                          JSONTREE_PAIR("attr",  &msg_tree_sub));

static void ICACHE_FLASH_ATTR recv_data_fn(char *buffer, unsigned short len)
{
    os_printf("recv from sx1278 [%s] len %d \n", buffer, len);

    SCHEDULE_OBJECT * handle = instance();
    if (NULL == handle)
    {
        os_printf("invalid param \n");
        return;
    }
    if (len > 0)
    {
        crypto_api_decrypt_buffer(buffer, len);

        int out_len = sizeof(handle->tmp_buf);
        if (0 == packet_dec(buffer, len, handle->tmp_buf, &out_len))
        {
            os_printf("get data from 1278 is %s \n", handle->tmp_buf);
            struct jsontree_context js;
            jsontree_setup(&js, (struct jsontree_value *)&msg_tree, json_putchar);
            json_parse(&js, handle->tmp_buf);

            tcp_server_send_msg(handle->tmp_buf, (int)out_len);
        }
    }
}
