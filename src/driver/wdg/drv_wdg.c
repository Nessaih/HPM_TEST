#include <stdbool.h>
#include "Wdg_Hal.h"
#include "api_rtos.h"
#include "drv_wdg.h"
#include "tbox_config.h"

const Wdg_HalConfigType wdg_cfg = {
    .ClockSource  = WDG_CLOCK_LSI,
    .WindowValue  = 0,
    .TimeoutValue = 5000,
    .Config       = (WDG_CONFIG_PRESCALER_EN | WDG_CONFIG_DBG_EN | WDG_CONFIG_WIN_EN),
};

static TaskHandle_t xTaskHandle = NULL;

static void wdg_feed_task(void *arg)
{
    for (;;) {
        vTaskDelay(pdTICKS_TO_MS(4000));
        Wdg_Hal_Feed();
    }
}

int32_t drv_wdg_init(void)
{
    BaseType_t xReturn = pdPASS;

    Wdg_Hal_Init(&wdg_cfg);

    xReturn = xTaskCreate(wdg_feed_task, "Wdg Svc", 256, NULL, TBOX_TASK_PRIORITY_LOW, &xTaskHandle);
    configASSERT(pdPASS == xReturn);

    return 0;
}

int32_t drv_wdg_deinit(void)
{
    if (NULL != xTaskHandle) {
        Wdg_Hal_DeInit();
        vTaskDelete(xTaskHandle);
    }
    return 0;
}

int32_t drv_wdg_wake(void)
{
    if (NULL != xTaskHandle) {
        Wdg_Hal_Init(&wdg_cfg);
        vTaskResume(xTaskHandle);
    }
    return 0;
}

int32_t drv_wdg_sleep(void)
{
    if (NULL != xTaskHandle) {
        vTaskSuspend(xTaskHandle);
        Wdg_Hal_DeInit();
    }
    return 0;
}

int32_t drv_wdg_feed(void)
{
    if (NULL != xTaskHandle) {
        Wdg_Hal_Feed();
    }
    return 0;
}

void drv_wdg_reset(void)
{
    for (;;) {
    }
}