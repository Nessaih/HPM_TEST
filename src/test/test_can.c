#include "tbox_log.h"
#include "can_if.h"
#include "api_rtos.h"
#include "tbox_pm_io.h"

#define CAN_TEST_INSTANCE 0

static TaskHandle_t  xTaskHandle  = NULL;
static TimerHandle_t xTimerHandle = NULL;

void can_send_task(void *param)
{
    int32_t    ret;
    can_msg_t  msg;
    static uint32_t seq = 0;

    memset(&msg, 0x00, sizeof(can_msg_t));
    msg.len = 8;

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if(!tbox_pm_io_acc_is_active())
            continue;

        memcpy(msg.data + 4, &seq, 4);

        msg.ins = 0;
        msg.id  = 0x9800E001;
        ret     = can_if_send(&msg);
        if (0 != ret) {
            // tbox_log_print("CAN1 send failed, ret = %d\r\n", ret);
        }

        msg.ins = 1;
        msg.id  = 0x9800E002;
        ret     = can_if_send(&msg);
        if (0 != ret) {
            // tbox_log_print("CAN2 send failed, ret = %d\r\n", ret);
        }

        seq++;
    }
}

static void can_timer_callback(TimerHandle_t xTimer)
{
    xTaskNotifyGive(xTaskHandle);
}

void test_can_init(void *param)
{

    BaseType_t xReturn = pdPASS;


    xReturn = xTaskCreate((TaskFunction_t)can_send_task, /* 任务函数 */
                          (const char *)"can_send",      /* 任务名称 */
                          (configSTACK_DEPTH_TYPE)256,   /* 任务堆栈大小 */
                          (void *)NULL,                  /* 传递给任务函数的参数 */
                          (UBaseType_t)16,               /* 任务优先级 */
                          (TaskHandle_t *)&xTaskHandle); /* 任务句柄 */

    configASSERT(pdPASS == xReturn);

    xTimerHandle = xTimerCreate("can_timer",         /* 定时器名称 */
                                pdMS_TO_TICKS(1000), /* 周期（ms） */
                                pdTRUE,              /* 自动重载（周期定时器） */
                                (void *)0,           /* 定时器ID（可自定义） */
                                can_timer_callback); /* 回调函数 */

    configASSERT(NULL != xTimerHandle);

    vTaskDelay(pdMS_TO_TICKS(5000)); /*确保任务已启动后再启动定时器*/
    xReturn = xTimerStart(xTimerHandle, 0);

    configASSERT(pdPASS == xReturn);
}