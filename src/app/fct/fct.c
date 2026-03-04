#include "tbox_common.h"
#include "tbox_core.h"
#include "stimer.h"

#include "fct.h"
#include "fct_shell.h"
#include "fct_cmd.h"

#define FCT_APP_CYCLE_INTV			(1*1000)

static INT32 fct_init(UINT8 seq);
static VOID  fct_stop(VOID);
static VOID  fct_start(VOID);
static VOID  fct_exit(VOID);

//模块定义
TBOX_MODULE_FUN(FCT, fct_init, fct_stop, fct_start, NULL_PTR, fct_exit, NULL_PTR);
TBOX_MODULE(FCT, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, FALSE, FALSE);
TBOX_MESSSAGE(FCT_APP_TIMEOUT_EVENT, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_MESSAGE);
TBOX_MODULE_LOADER(FCT)
{
    REGISTRY_TBOX_MESSAGE(FCT_APP_TIMEOUT_EVENT);
}


static TBOX_ID fct_module_id;
static STIMER_ID fct_timer;

static VOID fct_handle_message_event(const CHAR *name, TBOX_MSG_DATA *data)
{
	if(0 == strncmp(name, FCT_APP_TIMER_EVENT, strlen(FCT_APP_TIMER_EVENT)))
	{
		fct_cmd_timeout();
	}
}

static VOID fct_timer_callback(VOID)
{
	TBOX_ID module_id;
    GET_TBOX_MODULE_ID(FCT, module_id);

    tbox_message_send(FCT_APP_TIMER_EVENT, module_id, module_id, NULL_PTR);
}

static VOID fct_timer_creat(VOID)
{
	fct_timer = stimer_create(STIMER_TYPE_PERIOD, fct_timer_callback);
	if(STIMER_ID_INVALID == fct_timer)
	{
		MODULE_LOG_E(FCT, "create fct tbox timer failed");
	}
}

VOID fct_timer_start(VOID)
{
	INT32 ret = 0;
	ret = stimer_start(fct_timer, FCT_APP_CYCLE_INTV);
	if(0 != ret)
	{
		MODULE_LOG_E(FCT, "start fct tbox timer failed");
	}
}

VOID fct_timer_stop(VOID)
{
	INT32 ret = 0;
	ret = stimer_stop(fct_timer);
	if(0 != ret)
	{
		MODULE_LOG_E(FCT, "stop fct tbox timer failed");
	}
}

static INT32 fct_init(UINT8 seq)
{
	INT32 ret = (INT32)TBOX_E_OK;
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            GET_TBOX_MODULE_ID(FCT, fct_module_id);
			ret = tbox_message_add_handler(FCT_APP_TIMER_EVENT, fct_module_id, fct_handle_message_event);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:            
			fct_timer_creat();
            tbox_module_set_state(fct_module_id, TBOX_MODULE_STATE_START);
            break;
            
        default:
            break;
    }

	fct_cmd_init(seq);
	fct_shell_init(seq);
	
    MODULE_LOG_D(FCT, "init seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID fct_stop(VOID)
{
	fct_timer_stop();
	tbox_module_set_state(fct_module_id, TBOX_MODULE_STATE_STOP);
}

static VOID  fct_start(VOID)
{
	tbox_module_set_state(fct_module_id, TBOX_MODULE_STATE_START);
}

static VOID  fct_exit(VOID)
{
	//TODO: deinit
}



