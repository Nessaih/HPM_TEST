#ifndef __DRV_FLASH_MCU_H__
#define __DRV_FLASH_MCU_H__

#include <stdint.h>

int32_t drv_flash_mcu_init(void);
int32_t drv_flash_mcu_erase(uint32_t addr, uint32_t size);
int32_t drv_flash_mcu_read(uint32_t addr, uint8_t *data, uint32_t len);
int32_t drv_flash_mcu_write(uint32_t addr, const uint8_t *data, uint32_t len);

#endif //__DRV_FLASH_MCU_H__