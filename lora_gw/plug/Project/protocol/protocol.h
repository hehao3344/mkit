#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "../core/core.h"

// 设备到遥控器的协议 每500ms发送一次
// 协议一共 12 字节
/// header(1B) = 0xA5  len(1B) direction(1B) mac(4B) address(1B) payload(xB) checksum(1B)
// direction: device -> remote control 0x01
//            remote control -> device 0x02
//            mac addr 一共4个字节
//            address  一个字节
//            payload 负载内容格式如下
//                    |       1B      |      2B     |
//                         开关状态       状态反馈


// 约定   包头(header)固定为0xFF 包长度(len)是指从direction开始一直到校验和的字节长度(包括direction和校验和)。
// 		  校验和是指从 len 开始一直到 payload 所有数据的累加和与0xFF

// 遥控器到设备的协议
// 协议一共 12 字节
// header(1B) = 0xA5  len(1B) direction(1B)   mac(4B) address(1B) payload(xB) checksum(1B)
// direction: device -> remote control 0x01
//            remote control -> device 0x02
//            mac addr 一共4个字节
//            address  一个字节
//            payload 负载内容格式如下
//                    |       1B        |         1B         |        1B       |
//                       开关命令有效        开关命令0关1开       预留为0
// 从ASR遥控器发送过来的数据结构
typedef struct RfPlugResult
{
    uint8  switch_invalid;       // 开启关闭 是否有效
    uint8  switch_on_off;
    uint8  mac[4];
    uint8  address;
} RfPlugResult;

uint8 * protocol_device_get_send_buf( uint8 *mac, uint8 address, uint8 on_off, uint8 is_response );
RfPlugResult * protocol_device_resolve_data( uint8 * buffer, uint16 len );

#endif
