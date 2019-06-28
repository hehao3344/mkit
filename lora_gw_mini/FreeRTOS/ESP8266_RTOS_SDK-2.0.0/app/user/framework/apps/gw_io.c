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
#include "esp_common.h"
#include "../device/sx1276_hal.h"
#include "gw_io.h"

typedef struct _KEY_PARAM {
    uint8 key_level;
    uint8 gpio_id;
    uint8 gpio_func;
    uint32 gpio_name;
    os_timer_t key_5s;
    os_timer_t key_50ms;
    key_press_function short_press;
    key_press_function long_press;
} KEY_PARAM;

static KEY_PARAM key_param;

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
void ICACHE_FLASH_ATTR gw_io_sx1278_cs_output(uint8 on_off)
{
    GPIO_OUTPUT_SET(GW_SX1278_CS_IO_NUM, on_off);
}

void ICACHE_FLASH_ATTR gw_io_sx1278_sck_output(uint8 on_off)
{
    GPIO_OUTPUT_SET(GW_SX1278_SCK_IO_NUM, on_off);
}

void ICACHE_FLASH_ATTR gw_io_sx1278_mosi_output(uint8 on_off)
{
    GPIO_OUTPUT_SET(GW_SX1278_MOSI_IO_NUM, on_off);
}

uint8 ICACHE_FLASH_ATTR gw_io_sx1278_miso_input(void)
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
void ICACHE_FLASH_ATTR gw_io_init(key_press_function cb_short_press, key_press_function cb_long_press)
{
    GPIO_ConfigTypeDef gpio_in_cfg;                     //Define GPIO Init Structure

    // 按键
    gpio_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_NEGEDGE;  //
    gpio_in_cfg.GPIO_Mode     = GPIO_Mode_Input;        //Input mode
    gpio_in_cfg.GPIO_Pullup   = GPIO_PullUp_EN;
    gpio_in_cfg.GPIO_Pin      = GPIO_Pin_0;             // Enable GPIO
    gpio_config(&gpio_in_cfg);                          //Initialization function

    // sx1278 irq
    gpio_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_HILEVEL;  //
    gpio_in_cfg.GPIO_Mode     = GPIO_Mode_Input;        //Input mode
    gpio_in_cfg.GPIO_Pullup   = GPIO_PullUp_EN;
    gpio_in_cfg.GPIO_Pin      = GPIO_Pin_4;             // Enable GPIO
    //gpio_config(&gpio_in_cfg);

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

    key_param.gpio_id   = GW_KEY_0_IO_NUM;
    key_param.gpio_name = GW_KEY_0_IO_MUX;
    key_param.gpio_func = GW_KEY_0_IO_FUNC;
    key_param.long_press  = cb_long_press;
    key_param.short_press = cb_short_press;

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, TRUE);
    gpio_intr_handler_register(gpio_intr_handler, NULL); // Register the interrupt function
    _xt_isr_unmask(1 << ETS_GPIO_INUM);                 //Enable the GPIO interrupt

    printf("gpio set init now \n");

#if 0
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

    key_param.gpio_id   = GW_KEY_0_IO_NUM;
    key_param.gpio_name = GW_KEY_0_IO_MUX;
    key_param.gpio_func = GW_KEY_0_IO_FUNC;
    key_param.long_press  = cb_long_press;
    key_param.short_press = cb_short_press;

    // key irq input
    PIN_FUNC_SELECT(GW_KEY_0_IO_MUX, GW_KEY_0_IO_FUNC);
    //设置GPIO12为输入状态
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(GW_KEY_0_IO_NUM));
    // MTDI_U引脚启动上拉电阻
    PIN_PULLUP_EN(GW_KEY_0_IO_MUX);

    // SX1278 irq input.
    PIN_FUNC_SELECT(GW_SX1278_IRQ_IO_MUX, GW_SX1278_IRQ_IO_FUNC);
    //设置GPIO12为输入状态
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(GW_SX1278_IRQ_IO_NUM));
    // MTDI_U引脚启动上拉电阻
    PIN_PULLUP_EN(GW_SX1278_IRQ_IO_MUX);

    // 全局关闭GPIO中断
    // ETS_GPIO_INTR_DISABLE();
    _xt_isr_mask(1 << ETS_GPIO_INUM);

    //设置中断函数
    gpio_intr_handler_register(gpio_intr_handler, NULL); // Register the interrupt function

    // 设置中断触发方式：X电平触发
    gpio_pin_intr_state_set(GPIO_ID_PIN(GW_SX1278_IRQ_IO_NUM),  GPIO_PIN_INTR_HILEVEL);

    // 按键
    gpio_pin_intr_state_set(GPIO_ID_PIN(GW_KEY_0_IO_NUM),  GPIO_PIN_INTR_POSEDGE);

    // ETS_GPIO_INTR_ENABLE();

    _xt_isr_unmask(1 << ETS_GPIO_INUM);
