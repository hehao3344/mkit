#include "stm8s.h"
#include "system_config.h"

#ifndef BIT
#define BIT(x)	(1 << (x))
#endif


// 插座管脚定义
// MISO         PC7
// MOSI         PC6
// SCK          PC5
// NSS(SPI_CS)  PC4 

// CE_RST       PC3
// IRQ_DID0     PC2

// 其他引脚 
// 按键         PB7
// 状态         PD0
// 开关         PB6

static void clock_config( uint8 sys_clk );

// 初始化系统时钟
void system_config_clk_init( void )
{
    //clock_config( 16 ); // 内部时钟16M    
    clock_config( 8 ); // 内部时钟8M

#if 0
    CLK_DeInit();
    CLK_FastHaltWakeUpCmd( ENABLE );
    CLK_HSECmd( DISABLE );
    CLK_HSICmd( ENABLE );
    CLK_SYSCLKConfig( CLK_PRESCALER_HSIDIV4 );
#endif
}

// 初始化GPIO
void system_config_gpio_config(void)
{

    GPIO_DeInit( GPIOB );
    GPIO_DeInit( GPIOC );
    GPIO_DeInit( GPIOD );
    // GPIO_DeInit( GPIOE );

    // 中断引脚 没用到中断 只用到了电平判断
    // GPIO_Init( GPIOC, GPIO_PIN_2, GPIO_MODE_IN_PU_NO_IT );          
    GPIO_Init( GPIOC, GPIO_PIN_2, GPIO_MODE_IN_PU_IT );         // GIO0
    EXTI_SetExtIntSensitivity( EXTI_PORT_GPIOC,EXTI_SENSITIVITY_RISE_ONLY ); 
    // EXTI_SetTLISensitivity( EXTI_TLISENSITIVITY_FALL_ONLY );

    GPIO_Init( GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST ); // CE SX1278 - RST
    GPIO_Init( GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST ); // CS
    GPIO_Init( GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST );  // SCK
    GPIO_Init( GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST ); // SDI
    GPIO_Init( GPIOC, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT );      // SDO


    // 插座状态 和 插座继电器控制
    GPIO_Init( GPIOD, GPIO_PIN_0, GPIO_MODE_OUT_PP_HIGH_FAST ); // 是否收到信号 状态
    GPIO_Init( GPIOB, GPIO_PIN_6, GPIO_MODE_OUT_PP_LOW_FAST ); // 继电器  必须设置为低电平 否则会一上电 继电器就是开的状态

    // GPIO_Init( GPIOB, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT );      // 按键 每按一次 翻转一次
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
   
    GPIO_Init( GPIOD, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST ); // 继电器
    GPIO_Init( GPIOD, GPIO_PIN_7, GPIO_MODE_OUT_PP_HIGH_FAST ); // 是否收到信号
    GPIO_Init( GPIOD, GPIO_PIN_5, GPIO_MODE_IN_PU_NO_IT );      // 按键 每按一次 翻转一次
#endif

}

/*******************************************************************************
函数功能：系统内部时钟配置
输入参数：SYS_CLK : 2、4、8、16
输出参数：无
备    注：系统启动默认内部2ＭＨＺ
*******************************************************************************/
static void clock_config( uint8 sys_clk )
{
   // 时钟配置为内部RC，16M
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
