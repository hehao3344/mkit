#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "user_interface.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"

#include "protocol.h"

static char send_buffer[PACKET_LEN];

// 1字节    1字节    1字节     1字节  6字节    1字节      1字节
// 0xA5     len    direction    cmd    mac    payload    checksum
// 当前协议的payload都为16字节

cmb_handle_cb cmd_cb = NULL;

void ICACHE_FLASH_ATTR protocol_set_cb(cmb_handle_cb cb)
{
    cmd_cb = cb;
}

// 处理命令
int ICACHE_FLASH_ATTR protocol_handle_cmd(char * buf, char len)
{
    char i;
    char check_sum = 0;
    // 匹配协议长度和协议头
    if ((PACKET_LEN != len) || (0xA5 != buf[0]))
    {
        return -1;
    }

    for (i=1; i<(PACKET_LEN - 2); i++ )
    {
        check_sum += buf[i];
    }

    if (buf[PACKET_LEN-1] != check_sum)
    {
        return -1;
    }

    if (buf[2] != MSG_FROM_DEV)  // 如果不是 设备发送过来的数据则不理会
    {
        return -1;
    }

    /* 比较mac地址 */
    if (NULL != cmd_cb)
    {
        cmd_cb(&buf[4], buf[3], buf[8]);
    }

    return 0;
}

// 0x01：产生打开/关闭插座命令数据
char * ICACHE_FLASH_ATTR protocol_switch_cmd(char * mac, char on_off)
{
    char i;
    char check_sum = 0;
    for (i=0; i<PACKET_LEN; i++)
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = (PACKET_LEN - 2); /* 减去2是不包括0xA5 和 len */
    send_buffer[2] = MSG_FROM_CC;       /* 下行 */
    send_buffer[3] = E_SWITCH_ON_OFF;   /* */

    /* mac地址为4字节 */
    for (i=0; i<ADDRESS_LENGTH; i++)
    {
        send_buffer[4+i] = mac[i];
    }

    send_buffer[8] = on_off;

    /* 计算checksum */
    for (i=1; i<(send_buffer[1]); i++)
    {
        check_sum += send_buffer[i];
    }
    send_buffer[PACKET_LEN-1] = (check_sum&0xFF);

    return send_buffer;
}

//0x02：开始配对 payload = 1 表示开始配对 =3表示配对成功
char * ICACHE_FLASH_ATTR protocol_match_cmd(int payload)
{
    char i;
    char check_sum = 0;
    for (i=0; i<PACKET_LEN; i++)
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = (PACKET_LEN - 2); /* 减去2是不包括0xA5 和 len */
    send_buffer[2] = MSG_FROM_CC;       /* 下行 */
    send_buffer[3] = E_SWITCH_MATCH;    /* */

    /* mac地址为6字节 */
    for (i=0; i<ADDRESS_LENGTH; i++)
    {
        send_buffer[4+i] = 0xFF;
    }
    send_buffer[8] = payload;

    /* 计算checksum */
    for (i=1; i<send_buffer[1]; i++)
    {
        check_sum += send_buffer[i];
    }
    send_buffer[PACKET_LEN-1] = (check_sum&0xFF);

    return send_buffer;
}

//0x03：获取子设备属性
char * ICACHE_FLASH_ATTR protocol_get_property(char * mac)
{
    char i;
    char check_sum = 0;
    for (i=0; i<PACKET_LEN; i++)
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = (PACKET_LEN - 2); /* 减去2是不包括0xA5 和 len */
    send_buffer[2] = MSG_FROM_CC;       /* 下行 */
    send_buffer[3] = 0x03;              /* cmd */

    /* mac地址为6字节 */
    for (i=0; i<ADDRESS_LENGTH; i++)
    {
        send_buffer[4+i] = mac[i];
    }

    /* 14 字节 */
    send_buffer[8] = 0;   /* 0 关 1 开 */

    /* 计算checksum */
    for (i=1; i<PACKET_LEN; i++)
    {
        check_sum += send_buffer[i];
    }
    send_buffer[PACKET_LEN-1] = (check_sum&0xFF);

    return send_buffer;
}
