#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "../core/core.h"

#define PACKET_LEN      16

#define MSG_FROM_DEV    2   /* 消息来自设备 */
#define MSG_FROM_CC     1   /* 消息来自中控 */

typedef enum
{
    E_SWITCH_ON_OFF     = 0x01,
    E_SWITCH_MATCH      = 0x02,
    E_SWITCH_GET_PARAM  = 0x03,
    E_SWITCH_REPORT_MSG = 0x10
} E_CMD;

// 1字节    1字节    1字节     1字节  6字节    1字节   4字节      1字节
// 0xA5     len    direction    cmd    mac    payload  reserved  checksum

//说明: len表示从direction到checksum的长度 当前协议为为10
//direction表示数据流向：01表示下行，02表示上行
//checksum表示从len到payload所有字节的累积和

//cmd：命令定义
//0x01：打开/关闭插座
//下行时Payload：1字节：0x01打开，0x00：关闭
//上行时Payload：1字节：0x10成功，0x11失败

//0x02：开始配对
//下行时，mac address为全FF，Payload：1字节：0x01表示请求配对，0x03表示配对成功；
//上行时，mac address为子设备的mac address，Payload：1字节：0x02表示子设备收到消息并且正在配对；

//0x03：获取子设备属性
//下行时Payload：1字节，填0x00
//上行时Payload：1字节，开关状态为开时0x01，关时为0x00

//0x10：子设备主动上报的消息
//该消息不需要回复，状态有变化或者每隔1分钟需要上报该消息
//Payload：1字节，开关状态为开时0x01，关时为0x00

typedef void (*cmb_handle_cb)(char * mac, char cmd, char value);

// 设置回调
void protocol_set_cb(cmb_handle_cb cb);

// 处理命令
int protocol_handle_cmd(char * buf, char len);

//0x01：打开/关闭插座
char * protocol_switch_resp(char * mac, char on_off);

//0x03：获取子设备属性
char * protocol_get_property_resp(char * mac, char on_off);

char * protocol_get_period_msg(char * mac, char on_off);

#if 0
// 1字节    1字节    1字节     1字节  6字节    4字节       N字节      1字节
// 0xA5     len    direction    cmd    mac   timestamp     payload    checksum

//说明: len表示从direction到checksum的长度；
//Direction表示数据流向：01表示下行，02表示上行
//Timestamp表示时间戳，对于同一时间戳的命令子设备只响应一次
// Checksum表示从len到payload所有字节的累积和

//Cmd：命令定义
//0x01：打开/关闭插座
//下行时Payload：1字节：0x01打开，0x00：关闭
//上行时Payload：1字节：0x10成功，0x11失败

//0x02：开始配对
//下行时，mac address为全FF，Payload：1字节：0x01表示请求配对，0x03表示配对成功；
//上行时，mac address为子设备的mac address，Payload：1字节：0x02表示子设备收到消息并且正在配对；

//0x03：获取子设备属性
//下行时Payload：1字节，填0x00
//上行时Payload：1字节，开关状态为开时0x01，关时为0x00

//0x10：子设备主动上报的消息
//该消息不需要回复，状态有变化或者每隔1分钟需要上报该消息
//Payload：1字节，开关状态为开时0x01，关时为0x00

typedef void (*cmb_handle_cb)(char * mac, char cmd, char value);

// 设置回调
void protocol_set_cb(cmb_handle_cb cb);

// 处理命令
int protocol_handle_cmd(char * buf, char len);

//0x01：打开/关闭插座
char * protocol_switch_resp(char * mac, char is_success);

//0x02：开始配对
char * protocol_match_resp(char * mac);

//0x03：获取子设备属性
char * protocol_get_property_resp(char * mac, char on_off);

char * protocol_get_period_msg(char * mac, char on_off);
#endif

#endif
