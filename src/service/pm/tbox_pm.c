#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "tbox_pm_if.h"
#include "stimer.h"
#include "tbox_pm_inner.h"
#include "tbox_pm_state.h"
#include "tbox_pm_4g_mgr.h"
#include "tbox_pm_action.h"
#include "tbox_pm_io.h"
#include "tbox_pm_time_reboot.h"
#include "tbox_shell_if.h"

#define TBOX_PM_DISABLE_TIMEOUT (300000U) /*300s*/
#define TBOX_PM_IS_DISABLE     (0U)
#define TBOX_PM_IS_ENABLE      (1U)
#define TBOX_PM_EVENT_START    (1U << 0U)
#define TBOX_PM_EVENT_STOP     (1U << 1U)
#define TBOX_PM_EVENT_EXIT     (1U << 2U)
#define TBOX_PM_EVENT_TMOUT    (1U << 3U)
#define TBOX_PM_EVENT_4G_TMOUT (1U << 4U)
#define TBOX_PM_EVENT_ALL     (TBOX_PM_EVENT_START | TBOX_PM_EVENT_STOP | TBOX_PM_EVENT_EXIT | TBOX_PM_EVENT_TMOUT | TBOX_PM_EVENT_4G_TMOUT)

static INT32 tbox_pm_init(UINT8 seq);
static VOID tbox_pm_exit(VOID);
static INT32 tbox_pm_enable(BOOL enable);
static VOID tbox_pm_task(VOID *param);
static VOID tbox_pm_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data);
static VOID tbox_pm_handle_update_task(const CHAR *name, TBOX_MSG_DATA *data);
static VOID tbox_pm_callback(VOID);
static BaseType_t tbox_pm_shell_reset(CHAR *buf, UINT32 bufsz, const CHAR *cmd);
static BaseType_t tbox_pm_shell_cleanresetinfo(CHAR *buf, UINT32 bufsz, const CHAR *cmd);

static TaskHandle_t tbox_pm_task_handle = NULL_PTR;
static STIMER_ID  tbox_pm_timer_id;
static UINT8 tbox_pm_enable_flag;
static TickType_t tbox_pm_enable_tick;
TBOX_MODULE_FUN(TBOXPM, tbox_pm_init, NULL_PTR, NULL_PTR, tbox_pm_enable, tbox_pm_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(TBOXPM, TBOX_TASK_PRIORITY_HIGH, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, tbox_pm_task);
TBOX_SHELL_DEFINE(reset, "reset system[1:mcu 2:4g 3:deepreset4g 4:mcuand4g]", 1, tbox_pm_shell_reset);
TBOX_SHELL_DEFINE(cleanreset, "clean the reset infomation", 0, tbox_pm_shell_cleanresetinfo);
TBOX_MODULE_LOADER(TBOXPM)
{
    /*TODO 加载其他信息*/
}

VOID tbox_pm_start(VOID)
{
    tbox_pm_enable_flag = TBOX_PM_IS_ENABLE;
    tbox_pm_enable_tick = 0U;

    tbox_pm_4g_mgr_start();
    tbox_pm_action_start();

    stimer_start(tbox_pm_timer_id, 1000U);
    if(NULL_PTR != tbox_pm_task_handle)
    {
        xTaskNotify(tbox_pm_task_handle, TBOX_PM_EVENT_START, eSetBits);
    }
}

VOID tbox_pm_stop(VOID)
{
    tbox_pm_state_stop();
    tbox_pm_4g_mgr_stop();
    tbox_pm_action_stop();
    tbox_pm_time_reboot_stop();

    stimer_stop(tbox_pm_timer_id);
    if(NULL_PTR != tbox_pm_task_handle)
    {
        xTaskNotify(tbox_pm_task_handle, TBOX_PM_EVENT_STOP, eSetBits);
    }       
}

VOID tbox_pm_4g_tmout_notify(VOID)
{
    if(NULL_PTR != tbox_pm_task_handle)
    {
        xTaskNotify(tbox_pm_task_handle, TBOX_PM_EVENT_4G_TMOUT, eSetBits);
    }
}

static INT32 tbox_pm_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            {
                tbox_pm_enable_flag = TBOX_PM_IS_ENABLE;
                tbox_pm_enable_tick = 0U;
                GET_TBOX_MODULE_HANDLE(TBOXPM, tbox_pm_task_handle);
            }
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            {
                tbox_pm_timer_id = stimer_create(STIMER_TYPE_PERIOD, tbox_pm_callback);
                if(STIMER_ID_INVALID == tbox_pm_timer_id)
                {
                    MODULE_LOG_E(TBOXPM, "create timer failed");
                    return (INT32)TBOX_E_FAILED_INIT;
                }
  
                tbox_pm_state_init();
                tbox_pm_4g_mgr_init();
                tbox_pm_action_init();
                tbox_pm_io_init();
                tbox_pm_time_reboot_init();

                TBOX_ID module_id;
                GET_TBOX_MODULE_ID(TBOXPM, module_id);
                tbox_message_subscribe(TBOX_CORE_TASK_ABNORMAL_NOTIFY, module_id, tbox_pm_handle_task_abnormal);
                tbox_message_subscribe(TBOX_CORE_TASK_UPDATE_NOTIFY, module_id, tbox_pm_handle_update_task);
                if(NULL_PTR != tbox_pm_task_handle)
                {
                    xTaskNotify(tbox_pm_task_handle, TBOX_PM_EVENT_START, eSetBits);
                }
                stimer_start(tbox_pm_timer_id, 1000U);

                TBOX_SHELL_REGISTER(reset);
                TBOX_SHELL_REGISTER(cleanreset);        
            }
            break;

        default:
            break;
    }

    MODULE_LOG_D(TBOXPM, "tbox pm init seq:%d", seq);
    return (INT32)TBOX_E_OK;    
}

