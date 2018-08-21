#include <stdio.h>

#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "wifi.h"

typedef struct WifiObject
{
    int32 wifi_mode;
    int8  ssid[16];
    int8  password[32];
} WifiObject;

static WifiObject * instance( void );

boolean ICACHE_FLASH_ATTR wifi_create( void )
{
    WifiObject * handle = instance();

    int8   len;
    struct station_config station_conf;
    int32  wifi_mode = wifi_get_opmode();

    os_printf( "wifi mode %d \n", wifi_mode );

    ETS_UART_INTR_DISABLE();
    wifi_set_opmode( handle->wifi_mode );
    ETS_UART_INTR_ENABLE();

    // wifi auth
    wifi_station_disconnect();

    os_memset( &station_conf, 0, sizeof( struct station_config ) );

    if ( os_strlen( handle->ssid ) > 0 )
    {
        len = os_strcpy( &station_conf.ssid, handle->ssid );
        if ( len != -1 )
        {
            if ( os_strlen( handle->password ) > 0 )
            {
                os_strcpy( &station_conf.password, handle->password );
            }
        }

        ETS_UART_INTR_DISABLE();
        wifi_station_set_config( &station_conf );
        ETS_UART_INTR_ENABLE();
        wifi_station_connect();

        os_printf( "wifi start ... \n" );
    }

    return TRUE;
}

void ICACHE_FLASH_ATTR wifi_destroy( void )
{
    WifiObject * handle = instance();
    if ( NULL != handle )
    {
        os_free( handle );
    }
}

void ICACHE_FLASH_ATTR wifi_set_mode( uint32 mode )
{
    WifiObject * handle = instance();
    handle->wifi_mode = mode;
}

void ICACHE_FLASH_ATTR wifi_set_auth( int8 *ssid, int8 *password )
{
    WifiObject * handle = instance();

    os_memset( handle->ssid, 0, sizeof( handle->ssid ) );
    os_memset( handle->password, 0, sizeof( handle->password ) );

    os_strcpy( handle->ssid, ssid );
    if ( os_strlen( password ) > 0 )
    {
        os_strcpy( handle->password, password );
    }
}

void ICACHE_FLASH_ATTR wifi_disconnect( void )
{
    WifiObject * handle = instance();

    wifi_station_disconnect();
}

boolean ICACHE_FLASH_ATTR wifi_get_status( void )
{
    WifiObject * handle = instance();

    return FALSE;
}

//////////////////////////////////////////////////////////////////////
// static function.
//////////////////////////////////////////////////////////////////////
static WifiObject * instance( void )
{
    static WifiObject *handle = NULL;
    if ( NULL == handle )
    {
        handle = ( WifiObject * )os_zalloc( sizeof( WifiObject ) );
    }

    return handle;
}
