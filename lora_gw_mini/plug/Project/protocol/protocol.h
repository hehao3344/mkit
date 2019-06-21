#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "../core/core.h"

#define PACKET_LEN      10
#define ADDRESS_LENGTH  4

#define MSG_FROM_DEV    2   /* 消息来自设备 */
#define MSG_FROM_CC     1   /* 消息来自中控 */

typedef enum
{
    E_SWITCH_ON_OFF     = 0x01,
    E_SWITCH_MATCH      = 0x02,
    E_SWITCH_GET_PARAM  = 0x03,
    E_SWITCH_REPORT_MSG = 0x10
} E_CMD;

// v1.0 当前协议长度为12字节
// 1字节    1字节    1字节     1字节       6字节       4字节     1字节      1字节
// 0xA5     len    direction    cmd         mac         ts    payload    checksum

// v2.0 当前协议长度为10字节
// 1字节    1字节    1字节     1字节       4字节              1字节      1字节
// 0xA5     len    direction    cmd     1type+3addr          payload    checksum

//说明: len表示从direction到checksum的长度 当前协议为为10
//direction表示数据流向：01表示下行，02表示上行
//checksum表示从len到payload所有字节的累积和

//cmd：命令定义
//0x01：打开/关闭插座
//下行时Payload：1字节：0x01打开，0x00：关闭
//上行时Payload：1字节：0x10成功，0x11为失败

//0x02：开始配对
//下行时，Address后三字节为全FF，payload为1表示请求配对，为3表示已经收到配对确认；
//上行时，Address后三字节为子设备的地址，payload为2表示配对成功；
//0x03：获取子设备属性
//下行时Payload：1字节，填0x00
//上行时Payload：1字节，开关状态为开时0x01，关为0x00

//0x10：子设备主动上报的消息
//该消息不需要回复，状态有变化或者每隔1分钟需要上报该消息
//Payload：1字节，开关状态为开时0x01，关时为0x00。


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

#endif
