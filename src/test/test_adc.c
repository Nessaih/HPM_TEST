#include "api_rtos.h"
#include "drv_adc.h"
#include "main.h"
#include "log.h"

static TaskHandle_t xTaskHandle = NULL;

void test_adc_task(void *param)
{
    uint8_t adc_id    = 0;
    int32_t adc_value = 0;

    for(;;) {

        adc_value = drv_adc_get(adc_id);
        log_print("ADC%d value: %d\n", adc_id, adc_value);
        if (++adc_id >= DRV_ADC_COUNT)
            adc_id = 0;
        vTaskDelay(3 * 1000);
    }
}

void test_adc_init(void *param)
{
    BaseType_t xReturn = pdPASS;


    xReturn = xTaskCreate(
        (TaskFunction_t)test_adc_task, /* 任务函数 */
        (const char *)"adc_task",      /* 任务名称 */
        (configSTACK_DEPTH_TYPE)256,   /* 任务堆栈大小 */
        (void *)NULL,                  /* 传递给任务函数的参数 */
        (UBaseType_t)19,               /* 任务优先级 */
        (TaskHandle_t *)&xTaskHandle); /* 任务句柄 */

    configASSERT(pdPASS == xReturn);
}
