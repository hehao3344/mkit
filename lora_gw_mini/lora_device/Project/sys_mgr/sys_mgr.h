#ifndef __SYS_MGR_H
#define __SYS_MGR_H

#include "../core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

void sys_mgr_init( void );
void sys_mgr_send_msg( void );
void sys_mgr_handle_key( void );
void sys_mgr_handle_remote_msg( void );

#ifdef __cplusplus
}
#endif

#endif
