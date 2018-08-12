#ifndef __AES_API_H
#define __AES_API_H

#ifdef __cplusplus
extern "C"
{
#endif

void aes_api_cbc_set_key(char * key, int len);
int aes_api_cbc_encrypt_buffer(char * buffer, int len);
int aes_api_cbc_decrypt_buffer(char * buffer, int len);

int aes_api_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif

