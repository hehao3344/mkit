#include "stm8s.h"
#include "../rf/system_config.h"
#include "../rf/sx1276_hal.h"
#include "../sys_mgr/time1.h"
#include "../protocol/protocol.h"
#include "../flash/flash_eeprom.h"
#include "sys_mgr.h"

#define WRITE_MAC      0
#define PACKET_LEN     12
#define MAX_SEND_NUM   4

static uint8  switch_on_off     = 0;
static uint8  device_mac[5]     = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // 地址为5字节长度
static uint8  device_address    = 0x01;     // 第一个地址
static uint8  led_count_response= 0;
static uint8  status_led_flags  = 0;
static uint8  send_timer_count  = 0;

static void delay_ms( uint32 ms );

void sys_mgr_init( void )
{
    system_config_clk_init();

    system_config_gpio_config();

 #if WRITE_MAC
    sys_mgr_write_mac();
    return;
#endif

    flash_eeprom_init();

    if ( flash_eeprom_read_buf( 0x00, device_mac, 5 ) )
    { 
        device_address = device_mac[4];
    }

    sx1276_hal_register_rf_func();

    sx1276_hal_reset();

    sx1276_hal_lora_init();

    time1_init();

    time1_set_value( 0, 0 ); // 发送
    time1_set_value( 1, 0 ); // 没用到
    time1_set_value( 2, 0 ); // 按键  在stm8s_it.c中有用到
    time1_set_value( 3, 0 ); // 串口发送计时
}

void sys_mgr_send_msg( void )
{
    uint8 i;
    uint8 * buf       = NULL;
    uint32 timer3_ms  = 0;                      // 发送周期计时器
    uint32 tmp_ms     = 0;
    timer3_ms         = time1_get_value( 0 );   // 定时器0
    uint8 is_response = 0;
    uint8 should_send = FALSE;

    // 收到从ASR发送过来的消息之后 必须立即给回应
    if ( ( timer3_ms >= 500 ) || ( led_count_response > 0 ) )
    { 
        // 判断是否是反馈消息
        is_response = ( led_count_response > 0 ) ? 1 : 0;
        
        // ASR 地址填 0x00        
        buf = protocol_device_get_send_buf( device_mac, device_address, switch_on_off, is_response ); 
        // 长度固定为字节
        if ( NULL != buf )
        {
            if ( led_count_response > 0 )
            {
                should_send = TRUE;
            }
            else
            {
                send_timer_count++;
                if ( send_timer_count >= 6 )
                {
                    send_timer_count = 0;
                    should_send = TRUE;
                }
            }

            if ( should_send )
            {
                sx1276_hal_rf_send_packet( buf, PACKET_LEN );
                delay_ms( 100 );        // 发送之后 给予一定的延时
                sx1276_hal_rx_mode();   // 设置为接收模式
            }
        }
        
        // 收到消息 第二次把反馈消息发送出去后 开始闪烁
        if ( ( MAX_SEND_NUM - 1 ) == led_count_response )
        { 
            time1_set_value( 1, 0 );
            for ( i=0; i<4; i++ )
            {
                STATUS_ON;
                delay_ms( 100 ); 
                STATUS_OFF;
                delay_ms( 100 ); 
            }
        }
        else
        {
            status_led_flags++;
            // 状态灯每隔4s 闪烁一次
            if ( status_led_flags >= 8 )
            {
                status_led_flags = 0;
                STATUS_ON;
				delay_ms( 100 ); 
                STATUS_OFF;
            }
        }

        if ( led_count_response > 0 )
        {
            led_count_response--;         
        } 

        // 一旦执行了操作 则让定时器重新开始计时
        time1_set_value( 0, 0 );         
    }
}

void sys_mgr_handle_remote_msg( void )
{    
    RfPlugResult * rf_device_result = sx1276_hal_get_rf_result();
    if ( rf_device_result->switch_invalid )
    {
        // 如果前面4个字节的mac地址都为0xFF 则认为是非法的 ASR 设备 
        if ( ( rf_device_result->mac[0] != 0xFF ) &&
             ( rf_device_result->mac[0] == device_mac[0] ) &&
             ( rf_device_result->mac[1] == device_mac[1] ) &&
             ( rf_device_result->mac[2] == device_mac[2] ) &&
             ( rf_device_result->mac[3] == device_mac[3] ) &&
             ( rf_device_result->address == device_address ) ) // 目标地址等于本地地址
        {
            // 是发送到当前设备的数据包            
            led_count_response = MAX_SEND_NUM;
            if ( 1 == rf_device_result->switch_on_off )
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
        }
    }
}

void sys_mgr_handle_key( void )
{
    switch_on_off = ( 0 == switch_on_off ) ? 1 : 0; 
    if ( 0 == switch_on_off ) 
    {
        SWITCH_OFF;     // 继电器关
    }
    else
    {
        SWITCH_ON;      // 继电器开
    } 
}

#if WRITE_MAC
void sys_mgr_write_mac( void )
{
    uint8 mac[5] = { 0x01, 0x01, 0x01, 0x01, 0x01 };
    flash_eeprom_init();

    flash_eeprom_write( 1, mac, 5 );
}
#endif

static void delay_ms( uint32 ms )
{
    uint32 tmp_ms;
    tmp_ms = time1_get_value( 0 );
    while ( ( time1_get_value( 0 ) - tmp_ms ) < ms )
    {
        ;
    }    
}
