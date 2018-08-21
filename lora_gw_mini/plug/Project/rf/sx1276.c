#include "sx1276.h"
#include "stm8s.h"

static uint8   frequency[3] = { 0x6c, 0x80, 0x00 };
//static uint8 spreading_factor = 11;   // 7-12
static uint8   spreading_factor = 7;   // 7-12
static uint8   coding_rate      = 2;         // 1-4

//               0000 -> 7.8kHz
//               0001 -> 10.4kHz
//               0010 -> 15.6kHz
//               0011 -> 20.8kHz
//               0100 -> 31.25kHz
//               0101 -> 41.7kHz
//               0110 -> 62.5kHz
//               0111 -> 125kHz
//               1000 -> 250kHz
//               1001 -> 500kHz              
// static uint8   bw_frequency = 7; // 6-9
static uint8   bw_frequency = 3;    // 20.8K
static uint8   power_value  = 7;
static uint8   power_data[8]   = { 0x80, 0x80, 0x80, 0x83, 0x86, 0x89, 0x8c, 0x8f };
static uint8   recv_buffer[12];  
static lpCtrlTypefunc_t lp_type_func = { 0, 0, 0, 0, 0 };

static void write_buffer(uint8 addr, uint8 buffer);
static uint8 read_buffer(uint8 addr);
static void lora_set_op_mode(RFMODE_SET opMode);
static void lora_fsk(DEBUGGING_FSK_OOK opMode);
static void lora_set_rf_frequency(void);
static void lora_set_rf_power(uint8 power);
static void lora_set_spreading_factor(uint8 factor);
static void lora_set_nb_trig_peaks(uint8 value);
static void lora_set_error_coding(uint8 value);
static void lora_set_packet_crc_on(boolean enable);
static void lora_set_signal_bandwidth(uint8 bw);
static void lora_set_implicit_header_on(boolean enable);
static void lora_set_symb_timeout(uint32 value);
static void lora_set_payload_length(uint8 value);
static void lora_set_mobile_node(boolean enable);
static void rf_receive (void);

void sx1276_delay_1s(uint32 ii)
{
    uint8 j;
    while(ii--)
    {
        for(j=0; j<100; j++);
    }
}

void rx1276_rf_send_packet(uint8 *rf_tran_buf, uint8 len)
{
    uint8 i;

    lp_type_func.paSwitchCmdfunc(TX_OPEN);

    lora_set_op_mode(STDBY_MODE);
    write_buffer(REG_LR_HOPPERIOD, 0);                  
    write_buffer(REG_LR_IRQFLAGSMASK, IRQN_TXD_Value); 
    write_buffer(REG_LR_PAYLOADLENGTH, len);           
    write_buffer(REG_LR_FIFOTXBASEADDR, 0);
    write_buffer(REG_LR_FIFOADDRPTR, 0);

    lp_type_func.lpSwitchEnStatus(EN_OPEN);
    lp_type_func.lpByteWritefunc(0x80);

    for (i = 0; i < len; i++)
    {
        lp_type_func.lpByteWritefunc(*rf_tran_buf);
        rf_tran_buf++;
    }

    lp_type_func.lpSwitchEnStatus(EN_CLOSE);
    write_buffer(REG_LR_DIOMAPPING1, 0x40);
    write_buffer(REG_LR_DIOMAPPING2, 0x00);

    lora_set_op_mode(TRANSMITTER_MODE);
}
 
void rx1276_register_rf_func(lpCtrlTypefunc_t *func)
{
    if (0 != func->lpByteWritefunc)
    {
        lp_type_func.lpByteWritefunc = func->lpByteWritefunc;
    }

    if (0 != func->lpByteReadfunc)
    {
        lp_type_func.lpByteReadfunc = func->lpByteReadfunc;
    }

    if (0 != func->lpSwitchEnStatus)
    {
        lp_type_func.lpSwitchEnStatus = func->lpSwitchEnStatus;
    }

    if (0 != func->paSwitchCmdfunc)
    {
        lp_type_func.paSwitchCmdfunc = func->paSwitchCmdfunc;
    }

    if (0 != func->lpRecvDataTousr)
    {
        lp_type_func.lpRecvDataTousr = func->lpRecvDataTousr;
    }
}

void sx1276_lora_init(void)
{
    lora_set_op_mode(SLEEP_MODE);    
    lora_fsk(LORA_MODE);                
    lora_set_op_mode(STDBY_MODE);        
    write_buffer(REG_LR_DIOMAPPING1, GPIO_VARE_1);
    write_buffer(REG_LR_DIOMAPPING2, GPIO_VARE_2);
    lora_set_rf_frequency();
    lora_set_rf_power(power_value);
    lora_set_spreading_factor(spreading_factor);  
    lora_set_error_coding(coding_rate);          
    lora_set_packet_crc_on(TRUE);                
    lora_set_signal_bandwidth(bw_frequency);       
    lora_set_implicit_header_on(FALSE);            
    lora_set_payload_length(0xFF);
    lora_set_symb_timeout(0x3FF);
    lora_set_mobile_node(TRUE);   

    rf_receive();
}

