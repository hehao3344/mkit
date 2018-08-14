#include "stm8s.h"
#include "sx1276.h"
#include "sx1276_hal.h"
    
static RfPlugResult cur_rf_result = { 0, 0, { 0, 0, 0, 0 }, 0 };

static void  fn_send_byte( uint8 out );
static uint8 fn_spi_read_byte( void );
static void  fn_cmd_switch_en( CmdEntype_t cmd );
static void  fn_cmd_switch_pa( CmdPaType_t cmd );

// ���յ�RF������
void fn_fqc_recv_data( uint8 *lpbuf, uint16 len );

static lpCtrlTypefunc_t  ctrlTypefunc = 
{
    fn_send_byte,
    fn_spi_read_byte,
    fn_cmd_switch_en,
    fn_cmd_switch_pa, // PA ���ʷŴ���
    fn_fqc_recv_data
};

// оƬ��λ
void sx1276_hal_reset( void )
{
    RF_REST_L;
    sx1276_delay_1s( 2000 );
    RF_REST_H;
    sx1276_delay_1s( 500 );
}

void sx1276_hal_register_rf_func( void )
{
    rx1276_register_rf_func( &ctrlTypefunc );
}

void sx1276_hal_rf_send_packet( uint8 *rf_tran_buf, uint8 len )
{
    rx1276_rf_send_packet( rf_tran_buf, len );
}

RfPlugResult * sx1276_hal_get_rf_result( void )
{
    return &cur_rf_result;
}

void sx1276_hal_clear_rf_result( void )
{
    cur_rf_result.switch_invalid = 0;  
    cur_rf_result.switch_on_off  = 0;

    cur_rf_result.mac[0] = 0;
    cur_rf_result.mac[1] = 0;
    cur_rf_result.mac[2] = 0;
    cur_rf_result.mac[3] = 0;
    cur_rf_result.address = 0;
 
}

void sx1276_hal_rx_mode( void )
{
    sx1276_rx_mode();
}

void sx1276_hal_lora_init( void )
{
    sx1276_lora_init();
}

////////////////////////////////////////////////////////////////////////////////
// static function
////////////////////////////////////////////////////////////////////////////////
static void fn_send_byte( uint8 out )
{
    uint8 i;
    for ( i = 0; i < 8; i++ )
    {
        if ( out & 0x80 )           // check if MSB is high
        {
            RF_SDI_H;
        }
        else
        {
            RF_SDI_L;               // if not, set to low
        }

        RF_CKL_H;                   // toggle clock high
        out = ( out << 1 );         // shift 1 place for next bit
        RF_CKL_L;                   // toggle clock low
    }
}

static uint8 fn_spi_read_byte( void )
{
    uint8 i, j;

    j = 0;
    for ( i = 0; i < 8; i++ )
    {
        RF_CKL_H;
        j = (j << 1);           // shift 1 place to the left or shift in 0
        if ( SX1278_SDO )       // check to see if bit is high
        {
            j = j | 0x01;       // if high, make bit high
        }
                                // toggle clock high
        RF_CKL_L;               // toggle clock low
    }

    return j;                   // toggle clock low //
}

static void fn_cmd_switch_en( CmdEntype_t cmd )
{
    switch( cmd )
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
static void fn_cmd_switch_pa( CmdPaType_t cmd )
{
#if 0
    switch( cmd )
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

// ���յ�RF������
static void fn_fqc_recv_data( uint8 *buffer, uint16 len )
{
    RfPlugResult * rf_device_result = protocol_device_resolve_data( buffer, len );

    if ( NULL != rf_device_result )
    {
        cur_rf_result.switch_invalid = rf_device_result->switch_invalid;
        cur_rf_result.switch_on_off  = rf_device_result->switch_on_off;
        
        cur_rf_result.mac[0]         = rf_device_result->mac[0];
        cur_rf_result.mac[1]         = rf_device_result->mac[1];
        cur_rf_result.mac[2]         = rf_device_result->mac[2];
        cur_rf_result.mac[3]         = rf_device_result->mac[3];

        cur_rf_result.address        = rf_device_result->address;
    }
}
