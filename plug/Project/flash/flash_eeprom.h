#ifndef __FLASH_EEPROM_H
#define __FLASH_EEPROM_H

#include "../core/core.h"

#include "stm8s.h"
#include "stm8s_flash.h"

#ifdef __cplusplus
extern "C"
{
#endif

void flash_eeprom_init( void );

// block 1 2
void flash_eeprom_write( uint8 block, uint8 * buffer, uint8 len );

uint8 flash_eeprom_read_char( uint32 offset );

boolean flash_eeprom_read_buf( uint32 offset, uint8 * buf, uint16 len );

void flash_eeprom_erase( void );

#ifdef __cplusplus
}
#endif

#endif
