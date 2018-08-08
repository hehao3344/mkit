#ifndef __NO_OS_TIME_H
#define __NO_OS_TIME_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

void    no_os_get_ymd_hms_w( uint32 secs, int8 *value );
uint32  no_os_get_day_secs( uint32 value );
uint16  no_os_get_weekday( uint32 value );
boolean no_os_sec_is_valid( uint32 secs );

#ifdef __cplusplus
}
#endif

#endif // __NO_OS_TIME_H
