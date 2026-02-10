#include "tbox_core.h"
#include "api_rtos.h"
#include "tbox_memory_inner.h"
#include "tbox_message_inner.h"
#include "tbox_module_inner.h"
#include "tbox_log_inner.h"
#include "tbox_task.h"
#include "tbox_supervish_inner.h"

static volatile UINT8 tbox_core_state = TBOX_CORE_STATE_NOINIT;

static INT32 tbox_core_module_init(UINT8 seq);

TBOX_MODULE_FUN(ICORE, tbox_core_module_init, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR);
TBOX_MODULE(ICORE, TBOX_TASK_PRIORITY_MID, LOG_LEVEL_ERROR, 0U, TRUE, FALSE);
TBOX_MESSSAGE(core_memoryabnormal_notify, TBOX_MSG_PRIORITY_HIGH, TBOX_MSG_TYPE_TOPIC);
TBOX_MESSSAGE(core_taskabnormal_notify, TBOX_MSG_PRIORITY_HIGH, TBOX_MSG_TYPE_TOPIC);
TBOX_MESSSAGE(core_taskupdate_notify, TBOX_MSG_PRIORITY_HIGH, TBOX_MSG_TYPE_TOPIC);
TBOX_MODULE_LOADER(ICORE)
{
    REGISTRY_TBOX_MESSAGE(core_memoryabnormal_notify);
    REGISTRY_TBOX_MESSAGE(core_taskabnormal_notify); 
    REGISTRY_TBOX_MESSAGE(core_taskupdate_notify);           
}

INT32 tbox_core_init(VOID)
{
    INT32 ret = tbox_memory_init();
    ret = TBOX_COND_CALL(ret, (INT32)TBOX_E_OK, tbox_log_init());
    ret = TBOX_COND_CALL(ret, (INT32)TBOX_E_OK, tbox_task_init());
    ret = TBOX_COND_CALL(ret, (INT32)TBOX_E_OK, tbox_message_init());
    ret = TBOX_COND_CALL(ret, (INT32)TBOX_E_OK, tbox_module_init());
    ret = TBOX_COND_CALL(ret, (INT32)TBOX_E_OK, tbox_supervise_init());
    if(TBOX_E_OK == ret)
    {
        LOAD_TBOX_MODULE(ICORE);
    }

    tbox_core_state = TBOX_CORE_STATE_STOP;

    return ret;
}

VOID tbox_core_deinit(VOID)
{
    tbox_module_exit();    
    tbox_task_deinit();
    tbox_supervise_deinit();
    tbox_message_deinit();
    tbox_log_deinit();
    tbox_memory_deinit();

    tbox_core_state = TBOX_CORE_STATE_NOINIT;
}

INT32 tbox_core_start(BOOL start_module)
{
    INT32 ret = (INT32)TBOX_E_OK;
    tbox_log_start();
    ret = TBOX_COND_CALL(ret, (INT32)TBOX_E_OK, tbox_task_start());   
    ret = TBOX_COND_CALL(ret, (INT32)TBOX_E_OK, tbox_supervise_start());
    if((INT32)TBOX_E_OK == ret && 
        FALSE == tbox_allmodule_istarted() &&
        TRUE == start_module)
    {
      tbox_module_start();
    }
    
    tbox_core_state = TBOX_CORE_STATE_START;

    MODULE_LOG_I(ICORE, "core start finish, ret:%d", ret);

    return ret;
}

VOID tbox_core_stop(BOOL stop_module)
{
    if(TRUE == stop_module && 
       FALSE == tbox_allmodule_isstoped())
    {
        tbox_module_stop();
    }
    tbox_task_stop();
    tbox_supervise_stop();
    tbox_message_reset();
    tbox_log_stop();
    tbox_core_state = TBOX_CORE_STATE_STOP;   
}

UINT8 tbox_core_get_state(VOID)
{
    return tbox_core_state;
}

VOID tbox_core_main(TBOX_CORE_LOAD_FUN load_module_fun)
{
    INT32 ret = tbox_core_init();
    if((INT32)TBOX_E_OK != ret)
    {
        return;
    }
    
    tbox_log_start();

    if(NULL_PTR != load_module_fun)
    {
        (*load_module_fun)();
    }
    
    tbox_module_init_all_regmodule();

    ret = tbox_core_start(TRUE);
    if((INT32)TBOX_E_OK != ret)
    {
        tbox_core_stop(TRUE);
        tbox_core_deinit();
        return;
    }
    
    //vTaskStartScheduler();
    
    //MODULE_LOG_F(ICORE, "the scheduler is exit");

   // tbox_core_stop(TRUE);
   // tbox_core_deinit();
}

static INT32 tbox_core_module_init(UINT8 seq)
{
    MODULE_LOG_D(ICORE, "core init seq:%d", seq);
    return TBOX_E_OK;
}