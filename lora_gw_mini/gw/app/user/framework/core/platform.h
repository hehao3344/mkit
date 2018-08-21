#ifndef __PLATFORM_H
#define __PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_PLATFORM 1

#ifdef  ESP_PLATFORM
#define VALUE_ZONE      0x2F3   // 0755
#define VALUE_MODEL     0x5DD   // 1501
#define VALUE_HV        0x4E85  // 20010
#define VALUE_SV        0x271A  // 10010
#define ID_LEN          8
#endif

#ifndef ESP_PLATFORM

#define ICACHE_FLASH_ATTR
#define os_strncmp strncmp
#define os_strlen  strlen
#define os_malloc  malloc
#define os_free    free
#define os_memcpy  memcpy
#define os_strstr  strstr
#define os_zalloc  malloc
#define os_malloc  malloc
#define os_strcpy  strcpy
#define os_memset  memset
#define os_strcmp  strcmp
#define os_sscanf  sscanf

#else
// use the function in lwip lib.
#undef strlen
#undef malloc
#undef free
#undef memcpy
#undef malloc
#undef malloc
#undef strcpy
#undef memset
#undef strcmp
#undef sscanf

#define strlen os_strlen
#define malloc os_malloc
#define free   os_free
#define memcpy os_memcpy
#define strstr os_strstr
#define malloc os_malloc
#define strcpy os_strcpy
#define memset os_memset
#define strcmp os_strcmp
#define sscanf os_sscanf

#endif

#ifdef __cplusplus
}
#endif

#endif // __PLATFORM_H
