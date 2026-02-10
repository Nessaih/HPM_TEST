#include "api_rtos.h"
#include "drv_rtc.h"
#include "log.h"


static TaskHandle_t  xTaskHandle  = NULL;


void test_rtc_task(void *param)
{
    rtc_time_t time;

    time.year   = 25;
    time.month  = 11;
    time.day    = 25;
    time.hour   = 11;
    time.minute = 30;
    time.second = 30;
    drv_rtc_set_time(&time);

    for(;;) {
        drv_rtc_get_time(&time);
        log_print("RTC time: %02d-%02d-%02d %02d:%02d:%02d\n", time.year, time.month, time.day, time.hour, time.minute, time.second);
        vTaskDelay(60 * 1000);
    }
}

void test_rtc_init(void *param)
{
    BaseType_t xReturn = pdPASS;

    // drv_rtc_init();
    // drv_rtc_check();

    xReturn = xTaskCreate(
        (TaskFunction_t)test_rtc_task, /* 任务函数 */
        (const char *)"rtc_task",      /* 任务名称 */
        (configSTACK_DEPTH_TYPE)256,   /* 任务堆栈大小 */
        (void *)NULL,                  /* 传递给任务函数的参数 */
        (UBaseType_t)19,               /* 任务优先级 */
        (TaskHandle_t *)&xTaskHandle); /* 任务句柄 */

    configASSERT(pdPASS == xReturn);
}
