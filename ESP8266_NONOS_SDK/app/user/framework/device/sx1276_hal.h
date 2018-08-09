#ifndef __SX1276_HAL_H
#define __SX1276_HAL_H

#include "../core/core.h"
//#include "../protocol/protocol.h"

#define SWITCH_ON   //GPIO_WriteHigh( GPIOD, GPIO_PIN_6 )
#define SWITCH_OFF  //GPIO_WriteLow( GPIOD, GPIO_PIN_6 )
#define STATUS_ON   //GPIO_WriteHigh( GPIOD, GPIO_PIN_7 )
#define STATUS_OFF  //GPIO_WriteLow( GPIOD, GPIO_PIN_7 )

typedef struct RfPlugResult
{
    uint8 switch_invalid;       // 开启关闭 是否有效
    uint8 switch_on_off;
    uint8 match_cmd;            // 配对命令
    uint8 group;
    uint8 address;
} RfPlugResult;

void sx1276_hal_reset( void );
void sx1276_hal_register_rf_func( void );
void sx1276_hal_lora_init( void );
RfPlugResult * sx1276_hal_get_rf_result( void );
void sx1276_hal_rf_send_packet( uint8 *rf_tran_buf, uint8 len );
void sx1276_hal_recv_msg( void );
void sx1276_hal_clear_rf_result( void );

#endif
