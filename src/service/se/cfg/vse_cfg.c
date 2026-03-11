
#include "tbox_log.h"
#include "time_if.h"
#include "api_rtos.h"
#include "delay.h"
#include "drv_pin.h"
#include "drv_spi.h"
#include "vse_cfg.h"
#include "OsIf_Time.h"

#define VSE_DEBUG_ENABLE 1

static drv_spi_handle_t vse_spi_handle;
static drv_spi_config_t vse_spi_config = {
    .instance = 0,
    .mode     = DRV_SPI_MODE_0,
    .cs_mode  = DRV_SPI_CS_MODE_AUTO,
    .cs_index = DRV_SPI_CS_INDEX_0,
    .speed    = 500000UL,
};

void vse_gpio_reset(void)
{
    drv_pin_set_level(PIN_RESET_VSE, 1U);
    vse_delay_ms(3);
    drv_pin_set_level(PIN_RESET_VSE, 0U);
}

void vse_gpio_wakeup(void)
{
    drv_pin_set_level(PIN_RESET_VSE, 1U);
    vse_delay_us(90);
    drv_pin_set_level(PIN_RESET_VSE, 0U);
    vse_delay_us(210);
}

void vse_delay_us(uint16_t usec)
{
    delay_us(usec);
}

void vse_delay_ms(uint16_t msec)
{
    delay_ms(msec);
}

int32_t vse_get_time(vse_time_t *time)
{
    uint32_t tick = xTaskGetTickCount();

    time->sec  = time_if_get_basetime_utc_s() + tick / 1000;
    time->msec = tick % 1000;
    return 0;
}

int32_t vse_init(void)
{
    drv_spi_init(&vse_spi_config, &vse_spi_handle);
    return 0;
}

int32_t vse_deinit(void)
{
    drv_spi_deinit(&vse_spi_handle);
    return 0;
}

int32_t vse_send(uint8_t *data, uint16_t length)
{
    if ((data == NULL) || (length == 0))
    {
        MODULE_LOG_E(SE, "invalid params.");
        return -1;
    }

    if (0 != drv_spi_write(&vse_spi_handle, data, length))
    {
        return -1;
    }

    return 0;
}

int32_t vse_recv(uint8_t *data, uint16_t length)
{
    if ((data == NULL) || (length == 0))
    {
        MODULE_LOG_E(SE, "invalid params.");
        return -1;
    }

    if (0 != drv_spi_read(&vse_spi_handle, data, length))
    {
        return -1;
    }

    return 0;
}

void vse_printf(const char *fmt, ...)
{
#if VSE_DEBUG_ENABLE
    static char vse_log[512];

    va_list args;
    va_start(args, fmt);
    vsnprintf(vse_log, sizeof(vse_log) - 1, fmt, args);
    va_end(args);
    tbox_log_raw_output(vse_log, strlen(vse_log));
    tbox_log_flush();
#endif
}
