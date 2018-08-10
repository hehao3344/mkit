#include "system_config.h"

#ifndef BIT
#define BIT(x)	(1 << (x))
#endif

// �豸�˵� GPIO �������������:
// GPIOA1 --- LED ��ɫ ��ʾ������ͨ��״̬
// GPIOC4 --- LED ��ɫ ��ʾ������״̬ ������ť�� �õƻ���� ��ʾ���ڽ������ָ��
//                     �����ɺ� �õƳ���
// GPIOD1 --- ���ӵ�sx1278�ĸ�λ���� �ϵ��������
// Key: GPIOD-PIN7 ��Ӧ������ť ��һ�η�תһ��
//                 ��������Ļ� �ᷢ����Բ���

static void clock_config( uint8 sys_clk );

// ��ʼ��ϵͳʱ��
void system_config_clk_init( void )
{
    //clock_config( 16 ); // �ڲ�ʱ��16M
#if 0
    CLK_DeInit();
    CLK_FastHaltWakeUpCmd( ENABLE );
    CLK_HSECmd( DISABLE );
    CLK_HSICmd( ENABLE );
    CLK_SYSCLKConfig( CLK_PRESCALER_HSIDIV4 );
#endif
}

// ��ʼ��GPIO
void system_config_gpio_config(void)
{
    //spi_master_init(SPI);
    
    #if 0
    GPIO_DeInit( GPIOA );
    GPIO_DeInit( GPIOB );
    GPIO_DeInit( GPIOC );    
    GPIO_DeInit( GPIOD );
    GPIO_DeInit( GPIOE );    
    
    GPIO_Init( GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST ); //RESET
        
    GPIO_Init( GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST );  //SCK    
    GPIO_Init( GPIOC, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT );      //SDO
    GPIO_Init( GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST ); //SDI
    GPIO_Init( GPIOF, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST ); //CS
   
    GPIO_Init( GPIOC, GPIO_PIN_4, GPIO_MODE_IN_PU_IT );         //GIO0
    EXTI_SetExtIntSensitivity( EXTI_PORT_GPIOC,EXTI_SENSITIVITY_RISE_ONLY ); // Interrupt on Rising edge only
    EXTI_SetTLISensitivity( EXTI_TLISENSITIVITY_FALL_ONLY );
   
    GPIO_Init( GPIOD, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST ); // �̵���
    GPIO_Init( GPIOD, GPIO_PIN_7, GPIO_MODE_OUT_PP_HIGH_FAST ); // �Ƿ��յ��ź�
    GPIO_Init( GPIOD, GPIO_PIN_5, GPIO_MODE_IN_PU_NO_IT );      // ���� ÿ��һ�� ��תһ��
    #endif
}

/*******************************************************************************
�������ܣ�ϵͳ�ڲ�ʱ������
���������SYS_CLK : 2��4��8��16
�����������
��    ע��ϵͳ����Ĭ���ڲ�2�ͣȣ�
*******************************************************************************/
static void clock_config( uint8 sys_clk )
{
   #if 0
   // ʱ������Ϊ�ڲ�RC��16M
   CLK->CKDIVR &=~(BIT(4)|BIT(3));

   switch( sys_clk )
   {
        case 2:
        {
            CLK->CKDIVR |=((1<<4)|(1<<3));
            break;
        }
        case 4:
        {
            CLK->CKDIVR |=(1<<4);
            break;
        }
        case 8:
        {
            CLK->CKDIVR |=(1<<3);
            break;
        }
        default:
            break;
   }
   #endif
}
