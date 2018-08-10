#include <stdlib.h>
#include <string.h>

#include "os_type.h"
#include "osapi.h"
#include "mem.h"

/* center control API */
#include "aes_api.h"
#include "rsa_api.h"
#include "crypto_api.h"

#define  BLOCK_LEN   (16)

/* ����ֵ��Ϊ��ʱ������Ҫ�ͷ� */
char * crypto_api_gen_key(char * pub_key, char * key)
{
    if (NULL == pub_key)
    {
        os_printf("invalid param \n");
        return NULL;
    }
        
    return (char *)rsa_api_encrypt_buffer(pub_key, strlen(pub_key), key, strlen(key));
}

/* �пؼ��ܺ��� �������ݳ��ȱ�����16�ı��� */
int crypto_api_encrypt_buffer(char *key, char * buffer, int len)
{   
    if ((0 != len%BLOCK_LEN) || (NULL == key))
    {
        os_printf("invalid param \n");
        return -1;
    }
    
    return aes_api_cbc_encrypt_buffer(buffer, len, key);
}

/* �пؽ��ܺ��� �������ݳ��ȱ�����16�ı��� */
int crypto_api_decrypt_buffer(char *key, char * buffer, int len)
{   
    if ((0 != len%BLOCK_LEN) || (NULL == key))
    {
        os_printf("invalid param \n");
        return -1;
    }
    
    return aes_api_cbc_decrypt_buffer(buffer, len, key);  
}

int crypto_api_unit_test(void)
{
    char buffer[16] = {0x01, 0x02, 0x03, 0x04};
    crypto_api_encrypt_buffer(KEY_PASSWORD, buffer, sizeof(buffer));
    
    memset(buffer, 0, sizeof(buffer));
    crypto_api_decrypt_buffer(KEY_PASSWORD, buffer, sizeof(buffer));
    
    int i;
    for (i=0; i<sizeof(buffer); i++)
    {
        os_printf("0x%x ", buffer[i]);
    }
    os_printf("\n");
    
    return 0;
}
