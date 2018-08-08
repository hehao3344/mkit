#ifndef __DEBUG_H
#define __DEBUG_H

#ifndef DEBUG_ON_OFF
#define DEBUG_ON_OFF 1
#endif

#define debug_print(fmt, ...) \
	do { if (DEBUG_ON_OFF) Printf_High("debug: %s():%d " fmt, __func__,\
			__LINE__, ##__VA_ARGS__); } while (0)

#define debug_error(fmt, ...) \
	do { if (DEBUG_ON_OFF) Printf_High("error: %s:%d:%s(): " fmt, __FILE__, \
			__LINE__, __func__, ##__VA_ARGS__); } while (0)

#endif // __DEBUG_H
