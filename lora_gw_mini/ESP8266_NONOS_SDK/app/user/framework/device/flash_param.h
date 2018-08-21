#ifndef __FLASH_PARAM_H
#define __FLASH_PARAM_H

#define CONFIG_DONE_ID         "MOC10101"
#define CONFIG_RESET_ID        "HH101010"

#define BASIC_PARAM_SEC		    0x3C
#define USER_PARAM_BASIC        0
#define USER_PARAM_DFT_BASIC    1

#ifdef __cplusplus
extern "C" {
#endif

void ICACHE_FLASH_ATTR flash_param_get_dev_uuid(int index, char *buffer, int len);
void ICACHE_FLASH_ATTR flash_param_set_dev_uuid(int index, char *buffer, int len);

#ifdef __cplusplus
}
#endif

#endif // __FLASH_PARAM_H
