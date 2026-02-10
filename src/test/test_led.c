#include "api_rtos.h"
#include "drv_pin.h"

static TaskHandle_t  xTaskHandle  = NULL;
static TimerHandle_t xTimerHandle = NULL;

void led_twinkle_task(void *param)
{
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // drv_pin_toggle(PIN_TEST_OUT);
    }
}

void led_timer_callback(TimerHandle_t xTimer)
{
    xTaskNotifyGive(xTaskHandle);
}

void test_led_init(void *param)
{
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate(
        (TaskFunction_t)led_twinkle_task, /* 任务函数 */
        (const char *)"led_twinkle",      /* 任务名称 */
        (configSTACK_DEPTH_TYPE)256,      /* 任务堆栈大小 */
        (void *)NULL,                     /* 传递给任务函数的参数 */
        (UBaseType_t)19,                  /* 任务优先级 */
        (TaskHandle_t *)&xTaskHandle);    /* 任务句柄 */

    configASSERT(pdPASS == xReturn);

    xTimerHandle = xTimerCreate(
        "led_timer",         /* 定时器名称 */
        pdMS_TO_TICKS(1000), /* 周期（ms） */
        pdTRUE,              /* 自动重载（周期定时器） */
        (void *)0,           /* 定时器ID（可自定义） */
        led_timer_callback); /* 回调函数 */

    configASSERT(NULL != xTimerHandle);

    vTaskDelay(pdMS_TO_TICKS(10)); /*确保任务已启动后再启动定时器*/
    xReturn = xTimerStart(xTimerHandle, 0);

    configASSERT(pdPASS == xReturn);
}
