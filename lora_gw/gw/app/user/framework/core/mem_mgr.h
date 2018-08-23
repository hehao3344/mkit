#ifndef __MEM_MGR_H
#define __MEM_MGR_H

#include "os_type.h"
#include "core.h"

#ifdef __cplusplus
extern "C"
{
#endif

int8 * mem_mgr_get_ptr( uint16 index );
uint16 mem_mgr_get_len( uint16 index );

#ifdef __cplusplus
}
#endif

#endif
