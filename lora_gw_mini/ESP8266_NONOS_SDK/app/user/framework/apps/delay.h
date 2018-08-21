#ifndef __DELAY_H
#define __DELAY_H

#include "os_type.h"
#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

boolean delay_create( void );
void    delay_set( uint32 sys_sec, uint32 sec );
void    delay_get( uint32 sys_sec, uint32 *cur, uint32 *total );
uint32  delay_get_end( void );
boolean delay_now_is_in_delay( uint32 cur_sec );
void    delay_destroy( void);

#ifdef __cplusplus
}
#endif

#endif // __DELAY_H
