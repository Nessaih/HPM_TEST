#include "tbox_common.h"
#include "tbox_core.h"
#include "stimer.h"

#include "fota.h"
#include "fota_com.h"
#include "fota_content.h"
#include "fota_shell.h"

static INT32 fota_init(UINT8 seq);
static VOID  fota_stop(VOID);
static VOID  fota_start(VOID);
static VOID  fota_exit(VOID);
static BOOL fota_can_be_stop(VOID);

//模块定义
TBOX_MODULE_FUN(FOTA, fota_init, fota_stop, fota_start, NULL_PTR, fota_exit, fota_can_be_stop);
TBOX_MODULE(FOTA, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, FALSE, FALSE);
TBOX_MESSSAGE(FOTA_TIMEOUT_EVENT, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_MESSAGE);
TBOX_MESSSAGE(FOTA_STATE_CHANGE, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_TOPIC);
TBOX_MODULE_LOADER(FOTA)
{
    REGISTRY_TBOX_MESSAGE(FOTA_TIMEOUT_EVENT);
    REGISTRY_TBOX_MESSAGE(FOTA_STATE_CHANGE);
}

static TBOX_ID 	 fota_module_id;
static STIMER_ID fota_timer;

static VOID fota_handle_message_event(const CHAR *name, TBOX_MSG_DATA *data)
{
	if(0 == strncmp(name, FOTA_EVENT_TIMEOUT, strlen(FOTA_EVENT_TIMEOUT)))
	{
		fota_com_timeout_proc();
	}
	else if (0 == strncmp(name, FOTA_EVENT_STATE_CHANGE, strlen(FOTA_EVENT_STATE_CHANGE)))
	{
		fota_shell_value_changed_handle(data);
	}
}

static VOID fota_timer_callback(VOID)
{
	TBOX_ID module_id;
	GET_TBOX_MODULE_ID(FOTA, module_id);

	tbox_message_send(FOTA_EVENT_TIMEOUT, module_id, module_id, NULL_PTR);
}

static VOID fota_timer_creat(VOID)
{
	fota_timer = stimer_create(STIMER_TYPE_PERIOD, fota_timer_callback);
	if (STIMER_ID_INVALID == fota_timer)
	{
		MODULE_LOG_E(FOTA, "create fota tbox timer failed");
	}
}

static VOID fota_timer_start(VOID)
{
	INT32 ret = 0;
	ret = stimer_start(fota_timer, FOTA_CYCLE_INTV);
	if (0 != ret)
	{
		MODULE_LOG_E(FOTA, "start fota timer start failed");
	}
}

static VOID fota_timer_stop(VOID)
{
	INT32 ret = 0;
	ret = stimer_stop(fota_timer);
	if (0 != ret)
	{
		MODULE_LOG_E(HPM, "stop fota timer failed");
	}
}

static INT32 fota_init(UINT8 seq)
{
	INT32 ret = (INT32)TBOX_E_OK;
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            GET_TBOX_MODULE_ID(FOTA, fota_module_id);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
			ret = tbox_message_add_handler(FOTA_EVENT_TIMEOUT, fota_module_id, fota_handle_message_event);
			ret = tbox_message_subscribe(FOTA_EVENT_STATE_CHANGE, fota_module_id, fota_handle_message_event);
            tbox_module_set_state(fota_module_id, TBOX_MODULE_STATE_START);
			fota_timer_creat();
			fota_timer_start();
            break;
            
        default:
            break;
    }

	fota_com_init(seq);
	fota_shell_init(seq);
	
    MODULE_LOG_D(FOTA, "init seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID fota_stop(VOID)
{
	tbox_module_set_state(fota_module_id, TBOX_MODULE_STATE_STOP);
	fota_timer_stop();
}

static VOID  fota_start(VOID)
{
	tbox_module_set_state(fota_module_id, TBOX_MODULE_STATE_START);
	fota_timer_start();
}

static VOID  fota_exit(VOID)
{
	//TODO: deinit
}

static BOOL fota_can_be_stop(VOID)
{
	// True: 允许休眠, False: 不允许休眠
	if(FOTA_COM_IDLE != fota_com_get_state())
	{
		return FALSE;
	}

	return TRUE;
}


