#ifndef __HPM_FLASH_ADAPTER_H__
#define __HPM_FLASH_ADAPTER_H__

INT32 hpm_flash_adapter_init(VOID);
INT32 hpm_flash_adapter_read(UINT32 addr, UINT8 *buf, UINT32 len);
INT32 hpm_flash_adapter_write(UINT32 addr, UINT8 *buf, UINT32 len);
INT32 hpm_flash_adapter_erase(UINT32 addr, UINT16 len);
VOID hpm_flash_adapter_show(VOID);

#endif