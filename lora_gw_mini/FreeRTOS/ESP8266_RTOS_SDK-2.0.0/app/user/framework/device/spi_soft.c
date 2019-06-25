#include "spi_soft.h"

/* spi write one byte  */
void ICACHE_FLASH_ATTR softspi_write_byte(u8 data)
{
    u8 i;
    //CS_0();
	for(i = 0; i < 8; i++){
		if (data & 0x80){
			MOSI_1();
		}else{
			MOSI_0();
		}
		SCK_0();
		data <<= 1;
		SCK_1();
	}
	//CS_1();
}

/* spi read one byte */
u8 ICACHE_FLASH_ATTR softspi_read_byte(void)
{
	u8 read = 0;
    u8 i;
    //CS_0();
	for (i = 0; i < 8; i++){
		SCK_0();
		read = read<<1;
		if (MISO_IS_HIGH()){
			read++;
		}
		SCK_1();
	}
	// CS_1();
	return read;
}
