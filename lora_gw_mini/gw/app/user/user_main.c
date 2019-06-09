/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"
#include "os_type.h"
#include "user_devicefind.h"
#include "user_webserver.h"

#include "driver/uart.h"
#include "framework/device/flash_param.h"
#include "framework/apps/smart_config.h"
#include "framework/apps/schedule.h"

#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0xfd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0xfc000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0xfc000
#else
#error "The flash map is not supported"
#endif

#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM                SYSTEM_PARTITION_CUSTOMER_BEGIN

uint32 priv_param_start_sec;
static uint8      led_status = 0;
static os_timer_t status_timer;

static void ICACHE_FLASH_ATTR key_short_press(void);
static void ICACHE_FLASH_ATTR key_long_press(void);
static void ICACHE_FLASH_ATTR led_status_center(void *arg);

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
    { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
    { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
    { SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM,             SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR,          0x1000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    partition_item_t partition_item;
    os_printf("SDK version:%s\n", system_get_sdk_version());

    if (!system_partition_get_item(SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM, &partition_item)) {
        os_printf("Get partition information fail\n");
    }
    priv_param_start_sec = partition_item.addr/SPI_FLASH_SEC_SIZE;
#if ESP_PLATFORM
    /*Initialization of the peripheral drivers*/
    /*For light demo , it is user_light_init();*/
    /* Also check whether assigned ip addr by the router.If so, connect to ESP-server  */
    user_esp_platform_init();
#endif
    /*Establish a udp socket to receive local device detect info.*/
    /*Listen to the port 1025, as well as udp broadcast.
    /*If receive a string of device_find_request, it rely its IP address and MAC.*/
    // user_devicefind_init();

    /*Establish a TCP server for http(with JSON) POST or GET command to communicate with the device.*/
    /*You can find the command in "2B-SDK-Espressif IoT Demo.pdf" to see the details.*/
    /*the JSON command for curl is like:*/
    /*3 Channel mode: curl -X POST -H "Content-Type:application/json" -d "{\"period\":1000,\"rgb\":{\"red\":16000,\"green\":16000,\"blue\":16000}}" http://192.168.4.1/config?command=light      */
    /*5 Channel mode: curl -X POST -H "Content-Type:application/json" -d "{\"period\":1000,\"rgb\":{\"red\":16000,\"green\":16000,\"blue\":16000,\"cwhite\":3000,\"wwhite\",3000}}" http://192.168.4.1/config?command=light      */
#ifdef SERVER_SSL_ENABLE
    //user_webserver_init(SERVER_SSL_PORT);
#else
    //user_webserver_init(SERVER_PORT);
#endif

#if 1
    int i;
    struct station_config station_conf;
    wifi_station_get_config(&station_conf);
    os_printf(MACSTR ",%s,%s \n", MAC2STR(station_conf.bssid), station_conf.password, station_conf.ssid);

    //station_conf.bssid_set = 0;
    //os_strcpy(station_conf.ssid,     "hehao");
    //os_strcpy(station_conf.password, "ziqiangbuxi");
    //wifi_station_set_config(&station_conf);

    //wifi_station_get_config(&station_conf);
    //os_printf(MACSTR ", %s, %s %d\n", MAC2STR(station_conf.bssid), station_conf.password, station_conf.ssid, station_conf.bssid_set);


    int8 is_reset = 0;
    
    char flags_buf[RESET_FLAGS_LEN+4] = {0};    
    flash_param_get_reset_flags(flags_buf, RESET_FLAGS_LEN);
    
    
    for (i=0; i<RESET_FLAGS_LEN; i++)
    {
        os_printf("0x%x ", flags_buf[i]);
    }

    int count = 0;
    for (i=0; i<RESET_FLAGS_LEN; i++)
    {
        if (0xFF == flags_buf[i])
        {
            count++;
        }
    }
   
    if (RESET_FLAGS_LEN == count)
    {
        is_reset = 1;
    }
    
    if (0 == os_strlen(station_conf.ssid))        
    {
        is_reset = 1;
    }
    else        
    {
        is_reset = 0;
    }
    os_printf("\nget is_reset = %d \n", is_reset);

    if (1 == is_reset)
    {
        os_printf("airkiss start ... \n");
        smart_config_start();
        gw_io_init(key_short_press, key_long_press);

        os_timer_disarm(&status_timer);
        os_timer_setfn(&status_timer, (os_timer_func_t *)led_status_center, NULL);
        os_timer_arm(&status_timer, 2000, 0);
    }
    else
    {
        os_printf("start normal\n");   
        schedule_create(0);
    }
#endif     
}

static void ICACHE_FLASH_ATTR key_long_press(void)
{
    os_printf("long press \n");

    char reset_buf[RESET_FLAGS_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    flash_param_set_reset_flags(reset_buf, sizeof(reset_buf));
    
    system_restore();
    system_restart();
}

static void ICACHE_FLASH_ATTR key_short_press(void)
{
    os_printf("short press \n");
}

static void ICACHE_FLASH_ATTR led_status_center(void *arg)
{
    os_timer_arm(&status_timer, 2000, 0);
    gw_io_wifi_output(led_status);
    led_status = (0 == led_status) ? 1 : 0;
    os_printf("get status %d \n", smart_config_get_status());

    int status = wifi_station_get_connect_status();
    if (status == STATION_GOT_IP)
    {
        os_timer_disarm(&status_timer);
        
        char reset_done_buf[RESET_FLAGS_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        flash_param_set_reset_flags(reset_done_buf, sizeof(reset_done_buf));
        
        os_printf("schedule_create start ...\n");
        schedule_create(0);
    }
}
