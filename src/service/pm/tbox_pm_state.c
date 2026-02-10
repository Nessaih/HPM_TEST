#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "tbox_pm_if.h"
#include "tbox_pm_state.h"
#include "tbox_pm_inner.h"
#include "tbox_pm_4g_mgr.h"
#include "tbox_mode_if.h"
#include "tbox_pm_action.h"
#include "tbox_pm_io.h"

#define TBOX_PM_SLEEP_MODE_LISTEN_ONLY 0U
#define TBOX_PM_SLEEP_MODE_SLEEP_ONLY  1U

#define TBOX_PM_STATE_WAIT_STOP_TIMEOUT               (30U)      /*30S*/

typedef VOID (*TBOX_PM_STATE_PROC)(VOID);

typedef enum
{
    TBOX_PM_EVENT_WAKEUP,
    TBOX_PM_EVENT_TIMEOUT,
    TBOX_PM_EVENT_REBOOT,
    TBOX_PM_EVENT_MAX
}TBOX_PM_EVENT;

static inline VOID tbox_pm_state_enter(TBOX_PM_STATE state);

static VOID tbox_pm_wakeup_in_running(VOID);
static VOID tbox_pm_timeout_in_running(VOID);
static VOID tbox_pm_reboot_in_running(VOID);
static VOID tbox_pm_wakeup_in_sleepprecheck(VOID);
static VOID tbox_pm_timeout_in_sleepprecheck(VOID);
static VOID tbox_pm_reboot_in_sleepprecheck(VOID);
static VOID tbox_pm_wakeup_in_sleeppostcheck(VOID);
static VOID tbox_pm_timeout_in_sleeppostcheck(VOID);
static VOID tbox_pm_reboot_in_sleeppostcheck(VOID);
static VOID tbox_pm_wakeup_in_doaction(VOID);
static VOID tbox_pm_timeout_in_doaction(VOID);
static VOID tbox_pm_reboot_in_doaction(VOID);
static VOID tbox_pm_wakeup_in_finish(VOID);
static VOID tbox_pm_timeout_in_finish(VOID);
static VOID tbox_pm_reboot_in_finish(VOID);

static UINT8 tbox_pm_state;
static UINT8 tbox_pm_post_action;
static UINT8 tbox_pm_wake_type;
static UINT8 tbox_pm_reboot_type;
static UINT16 tbox_pm_tmcount;
static SemaphoreHandle_t tbox_pm_state_mutex = NULL_PTR;
static TBOX_PM_STATE_PROC tbox_pm_state_proc[TBOX_PM_STATE_MAX][TBOX_PM_EVENT_MAX] = 
{
    /*TBOX_PM_EVENT_WAKEUP                  TBOX_PM_EVENT_TIMEOUT                    TBOX_PM_EVENT_REBOOT*/
    {tbox_pm_wakeup_in_running,             tbox_pm_timeout_in_running,             tbox_pm_reboot_in_running},        /*TBOX_PM_STATE_RUNNING*/
    {tbox_pm_wakeup_in_sleepprecheck,       tbox_pm_timeout_in_sleepprecheck,       tbox_pm_reboot_in_sleepprecheck},  /*TBOX_PM_STATE_SLEEP_PRE_CHECK*/
    {tbox_pm_wakeup_in_sleeppostcheck,      tbox_pm_timeout_in_sleeppostcheck,      tbox_pm_reboot_in_sleeppostcheck}, /*TBOX_PM_STATE_SLEEP_POST_CHECK*/
    {tbox_pm_wakeup_in_doaction,            tbox_pm_timeout_in_doaction,            tbox_pm_reboot_in_doaction},       /*TBOX_PM_STATE_DOACITON*/
    {tbox_pm_wakeup_in_finish,              tbox_pm_timeout_in_finish,              tbox_pm_reboot_in_finish},         /*TBOX_PM_STATE_FINISH*/
};

