#include <string.h>
#include "tbox_log.h"
#include "api_rtos.h"
#include "drv_flash_sd.h"
#include "spi_sdcard.h"


static TaskHandle_t xTaskHandle = NULL;
static uint8_t      write_data[512];
static uint8_t      read_data[512];

void test_sd_flash1(void)
{
    if (sd_init())
    {
        tbox_log_print("\nSD card init Failed!\n");
        return;
    }
    else
    {
        tbox_log_print("\nSD card init success!\n");
    }

    uint32_t c = sd_get_sector_count();

    tbox_log_print("\nSD card sector count:%d\n", c);

    memset(write_data, 0xAB, sizeof(write_data));
    if (sd_write_disk(write_data, 0, 1))
    {
        tbox_log_print("\nSD card write Failed!\n");
        return;
    }

    memset(read_data, 0x00, sizeof(read_data));
    if (sd_read_disk(read_data, 0, 1))
    {
        tbox_log_print("\nSD card read Failed!\n");
        return;
    }

    for (size_t j = 0; j < 16; j++)
    {
        for (size_t i = 0; i < 32; i++)
        {
            tbox_log_print("%02X ", read_data[j * 32 + i]);
        }
        tbox_log_print("\n");
    }
}

void test_sd_flash2(void)
{
    if (drv_flash_sd_init())
    {
        tbox_log_print("\nSD card init Failed!\n");
        return;
    }
    else
    {
        tbox_log_print("\nSD card init success!\n");
    }

    memset(write_data, 0xAB, sizeof(write_data));
    drv_flash_sd_write(0, write_data, 512);

    memset(read_data, 0x00, sizeof(read_data));
    drv_flash_sd_read(0, read_data, 512);
    for (size_t j = 0; j < 16; j++)
    {
        for (size_t i = 0; i < 32; i++)
        {
            tbox_log_print("%02X ", read_data[j * 32 + i]);
        }
        tbox_log_print("\n");
    }
}

void test_flash_task(void *param)
{
    // drv_flash_init();
    // test_flash_write();
    // test_flash_read();
    test_sd_flash1();
    vTaskDelete(NULL);
}

void test_flash_init(void *param)
{
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate((TaskFunction_t)test_flash_task, /* 任务函数 */
                          (const char *)"flash_task",      /* 任务名称 */
                          (configSTACK_DEPTH_TYPE)512,     /* 任务堆栈大小 */
                          (void *)NULL,                    /* 传递给任务函数的参数 */
                          (UBaseType_t)19,                 /* 任务优先级 */
                          (TaskHandle_t *)&xTaskHandle);   /* 任务句柄 */

    configASSERT(pdPASS == xReturn);
}
