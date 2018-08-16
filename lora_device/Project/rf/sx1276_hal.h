#ifndef __SX1276_HAL_H
#define __SX1276_HAL_H

#include "stm8s.h"
#include "../core/core.h"
#include "../protocol/protocol.h"

typedef void (* recv_data_callback)(char *buffer, unsigned short length);

// SPI 接口
// MISO         PC7
// MOSI         PC6
// SCK          PC5
// NSS(SPI_CS)  PC4 

// CE_RST       PC3
// IRQ_DID0     PC2

// 其他引脚 
// 按键         PB7
// 状态         PD0
// 开关         PB6

#define  RF_REST_L       GPIO_WriteLow( GPIOC, GPIO_PIN_3 )
#define  RF_REST_H       GPIO_WriteHigh( GPIOC, GPIO_PIN_3 )

#define  RF_CE_L         GPIO_WriteLow( GPIOC, GPIO_PIN_4 )     // SPI_CS
#define  RF_CE_H         GPIO_WriteHigh( GPIOC, GPIO_PIN_4 )

#define  SX1278_SDO      GPIO_ReadInputPin( GPIOC, GPIO_PIN_7 )  // SPI输入

#define  RF_CKL_L        GPIO_WriteLow( GPIOC, GPIO_PIN_5 )
#define  RF_CKL_H        GPIO_WriteHigh( GPIOC, GPIO_PIN_5 )
#define  RF_SDI_L        GPIO_WriteLow( GPIOC, GPIO_PIN_6 )
#define  RF_SDI_H        GPIO_WriteHigh( GPIOC, GPIO_PIN_6 )

#define  SX1278_IRQ      GPIO_ReadInputPin( GPIOC, GPIO_PIN_2 )

// 功率放大 不用
//#define  PA_TXD_OUT()            
//#define  PA_RXD_OUT()   
#define KEY_STATUS      GPIO_ReadInputPin( GPIOB, GPIO_PIN_7 )

// 低电平亮 高电平灭
#define STATUS_ON       GPIO_WriteLow( GPIOD, GPIO_PIN_0 )
#define STATUS_OFF      GPIO_WriteHigh( GPIOD, GPIO_PIN_0 )

#define SWITCH_ON       GPIO_WriteHigh( GPIOB, GPIO_PIN_6 )
#define SWITCH_OFF      GPIO_WriteLow( GPIOB, GPIO_PIN_6 )

void sx1276_hal_set_recv_cb(recv_data_callback cb);
void sx1276_hal_reset( void );
void sx1276_hal_register_rf_func( void );
void sx1276_hal_lora_init( void );
void sx1276_hal_rf_send_packet( uint8 *rf_tran_buf, uint8 len );
void sx1276_hal_rx_mode( void );

#endif
