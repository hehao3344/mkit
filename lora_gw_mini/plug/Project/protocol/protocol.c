#include "protocol.h"

#define MAX_BUF_LEN  16

static char send_buffer[MAX_BUF_LEN];

// 1字节    1字节    1字节     1字节  6字节    4字节       N字节      1字节
// 0xA5     len    direction    cmd    mac   timestamp     payload    checksum
// 当前协议的payload都为16字节


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
    if ((MAX_BUF_LEN != len) || (0xA5 != buf[0]))
    {
        return -1;
    }

    for (i=1; i<(MAX_BUF_LEN - 1); i++ )
    {
        check_sum += buffer[i];
    }

    if (buffer[MAX_BUF_LEN-1] != check_sum)
    {
        return NULL;
    }

    if (buffer[2] != 0x01)  // 如果不是 中控发送过来的数据则不理会
    {
        return NULL;
    }
    
    /* 比较mac地址 */
    if (NULL != cmd_cb)
    {
        cmd_cb(buf[4], buf[3], buffer[14]);        
    }
    
    return 0;
}

//0x01：打开/关闭插座
char * protocol_switch_resp(char * mac, char is_success)
{
    char i;
    char check_sum = 0;
    for (i=0; i<MAX_BUF_LEN; i++)
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = (MAX_BUF_LEN - 2); /* 减去2是不包括0xA5 和 len */
    send_buffer[2] = 2;                 /* 上行 */
    send_buffer[3] = 0x01; 
    
    /* mac地址为6字节 */
    for (i=0; i<6; i++)
    {
        send_buffer[4+i] = mac[i];
    }
    /* ts 10开始到13 回复的都填0 */
    
    /* 14 字节 */    
    send_buffer[14] = (1 == is_success) ? 0x10 : 0x11;
        
    /* 计算checksum */    
    for (i=1; i<(MAX_BUF_LEN-1); i++)
    {
        check_sum += send_buffer[i];
    }
    send_buffer[15] = (check_sum&0xFF);

    return send_buffer;
}

//0x02：开始配对
char * protocol_match_resp(char * mac)
{
    char i;
    char check_sum = 0;
    for (i=0; i<MAX_BUF_LEN; i++)
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = (MAX_BUF_LEN - 2); /* 减去2是不包括0xA5 和 len */
    send_buffer[2] = 2;                 /* 上行 */
    send_buffer[3] = 0x02; 
    
    /* mac地址为6字节 */
    for (i=0; i<6; i++)
    {
        send_buffer[4+i] = mac[i];
    }
    /* ts 10开始到13 回复的都填0 */
    
    /* 14 字节 */    
    send_buffer[14] = 0x02;
        
    /* 计算checksum */    
    for (i=1; i<(MAX_BUF_LEN-1); i++)
    {
        check_sum += send_buffer[i];
    }
    send_buffer[15] = (check_sum&0xFF);

    return send_buffer;
}

//0x03：获取子设备属性
char * protocol_get_property_resp(char * mac, char on_off)
{
    char i;
    char check_sum = 0;
    for (i=0; i<MAX_BUF_LEN; i++)
    {
        send_buffer[i] = 0x00;
    }

    send_buffer[0] = 0xA5;
    send_buffer[1] = (MAX_BUF_LEN - 2); /* 减去2是不包括0xA5 和 len */
    send_buffer[2] = 2;                 /* 上行 */
    send_buffer[3] = 0x03;              /* cmd */
    
    /* mac地址为6字节 */
    for (i=0; i<6; i++)
    {
        send_buffer[4+i] = mac[i];
    }
    /* ts 10开始到13 回复的都填0 */
    
    /* 14 字节 */    
    send_buffer[14] = on_off;   /* 0 关 1 开 */
        
    /* 计算checksum */    
    for (i=1; i<(MAX_BUF_LEN-1); i++)
    {
        check_sum += send_buffer[i];
    }
    send_buffer[15] = (check_sum&0xFF);

    return send_buffer;
}
