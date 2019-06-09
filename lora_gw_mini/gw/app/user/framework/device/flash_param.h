#ifndef __FLASH_PARAM_H
#define __FLASH_PARAM_H

/* һ���豸����1���п�+4�����豸 */
#define MAX_DEV_COUNT           4
#define DEV_MAC_LEN             6
#define CC_MAC_LEN              6
#define RESET_FLAGS_LEN         8

#define CC_MAC_OFFSET           32
#define RESET_FLAGS_OFFSET      16

// Non-FOTA һ��1024������ ����4��������RF_CAL��Ĭ�ϲ�����ʹ��
//#define BASIC_PARAM_SEC		    0x3C
// 3E0 = 1000 ���24��������ʹ��
#define BASIC_PARAM_SEC		    0x3E0 

#define USER_PARAM_BASIC        0
#define USER_PARAM_DFT_BASIC    1
#define MAX_PARAM_BUF_LEN       64 // int �� buffer

#ifdef __cplusplus
extern "C" {
#endif

void ICACHE_FLASH_ATTR flash_param_get_dev_mac(int index, char *buffer, int len);
void ICACHE_FLASH_ATTR flash_param_get_cc_mac(char *buffer, int len);
void ICACHE_FLASH_ATTR flash_param_set_dev_mac(int index, char *buffer, int len);
void ICACHE_FLASH_ATTR flash_param_set_cc_mac(char *buffer, int len);
void ICACHE_FLASH_ATTR flash_param_get_reset_flags(char *buffer, int len);
void ICACHE_FLASH_ATTR flash_param_set_reset_flags(char *buffer, int len);

#ifdef __cplusplus
}
#endif

#endif // __FLASH_PARAM_H
