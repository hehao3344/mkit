#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes.h"
#include "aes_api.h"

static unsigned char key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
static unsigned char iv[]  = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

int aes_api_cbc_encrypt_buffer(char * buffer, int len)
{
    if ((0 != len%16) || (len <= 0))
    {
        printf("invalid param %d \n", len);
        return -1;
    }
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, (unsigned char *)buffer, (unsigned int)len);

    return 0;
}

int aes_api_cbc_decrypt_buffer(char * buffer, int len)
{
    if ((0 != len%16) || (len <= 0))
    {
        printf("invalid param %d \n", len);
        return -1;
    }
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, (unsigned char *)buffer, (unsigned int)len);

    return 0;
}

int aes_api_unit_test(void)
{
    int i;
    char buffer[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    int ret = aes_api_cbc_encrypt_buffer(buffer, sizeof(buffer));
    if (0 == ret)
    {
        printf("input: \n");
        for (i=0; i<sizeof(buffer); i++)
        {
            printf("0x%x \n", buffer[i]);
        }
        printf("\n");
        aes_api_cbc_decrypt_buffer(buffer, sizeof(buffer));
        printf("output: \n");
        for (i=0; i<sizeof(buffer); i++)
        {
            printf("0x%x \n", buffer[i]);
        }
        printf("\n");
    }

    return 0;
}
