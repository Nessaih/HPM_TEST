#include "api_rtos.h"
#include "drv_flash.h"
#include "log.h"
#include "main.h"
#include "module_id.h"


static TaskHandle_t xTaskHandle = NULL;

void test_flash_write(void)
{
    uint8_t data[100] = {0};

    for (size_t i = 0; i < 100; i++) {
        data[i] = i + 1;
    }

    drv_flash_erase(0x01000000, 100);
    drv_flash_write(0x01000000, data, 100);

}


void test_flash_read(void)
{
    uint8_t data[100] = {0};


    drv_flash_read(0x01000000, data, 100);

    log_dumphex(MODULE_ID_SVR, "flash read data:", data, 100);
}


void test_flash_task(void *param)
{
    drv_flash_init();
    test_flash_write();
    test_flash_read();
    vTaskDelete(NULL);
}

void test_flash_init(void *param)
{
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate(
        (TaskFunction_t)test_flash_task, /* 任务函数 */
        (const char *)"flash_task",      /* 任务名称 */
        (configSTACK_DEPTH_TYPE)512,     /* 任务堆栈大小 */
        (void *)NULL,                    /* 传递给任务函数的参数 */
        (UBaseType_t)19,                 /* 任务优先级 */
        (TaskHandle_t *)&xTaskHandle);   /* 任务句柄 */

    configASSERT(pdPASS == xReturn);
}
