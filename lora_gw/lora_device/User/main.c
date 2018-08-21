#include "stm8s.h"
#include "sys_mgr/sys_mgr.h"





//����CPU�ڲ�ʱ��
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
    CLOCK_Config(SYS_CLOCK);//ϵͳʱ�ӳ�ʼ��  
  
}
#ifndef BIT
#define BIT(x)	(1 << (x))
#endif

/*********************************************
�������ܣ�ϵͳ�ڲ�ʱ������
���������SYS_CLK : 2��4��8��16
�����������
��    ע��ϵͳ����Ĭ���ڲ�2�ͣȣ�
*********************************************/
void CLOCK_Config(u8 SYS_CLK)
{
   //ʱ������Ϊ�ڲ�RC��16M
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
        sys_mgr_send_msg();          // ÿ500ms����һ��
            
        // sys_mgr_handle_remote_msg(); // ����Զ����Ϣ
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

