#ifndef __DRV_FLASH_SD_H__
#define __DRV_FLASH_SD_H__

#include <stdint.h>

int32_t drv_flash_sd_init(void);
int32_t drv_flash_sd_sleep(void);
int32_t drv_flash_sd_wake(void);
int32_t drv_flash_sd_read(uint32_t addr, uint8_t *data, uint32_t data_len);
int32_t drv_flash_sd_write(uint32_t addr, uint8_t *data, uint32_t data_len);
int32_t drv_flash_sd_get_id(uint32_t *id);
int32_t drv_flash_sd_get_size(uint32_t *size);

#endif //__DRV_FLASH_SD_H__