INT32 tbox_pm_state_init(VOID)
{
    tbox_pm_post_action = TBOX_PM_SLEEPPOST_ACTION_NONE;
    tbox_pm_state = TBOX_PM_STATE_RUNNING;
    tbox_pm_tmcount = 0U;
    tbox_pm_reboot_type = TBOX_PM_REBOOT_NONE;
    if(tbox_pm_io_acc_is_active())
    {
        tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
    }
    else
    {
        tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_TIMER;
    }

    tbox_pm_state_mutex = xSemaphoreCreateMutex();
    if(NULL_PTR == tbox_pm_state_mutex)
    {
        MODULE_LOG_E(TBOXPM, "create mutex failed");
        return (INT32)TBOX_E_FAILED_INIT;
    }
    
    return (INT32)TBOX_E_OK;
}

VOID tbox_pm_state_stop(VOID)
{
    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    tbox_pm_state_enter(TBOX_PM_STATE_FINISH);
//    tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_NONE;
    tbox_pm_tmcount = 0U;
    tbox_pm_reboot_type = TBOX_PM_REBOOT_NONE;
    xSemaphoreGive(tbox_pm_state_mutex);    
}

VOID tbox_pm_state_timeout(VOID)
{
    UINT8 old_state;
    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    old_state = tbox_pm_state;
    xSemaphoreGive(tbox_pm_state_mutex);
    if(old_state >=  TBOX_PM_STATE_MAX)
    {
        return;
    }

    tbox_pm_state_proc[old_state][TBOX_PM_EVENT_TIMEOUT]();
}

TBOX_PM_STATE tbox_pm_state_get(VOID)
{
    UINT8 old_state;

    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    old_state = tbox_pm_state;
    xSemaphoreGive(tbox_pm_state_mutex);

    return (TBOX_PM_STATE)old_state;
}

TBOX_PM_SLEEPPOST_ACTION tbox_pm_get_sleeppost_action(VOID)
{
    UINT8 old_post_action;
    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    old_post_action = tbox_pm_post_action;
    xSemaphoreGive(tbox_pm_state_mutex);

    return (TBOX_PM_SLEEPPOST_ACTION)old_post_action;
}

INT32 tbox_pm_reboot(TBOX_PM_REBOOT_TYPE type)
{
    UINT8 old_state;
    
    if(TBOX_PM_REBOOT_NONE == type)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    switch(type)
    {
        case TBOX_PM_REBOOT_MCU:
            if(FALSE == tbox_pm_action_is_cando(TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU))
            {
                return (INT32)TBOX_E_FAILED;
            }
            break;

        case TBOX_PM_REBOOT_4G:
            if(FALSE == tbox_pm_action_is_cando(TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G))
            {
                return (INT32)TBOX_E_FAILED;
            }
            break;

        case TBOX_PM_DEEPREBOOT_4G:
            if(FALSE == tbox_pm_action_is_cando(TBOX_PM_SLEEPPOST_ACTION_DEEPREBOOT_4G))
            {
                return (INT32)TBOX_E_FAILED;
            }
            break;

        case TBOX_PM_REBOOT_4G_MCU:
            if(FALSE == tbox_pm_action_is_cando(TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU))
            {
                return (INT32)TBOX_E_FAILED;
            }
            break;

        default:
            break;
    }

    if(TBOX_PM_REBOOT_4G == type   ||
       TBOX_PM_DEEPREBOOT_4G == type)
    {
        MODULE_LOG_I(TBOXPM, "it is not need stop module for reboot type:%d", type);
        xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
        if(TBOX_PM_REBOOT_4G == type)
        {
            tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G;
        }
        else
        {
            tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_DEEPREBOOT_4G;
        }
        xSemaphoreGive(tbox_pm_state_mutex);
        return tbox_pm_action_do((TBOX_PM_SLEEPPOST_ACTION)tbox_pm_post_action);
    }

    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    old_state = tbox_pm_state;
    tbox_pm_reboot_type = (UINT8)type;
    xSemaphoreGive(tbox_pm_state_mutex);
    if(old_state >=  TBOX_PM_STATE_MAX)
    {
        return TBOX_E_INVALID_STATE;
    }

    tbox_pm_state_proc[old_state][TBOX_PM_EVENT_REBOOT]();

    return (INT32)TBOX_E_OK;
}

