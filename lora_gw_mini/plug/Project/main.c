#include "stm8s.h"
#include "sys_mgr/sys_mgr.h"


void main( void )
{
    sys_mgr_init();
    enableInterrupts();
    
    while( 1 )
    {
        sys_mgr_send_msg();          // ÿ500ms����һ��            
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

