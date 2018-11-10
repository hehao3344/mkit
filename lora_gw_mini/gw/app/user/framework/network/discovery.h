#ifndef __DISCOVERY_H
#define __DISCOVERY_H

#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEV_UUID_LEN        (16)
#define BROADCASE_PORT      (12088)

typedef struct _DISCOVER_ENV
{
    char  dev_uuid[DEV_UUID_LEN];   /* 中控设备的ID   */
    int   port;                     /* tcp bind port. */
} DISCOVER_ENV;


int  ICACHE_FLASH_ATTR discovery_create(DISCOVER_ENV *env);
void ICACHE_FLASH_ATTR discovery_destroy( void );

#ifdef __cplusplus
}
#endif

#endif // __DISCOVERY_H