void sx1276_rx_mode(void)
{
    rf_receive();
}

void sx1278_recv_handle(void)
{
    uint8 i;
    uint8 rf_ex0_status = 0;
    uint8 crc_value     = 0;
    uint8 sx1278_rlen   = 0;

    rf_ex0_status = read_buffer(REG_LR_IRQFLAGS);
    if (0x40 == (rf_ex0_status & 0x40))
    {
        crc_value = read_buffer(REG_LR_MODEMCONFIG2);

        if (crc_value & 0x04 == 0x04)
        {
            write_buffer (REG_LR_FIFOADDRPTR, 0x00);
            sx1278_rlen = read_buffer(REG_LR_NBRXBYTES);
            lp_type_func.lpSwitchEnStatus(EN_OPEN);
            lp_type_func.lpByteWritefunc(0x00);
            for (i = 0; i < sx1278_rlen; i++)
            {
                if (i < sizeof(recv_buffer))
                {
                    recv_buffer[i] = lp_type_func.lpByteReadfunc();
                }
            }
            lp_type_func.lpSwitchEnStatus(EN_CLOSE);
        }
 
        lp_type_func.lpRecvDataTousr(recv_buffer, sx1278_rlen);

#if 0
        // test add for rev rssi --- Received Signal Strength Indication
        current_rssi_value       = sx1276_lora_read_rssi();
        current_rssi[rece_count] = current_rssi_value;
        pack_rssi_value          = sx1276_lora_read_pack_rssi_value();
        pack_rssi[rece_count]    = pack_rssi_value;
        rece_count               = rece_count + 1;
#endif

        lora_set_op_mode(STDBY_MODE);
        write_buffer(REG_LR_IRQFLAGSMASK, IRQN_RXD_Value);  
        write_buffer(REG_LR_HOPPERIOD,    PACKET_MIAX_Value);
        write_buffer(REG_LR_DIOMAPPING1, 0X00);
        write_buffer(REG_LR_DIOMAPPING2, 0x00);
        lora_set_op_mode(RECEIVER_MODE);
        lp_type_func.paSwitchCmdfunc(RX_OPEN);
    }
    else if (0x08 == (rf_ex0_status & 0x08))
    { 
        lora_set_op_mode(STDBY_MODE);
        write_buffer(REG_LR_IRQFLAGSMASK, IRQN_RXD_Value);  
        write_buffer(REG_LR_HOPPERIOD,   PACKET_MIAX_Value);
        write_buffer(REG_LR_DIOMAPPING1, 0X00);
        write_buffer(REG_LR_DIOMAPPING2, 0x00);
        lora_set_op_mode(RECEIVER_MODE);
        lp_type_func.paSwitchCmdfunc(RX_OPEN);
    }
    else if(0x04 == (rf_ex0_status & 0x04))
    {
        if (0x01 == (rf_ex0_status&0x01))
        { 
            lora_set_op_mode(STDBY_MODE);
            write_buffer(REG_LR_IRQFLAGSMASK, IRQN_RXD_Value);  
            write_buffer(REG_LR_HOPPERIOD,    PACKET_MIAX_Value);
            write_buffer(REG_LR_DIOMAPPING1, 0X02);
            write_buffer(REG_LR_DIOMAPPING2, 0x00);
            lora_set_op_mode(RECEIVER_MODE);
            lp_type_func.paSwitchCmdfunc(RX_OPEN);
        }
        else
        {                          
            lora_set_op_mode(STDBY_MODE);
            write_buffer(REG_LR_IRQFLAGSMASK, IRQN_SEELP_Value);  
            write_buffer(REG_LR_DIOMAPPING1, 0x00);
            write_buffer(REG_LR_DIOMAPPING2, 0x00);
            lora_set_op_mode(SLEEP_MODE);
            //PA_SEELP_OUT();
        }
    }
    else
    {
        lora_set_op_mode(STDBY_MODE);
        write_buffer(REG_LR_IRQFLAGSMASK,IRQN_RXD_Value);  
        write_buffer(REG_LR_HOPPERIOD,   PACKET_MIAX_Value);
        write_buffer(REG_LR_DIOMAPPING1, 0X02);
        write_buffer(REG_LR_DIOMAPPING2, 0x00);
        lora_set_op_mode(RECEIVER_MODE);
        lp_type_func.paSwitchCmdfunc(RX_OPEN);
    }

    write_buffer(REG_LR_IRQFLAGS, 0xff);
}

////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
static void write_buffer(uint8 addr, uint8 buffer)
{
    lp_type_func.lpSwitchEnStatus(EN_OPEN);    // NSS = 0;
    lp_type_func.lpByteWritefunc(addr | 0x80);
    lp_type_func.lpByteWritefunc(buffer);
    lp_type_func.lpSwitchEnStatus(EN_CLOSE); // NSS = 1;
}

