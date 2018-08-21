#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* center control API */
#include "aes_api.h"
#include "crypto_api.h"

#define  BLOCK_LEN   (16)

void crypto_api_cbc_set_key(char * key, int len)
{
    aes_api_cbc_set_key(key, len);
}

/* �пؼ��ܺ��� �������ݳ��ȱ�����16�ı��� */
int crypto_api_encrypt_buffer(char * buffer, int len)
{   
    if (0 != len%BLOCK_LEN)
    {
        return -1;
    }
    
    return aes_api_cbc_encrypt_buffer(buffer, len);
}

/* �пؽ��ܺ��� �������ݳ��ȱ�����16�ı��� */
int crypto_api_decrypt_buffer(char * buffer, int len)
{   
    if (0 != len%BLOCK_LEN)
    {
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
    
   
    return 0;
}
