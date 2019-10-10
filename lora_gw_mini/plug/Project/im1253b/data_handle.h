#ifndef __DATA_HANDLE_H
#define __DATA_HANDLE_H

typedef struct _METER_PARAM
{
    unsigned long voltage_data;
    unsigned long current_data;
    unsigned long power_data;
    unsigned long energy_data;
    unsigned long pf_data;
    unsigned long co2_data;
} METER_PARAM;

unsigned char * data_handle_get_read_param_buf(void);
int data_handle_analysis_data(unsigned char * rx_buf, int reveive_number, METER_PARAM * meter);

#endif
