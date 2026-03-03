#include <stdbool.h>
#include <stdint.h>
#include "Gpio_Hal.h"
#include "Spi_Hal.h"
#include "api_rtos.h"
#include "drv_log.h"

#define FLS_SPI_INSTANCE 1U

static const Spi_HalConfigType fls_spi_cfg = {
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

static bool          fls_spi_is_init = false;
static volatile bool fls_spi_is_busy = true;

int32_t fls_spi_init(void)
{
    Spi_Hal_Init(FLS_SPI_INSTANCE, &fls_spi_cfg);
    fls_spi_is_init = true;
    fls_spi_is_busy = false;
    return 0;
}

int32_t fls_spi_deinit(void)
{
    Spi_Hal_DeInit(FLS_SPI_INSTANCE);
    fls_spi_is_init = false;
    fls_spi_is_busy = true;
    return 0;
}

int32_t fls_spi_wake(void)
{
    if (!fls_spi_is_init)
    {
        fls_spi_init();
    }
    return 0;
}

int32_t fls_spi_sleep(void)
{
    if (fls_spi_is_init)
    {
        fls_spi_deinit();
    }
    return 0;
}

int32_t fls_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len, Spi_PcsType pcs)
{

    int32_t status;
    int32_t timeout = 10;

    if (fls_spi_is_busy)
        return -1;
    fls_spi_is_busy = true;

    Spi_Hal_SetCsPin(FLS_SPI_INSTANCE, pcs, SPI_PCS_POLARITY_LOW);
    status = Spi_Hal_TransceivePoll(FLS_SPI_INSTANCE, tx_buf, rx_buf, len, 50000UL);

    if (STATUS_SUCCESS != status)
    {
        DRV_LOG_E(DRVSPI, "efs spi transfer error, status = %d", status);
        fls_spi_is_busy = false;
        return -2;
    }

    do
    {
        status = Spi_Hal_GetTransceiveStatus(FLS_SPI_INSTANCE);
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
        fls_spi_is_busy = false;
        return -3;
    }

    fls_spi_is_busy = false;

    return 0;
}