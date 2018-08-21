#ifndef __CRYPTO_API_H
#define __CRYPTO_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#define KEY_PASSWORD    "abcdmkitABCDMKIT"


void crypto_api_cbc_set_key(char * key, int len);

/* 中控加密函数 输入数据长度必须是16的倍数 */
int crypto_api_encrypt_buffer(char * buffer, int len);
/* 中控解密函数 输入数据长度必须是16的倍数 */
int crypto_api_decrypt_buffer(char * buffer, int len);

int crypto_api_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif

