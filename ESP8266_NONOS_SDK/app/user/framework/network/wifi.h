#ifndef __WIFI_H
#define __WIFI_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

void ICACHE_FLASH_ATTR wifi_set_mode( uint32 mode );
void ICACHE_FLASH_ATTR wifi_set_auth( int8 *ssid, int8 *password );
boolean ICACHE_FLASH_ATTR wifi_create( void );
void    ICACHE_FLASH_ATTR wifi_destroy( void );
void    ICACHE_FLASH_ATTR wifi_disconnect( void );
// boolean ICACHE_FLASH_ATTR wifi_get_status( void );

// ap mode operation.
//boolean ICACHE_FLASH_ATTR wifi_ap_set_param( int8 *ssid, int8 *password );

#ifdef __cplusplus
}
#endif

#endif
