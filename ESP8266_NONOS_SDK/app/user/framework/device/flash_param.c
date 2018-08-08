#include <stdio.h>
#include <string.h>

#include "user_interface.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"
#include "../core/platform.h"

#include "flash_param.h"

// page1 flash map: 16+124+4+16 = 160
// name       length  offset
// id:           8      0
// password:     16     8
// ssid:         16     24
// sta_password: 32     40
// week_plan:    20     72
// svr_url:      32     92
// svr_ip:       16     124
// svr_url:      2      140
// svr_ip:       2      142
// dev_name:     16     160

//////////////////////////////////////////////////////////////////////////////////
// page1: get function.
//////////////////////////////////////////////////////////////////////////////////
static void ICACHE_FLASH_ATTR read_all( int8 * buffer, uint16 len );

static int8 rbuffer[256];

void ICACHE_FLASH_ATTR flash_param_get_id( int8 *value )
{    
    read_all(rbuffer, 256);
    os_memcpy(value, &rbuffer[0], ID_LEN);
}

void ICACHE_FLASH_ATTR flash_param_get_password( int8 *value )
{
    read_all( rbuffer, 256 );

    int8 *test_pad = "admin";
    read_all( rbuffer, 256 );

    os_memcpy( value, test_pad, 5 );
    // os_memcpy( value, &rbuffer[8], 16 ); //
}

void ICACHE_FLASH_ATTR flash_param_get_ssid( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( value, &rbuffer[24], 16 );
}

void ICACHE_FLASH_ATTR flash_param_get_sta_password( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( value, &rbuffer[40], 32 );
}

void ICACHE_FLASH_ATTR flash_param_get_week_plan( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( value, &rbuffer[72], 20 );
}

void ICACHE_FLASH_ATTR flash_param_get_svr_url( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( value, &rbuffer[92], 32 );
}

void ICACHE_FLASH_ATTR flash_param_get_svr_ip( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( value, &rbuffer[124], 16 );
}

void ICACHE_FLASH_ATTR flash_param_get_version( uint16 * sv, uint16 * hv )
{
    uint16 value = 0;
    read_all( rbuffer, 256 );

    os_memcpy( &value, &rbuffer[140], 2 );
    *sv = value;

    os_memcpy( &value, &rbuffer[142], 2 );
    *hv = value;
}

void ICACHE_FLASH_ATTR flash_param_get_devname( int8 *value )
{
    int8 *test_dev = "home";
    read_all( rbuffer, 256 );

    os_memcpy( value, test_dev, 4 );
    //os_memcpy( value, &rbuffer[144], 16 );
}

//////////////////////////////////////////////////////////////////////////////////
// page1: set function.
//////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR flash_param_set_id( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( &rbuffer[0], value, ID_LEN );

    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	     ( uint32 * )rbuffer, 256 );
}

void ICACHE_FLASH_ATTR flash_param_set_password( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( &rbuffer[8], value, 16 );
    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	    ( uint32 * )rbuffer, 256 );
}

void ICACHE_FLASH_ATTR flash_param_set_ssid( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( &rbuffer[24], value, 16 );
    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	    ( uint32 * )rbuffer, 256 );
}

void ICACHE_FLASH_ATTR flash_param_set_sta_password( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( &rbuffer[40], value, 32 );
    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	    ( uint32 * )rbuffer, 256 );
}

void ICACHE_FLASH_ATTR flash_param_set_week_plan( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( &rbuffer[72], value, 20 );
    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	    ( uint32 * )rbuffer, 256 );
}

void ICACHE_FLASH_ATTR flash_param_set_svr_url( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( &rbuffer[92], value, 32 );
    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	    ( uint32 * )rbuffer, 256 );
}

void ICACHE_FLASH_ATTR flash_param_set_svr_ip( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( &rbuffer[124], value, 16 );
    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	     ( uint32 * )rbuffer, 256 );
}

void ICACHE_FLASH_ATTR flash_param_set_version( uint16 sv, uint16 hv )
{
    read_all( rbuffer, 256 );

    os_memcpy( &rbuffer[140], &sv, 2 );
    os_memcpy( &rbuffer[144], &hv, 2 );

    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	     ( uint32 * )rbuffer, 256 );
}

void ICACHE_FLASH_ATTR flash_param_set_devname( int8 *value )
{
    read_all( rbuffer, 256 );

    os_memcpy( &rbuffer[144], value, 16 );
    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	     ( uint32 * )rbuffer, 256 );
}
////////////////////////////////////////////////////////////////////////////////
// basic param section.
////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR flash_param_get_basic_param( int8 *value )
{
    os_memset( rbuffer, 0, sizeof( rbuffer ) );
    spi_flash_read( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	    ( uint32 * )rbuffer, 256 );
    os_memcpy( value, rbuffer, 256 );
}

void ICACHE_FLASH_ATTR flash_param_set_basic_param( int8 *rbuffer )
{
    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	     ( uint32 * )rbuffer, 256 );
}

////////////////////////////////////////////////////////////////////////////////
// default section.
////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR flash_param_set_default_basic_param( int8 * rbuffer )
{
    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_DFT_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_DFT_BASIC ) * SPI_FLASH_SEC_SIZE,
            	     ( uint32 * )rbuffer, 256 );
}

////////////////////////////////////////////////////////////////////////////////
// recover default section.
////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR flash_param_default_restore( void )
{
    os_memset( rbuffer, 0, sizeof( rbuffer ) );

    spi_flash_read( ( BASIC_PARAM_SEC + USER_PARAM_DFT_BASIC ) * SPI_FLASH_SEC_SIZE,
            	    ( uint32 * )rbuffer, 256 );

    spi_flash_erase_sector( BASIC_PARAM_SEC + USER_PARAM_BASIC );
    spi_flash_write( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	     ( uint32 * )rbuffer, 256 );
}

static void ICACHE_FLASH_ATTR read_all( int8 * buffer, uint16 len )
{
    os_memset( buffer, 0, len );
    spi_flash_read( ( BASIC_PARAM_SEC + USER_PARAM_BASIC ) * SPI_FLASH_SEC_SIZE,
            	    ( uint32 * )buffer, 256 );
}
