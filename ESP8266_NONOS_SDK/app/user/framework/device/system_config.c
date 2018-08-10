#include "system_config.h"

#ifndef BIT
#define BIT(x)	(1 << (x))
#endif

// 设备端的 GPIO 口配置情况如下:
// GPIOA1 --- LED 蓝色 显示插座的通断状态
// GPIOC4 --- LED 红色 显示插座的状态 长按按钮后 该灯会快闪 表示正在接收配对指令
//                     配对完成后 该灯长亮
// GPIOD1 --- 连接到sx1278的复位引脚 上电后将它拉高
// Key: GPIOD-PIN7 对应插座按钮 按一次翻转一次
//                 如果长按的话 会发起配对操作

static void clock_config( uint8 sys_clk );

// 初始化系统时钟
void system_config_clk_init( void )
{
    //clock_config( 16 ); // 内部时钟16M
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
   #if 0
   // 时钟配置为内部RC，16M
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
