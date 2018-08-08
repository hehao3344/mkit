#ifndef __FLASH_PARAM_H
#define __FLASH_PARAM_H

#include "../core/core.h"

#define CONFIG_DONE_ID         "MOC10101"
#define CONFIG_RESET_ID        "HH101010"

#define BASIC_PARAM_SEC		    0x3C

// #define SYSTEM_DEFAULT_SEC      0x7C // 0x7c000 = 507904 / 4096 = 124 = 0x7C

#define USER_PARAM_BASIC        0
#define USER_PARAM_DFT_BASIC    1

#ifdef __cplusplus
extern "C" {
#endif

void ICACHE_FLASH_ATTR flash_param_get_id(int8 *value);
void ICACHE_FLASH_ATTR flash_param_get_devname(int8 *value);
void ICACHE_FLASH_ATTR flash_param_get_username(int8 *value);
void ICACHE_FLASH_ATTR flash_param_get_password(int8 *value);
void ICACHE_FLASH_ATTR flash_param_get_ssid(int8 *value);
void ICACHE_FLASH_ATTR flash_param_get_sta_password(int8 *value);
void ICACHE_FLASH_ATTR flash_param_get_week_plan(int8 *value);
void ICACHE_FLASH_ATTR flash_param_get_svr_url(int8 *value);
void ICACHE_FLASH_ATTR flash_param_get_svr_ip(int8 *value);
void ICACHE_FLASH_ATTR flash_param_get_version(uint16 * sv, uint16 * hv);
void ICACHE_FLASH_ATTR flash_param_get_devname(int8 *value);

void ICACHE_FLASH_ATTR flash_param_set_id( int8 *value );
void ICACHE_FLASH_ATTR flash_param_set_devname( int8 *value );
void ICACHE_FLASH_ATTR flash_param_set_username( int8 *value );
void ICACHE_FLASH_ATTR flash_param_set_password( int8 *value );
void ICACHE_FLASH_ATTR flash_param_set_ssid( int8 *value );
void ICACHE_FLASH_ATTR flash_param_set_sta_password( int8 *value );
void ICACHE_FLASH_ATTR flash_param_set_week_plan( int8 *value );
void ICACHE_FLASH_ATTR flash_param_set_svr_url( int8 *value );
void ICACHE_FLASH_ATTR flash_param_set_svr_ip( int8 *value );

void ICACHE_FLASH_ATTR flash_param_get_basic_param( int8 *value );
void ICACHE_FLASH_ATTR flash_param_set_basic_param( int8 *rbuffer );
void ICACHE_FLASH_ATTR flash_param_set_default_basic_param( int8 * rbuffer );
void ICACHE_FLASH_ATTR flash_param_default_restore( void );
void ICACHE_FLASH_ATTR flash_param_set_version( uint16 sv, uint16 hv );
void ICACHE_FLASH_ATTR flash_param_set_devname( int8 *value );

#ifdef __cplusplus
}
#endif

#endif // __FLASH_PARAM_H
