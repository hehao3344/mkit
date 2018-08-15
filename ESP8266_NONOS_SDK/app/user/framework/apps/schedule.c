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
#include "../core/mem_mgr.h"
#include "../device/flash_param.h"
#include "../device/sx1276_hal.h"
#include "../network/wifi.h"
#include "../network/discovery.h"
#include "../crypto/crypto_api.h"
// #include "../tcp/tcp_client.h"
#include "../tcp/tcp_server.h"
#include "delay.h"
#include "user_plug.h"
#include "schedule.h"

#define MAX_SUB_DEV_COUNT   4

typedef struct _SCHEDULE_OBJECT
{
    // system timer.
    uint32     sys_sec;
    os_timer_t sys_timer;

    int8 mac[40];
    
    char dev_uuid[MAX_SUB_DEV_COUNT][16];
    
    // 3 secs off - 100 ms on - ok
    // 100 ms off - 100 ms on - auth failed.
    os_timer_t net_led_timer;
    uint8 net_led_status;
    uint8 net_led_status_count;
    uint8 net_wifi_status;          // 0 auth failed, 1 success.
    uint8 switch_level;
    uint8 off_count;
    
    char send_buf[256];
    // key press.
    struct keys_param keys;
    struct single_key_param *single_key[PLUG_KEY_NUM];

} SCHEDULE_OBJECT;

static SCHEDULE_OBJECT * instance(void);
static void ICACHE_FLASH_ATTR schedule_proc_task(os_event_t *events);
static void ICACHE_FLASH_ATTR system_timer_center(void *arg);
static void ICACHE_FLASH_ATTR net_led_center(void *arg);
static void ICACHE_FLASH_ATTR key_long_press(void);
static void ICACHE_FLASH_ATTR key_short_press(void);
static void ICACHE_FLASH_ATTR tcp_recv_data_callback(void *arg, char *buffer, unsigned short length);
static void ICACHE_FLASH_ATTR recv_data_fn(char *buffer, unsigned short len);

