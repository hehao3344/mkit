#include "protocol.h"

static char send_buffer[PACKET_LEN];

// v2.0 当前协议长度为10字节
// 1字节    1字节    1字节     1字节       4字节              1字节      1字节
// 0xA5     len    direction    cmd     1type+3addr          payload    checksum

cmb_handle_cb cmd_cb = NULL;

void protocol_set_cb(cmb_handle_cb cb)
{
    cmd_cb = cb;
}

// 处理命令
int protocol_handle_cmd(char * buf, char len)
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
        return NULL;
    }

    if (buf[2] != MSG_FROM_CC)  // 如果不是 中控发送过来的数据则不理会
    {
        return NULL;
    }

    /* 比较mac地址 */
    if (NULL != cmd_cb)
    {
        cmd_cb(&buf[ADDRESS_LENGTH], buf[3], buf[8]);
    }

    return 0;
}

//0x01：打开/关闭插座
char * protocol_switch_resp(char * address, char on_off)
{
    char i;
    char check_sum = 0;
    for (i=0; i<PACKET_LEN; i++)
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = (PACKET_LEN - 2);  /* 减去2是不包括0xA5 和 len */
    send_buffer[2] = MSG_FROM_DEV;      /* 上行 */
    send_buffer[3] = E_SWITCH_ON_OFF;

    /* 地址为4字节 */
    for (i=0; i<ADDRESS_LENGTH; i++)
    {
        send_buffer[4+i] = address[i];
    }

    /* 第10 字节 */
    send_buffer[8] = on_off;

    /* 计算checksum */
    for (i=1; i<(PACKET_LEN-2); i++)
    {
        check_sum += send_buffer[i];
    }
    send_buffer[PACKET_LEN-1] = (check_sum&0xFF);

    return send_buffer;
}

//0x03：获取子设备属性
char * protocol_get_property_resp(char * address, char on_off)
{
    char i;
    char check_sum = 0;
    for (i=0; i<PACKET_LEN; i++)
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = (PACKET_LEN - 2);      /* 减去2是不包括0xA5 和 len */
    send_buffer[2] = MSG_FROM_DEV;          /* 上行 */
    send_buffer[3] = E_SWITCH_GET_PARAM;    /* cmd */

    /* 地址为4字节 */
    for (i=0; i<ADDRESS_LENGTH; i++)
    {
        send_buffer[4+i] = address[i];
    }

    /* 字节 */
    send_buffer[8] = on_off;   /* 0 关 1 开 */

    /* 计算checksum */
    for (i=1; i<(PACKET_LEN-2); i++)
    {
        check_sum += send_buffer[i];
    }
    send_buffer[PACKET_LEN-1] = (check_sum&0xFF);

    return send_buffer;
}


//0x10：周期性上报消息
char * protocol_get_period_msg(char * address, char on_off)
{
    char i;
    char check_sum = 0;
    for (i=0; i<PACKET_LEN; i++)
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = (PACKET_LEN - 2);          /* 减去2是不包括0xA5 和 len */
    send_buffer[2] = MSG_FROM_DEV;              /* 上行 */
    send_buffer[3] = E_SWITCH_REPORT_MSG;       /* cmd */

    /* address地址为4字节 */
    for (i=0; i<ADDRESS_LENGTH; i++)
    {
        send_buffer[4+i] = address[i];
    }

    /* 10 字节 */
    send_buffer[8] = on_off;   /* 0 关 1 开 */

    /* 计算checksum */
    for (i=1; i<(PACKET_LEN-2); i++)
    {
        check_sum += send_buffer[i];
    }
    send_buffer[PACKET_LEN-1] = (check_sum&0xFF);

    return send_buffer;
}
