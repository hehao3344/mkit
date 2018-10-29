#include "driver/spi_interface.h"

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

#define  SX1278_SDO      1 //GPIO_ReadInputPin(GPIOC, GPIO_PIN_7)  // SPI输入

#define  RF_REST_L       gw_io_sx1278_rst_output(0);
#define  RF_REST_H       gw_io_sx1278_rst_output(1); // GPIO_WriteHigh(GPIOC, GPIO_PIN_3)

#define  RF_CE_L         //GPIO_WriteLow(GPIOF, GPIO_PIN_4)
#define  RF_CE_H         //GPIO_WriteHigh(GPIOF, GPIO_PIN_4)
#define  RF_CKL_L        //GPIO_WriteLow(GPIOC, GPIO_PIN_5)
#define  RF_CKL_H        //GPIO_WriteHigh(GPIOC, GPIO_PIN_5)
#define  RF_SDI_L        //GPIO_WriteLow(GPIOC, GPIO_PIN_6)
#define  RF_SDI_H        //GPIO_WriteHigh(GPIOC, GPIO_PIN_6)

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

lpCtrlTypefunc_t  ctrlTypefunc = {
   fn_init,
   fn_send_byte,
   fn_spi_read_byte,
   fn_cmd_switch_en,
   fn_cmd_switch_pa, // PA 功率放大器
   fn_fqc_recv_data
};

void sx1276_hal_recv_msg(void)
{
    sx1278_recv_handle();
}

void sx1276_hal_init(void)
{
    SpiAttr hSpiAttr;
    hSpiAttr.bitOrder = SpiBitOrder_MSBFirst;
    hSpiAttr.speed    = SpiSpeed_10MHz;
    hSpiAttr.mode     = SpiMode_Master;
    hSpiAttr.subMode  = SpiSubMode_0;

    // Init HSPI GPIO
    WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2);//configure io to spi mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);//configure io to spi mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);//configure io to spi mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2);//configure io to spi mode

    SPIInit(SpiNum_HSPI, &hSpiAttr);
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

void sx1276_hal_register_rf_func(void)
{
    rx1276_register_rf_func(&ctrlTypefunc);
}

void sx1276_hal_rf_send_packet(uint8 *rf_tran_buf, uint8 len)
{
     rx1276_rf_send_packet(rf_tran_buf, len);
}

void sx1276_hal_lora_init(void)
{
    sx1276_lora_init();
}

////////////////////////////////////////////////////////////////////////////////
// static function
////////////////////////////////////////////////////////////////////////////////
static void fn_init(void)
{
    sx1276_hal_init();
}

static void fn_send_byte(uint8 addr, uint8 out)
{
    SpiData spi_data;

    os_printf("\r\n =============   spi init master   ============= \r\n");

//  Test 8266 slave.Communication format: 1byte command + 1bytes address + x bytes Data.
    os_printf("\r\n Master send 1 bytes data to slave(sx1278)\r\n");

    spiData.cmd     = MASTER_WRITE_DATA_TO_SLAVE_CMD;
    spiData.cmdLen  = 1;
    spiData.addr    = &addr;
    spiData.addrLen = 1;
    spiData.data    = out;
    spiData.dataLen = 1;
    SPIMasterSendData(SpiNum_HSPI, &spiData);

    // spi_mast_byte_write(SPI, out);
}

static uint8 fn_spi_read_byte(uint8 addr)
{
    uint8 read_bytes;
    os_printf("\r\n Master receive 24 bytes data from slave(8266)\r\n");
    spiData.cmd = MASTER_READ_DATA_FROM_SLAVE_CMD;
    spiData.cmdLen = 1;
    spiData.addr   = &addr;
    spiData.addrLen = 1;
    spiData.data    = &read_bytes;
    spiData.dataLen = 1;

    SPIMasterRecvData(SpiNum_HSPI, &spiData);

    os_printf("Recv Slave data0[0x%x]\r\n", read_bytes);

    uint32 value = SPIMasterRecvStatus(SpiNum_HSPI);
    os_printf("\r\n Master read slave(sx1278) status[0x%02x]\r\n", value);

    SPIMasterSendStatus(SpiNum_HSPI, 0x99);
    os_printf("\r\n Master write status[0x99] to slavue(8266).\r\n");
    // SHOWSPIREG(SpiNum_HSPI);

#if 0
    uint8 i, j;

    j = 0;
    for (i = 0; i < 8; i++)
    {
        RF_CKL_H;
        j = (j << 1);           // shift 1 place to the left or shift in 0
        if (SX1278_SDO)       // check to see if bit is high
        {
            j = j | 0x01;       // if high, make bit high
        }
                                // toggle clock high
        RF_CKL_L;               // toggle clock low
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
