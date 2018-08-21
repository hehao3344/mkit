#ifndef __USER_ESPSWITCH_H__
#define __USER_ESPSWITCH_H__

#include "../../../include/driver/key.h"

// gpio0/13 key
// gpio4    switch
// gpio5    wifi status uart0tx

#if 0
#define PLUG_KEY_NUM            1
#define PLUG_KEY_0_IO_MUX       PERIPHS_IO_MUX_MTCK_U
#define PLUG_KEY_0_IO_NUM       13
#define PLUG_KEY_0_IO_FUNC      FUNC_GPIO13
#else
#define PLUG_KEY_NUM            1
#define PLUG_KEY_0_IO_MUX       PERIPHS_IO_MUX_GPIO0_U
#define PLUG_KEY_0_IO_NUM       0
#define PLUG_KEY_0_IO_FUNC      FUNC_GPIO0
#endif

#define PLUG_SWITCH_LED_IO_MUX  PERIPHS_IO_MUX_GPIO4_U
#define PLUG_SWITCH_LED_IO_NUM  5
#define PLUG_SWITCH_LED_IO_FUNC FUNC_GPIO5


#define PLUG_WIFI_LED_IO_MUX    PERIPHS_IO_MUX_GPIO5_U
#define PLUG_WIFI_LED_IO_NUM    4
#define PLUG_WIFI_LED_IO_FUNC   FUNC_GPIO4

#if 0
#define PLUG_WIFI_LED_IO_MUX    PERIPHS_IO_MUX_U0TXD_U
#define PLUG_WIFI_LED_IO_NUM    1
#define PLUG_WIFI_LED_IO_FUNC   FUNC_GPIO1
#endif

#define PLUG_STATUS_OUTPUT(pin, on)     GPIO_OUTPUT_SET(pin, on)

void ICACHE_FLASH_ATTR user_switch_init( void );
void ICACHE_FLASH_ATTR user_wifi_output( uint8 status );

#endif

