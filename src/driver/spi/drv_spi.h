#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include <stdint.h>

struct spi_config
{
    uint8_t  instance;
    uint8_t  mode;
    uint8_t  cs_mode;
    uint8_t  cs_index;
    uint32_t speed;
};

struct spi_handle
{
    uint8_t          instance;
    uint8_t          mode;
    uint8_t          cs_mode;
    uint8_t          cs_index;
    uint32_t         speed;
    volatile uint8_t is_init;
    volatile uint8_t is_buzy;
};

typedef struct spi_config drv_spi_config_t;
typedef struct spi_handle drv_spi_handle_t;


#define DRV_SPI_INSTANCE_COUNT 3

#define DRV_SPI_MODE_0         0
#define DRV_SPI_MODE_1         1
#define DRV_SPI_MODE_2         2
#define DRV_SPI_MODE_3         3

#define DRV_SPI_CS_MODE_AUTO   0
#define DRV_SPI_CS_MODE_MANUAL 1

#define DRV_SPI_CS_INDEX_0     0
#define DRV_SPI_CS_INDEX_1     1
#define DRV_SPI_CS_INDEX_2     2
#define DRV_SPI_CS_INDEX_3     3
#define DRV_SPI_CS_INDEX_GPIO  4



int32_t drv_spi_init(drv_spi_config_t *config, drv_spi_handle_t *handle);
int32_t drv_spi_deinit(drv_spi_handle_t *handle);
int32_t drv_spi_read(drv_spi_handle_t *handle, uint8_t *buf, uint32_t length);
int32_t drv_spi_write(drv_spi_handle_t *handle, uint8_t *buf, uint32_t length);
int32_t drv_spi_transfer(drv_spi_handle_t *handle, uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length);

#endif //__DRV_SPI_H__