boolean ICACHE_FLASH_ATTR schedule_create(uint16 smart_config)
{
    SCHEDULE_OBJECT * handle = instance();

    user_switch_init();
    //handle->switch_level = 0;
    //user_switch_output(handle->switch_level);
    //os_printf("turn switch off \n");
    handle->single_key[0] = key_init_single(PLUG_KEY_0_IO_NUM,
                                            PLUG_KEY_0_IO_MUX,
                                            PLUG_KEY_0_IO_FUNC,
                                            key_long_press,
                                            key_short_press);
    handle->keys.key_num    = PLUG_KEY_NUM;
    handle->keys.single_key = handle->single_key;
    key_init(&handle->keys);
    
    int i;
    for (i=0; i<MAX_SUB_DEV_COUNT; i++)
    {
        os_memset(handle->dev_uuid[i], 0, sizeof(handle->dev_uuid[i]));
        flash_param_get_dev_uuid(i, handle->dev_uuid[i], sizeof(handle->dev_uuid[i]));
    }
    crypto_api_cbc_set_key(KEY_PASSWORD, strlen(KEY_PASSWORD));
    sx1276_hal_set_recv_cb(recv_data_fn);    
    
    os_timer_disarm(&handle->sys_timer);
    os_timer_setfn(&handle->sys_timer, (os_timer_func_t *)system_timer_center, handle);
    os_timer_arm(&handle->sys_timer, 1000, 1); // 0 at once, 1 restart auto.

    os_timer_disarm(&handle->net_led_timer);
    os_timer_setfn(&handle->net_led_timer, (os_timer_func_t *)net_led_center, handle);
    os_timer_arm(&handle->net_led_timer, 100, 1);

    //tcp_client_create();
    //tcp_client_set_callback(tcp_recv_data_callback, handle);

    char mac[24] = {0};
    wifi_get_macaddr(STATION_IF, mac);
    memset(handle->mac, 0, sizeof(handle->mac));
  	os_sprintf(handle->mac, MACSTR, MAC2STR(mac));

    int j;
    for (i=0,j=0; i<12;)
    {
        handle->mac[i]   = handle->mac[j];
        handle->mac[i+1] = handle->mac[j+1];
        i += 2;
        j += 3;
    }
    handle->mac[12] = 0;
    
    DISCOVER_ENV dis_env;
    dis_env.port = TCP_BIND_PORT;
    os_memset(&dis_env, 0, sizeof(DISCOVER_ENV));
    os_sprintf(dis_env.dev_uuid, "01%s", handle->mac); /* 01表示中控 */
    if (0 != discovery_create(&dis_env))
    {
        os_printf("dis failed \n");
        return -1;
    }

    os_printf("[sch] init ok \n");

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

static void ICACHE_FLASH_ATTR tcp_recv_data_callback(void *arg, char *buffer, unsigned short length)
{
    SCHEDULE_OBJECT * handle = (SCHEDULE_OBJECT *)arg;
    if (NULL == handle)
    {
        os_printf("invalid param \n");
        return;
    }

    os_printf("receive len:%d msg:%s \n", length, buffer);
}

#define SYNC_TIME "{\
\"method\":\"down_msg\",\
\"dev_uuid\":\"%s\",\
\"req_id\":%d,\
\"attr\":\
{\
\"cmd\":\"set_time\":\
{\
\"ts\":%d\
}\
}\
}"

static void system_timer_center( void *arg )
{
    SCHEDULE_OBJECT * handle = ( SCHEDULE_OBJECT * )arg;
    
    if (0 == handle->sys_sec++ % 60)
    {
        int i;
        int req_id = (int)rand();
        for (i=0; i<MAX_SUB_DEV_COUNT; i++)
        {            
            /* 每隔XX秒同步一次时间 */
            os_memset(handle->send_buf, 0, sizeof(handle->send_buf));
            os_sprintf(handle->send_buf, SYNC_TIME, handle->dev_uuid[i], req_id, handle->sys_sec);
            
            crypto_api_encrypt_buffer(handle->send_buf, sizeof(handle->send_buf));
             /* 发送到子设备 */
            sx1276_hal_rf_send_packet(handle->send_buf, (unsigned char)sizeof(handle->send_buf));           
        }
    }    

    // check wifi status.
    static uint32 wifi_check = 0;
    if (wifi_check++ >= 2)
    {
        uint8 wifi_status = wifi_station_get_connect_status();
        if (STATION_GOT_IP == wifi_status)
        {
            handle->net_wifi_status = 1;
            wifi_check = 0;
        }
        else
        {
            handle->net_wifi_status = 0;
            wifi_check = 2;
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
        user_wifi_output(handle->net_led_status);
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
        user_wifi_output(handle->net_led_status);
    }
}

/******************************************************************************
 * FunctionName : key_short_press
 * Description  : key's short press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
#define SET_MATCH_MSG  "{\
\"method\":\"down_msg\",\
\"dev_uuid\":\"FFFFFFFFFFFF\",\
\"req_id\":%d,\
\"ts\":%d,\
\"attr\":\
{\
\"cmd\":\"set_match\":\
{\
}\
}\
}"

static void ICACHE_FLASH_ATTR key_short_press( void )
{
    SCHEDULE_OBJECT * handle = instance();
    int i;
    int req_id = (int)rand();
    for (i=0; i<MAX_SUB_DEV_COUNT; i++)
    {            
        /* 每隔XX秒同步一次时间 */
        os_memset(handle->send_buf, 0, sizeof(handle->send_buf));
        os_sprintf(handle->send_buf, SET_MATCH_MSG, handle->dev_uuid[i], req_id, handle->sys_sec);
        
        crypto_api_encrypt_buffer(handle->send_buf, sizeof(handle->send_buf));
         /* 发送到子设备 */
        sx1276_hal_rf_send_packet(handle->send_buf, (unsigned char)sizeof(handle->send_buf));       
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

    //flash_param_default_restore();
    //user_esp_platform_set_active(0);
    //flash_param_set_id(CONFIG_RESET_ID);

    system_restore();
    system_restart();
}

static void ICACHE_FLASH_ATTR recv_data_fn(char *buffer, unsigned short len)
{
    os_printf("recv from sx1278 [%s] len %d \n", buffer, len);
    if (len > 0)
    {
        tcp_server_send_msg(buffer, (int)len);
    }
}
