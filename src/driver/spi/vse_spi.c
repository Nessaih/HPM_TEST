#include "Spi_Hal.h"
#include "api_rtos.h"
#include "vse_spi.h"
#include "drv_pin.h"
#include "delay.h"
#include "drv_log.h"

#define VSE_SPI_INSTANCE 0U


static const Spi_HalConfigType vse_spi_cfg = {
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
    .BaudRate     = 1000000UL,
};


int32_t vse_spi_open(void)
{
    Spi_Hal_Init(VSE_SPI_INSTANCE, &vse_spi_cfg);
    return 0;
}

int32_t vse_spi_close(void)
{
    Spi_Hal_DeInit(VSE_SPI_INSTANCE);
    return 0;
}

int32_t vse_spi_send(uint8_t *data, uint16_t length)
{
    int32_t status;
    int32_t timeout = 10000;


    status = Spi_Hal_TransceivePoll(VSE_SPI_INSTANCE, data, NULL_PTR, length, 50000UL);
    do
    {
        status = Spi_Hal_GetTransceiveStatus(VSE_SPI_INSTANCE);
        delay_us(200);
        if (SPI_TRANSCEIVE_SUCCESS == status)
        {
            break;
        }
        timeout--;
    } while (timeout > 0);

    if (SPI_TRANSCEIVE_SUCCESS != status)
    {
        DRV_LOG_E(DRVSPI, "efs spi transfer get status error, status = %d", status);
        return 1;
    }
    return 0;
}



int32_t vse_spi_recv(uint8_t *data, uint16_t length)
{
    int32_t status;
    int32_t timeout = 10000;

    status = Spi_Hal_TransceivePoll(VSE_SPI_INSTANCE, NULL_PTR, data, length, 50000UL);

    do
    {
        status = Spi_Hal_GetTransceiveStatus(VSE_SPI_INSTANCE);
        delay_us(200);
        if (SPI_TRANSCEIVE_SUCCESS == status)
        {
            break;
        }
        timeout--;
    } while (timeout > 0);

    if (SPI_TRANSCEIVE_SUCCESS != status)
    {
        DRV_LOG_E(DRVSPI, "efs spi transfer get status error, status = %d", status);
        return 1;
    }
    
    return 0;
}

