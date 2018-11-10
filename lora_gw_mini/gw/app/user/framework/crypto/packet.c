#include <stdlib.h>
#include <string.h>

#include "os_type.h"
#include "osapi.h"
#include "mem.h"

#include "packet.h"

#define  MIN_BLOCK_LEN   (16)

int packet_enc(char * buf_in, int len_in, char * buf_out, int * out_len)
{
    int ret = -1;
    int valid_len = 0;
    int padding_len = 0;
    PACKET_HEAD head;
    head.magic      = MAGIC_NUM;    
    
    if (0 == len_in%MIN_BLOCK_LEN)
    {
        valid_len = len_in;  
        padding_len = 0;  
    }
    else
    {
        /* ÐèÒªÌî³ä (MIN_BLOCK_LEN - len_in%MIN_BLOCK_LEN) ÔÚÄ©Î² */
        padding_len = (MIN_BLOCK_LEN - len_in%MIN_BLOCK_LEN);
        valid_len   = len_in + padding_len; 
    }
    head.len        = valid_len;
    head.valid_len  = len_in;
    
    if (*out_len >= sizeof(PACKET_HEAD) + valid_len)
    {
        ret = 0;
        os_memcpy(buf_out, &head, sizeof(PACKET_HEAD));
        os_memcpy(buf_out + sizeof(PACKET_HEAD), buf_in, len_in);
        if (padding_len > 0)
        {
            os_memset(buf_out + sizeof(PACKET_HEAD) + len_in, 0, padding_len);
        }
        *out_len = sizeof(PACKET_HEAD) + len_in + padding_len;
    }
    
    return ret;
}

int packet_dec(char * buf_in, int len_in, char * buf_out, int * out_len)
{
    int ret = -1;
    PACKET_HEAD head;
    if (len_in > sizeof(PACKET_HEAD))
    {
        os_memcpy(&head, buf_in, sizeof(PACKET_HEAD));
        os_printf("got valid len %d \n", head.valid_len);
        if ((head.valid_len <= * out_len) &&
            (head.valid_len > 0))
        {
            ret = 0;
            os_memcpy(buf_out, buf_in + sizeof(PACKET_HEAD), head.valid_len);
            * out_len = head.valid_len;
        }
    }
    
    return ret;
}

int packet_test(void)
{
    char * in = "iloveu";
    char buffer[128];
    int  out_len = sizeof(buffer);
    packet_enc(in, strlen(in), buffer, &out_len);
    
    os_printf("get total %d \n", out_len);
    
    char buffer_out[64] = {0};
    out_len = sizeof(buffer_out);
    packet_dec(buffer, out_len, buffer_out, &out_len);
    os_printf("get out string %s len %d \n", buffer_out, out_len);
        
    return 0;    
}
