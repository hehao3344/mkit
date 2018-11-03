#ifndef __APP_CC_H
#define __APP_CC_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

int ICACHE_FLASH_ATTR  app_cc_create(void);
void ICACHE_FLASH_ATTR app_cc_set_param(char *dev_uuid, int status, int on_off);
void ICACHE_FLASH_ATTR app_cc_destroy(void);

#ifdef __cplusplus
}
#endif

#endif //__APP_CC_H
