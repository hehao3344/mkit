/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"

#include "user_interface.h"
#include "smartconfig.h"
#include "airkiss.h"
#include "driver/uart.h"
#include "user_json.h"
#include "framework/apps/user_plug.h"
#include "framework/apps/smart_config.h"
#include "framework/device/flash_param.h"

#include "framework/crypto/rsa_api.h"

static os_timer_t status_timer;
static uint8      led_status = 0;
static uint32     sys_sec;
static uint8 switch_level;
static uint8 off_count;
static os_timer_t sys_timer;
// static os_event_t proc_task_queue[SCH_PROC_TASK_QUEUE_LEN];
// key press.
static struct keys_param keys;
static struct single_key_param *single_key[PLUG_KEY_NUM];


static void ICACHE_FLASH_ATTR led_status_center(void *arg);
static void ICACHE_FLASH_ATTR system_secs_center(void *arg);
static void ICACHE_FLASH_ATTR key_short_press(void);
static void ICACHE_FLASH_ATTR key_long_press(void);


LOCAL int ICACHE_FLASH_ATTR version_get(struct jsontree_context *js_ctx)
{
    const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);
    char string[32];

    if (os_strncmp(path, "hardware", 8) == 0) {
        os_sprintf(string, "0.1");
    }

    jsontree_write_string(js_ctx, string);

    return 0;
}

LOCAL struct jsontree_callback version_callback =
                                JSONTREE_CALLBACK(version_get, NULL);

JSONTREE_OBJECT(INFOTree,
                JSONTREE_PAIR("info", &version_callback));



LOCAL int ICACHE_FLASH_ATTR msg_set(struct jsontree_context *js_ctx, struct jsonparse_state *parse)
{
    int type;
    while ((type = jsonparse_next(parse)) != 0)
    {
        if(jsonparse_strcmp_value(parse,"v") == 0)
        {
            u8 version=0;
            jsonparse_next(parse);
            jsonparse_next(parse);
            version = jsonparse_get_value_as_int(parse);
            os_printf("version : %d \r\n",version);
        }
    }
    return 0;
}

struct jsontree_callback msg_callback = JSONTREE_CALLBACK(NULL, msg_set);
JSONTREE_OBJECT(msg_tree,
JSONTREE_PAIR("v",&msg_callback));


LOCAL int ICACHE_FLASH_ATTR msg_set2(struct jsontree_context *js_ctx, struct jsonparse_state *parse)
{
    int type;
    while ((type = jsonparse_next(parse)) != 0)
    {
        if(jsonparse_strcmp_value(parse,"v") == 0)
        {
            u8 version=0;
            jsonparse_next(parse);
            jsonparse_next(parse);
            version = jsonparse_get_value_as_int(parse);
            os_printf("version : %d \r\n",version);
        }
    }
    return 0;
}

struct jsontree_callback msg_callback2 = JSONTREE_CALLBACK(NULL, msg_set2);
JSONTREE_OBJECT(msg_tree2, JSONTREE_PAIR("v",&msg_callback2));
JSONTREE_OBJECT(msg_tree_lv2, JSONTREE_PAIR("v2",&msg_tree2));

int json_test(void)
{
    char json_buffer[32] = {0};
    json_ws_send((struct jsontree_value *)&INFOTree, "info", json_buffer);
    os_printf("[JSON]:%s \n", json_buffer);

    char * parse_string = "{\"v\":\"100\"}";
    struct jsontree_context js;
    jsontree_setup(&js, (struct jsontree_value *)&msg_tree, json_putchar);
    json_parse(&js, parse_string);

    os_printf("[JSON]:%s \n", json_buffer);


    char * parse_string2 = "{\"v\":{\"v2\":\"200\"}}";
    jsontree_setup(&js, (struct jsontree_value *)&msg_tree_lv2, json_putchar);
    json_parse(&js, parse_string2);
}



void user_rf_pre_init(void)
{
}

