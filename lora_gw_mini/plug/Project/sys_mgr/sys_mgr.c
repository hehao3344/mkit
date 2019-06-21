#include "stm8s.h"
#include "../rf/system_config.h"
#include "../rf/sx1276_hal.h"
#include "../sys_mgr/time1.h"
#include "../crypto/packet.h"
#include "../protocol/protocol.h"
#include "../crypto/crypto_api.h"
#include "../protocol/protocol.h"
#include "../flash/flash_eeprom.h"
#include "sys_mgr.h"

#include <string.h>

#define WRITE_MAC               0

static uint8  pre_send_on_off   = 0;
static uint8  switch_on_off     = 0;
static char   dev_address[4]    = {0xFF, 0xFF, 0xFF, 0xFF};
static uint8  trigger_report    = 0;
static uint8  trigger_on_off_response  = 0;
static uint8  trigger_get_param_response  = 0;

static void delay_ms(uint32 ms);
static void recv_data_fn(char *buffer, unsigned short len);
static void handle_cb(char * mac, char cmd, char value);

void sys_mgr_init(void)
{
    system_config_clk_init();
    system_config_gpio_config();

    protocol_set_cb(handle_cb);

#if WRITE_MAC
    sys_mgr_write_mac();
    return;
#endif

    flash_eeprom_init();

    //crypto_api_cbc_set_key(KEY_PASSWORD, strlen(KEY_PASSWORD));

    sx1276_hal_set_recv_cb(recv_data_fn);

    flash_eeprom_read_buf(0x00, (uint8 *)dev_address, 4);

    sx1276_hal_register_rf_func();

    sx1276_hal_reset();

    sx1276_hal_lora_init();

    time1_init();

    time1_set_value(0, 0); // 发送
    time1_set_value(1, 0); // 没用到
    time1_set_value(2, 0); // 按键  在stm8s_it.c中有用到
    time1_set_value(3, 0); // 串口发送计时
}

void sys_mgr_send_msg(void)
{
    uint8 i, j;
    uint8 send_flags  = 0;
    uint32 timer3_ms  = 0;                  // 发送周期计时器
    timer3_ms         = time1_get_value(0); // 定时器0

    //char enc_buf[PACKET_LEN] = {0};

    char * resp = NULL;

    if ((timer3_ms >= 15000) || (1 == trigger_report))
    {
        pre_send_on_off = switch_on_off;
        resp = protocol_get_period_msg(dev_address, switch_on_off);
        trigger_report = 0;
        send_flags = 1;
        time1_set_value(0, 0);
    }
    else if (1 == trigger_on_off_response)
    {
        resp = protocol_switch_resp(dev_address, switch_on_off);
        send_flags = 1;
        trigger_on_off_response = 0;
    }
    else if (1 == trigger_get_param_response)
    {
        resp = protocol_get_property_resp(dev_address, switch_on_off);
        send_flags = 1;
        trigger_get_param_response = 0;
    }

    if ((1 == send_flags) && (NULL != resp))
    {
        /* 保证定时发送和事件型发送之间的时间间隔大于1s */
        timer3_ms = time1_get_value(0); // 定时器0
        while (timer3_ms < 1000)
        {
            delay_ms(100);
            timer3_ms = time1_get_value(0); // 定时器0
        }

        //memcpy(enc_buf, resp, PACKET_LEN);
        //crypto_api_encrypt_buffer((char *)enc_buf, sizeof(enc_buf));
        //sx1276_hal_rf_send_packet((uint8 *)enc_buf, sizeof(enc_buf));

        /* 不使用加密 */
        sx1276_hal_rf_send_packet((uint8 *)enc_buf, PACKET_LEN);

        for(j=0; j<20; j++)
        {
            if (1 == sx1276_hal_get_send_flags())
            {
                break;
            }
            delay_ms(50);
        }
        sx1276_hal_set_send_flags(1);

        /* 闪烁三次 */
        for (i=0; i<3; i++)
        {
            STATUS_ON;
            delay_ms(100);
            STATUS_OFF;
            delay_ms(100);
        }
        /* 发送完之后如果开关状态发生变化则立即重新发送一次 */
        if (pre_send_on_off != switch_on_off)
        {
            trigger_report = 1;
        }
        trigger_on_off_response = 0;
    }
}

static void recv_data_fn(char *buffer, unsigned short len)
{
    //char packet[PACKET_LEN];

    //if ((len > 0) && (len <= PACKET_LEN) && (0 == len%VALID_PACKET_LEN))

    if ((len > 0) && (len == PACKET_LEN))
    {
        //memcpy(packet, buffer, PACKET_LEN);
        //crypto_api_decrypt_buffer(packet, len);
        //protocol_handle_cmd(packet, len);

        protocol_handle_cmd(buffer, len);

    }
}

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
    trigger_report = 1;
    time1_set_value(2, 0);
}

#if WRITE_MAC
void sys_mgr_write_mac(void)
{
    uint8 address[6] = {0x02, 0x00, 0x00, 0x01};
    flash_eeprom_init();
    flash_eeprom_write(1, address, 4);
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

static void handle_cb(char * addr, char cmd, char value)
{
    /* 先要比较下 mac是否为本设备的mac */
    if (0 != memcmp(dev_address, addr, ADDRESS_LENGTH))
    {
        return;
    }

    switch(cmd)
    {
        case E_SWITCH_ON_OFF:
        {
            if (0x01 == value)
            {
                SWITCH_ON;     /* 继电器开 */
                switch_on_off = 1;
            }
            else
            {
                SWITCH_OFF;     /* 继电器关 */
                switch_on_off = 0;
            }
            trigger_on_off_response = 1;
            break;
        }
        case E_SWITCH_MATCH:
        {
            break;
        }
        case E_SWITCH_GET_PARAM:
        {
            trigger_get_param_response = 1;
            break;
        }
        default:
            break;
    }
}
