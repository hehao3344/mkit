#ifndef __SMART_CONFIG_H
#define __SMART_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    E_STATUS_INIT = 0x0000,
    E_STATUS_WAIT,
    E_STATUS_FIND_CHANNEL,
    E_STATUS_GETTING_SSID_PSWD,
    E_STATUS_LINK,
    E_STATUS_LINK_OVER,
} E_WIFI_STATUS;

void   smart_config_start(void);
uint32 smart_config_get_status(void);

#ifdef __cplusplus
}
#endif

#endif // __TCP_CLIENT_H

