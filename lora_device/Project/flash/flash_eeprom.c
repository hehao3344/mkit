#include "flash_eeprom.h"
 
// 如果需要烧写MAC 需要在 stm8s.h中打开如下语句
// #define RAM_EXECUTION  (1) 

void flash_eeprom_init( void )
{
    // Define flash programming Time
    FLASH_SetProgrammingTime( FLASH_PROGRAMTIME_STANDARD );

    // Unlock Data memory
    FLASH_Unlock( FLASH_MEMTYPE_DATA );
}

// block 1 2
void flash_eeprom_write( uint8 block, uint8 * buffer, uint8 len )
{
    uint8 i;
    u8 write_buffer[FLASH_BLOCK_SIZE];
    // Çå¿Õ
    for ( i = 0; i < FLASH_BLOCK_SIZE; i++ )
    {
        write_buffer[i] = 0x00;
    }

    // Fill the buffer in RAM
    for ( i = 0; i < len; i++ )
    {
        write_buffer[i] = buffer[i];
    }

    //  Program the block 0
    // block 0 is first block of Data memory: address is 0x4000
    block = 0;

    FLASH_ProgramBlock( block, FLASH_MEMTYPE_DATA, FLASH_PROGRAMMODE_STANDARD, write_buffer );
    FLASH_WaitForLastOperation( FLASH_MEMTYPE_DATA );
}

uint8 flash_eeprom_read_char( uint32 offset )
{
    u32 start_add, stop_add;
    u8  read_value = 0;

    // Check the programmed block
    start_add = FLASH_DATA_START_PHYSICAL_ADDRESS; // 0x4000
    stop_add  = FLASH_DATA_START_PHYSICAL_ADDRESS + (u32)FLASH_BLOCK_SIZE;

    offset = ( FLASH_DATA_START_PHYSICAL_ADDRESS + offset );

    if ( ( offset >= start_add ) && ( offset < stop_add ) )
    {
        read_value = FLASH_ReadByte( offset );
    }

    return read_value;
}

boolean flash_eeprom_read_buf( uint32 offset, uint8 * buf, uint16 len )
{
    uint8 i;
    u32 start_add, stop_add;
 
    // 目前仅仅支持长度小于6的数据读取
    if ( len >= 6 )
    {
        return FALSE;
    }

    // Check the programmed block
    start_add = FLASH_DATA_START_PHYSICAL_ADDRESS; // 0x4000
    stop_add  = FLASH_DATA_START_PHYSICAL_ADDRESS + (u32)FLASH_BLOCK_SIZE;

    offset = ( FLASH_DATA_START_PHYSICAL_ADDRESS + offset );

    if ( ( offset >= start_add ) && ( offset < stop_add ) )
    {
        for ( i=0; i<len; i++ )
        {
            buf[i] = FLASH_ReadByte( offset + i );
        } 
    }

    return TRUE;
}

void flash_eeprom_erase( void )
{
    u32 add, start_add, stop_add ;
    u8 block = 0;

    // Check the programmed block
    start_add = FLASH_DATA_START_PHYSICAL_ADDRESS;
    stop_add  = FLASH_DATA_START_PHYSICAL_ADDRESS + (u32)FLASH_BLOCK_SIZE;
    // Erase block 0 and verify it
    FLASH_EraseBlock( block, FLASH_MEMTYPE_DATA );
    FLASH_WaitForLastOperation( FLASH_MEMTYPE_DATA );
    for ( add = start_add; add < stop_add; add++ )
    {
        if ( FLASH_ReadByte(add) != 0x00 )
        {
            // ´íÎó
        }
    }
}
