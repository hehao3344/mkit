#include "esp_common.h"
#include "MQTTClient.h"
#include "../apps/json_format.h"
#include "mqtt_app.h"

static int get_dev_mac(char * dev_mac, int buf_len);

#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  2048
#define MQTT_CLIENT_THREAD_PRIO         8

LOCAL xTaskHandle mqttc_client_handle;


static void messageArrived(MessageData* data);
static void mqtt_client_thread(void* pvParameters);
static int get_dev_mac(char * dev_mac, int buf_len);

void mqtt_app_init(void)
{
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
}


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

    char mac[16] = {0};
    if (0 != get_dev_mac(mac, sizeof(mac)))
    {
        return;
    }

    connectData.MQTTVersion      = 3;
    connectData.clientID.cstring = mac;

    if ((rc = MQTTConnect(&client, &connectData)) != 0) {
        printf("Return code from MQTT connect is %d\n", rc);
    } else {
        printf("MQTT Connected\n");
    }

    /* /´ò¿ª/¹Ø±Õ²å×ù */
    char msg_buf[32] = {0};

    snprintf(msg_buf, sizeof(msg_buf), "/%s/set_switch", mac);

    //MQTTSetMessageHandler(&client, msg_buf, messageArrived);
    if ((rc = MQTTSubscribe(&client, msg_buf, 1, messageArrived)) != 0) {
    //if ((rc = MQTTSetMessageHandler(&client, msg_buf, messageArrived)) != 0) {
        printf("Return code from MQTT subscribe is %d\n", rc);
    } else {
        printf("MQTT subscribe to topic %s\n", msg_buf);
    }

    /* ×¢²á */
    MQTTMessage message;
    char payload[128];

    message.qos = QOS1;
    message.retained = 0;
    message.payload  = payload;

    memset(msg_buf, 0, sizeof(msg_buf));
    snprintf(msg_buf, sizeof(msg_buf), "/%s/register", mac);

    sprintf(payload, MQTT_REGISTER_MSG);
    message.payloadlen = strlen(payload);

    if ((rc = MQTTPublish(&client, msg_buf, &message)) != 0) {
        printf("Return code from MQTT publish is %d\n", rc);
    } else {
        printf("MQTT publish topic %s, message number is %d\n", payload);
    }

    while (1) {
        message.qos      = QOS0;
        message.retained = 0;
        message.payload  = payload;

        memset(payload, 0, sizeof(payload));
        sprintf(payload, MQTT_HEART_BEAT);
        message.payloadlen = strlen(payload);

        memset(msg_buf, 0, sizeof(msg_buf));
        snprintf(msg_buf, sizeof(msg_buf), "/%s/heart_beat", mac);

        if ((rc = MQTTPublish(&client, msg_buf, &message)) != 0) {
            printf("Return code from MQTT publish is %d\n", rc);
        } else {
            printf("MQTT publish topic %s\n", msg_buf);
        }

        vTaskDelay(1000 / portTICK_RATE_MS);  //send every 1 seconds
    }

    printf("mqtt_client_thread going to be deleted\n");
    vTaskDelete(NULL);
    return;
}

static int get_dev_mac(char * dev_mac, int buf_len)
{
    char mac[24] = {0};
    char cc_mac[24] = {0};

    if (buf_len < 14)
    {
        printf("get mac failed \n");
        return -1;
    }

    wifi_get_macaddr(STATION_IF, mac);
  	sprintf(cc_mac, MACSTR, MAC2STR(mac));

    unsigned char i, j, k;

    printf("get dev mac %s \n", cc_mac);
    for (i=0,j=0,k=0; k<6;k++)
    {
        cc_mac[i]   = cc_mac[j];
        cc_mac[i+1] = cc_mac[j+1];
        i += 2;
        j += 3;
    }
    cc_mac[12] = 0;
    printf("get dev mac1 %s \n", cc_mac);
    strcpy(dev_mac, cc_mac);
    printf("get dev mac %s \n", dev_mac);

    return 0;
}
