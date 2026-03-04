#include "api_drv.h"
#include "delay.h"

#define DRV_INIT_RETRY(func, retry)                                                                                                                  \
    for (int i = 0; i < retry; i++)                                                                                                                  \
    {                                                                                                                                                \
        if (!func())                                                                                                                                 \
        {                                                                                                                                            \
            break;                                                                                                                                   \
        }                                                                                                                                            \
        delay_ms(100);                                                                                                                               \
    }

void driver_init(void)
{
    DRV_INIT_RETRY(drv_clock_init, 3);
    DRV_INIT_RETRY(drv_nvic_init, 3);
    DRV_INIT_RETRY(drv_pin_init, 3);
    DRV_INIT_RETRY(drv_uart_init, 3);
    DRV_INIT_RETRY(drv_uart_485_init, 3);
    DRV_INIT_RETRY(drv_uart_bt_init, 3);
    DRV_INIT_RETRY(fls_spi_init, 3);
    DRV_INIT_RETRY(drv_adc_init, 3);
    DRV_INIT_RETRY(drv_i2c_init, 3);
    DRV_INIT_RETRY(drv_flash_nor_init, 3);
    DRV_INIT_RETRY(drv_flash_nand_init, 3);
    DRV_INIT_RETRY(drv_spm_init, 3);
	// DRV_INIT_RETRY(drv_eio_gnss_init, 3);
    DRV_INIT_RETRY(drv_wdg_init, 3);
}

#ifndef __BOOTLOADER__

void driver_sleep(void)
{
    drv_uart_sleep();
    drv_uart_485_sleep();
    drv_uart_bt_sleep();
    fls_spi_sleep();
    drv_adc_sleep();
    drv_i2c_sleep();
    drv_flash_nor_sleep();
    drv_flash_nand_sleep();
    drv_wdg_sleep();
}

void driver_wake(void)
{
    drv_uart_wake();
    drv_uart_485_wake();
    drv_uart_bt_wake();
    fls_spi_wake();
    drv_adc_wake();
    drv_i2c_wake();
    drv_flash_nor_wake();
    drv_flash_nand_wake();
    drv_wdg_wake();
}

#endif