static VOID tbox_pm_exit(VOID)
{
    tbox_pm_4g_mgr_deinit();

    if(NULL_PTR != tbox_pm_task_handle)
    {
        xTaskNotify(tbox_pm_task_handle, TBOX_PM_EVENT_EXIT, eSetBits);
    }

    stimer_stop(tbox_pm_timer_id);
}

static INT32 tbox_pm_enable(BOOL enable)
{
    if(FALSE == enable)
    {
        TickType_t tick = xTaskGetTickCount();
        tbox_pm_enable_flag = TBOX_PM_IS_DISABLE;
        tbox_pm_enable_tick = tick;
        MODULE_LOG_I(TBOXPM, "tbox pm disable, tick:%d", tick);
    }
    else
    {
        tbox_pm_enable_flag = TBOX_PM_IS_ENABLE;
        tbox_pm_enable_tick = 0U;
        MODULE_LOG_I(TBOXPM, "tbox pm enable");        
    }

    return (INT32)TBOX_E_OK;
}

static VOID tbox_pm_task(VOID *param)
{
    UINT32 event_bits;
    
    for (;;) 
    {
        xTaskNotifyWait(0, TBOX_PM_EVENT_ALL, &event_bits, portMAX_DELAY); // 等待通知
        if((event_bits & TBOX_PM_EVENT_EXIT) == TBOX_PM_EVENT_EXIT)
        {
            break;
        }
        if((event_bits & TBOX_PM_EVENT_START) == TBOX_PM_EVENT_START)
        {
            MODULE_LOG_I(TBOXPM, "tbox pm started"); 
            continue;
        }
        if((event_bits & TBOX_PM_EVENT_STOP) == TBOX_PM_EVENT_STOP)
        {
            MODULE_LOG_I(TBOXPM, "tbox pm stopped");      
            continue;
        }
        if((event_bits & TBOX_PM_EVENT_TMOUT) == TBOX_PM_EVENT_TMOUT)
        {
            if(TBOX_PM_IS_DISABLE == tbox_pm_enable_flag)
            {
                TickType_t tick = xTaskGetTickCount();
                if(tick - tbox_pm_enable_tick >= TBOX_PM_DISABLE_TIMEOUT)
                {
                    tbox_pm_enable_flag = TBOX_PM_IS_ENABLE;
                    tbox_pm_enable_tick = 0U;
                    MODULE_LOG_I(TBOXPM, "tbox pm disable timeout and change to enable");
                }
                else
                {
                    continue;
                }
            }

            tbox_pm_state_timeout();
            tbox_pm_4g_mgr_period();
            tbox_pm_action_period();
            tbox_pm_time_reboot_period();
        }
        if((event_bits & TBOX_PM_EVENT_4G_TMOUT) == TBOX_PM_EVENT_4G_TMOUT)
        {
            tbox_pm_4g_do_timeout();
        }
    }
    
    vTaskDelete(NULL);
    tbox_pm_task_handle = NULL_PTR;   
}

static VOID tbox_pm_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data)
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
    if(abnormal_info->task_handle == tbox_pm_task_handle)
    {
        tbox_pm_task_handle = NULL_PTR;
    }
}

static VOID tbox_pm_handle_update_task(const CHAR *name, TBOX_MSG_DATA *data)
{
    TBOX_CORE_TASK_UPDATE_INFO *update_info;
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOXPM, module_id);

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
    tbox_pm_task_handle = update_info->task_handle;
}

static VOID tbox_pm_callback(VOID)
{
    if(NULL_PTR != tbox_pm_task_handle)
    {
        xTaskNotify(tbox_pm_task_handle, TBOX_PM_EVENT_TMOUT, eSetBits);
    }   
}

static BaseType_t tbox_pm_shell_reset(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    CHAR *endptr;
    BaseType_t param_len = 0;
    UINT32 reset_type;

    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL == param_ptr || param_len == 0) 
    {
        snprintf(buf, bufsz, "parse reset type failed.\r\n");
        return pdFALSE;
    }
    reset_type = strtoul(param_ptr, &endptr, 10);
    if (endptr == param_ptr || reset_type > 4U || reset_type < 1U) 
    {
        snprintf(buf, bufsz, "the reset type must be 1-4.\r\n");
        return pdFALSE;
    }
    INT32 ret = tbox_pm_reboot((TBOX_PM_REBOOT_TYPE)reset_type);
    if(ret != (INT32)TBOX_E_OK)
    {
        snprintf(buf, bufsz, "reset failed.\r\n");
    }

    return pdFALSE;    
}

static BaseType_t tbox_pm_shell_cleanresetinfo(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    UNUSED(cmd);
    
    tbox_pm_action_cleanresetinfo();

    return pdFALSE;
}