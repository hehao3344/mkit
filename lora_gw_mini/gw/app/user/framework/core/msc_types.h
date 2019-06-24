#ifndef __MSC_TYPES_H
#define __MSC_TYPES_H

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) ((int)(sizeof(x)/sizeof((x)[0])))
#endif

// new type define style
typedef unsigned char           boolean;

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
