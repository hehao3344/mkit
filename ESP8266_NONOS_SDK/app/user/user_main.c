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

#include "driver/uart.h"
#include "framework/apps/user_plug.h"
#include "framework/apps/smart_config.h"
#include "framework/device/flash_param.h"

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

void user_rf_pre_init(void)
{
}

void user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());

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

