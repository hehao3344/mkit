#ifndef __SX1276_HAL_H
#define __SX1276_HAL_H

#include "../apps/gw_io.h"
#include "driver/spi_lora.h"

typedef void (* recv_data_callback)(char *buffer, unsigned short length);

// GPIO15 NSS
// GPIO14 SCK
// GPIO13 MOSI
// GPIO12 MISO
// gpio4  sx1278 irq status
// gpio2  crst

#undef USE_HSPI

#ifdef USE_HSPI
#define  RF_REST_L      gw_io_sx1278_rst_output(0)
#define  RF_REST_H      gw_io_sx1278_rst_output(1)

#define  RF_CE_L        spi_cs_output(0)  // gw_io_sx1278_cs_output(0)
#define  RF_CE_H        spi_cs_output(1)  // gw_io_sx1278_cs_output(1)

#define  SX1278_SDO      //gw_io_sx1278_miso_input()  // SPI输入

#define  RF_CKL_L        //gw_io_sx1278_sck_output(0)
#define  RF_CKL_H        //gw_io_sx1278_sck_output(1)

#define  RF_SDI_L        //gw_io_sx1278_mosi_output(0)
#define  RF_SDI_H        //gw_io_sx1278_mosi_output(1)
#else
#define  RF_REST_L       gw_io_sx1278_rst_output(0)
#define  RF_REST_H       gw_io_sx1278_rst_output(1)

#define  RF_CE_L         gw_io_sx1278_cs_output(0)
#define  RF_CE_H         gw_io_sx1278_cs_output(1)

#define  SX1278_SDO      gw_io_sx1278_miso_input()  // SPI输入

#define  RF_CKL_L        gw_io_sx1278_sck_output(0)
#define  RF_CKL_H        gw_io_sx1278_sck_output(1)

#define  RF_SDI_L        gw_io_sx1278_mosi_output(0)
#define  RF_SDI_H        gw_io_sx1278_mosi_output(1)
#endif

void sx1276_hal_set_recv_cb(recv_data_callback cb);
void sx1276_hal_init(void);
void sx1276_hal_send(uint8 *rf_tran_buf, uint8 len);
void sx1276_hal_rx_mode(void);
void sx1276_hal_receive_handle(void);
void sx1276_hal_set_send_flags(uint8 value);
uint8 sx1276_hal_get_send_flags(void);

#endif
