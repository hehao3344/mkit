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

#define WRITE_MAC      0
#define PACKET_LEN     12
#define MAX_SEND_NUM   4

static uint8  switch_on_off     = 0;
static uint8  device_mac[6]     = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  
static uint8  dev_uuid[16]      = {0x00};

static uint8  led_count_response= 0;
static uint8  status_led_flags  = 0;
static uint8  send_timer_count  = 0;
static uint8  send_buf[128];
static uint8  tmp_buf[136];
static int    sys_sec = 0;
static int    match_end = 0;    /* ���ģʽ���������� */
static uint8  sys_mode  = 0;    /* 0 ��������ģʽ 1 ���ģʽ */

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

    time1_set_value(0, 0); // ����
    time1_set_value(1, 0); // û�õ�
    time1_set_value(2, 0); // ����  ��stm8s_it.c�����õ�
    time1_set_value(3, 0); // ���ڷ��ͼ�ʱ  
}

void sys_mgr_send_msg(void)
{
    uint8 i;
    uint32 timer3_ms  = 0;                      // �������ڼ�ʱ��
    timer3_ms         = time1_get_value(0);   // ��ʱ��0

    // �յ���ASR���͹�������Ϣ֮�� ������������Ӧ
    if ((timer3_ms >= 10000) || (led_count_response > 0))
    {
        // �ж��Ƿ��Ƿ�����Ϣ
        // is_response = (led_count_response > 0) ? 1 : 0;
        memset(send_buf, 0, sizeof(send_buf));        

        int out_len = sizeof(tmp_buf);
        if (0 == packet_enc((char *)send_buf, strlen((char *)send_buf), (char *)tmp_buf, &out_len))
        {
            /* ���� */
            crypto_api_encrypt_buffer((char *)tmp_buf, out_len);

            sx1276_hal_rf_send_packet(tmp_buf, out_len);
            delay_ms(100);        // ����֮�� ����һ������ʱ
            sx1276_hal_rx_mode();   // ����Ϊ����ģʽ


            // �յ���Ϣ �ڶ��ΰѷ�����Ϣ���ͳ�ȥ�� ��ʼ��˸
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
                // ״̬��ÿ��4s ��˸һ��
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

            // һ��ִ���˲��� ���ö�ʱ�����¿�ʼ��ʱ
            time1_set_value(0, 0);
        }
    }
}

static void recv_data_fn(char *buffer, unsigned short len)
{
    //printf("recv from sx1278 [%s] len %d \n", buffer, len);
    if (len > 0)
    {
        crypto_api_decrypt_buffer(buffer, len);
        int out_len = sizeof(tmp_buf);
        if (0 == packet_dec(buffer, len, (char *)tmp_buf, &out_len))
        {
            //printf("get data from 1278 is %s \n", tmp_buf);            
            protocol_handle_cmd((char *)tmp_buf, out_len);            
            
        }
    }
}

void sys_mgr_handle_key(void)
{
    if (0)
    {

    }
    else
    {
        switch_on_off = (0 == switch_on_off) ? 1 : 0;
        if (0 == switch_on_off)
        {
            SWITCH_OFF;     // �̵�����
        }
        else
        {
            SWITCH_ON;      // �̵�����
        }
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

static void handle_cb(char * mac, char cmd, char value)
{
    switch(cmd)
    {
        case 0x01:
        {
            if (0x01 == value)
            {
                SWITCH_ON;     /* �̵����� */
                switch_on_off = 1;
            }
            else
            {
                SWITCH_OFF;     /* �̵����� */
                switch_on_off = 0;
            }            
            char * resp = protocol_switch_resp(1, 1);
            
            /* ���͸�SX1278�п� */
                        
            break;
        }
        case 0x02:
        {
            break;
        }
    }
}
