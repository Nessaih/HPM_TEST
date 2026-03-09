#ifndef __DRV_SPI_VSE_H__
#define __DRV_SPI_VSE_H__

#include <stdint.h>


int32_t drv_spi_vse_open(void);
int32_t drv_spi_vse_close(void);
int32_t drv_spi_vse_send(uint8_t *data, uint16_t length);
int32_t drv_spi_vse_recv(uint8_t *data, uint16_t length);


#endif //__DRV_SPI_VSE_H__