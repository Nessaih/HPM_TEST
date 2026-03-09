#ifndef __FLS_SPI_H__
#define __FLS_SPI_H__

#include <stdint.h>
#include "Spi_Hal.h"

int32_t drv_spi_nor_flash_init(void);
int32_t drv_spi_nor_flash_deinit(void);
int32_t drv_spi_nor_flash_wake(void);
int32_t drv_spi_nor_flash_sleep(void);
int32_t drv_spi_nor_flash_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len, Spi_PcsType pcs);

#endif //__FLS_SPI_H__