BOOL  tbox_pm_is_reboot_finish(TBOX_PM_REBOOT_TYPE type)
{
    BOOL ret = TRUE;

    switch (type)
    {
        case TBOX_PM_REBOOT_4G:
        case TBOX_PM_DEEPREBOOT_4G:
            {
                if(TBOX_PM_4G_DO_NOTHING != tbox_pm_4g_get_do_state())
                {
                    ret = FALSE;
                }
            }
            break;
            
        case TBOX_PM_REBOOT_MCU:
        case TBOX_PM_REBOOT_4G_MCU:
            {
                if(TBOX_PM_STATE_FINISH != tbox_pm_state_get())
                {
                     ret = FALSE;
                }
            }
            break;

        default:
            break;
    }

    return ret;
}

INT32 tbox_pm_wakeup(TBOX_PM_WAKEUP_TYPE type)
{
    UINT8 old_state;
    
    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    old_state = tbox_pm_state;
    if(tbox_pm_io_acc_is_active())
    {
        tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
    }
    else
    {
        tbox_pm_wake_type = (UINT8)type;
    }
    xSemaphoreGive(tbox_pm_state_mutex);
    
    if(old_state >=  TBOX_PM_STATE_MAX)
    {
        return TBOX_E_INVALID_STATE;
    }

    tbox_pm_state_proc[old_state][TBOX_PM_EVENT_WAKEUP]();

    return (INT32)TBOX_E_OK;
}

VOID  tbox_pm_fctsleep(VOID)
{
    tbox_pm_stop();
    tbox_module_stop();
    for(UINT8 retry_count = 0U; retry_count <= 20U; retry_count++)
    {
        drv_wdg_feed();
        if(TRUE == tbox_allmodule_isstoped())
        {
            break;
        }
        vTaskDelay(pdTICKS_TO_MS(200U));
    }
    tbox_pm_force_shutdown();
    tbox_module_start();
    tbox_pm_start();
    tbox_pm_4g_startup();
}

VOID  tbox_pm_4g_startup(VOID)
{
    tbox_pm_4g_do_startup();
}

VOID  tbox_pm_4g_shutdown(VOID)
{
    tbox_pm_4g_do_shutdown();
}

static inline VOID tbox_pm_state_enter(TBOX_PM_STATE state)
{
    MODULE_LOG_I(TBOXPM, "change state from %u to %u", tbox_pm_state, (UINT8)state);

    tbox_pm_state = state;
}

static VOID tbox_pm_wakeup_in_running(VOID)
{
    MODULE_LOG_I(TBOXPM, "wakeup in running, wake type:%d", tbox_pm_wake_type);

    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    if(tbox_pm_io_acc_is_active())
    {
        tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
    }
    tbox_pm_tmcount = 0U;
    tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_NONE;
    tbox_pm_reboot_type = TBOX_PM_REBOOT_NONE;
    xSemaphoreGive(tbox_pm_state_mutex);
}

static VOID tbox_pm_timeout_in_running(VOID)
{
    if(tbox_pm_io_acc_is_active())
    {
        xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
        tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
        xSemaphoreGive(tbox_pm_state_mutex);
    }
    else
    {
        if(tbox_allmodule_canbe_stop())
        {
            xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
            tbox_pm_state_enter(TBOX_PM_STATE_SLEEP_PRE_CHECK);
            xSemaphoreGive(tbox_pm_state_mutex);
        }
    }
}

