#include "protocol.h"

#define PROTOCOL_BUF_LEN    12

static uint8 send_buffer[PROTOCOL_BUF_LEN];
static RfPlugResult rf_plug_result;

// 产生由设备端发送到ASR遥控器端的数据
uint8 * protocol_device_get_send_buf( uint8 *mac, uint8 address, uint8 on_off, uint8 is_response )
{
    uint8 i;
    uint8 check_sum = 0;
    for ( i=0; i<PROTOCOL_BUF_LEN; i++ )
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = ( PROTOCOL_BUF_LEN - 2 ); // 减去2是不包括0xA5 和 len
    send_buffer[2] = 1;

    send_buffer[3] = mac[0];
    send_buffer[4] = mac[1];
    send_buffer[5] = mac[2];
    send_buffer[6] = mac[3];

    send_buffer[7] = address;
    send_buffer[8] = on_off;
    send_buffer[9] = is_response; // 是否是状态反馈

    for ( i=1; i < ( PROTOCOL_BUF_LEN - 1 ); i++ )
    {
        check_sum += send_buffer[i];
    }

    send_buffer[PROTOCOL_BUF_LEN-1] = ( check_sum & 0xFF );

    return send_buffer;
}

// 解析从ASR遥控器收到的数据
RfPlugResult * protocol_device_resolve_data( uint8 * buffer, uint16 len )
{
    uint8 i;
    uint8 check_sum = 0;
    // 匹配协议长度和协议头
    if ( ( PROTOCOL_BUF_LEN != len ) ||
         ( 0xA5 != buffer[0] )  )
    {
        return NULL;
    }

    for ( i=1; i < ( PROTOCOL_BUF_LEN - 1 ); i++ )
    {
        check_sum += buffer[i];
    }

    if ( buffer[PROTOCOL_BUF_LEN-1] != check_sum )
    {
        return NULL;
    }

    if ( buffer[2] != 2 )  // 如果不是 遥控端发送过来的数据则不理会
    {
        return NULL;
    }

    rf_plug_result.mac[0]         = buffer[3];
    rf_plug_result.mac[1]         = buffer[4];
    rf_plug_result.mac[2]         = buffer[5];
    rf_plug_result.mac[3]         = buffer[6];
    rf_plug_result.address        = buffer[7];

    rf_plug_result.switch_invalid = buffer[8];
    rf_plug_result.switch_on_off  = buffer[9];

    return &rf_plug_result;
}
