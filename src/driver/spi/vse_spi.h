#ifndef __VSE_SPI_H__
#define __VSE_SPI_H__

#include <stdint.h>


int32_t vse_spi_open(void);
int32_t vse_spi_close(void);
int32_t vse_spi_send(uint8_t *data, uint16_t length);
int32_t vse_spi_recv(uint8_t *data, uint16_t length);

#endif //__VSE_SPI_H__