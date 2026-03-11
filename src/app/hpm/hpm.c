#include "tbox_common.h"
#include "tbox_core.h"
#include "stimer.h"
#include "tbox_cfg_if.h"
#include "fota_if.h"

#include "hpm_content.h"
#include "hpm.h"
#include "hpm_net.h"
#include "hpm_mgr.h"
#include "hpm_cfg.h"
#include "hpm_flash.h"
#include "hpm_data.h"
#include "hpm_can.h"
#include "hpm_data_recv.h"
#include "hpm_socket.h"
#include "hpm_session.h"
#include "hpm_param_fetch.h"
#include "hpm_dev.h"
#include "hpm_ota_tbox.h"
#include "hpm_status.h"
#include "hpm_shell.h"

static INT32 hpm_init(UINT8 seq);
static VOID hpm_stop(VOID);
static VOID hpm_start(VOID);
static VOID hpm_exit(VOID);
static BOOL hpm_can_be_stop(VOID);

// 模块定义
TBOX_MODULE_FUN(HPM, hpm_init, hpm_stop, hpm_start, NULL_PTR, hpm_exit, hpm_can_be_stop);
TBOX_MODULE(HPM, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_ERROR, TBOX_TASK_LARGE_STACK_SIZE, FALSE, FALSE);
TBOX_MESSSAGE(HPM_APP_TIMEOUT_EVENT, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_MESSAGE);
TBOX_MODULE_LOADER(HPM)
{
    REGISTRY_TBOX_MESSAGE(HPM_APP_TIMEOUT_EVENT);
}

static TBOX_ID hpm_module_id;
static STIMER_ID hpm_timer;
SemaphoreHandle_t hpm_mutex;
UINT32 hpm_timer_ticks;

static VOID hpm_handle_message_event(const CHAR *name, TBOX_MSG_DATA *data)
{
    if (0 == strncmp(name, HPM_APP_TIMER_EVENT, strlen(HPM_APP_TIMER_EVENT)))
    {
        hpm_timer_ticks++;
        if (HPM_APP_CYCLE_INTV * hpm_timer_ticks >= 1000U)
        {
            hpm_timer_ticks = 0;
            hpm_mgr_process();
            hpm_socket_process();
            hpm_cfg_process();
            hpm_control_process();
            hpm_param_fetch_process();
            hpm_status_process();
            hpm_flash_period();
        }
        hpm_dev_process();
        hpm_session_process();
    }
    else if (0 == strncmp(name, TBOX_CFG_EVENT_VALUE_CHANGE, strlen(TBOX_CFG_EVENT_VALUE_CHANGE)))
    {
        MODULE_LOG_I(HPM, "tbox cfg value change\r\n");
        hpm_socket_handle_msg(data);
    }
    else if (0 == strncmp(name, FOTA_EVENT_STATE_CHANGE, strlen(FOTA_EVENT_STATE_CHANGE)))
    {
        MODULE_LOG_I(HPM, "fota state value change\r\n");
        hpm_ota_tbox_handle_msg(data);
    }
}

static VOID hpm_timer_callback(VOID)
{
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(HPM, module_id);

    tbox_message_send(HPM_APP_TIMER_EVENT, module_id, module_id, NULL_PTR);
}

static VOID hpm_timer_creat(VOID)
{
    hpm_timer = stimer_create(STIMER_TYPE_PERIOD, hpm_timer_callback);
    if (STIMER_ID_INVALID == hpm_timer)
    {
        MODULE_LOG_E(HPM, "create hpm tbox timer failed");
    }
}

static VOID hpm_timer_start(VOID)
{
    INT32 ret = 0;
    ret = stimer_start(hpm_timer, HPM_APP_CYCLE_INTV);
    if (0 != ret)
    {
        MODULE_LOG_E(HPM, "start hpm tbox timer failed");
    }
}

static VOID hpm_timer_stop(VOID)
{
    INT32 ret = 0;
    ret = stimer_stop(hpm_timer);
    if (0 != ret)
    {
        MODULE_LOG_E(HPM, "stop hpm tbox timer failed");
    }
}

static INT32 hpm_init(UINT8 seq)
{
    INT32 ret = (INT32)TBOX_E_OK;
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        hpm_mutex = NULL;
        hpm_timer_ticks = 0;
        GET_TBOX_MODULE_ID(HPM, hpm_module_id);
        ret = tbox_message_add_handler(HPM_APP_TIMER_EVENT, hpm_module_id, hpm_handle_message_event);
        ret = tbox_message_subscribe(TBOX_CFG_EVENT_VALUE_CHANGE, hpm_module_id, hpm_handle_message_event);
        ret = tbox_message_subscribe(FOTA_EVENT_STATE_CHANGE, hpm_module_id, hpm_handle_message_event);
        hpm_mutex = xSemaphoreCreateMutex();
        if (NULL == hpm_mutex)
        {
            MODULE_LOG_E(HPM, "hpm creat mutex faied");
            vSemaphoreDelete(hpm_mutex);
            hpm_mutex = NULL;
        }
        break;

    case MODULE_INIT_SEQ_STORAGE:
        break;

    case MODULE_INIT_SEQ_MODULE:
        hpm_timer_creat();
        hpm_timer_start();
        tbox_module_set_state(hpm_module_id, TBOX_MODULE_STATE_START);
        hpm_flash_init();
        break;

    default:
        break;
    }

    hpm_cfg_init(seq);
    hpm_mgr_init(seq);
    hpm_socket_init(seq);
    hpm_session_init(seq);
    hpm_net_init(seq);
    hpm_data_recv_init(seq);
    hpm_dev_init(seq);
    hpm_can_init(seq);
    hpm_shell_init(seq);
    hpm_ota_tbox_init(seq);
    hpm_param_fetch_init(seq);
    hpm_status_init(seq);
    MODULE_LOG_D(HPM, "init seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID hpm_stop(VOID)
{
    hpm_cfg_sleep();
    hpm_session_sleep();
    hpm_dev_sleep();
    hpm_timer_stop();
    hpm_ota_tbox_sleep();
    hpm_param_fetch_sleep();
    hpm_status_sleep();
    hpm_flash_sleep();
    tbox_module_set_state(hpm_module_id, TBOX_MODULE_STATE_STOP);
}

static VOID hpm_start(VOID)
{
    hpm_timer_ticks = 0;
    hpm_mgr_wakeup();
    hpm_cfg_wake();
    hpm_session_wake();
    hpm_dev_wakeup();
    hpm_param_fetch_wakeup();
    hpm_status_wakeup();
    hpm_timer_start();
    hpm_ota_tbox_wake();
    tbox_module_set_state(hpm_module_id, TBOX_MODULE_STATE_START);
}

static VOID hpm_exit(VOID)
{
    // TODO: deinit
}

static BOOL hpm_can_be_stop(VOID)
{
    // True: 允许休眠, False: 不允许休眠
    return hpm_mgr_allow_sleep();
}

INT32 hpm_get_moudle_id(VOID)
{
    return hpm_module_id;
}
