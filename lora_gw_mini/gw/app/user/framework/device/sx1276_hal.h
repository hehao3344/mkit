#ifndef __SX1276_HAL_H
#define __SX1276_HAL_H

#include "../core/core.h"



void sx1276_hal_init(void);
void sx1276_hal_reset(void);

void sx1276_hal_register_rf_func(void);
void sx1276_hal_lora_init(void);
void sx1276_hal_rf_send_packet(uint8 *rf_tran_buf, uint8 len);
void sx1276_hal_recv_msg(void);
void sx1276_hal_clear_rf_result(void);

#endif
