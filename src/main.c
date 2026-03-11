
#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "driver.h"
#include "version.h"
#include "stimer.h"
#include "test.h"

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
    //LOAD_TBOX_MODULE(SE);
    LOAD_TBOX_MODULE(BATTERY);
    LOAD_TBOX_MODULE(TIME);
    LOAD_TBOX_MODULE(CAN);
    LOAD_TBOX_MODULE(CAN_TX);
    LOAD_TBOX_MODULE(UDS);
    LOAD_TBOX_MODULE(J1939);
    LOAD_TBOX_MODULE(FOTA);
    /*load app module*/     
    LOAD_TBOX_MODULE(FCT);
    LOAD_TBOX_MODULE(HPM);
}

VOID tbox_main_task(VOID *param)
{
    UNUSED(param);

    tbox_core_main(tbox_load_all_module);
    test_init();
    vTaskDelete(NULL_PTR);
}

void main(void)
{
    driver_init();
    version_show();
    BaseType_t ret = xTaskCreate(tbox_main_task, 
                            "tbox_main_task", 
                            4096U/sizeof(StackType_t), 
                            NULL, 
                            configMAX_PRIORITIES-1, 
                            NULL);
    if(pdPASS != ret)
    {
        CHAR print_buff[64] = "\0";
        strncpy(print_buff, "tbox main task create fail, ret:%d", ret);
        tbox_log_raw_output(print_buff, strlen(print_buff));
        return;        
    }

    vTaskStartScheduler();
    for (;;);
}