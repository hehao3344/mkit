#ifndef __RSA_API_H
#define __RSA_API_H

#ifdef __cplusplus
extern "C"
{
#endif

int  rsa_api_gen_keys(char *pub_key, int len1, char *priv_key, int len2);
long long * rsa_api_encrypt_buffer(char *pub_key, int buf_len, char * buffer, int len);
char * rsa_api_decrypt_buffer(char *pri_key, int buf_len, long long * buffer, int len);
int rsa_api_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif

