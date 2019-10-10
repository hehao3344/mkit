#include <stdio.h>
#include <string.h>
#include "data_handle.h"

#define READ_ID     0x01

unsigned char tx_buf[8];

static unsigned short calccrc(unsigned char crcbuf, unsigned short crc);
static unsigned short chkcrc(unsigned char *buf, unsigned char len);

unsigned char * data_handle_get_read_param_buf(void)
{
    union crcdata
    {
        unsigned short word16;
        unsigned char  byte[2];
    } crcnow;

    tx_buf[0] = READ_ID; //模块的ID号，默认ID为0x01
    tx_buf[1] = 0x03;
    tx_buf[2] = 0x00;
    tx_buf[3] = 0x48;
    tx_buf[4] = 0x00;
    tx_buf[5] = 0x06;
    crcnow.word16 = chkcrc(tx_buf, 6);
    tx_buf[6] = crcnow.byte[1]; //CRC效验低字节在前
    tx_buf[7] = crcnow.byte[0];

    // Send_data(8); //发送8个数据，请根据单片机类型自己编程

    return tx_buf;
}

int data_handle_analysis_data(unsigned char * rx_buf, int reveive_number, METER_PARAM * meter)
{
    int ret = -1;
    unsigned char i;
    union crcdata
    {
        unsigned short word16;
        unsigned char byte[2];
    } crcnow;

    if (rx_buf[0] == READ_ID) // 确认ID正确
    {
        crcnow.word16 = chkcrc(rx_buf, reveive_number-2); // reveive_numbe是接收数据总长度
        if ((crcnow.byte[0] == rx_buf[reveive_number-1]) && (crcnow.byte[1] == rx_buf[reveive_number-2]))
        {
            meter->voltage_data = (((unsigned long)(rx_buf[3]))<<24)|(((unsigned long)(rx_buf[4]))<<16)|(((unsigned long)(rx_buf[5]))<<8)|rx_buf[6];
            meter->current_data = (((unsigned long)(rx_buf[7]))<<24)|(((unsigned long)(rx_buf[8]))<<16)|(((unsigned long)(rx_buf[9]))<<8)|rx_buf[10];
            meter->power_data   = (((unsigned long)(rx_buf[11]))<<24)|(((unsigned long)(rx_buf[12]))<<16)|(((unsigned long)(rx_buf[13]))<<8)|rx_buf[14];
            meter->energy_data  = (((unsigned long)(rx_buf[15]))<<24)|(((unsigned long)(rx_buf[16]))<<16)|(((unsigned long)(rx_buf[17]))<<8)|rx_buf[18];
            meter->pf_data      = (((unsigned long)(rx_buf[19]))<<24)|(((unsigned long)(rx_buf[20]))<<16)|(((unsigned long)(rx_buf[21]))<<8)|rx_buf[22];
            meter->co2_data     = (((unsigned long)(rx_buf[23]))<<24)|(((unsigned long)(rx_buf[24]))<<16)|(((unsigned long)(rx_buf[25]))<<8)|rx_buf[26];
            ret = 0;
        }
    }

    return ret;
}

static unsigned short calccrc(unsigned char crcbuf, unsigned short crc)
{
    unsigned char i;
    unsigned char chk;
    crc = crc ^ crcbuf;
    for (i=0; i<8; i++)
    {
        chk = (unsigned char)(crc&1);
        crc = crc>>1;
        crc = crc&0x7fff;
        if (1 == chk)
        {
            crc = crc^0xa001;
        }
        crc = crc&0xffff;
    }

    return crc;
}

static unsigned short chkcrc(unsigned char *buf, unsigned char len)
{
    unsigned char hi,lo;
    unsigned int i;
    unsigned short crc;
    crc = 0xFFFF;
    for (i=0; i<len; i++)
    {
        crc = calccrc(*buf, crc);
        buf++;
    }
    hi  = (unsigned char)(crc%256);
    lo  = (unsigned char)(crc/256);
    crc = (((unsigned int)(hi))<<8)|lo;

    return crc;
}

int data_handle_unit_test(void)
{
    int i;
    unsigned char * buf = data_handle_get_read_param_buf();
    for(i=0; i<8; i++)
    {
        printf("0x%x ", buf[i]);
    }
    printf("\n");

    unsigned char recv_buf[40] = { 0x01, 0x03, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x28, 0x00, 0x00, 0x13, 0x88, 0xA8, 0x51};

    METER_PARAM meter;
    unsigned long voltage_data;
    unsigned long current_data;
    unsigned long power_data;
    unsigned long energy_data;
    unsigned long pf_data;
    unsigned long co2_data;
    if (0 == data_handle_analysis_data(recv_buf, 37, &meter))
    {
        printf("%ld %ld %ld %ld %ld %ld\n", meter.voltage_data, meter.current_data, meter.power_data, meter.power_data, meter.pf_data, meter.co2_data);
    }
    else
    {
        printf("data_handle_analysis_data failed \n");
    }

    return 0;
    //01 03 00 48 00 08 C4 1A
}
