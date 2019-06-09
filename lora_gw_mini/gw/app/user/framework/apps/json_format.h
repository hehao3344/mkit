#ifndef __JSON_FORMAT_H
#define __JSON_FORMAT_H

#include "os_type.h"
#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

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
\"dev_uuid\":\"AAA\",\
\"online\":\"no\",\
\"switch\":\"off\"\
},\
\"dev3\":\
{\
\"dev_uuid\":\"BBB\",\
\"online\":\"yes\",\
\"switch\":\"on\",\
}\
}\
}"

#ifdef __cplusplus
}
#endif

#endif // __JSON_FORMAT_H
