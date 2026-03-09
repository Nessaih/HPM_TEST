#ifndef __DRV_SPI_SD_FLASH_H__
#define __DRV_SPI_SD_FLASH_H__

#include <stdint.h>

void    drv_spi_sd_flash_init(void);
void    drv_spi_sd_flash_set_speed(uint8_t speed);
uint8_t drv_spi_sd_flash_transfer(uint8_t txdata);

#endif //__DRV_SPI_SD_FLASH_H__