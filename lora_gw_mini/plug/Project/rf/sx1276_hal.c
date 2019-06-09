#include <string.h>
#include "stm8s.h"
#include "sx1276.h"
#include "sx1276_hal.h"
    
static recv_data_callback recv_cb = NULL;

static void  fn_send_byte(uint8 out);
static uint8 fn_spi_read_byte(void);
static void  fn_cmd_switch_en(CmdEntype_t cmd);
static void  fn_cmd_switch_pa(CmdPaType_t cmd);

// 接收到RF的数据
void fn_fqc_recv_data(uint8 *lpbuf, uint16 len);

static lpCtrlTypefunc_t  ctrlTypefunc = 
{
    fn_send_byte,
    fn_spi_read_byte,
    fn_cmd_switch_en,
    fn_cmd_switch_pa, // PA 功率放大器
    fn_fqc_recv_data
};

// 芯片复位
void sx1276_hal_reset(void)
{
    RF_REST_L;
    sx1276_delay_1s(2000);
    RF_REST_H;
    sx1276_delay_1s(500);
}

void sx1276_hal_set_recv_cb(recv_data_callback cb)
{
    recv_cb = cb;
}

void sx1276_hal_register_rf_func(void)
{
    rx1276_register_rf_func(&ctrlTypefunc);
}

void sx1276_hal_rf_send_packet(uint8 *rf_tran_buf, uint8 len)
{
    rx1276_rf_send_packet(rf_tran_buf, len);
}

uint8 sx1276_hal_get_send_flags(void)
{
    return sx1276_get_send_flags();
}

void sx1276_hal_set_send_flags(uint8 value)
{
    sx1276_set_send_flags(value);
}

void sx1276_hal_rx_mode(void)
{
    sx1276_rx_mode();
}

void sx1276_hal_lora_init(void)
{
    sx1276_lora_init();
}

////////////////////////////////////////////////////////////////////////////////
// static function
////////////////////////////////////////////////////////////////////////////////
static void fn_send_byte(uint8 out)
{
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
        out = (out << 1);           // shift 1 place for next bit
        RF_CKL_L;                   // toggle clock low
    }
}

static uint8 fn_spi_read_byte(void)
{
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
                                // toggle clock high
        RF_CKL_L;               // toggle clock low
    }

    return j;                   // toggle clock low //
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
