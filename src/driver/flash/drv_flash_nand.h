#ifndef __DRV_FLASH_NAND_H__
#define __DRV_FLASH_NAND_H__

#include <stdint.h>

int32_t drv_flash_nand_init(void);
int32_t drv_flash_nand_sleep(void);
int32_t drv_flash_nand_wake(void);
int32_t drv_flash_nand_read(uint32_t addr, uint8_t *data, uint32_t data_len);
int32_t drv_flash_nand_write(uint32_t addr, uint8_t *data, uint32_t data_len);
int32_t drv_flash_nand_erase(uint32_t addr, uint16_t n_128KB);
int32_t drv_flash_nand_get_id(uint32_t *id);

#endif //__DRV_FLASH_NAND_H__