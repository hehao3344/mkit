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
#include "../device/sx1276.h"
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
    /* gpio16 需要单独控制 */
    gpio16_output_set(on_off);
}

void ICACHE_FLASH_ATTR gw_io_sx1278_rst_output(uint8 on_off)
{
    GPIO_OUTPUT_SET(GW_SX1278_IO_NUM, on_off);
}

// SPI
void gw_io_sx1278_cs_output(uint8 on_off)
{
    GPIO_OUTPUT_SET(GW_SX1278_CS_IO_NUM, on_off);
}

void gw_io_sx1278_sck_output(uint8 on_off)
{
    GPIO_OUTPUT_SET(GW_SX1278_SCK_IO_NUM, on_off);
}

void gw_io_sx1278_mosi_output(uint8 on_off)
{
    GPIO_OUTPUT_SET(GW_SX1278_MOSI_IO_NUM, on_off);
}

uint8 gw_io_sx1278_miso_input(void)
{
    return  (uint8)GPIO_INPUT_GET(GPIO_ID_PIN(GW_SX1278_MISO_IO_NUM));
}

/******************************************************************************
 * FunctionName : user_plug_init
 * Description  : init plug's key function and relay output
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
// https://blog.csdn.net/freelifewe/article/details/62039263
// GPIO_PIN_INTR_DISABLE = 0,   // 不使用中断
// GPIO_PIN_INTR_POSEDGE = 1,   // 上升沿
// GPIO_PIN_INTR_NEGEDGE = 2,   // 下降沿
// GPIO_PIN_INTR_ANYEDGE = 3,   // 双边沿
// GPIO_PIN_INTR_LOLEVEL = 4,   // 低电平
// GPIO_PIN_INTR_HILEVEL = 5    // 高电平
void ICACHE_FLASH_ATTR gw_io_init(void)
{
    /* wifi status */
    gpio16_output_conf();
    /* system status */
    PIN_FUNC_SELECT(GW_STATUS_IO_MUX, GW_STATUS_IO_FUNC);
    /* 1278 reset */
    PIN_FUNC_SELECT(GW_SX1278_IO_MUX, GW_SX1278_IO_FUNC);

    // GPIO-SPI init
    PIN_FUNC_SELECT(GW_SX1278_CS_IO_MUX,   GW_SX1278_CS_IO_FUNC);
    PIN_FUNC_SELECT(GW_SX1278_SCK_IO_MUX,  GW_SX1278_SCK_IO_FUNC);
    PIN_FUNC_SELECT(GW_SX1278_MOSI_IO_MUX, GW_SX1278_MOSI_IO_FUNC);
    PIN_FUNC_SELECT(GW_SX1278_MISO_IO_MUX, GW_SX1278_MISO_IO_FUNC);

    // SX1278 irq input.
    // 将MTDI_U管脚设置为GPIO口
    PIN_FUNC_SELECT(GW_SX1278_IRQ_IO_MUX, GW_SX1278_IRQ_IO_FUNC);
    //设置GPIO12为输入状态
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(GW_SX1278_IRQ_IO_NUM));
    // MTDI_U引脚启动上拉电阻
    PIN_PULLUP_EN(GW_SX1278_IRQ_IO_MUX);
    // 全局关闭GPIO中断
    ETS_GPIO_INTR_DISABLE();
    //设置中断函数
    ETS_GPIO_INTR_ATTACH(&gpio_intr_handler, NULL);
    // 设置中断触发方式：低电平触发
    gpio_pin_intr_state_set(GPIO_ID_PIN(GW_SX1278_IRQ_IO_NUM),  GPIO_PIN_INTR_ANYEDGE);
    ETS_GPIO_INTR_ENABLE();
}

////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
LOCAL void gpio_intr_handler(void *arg)
{
    /** 读取GPIO中断状态 */
    u32 pin_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    os_printf("enter =222  in 0x%x \n", pin_status);
    /** 关闭GPIO中断 */
    ETS_GPIO_INTR_DISABLE();

    /** 清除GPIO中断标志 */
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, pin_status);

    /** 检测是否已开关输入引脚中断 */
    if (pin_status & BIT(GW_SX1278_IRQ_IO_NUM))
    {
        // sx1278_recv_handle();
    }

    /** 开启GPIO中断 */
    ETS_GPIO_INTR_ENABLE();
}

#if 0
static void gpio_intr_handler(void *arg)
{
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    ETS_GPIO_INTR_DISABLE();

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    os_printf("get status %d \n", gpio_status);

    if (gpio_status & (BIT(4)))
    {
        os_printf("get status %d \n", gpio_status);

        sx1278_recv_handle();
    }

    ETS_GPIO_INTR_ENABLE();
}
#endif

