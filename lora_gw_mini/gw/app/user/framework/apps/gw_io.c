/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_plug.c
 *
 * Description: plug demo's function realization
 *
 * Modification history:
 *     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "gw_io.h"

static void gpio_intr_handler();

/******************************************************************************
 * FunctionName : gw_io_status_output
 * Description  : set plug's status, 0x00 or 0x01
 * Parameters   : uint8 - status
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR gw_io_status_output(uint8 on_off)
{
    GPIO_OUTPUT_SET(GW_STATUS_IO_NUM, on_off);
}

/******************************************************************************
 * FunctionName : gw_io_wifi_output
 * Description  : set plug's status, 0x00 or 0x01
 * Parameters   : uint8 - status
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR gw_io_wifi_output(uint8 on_off)
{
    GPIO_OUTPUT_SET(GW_WIFI_IO_NUM, on_off);
}

// GPIO16控制复位
void ICACHE_FLASH_ATTR gw_io_sx1278_rst_output(uint8 on_off)
{

}

/******************************************************************************
 * FunctionName : user_plug_init
 * Description  : init plug's key function and relay output
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
// https://blog.csdn.net/freelifewe/article/details/62039263
// GPIO_PIN_INTR_DISABLE = 0,  //不使用中断
// GPIO_PIN_INTR_POSEDGE = 1,  //上升沿
// GPIO_PIN_INTR_NEGEDGE = 2,  //下降沿
// GPIO_PIN_INTR_ANYEDGE = 3,  //双边沿
// GPIO_PIN_INTR_LOLEVEL = 4,  //低电平
// GPIO_PIN_INTR_HILEVEL = 5   //高电平
void ICACHE_FLASH_ATTR gw_io_init( void )
{
    PIN_FUNC_SELECT(GW_STATUS_IO_MUX, GW_STATUS_IO_FUNC);
    PIN_FUNC_SELECT(GW_WIFI_IO_MUX,   GW_WIFI_IO_FUNC);

    // 将管脚设置为GPIO口
    PIN_FUNC_SELECT(GW_SX1278_IRQ_IO_MUX, GW_SX1278_IRQ_IO_FUNC);

    // 设置GPIO5为输入状态
    GPIO_DIS_OUTPUT(GW_SX1278_IRQ_IO_FUNC);  // 使能输出功能

    // 引脚启动上拉电阻
    PIN_PULLUP_EN(GW_SX1278_IRQ_IO_MUX);

    // 设置中断函数
    ETS_GPIO_INTR_ATTACH(&gpio_intr_handler, NULL);

    //设置中断触发方式 下降沿
    gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_NEGEDGE);

    // 全局关闭GPIO中断
    ETS_GPIO_INTR_DISABLE();

    // 清除该引脚的GPIO中断标志
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS,  BIT(5));

    // 中断使能
    ETS_GPIO_INTR_ENABLE();

    PIN_PULLUP_EN(GW_SX1278_IRQ_IO_MUX);   // 使能上拉

    gpio_output_set(0, 0, 0, BIT5);     // 使能输入
}

////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
static void gpio_intr_handler()
{
    // 读取GPIO状态寄存器，获取中断信息
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    // 关闭GPIO中断
    ETS_GPIO_INTR_DISABLE();


    // 清除中断信息
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    if (gpio_status & (BIT(5)))
    {
    }
    // 开启GPIO中断
    ETS_GPIO_INTR_ENABLE();
}
