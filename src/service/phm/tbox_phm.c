#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "stimer.h"
#include "tbox_phm_module.h"
#include "tbox_phm_4g.h"

#define TBOX_PHM_TIMER_EVENT  "PHM_TIMER_EVENT"

static STIMER_ID tbox_phm_timer_id;
static INT32 tbox_phm_init(UINT8 seq);
static VOID tbox_phm_start(VOID);
static VOID tbox_phm_stop(VOID);
static VOID tbox_phm_exit(VOID);
static VOID tbox_phm_timeout_callback(VOID);
static VOID tbox_phm_handle_timer_event(const CHAR *name, TBOX_MSG_DATA *data);

TBOX_MODULE_FUN(TBOXPHM, tbox_phm_init, tbox_phm_stop, tbox_phm_start, NULL_PTR, tbox_phm_exit, NULL_PTR);
TBOX_MODULE(TBOXPHM, TBOX_TASK_PRIORITY_LOW, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, FALSE, FALSE);
TBOX_MESSSAGE(PHM_TIMER_EVENT, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_MESSAGE);
TBOX_MODULE_LOADER(TBOXPHM)
{
    REGISTRY_TBOX_MESSAGE(PHM_TIMER_EVENT);
}

static INT32 tbox_phm_init(UINT8 seq)
{
    INT32 ret = (INT32)TBOX_E_OK;
    
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            {
                TBOX_ID module_id;
                GET_TBOX_MODULE_ID(TBOXPHM, module_id);
                ret = tbox_message_add_handler(TBOX_PHM_TIMER_EVENT, module_id, tbox_phm_handle_timer_event);
            }
            break;
        
        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            {
                tbox_phm_timer_id = stimer_create(STIMER_TYPE_PERIOD, tbox_phm_timeout_callback);
                if(STIMER_ID_INVALID == tbox_phm_timer_id)
                {
                    MODULE_LOG_E(TBOXPHM, "create phm timer failed");
                    ret = (INT32)TBOX_E_FAILED_CREATE;
                }
                else
                {
                    ret = TBOX_COND_CALL(ret, (INT32)TBOX_E_OK, tbox_phm_module_init());
                    ret = TBOX_COND_CALL(ret, (INT32)TBOX_E_OK, tbox_phm_4g_init());
                }
            }
            break;

        default:
            break;
    }
    
    MODULE_LOG_D(TBOXPHM, "tbox phm init, seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID tbox_phm_start(VOID)
{
    tbox_phm_module_start();
    tbox_phm_4g_start();

    if(STIMER_ID_INVALID != tbox_phm_timer_id)
    {
        stimer_start(tbox_phm_timer_id, 1000U);
    }
    
    MODULE_LOG_D(TBOXPHM, "tbox phm start");
}

static VOID tbox_phm_stop(VOID)
{
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOXPHM, module_id);

    tbox_phm_module_stop();
    tbox_phm_4g_stop();

    if(STIMER_ID_INVALID != tbox_phm_timer_id)
    {
        stimer_stop(tbox_phm_timer_id);
    }

    tbox_module_set_state(module_id, TBOX_MODULE_STATE_STOP);

    MODULE_LOG_D(TBOXPHM, "tbox phm stop");
}

static VOID tbox_phm_exit(VOID)
{
    /*NOTHING TO DO*/
    MODULE_LOG_D(TBOXPHM, "tbox phm exit");
}

static VOID tbox_phm_timeout_callback(VOID)
{
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOXPHM, module_id);

    tbox_message_send(TBOX_PHM_TIMER_EVENT, module_id, module_id, NULL_PTR); 
}

static VOID tbox_phm_handle_timer_event(const CHAR *name, TBOX_MSG_DATA *data)
{
    UNUSED(data);

    if(0U != strncmp(name, TBOX_PHM_TIMER_EVENT, strlen(TBOX_PHM_TIMER_EVENT)))
    {
         return;
    }

    tbox_phm_module_period();
    tbox_phm_4g_period();
}