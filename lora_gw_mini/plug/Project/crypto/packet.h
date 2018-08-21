#ifndef __PACKET_H
#define __PACKET_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MAGIC_NUM   (0x21222324)

typedef struct _PACKET_HEAD
{
    long magic;                  /* 魔术字 0x21222324 */
    unsigned short len;         /* 总长度 不包括头 */
    unsigned short valid_len;   /* 有效数据长度 不包括头 */
    char data[0];
} PACKET_HEAD;

int packet_enc(char * buf_in, int len_in, char * buf_out, int * out_len);
int packet_dec(char * buf_in, int len_in, char * buf_out, int * out_len);
int packet_test(void);

#ifdef __cplusplus
}
#endif

#endif

