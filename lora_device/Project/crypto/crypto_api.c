#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* center control API */
#include "aes_api.h"
#include "rsa_api.h"
#include "crypto_api.h"

#define  BLOCK_LEN   (16)

/* 返回值不为空时用完需要释放 */
char * crypto_api_gen_key(char * pub_key, char * key)
{
    if (NULL == pub_key)
    {
        printf("invalid param \n");
        return NULL;
    }
        
    return (char *)rsa_api_encrypt_buffer(pub_key, strlen(pub_key), key, strlen(key));
}

void crypto_api_cbc_set_key(char * key, int len)
{
    aes_api_cbc_set_key(key, len);
}

/* 中控加密函数 输入数据长度必须是16的倍数 */
int crypto_api_encrypt_buffer(char * buffer, int len)
{   
    if (0 != len%BLOCK_LEN)
    {
        printf("invalid param \n");
        return -1;
    }
    
    return aes_api_cbc_encrypt_buffer(buffer, len);
}

/* 中控解密函数 输入数据长度必须是16的倍数 */
int crypto_api_decrypt_buffer(char * buffer, int len)
{   
    if (0 != len%BLOCK_LEN)
    {
        printf("invalid param \n");
        return -1;
    }
    
    return aes_api_cbc_decrypt_buffer(buffer, len);  
}

int crypto_api_unit_test(void)
{
    char buffer[32] = {0x01, 0x02, 0x03, 0x04};
    
    aes_api_cbc_set_key(KEY_PASSWORD, strlen(KEY_PASSWORD));
    crypto_api_encrypt_buffer(buffer, sizeof(buffer));
    
    //memset(buffer, 0, sizeof(buffer));
    crypto_api_decrypt_buffer(buffer, sizeof(buffer));
    
    int i;
    for (i=0; i<sizeof(buffer); i++)
    {
        printf("0x%x ", buffer[i]);
    }
    printf("\n");
    
    return 0;
}
