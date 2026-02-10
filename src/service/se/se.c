#include "tbox_common.h"
#include "tbox_core.h"

#include "se.h"
#include "se_shell.h"
#include "se_mgr.h"

static INT32 se_init(UINT8 seq);
static VOID  se_stop(VOID);
static VOID  se_start(VOID);
static VOID  se_exit(VOID);

//模块定义
TBOX_MODULE_FUN(SE, se_init, se_stop, se_start, NULL_PTR, se_exit, NULL_PTR);
TBOX_MODULE(SE, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_DEBUG, TBOX_TASK_MEDIUM_STACK_SIZE, FALSE, FALSE);
TBOX_MESSSAGE(SE_TBOX_TIMEOUT_EVENT, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_MESSAGE);
TBOX_MODULE_LOADER(SE)
{
    REGISTRY_TBOX_MESSAGE(SE_TBOX_TIMEOUT_EVENT);
}


static TBOX_ID se_module_id;

static VOID se_handle_message_event(const CHAR *name, TBOX_MSG_DATA *data)
{
	if(0 == strncmp(name, SE_TBOX_TIMER_EVENT, strlen(SE_TBOX_TIMER_EVENT)))
	{
		se_mgr_timeout();
	}
}


static INT32 se_init(UINT8 seq)
{
	INT32 ret = (INT32)TBOX_E_OK;
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            GET_TBOX_MODULE_ID(SE, se_module_id);
			ret = tbox_message_add_handler(SE_TBOX_TIMER_EVENT, se_module_id, se_handle_message_event);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            tbox_module_set_state(se_module_id, TBOX_MODULE_STATE_START);
            break;
            
        default:
            break;
    }

	se_mgr_init(seq);
	se_shell_init(seq);
    MODULE_LOG_D(SE, "init seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID se_stop(VOID)
{
	se_mgr_sleep();
	tbox_module_set_state(se_module_id, TBOX_MODULE_STATE_STOP);
}

static VOID  se_start(VOID)
{
	se_mgr_wake();
	tbox_module_set_state(se_module_id, TBOX_MODULE_STATE_START);
}

static VOID  se_exit(VOID)
{
	//TODO: deinit
}



