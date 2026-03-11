
#include <stddef.h>
#include "api_rtos.h"
#include "test_can.h"
#include "test_dv.h"
// #include "test_adc.h"
// #include "test_flash.h"
// #include "test_led.h"
// #include "test_rtc.h"
// #include "test_timer.h"

extern void test_j1939_init(void);
extern void test_flash_init(void *param);
extern void vse_test(void);

void test_init_task(void *param)
{
    // log_init();
    // log_register(MODULE_ID_SVR, "SVR", LOG_LVL_INFO);
    // test_led_init(NULL);
    // test_can_init(NULL);
    // test_timer_init(NULL);
    // test_rtc_init(NULL);
    // test_adc_init(NULL);
    // test_log_init(NULL);
    // test_flash_init(NULL);
    // test_j1939_init();
    vse_test();
    test_dv_init(NULL);
    vTaskDelete(NULL);
}

void test_init(void)
{

    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate((TaskFunction_t)test_init_task, /* 任务函数 */
                          (const char *)"test_init",      /* 任务名称 */
                          (configSTACK_DEPTH_TYPE)512,    /* 任务堆栈大小 */
                          (void *)NULL,                   /* 传递给任务函数的参数 */
                          (UBaseType_t)10U,               /* 任务优先级 */
                          (TaskHandle_t *)NULL);          /* 任务句柄 */

    (void)xReturn;
}
