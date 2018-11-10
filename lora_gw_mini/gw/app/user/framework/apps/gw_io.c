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

// GPIO16���Ƹ�λ
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
// GPIO_PIN_INTR_DISABLE = 0,  //��ʹ���ж�
// GPIO_PIN_INTR_POSEDGE = 1,  //������
// GPIO_PIN_INTR_NEGEDGE = 2,  //�½���
// GPIO_PIN_INTR_ANYEDGE = 3,  //˫����
// GPIO_PIN_INTR_LOLEVEL = 4,  //�͵�ƽ
// GPIO_PIN_INTR_HILEVEL = 5   //�ߵ�ƽ
void ICACHE_FLASH_ATTR gw_io_init( void )
{
    PIN_FUNC_SELECT(GW_STATUS_IO_MUX, GW_STATUS_IO_FUNC);
    PIN_FUNC_SELECT(GW_WIFI_IO_MUX,   GW_WIFI_IO_FUNC);

    // ���ܽ�����ΪGPIO��
    PIN_FUNC_SELECT(GW_SX1278_IRQ_IO_MUX, GW_SX1278_IRQ_IO_FUNC);

    // ����GPIO5Ϊ����״̬
    GPIO_DIS_OUTPUT(GW_SX1278_IRQ_IO_FUNC);  // ʹ���������

    // ����������������
    PIN_PULLUP_EN(GW_SX1278_IRQ_IO_MUX);

    // �����жϺ���
    ETS_GPIO_INTR_ATTACH(&gpio_intr_handler, NULL);

    //�����жϴ�����ʽ �½���
    gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_NEGEDGE);

    // ȫ�ֹر�GPIO�ж�
    ETS_GPIO_INTR_DISABLE();

    // ��������ŵ�GPIO�жϱ�־
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS,  BIT(5));

    // �ж�ʹ��
    ETS_GPIO_INTR_ENABLE();

    PIN_PULLUP_EN(GW_SX1278_IRQ_IO_MUX);   // ʹ������

    gpio_output_set(0, 0, 0, BIT5);     // ʹ������
}

////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
static void gpio_intr_handler()
{
    // ��ȡGPIO״̬�Ĵ�������ȡ�ж���Ϣ
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    // �ر�GPIO�ж�
    ETS_GPIO_INTR_DISABLE();


    // ����ж���Ϣ
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    if (gpio_status & (BIT(5)))
    {
    }
    // ����GPIO�ж�
    ETS_GPIO_INTR_ENABLE();
}
