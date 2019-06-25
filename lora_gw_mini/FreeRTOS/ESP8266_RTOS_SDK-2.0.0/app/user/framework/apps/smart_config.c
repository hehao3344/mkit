/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: smart_config.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "esp_common.h"
#include "espconn.h"

#include "smartconfig.h"
#include "airkiss.h"

#include "smart_config.h"

#define DEVICE_TYPE 		"gh_9e2cff3dfa51"   // wechat public number
#define DEVICE_ID 			"122475"            // model ID
#define DEFAULT_LAN_PORT 	12476

static esp_udp ssdp_udp;
static struct  espconn pssdpudpconn;
static os_timer_t ssdp_time_serv;

static uint8_t  lan_buf[200];
static uint16_t lan_buf_len;
static uint8 	udp_sent_cnt = 0;
static uint32   smart_config_status = 0;

const airkiss_config_t akconf =
{
	(airkiss_memset_fn)&memset,
	(airkiss_memcpy_fn)&memcpy,
	(airkiss_memcmp_fn)&memcmp,
	0,
};


static void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata);
LOCAL void ICACHE_FLASH_ATTR  airkiss_wifilan_time_callback(void);
LOCAL void ICACHE_FLASH_ATTR  airkiss_wifilan_recv_callbk(void *arg, char *pdata, unsigned short len);
void ICACHE_FLASH_ATTR airkiss_start_discover(void);

void smart_config_start(void)
{
    os_printf("start smart config mode = %d %d \n", wifi_get_opmode(), STATION_MODE);
    smartconfig_set_type(SC_TYPE_AIRKISS); // SC_TYPE_ESPTOUCH, SC_TYPE_AIRKISS, SC_TYPE_ESPTOUCH_AIRKISS
    wifi_set_opmode(STATION_MODE);
    smartconfig_start(smartconfig_done);
    os_printf("start smart config mode = %d %d done\n", wifi_get_opmode(), STATION_MODE);
}

uint32 smart_config_get_status(void)
{
    return smart_config_status;
}

/*********************************************************************************
* static function.
*********************************************************************************/
static void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata)
{
    switch(status)
    {
        case SC_STATUS_WAIT:
        {
            os_printf("SC_STATUS_WAIT\n");
            smart_config_status = E_STATUS_WAIT;
            break;
        }
        case SC_STATUS_FIND_CHANNEL:
        {
            os_printf("SC_STATUS_FIND_CHANNEL\n");
            smart_config_status = E_STATUS_FIND_CHANNEL;
            break;
        }
        case SC_STATUS_GETTING_SSID_PSWD:
        {
            os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
			sc_type *type = pdata;
            if (*type == SC_TYPE_ESPTOUCH)
            {
                os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
            }
            else
            {
                os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
            }
            smart_config_status = E_STATUS_GETTING_SSID_PSWD;
            break;
        }
        case SC_STATUS_LINK:
        {
            os_printf("SC_STATUS_LINK\n");
            struct station_config *sta_conf = pdata;

            wifi_station_set_config(sta_conf);
            wifi_station_disconnect();
            wifi_station_connect();
            smart_config_status = E_STATUS_LINK;
            break;
        }
        case SC_STATUS_LINK_OVER:
        {
            os_printf("SC_STATUS_LINK_OVER\n");
            if (pdata != NULL)
            {
				//SC_TYPE_ESPTOUCH
                uint8 phone_ip[4] = {0};
                os_memcpy(phone_ip, (uint8*)pdata, 4);
                os_printf("Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
            else
            {
            	//SC_TYPE_AIRKISS - support airkiss v2.0
                airkiss_start_discover();
            }
            smart_config_status = E_STATUS_LINK_OVER;
            smartconfig_stop();
            break;
        }
        default:
            break;
    }
}

LOCAL void ICACHE_FLASH_ATTR airkiss_wifilan_time_callback(void)
{
	uint16 i;
	airkiss_lan_ret_t ret;

	if ((udp_sent_cnt++) >30)
	{
		udp_sent_cnt = 0;
		os_timer_disarm(&ssdp_time_serv);//s
		//return;
	}

	ssdp_udp.remote_port = DEFAULT_LAN_PORT;
	ssdp_udp.remote_ip[0] = 255;
	ssdp_udp.remote_ip[1] = 255;
	ssdp_udp.remote_ip[2] = 255;
	ssdp_udp.remote_ip[3] = 255;
	lan_buf_len = sizeof(lan_buf);
	ret = airkiss_lan_pack(AIRKISS_LAN_SSDP_NOTIFY_CMD,
		DEVICE_TYPE, DEVICE_ID, 0, 0, lan_buf, &lan_buf_len, &akconf);
	if (ret != AIRKISS_LAN_PAKE_READY) {
		os_printf("Pack lan packet error!");
		return;
	}

	ret = espconn_sendto(&pssdpudpconn, lan_buf, lan_buf_len);
	if (ret != 0) {
		os_printf("UDP send error!");
	}
	os_printf("Finish send notify!\n");
}

LOCAL void ICACHE_FLASH_ATTR airkiss_wifilan_recv_callbk(void *arg, char *pdata, unsigned short len)
{
	uint16 i;
	remot_info* pcon_info = NULL;

	airkiss_lan_ret_t ret = airkiss_lan_recv(pdata, len, &akconf);
	airkiss_lan_ret_t packret;

	switch (ret)
	{
	case AIRKISS_LAN_SSDP_REQ:
		espconn_get_connection_info(&pssdpudpconn, &pcon_info, 0);
		os_printf("remote ip: %d.%d.%d.%d \r\n",pcon_info->remote_ip[0],pcon_info->remote_ip[1],
			                                    pcon_info->remote_ip[2],pcon_info->remote_ip[3]);
		os_printf("remote port: %d \r\n",pcon_info->remote_port);

        pssdpudpconn.proto.udp->remote_port = pcon_info->remote_port;
		os_memcpy(pssdpudpconn.proto.udp->remote_ip,pcon_info->remote_ip,4);
		ssdp_udp.remote_port = DEFAULT_LAN_PORT;

		lan_buf_len = sizeof(lan_buf);
		packret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD,
			DEVICE_TYPE, DEVICE_ID, 0, 0, lan_buf, &lan_buf_len, &akconf);

		if (packret != AIRKISS_LAN_PAKE_READY) {
			os_printf("Pack lan packet error!");
			return;
		}

		os_printf("\r\n\r\n");
		for (i=0; i<lan_buf_len; i++)
			os_printf("%c",lan_buf[i]);
		os_printf("\r\n\r\n");

		packret = espconn_sendto(&pssdpudpconn, lan_buf, lan_buf_len);
		if (packret != 0) {
			os_printf("LAN UDP Send err!");
		}

		break;
	default:
		os_printf("Pack is not ssdq req!%d\r\n",ret);
		break;
	}
}

void ICACHE_FLASH_ATTR airkiss_start_discover(void)
{
	ssdp_udp.local_port = DEFAULT_LAN_PORT;
	pssdpudpconn.type = ESPCONN_UDP;
	pssdpudpconn.proto.udp = &(ssdp_udp);
	espconn_regist_recvcb(&pssdpudpconn, airkiss_wifilan_recv_callbk);
	espconn_create(&pssdpudpconn);

	os_timer_disarm(&ssdp_time_serv);
	os_timer_setfn(&ssdp_time_serv, (os_timer_func_t *)airkiss_wifilan_time_callback, NULL);
	os_timer_arm(&ssdp_time_serv, 1000, 1); // 1s
}