static uint8 read_buffer(uint8 addr)
{
    uint8 value;
    
    lp_type_func.lpSwitchEnStatus(EN_OPEN); // NSS = 0;
    lp_type_func.lpByteWritefunc(addr & 0x7f );
    value = lp_type_func.lpByteReadfunc();
    lp_type_func.lpSwitchEnStatus(EN_CLOSE);// NSS = 1;
    
    return value;
}

static void lora_set_op_mode(RFMODE_SET opMode)
{
    uint8 op_mode_prev;
    op_mode_prev = read_buffer(REG_LR_OPMODE);
    op_mode_prev &= 0xf8;
    op_mode_prev |= (uint8)opMode;
    
    write_buffer(REG_LR_OPMODE, op_mode_prev);
}

static void lora_fsk(DEBUGGING_FSK_OOK opMode)
{
    uint8 op_mode_prev;
    op_mode_prev = read_buffer(REG_LR_OPMODE);
    op_mode_prev &=0x7F;
    op_mode_prev |= (uint8)opMode;
    
    write_buffer(REG_LR_OPMODE, op_mode_prev);
}

static void lora_set_rf_frequency(void)
{
    write_buffer(REG_LR_FRFMSB, frequency[0]);
    write_buffer(REG_LR_FRFMID, frequency[1]);
    write_buffer(REG_LR_FRFLSB, frequency[2]);
}

static void lora_set_rf_power(uint8 power)
{
    write_buffer(REG_LR_PADAC, 0x87);
    write_buffer(REG_LR_PACONFIG, power_data[power]);
}

static void lora_set_spreading_factor(uint8 factor)
{
    uint8 recv_data;
    lora_set_nb_trig_peaks(3);
    recv_data = read_buffer(REG_LR_MODEMCONFIG2);
    recv_data = (recv_data & RFLR_MODEMCONFIG2_SF_MASK) | (factor << 4);
    write_buffer(REG_LR_MODEMCONFIG2, recv_data);
}

static void lora_set_nb_trig_peaks(uint8 value)
{
    uint8 recv_data;
    recv_data = read_buffer(0x31);
    recv_data = (recv_data & 0xF8) | value;
    write_buffer(0x31, recv_data);
}

static void lora_set_error_coding(uint8 value)
{
    uint8 recv_data;
    recv_data = read_buffer(REG_LR_MODEMCONFIG1);
    recv_data = (recv_data & RFLR_MODEMCONFIG1_CODINGRATE_MASK) | (value << 1);
    write_buffer(REG_LR_MODEMCONFIG1, recv_data);
    // LoRaset_tings.ErrorCoding = value;
}

static void lora_set_packet_crc_on(boolean enable)
{
    uint8 recv_data;
    recv_data = read_buffer(REG_LR_MODEMCONFIG2);
    recv_data = (recv_data & RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK) | (enable << 2);
    
    write_buffer(REG_LR_MODEMCONFIG2, recv_data);
}

static void lora_set_signal_bandwidth(uint8 bw)
{
    uint8 recv_data;
    recv_data = read_buffer(REG_LR_MODEMCONFIG1);
    recv_data = (recv_data & RFLR_MODEMCONFIG1_BW_MASK) | (bw << 4);  
    write_buffer(REG_LR_MODEMCONFIG1, recv_data);   
}

static void lora_set_implicit_header_on(boolean enable)
{
    uint8 recv_data;
    recv_data = read_buffer(REG_LR_MODEMCONFIG1);
    recv_data = ((recv_data & RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK) | (enable));
    
    write_buffer(REG_LR_MODEMCONFIG1, recv_data);
}

static void lora_set_symb_timeout(uint32 value)
{
    uint8 recv_data[2];
    
    recv_data[0] = read_buffer(REG_LR_MODEMCONFIG2);
    recv_data[1] = read_buffer(REG_LR_SYMBTIMEOUTLSB);
    recv_data[0] = ((recv_data[0] & RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK) |
                    ((value >> 8) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK));
    recv_data[1] = value & 0xFF;
    write_buffer(REG_LR_MODEMCONFIG2, recv_data[0]);
    write_buffer(REG_LR_SYMBTIMEOUTLSB, recv_data[1]);
}

static void lora_set_payload_length(uint8 value)
{
    write_buffer(REG_LR_PAYLOADLENGTH, value);
}

static void lora_set_mobile_node(boolean enable)
{
    uint8 recv_data;
    recv_data = read_buffer(REG_LR_MODEMCONFIG3);
    recv_data = ((recv_data & RFLR_MODEMCONFIG3_MOBILE_NODE_MASK) | (enable << 3));
    write_buffer(REG_LR_MODEMCONFIG3, recv_data);
}

static void rf_receive(void)
{
    lora_set_op_mode(STDBY_MODE);
    write_buffer(REG_LR_IRQFLAGSMASK, IRQN_RXD_Value);   
    write_buffer(REG_LR_HOPPERIOD,  PACKET_MIAX_Value);
    write_buffer(REG_LR_DIOMAPPING1, 0x00);
    write_buffer(REG_LR_DIOMAPPING2, 0x00);
    lora_set_op_mode(RECEIVER_MODE);
    lp_type_func.paSwitchCmdfunc(RX_OPEN);
}
