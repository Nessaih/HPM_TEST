#include "api_rtos.h"
#include "drv_can.h"
#include "log.h"


#define CAN_TEST_INSTANCE 0

static TaskHandle_t  xTaskHandle  = NULL;
static TimerHandle_t xTimerHandle = NULL;

void can_send_task(void *param)
{
    can_msg_t msg;
    int32_t   ret;

    msg.ins     = CAN_TEST_INSTANCE;
    msg.id      = 0x123;
    msg.len     = 8;
    msg.data[0] = 0x11;
    msg.data[1] = 0x22;
    msg.data[2] = 0x33;
    msg.data[3] = 0x44;
    msg.data[4] = 0x55;
    msg.data[5] = 0x66;
    msg.data[6] = 0x77;

    for(;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ret = drv_can_send(&msg);
        if (0 != ret) {
            log_print("CAN send failed, ret = %d\r\n", ret);
        }
    }
}

void can_recv_task(void *param)
{
}

static void can_timer_callback(TimerHandle_t xTimer)
{
    xTaskNotifyGive(xTaskHandle);
}

void can_recv_callback(uint8_t ins, uint32_t event, void *para)
{
    can_msg_t *msg = para;

    if (DRV_CAN_EVENT_RX_DONE == event) {
        /* For debugging purposes only. Do not recommend printing logs during a system interruption for the release version. */
        log_print("CAN recv, id = 0x%x, len = %d, data = %02x %02x %02x\r\n", msg->id, msg->len, msg->data[0], msg->data[1], msg->data[2]);
    }
}

void test_can_init(void *param)
{

    BaseType_t xReturn = pdPASS;
    int32_t    ret;

    drv_can_register(CAN_TEST_INSTANCE, can_recv_callback);

    ret = drv_can_init(CAN_TEST_INSTANCE, 500, DRV_CAN_MODE_NORMAL);
    if (0 != ret) {
        log_print("CAN init failed, ret = %d\r\n", ret);
    }

    log_print("can struct size = %d\r\n", sizeof(can_msg_t));

    xReturn = xTaskCreate(
        (TaskFunction_t)can_send_task, /* 任务函数 */
        (const char *)"can_send",      /* 任务名称 */
        (configSTACK_DEPTH_TYPE)256,   /* 任务堆栈大小 */
        (void *)NULL,                  /* 传递给任务函数的参数 */
        (UBaseType_t)19,               /* 任务优先级 */
        (TaskHandle_t *)&xTaskHandle); /* 任务句柄 */

    configASSERT(pdPASS == xReturn);

    xTimerHandle = xTimerCreate(
        "can_timer",         /* 定时器名称 */
        pdMS_TO_TICKS(100),  /* 周期（ms） */
        pdTRUE,              /* 自动重载（周期定时器） */
        (void *)0,           /* 定时器ID（可自定义） */
        can_timer_callback); /* 回调函数 */

    configASSERT(NULL != xTimerHandle);

    vTaskDelay(pdMS_TO_TICKS(10)); /*确保任务已启动后再启动定时器*/
    xReturn = xTimerStart(xTimerHandle, 0);

    configASSERT(pdPASS == xReturn);
}