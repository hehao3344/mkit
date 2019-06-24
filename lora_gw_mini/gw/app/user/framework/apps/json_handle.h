#ifndef __JSON_HANDLE_H
#define __JSON_HANDLE_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    E_REGISTER_RESP = 0x01,
    E_HEART_BEAT_RESP,
    E_FW_UPGRADE_CMD,
    E_SET_SWITCH_CMD,
    E_GET_SUB_DEV_CMD,
} E_JSON_CMD;

typedef void (* json_msg_cb)(void * arg, E_JSON_CMD e_cmd, int req_id, int int_param, char * char_param);

void ICACHE_FLASH_ATTR json_handle_handle_data(char *buffer, int length);
void ICACHE_FLASH_ATTR json_handle_set_callback(json_msg_cb cb, void * arg);

#ifdef __cplusplus
}
#endif

#endif //__JSON_HANDLE_H
