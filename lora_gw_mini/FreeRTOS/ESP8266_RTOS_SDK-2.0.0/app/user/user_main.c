/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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

#include "esp_common.h"
#include "framework/apps/schedule.h"

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;
        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}
/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

void task2(void * arg)
{
    printf("welcome to task2 \n");
    int status = 0;
    while(1)
    {
        vTaskDelay(3000/portTICK_RATE_MS);
        status = wifi_station_get_connect_status();
        printf("=== get status %d \n", status);
    }
    vTaskDelete(NULL);
}

void wifi_event_handler_cb(System_Event_t *event)
{
    if (event == NULL) {
        return;
    }

    switch (event->event_id) {
        case EVENT_STAMODE_GOT_IP:
            printf("sta got ip ,create task and free heap size is %d\n", system_get_free_heap_size());
            schedule_create(0);
            break;

        case EVENT_STAMODE_CONNECTED:
            printf("sta connected\n");
            break;

        case EVENT_STAMODE_DISCONNECTED:
            wifi_station_connect();
            break;

        default:
            break;
    }
}


void scan_done(void *arg,STATUS status)
{

     unsigned char ssid[33];
      char temp[128];
      struct station_config stationConf;
      if (status == OK)
       {
         struct bss_info *bss_link = (struct bss_info *)arg;
         bss_link = bss_link->next.stqe_next;//ignore first

         while (bss_link != NULL)
         {
           os_memset(ssid, 0, 33);
           if (os_strlen(bss_link->ssid) <= 32)
           {
             os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
           }
           else
           {
             os_memcpy(ssid, bss_link->ssid, 32);
           }
           os_sprintf(temp,"+CWLAP:(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
                      bss_link->authmode, ssid, bss_link->rssi,
                      MAC2STR(bss_link->bssid),bss_link->channel);
            printf("%s",temp);
           bss_link = bss_link->next.stqe_next;
         }

        os_memcpy(&stationConf.ssid, "XINGLUO_034242", 32);
        os_memcpy(&stationConf.password, "12345678", 64);
        wifi_station_set_config_current(&stationConf);
        wifi_station_connect();
       }
       else
       {
            printf("%s","Error");
       }
}

void user_init(void)
{
    uart_init_new();
    xTaskCreate(task2, "tsk2", 256, NULL, 2, NULL);





    wifi_station_scan(NULL,scan_done);

    printf("SDK version:%s == %d \n", system_get_sdk_version(), portTICK_RATE_MS);

    schedule_create(0);
    wifi_station_connect();

    return ;

    /* 1 station 2 softap 3 station_ap */
    wifi_set_opmode(STATION_MODE);

    struct station_config config;
    bzero(&config, sizeof(struct station_config));
    sprintf(config.ssid, "XINGLUO_034242");
    sprintf(config.password, "12345678");
    wifi_station_set_config(&config);

    //wifi_set_event_handler_cb(wifi_event_handler_cb);

    wifi_station_connect();


    //while(1)
    //{
    //    vTaskDelay(1000/portTICK_RATE_MS);
    //    printf("SDK version:%s\n", system_get_sdk_version());
    //}

}

