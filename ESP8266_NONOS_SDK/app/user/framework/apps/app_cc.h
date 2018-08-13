#ifndef __APP_CC_H
#define __APP_CC_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

boolean schedule_create( uint16 smart_config );
void    schedule_destroy( void );

#ifdef __cplusplus
}
#endif

#endif //__APP_CC_H
