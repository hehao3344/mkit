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

#include "../device/flash_param.h"
#include "../device/sx1276.h"
#include "../device/sx1276_hal.h"
#include "../network/wifi.h"
#include "../tcp/tcp_client.h"
#include "../crypto/crypto_api.h"
#include "../crypto/packet.h"
#include "../protocol/protocol.h"
#include "json_format.h"
#include "delay.h"
#include "schedule.h"

typedef struct _DEV_PARAM
{
    char mac[DEV_MAC_LEN];
    int  status;        /* online-1 offline-0 */
    int  on_off;        /* on-1 off-0 */
    int  alive_sec;     /* */
    int  success;
} DEV_PARAM;

typedef struct _SCHEDULE_OBJECT
{
    // system timer.
    uint32     sys_sec;
    os_timer_t sys_timer;

    char cc_mac[DEV_MAC_LEN];       /* 中控的mac地址 */
    
    char dev_mac[DEV_MAC_LEN];      /* 设备的MAC地址 */
    DEV_PARAM dev_param[MAX_DEV_COUNT];

    // 3 secs off - 100 ms on - ok
    // 100 ms off - 100 ms on - auth failed.
    os_timer_t net_led_timer;

    uint8 net_led_status;
    uint8 net_led_status_count;
    uint8 net_wifi_status;          // 0 auth failed, 1 success.
    uint8 switch_level;
    uint8 off_count;
    uint8 wifi_check;

    char send_buf[256];
    char tmp_buf[128];

} SCHEDULE_OBJECT;

static SCHEDULE_OBJECT * instance(void);
static void ICACHE_FLASH_ATTR schedule_proc_task(os_event_t *events);
static void ICACHE_FLASH_ATTR system_timer_center(void *arg);
static void ICACHE_FLASH_ATTR net_led_center(void *arg);
static void ICACHE_FLASH_ATTR key_long_press(void);
static void ICACHE_FLASH_ATTR key_short_press(void);
static void ICACHE_FLASH_ATTR tcp_client_recv_data_callback(void *arg, char *buffer, unsigned short length);
static void ICACHE_FLASH_ATTR recv_data_fn(char *buffer, unsigned short len);
static void ICACHE_FLASH_ATTR protocol_handle_data_cb(char * mac, char cmd, char value);

