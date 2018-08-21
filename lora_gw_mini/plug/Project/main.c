#include "stm8s.h"
#include "sys_mgr/sys_mgr.h"


void main( void )
{
    sys_mgr_init();
    enableInterrupts();
    
    while( 1 )
    {
        sys_mgr_send_msg();          // 每500ms发送一次            
    }
}

#ifdef  USE_FULL_ASSERT
void assert_failed( uint8_t* file, uint32_t line )
{
    while (1)
    {
    }
}
#endif

