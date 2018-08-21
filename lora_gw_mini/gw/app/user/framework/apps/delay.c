#include <stdio.h>
#include <string.h>

#include "os_type.h"
#include "osapi.h"
#include "mem.h"

#include "../core/core.h"
#include "delay.h"

typedef struct DelayObject
{
    uint32 total;
    uint32 end;
    uint32 last;
} DelayObject;

static DelayObject *instance( void );

boolean ICACHE_FLASH_ATTR delay_create( void )
{
    DelayObject *handle = instance();
    if ( NULL == handle )
    {
        os_printf("delay error \n");
        return FALSE;
    }

	return TRUE;
}

void delay_set( uint32 sys_sec, uint32 sec )
{
    DelayObject *handle = instance();

    handle->end   = sys_sec + sec;
    handle->total = sec;
    handle->last  = sys_sec;
}

void delay_get( uint32 sys_sec, uint32 *cur, uint32 *total )
{
    DelayObject *handle = instance();

    if ( sys_sec < handle->end )
    {
        *total = handle->total;
        *cur   = ( sys_sec - handle->last + 1 );
    }
    else
    {
        *total = 0;
        *cur   = 0;
    }
}

uint32 delay_get_end( void )
{
    DelayObject *handle = instance();

    return handle->end;
}

boolean delay_now_is_in_delay( uint32 cur_sec )
{
    boolean ret = FALSE;

    DelayObject *handle = instance();

    //os_printf( "%d = %d \n", cur_sec, handle->end );
    if ( cur_sec < handle->end )
    {
        ret = TRUE;
    }

    return ret;
}

void ICACHE_FLASH_ATTR delay_destroy( void )
{
    DelayObject *handle = instance();

    os_free( handle );
}

////////////////////////////////////////////////////////////////////////////////
// static function.
////////////////////////////////////////////////////////////////////////////////
static DelayObject *instance( void )
{
    static DelayObject *handle = NULL;
    if ( NULL == handle )
    {
        handle = ( DelayObject * )os_zalloc( sizeof( DelayObject ) );
        os_memset( handle, 0, sizeof( DelayObject ) );
    }

    return handle;
}
