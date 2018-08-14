#include "stm8s.h"
#include "system_config.h"

#ifndef BIT
#define BIT(x)	(1 << (x))
#endif


// �����ܽŶ���
// MISO         PC7
// MOSI         PC6
// SCK          PC5
// NSS(SPI_CS)  PC4 

// CE_RST       PC3
// IRQ_DID0     PC2

// �������� 
// ����         PB7
// ״̬         PD0
// ����         PB6

static void clock_config( uint8 sys_clk );

// ��ʼ��ϵͳʱ��
void system_config_clk_init( void )
{
    //clock_config( 16 ); // �ڲ�ʱ��16M    
    clock_config( 8 ); // �ڲ�ʱ��8M

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

    GPIO_DeInit( GPIOB );
    GPIO_DeInit( GPIOC );
    GPIO_DeInit( GPIOD );
    // GPIO_DeInit( GPIOE );

    // �ж����� û�õ��ж� ֻ�õ��˵�ƽ�ж�
    // GPIO_Init( GPIOC, GPIO_PIN_2, GPIO_MODE_IN_PU_NO_IT );          
    GPIO_Init( GPIOC, GPIO_PIN_2, GPIO_MODE_IN_PU_IT );         // GIO0
    EXTI_SetExtIntSensitivity( EXTI_PORT_GPIOC,EXTI_SENSITIVITY_RISE_ONLY ); 
    // EXTI_SetTLISensitivity( EXTI_TLISENSITIVITY_FALL_ONLY );

    GPIO_Init( GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST ); // CE SX1278 - RST
    GPIO_Init( GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST ); // CS
    GPIO_Init( GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST );  // SCK
    GPIO_Init( GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST ); // SDI
    GPIO_Init( GPIOC, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT );      // SDO


    // ����״̬ �� �����̵�������
    GPIO_Init( GPIOD, GPIO_PIN_0, GPIO_MODE_OUT_PP_HIGH_FAST ); // �Ƿ��յ��ź� ״̬
    GPIO_Init( GPIOB, GPIO_PIN_6, GPIO_MODE_OUT_PP_LOW_FAST ); // �̵���  ��������Ϊ�͵�ƽ �����һ�ϵ� �̵������ǿ���״̬

    // GPIO_Init( GPIOB, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT );      // ���� ÿ��һ�� ��תһ��
    GPIO_Init( GPIOB, GPIO_PIN_7, GPIO_MODE_IN_PU_IT );
    EXTI_SetExtIntSensitivity( EXTI_PORT_GPIOB, EXTI_SENSITIVITY_RISE_FALL ); // EXTI_SENSITIVITY_RISE_ONLY ); 
    // EXTI_SetTLISensitivity( EXTI_TLISENSITIVITY_RISE_ONLY );

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
   // ʱ������Ϊ�ڲ�RC��16M
   CLK->CKDIVR &= ~ (BIT(4)|BIT(3));

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
}
