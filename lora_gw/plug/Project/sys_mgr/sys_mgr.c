#include "stm8s.h"
#include "../rf/system_config.h"
#include "../rf/sx1276_hal.h"
#include "../json/cJson.h"
#include "../sys_mgr/time1.h"
#include "../crypto/packet.h"
#include "../crypto/crypto_api.h"
#include "../flash/flash_eeprom.h"
#include "sys_mgr.h"
#include <string.h>

#define WRITE_MAC      0
#define PACKET_LEN     12
#define MAX_SEND_NUM   4

static uint8  switch_on_off     = 0;
static uint8  device_mac[6]     = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // 地址为5字节长度
static uint8  dev_uuid[16]      = {0x00};

static uint8  led_count_response= 0;
static uint8  status_led_flags  = 0;
static uint8  send_timer_count  = 0;
static uint8  send_buf[128];
static uint8  tmp_buf[136];
static long   sys_sec = 0;
static long   match_end = 0;    /* 配对模式结束的秒数 */
static uint8  sys_mode  = 0;    /* 0 正常工作模式 1 配对模式 */

static int handle_json_msg(char *msg, char *out_buf, int len);
static void delay_ms(uint32 ms);
static void recv_data_fn(char *buffer, unsigned short len);

void sys_mgr_init(void)
{
    system_config_clk_init();

    system_config_gpio_config();

#if WRITE_MAC
    sys_mgr_write_mac();
    return;
#endif

    flash_eeprom_init();
    sx1276_hal_set_recv_cb(recv_data_fn);

    if (flash_eeprom_read_buf(0x00, device_mac, 6))
    {
        memset(dev_uuid, 0, sizeof(dev_uuid));
        sprintf((char *)dev_uuid, "02%02X%02X%02X%02X%02X%02X", (int)device_mac[0], (int)device_mac[1],
                            (int)device_mac[2], (int)device_mac[3], (int)device_mac[4], (int)device_mac[5]);
        // device_address = device_mac[4];
    }

    sx1276_hal_register_rf_func();

    sx1276_hal_reset();

    sx1276_hal_lora_init();

    time1_init();

    time1_set_value(0, 0); // 发送
    time1_set_value(1, 0); // 没用到
    time1_set_value(2, 0); // 按键  在stm8s_it.c中有用到
    time1_set_value(3, 0); // 串口发送计时
}

#define  REPORT_MSG_FORMAT "{\
\"method\":\"report_msg\",\
\"dev_uuid\":\"%s\",\
\"attr\":\
{\
\"switch\":\"%s\"\
}\
}"

void sys_mgr_send_msg(void)
{
    uint8 i;
    uint32 timer3_ms  = 0;                      // 发送周期计时器
    timer3_ms         = time1_get_value(0);     // 定时器0

    // 收到从ASR发送过来的消息之后 必须立即给回应
    if ((timer3_ms >= 10000) || (led_count_response > 0))
    {
        // 判断是否是反馈消息
        // is_response = (led_count_response > 0) ? 1 : 0;

        memset(send_buf, 0, sizeof(send_buf));
        sprintf((char *)send_buf, REPORT_MSG_FORMAT, dev_uuid, (1 == switch_on_off) ? "on":"off" );

        int out_len = sizeof(tmp_buf);
        if (0 == packet_enc((char *)send_buf, strlen((char *)send_buf), (char *)tmp_buf, &out_len))
        {
            /* 加密 */
            crypto_api_encrypt_buffer((char *)tmp_buf, out_len);

            sx1276_hal_rf_send_packet(tmp_buf, out_len);
            delay_ms(100);        // 发送之后 给予一定的延时
            sx1276_hal_rx_mode();   // 设置为接收模式


            // 收到消息 第二次把反馈消息发送出去后 开始闪烁
            if ((MAX_SEND_NUM - 1) == led_count_response)
            {
                time1_set_value(1, 0);
                for (i=0; i<4; i++)
                {
                    STATUS_ON;
                    delay_ms(100);
                    STATUS_OFF;
                    delay_ms(100);
                }
            }
            else
            {
                status_led_flags++;
                // 状态灯每隔4s 闪烁一次
                if (status_led_flags >= 8)
                {
                    status_led_flags = 0;
                    STATUS_ON;
    				delay_ms(100);
                    STATUS_OFF;
                }
            }

            if (led_count_response > 0)
            {
                led_count_response--;
            }

            // 一旦执行了操作 则让定时器重新开始计时
            time1_set_value(0, 0);
        }
    }
}

