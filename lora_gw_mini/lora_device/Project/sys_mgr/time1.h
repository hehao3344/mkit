#ifndef __TIME1_H
#define __TIME1_H

#include "stm8s.h"

void time1_init( void );
void time1_increment( void );
void time1_set_value( u32 index, u32 value );
u32  time1_get_value( u32 index );

#endif