#ifndef __JSON_FORMAT_H
#define __JSON_FORMAT_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

// mqtt
#define MQTT_REGISTER_MSG "{\
\"cmd\":\"register\",\
\"version\":\"v2.0.0\"\
}"

#define MQTT_HEART_BEAT "{ \
\"cmd\":\"heart_beat\" \
}"



#define DEV_REGISTER_MSG  "{\
\"method\":\"up_msg\",\
\"cc_uuid\":\"%s%s\",\
\"req_id\":%d,\
\"attr\":\
{\
\"cmd\":\"register\",\
\"version\":\"V%d.%02d\"\
}\
}"

#define SET_MATCH_MSG  "{\
\"method\":\"down_msg\",\
\"dev_uuid\":\"FFFFFFFFFFFF\",\
\"req_id\":%d,\
\"ts\":%d,\
\"attr\":\
{\
\"cmd\":\"set_match\":\
{\
}\
}\
}"

#define SET_SWITCH_RESP "{\
\"method\":\"up_msg\",\
\"cc_uuid\":\"%s\",\
\"req_id\":%d,\
\"code\":%d\
}"

#define GET_SUB_DEV_PARAM_RESP "{ \
\"method\":\"up_msg\",\
\"dev_uuid\":\"%s\",\
\"req_id\":%d,\
\"code\":%d,\
\"attribute\":\
{\
\"online\":\"%s\",\
\"switch\":\"%s\",\
}\
}"


#define UPLOAD_EVENT_MSG "{\
\"method\":\"report_msg\",\
\"cc_uuid\":\"%s%s\",\
\"attr\":\
{\
\"cmd\":\"updata_status\",\
\"dev1\":\
{\
\"dev_uuid\":\"%s\",\
\"online\":\"%s\",\
\"switch\":\"%s\",\
},\
\"dev2\":\
{\
\"dev_uuid\":\"%s\",\
\"online\":\"%s\",\
\"switch\":\"%s\"\
},\
\"dev3\":\
{\
\"dev_uuid\":\"%s\",\
\"online\":\"%s\",\
\"switch\":\"%s\",\
},\
\"dev4\":\
{\
\"dev_uuid\":\"%s\",\
\"online\":\"%s\",\
\"switch\":\"%s\",\
}\
}\
}"

#ifdef __cplusplus
}
#endif

#endif // __JSON_FORMAT_H