boolean ICACHE_FLASH_ATTR schedule_create(uint16 smart_config)
{    
    SCHEDULE_OBJECT * handle = instance();

    
    os_printf("[schedule] start \n");
    
    gw_io_init(key_short_press, key_long_press);

    protocol_set_cb(protocol_handle_data_cb);

    gw_io_status_output(1);

    gw_io_sx1278_rst_output(1);

    sx1276_hal_set_recv_cb(recv_data_fn);

    sx1276_hal_init();

    char mac[24] = {0};
    wifi_get_macaddr(STATION_IF, mac);

  	os_sprintf(handle->dev_mac, MACSTR, MAC2STR(mac));

    uint8 i, j;

    for (i=0,j=0; i<6;)
    {
        handle->dev_mac[i]   = handle->dev_mac[j];
        handle->dev_mac[i+1] = handle->dev_mac[j+1];
        i += 2;
        j += 3;
    }
    handle->dev_mac[12] = 0;

#if 1
    flash_param_get_cc_mac(handle->cc_mac, sizeof(handle->cc_mac));
    for (i=0; i<MAX_DEV_COUNT; i++)
    {
        os_memset(handle->dev_param[i].mac, 0, sizeof(handle->dev_param[i].mac));
        flash_param_get_dev_mac(i, handle->dev_param[i].mac, sizeof(handle->dev_param[i].mac));
    }
#endif

    crypto_api_cbc_set_key(KEY_PASSWORD, strlen(KEY_PASSWORD));

    os_timer_disarm(&handle->sys_timer);
    os_timer_setfn(&handle->sys_timer, (os_timer_func_t *)system_timer_center, handle);
    os_timer_arm(&handle->sys_timer, 1000, 1); // 0 at once, 1 restart auto.

    os_timer_disarm(&handle->net_led_timer);
    os_timer_setfn(&handle->net_led_timer, (os_timer_func_t *)net_led_center, handle);
    os_timer_arm(&handle->net_led_timer, 100, 1);

    tcp_client_create();
    tcp_client_set_callback(tcp_client_recv_data_callback, handle);

    os_printf("[schedule] init ok \n");

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

static void ICACHE_FLASH_ATTR tcp_client_recv_data_callback(void *arg, char *buffer, unsigned short length)
{
    SCHEDULE_OBJECT * handle = (SCHEDULE_OBJECT *)arg;
    if (NULL == handle)
    {
        os_printf("invalid param \n");
        return;
    }

    os_printf("receive len:%d msg:%s \n", length, buffer);
    
    int i;
    // 测试版本 后期需要注意的是 一包发送完毕 等待发送中断收到才能发送下一包
    for(i=0; i<1; i++)
    {
        int dst_on_off = 0;
        if (NULL != os_strstr(buffer, "on"))
        {
            dst_on_off = 1;         
        }
        
        os_printf("setting switch to %d \n", dst_on_off);
        
        char * buf = protocol_switch_cmd(handle->dev_param[i].mac, (char)dst_on_off);

        /* 注意 buf len的长度为16字节 */
        
        int j;
        char send_buf[PACKET_LEN];        
        os_memcpy(send_buf, buf, sizeof(send_buf));
 
        crypto_api_encrypt_buffer(send_buf, sizeof(send_buf));

        sx1276_hal_send(send_buf, sizeof(send_buf));

        for(j=0; j<200; j++)
        {
            if (1 == sx1276_hal_get_send_flags())
            {
                os_printf("ok send finish j=%d on_off to %d \n", j, dst_on_off);
                handle->dev_param[i].on_off  = dst_on_off;
                break;
            }
            os_delay_ms(50);
        }
        sx1276_hal_set_send_flags(1);
    }
    
}

static void system_timer_center( void *arg )
{
    SCHEDULE_OBJECT * handle = ( SCHEDULE_OBJECT * )arg;

    // os_printf("get count %d \n", handle->sys_sec);
    handle->sys_sec++;

    // check wifi status.
    if (handle->wifi_check++ >= 2)
    {
        int wifi_status = wifi_station_get_connect_status();
        if (STATION_GOT_IP == wifi_status)
        {
            handle->net_wifi_status = 1;
            handle->wifi_check = 0;
        }
        else
        {
            handle->net_wifi_status = 0;
            handle->wifi_check = 2;
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

    // 测试版本 后期需要注意的是 一包发送完毕 等待发送中断收到才能发送下一包
    for(i=0; i<1; i++)
    {
        int dst_on_off = (1 == handle->dev_param[i].on_off) ? 0 : 1;
        
        char * buf = protocol_switch_cmd(handle->dev_param[i].mac, (char)dst_on_off);

        /* 注意 buf len的长度为16字节 */
        
        int j;
        char send_buf[PACKET_LEN];        
        os_memcpy(send_buf, buf, sizeof(send_buf));
 
        crypto_api_encrypt_buffer(send_buf, sizeof(send_buf));

        sx1276_hal_send(send_buf, sizeof(send_buf));

        for(j=0; j<200; j++)
        {
            if (1 == sx1276_hal_get_send_flags())
            {
                os_printf("ok send finish j=%d on_off to %d \n", j, dst_on_off);
                handle->dev_param[i].on_off  = dst_on_off;
                break;
            }
            os_delay_ms(50);
        }
        sx1276_hal_set_send_flags(1);
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

#if 1
    char reset_buf[RESET_FLAGS_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    flash_param_set_reset_flags(reset_buf, sizeof(reset_buf));
    
    system_restore();
    system_restart();
#endif
    
}

static void ICACHE_FLASH_ATTR recv_data_fn(char * buffer, unsigned short len)
{
    int i;
    os_printf("recv from sx1278 len %d \n", len);

    SCHEDULE_OBJECT * handle = instance();
    if (NULL == handle)
    {
        os_printf("invalid param \n");
        return;
    }

    if (len > 0)
    {
        /* 1解码 */
        crypto_api_decrypt_buffer(buffer, len);
        os_printf("dec: \n");
        for(i=0; i<len; i++)
        {
            os_printf("0x%x ", buffer[i]);
        }
        os_printf("\n");
        /* 处理命令 */
        protocol_handle_cmd(buffer, len);

        //gw_io_wifi_output(0);
        //os_delay_ms(50);
        //gw_io_wifi_output(1);
    }
}

static void protocol_handle_data_cb(char * mac, char cmd, char value)
{
    int i, j;
    int equal = 0;
    char upload_buf[512] = {0};
    
    os_printf("protocol callback \n");
    SCHEDULE_OBJECT * handle = instance();
    switch(cmd)
    {
        case E_SWITCH_ON_OFF:
            for (i=0; i<1; i++)
            {
                os_printf("update on_off to %d \n", value);
                handle->dev_param[i].on_off = value;
  
                os_sprintf(upload_buf, UPLOAD_EVENT_MSG, "10", handle->dev_mac, 
                           handle->dev_mac, "yes", (1 == value) ? "on" : "off");
                os_printf("=== send msg %s \n", upload_buf);
                tcp_client_send_msg(upload_buf, os_strlen(upload_buf));
                
                break; 
            }   
            break;
        case E_SWITCH_MATCH:
            break;
        case E_SWITCH_GET_PARAM:
            break;
        case E_SWITCH_REPORT_MSG:
            for (i=0; i<1; i++)
            {
                os_printf("update on_off to %d \n", value);                
                handle->dev_param[i].on_off = value;
                break; 
            }  
            // tcp_server_send_msg(handle->tmp_buf, (int)out_len);
            break;
        default:
            break;
    }
}
