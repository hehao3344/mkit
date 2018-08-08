#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SCH_PROC_TASK_PRIO         0
#define SCH_PROC_TASK_QUEUE_LEN    4 // wifi+ntp

boolean schedule_create( uint16 smart_config );
void    schedule_destroy( void );

#ifdef __cplusplus
}
#endif

#endif //__SCHEDULE_H
