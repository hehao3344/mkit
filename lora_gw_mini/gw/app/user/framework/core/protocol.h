#ifndef	__PROTOCOL_H
#define	__PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#define DEVICE_A_REGISTER               0x01
#define DEVICE_R_REGISTER               0x81

#define DEVICE_A_INFORM_PARAM           0x02    // keep alive.
#define DEVICE_R_INFORM_PARAM           0x82

#define CLIENT_A_GET_PARAM              0x03    // client request get param from server.
#define CLIENT_R_GET_PARAM              0x83

#define CLIENT_A_PROXY                  0x04
#define SERVER_A_PROXY                  0x05
#define DEVICE_R_PROXY                  0x85
#define SERVER_R_PROXY                  0x84

#define CLIENT_COMMU_A_CONFIG           0x10
#define DEVICE_COMMU_R_CONFIG           0x90

#define CLIENT_COMMU_A_GET_PARAM        0x11
#define DEVICE_COMMU_R_GET_PARAM        0x91

#define SERVER_A_UPGRADE                0x15    // server request update param. [sv + hv + time + url + ip]
#define SERVER_R_UPGRADE                0x95

// subscribe command.
#define MSC_A_DISCOVERY                 0x20
#define MSC_R_DISCOVERY                 0xA0

#define SUB_CMD_CONFIG_SWITCH           0x01
#define SUB_CMD_CONFIG_TIMER            0x02
#define SUB_CMD_CONFIG_WEEK_PLAN        0x03
#define SUB_CMD_CONFIG_DEVICE_NAME      0x04
#define SUB_CMD_CONFIG_AP_PASSWORD      0x05
#define SUB_CMD_CONFIG_STA_SSID         0x06
#define SUB_CMD_CONFIG_STA_PASSWORD     0x07
#define SUB_CMD_CONFIG_USERNAME         0x08
#define SUB_CMD_CONFIG_PASSWORD         0x09

#define SUB_CMD_CONFIG_RESPONSE         0x80

#ifdef __cplusplus
}
#endif

#endif //__PROTOCOL_H
