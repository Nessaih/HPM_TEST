#include "4g_depend_header.h"
#include "stimer.h"
#include "4g_content.h"
#include "4g_if.h"
#include "4g_dev.h"
#include "4g_mgr.h"
#include "4g_power.h"
#include "4g_ftp.h"
#include "4g_at_startup.h"

#define TBOX_4G_EVENT_START   (1U << 0U)
#define TBOX_4G_EVENT_STOP    (1U << 1U)
#define TBOX_4G_EVENT_EXIT    (1U << 2U)
#define TBOX_4G_EVENT_PERIOD  (1U << 3U)
#define TBOX_4G_EVENT_ALL     (TBOX_4G_EVENT_START | TBOX_4G_EVENT_STOP | TBOX_4G_EVENT_EXIT | TBOX_4G_EVENT_PERIOD)

static INT32 tbox_4g_init(UINT8 seq);
static VOID tbox_4g_start(VOID);
static VOID tbox_4g_stop(VOID);
static VOID tbox_4g_exit(VOID);
static VOID tbox_4g_task(VOID *param);
static BOOL tbox_4g_can_be_stop(VOID);
static VOID tbox_4g_timer_callback(VOID);
static VOID tbox_4g_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data);
static VOID tbox_4g_handle_update_task(const CHAR *name, TBOX_MSG_DATA *data);
static VOID tbox_4g_check_state(VOID);

static UINT8 tbox_4g_state;
static STIMER_ID  tbox_4g_timer_id;
static TaskHandle_t tbox_4g_task_handle = NULL_PTR;
SemaphoreHandle_t tbox_4g_mutex = NULL_PTR;

TBOX_MODULE_FUN(TBOX4G, tbox_4g_init, tbox_4g_stop, tbox_4g_start, NULL_PTR, tbox_4g_exit, tbox_4g_can_be_stop);
TBOX_RUNLOOP_MODULE(TBOX4G, TBOX_TASK_PRIORITY_MID2, LOG_LEVEL_ERROR, TBOX_TASK_LARGE_STACK_SIZE, tbox_4g_task);
TBOX_MODULE_LOADER(TBOX4G)
{
    /*TODO 加载其他信息*/
}

UINT8 tbox_4g_get_state(VOID)
{
    UINT8 state;

    TBOX_4G_MUTEX_LOCK();
    state = tbox_4g_state;
    TBOX_4G_MUTEX_UNLOCK();
    
    return state;
}

VOID tbox_4g_reset(VOID)
{
    TBOX_4G_MUTEX_LOCK();
    tbox_4g_state = (UINT8)TBOX_4G_STATE_STARTING;
    TBOX_4G_MUTEX_UNLOCK();
        
    dev_4g_open();

    if(STIMER_ID_INVALID != tbox_4g_timer_id && 
       FALSE == stimer_is_started(tbox_4g_timer_id))
    {
        stimer_start(tbox_4g_timer_id, PERIODIC_UNIT_4G);
    }

    mgr_4g_reset_sequence();   
}

static INT32 tbox_4g_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            {
                tbox_4g_mutex = xSemaphoreCreateMutex();
                if(NULL_PTR == tbox_4g_mutex)
                {
                    MODULE_LOG_E(TBOX4G, "create mutex failed");
                    return (INT32)TBOX_E_FAILED_INIT;
                }

                tbox_4g_state = (UINT8)TBOX_4G_STATE_UNKNOWN;
                tbox_4g_timer_id = STIMER_ID_INVALID;
                GET_TBOX_MODULE_HANDLE(TBOX4G, tbox_4g_task_handle);
            }
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            {
                TBOX_ID module_id;
                GET_TBOX_MODULE_ID(TBOX4G, module_id);
                tbox_message_subscribe(TBOX_CORE_TASK_ABNORMAL_NOTIFY, module_id, tbox_4g_handle_task_abnormal);
                tbox_message_subscribe(TBOX_CORE_TASK_UPDATE_NOTIFY, module_id, tbox_4g_handle_update_task);

                tbox_4g_timer_id = stimer_create(STIMER_TYPE_PERIOD, tbox_4g_timer_callback);
                if(STIMER_ID_INVALID == tbox_4g_timer_id)
                {
                    MODULE_LOG_E(TBOX4G, "create timer failed");
                    return (INT32)TBOX_E_FAILED_INIT;
                }

                /*启动4G模块处理*/
                tbox_4g_state = (UINT8)TBOX_4G_STATE_STARTING;
                dev_4g_open();
                mgr_4g_init();
                stimer_start(tbox_4g_timer_id, PERIODIC_UNIT_4G);
                tbox_module_set_state(module_id, TBOX_MODULE_STATE_START);
                MODULE_LOG_D(TBOX4G, "4g init finish and start");
            }
            break;

        default:
            break;
    }

    MODULE_LOG_D(TBOX4G, "tbox 4g init seq:%d", seq);
    return (INT32)TBOX_E_OK;
}

static VOID tbox_4g_start(VOID)
{
    TBOX_4G_MUTEX_LOCK();
    tbox_4g_state = (UINT8)TBOX_4G_STATE_STARTING;
    TBOX_4G_MUTEX_UNLOCK();

    dev_4g_open();

    if(STIMER_ID_INVALID != tbox_4g_timer_id)
    {
        stimer_start(tbox_4g_timer_id, PERIODIC_UNIT_4G);
    }

    power_4g_wakeup();

    MODULE_LOG_I(TBOX4G, "tbox 4g started");
}

static VOID tbox_4g_stop(VOID)
{
    TBOX_4G_MUTEX_LOCK();
    tbox_4g_state = (UINT8)TBOX_4G_STATE_STOPPING;
    TBOX_4G_MUTEX_UNLOCK();

    power_4g_sleep(); 
}

