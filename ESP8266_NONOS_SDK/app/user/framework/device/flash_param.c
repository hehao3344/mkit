#include <stdio.h>
#include <string.h>

#include "user_interface.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"

#include "flash_param.h"

#define DEV_UUID_LEN            16

// page1 flash map: 

//////////////////////////////////////////////////////////////////////////////////
// page1: get function.
//////////////////////////////////////////////////////////////////////////////////
static void ICACHE_FLASH_ATTR read_all(char * buffer, unsigned short len);

static int8 rbuffer[256];

/* Ã¿¸ö16×Ö½Ú */
void ICACHE_FLASH_ATTR flash_param_get_dev_uuid(int index, char *buffer, int len)
{
    if ((len < DEV_UUID_LEN) || (index >= 4))
    {
        return;
    }
    
    read_all(rbuffer, 256);
    os_memcpy(buffer, &rbuffer[index*DEV_UUID_LEN], DEV_UUID_LEN);
}

//////////////////////////////////////////////////////////////////////////////////
// page1: set function.
//////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR flash_param_set_dev_uuid(int index, char *buffer, int len)
{
    if ((len < DEV_UUID_LEN) || (index >= 4))
    {
        return;
    }
    
    read_all(rbuffer, 256);
    os_memcpy(&rbuffer[index*DEV_UUID_LEN], buffer, DEV_UUID_LEN);

    spi_flash_erase_sector(BASIC_PARAM_SEC + USER_PARAM_BASIC);
    spi_flash_write((BASIC_PARAM_SEC + USER_PARAM_BASIC) * SPI_FLASH_SEC_SIZE,
            	     (unsigned int *)rbuffer, 256);
}

static void ICACHE_FLASH_ATTR read_all(char * buffer, unsigned short len)
{
    os_memset(buffer, 0, len);
    spi_flash_read((BASIC_PARAM_SEC + USER_PARAM_BASIC) * SPI_FLASH_SEC_SIZE,
                   (unsigned int *)buffer, 256);
}
