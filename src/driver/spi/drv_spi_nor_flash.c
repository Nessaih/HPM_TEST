#include <stdbool.h>
#include <stdint.h>
#include "Gpio_Hal.h"
#include "Spi_Hal.h"
#include "api_rtos.h"
#include "drv_log.h"

#define DRV_SPI_NOR_FLASH_INSTANCE 1U

static const Spi_HalConfigType drv_spi_nor_flash_cfg = {
    .CsSetup      = 5U,
    .CsHold       = 5U,
    .CsIdle       = 5U,
    .FrmSize      = 8U,
    .MsbFirst     = TRUE,
    .CsOutputEn   = TRUE,
    .ContinuousCs = TRUE,
    .HreqEn       = FALSE,
    .Mode         = SPI_MASTER,
    .Cpol         = SPI_CPOL_LOW,
    .Cpha         = SPI_CPHA_0,
    .PcsCfg       = SPI_PCS_0,
    .PcsPol       = SPI_PCS_POLARITY_LOW,
    .HreqPol      = SPI_HREQ_POLARITY_HIGH,
    .Width        = SPI_DATA_WIDTH_1BIT,
    .PinCfg       = SPI_SOUT_MOSI_SIN_MISO,
    .Callback     = NULL_PTR,
    .BaudRate     = 8000000UL,
};

static bool          drv_spi_nor_flash_is_init = false;
static volatile bool drv_spi_nor_flash_is_busy = true;

int32_t drv_spi_nor_flash_init(void)
{
    Spi_Hal_Init(DRV_SPI_NOR_FLASH_INSTANCE, &drv_spi_nor_flash_cfg);
    drv_spi_nor_flash_is_init = true;
    drv_spi_nor_flash_is_busy = false;
    return 0;
}

int32_t drv_spi_nor_flash_deinit(void)
{
    Spi_Hal_DeInit(DRV_SPI_NOR_FLASH_INSTANCE);
    drv_spi_nor_flash_is_init = false;
    drv_spi_nor_flash_is_busy = true;
    return 0;
}

int32_t drv_spi_nor_flash_wake(void)
{
    if (!drv_spi_nor_flash_is_init)
    {
        drv_spi_nor_flash_init();
    }
    return 0;
}

int32_t drv_spi_nor_flash_sleep(void)
{
    if (drv_spi_nor_flash_is_init)
    {
        drv_spi_nor_flash_deinit();
    }
    return 0;
}

int32_t drv_spi_nor_flash_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len, Spi_PcsType pcs)
{

    int32_t status;
    int32_t timeout = 10;

    if (drv_spi_nor_flash_is_busy)
        return -1;
    drv_spi_nor_flash_is_busy = true;

    Spi_Hal_SetCsPin(DRV_SPI_NOR_FLASH_INSTANCE, pcs, SPI_PCS_POLARITY_LOW);
    status = Spi_Hal_TransceivePoll(DRV_SPI_NOR_FLASH_INSTANCE, tx_buf, rx_buf, len, 50000UL);

    if (STATUS_SUCCESS != status)
    {
        DRV_LOG_E(DRVSPI, "efs spi transfer error, status = %d", status);
        drv_spi_nor_flash_is_busy = false;
        return -2;
    }

    do
    {
        status = Spi_Hal_GetTransceiveStatus(DRV_SPI_NOR_FLASH_INSTANCE);
        if (SPI_TRANSCEIVE_SUCCESS == status)
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        timeout--;
    } while (timeout > 0);

    if (SPI_TRANSCEIVE_SUCCESS != status)
    {
        DRV_LOG_E(DRVFLASH, "efs spi transfer get status error, status = %d", status);
        drv_spi_nor_flash_is_busy = false;
        return -3;
    }

    drv_spi_nor_flash_is_busy = false;

    return 0;
}