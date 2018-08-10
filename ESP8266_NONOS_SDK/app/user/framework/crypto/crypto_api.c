#include <stdlib.h>
#include <string.h>

#include "os_type.h"
#include "osapi.h"
#include "mem.h"

/* center control API */
#include "aes_api.h"
#include "rsa_api.h"

/* 返回值不为空时用完需要释放 */
char * crypto_api_gen_key(char * pub_key, char * key)
{
    if (NULL == pub_key)
    {
        os_printf("invalid param \n");
        return NULL;
    }
        
    return rsa_api_encrypt_buffer(pub_key, key);
}

int crypto_api_encrypt_buffer(char *key, char * buffer, int len)
{   
    if ((0 != len%16) || (NULL == key))
    {
        os_printf("invalid param \n");
        return -1;
    }
    
    return aes_api_cbc_encrypt_buffer(buffer, sizeof(buffer));
}

/* len为buffer占用的字节数 返回值不为空时用完需要释放 */
char * rsa_api_decrypt_buffer(char *pri_key, int buf_len, long long * buffer, int len)
{   
    if ((NULL == pri_key) || (buf_len < PRI_KEY_LEN*2))
    {
        os_printf("invalid param \n");
        return NULL;
    }
    
    char mod_buf[PRI_KEY_LEN] = {0};
    char exp_buf[PRI_KEY_LEN] = {0};
        
    int i;
    for (i=0; i<PRI_KEY_LEN; i++)
    {
        if ('0' != pri_key[i])
        {
            break;
        }
    }
    if (i >= PUB_KEY_LEN)
    {
        os_printf("string invalid \n");
        return NULL;
    }
    memcpy(mod_buf, &pri_key[i], PRI_KEY_LEN - i);
    
    for (i=PUB_KEY_LEN; i<PUB_KEY_LEN*2; i++)
    {
        if ('0' != pri_key[i])
        {
            break;
        }
    }
    if (i >= PRI_KEY_LEN*2)
    {
        os_printf("input string invalid \n");
        return NULL;
    }    
    memcpy(exp_buf, &pri_key[i], PRI_KEY_LEN*2 - i);
    
    struct private_key_class pri;  
    
    pri.modulus  = atoll(mod_buf);
    pri.exponent = atoll(exp_buf);
    
    //os_sscanf(mod_buf, "%lld", &pri.modulus);
    //os_sscanf(exp_buf, "%lld", &pri.exponent);
    
    os_printf("get pri %lld %lld \n", pri.modulus, pri.exponent);
            
    char *decrypted = rsa_decrypt(buffer, len, &pri);
    if (NULL == decrypted)
    {
        os_printf("decrypted failed \n");
        return NULL;
    }
    
    return decrypted;   
}

int rsa_api_unit_test(void)
{
    char pub_key[128] = {0};
    char pri_key[128] = {0};
        
    int ret = rsa_api_gen_keys(pub_key, sizeof(pub_key), pri_key, sizeof(pri_key));
    
    strcpy(pub_key, "0000000000000000000000010767165700000000000000000000000000000257");
    strcpy(pri_key, "0000000000000000000000010767165700000000000000000000000077491893");

    ret = 0;
    if (0 == ret)
    {
        os_printf("get pub key %s \n", pub_key);
        os_printf("get pri key %s \n", pri_key);
        
        os_printf("start to enc \n");
        long long * enc_data = rsa_api_encrypt_buffer(pub_key, sizeof(pub_key), "iloveu", strlen("iloveu"));
        os_printf("start to dec \n");        
        char * dec_data      = rsa_api_decrypt_buffer(pri_key, sizeof(pri_key), enc_data, 8*strlen("iloveu"));
        os_printf("dec done \n");
        if (NULL != dec_data)
        {
            os_printf("get dec %s \n", dec_data);
        }
        
        if (NULL != enc_data)
        {
            os_free(enc_data);
        }
        if (NULL != dec_data)
        {
            os_free(dec_data);
        }        
    }
    
    return 0;
}
