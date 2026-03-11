#include "Spi_Hal.h"
#include "delay.h"
#include "drv_log.h"
#include "drv_spi.h"

#define DRV_SPI_NAME "SPI"

static Spi_HalConfigType *spi_init_hal_config(drv_spi_config_t *config)
{
    static Spi_HalConfigType hal_cfg;

    hal_cfg.CsSetup  = 5U;
    hal_cfg.CsHold   = 5U;
    hal_cfg.CsIdle   = 5U;
    hal_cfg.FrmSize  = 8U;
    hal_cfg.MsbFirst = TRUE;
    hal_cfg.HreqEn   = FALSE;
    hal_cfg.Mode     = SPI_MASTER;
    hal_cfg.Cpol     = SPI_CPOL_LOW;
    hal_cfg.Cpha     = SPI_CPHA_0;
    hal_cfg.PcsCfg   = SPI_PCS_0;
    hal_cfg.PcsPol   = SPI_PCS_POLARITY_LOW;
    hal_cfg.HreqPol  = SPI_HREQ_POLARITY_HIGH;
    hal_cfg.Width    = SPI_DATA_WIDTH_1BIT;
    hal_cfg.PinCfg   = SPI_SOUT_MOSI_SIN_MISO;
    hal_cfg.Callback = NULL_PTR;

    hal_cfg.BaudRate = config->speed;

    if (config->cs_mode == DRV_SPI_CS_MODE_AUTO)
    {
        hal_cfg.CsOutputEn   = TRUE;
        hal_cfg.ContinuousCs = TRUE;
    }
    else
    {
        hal_cfg.CsOutputEn   = FALSE;
        hal_cfg.ContinuousCs = FALSE;
    }

    switch (config->mode)
    {
    case DRV_SPI_MODE_1:
        hal_cfg.Cpol = SPI_CPOL_LOW;
        hal_cfg.Cpha = SPI_CPHA_1;
        break;

    case DRV_SPI_MODE_2:
        hal_cfg.Cpol = SPI_CPOL_HIGH;
        hal_cfg.Cpha = SPI_CPHA_0;
        break;

    case DRV_SPI_MODE_3:
        hal_cfg.Cpol = SPI_CPOL_HIGH;
        hal_cfg.Cpha = SPI_CPHA_1;
        break;

    default:
        hal_cfg.Cpol = SPI_CPOL_LOW;
        hal_cfg.Cpha = SPI_CPHA_0;
        break;
    }

    return &hal_cfg;
}

static int32_t check_instance_valid(drv_spi_handle_t *handle)
{
    if (handle == NULL)
        return -1;
    if (handle->instance >= DRV_SPI_INSTANCE_COUNT)
        return -2;
    if (handle->is_init == FALSE)
        return -3;
    if (handle->is_buzy)
        return -4;

    return 0;
}

int32_t drv_spi_init(drv_spi_config_t *config, drv_spi_handle_t *handle)
{
    if (config == NULL || handle == NULL)
        return -1;

    if (config->instance >= DRV_SPI_INSTANCE_COUNT)
        return -2;

    handle->instance = config->instance;
    handle->mode     = config->mode;
    handle->cs_mode  = config->cs_mode;
    handle->cs_index = config->cs_index;
    handle->speed    = config->speed;
    handle->is_init  = FALSE;
    handle->is_buzy  = FALSE;

    Spi_HalConfigType *hal_cfg = spi_init_hal_config(config);

    Spi_Hal_Init(config->instance, hal_cfg);

    handle->is_init = TRUE;
    handle->is_buzy = FALSE;

    return 0;
}

int32_t drv_spi_deinit(drv_spi_handle_t *handle)
{
    if (handle == NULL)
        return -1;

    if (handle->is_init == FALSE)
        return -2;

    Spi_Hal_DeInit(handle->instance);

    handle->is_init = FALSE;
    handle->is_buzy = FALSE;

    return 0;
}

int32_t drv_spi_read(drv_spi_handle_t *handle, uint8_t *buf, uint32_t length)
{
    return drv_spi_transfer(handle, NULL, buf, length);
}

int32_t drv_spi_write(drv_spi_handle_t *handle, uint8_t *buf, uint32_t length)
{
    return drv_spi_transfer(handle, buf, NULL, length);
}

int32_t drv_spi_transfer(drv_spi_handle_t *handle, uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length)
{
    int32_t status  = check_instance_valid(handle);
    int32_t timeout = 100;

    if (status)
    {
        DRV_LOG_E(DRV_SPI_NAME, "spi%d invalid:%d", handle->instance, status);
        return status;
    }

    handle->is_buzy = TRUE;

    if (handle->cs_mode == DRV_SPI_CS_MODE_AUTO)
    {
        Spi_Hal_SetCsPin(handle->instance, (Spi_PcsType)handle->cs_index, SPI_PCS_POLARITY_LOW);
    }
    status = Spi_Hal_TransceivePoll(handle->instance, tx_buf, rx_buf, length, 50000UL);
    if (STATUS_SUCCESS != status)
    {
        DRV_LOG_E(DRV_SPI_NAME, "spi%d error:%d", handle->instance, status);
        handle->is_buzy = FALSE;
        return -5;
    }

    do
    {
        status = Spi_Hal_GetTransceiveStatus(handle->instance);
        if (SPI_TRANSCEIVE_SUCCESS == status)
        {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1));

        timeout--;
    } while (timeout > 0);

    handle->is_buzy = FALSE;

    if (SPI_TRANSCEIVE_SUCCESS != status)
    {

        DRV_LOG_E(DRV_SPI_NAME, "spi%d error:%d", handle->instance, status);

        return -6;
    }

    return 0;
}
