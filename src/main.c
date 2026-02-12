
#include "tbox_cfg_if.h"
#include "tbox_common.h"
#include "tbox_core.h"
#include "driver.h"
#include "stimer.h"
#include "version.h"

void fault_test_by_unalign(void)
{
    volatile int *SCB_CCR = (volatile int *)0xE000ED14; // SCB->CCR
    volatile int *p;
    volatile int  value;

    *SCB_CCR |= (1 << 3); /* bit3: UNALIGN_TRP. */

    p     = (int *)0x00;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int)p, value);

    p     = (int *)0x04;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int)p, value);

    p     = (int *)0x03;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int)p, value);
}

void fault_test_by_div0(void)
{
    volatile int *SCB_CCR = (volatile int *)0xE000ED14; // SCB->CCR
    int           x, y, z;

    *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */

    x = 10;
    y = 0;
    z = x / y;
    printf("z:%d\n", z);
}

VOID tbox_load_all_module(VOID)
{
    /*load service module*/
    LOAD_TBOX_MODULE(TBOXSVR);
    LOAD_TBOX_MODULE(TBOXADSP);
    LOAD_TBOX_MODULE(TBOXSHELL);
    LOAD_TBOX_MODULE(TBOXCFG);
    LOAD_TBOX_MODULE(STIMER);
    LOAD_TBOX_MODULE(TBOXMODE);
    LOAD_TBOX_MODULE(TBOXPM);
    LOAD_TBOX_MODULE(TBOXPHM);
    LOAD_TBOX_MODULE(LED);
    LOAD_TBOX_MODULE(TBOX4G);
    LOAD_TBOX_MODULE(GNSS);
    LOAD_TBOX_MODULE(ANALOG);
    // LOAD_TBOX_MODULE(SE);
    LOAD_TBOX_MODULE(BATTERY);
    LOAD_TBOX_MODULE(TIME);
    LOAD_TBOX_MODULE(CAN);

    /*load app module*/
    LOAD_TBOX_MODULE(FCT);
    LOAD_TBOX_MODULE(HPM);

    fault_test_by_div0();
    fault_test_by_unalign();

}

VOID tbox_main_task(VOID *param)
{
    UNUSED(param);

    tbox_core_main(tbox_load_all_module);
    vTaskDelete(NULL_PTR);
}

void main(void)
{
    driver_init();
    version_show();
    BaseType_t ret = xTaskCreate(tbox_main_task, "tbox_main_task", 4096U / sizeof(StackType_t), NULL, configMAX_PRIORITIES - 1, NULL);
    if (pdPASS != ret)
    {
        CHAR print_buff[64] = "\0";
        strncpy(print_buff, "tbox main task create fail, ret:%d", ret);
        tbox_log_raw_output(print_buff, strlen(print_buff));
        return;
    }

    vTaskStartScheduler();
    for (;;)
        ;
}