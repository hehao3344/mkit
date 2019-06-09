#include <string.h>
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "driver/spi_lora.h"
#include "sx1276.h"
#include "sx1276_hal.h"

static recv_data_callback recv_cb = NULL;

static void  fn_send_byte(uint8 out);
static uint8 fn_spi_read_byte(void);
static void  fn_cmd_switch_en(CmdEntype_t cmd);
static void  fn_cmd_switch_pa(CmdPaType_t cmd);
static void  sx1276_reset(void);

// 接收到RF的数据
static void fn_fqc_recv_data(uint8 *lpbuf, uint16 len);

static lpCtrlTypefunc_t  ctrlTypefunc =
{
    fn_send_byte,
    fn_spi_read_byte,
    fn_cmd_switch_en,
    fn_cmd_switch_pa, // PA 功率放大器
    fn_fqc_recv_data
};

void sx1276_hal_set_recv_cb(recv_data_callback cb)
{
    recv_cb = cb;
}

void sx1276_hal_send(uint8 *rf_tran_buf, uint8 len)
{
    rx1276_rf_send_packet(rf_tran_buf, len);
}

void sx1276_hal_rx_mode(void)
{
    sx1276_rx_mode();
}

uint8 sx1276_hal_get_send_flags(void)
{
    return sx1276_get_send_flags();
}

void sx1276_hal_set_send_flags(uint8 value)
{
    sx1276_set_send_flags(value);
}

void sx1276_hal_receive_handle(void)
{
    sx1278_recv_handle();
}

void sx1276_hal_init(void)
{
#ifdef USE_HSPI
    spi_init(HSPI);
    spi_mode(HSPI, 1, 1);
    spi_init_gpio(HSPI, 0);
    spi_tx_byte_order(HSPI, 0);
    spi_rx_byte_order(HSPI, 0);
#else
#endif
    sx1276_reset();
    rx1276_register_rf_func(&ctrlTypefunc);
    sx1276_lora_init();

}

////////////////////////////////////////////////////////////////////////////////
// static function
////////////////////////////////////////////////////////////////////////////////
static void fn_send_byte(uint8 out)
{
#ifdef USE_HSPI
    spi_tx8(HSPI, out);
#else
    uint8 i;
    for (i = 0; i < 8; i++)
    {
        if (out & 0x80)           // check if MSB is high
        {
            RF_SDI_H;
        }
        else
        {
            RF_SDI_L;               // if not, set to low
        }

        RF_CKL_H;                   // toggle clock high
        //clk_delay();
        out = (out << 1);           // shift 1 place for next bit
        RF_CKL_L;                   // toggle clock low
        //clk_delay();
    }
#endif
}

static uint8 fn_spi_read_byte(void)
{

#ifdef USE_HSPI
    return spi_rx8(HSPI);
#else
    uint8 i, j;

    j = 0;
    for (i = 0; i < 8; i++)
    {
        RF_CKL_H;
        j = (j << 1);           // shift 1 place to the left or shift in 0
        if (SX1278_SDO)         // check to see if bit is high
        {
            j = j | 0x01;       // if high, make bit high
        }
        //clk_delay();            // toggle clock high
        RF_CKL_L;               // toggle clock low
        //clk_delay();
    }

    return j;                   // toggle clock low //
#endif
}

static void fn_cmd_switch_en(CmdEntype_t cmd)
{
    switch(cmd)
    {
        case EN_OPEN:
        {
            RF_CE_L;
        }
        break;
        case EN_CLOSE:
        {
            RF_CE_H;
        }
        break;
        default:
            break;
    }
}

// 没用上 暂时不处理
static void fn_cmd_switch_pa(CmdPaType_t cmd)
{
#if 0
    switch(cmd)
    {
        case RX_OPEN:
        {
            PA_RXD_OUT();
        }
        break;
        case TX_OPEN:
        {
            PA_TXD_OUT();
        }
        break;

        default:
            break;
    }
#endif
}

// 接收到RF的数据
static void fn_fqc_recv_data(uint8 *buffer, uint16 len)
{
    if (NULL != recv_cb)
    {
        recv_cb((char *)buffer, len);
    }
}

// 芯片复位
static void sx1276_reset(void)
{
    RF_REST_L;
    sx1276_delay_1s(2000);
    RF_REST_H;
    sx1276_delay_1s(500);
}
