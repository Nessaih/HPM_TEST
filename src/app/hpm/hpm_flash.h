#ifndef __HPM_FLASH_H__
#define __HPM_FLASH_H__

typedef enum
{
    HPM_FLASH_INIT_FAILED = -3,
    HPM_FLASH_DEV_FORMATTING = -2,
    HPM_FLASH_ERROR = -1,
    HPM_FLASH_OK = 0
} HPM_FLASH_ERROR_CODE;

VOID hpm_flash_init(VOID);

VOID hpm_flash_sleep(VOID);

UINT8 hpm_flash_save_pack(UINT8* data, UINT16 len, UINT8 cmd);

UINT8 hpm_flash_load_pack(UINT8* data, UINT16* len, UINT8 *cmd);

UINT8 hpm_flash_clear_info(VOID);

VOID hpm_flash_period(VOID);

VOID hpm_flash_print_info(VOID);

#endif
