#include "stm8s.h"
#include "sys_mgr/sys_mgr.h"





//定义CPU内部时钟
#define  SYS_CLOCK    16


void CLOCK_Config(u8 SYS_CLK);
void All_Congfig(void);


int main(void)
{     
      All_Congfig();
               
      while(1)
      {         
      }
        
}


void All_Congfig(void)
{
    CLOCK_Config(SYS_CLOCK);//系统时钟初始化  
  
}
#ifndef BIT
#define BIT(x)	(1 << (x))
#endif

/*********************************************
函数功能：系统内部时钟配置
输入参数：SYS_CLK : 2、4、8、16
输出参数：无
备    注：系统启动默认内部2ＭＨＺ
*********************************************/
void CLOCK_Config(u8 SYS_CLK)
{
   //时钟配置为内部RC，16M
   CLK->CKDIVR &=~(BIT(4)|BIT(3));
  
   switch(SYS_CLK)
   {
      case 2: CLK->CKDIVR |=((1<<4)|(1<<3)); break;
      case 4: CLK->CKDIVR |=(1<<4); break;
      case 8: CLK->CKDIVR |=(1<<3); break;
   }
}


















void main1( void )
{
    //sys_mgr_init();
    //enableInterrupts();
    
    while( 1 )
    {
        sys_mgr_send_msg();          // 每500ms发送一次
            
        // sys_mgr_handle_remote_msg(); // 处理远程消息
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