static BOOL tbox_4g_can_be_stop(VOID)
{
    return (TRUE == ftp_4g_isdownloading()) ? FALSE : TRUE;
}

static VOID tbox_4g_exit(VOID)
{
    if(NULL_PTR != tbox_4g_task_handle)
    {
        xTaskNotify(tbox_4g_task_handle, TBOX_4G_EVENT_EXIT, eSetBits);
    }
    
    MODULE_LOG_D(TBOX4G, "tbox 4g exited");        
}

static VOID tbox_4g_task(VOID *param)
{
    UINT32 event_bits;

    for (;;) 
    {
        xTaskNotifyWait(0, TBOX_4G_EVENT_ALL, &event_bits, portMAX_DELAY); // 等待通知
        if((event_bits & TBOX_4G_EVENT_EXIT) == TBOX_4G_EVENT_EXIT)
        {
            mgr_4g_reset_sequence();
            stimer_stop(tbox_4g_timer_id);
            dev_4g_close();
            if(NULL_PTR != tbox_4g_mutex)
            {
                vSemaphoreDelete(tbox_4g_mutex);
                tbox_4g_mutex = NULL_PTR;
            }
            break;
        }
        if((event_bits & TBOX_4G_EVENT_PERIOD) == TBOX_4G_EVENT_PERIOD)
        {
            tbox_4g_check_state();
            mgr_4g_all_period();
            continue;
        }
    }

    tbox_4g_task_handle = NULL_PTR;      
    vTaskDelete(NULL);
}

static VOID tbox_4g_timer_callback(VOID)
{
    if(NULL_PTR != tbox_4g_task_handle)
    {
        xTaskNotify(tbox_4g_task_handle, TBOX_4G_EVENT_PERIOD, eSetBits);
    }
}

static VOID tbox_4g_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data)
{
    TBOX_CORE_TASK_ABNORMAL_INFO *abnormal_info;
    if(0U != strncmp(TBOX_CORE_TASK_ABNORMAL_NOTIFY, name, strlen(TBOX_CORE_TASK_ABNORMAL_NOTIFY)) ||
       NULL_PTR == data)
    {
        return;
    }

    if(data->size < sizeof(TBOX_CORE_TASK_ABNORMAL_INFO))
    {
        return;
    }
    abnormal_info = (TBOX_CORE_TASK_ABNORMAL_INFO *)data->data;
    if(NULL_PTR == abnormal_info)
    {
        return;
    }
    if(abnormal_info->task_handle == tbox_4g_task_handle)
    {
        tbox_4g_task_handle = NULL_PTR;
    }
}

static VOID tbox_4g_handle_update_task(const CHAR *name, TBOX_MSG_DATA *data)
{
    TBOX_CORE_TASK_UPDATE_INFO *update_info;
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOX4G, module_id);

    if(0U != strncmp(TBOX_CORE_TASK_UPDATE_NOTIFY, name, strlen(TBOX_CORE_TASK_UPDATE_NOTIFY)) ||
       NULL_PTR == data)
    {
        return;
    }
    if(data->size < sizeof(TBOX_CORE_TASK_UPDATE_INFO))
    {
        return;
    } 
    update_info = (TBOX_CORE_TASK_UPDATE_INFO *)data->data;
    if(NULL_PTR == update_info || 
       NULL_PTR == update_info->task_handle ||
       module_id != update_info->module_id)
    {
        return;
    }
    tbox_4g_task_handle = update_info->task_handle;
}

static VOID tbox_4g_check_state(VOID)
{
    UINT8 state;

    TBOX_4G_MUTEX_LOCK();
    state = tbox_4g_state;
    TBOX_4G_MUTEX_UNLOCK();

    switch(state)
    {
        case TBOX_4G_STATE_STARTING:
        case TBOX_4G_STATE_RUNNING:
            {
                if(FALSE == dev_4g_is_opened())
                {
                    dev_4g_open();
                }
                if(FALSE == stimer_is_started(tbox_4g_timer_id))
                {
                    stimer_start(tbox_4g_timer_id, PERIODIC_UNIT_4G);
                }
                if(state == TBOX_4G_STATE_STARTING)
                {
                    if(AT_4G_STARTUP_SEQ_FINISH == at_4g_get_start_state())
                    {
                        TBOX_4G_MUTEX_LOCK();
                        tbox_4g_state = (UINT8)TBOX_4G_STATE_RUNNING;
                        TBOX_4G_MUTEX_UNLOCK();
                    }
                }
                else
                {
                    if(AT_4G_STARTUP_IDLE == at_4g_get_start_state())
                    {
                        TBOX_4G_MUTEX_LOCK();
                        tbox_4g_state = (UINT8)TBOX_4G_STATE_STARTING;
                        TBOX_4G_MUTEX_UNLOCK();
                    }                    
                }
            }
            break;

        case TBOX_4G_STATE_STOPPING:
            {
                if(TRUE == mgr_4g_is_stopped())
                {
                    TBOX_ID module_id;
                    GET_TBOX_MODULE_ID(TBOX4G, module_id);

                    stimer_stop(tbox_4g_timer_id);
                    dev_4g_close();

                    TBOX_4G_MUTEX_LOCK();
                    tbox_4g_state = (UINT8)TBOX_4G_STATE_SHUTDOWN;
                    TBOX_4G_MUTEX_UNLOCK();

                    tbox_module_set_state(module_id, TBOX_MODULE_STATE_STOP);

                    MODULE_LOG_I(TBOX4G, "tbox 4g timer and uart stoped");
                }
            }
            break;

        default:
            break;
    }
}