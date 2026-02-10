
#include "api_rtos.h"
#include "delay.h"
#include "time_if.h"
#include "drv_pin.h"
#include "tbox_log.h"
#include "vse_cfg.h"
#include "vse_spi.h"

#define VSE_DEBUG_ENABLE 1

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
    uint32_t tick;

    tick       = (xTaskGetTickCount() / 10) - time_if_get_basetime_tick();
    time->sec  = time_if_get_basetime_utc_s() + tick / 100;
    time->msec = (tick % 100) * 10;
    return 0;
}

int32_t vse_init(void)
{
    return vse_spi_open();
}

int32_t vse_deinit(void)
{
    return vse_spi_close();
}

int32_t vse_send(uint8_t *data, uint16_t length)
{
    if ((data == NULL) || (length == 0))
    {
        MODULE_LOG_E(SE, "invalid params.");
        return -1;
    }

    if (0 != vse_spi_send(data, length))
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

    if (0 != vse_spi_recv(data, length))
    {
        return -1;
    }

    return 0;
}


void vse_printf(const char *fmt, ...)
{
#if VSE_DEBUG_ENABLE
    static char vse_log[512];

    va_list     args;
    va_start(args, fmt);
    vsnprintf(vse_log, sizeof(vse_log) - 1, fmt, args);
    va_end(args);
    tbox_log_flush();
#endif
}