void user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    
    rsa_api_unit_test();
    
    struct station_config station_conf;
    wifi_station_get_config(&station_conf);
    os_printf(MACSTR ",%s,%s \n", MAC2STR(station_conf.bssid), station_conf.password, station_conf.ssid);


    //struct station_config {
    //    uint8 ssid[32];
    //    uint8 password[64];
    //    uint8 bssid_set;    // Note: If bssid_set is 1, station will just connect to the router
    //                        // with both ssid[] and bssid[] matched. Please check about this.
    //    uint8 bssid[6];
    //};

    // 0c:4b:54:84:9e:2d t77y2qs4
    //station_conf.bssid[0] = 0x0c;
    //station_conf.bssid[1] = 0x4b;
    //station_conf.bssid[2] = 0x54;
    //station_conf.bssid[3] = 0x84;
    //station_conf.bssid[4] = 0x9e;
    //station_conf.bssid[5] = 0x2d;
    station_conf.bssid_set = 0;
    //os_strcpy(station_conf.ssid,     "hehao");
    //os_strcpy(station_conf.password, "ziqiangbuxi");
    wifi_station_set_config(&station_conf);

    wifi_station_get_config(&station_conf);
    os_printf(MACSTR ", %s, %s %d\n", MAC2STR(station_conf.bssid), station_conf.password, station_conf.ssid, station_conf.bssid_set);

    int8 id_buf[16] = {0};
    flash_param_get_id(id_buf);
    os_printf("get id = %s \n", id_buf);

    if (0 == os_strcmp(id_buf, CONFIG_RESET_ID))
    {
        os_printf("airkiss start ... \n");
        smart_config_start();
        user_switch_init();

        os_timer_disarm(&status_timer);
        os_timer_setfn(&status_timer, (os_timer_func_t *)led_status_center, NULL);
        os_timer_arm(&status_timer, 2000, 0);

        single_key[0] = key_init_single(PLUG_KEY_0_IO_NUM,
                                        PLUG_KEY_0_IO_MUX,
                                        PLUG_KEY_0_IO_FUNC,
                                        key_long_press,
                                        key_short_press);
        keys.key_num    = PLUG_KEY_NUM;
        keys.single_key = single_key;
        key_init(&keys);

        os_timer_disarm(&sys_timer);
        os_timer_setfn(&sys_timer, (os_timer_func_t *)system_secs_center, NULL);
        os_timer_arm(&sys_timer, 1000, 1); // 0 at once, 1 restart auto.

        /* 打开开关 */
        switch_level = 0x01;
        user_switch_output(1);
    }
    else
    {
        schedule_create(0);
    }
}

static void ICACHE_FLASH_ATTR led_status_center(void *arg)
{
    os_timer_arm(&status_timer, 2000, 0);
    user_wifi_output(led_status);
    led_status = (0 == led_status) ? 1 : 0;
    os_printf("get status %d \n", smart_config_get_status());

    int status = wifi_station_get_connect_status();
    int smart_status = wifi_station_get_connect_status();

    if ((E_STATUS_LINK_OVER == smart_status) &&
        (status == STATION_GOT_IP))
    {
        os_timer_disarm(&status_timer, 2000, 0);
        flash_param_set_id(CONFIG_DONE_ID);
        schedule_create(0);
    }
}

static void system_secs_center( void *arg )
{
    /* 根据需求  锁关闭后15秒钟必须将门锁上 */
    off_count++;
    if (off_count >= 15)
    {
        off_count = 0;
        switch_level = 0x01;
        user_switch_output(1);
    }
}

/******************************************************************************
 * FunctionName : key_short_press
 * Description  : key's short press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
static void ICACHE_FLASH_ATTR key_short_press( void )
{
    switch_level = (~switch_level)&0x01;
    if (0x00 == (switch_level&0x01))
    {
        off_count = 0;
    }
    user_switch_output(switch_level);

    os_printf("short press level is %d \n", switch_level);
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

    flash_param_set_id(CONFIG_RESET_ID);
    system_restore();
    system_restart();
}

