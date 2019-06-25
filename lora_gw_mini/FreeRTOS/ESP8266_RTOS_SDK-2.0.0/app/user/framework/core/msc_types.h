#ifndef __MSC_TYPES_H
#define __MSC_TYPES_H

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) ((int)(sizeof(x)/sizeof((x)[0])))
#endif

// new type define style
typedef unsigned char           boolean;

#ifndef os_strlen
    #define os_strlen   strlen
#endif

#ifndef os_memset
    #define os_memset   memset
#endif

#ifndef os_memcpy
    #define os_memcpy   memcpy
#endif

#ifndef os_sprintf
    #define os_sprintf  sprintf
#endif

#ifndef os_free
    #define os_free     free
#endif

#if 0
//typedef	char    		    int8;
typedef	unsigned char	        uint8;
typedef	short int		        int16;
typedef	unsigned short int      uint16;
typedef	int			            int32;
typedef	unsigned int	        uint32;
typedef	long long               int64;
typedef	unsigned long long      uint64;
#endif

#endif // __MSC_TYPES_H
