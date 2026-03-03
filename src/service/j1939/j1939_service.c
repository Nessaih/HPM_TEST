#include "tbox_common.h"
#include "tbox_core.h"
#include "can_if.h"
#include "api_rtos.h"
#include "j1939_if.h"

#define J1939_SVR_TASK_EVENT_TASK_START   0x01U
#define J1939_SVR_TASK_EVENT_TASK_STOP    0x02U
#define J1939_SVR_TASK_EVENT_CAN_ACTIVE   0x04U
#define J1939_SVR_TASK_EVENT_CAN_INACTIVE 0x08U
#define J1939_SVR_TASK_EVENT_CAN_DATAIN   0x10U
#define J1939_SVR_TASK_EVENT_ALL          0x1FU

static INT32 j1939_svr_init(UINT8 seq);
static VOID  j1939_svr_stop(VOID);
static VOID  j1939_svr_start(VOID);
static VOID  j1939_svr_exit(VOID);
static VOID  j1939_svr_task(VOID *param);
static INT32 j1939_svr_can_handler(CAN_EVENT event, UINT32 arg1, UINT32 arg2);

TBOX_MODULE_FUN(J1939, j1939_svr_init, j1939_svr_stop, j1939_svr_start, NULL_PTR, j1939_svr_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(J1939, TBOX_TASK_PRIORITY_MID1, LOG_LEVEL_INFO, TBOX_TASK_MEDIUM_STACK_SIZE, j1939_svr_task);
TBOX_MODULE_LOADER(J1939)
{
}

static TBOX_ID       j1939_svr_module_id;
static MODULE_HANDLE j1939_svr_module_handle;

static INT32 j1939_svr_init(UINT8 seq)
{
    INT32 ret = (INT32)TBOX_E_OK;

    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        GET_TBOX_MODULE_ID(J1939, j1939_svr_module_id);
        GET_TBOX_MODULE_HANDLE(J1939, j1939_svr_module_handle);
        break;
    case MODULE_INIT_SEQ_STORAGE:
        break;
    case MODULE_INIT_SEQ_MODULE:
        j1939_init();
        can_if_reg_cb(j1939_svr_can_handler);
        break;
    default:
        break;
    }

    return ret;
}

static VOID j1939_svr_stop(VOID)
{
    if (j1939_svr_module_handle)
        xTaskNotify(j1939_svr_module_handle, J1939_SVR_TASK_EVENT_TASK_STOP, eSetBits);
}

static VOID j1939_svr_start(VOID)
{
    if (j1939_svr_module_handle)
        xTaskNotify(j1939_svr_module_handle, J1939_SVR_TASK_EVENT_TASK_START, eSetBits);
}

static VOID j1939_svr_exit(VOID)
{
}

static VOID j1939_svr_task(VOID *param)
{
    UINT32        notify_value = 0;
    UINT32        event_mask   = J1939_SVR_TASK_EVENT_ALL;
    UINT32        wait_ticks   = pdTICKS_TO_MS(10);
    volatile BOOL is_active    = FALSE;

    for (;;)
    {
        if (pdPASS == xTaskNotifyWait(0, event_mask, &notify_value, wait_ticks))
        {
            if (notify_value & J1939_SVR_TASK_EVENT_TASK_START)
            {
                wait_ticks = pdTICKS_TO_MS(10);
                event_mask = J1939_SVR_TASK_EVENT_ALL;
                tbox_module_set_state(j1939_svr_module_id, TBOX_MODULE_STATE_START);
            }

            if (notify_value & J1939_SVR_TASK_EVENT_TASK_STOP)
            {
                wait_ticks = portMAX_DELAY;
                event_mask = J1939_SVR_TASK_EVENT_TASK_START;
                tbox_module_set_state(j1939_svr_module_id, TBOX_MODULE_STATE_STOP);
            }

            if (notify_value & J1939_SVR_TASK_EVENT_CAN_ACTIVE)
            {
                is_active = TRUE;
            }

            if (notify_value & J1939_SVR_TASK_EVENT_CAN_INACTIVE)
            {
                is_active = FALSE;
            }
        }

        if (is_active)
        {
            j1939_process();
        }
    }
}

static INT32 j1939_svr_can_handler(CAN_EVENT event, UINT32 arg1, UINT32 arg2)
{
    switch (event)
    {
    case CAN_EVENT_ACTIVE:
    {
        if (j1939_svr_module_handle)
            xTaskNotify(j1939_svr_module_handle, J1939_SVR_TASK_EVENT_CAN_ACTIVE, eSetBits);
        break;
    }

    case CAN_EVENT_INACTIVE:
    {
        if (j1939_svr_module_handle)
            xTaskNotify(j1939_svr_module_handle, J1939_SVR_TASK_EVENT_CAN_INACTIVE, eSetBits);
        break;
    }

    case CAN_EVENT_DATAIN:
    {
        can_msg_t *msg   = (can_msg_t *)arg1;
        UINT32     count = arg2;

        for (UINT32 i = 0; i < count; i++)
        {
            j1939_receive(msg[i].ins, &msg[i]);
        }
        break;
    }

    default:
        break;
    }

    return 0;
}
