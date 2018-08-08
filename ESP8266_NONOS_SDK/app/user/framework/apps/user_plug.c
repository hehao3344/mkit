/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_plug.c
 *
 * Description: plug demo's function realization
 *
 * Modification history:
 *     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_plug.h"

LOCAL void ICACHE_FLASH_ATTR io_init(void);

/******************************************************************************
 * FunctionName : user_plug_set_status
 * Description  : set plug's status, 0x00 or 0x01
 * Parameters   : uint8 - status
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR user_switch_output( uint8 status )
{
    PLUG_STATUS_OUTPUT( PLUG_SWITCH_LED_IO_NUM, status );
}

/******************************************************************************
 * FunctionName : user_wifi_output
 * Description  : set plug's status, 0x00 or 0x01
 * Parameters   : uint8 - status
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR user_wifi_output( uint8 status )
{
    PLUG_STATUS_OUTPUT( PLUG_WIFI_LED_IO_NUM, status );
}

/******************************************************************************
 * FunctionName : user_plug_init
 * Description  : init plug's key function and relay output
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR user_switch_init( void )
{
    io_init();

    // wifi_status_led_install( PLUG_WIFI_LED_IO_NUM, PLUG_WIFI_LED_IO_MUX, PLUG_WIFI_LED_IO_FUNC );
}

////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
LOCAL void ICACHE_FLASH_ATTR io_init(void)
{
    PIN_FUNC_SELECT( PLUG_SWITCH_LED_IO_MUX, PLUG_SWITCH_LED_IO_FUNC );
    PIN_FUNC_SELECT( PLUG_WIFI_LED_IO_MUX,   PLUG_WIFI_LED_IO_FUNC );
}

