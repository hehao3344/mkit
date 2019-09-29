#ifndef __MQTT_APP_H
#define __MQTT_APP_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MQTT_BROKER  "10.101.70.32"     /* MQTT Broker Address*/
#define MQTT_PORT    1883               /* MQTT Port*/

void mqtt_app_init(void);

#ifdef __cplusplus
}
#endif

#endif // __TCP_CLIENT_H
