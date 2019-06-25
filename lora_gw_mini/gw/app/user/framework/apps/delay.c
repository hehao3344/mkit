#include <stdio.h>
#include <string.h>

#include "os_type.h"
#include "osapi.h"
#include "mem.h"

#include "../core/core.h"
#include "delay.h"

void ICACHE_FLASH_ATTR os_delay_ms(unsigned int value)
{
	unsigned int i, j;
	for(i=0; i<value; i++)
	{
		for(j = 0; j < 1000; j ++)
		{
			os_delay_us(1);
		}
	}
}
