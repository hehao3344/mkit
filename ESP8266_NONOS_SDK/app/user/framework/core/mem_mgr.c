#include <stdio.h>
#include <string.h>

#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "core.h"

#include "mem_mgr.h"

typedef struct MemMgrObject
{
    int8 buffer1[256];
    int8 buffer2[256];
    int8 buffer3[256];
    int8 buffer4[256];
} MemMgrObject;

static MemMgrObject *instance( void );

int8 * mem_mgr_get_ptr( uint16 index )
{
    int8 *ret = NULL;
    if ( index > 3 )
    {
        return NULL;
    }

    MemMgrObject *handle = instance();

    switch( index )
    {
        case 0:
            ret = handle->buffer1;
            break;
        case 1:
            ret = handle->buffer2;
            break;
        case 2:
            ret = handle->buffer3;
            break;
        case 3:
            ret = handle->buffer4;
            break;
        default:
            break;
    }

    return ret;
}

uint16 mem_mgr_get_len( uint16 index )
{
    uint16 ret = 0;
    if ( index > 3 )
    {
        return 0;
    }

    MemMgrObject *handle = instance();

    switch( index )
    {
        case 0:
            ret = sizeof( handle->buffer1 );
            break;
        case 1:
            ret = sizeof( handle->buffer2 );
            break;
        case 2:
            ret = sizeof( handle->buffer3 );
            break;
        case 3:
            ret = sizeof( handle->buffer4 );
            break;
        default:
            break;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
static MemMgrObject *instance( void )
{
    static MemMgrObject *handle = NULL;
    if ( NULL == handle )
    {
        handle = ( MemMgrObject * )os_malloc( sizeof( MemMgrObject ) );
        if ( NULL == handle )
        {
            os_printf( "mem mgr: failed \n" );
        }
    }

    return handle;
}
