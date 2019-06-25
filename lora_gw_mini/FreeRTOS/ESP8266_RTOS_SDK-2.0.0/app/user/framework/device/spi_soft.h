#ifndef __SOFT_SPI_H_
#define __SOFT_SPI_H_

#include "esp_common.h"

#define SPI_MISO	12
#define SPI_MOSI	13
#define SPI_SCK		14
#define SPI_CS		15

#define SOFT_SPI_INIT()	do{\
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);\
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);\
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);\
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);\
    GPIO_DIS_OUTPUT(SPI_MISO);\
} while(0)

#define MOSI_0()	GPIO_OUTPUT_SET(SPI_MOSI,   0)
#define MOSI_1()	GPIO_OUTPUT_SET(SPI_MOSI,   1)
#define CS_0()		GPIO_OUTPUT_SET(SPI_CS,     0)
#define CS_1()		GPIO_OUTPUT_SET(SPI_CS,     1)
#define SCK_0()		GPIO_OUTPUT_SET(SPI_SCK,    0)
#define SCK_1()		GPIO_OUTPUT_SET(SPI_SCK,    1)

#define MISO_IS_HIGH()	(GPIO_INPUT_GET(SPI_MISO) != 0)

u8 ICACHE_FLASH_ATTR   softspi_read_byte(void);
void ICACHE_FLASH_ATTR softspi_write_byte(u8 data);

#endif /* __SOFT_SPI_H_ */
