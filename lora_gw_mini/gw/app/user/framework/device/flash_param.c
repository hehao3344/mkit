#include <stdio.h>
#include <string.h>

#include "user_interface.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"

#include "flash_param.h"

// page1 flash map:

//////////////////////////////////////////////////////////////////////////////////
// page1: get function.
//////////////////////////////////////////////////////////////////////////////////
static void ICACHE_FLASH_ATTR read_all(void);

// Fatal exception 9(LoadStoreAlignmentCause): 一定要定义成 unsigned int 类型的
static unsigned int rbuffer[MAX_PARAM_BUF_LEN];

void ICACHE_FLASH_ATTR flash_param_get_reset_flags(char *buffer, int len)
{
    if (len < RESET_FLAGS_LEN)
    {
        return;
    }

    read_all();
    
    os_memcpy(buffer, &rbuffer[RESET_FLAGS_OFFSET], RESET_FLAGS_LEN);
}

/* 每个X字节 */
void ICACHE_FLASH_ATTR flash_param_get_dev_mac(int index, char *buffer, int len)
{
    if ((len < DEV_MAC_LEN) || (index >= MAX_DEV_COUNT))
    {
        return;
    }

    read_all();
    os_memcpy(buffer, &rbuffer[index*DEV_MAC_LEN], DEV_MAC_LEN);
}

void ICACHE_FLASH_ATTR flash_param_get_cc_mac(char *buffer, int len)
{
    if (len < DEV_MAC_LEN)
    {
        return;
    }

    read_all();
    os_memcpy(buffer, &rbuffer[CC_MAC_OFFSET], DEV_MAC_LEN);
}

//////////////////////////////////////////////////////////////////////////////////
// page1: set function.
//////////////////////////////////////////////////////////////////////////////////
void ICACHE_FLASH_ATTR flash_param_set_dev_mac(int index, char *buffer, int len)
{
    if ((len < DEV_MAC_LEN) || (index >= MAX_DEV_COUNT))
    {
        return;
    }

    read_all();
    os_memcpy(&rbuffer[index*DEV_MAC_LEN], buffer, DEV_MAC_LEN);
    
    spi_flash_erase_protect_disable();
    spi_flash_erase_sector(BASIC_PARAM_SEC + USER_PARAM_BASIC);

    spi_flash_write((BASIC_PARAM_SEC + USER_PARAM_BASIC) * SPI_FLASH_SEC_SIZE,
            	    rbuffer, sizeof(rbuffer));
}

void ICACHE_FLASH_ATTR flash_param_set_cc_mac(char *buffer, int len)
{
    if (len < DEV_MAC_LEN)
    {
        return;
    }

    read_all();
    os_memcpy(&rbuffer[CC_MAC_OFFSET], buffer, DEV_MAC_LEN);

    spi_flash_erase_sector(BASIC_PARAM_SEC + USER_PARAM_BASIC);
    spi_flash_write((BASIC_PARAM_SEC + USER_PARAM_BASIC) * SPI_FLASH_SEC_SIZE,
            	     (unsigned int *)rbuffer, sizeof(rbuffer));
}

void ICACHE_FLASH_ATTR flash_param_set_reset_flags(char *buffer, int len)
{
    if (len < RESET_FLAGS_LEN)
    {
        return;
    }

    read_all();
    os_memcpy(&rbuffer[RESET_FLAGS_OFFSET], buffer, RESET_FLAGS_LEN);
    
    spi_flash_erase_protect_disable();
    spi_flash_erase_sector(BASIC_PARAM_SEC + USER_PARAM_BASIC);
    
    spi_flash_erase_sector(BASIC_PARAM_SEC + USER_PARAM_BASIC);
    spi_flash_write((BASIC_PARAM_SEC + USER_PARAM_BASIC) * SPI_FLASH_SEC_SIZE,
            	    rbuffer, sizeof(rbuffer));
}

static void ICACHE_FLASH_ATTR read_all(void)
{    
    spi_flash_read((BASIC_PARAM_SEC + USER_PARAM_BASIC) * SPI_FLASH_SEC_SIZE,
                   (unsigned int *)rbuffer, sizeof(rbuffer));
    
}