static void recv_data_fn(char *buffer, unsigned short len)
{
    printf("recv from sx1278 [%s] len %d \n", buffer, len);
    if (len > 0)
    {
        crypto_api_decrypt_buffer(buffer, len);

        int out_len = sizeof(tmp_buf);
        if (0 == packet_dec(buffer, len, (char *)tmp_buf, &out_len))
        {
            printf("get data from 1278 is %s \n", tmp_buf);

            handle_json_msg((char *)tmp_buf, (char *)tmp_buf, sizeof(tmp_buf));
            /* 发送给SX1278 */

        }
    }
}

#if 0
    if (1 == rf_device_result->switch_on_off)
    {
        // 打开开关
        SWITCH_ON;     // 继电器开
        switch_on_off = 1;
    }
    else
    {
        SWITCH_OFF;    // 继电器关
        switch_on_off = 0;
    }
#endif


void sys_mgr_handle_key(void)
{
    switch_on_off = (0 == switch_on_off) ? 1 : 0;
    if (0 == switch_on_off)
    {
        SWITCH_OFF;     // 继电器关
    }
    else
    {
        SWITCH_ON;      // 继电器开
    }
}

#if WRITE_MAC
void sys_mgr_write_mac(void)
{
    uint8 mac[6] = {0x5A, 0x5A, 0x00, 0x00, 0x00, 0x00, 01};
    flash_eeprom_init();
    flash_eeprom_write(1, mac, 6);
}
#endif

static void delay_ms(uint32 ms)
{
    uint32 tmp_ms;
    tmp_ms = time1_get_value(0);
    while ((time1_get_value(0) - tmp_ms) < ms)
    {
        ;
    }
}

#define CONTROL_RESPONSE_MSG "{\
\"method\":\"up_msg\", \
\"dev_uuid\":\"%s\", \
\"req_id\":%d,\
\"code\":%d\
}"

#define MATCH_RESPONSE_MSG "{\
\"method\":\"up_msg\",\
\"dev_uuid\":\"%s\",\
\"req_id\":%d,\
\"code\":0\
\"attr\":\
{\
\"dev_uuid\":\"%s\",\
}\
}"

static int handle_json_msg(char *msg, char *out_buf, int len)
{
    int ret = -1;
    cJSON * root_obj = cJSON_Parse(msg);
    if (NULL == root_obj)
    {
        return -1;
    }

    cJSON * sub_obj = cJSON_GetObjectItem(root_obj, "dev_uuid");
    if (NULL == sub_obj)
    {
        return -1;
    }

    if (0 != strcmp((char *)sub_obj->valuestring, (char *)dev_uuid))
    {
        return -1;
    }
    sub_obj = cJSON_GetObjectItem(root_obj, "req_id");
    if (NULL == sub_obj)
    {
        return -1;
    }

    int req_id = sub_obj->valueint;

    sub_obj = cJSON_GetObjectItem(root_obj, "ts");
    if (NULL == sub_obj)
    {
        return -1;
    }

    long cur_ts = sub_obj->valueint;
    /* 判断ts如果和系统的时间戳相差10s以上则认为该指令非法 */

    cJSON * attr_obj = cJSON_GetObjectItem(sub_obj, "attr");
    if (NULL == attr_obj)
    {
        return -1;
    }

    cJSON * cmd_obj = cJSON_GetObjectItem(attr_obj, "cmd");
    if (NULL == cmd_obj)
    {
        return -1;
    }

    if (0 == strcmp("set_switch", cmd_obj->valuestring))
    {
        if ((cur_ts - sys_sec) <= 10)
        {
            cJSON * switch_obj = cJSON_GetObjectItem(cmd_obj, "switch");
            if (NULL == switch_obj)
            {
                return -1;
            }
            if (0 == strcmp("on", switch_obj->valuestring))
            {
                // 打开开关
                SWITCH_ON;     // 继电器开
                switch_on_off = 1;
            }
            else if (0 == strcmp("off", switch_obj->valuestring))
            {
                SWITCH_OFF;    // 继电器关
                switch_on_off = 0;
            }
    
            snprintf(out_buf, len, CONTROL_RESPONSE_MSG, dev_uuid, req_id, 0);
            ret = 0;
        }
    }
    else if (0 == strcmp("set_time", cmd_obj->valuestring))
    {
        cJSON * ts_obj = cJSON_GetObjectItem(cmd_obj, "ts");
        if (NULL == ts_obj)
        {
            return -1;
        }
        sys_sec = ts_obj->valueint;

        snprintf(out_buf, len, CONTROL_RESPONSE_MSG, dev_uuid, req_id, 0);
        ret = 0;
    }
    else if (0 == strcmp("set_match", cmd_obj->valuestring))
    {
        /* 开始配对 */
        sys_mode = 1;
        match_end = sys_sec + 60; /* 60秒配对时间 */

        snprintf(out_buf, len, MATCH_RESPONSE_MSG, dev_uuid, req_id, dev_uuid);
        ret = 0;
    }
    
    return ret;
}
