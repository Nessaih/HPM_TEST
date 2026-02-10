#include "api_rtos.h"
#include "drv_timer.h"
#include "log.h"

enum { MODULE_MAIN = 10, MODULE_UART, MODULE_ISR };

static TaskHandle_t xTaskHandle = NULL;
static drv_timer_t  test_timer;
static drv_tcb_t    test_tcb;

static void timer_callback(void)
{
    MODULE_LOG_E(MODULE_ISR, "log from isr");
}


void test_log_task(void *param)
{

    uint8_t test_data[] = {0x11, 0x22, 0x33, 0x44, 'A', 'B', 'C'};

    for(;;) {
        log_i(MODULE_MAIN, "tassk param：%d", (uint32_t)param);
        log_dumphex(MODULE_MAIN, "test data", test_data, sizeof(test_data));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void test_log_init(void *param)
{
    BaseType_t xReturn = pdPASS;

    log_init();
    log_register(MODULE_MAIN, "MAIN", LOG_LVL_INFO);
    log_register(MODULE_ISR, "ISR", LOG_LVL_ERR);

    drv_timer_control(&test_timer, DRV_TIMER_CMD_SET_CFG, DRV_TIMER_INS_TIMER1, 1000, DRV_TIMER_MODE_INTERRUPT | DRV_TIMER_MODE_STARTUP);
    drv_timer_control(&test_timer, DRV_TIMER_CMD_ATTACH, timer_callback, &test_tcb);
    drv_timer_init(&test_timer);

    xReturn = xTaskCreate(
        (TaskFunction_t)test_log_task, /* 任务函数 */
        (const char *)"log_task",      /* 任务名称 */
        (configSTACK_DEPTH_TYPE)256,   /* 任务堆栈大小 */
        (void *)NULL,                  /* 传递给任务函数的参数 */
        (UBaseType_t)19,               /* 任务优先级 */
        (TaskHandle_t *)&xTaskHandle); /* 任务句柄 */

    configASSERT(pdPASS == xReturn);
}
