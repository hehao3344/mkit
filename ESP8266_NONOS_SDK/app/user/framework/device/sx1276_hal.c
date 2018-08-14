#include "sx1276.h"
#include "sx1276_hal.h"

// SPI �ӿ�
//PC7 (HS)/SPI_MISO
//PC6 (HS)/SPI_MOSI
//PC5 (HS)/SPI_SCK

// GPIO D7 --- ��ɫ LED ״ָ̬ʾ �źű�ʾ ������ʾ���յ��ź�
// GPIO D6 --- ��ɫ LED �̵���״̬ 
// GPIO D5 --- ���� ÿ��һ�μ̵�����תһ��
// GPIO C4 --- GPIO�������� �����ش��� ��ʾ�յ� �շ�����״ָ̬ʾ ���ӵ� SX1278 DIO0


// Ӳ������
// GPIOC 7 --- MISO
// GPIOC 3 --- ģ�鸴λ����
// GPIOF 4 --- CE
// GPIOC 5 --- SPI_CLK
// GPIOC 6 --- MOSI
// PA = ���ʷŴ��� �ο� ���ſɵĴ��� û�õ�

#define  SX1278_SDO      1 //GPIO_ReadInputPin(GPIOC, GPIO_PIN_7)  // SPI����

#define  RF_REST_L       //GPIO_WriteLow(GPIOC, GPIO_PIN_3)
#define  RF_REST_H       //GPIO_WriteHigh(GPIOC, GPIO_PIN_3)
#define  RF_CE_L         //GPIO_WriteLow(GPIOF, GPIO_PIN_4)
#define  RF_CE_H         //GPIO_WriteHigh(GPIOF, GPIO_PIN_4)
#define  RF_CKL_L        //GPIO_WriteLow(GPIOC, GPIO_PIN_5)
#define  RF_CKL_H        //GPIO_WriteHigh(GPIOC, GPIO_PIN_5)
#define  RF_SDI_L        //GPIO_WriteLow(GPIOC, GPIO_PIN_6)
#define  RF_SDI_H        //GPIO_WriteHigh(GPIOC, GPIO_PIN_6)

// ���ʷŴ� ����
#define  PA_TXD_OUT()            
#define  PA_RXD_OUT()         

static void  fn_send_byte(uint8 out);
static uint8 fn_spi_read_byte(void);
static void  fn_cmd_switch_en(CmdEntype_t cmd);
static void  fn_cmd_switch_pa(CmdPaType_t cmd);

static recv_data_callback recv_cb = NULL;

// ���յ�RF������
static void fn_fqc_recv_data(uint8 *lpbuf, uint16 len);

lpCtrlTypefunc_t  ctrlTypefunc = {
   fn_send_byte,
   fn_spi_read_byte,
   fn_cmd_switch_en,
   fn_cmd_switch_pa, // PA ���ʷŴ���
   fn_fqc_recv_data
};

void sx1276_hal_recv_msg(void)
{
    sx1278_recv_handle();
}

void sx1276_hal_set_recv_cb(recv_data_callback cb)
{
    recv_cb = cb;
}

// оƬ��λ
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
static void fn_send_byte(uint8 out)
{
    // spi_mast_byte_write(SPI, out);
#if 0
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
        out = (out << 1);         // shift 1 place for next bit
        RF_CKL_L;                   // toggle clock low
    }
#endif    
}

static uint8 fn_spi_read_byte(void)
{

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

// û���� ��ʱ������
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

// ���յ�RF������
static void fn_fqc_recv_data(uint8 *buffer, uint16 len)
{
    os_printf("get buf %s len %d \n", buffer, len);
    if (NULL != recv_cb)
    {
        recv_cb(buffer, len);
    }
}
