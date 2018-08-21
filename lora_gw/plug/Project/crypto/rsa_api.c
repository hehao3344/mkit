#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "rsa.h"
#include "rsa_api.h"

#define PUB_KEY_LEN     (32)
#define PRI_KEY_LEN     (32)

int rsa_api_gen_keys(char *pub_key, int len1, char *priv_key, int len2)
{
    if ((len1 <= (PUB_KEY_LEN+1)) || (len2 <= (PRI_KEY_LEN+1)))
    {
        printf("invalid len %d %d \n", len1, len2);
        return -1;
    }
    struct public_key_class pub;
    struct private_key_class priv;
  
    rsa_gen_keys(&pub, &priv);    
    printf("get pub %lld %lld \n", pub.modulus,  pub.exponent);
    printf("get pri %lld %lld \n", priv.modulus,  priv.exponent);

    sprintf(pub_key,  "%032lld%032lld", pub.modulus,  pub.exponent);    
    sprintf(priv_key, "%032lld%032lld", priv.modulus, priv.exponent);
    
    return 0;
}

/* 返回值不为空时用完需要释放 */
long long * rsa_api_encrypt_buffer(char *pub_key, int buf_len, char * buffer, int len)
{   
    if ((NULL == pub_key) || (buf_len < PUB_KEY_LEN*2))
    {
        printf("invalid param \n");
        return NULL;
    }
    
    char mod_buf[PUB_KEY_LEN] = {0};
    char exp_buf[PUB_KEY_LEN] = {0};
    
    int i;
    for (i=0; i<PUB_KEY_LEN; i++)
    {
        if ('0' != pub_key[i])
        {
            break;
        }
    }
    if (i >= PUB_KEY_LEN)
    {
        printf("string invalid \n");
        return NULL;
    }
    memcpy(mod_buf, &pub_key[i], PUB_KEY_LEN - i);
    
    for (i=PUB_KEY_LEN; i<PUB_KEY_LEN*2; i++)
    {
        if ('0' != pub_key[i])
        {
            break;
        }
    }
    if (i >= PUB_KEY_LEN*2)
    {
        printf("string invalid \n");
        return NULL;
    }    
    memcpy(exp_buf, &pub_key[i], PUB_KEY_LEN*2 - i);    

    struct public_key_class pub;
    
    pub.modulus  = atoll(mod_buf);
    pub.exponent = atoll(exp_buf);
    
    //os_sscanf(mod_buf, "%lld", &pub.modulus);
    //os_sscanf(exp_buf, "%lld", &pub.exponent);
    
    printf("get pub %lld %lld \n", pub.modulus, pub.exponent);
      
    long long *encrypted = rsa_encrypt(buffer, len, &pub);
    if (NULL == encrypted)
    {
        printf("rsa enc failed \n");
        return NULL;
    }
    
    return encrypted;
}

/* len为buffer占用的字节数 返回值不为空时用完需要释放 */
char * rsa_api_decrypt_buffer(char *pri_key, int buf_len, long long * buffer, int len)
{   
    if ((NULL == pri_key) || (buf_len < PRI_KEY_LEN*2))
    {
        printf("invalid param \n");
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
        printf("string invalid \n");
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
        printf("input string invalid \n");
        return NULL;
    }    
    memcpy(exp_buf, &pri_key[i], PRI_KEY_LEN*2 - i);
    
    struct private_key_class pri;  
    
    pri.modulus  = atoll(mod_buf);
    pri.exponent = atoll(exp_buf);
    
    //os_sscanf(mod_buf, "%lld", &pri.modulus);
    //os_sscanf(exp_buf, "%lld", &pri.exponent);
    
    printf("get pri %lld %lld \n", pri.modulus, pri.exponent);
            
    char *decrypted = rsa_decrypt(buffer, len, &pri);
    if (NULL == decrypted)
    {
        printf("decrypted failed \n");
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
        printf("get pub key %s \n", pub_key);
        printf("get pri key %s \n", pri_key);
        
        printf("start to enc \n");
        long long * enc_data = rsa_api_encrypt_buffer(pub_key, sizeof(pub_key), "iloveu", strlen("iloveu"));
        printf("start to dec \n");        
        char * dec_data      = rsa_api_decrypt_buffer(pri_key, sizeof(pri_key), enc_data, 8*strlen("iloveu"));
        printf("dec done \n");
        if (NULL != dec_data)
        {
            printf("get dec %s \n", dec_data);
        }
        
        if (NULL != enc_data)
        {
            free(enc_data);
        }
        if (NULL != dec_data)
        {
            free(dec_data);
        }        
    }
    
    return 0;
}
