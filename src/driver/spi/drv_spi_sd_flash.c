#include <stdbool.h>
#include <stdint.h>
#include "Gpio_Hal.h"
#include "Spi_Hal.h"
#include "api_rtos.h"
#include "delay.h"
#include "drv_log.h"

#define DRV_SPI_SD_FLASH_INSTANCE 2U
#define DRV_SPI_SD_LOG_NAME       "SPI2"

static Spi_HalConfigType drv_spi_sd_flash_cfg = {
    .CsSetup      = 5U,
    .CsHold       = 5U,
    .CsIdle       = 5U,
    .FrmSize      = 8U,
    .MsbFirst     = TRUE,
    .CsOutputEn   = FALSE,
    .ContinuousCs = FALSE,
    .HreqEn       = FALSE,
    .Mode         = SPI_MASTER,
    .Cpol         = SPI_CPOL_LOW,
    .Cpha         = SPI_CPHA_0,
    .PcsCfg       = SPI_PCS_GPIO,
    .PcsPol       = SPI_PCS_POLARITY_LOW,
    .HreqPol      = SPI_HREQ_POLARITY_HIGH,
    .Width        = SPI_DATA_WIDTH_1BIT,
    .PinCfg       = SPI_SOUT_MOSI_SIN_MISO,
    .Callback     = NULL_PTR,
    .BaudRate     = 200000UL,
};

static bool drv_spi_sd_flash_is_init = false;

void drv_spi_sd_flash_init(void)
{
    Spi_Hal_Init(DRV_SPI_SD_FLASH_INSTANCE, &drv_spi_sd_flash_cfg);
    drv_spi_sd_flash_is_init = true;
}

int32_t drv_spi_sd_flash_deinit(void)
{
    Spi_Hal_DeInit(DRV_SPI_SD_FLASH_INSTANCE);
    drv_spi_sd_flash_is_init = false;
    return 0;
}

int32_t drv_spi_sd_flash_wake(void)
{
    if (!drv_spi_sd_flash_is_init)
    {
        drv_spi_sd_flash_init();
    }
    return 0;
}

int32_t drv_spi_sd_flash_sleep(void)
{
    if (drv_spi_sd_flash_is_init)
    {
        drv_spi_sd_flash_deinit();
    }
    return 0;
}

uint8_t drv_spi_sd_flash_transfer(uint8_t txdata)
{
    int32_t status;
    int32_t timeout = 10;
    uint8_t rx_buf  = 0;

    status = Spi_Hal_TransceivePoll(DRV_SPI_SD_FLASH_INSTANCE, &txdata, &rx_buf, 1, 5000UL);

    if (STATUS_SUCCESS != status)
    {
        DRV_LOG_E(DRV_SPI_SD_LOG_NAME, "error:%d", status);
        return 0xFF;
    }

    do
    {
        status = Spi_Hal_GetTransceiveStatus(DRV_SPI_SD_FLASH_INSTANCE);
        if (SPI_TRANSCEIVE_SUCCESS == status)
        {
            break;
        }
        delay_us(100);
        timeout--;
    } while (timeout > 0);

    if (SPI_TRANSCEIVE_SUCCESS != status)
    {
        DRV_LOG_E(DRV_SPI_SD_LOG_NAME, "error:%d", status);
        return 1;
    }

    return rx_buf;
}

void drv_spi_sd_flash_set_speed(uint32_t speed)
{

    if (speed == drv_spi_sd_flash_cfg.BaudRate && drv_spi_sd_flash_is_init)
        return;

    drv_spi_sd_flash_deinit();
    drv_spi_sd_flash_cfg.BaudRate = speed;
    drv_spi_sd_flash_init();
}