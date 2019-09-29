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
#include "MQTTClient.h"
#include "framework/apps/schedule.h"
#include "framework/mqtt/mqtt_app.h"

void user_conn_init(void);

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


#include "mqtt_api.h"
#include "lwip/sockets.h"
#include "esp_common.h"
void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    printf("msg->event_type : %d", msg->event_type);
}

void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            printf("Message Arrived:");
            printf("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
            printf("Payload: %.*s", topic_info->payload_len, topic_info->payload);
            printf("\n");
            break;
        default:
            break;
    }
}

int example_publish(void *handle)
{
    int             res = 0;
    const char     *fmt = "/%s/%s/user/get";
    char           *topic = NULL;
    int             topic_len = 0;
    char           *payload = "{\"message\":\"hello!\"}";

    topic_len = strlen(fmt) + strlen("a1RJaoarYEB") + strlen("6k32YxnJ8VYBuF44CtRW") + 1;
    topic = malloc(topic_len);
    if (topic == NULL) {
        printf("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    snprintf(topic, topic_len, fmt, "a1RJaoarYEB", "6k32YxnJ8VYBuF44CtRW");

    res = IOT_MQTT_Publish_Simple(handle, topic, IOTX_MQTT_QOS0, payload, strlen(payload));
    if (res < 0) {
        printf("publish failed, res = %d", res);
        free(topic);
        return -1;
    }

    free(topic);
    return 0;
}


void dns_found_callback_fn(const char *name, ip_addr_t *ipaddr, void *callback_arg)
{
    printf("get ip %s \n", inet_ntoa(ipaddr));
}


#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  2048
#define MQTT_CLIENT_THREAD_PRIO         8


void wifi_event_handler_cb(System_Event_t *event)
{
    if (event == NULL) {
        return;
    }

    switch (event->event_id) {
        case EVENT_STAMODE_GOT_IP:
            printf("sta got ip ,create task and free heap size is %d\n", system_get_free_heap_size());
            //mqtt_app_init();
            //schedule_create(0);
            user_conn_init();

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

LOCAL xTaskHandle mqttc_client_handle;
//#define MQTT_BROKER  "iot.eclipse.org"  /* MQTT Broker Address*/
#define MQTT_BROKER  "10.101.70.32"     /* MQTT Broker Address*/
#define MQTT_PORT    1883               /* MQTT Port*/

static void messageArrived(MessageData* data)
{
    printf("Message arrived: %s\n", data->message->payload);
}

static void mqtt_client_thread(void* pvParameters)
{
    printf("mqtt client thread starts\n");
    MQTTClient client;
    Network network;
    unsigned char sendbuf[80], readbuf[80] = {0};
    int rc = 0, count = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    pvParameters = 0;
    NetworkInit(&network);
    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    char* address = MQTT_BROKER;

    if ((rc = NetworkConnect(&network, address, MQTT_PORT)) != 0) {
        printf("Return code from network connect is %d\n", rc);
    }

#if defined(MQTT_TASK)

    if ((rc = MQTTStartTask(&client)) != pdPASS) {
        printf("Return code from start tasks is %d\n", rc);
    } else {
        printf("Use MQTTStartTask\n");
    }

#endif

    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = "ESP8266_sample";

    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        printf("Return code from MQTT connect is %d\n", rc);
    } else {
        printf("MQTT Connected\n");
    }

    if ((rc = MQTTSubscribe(&client, " /cc50e33528de/set_switch", 2, messageArrived)) != 0) {
        printf("Return code from MQTT subscribe is %d\n", rc);
    } else {
        printf("MQTT subscribe to topic \"ESP8266/sample/sub\"\n");
    }

    while (++count) {
        MQTTMessage message;
        char payload[30];

        message.qos = QOS2;
        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d", count);
        message.payloadlen = strlen(payload);

        //if ((rc = MQTTPublish(&client, "ESP8266/sample/pub", &message)) != 0) {
        if ((rc = MQTTPublish(&client, "/cc50e33528de/heart_beat", &message)) != 0) {
            printf("Return code from MQTT publish is %d\n", rc);
        } else {
            printf("MQTT publish topic \"ESP8266/sample/pub\", message number is %d\n", count);
        }

        vTaskDelay(1000 / portTICK_RATE_MS);  //send every 1 seconds
    }

    printf("mqtt_client_thread going to be deleted\n");
    vTaskDelete(NULL);
    return;
}


char DEMO_PRODUCT_KEY[IOTX_PRODUCT_KEY_LEN + 1] = {0};
char DEMO_DEVICE_NAME[IOTX_DEVICE_NAME_LEN + 1] = {0};
char DEMO_DEVICE_SECRET[IOTX_DEVICE_SECRET_LEN + 1] = {0};

void user_conn_init(void)
{

    HAL_GetProductKey(DEMO_PRODUCT_KEY);
            HAL_GetDeviceName(DEMO_DEVICE_NAME);
            HAL_GetDeviceSecret(DEMO_DEVICE_SECRET);

            iotx_mqtt_param_t mqtt_params;
            memset(&mqtt_params, 0x0, sizeof(mqtt_params));

            mqtt_params.host = "a1RJaoarYEB.iot-as-mqtt.cn-shanghai.aliyuncs.com";
            mqtt_params.handle_event.h_fp = example_event_handle;

            void * iot_mqtt_handle = IOT_MQTT_Construct(&mqtt_params);
            if (NULL != iot_mqtt_handle)
            {
                printf("IOT_MQTT_Construct successful \n");
            }
            else
            {
                printf("IOT_MQTT_Construct failed \n");
            }


            int res = 0;
            const char *fmt = "/%s/%s/get";
            char *topic = NULL;
            int topic_len = 0;

            topic_len = strlen(fmt) + strlen("a1RJaoarYEB") + strlen("6k32YxnJ8VYBuF44CtRW") + 1;
            topic = malloc(topic_len);
            if (topic == NULL) {
                printf("memory not enough");
                return ;
            }
            memset(topic, 0, topic_len);
            snprintf(topic, topic_len, fmt, "a1RJaoarYEB", "6k32YxnJ8VYBuF44CtRW");

            res = IOT_MQTT_Subscribe(iot_mqtt_handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
            if (res < 0) {
                printf("subscribe failed");
                free(topic);
                return ;
            }
            free(topic);
            int loop_cnt = 0;
            while (1) {
                if (0 == loop_cnt % 20) {
                    //example_publish(iot_mqtt_handle);
                }

                IOT_MQTT_Yield(iot_mqtt_handle, 200);

                loop_cnt += 1;
            }


#if 0
    int ret;
    ret = xTaskCreate(mqtt_client_thread,
                      MQTT_CLIENT_THREAD_NAME,
                      MQTT_CLIENT_THREAD_STACK_WORDS,
                      NULL,
                      MQTT_CLIENT_THREAD_PRIO,
                      &mqttc_client_handle);

    if (ret != pdPASS)  {
        printf("mqtt create client thread %s failed\n", MQTT_CLIENT_THREAD_NAME);
    }
#endif
}


void user_init(void)
{
    uart_init_new();
    //user_conn_init();
    printf("SDK version:%s == %d \n", system_get_sdk_version(), portTICK_RATE_MS);

    /* 1 station 2 softap 3 station_ap */
    wifi_set_opmode(STATION_MODE);
    printf("%s %d called \n", __FUNCTION__, __LINE__);
    struct station_config config;
    bzero(&config, sizeof(struct station_config));

    config.bssid_set = 0;

    sprintf(config.ssid, "XINGLUO_034242");
    sprintf(config.password, "12345678");
    printf("%s %d called \n", __FUNCTION__, __LINE__);
    wifi_station_set_config(&config);
    printf("%s %d called \n", __FUNCTION__, __LINE__);
    wifi_set_event_handler_cb(wifi_event_handler_cb);
    printf("%s %d called \n", __FUNCTION__, __LINE__);
    wifi_station_connect();

    printf("waitting for connect ... \n");

    while(1)
    {
        vTaskDelay(1000/portTICK_RATE_MS);
        printf("SDK version:%s\n", system_get_sdk_version());
    }

}