#endif

}


////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
LOCAL void ICACHE_FLASH_ATTR key_5s_cb(KEY_PARAM * single_key)
{
    os_timer_disarm(&single_key->key_5s);
    os_printf("long press enter\n");
    // low, then restart
    if (0 == GPIO_INPUT_GET(GPIO_ID_PIN(single_key->gpio_id)))
    {
        os_printf("long press ===> \n");
        if (NULL != single_key->long_press)
        {
            single_key->long_press();
        }
    }
}

/******************************************************************************
 * FunctionName : key_50ms_cb
 * Description  : 50ms timer callback to check it's a real key push
 * Parameters   : single_key_param *single_key - single key parameter
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR key_50ms_cb(KEY_PARAM * single_key)
{
    // os_timer_disarm(&single_key->key_50ms);
    os_printf("short press \n");
    // high, then key is up
    if (1 == GPIO_INPUT_GET(GPIO_ID_PIN(single_key->gpio_id)))
    {
        os_timer_disarm(&single_key->key_50ms);
        single_key->key_level = 1;

        // 按键
        GPIO_ConfigTypeDef gpio_in_cfg;                     //Define GPIO Init Structure
        gpio_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_NEGEDGE;  //
        gpio_in_cfg.GPIO_Mode     = GPIO_Mode_Input;        //Input mode
        gpio_in_cfg.GPIO_Pullup   = GPIO_PullUp_EN;
        gpio_in_cfg.GPIO_Pin      = GPIO_Pin_0;             // Enable GPIO
        gpio_config(&gpio_in_cfg);

        if (NULL != single_key->short_press)
        {
            single_key->short_press();
        }
    }
    else
    {
        // 按键
        GPIO_ConfigTypeDef gpio_in_cfg;                     //Define GPIO Init Structure
        gpio_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_POSEDGE;  //
        gpio_in_cfg.GPIO_Mode     = GPIO_Mode_Input;        //Input mode
        gpio_in_cfg.GPIO_Pullup   = GPIO_PullUp_EN;
        gpio_in_cfg.GPIO_Pin      = GPIO_Pin_0;             // Enable GPIO
        gpio_config(&gpio_in_cfg);
    }
}

static void gpio_intr_handler()
{
    // uint32 gpio_status;

    //_xt_isr_mask(1<<ETS_GPIO_INUM);                     //disable interrupt
    //GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, TRUE);   //clear interrupt mask
    //_xt_isr_unmask(1 << ETS_GPIO_INUM);                 //Enable the GPIO interrupt

	//gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	//GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
	//printf("gpio_interrupt_cb \n");

#if 1
    /* 读取GPIO中断状态 */
    u32 pin_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    printf("gpio intr enter 0x%x \n", pin_status);

    /** 关闭GPIO中断 */
    _xt_isr_mask(1<<ETS_GPIO_INUM);    //disable interrupt

    /** 检测是否已开关输入引脚中断 */
    if (pin_status & BIT(GW_SX1278_IRQ_IO_NUM))
    {
        //sx1276_hal_receive_handle();
    }

    if (pin_status & BIT(GW_KEY_0_IO_NUM))
    {
        if (key_param.key_level == 1)
        {
            // 5s, restart & enter softap mode
            os_timer_disarm(&key_param.key_5s);
            os_timer_setfn(&key_param.key_5s, (os_timer_func_t *)key_5s_cb, &key_param);
            os_timer_arm(&key_param.key_5s, 5000, 0);
            key_param.key_level = 0;
            // 按键
            GPIO_ConfigTypeDef gpio_in_cfg;                     //Define GPIO Init Structure
            gpio_in_cfg.GPIO_IntrType = GPIO_PIN_INTR_POSEDGE;  //
            gpio_in_cfg.GPIO_Mode     = GPIO_Mode_Input;        //Input mode
            gpio_in_cfg.GPIO_Pullup   = GPIO_PullUp_EN;
            gpio_in_cfg.GPIO_Pin      = GPIO_Pin_0;             // Enable GPIO
            gpio_config(&gpio_in_cfg);
        }
        else
        {
            // 50ms, check if this is a real key up
            os_timer_disarm(&key_param.key_50ms);
            os_timer_setfn(&key_param.key_50ms, (os_timer_func_t *)key_50ms_cb, &key_param);
            os_timer_arm(&key_param.key_50ms, 50, 0);
        }
    }

    /** 清除GPIO中断标志 */
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, pin_status);

    /* 开启GPIO中断 */
    _xt_isr_unmask(1 << ETS_GPIO_INUM); //Enable the GPIO interrupt
#endif
}
