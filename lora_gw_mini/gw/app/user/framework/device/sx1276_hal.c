#include "../apps/gw_io.h"
// #include "driver/spi_interface.h"

#include "sx1276.h"
#include "sx1276_hal.h"

// SPI 接口
//PC7 (HS)/SPI_MISO
//PC6 (HS)/SPI_MOSI
//PC5 (HS)/SPI_SCK


// 硬件连接
// GPIOC 7 --- MISO
// GPIOC 3 --- 模块复位引脚
// GPIOF 4 --- CE
// GPIOC 5 --- SPI_CLK
// GPIOC 6 --- MOSI
// PA = 功率放大器 参考 安信可的代码 没用到

#define  RF_REST_L       
#define  RF_REST_H       gw_io_sx1278_rst_output(1); // GPIO_WriteHigh(GPIOC, GPIO_PIN_3)


// 功率放大 不用
#define  PA_TXD_OUT()
#define  PA_RXD_OUT()

static void  fn_send_byte(uint8 out);
static uint8 fn_spi_read_byte(void);
static void  fn_cmd_switch_en(CmdEntype_t cmd);
static void  fn_cmd_switch_pa(CmdPaType_t cmd);

static recv_data_callback recv_cb = NULL;

// 接收到RF的数据
static void fn_fqc_recv_data(uint8 *lpbuf, uint16 len);


void sx1276_hal_recv_msg(void)
{
    sx1278_recv_handle();
}

void sx1276_hal_set_recv_cb(recv_data_callback cb)
{
    recv_cb = cb;
}

// 芯片复位
void sx1276_hal_reset(void)
{
   RF_REST_L;
   sx1276_delay_1s(20000);
   RF_REST_H;
   sx1276_delay_1s(5000);
}

void sx1276_hal_rf_send_packet(uint8 *rf_tran_buf, uint8 len)
{
     rx1276_rf_send_packet(rf_tran_buf, len);
}

////////////////////////////////////////////////////////////////////////////////
// static function
////////////////////////////////////////////////////////////////////////////////
static void fn_cmd_switch_en(CmdEntype_t cmd)
{
    switch(cmd)
    {
        case EN_OPEN:
        {
            //RF_CE_L;
        }
        break;
        case EN_CLOSE:
        {
            //RF_CE_H;
        }
        break;
        default:
            break;
    }
}

// 没用上 暂时不处理
static void fn_cmd_switch_pa(CmdPaType_t cmd)
{
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
}

// 接收到RF的数据
static void fn_fqc_recv_data(uint8 *buffer, uint16 len)
{
    os_printf("get buf %s len %d \n", buffer, len);
    if (NULL != recv_cb)
    {
        recv_cb(buffer, len);
    }
}