static VOID tbox_pm_reboot_in_running(VOID)
{
    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    if((UINT8)TBOX_PM_REBOOT_MCU == tbox_pm_reboot_type)
    {
        tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU;
        tbox_pm_state_enter(TBOX_PM_STATE_SLEEP_PRE_CHECK);   
    }
    else if((UINT8)TBOX_PM_REBOOT_4G_MCU == tbox_pm_reboot_type)
    {
        tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU;
        tbox_pm_state_enter(TBOX_PM_STATE_SLEEP_PRE_CHECK);
    }
    else
    {
        /*TODO*/
    }
    xSemaphoreGive(tbox_pm_state_mutex);    
}

static VOID tbox_pm_wakeup_in_sleepprecheck(VOID)
{
    MODULE_LOG_I(TBOXPM, "wakeup in sleepprecheck, wake type:%d", tbox_pm_wake_type);

    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    if((UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU != tbox_pm_post_action &&
       (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU != tbox_pm_post_action)
    {
        if(tbox_pm_io_acc_is_active())
        {
            tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
        }
        tbox_pm_state_enter(TBOX_PM_STATE_RUNNING);  
    }
    else
    {
        MODULE_LOG_I(TBOXPM, "continue to execute the reboot action, type:%d", tbox_pm_post_action);
    }
    xSemaphoreGive(tbox_pm_state_mutex);
}

static VOID tbox_pm_timeout_in_sleepprecheck(VOID)
{
    if((UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU == tbox_pm_post_action ||
       (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU == tbox_pm_post_action)
    {
        tbox_module_stop();
        xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
        tbox_pm_state_enter(TBOX_PM_STATE_SLEEP_POST_CHECK);
        tbox_pm_tmcount = 0U;
        xSemaphoreGive(tbox_pm_state_mutex);
    }
    else
    {
        if(tbox_pm_io_acc_is_active())
        {
            xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
            tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
            tbox_pm_state_enter(TBOX_PM_STATE_RUNNING);
            xSemaphoreGive(tbox_pm_state_mutex);
        }
        else
        {
            if(!tbox_allmodule_canbe_stop())
            {
                xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
                tbox_pm_state_enter(TBOX_PM_STATE_RUNNING);
                xSemaphoreGive(tbox_pm_state_mutex);
            }
            else
            {
                tbox_module_stop();

                UINT8 sleep_mode = 0U;
                TBOX_CFG_ID sleep_mode_id;
                TBOX_CFG_ID_GET(SLEEPMODE, sleep_mode_id);
                tbox_cfg_read(sleep_mode_id, &sleep_mode);

                TBOX_MODE_TYPE mode = TBOX_MODE_MAX;
                tbox_mode_get(&mode);

                xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
                if(TBOX_MODE_FACTORY == mode || 
                   TBOX_MODE_UNDERVOLTAGE == mode)
                {
                    tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN;
                }
                else
                {
                    if(TBOX_PM_SLEEP_MODE_SLEEP_ONLY == sleep_mode)
                    {
                        tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN;
                    }
                    else
                    {
                        tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_SLEEP;
                    }
                }
                tbox_pm_tmcount = 0U;
                tbox_pm_state_enter(TBOX_PM_STATE_SLEEP_POST_CHECK);
                xSemaphoreGive(tbox_pm_state_mutex);
            }
        }
    }
}

static VOID tbox_pm_reboot_in_sleepprecheck(VOID)
{
    if((UINT8)TBOX_PM_REBOOT_MCU == tbox_pm_reboot_type)
    {
        tbox_module_stop();
        xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
        tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU;
        tbox_pm_state_enter(TBOX_PM_STATE_SLEEP_POST_CHECK);
        tbox_pm_tmcount = 0U;
        xSemaphoreGive(tbox_pm_state_mutex);    
    }
    else if((UINT8)TBOX_PM_REBOOT_4G_MCU == tbox_pm_reboot_type)
    {
        tbox_module_stop();
        xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
        tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU;
        tbox_pm_state_enter(TBOX_PM_STATE_SLEEP_POST_CHECK);
        tbox_pm_tmcount = 0U;
        xSemaphoreGive(tbox_pm_state_mutex);
    }
    else
    {
        /*TODO*/
    } 
}

static VOID tbox_pm_wakeup_in_sleeppostcheck(VOID)
{
    MODULE_LOG_I(TBOXPM, "wakeup in sleeppostcheck, wake type:%d", tbox_pm_wake_type);
   
    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    if((UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU != tbox_pm_post_action &&
       (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU != tbox_pm_post_action)
    {
        tbox_module_start();
        if(tbox_pm_io_acc_is_active())
        {
            tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
        }
        tbox_pm_state_enter(TBOX_PM_STATE_RUNNING);  
    }
    else
    {
         MODULE_LOG_I(TBOXPM, "continue to execute the reboot action, type:%d", tbox_pm_post_action);
    }
    xSemaphoreGive(tbox_pm_state_mutex);
}

static VOID tbox_pm_timeout_in_sleeppostcheck(VOID)
{
    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    UINT8 post_action = tbox_pm_post_action;
    xSemaphoreGive(tbox_pm_state_mutex);

    MODULE_LOG_I(TBOXPM, "wait module stop finish[%u S], post action:%d", tbox_pm_tmcount, post_action);

    if((UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU == post_action ||
       (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU == post_action)
    {
        if(TRUE == tbox_allmodule_isstoped())
        {
            xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
            tbox_pm_state_enter(TBOX_PM_STATE_DOACITON);
            tbox_pm_tmcount = 0U;
            xSemaphoreGive(tbox_pm_state_mutex);

            if((UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU == post_action)
            {
                tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU);
            }
            else
            {
                tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU);
            }
        }
        else
        {
            xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
            if(++tbox_pm_tmcount >= TBOX_PM_STATE_WAIT_STOP_TIMEOUT)
            {
                tbox_pm_state_enter(TBOX_PM_STATE_DOACITON);
                tbox_pm_tmcount = 0U;
            }
            else
            {
                xSemaphoreGive(tbox_pm_state_mutex);
                return;
            }
            xSemaphoreGive(tbox_pm_state_mutex);
            
            if((UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU == post_action)
            {
                tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU);
            }
            else
            {
                tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU);
            }          
        }
    }
    else
    {
        if(tbox_pm_io_acc_is_active())
        {
            tbox_module_start();

            xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
            tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
            tbox_pm_tmcount = 0U;
            tbox_pm_state_enter(TBOX_PM_STATE_RUNNING);
            xSemaphoreGive(tbox_pm_state_mutex);
        }
        else
        {
            if(TRUE == tbox_allmodule_isstoped())
            {
                xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
                tbox_pm_state_enter(TBOX_PM_STATE_DOACITON);
                tbox_pm_tmcount = 0U;
                xSemaphoreGive(tbox_pm_state_mutex);

                if((UINT8)TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN == post_action)
                {
                    tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN);
                }
                else
                {
                    tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION_SLEEP);
                }
            }
            else
            {
                xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
                if(++tbox_pm_tmcount >= TBOX_PM_STATE_WAIT_STOP_TIMEOUT)
                {
                    tbox_pm_state_enter(TBOX_PM_STATE_DOACITON);
                    tbox_pm_tmcount = 0U;
                }
                else
                {
                    xSemaphoreGive(tbox_pm_state_mutex);
                    return;
                }
                xSemaphoreGive(tbox_pm_state_mutex);

                if((UINT8)TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN == post_action)
                {
                    tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN);
                }
                else
                {
                    tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION_SLEEP);
                }
            }
        }  
    } 
}

static VOID tbox_pm_reboot_in_sleeppostcheck(VOID)
{
    if((UINT8)TBOX_PM_REBOOT_MCU == tbox_pm_reboot_type)
    {
        xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
        tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU;
        xSemaphoreGive(tbox_pm_state_mutex);    
    }
    else if((UINT8)TBOX_PM_REBOOT_4G_MCU == tbox_pm_reboot_type)
    {
        xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
        tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU;
        xSemaphoreGive(tbox_pm_state_mutex);
    }
    else
    {
        /*TODO*/
    }
}

static VOID tbox_pm_wakeup_in_doaction(VOID)
{
    MODULE_LOG_I(TBOXPM, "wakeup in doaction, wake type:%d", tbox_pm_wake_type);
    
    if((UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU != tbox_pm_post_action &&
       (UINT8)TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU != tbox_pm_post_action)
    {
        tbox_module_start();
        if(tbox_pm_io_acc_is_active())
        {
            tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
        }
        tbox_pm_state_enter(TBOX_PM_STATE_RUNNING);  
    }
    else
    {
        MODULE_LOG_I(TBOXPM, "continue to execute the reboot action, type:%d", tbox_pm_post_action);
    }
}

static VOID tbox_pm_timeout_in_doaction(VOID)
{
    MODULE_LOG_I(TBOXPM, "wait doaction finish[%u S], post action:%d", tbox_pm_tmcount, tbox_pm_post_action);

    BOOL is_acc_active = tbox_pm_io_acc_is_active();
    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    if((UINT8)TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN == tbox_pm_post_action ||
       (UINT8)TBOX_PM_SLEEPPOST_ACTION_SLEEP == tbox_pm_post_action)
    {
        if(TRUE == is_acc_active)
        {
            tbox_pm_abort_action();
        }
        else
        {
            tbox_pm_clear_abort_action();
        }
    }
    tbox_pm_tmcount++;
    xSemaphoreGive(tbox_pm_state_mutex);
}

static VOID tbox_pm_reboot_in_doaction(VOID)
{
    /*TODO:根据实际情况修改，目前不处理RBEOOT动作*/
}

static VOID tbox_pm_wakeup_in_finish(VOID)
{
    MODULE_LOG_I(TBOXPM, "wakeup in finish, wake type:%d", tbox_pm_wake_type);

    tbox_module_start();

    xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
    if((UINT8)TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN == tbox_pm_post_action)
    {
        tbox_pm_4g_do_startup();
        tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_NONE;
        MODULE_LOG_I(TBOXPM, "start up 4g");        
    }
    if(tbox_pm_io_acc_is_active())
    {
        tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
    }
    tbox_pm_state_enter(TBOX_PM_STATE_RUNNING);    
    xSemaphoreGive(tbox_pm_state_mutex);
}

static VOID tbox_pm_timeout_in_finish(VOID)
{
    BOOL is_acc_active = tbox_pm_io_acc_is_active();
    if(is_acc_active)
    {
        tbox_module_start();

        xSemaphoreTake(tbox_pm_state_mutex, portMAX_DELAY);
        if((UINT8)TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN == tbox_pm_post_action)
        {
            tbox_pm_4g_do_startup();
            tbox_pm_post_action = (UINT8)TBOX_PM_SLEEPPOST_ACTION_NONE;        
        }    
        tbox_pm_wake_type = TBOX_PM_WAKEUP_BY_KEY;
        tbox_pm_state_enter(TBOX_PM_STATE_RUNNING);    
        xSemaphoreGive(tbox_pm_state_mutex);

        MODULE_LOG_I(TBOXPM, "wake by key in finish");        
    }
    else
    {
        if((UINT8)TBOX_PM_SLEEPPOST_ACTION_SLEEP == tbox_pm_post_action)
        {
            tbox_pm_force_sleep();
        }
        else if((UINT8)TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN == tbox_pm_post_action)
        {
            tbox_pm_force_shutdown();
        }
        else
        {
            /*TODO*/
        }
    }   
}

static VOID tbox_pm_reboot_in_finish(VOID)
{
    /*DO NOTHING*/
}
