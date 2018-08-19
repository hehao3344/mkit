#ifndef __CORE_H
#define __CORE_H

#ifdef WIN32
#include <stdio.h>
#else
#include <stdlib.h>
#include <stdio.h>
#endif

//#define TRUE                    1
//#define FALSE                   0

// new type define style
typedef unsigned char           boolean;
typedef char                    int8;
typedef unsigned char           uint8;
typedef short int               int16;
typedef unsigned short int      uint16;
typedef long                    int32;
typedef unsigned long           uint32;
typedef long long               int64;
typedef unsigned long long      uint64;
typedef int                     intptr;
typedef unsigned int            uintptr;

#endif // __CORE_H
