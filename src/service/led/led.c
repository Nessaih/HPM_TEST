#include "tbox_common.h"
#include "tbox_core.h"

#include "led.h"
#include "led_shell.h"
#include "led_tbox.h"

static INT32 led_init(UINT8 seq);
static VOID  led_stop(VOID);
static VOID  led_start(VOID);
static VOID  led_exit(VOID);

//模块定义
TBOX_MODULE_FUN(LED, led_init, led_stop, led_start, NULL_PTR, led_exit, NULL_PTR);
TBOX_MODULE(LED, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, FALSE, FALSE);
TBOX_MESSSAGE(LED_TBOX_TIMEOUT_EVENT, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_MESSAGE);
TBOX_MODULE_LOADER(LED)
{
    REGISTRY_TBOX_MESSAGE(LED_TBOX_TIMEOUT_EVENT);
}


static TBOX_ID led_module_id;

static VOID led_handle_message_event(const CHAR *name, TBOX_MSG_DATA *data)
{
	if(0 == strncmp(name, LED_TBOX_TIMER_EVENT, strlen(LED_TBOX_TIMER_EVENT)))
	{
		led_tbox_timeout();
	}
}


static INT32 led_init(UINT8 seq)
{
	INT32 ret = (INT32)TBOX_E_OK;
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            GET_TBOX_MODULE_ID(LED, led_module_id);
			ret = tbox_message_add_handler(LED_TBOX_TIMER_EVENT, led_module_id, led_handle_message_event);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            led_tbox_init();
			led_shell_init();
            tbox_module_set_state(led_module_id, TBOX_MODULE_STATE_START);
            break;
            
        default:
            break;
    }

    MODULE_LOG_D(LED, "init seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID led_stop(VOID)
{
	led_tbox_sleep();
	tbox_module_set_state(led_module_id, TBOX_MODULE_STATE_STOP);
}

static VOID  led_start(VOID)
{
	led_tbox_wake();
	tbox_module_set_state(led_module_id, TBOX_MODULE_STATE_START);
}

static VOID  led_exit(VOID)
{
	//TODO: deinit
}


