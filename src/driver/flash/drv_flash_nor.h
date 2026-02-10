#ifndef __DRV_FLASH_NOR_H__
#define __DRV_FLASH_NOR_H__

#include <stdint.h>

int32_t drv_flash_nor_init(void);
int32_t drv_flash_nor_sleep(void);
int32_t drv_flash_nor_wake(void);
int32_t drv_flash_nor_read(uint32_t addr, uint8_t *data, uint32_t data_len);
int32_t drv_flash_nor_write(uint32_t addr, uint8_t *data, uint32_t data_len);
int32_t drv_flash_nor_erase(uint32_t addr, uint16_t n_4KB);
int32_t drv_flash_nor_read_id(uint32_t *id);

#endif //__DRV_FLASH_NOR_H__