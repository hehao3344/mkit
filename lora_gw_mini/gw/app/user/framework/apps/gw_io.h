#ifndef __HW_IO_H
#define __HW_IO_H

#include "../../../include/driver/key.h"

// gpio0  key
// gpio5  system status
// gpio16 wifi   status
// gpio4  sx1278 irq status
// gpio2  crst

#define PLUG_KEY_NUM            1

// hardware connection
#define GW_KEY_NUM              1
#define GW_KEY_0_IO_MUX         PERIPHS_IO_MUX_GPIO0_U
#define GW_KEY_0_IO_NUM         0
#define GW_KEY_0_IO_FUNC        FUNC_GPIO0

#define GW_STATUS_IO_MUX        PERIPHS_IO_MUX_GPIO5_U
#define GW_STATUS_IO_NUM        5
#define GW_STATUS_IO_FUNC       FUNC_GPIO5

#define GW_SX1278_IO_MUX        PERIPHS_IO_MUX_GPIO2_U
#define GW_SX1278_IO_NUM        2
#define GW_SX1278_IO_FUNC       FUNC_GPIO2

#define GW_SX1278_IRQ_IO_MUX    PERIPHS_IO_MUX_GPIO4_U
#define GW_SX1278_IRQ_IO_NUM    4
#define GW_SX1278_IRQ_IO_FUNC   FUNC_GPIO4

void ICACHE_FLASH_ATTR gw_io_init(void);
void ICACHE_FLASH_ATTR gw_io_status_output(uint8 on_off);
void ICACHE_FLASH_ATTR gw_io_wifi_output(uint8 on_off);
void ICACHE_FLASH_ATTR gw_io_sx1278_rst_output(uint8 on_off);

#endif

