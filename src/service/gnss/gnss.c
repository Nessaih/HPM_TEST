#include "tbox_common.h"
#include "tbox_core.h"
#include "stimer.h"
#include "tbox_cfg_if.h"

#include "gnss.h"
#include "gnss_com.h"
#include "gnss_parse.h"
#include "gnss_control.h"
#include "gnss_shell.h"

static INT32 gnss_init(UINT8 seq);
static VOID  gnss_stop(VOID);
static VOID  gnss_start(VOID);
static VOID  gnss_exit(VOID);

//模块定义
TBOX_MODULE_FUN(GNSS, gnss_init, gnss_stop, gnss_start, NULL_PTR, gnss_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(GNSS, TBOX_TASK_PRIORITY_MID1, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, gnss_com_task);
TBOX_MESSSAGE(GNSS_SVR_TIMEOUT_EVENT, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_MESSAGE);
TBOX_MODULE_LOADER(GNSS)
{
    REGISTRY_TBOX_MESSAGE(GNSS_SVR_TIMEOUT_EVENT);
}


static TBOX_ID gnss_module_id;
static STIMER_ID gnss_timer;
SemaphoreHandle_t  gnss_mutex;

static VOID gnss_handle_message_event(const CHAR *name, TBOX_MSG_DATA *data)
{
    if (0 == strncmp(name, GNSS_SVR_TIMER_EVENT, strlen(GNSS_SVR_TIMER_EVENT)))
    {
    	//TODO
    	MODULE_LOG_I(GNSS, "gnss svr timeout.");
    }
	else if (0 == strncmp(name, TBOX_CFG_EVENT_VALUE_CHANGE, strlen(TBOX_CFG_EVENT_VALUE_CHANGE)))
	{
		gnss_control_handle_cfg_change(data);
	}
}

static VOID gnss_timer_callback(VOID)
{
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(GNSS, module_id);

    tbox_message_send(GNSS_SVR_TIMER_EVENT, module_id, module_id, NULL_PTR);
}

static VOID gnss_timer_creat(VOID)
{
	gnss_timer = stimer_create(STIMER_TYPE_PERIOD, gnss_timer_callback);
    if (STIMER_ID_INVALID == gnss_timer)
    {
        MODULE_LOG_E(GNSS, "create gnss timer failed");
    }
}

static VOID gnss_timer_start(VOID)
{
	INT32 ret = 0;
    ret = stimer_start(gnss_timer, GNSS_SVR_CYCLE_INTV);
    if (0 != ret)
    {
        MODULE_LOG_E(GNSS, "start gnss timer failed");
    }
}

static VOID gnss_timer_stop(VOID)
{
	INT32 ret = 0;
    ret = stimer_stop(gnss_timer);
    if (0 != ret)
    {
        MODULE_LOG_E(GNSS, "stop gnss timer failed");
    }
}

static INT32 gnss_init(UINT8 seq)
{
	INT32 ret = (INT32)TBOX_E_OK;
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
			gnss_mutex = NULL;
            GET_TBOX_MODULE_ID(GNSS, gnss_module_id);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            tbox_module_set_state(gnss_module_id, TBOX_MODULE_STATE_START);
			ret = tbox_message_add_handler(GNSS_SVR_TIMER_EVENT, gnss_module_id, gnss_handle_message_event);
			ret = tbox_message_subscribe(TBOX_CFG_EVENT_VALUE_CHANGE, gnss_module_id, gnss_handle_message_event);
			gnss_mutex = xSemaphoreCreateMutex();
			if(NULL == gnss_mutex)
			{
				MODULE_LOG_E(GNSS, "gnss creat mutex faied");
				vSemaphoreDelete(gnss_mutex);
                gnss_mutex = NULL;
			}
			gnss_timer_creat();
			gnss_timer_start();
            break;
            
        default:
            break;
    }

	ret |= gnss_parse_init(seq);
	ret |= gnss_com_init(seq);
	ret |= gnss_control_init(seq);
	ret |= gnss_shell_init(seq);

    MODULE_LOG_D(GNSS, "init seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID gnss_stop(VOID)
{
	tbox_module_set_state(gnss_module_id, TBOX_MODULE_STATE_STOP);
	gnss_control_sleep();
	gnss_com_sleep();
	gnss_parse_sleep();
	gnss_timer_stop();

}

static VOID  gnss_start(VOID)
{
	tbox_module_set_state(gnss_module_id, TBOX_MODULE_STATE_START);
	gnss_control_wake();
	gnss_com_wake();
	gnss_parse_wake();
	gnss_timer_start();
}

static VOID  gnss_exit(VOID)
{
	//TODO: deinit
}



