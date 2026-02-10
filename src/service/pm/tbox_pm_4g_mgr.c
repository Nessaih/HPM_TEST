#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "tbox_pm_inner.h"
#include "tbox_pm_4g_mgr.h"
#include "stimer.h"
#include "tbox_pm_if.h"
#include "tbox_pm_io.h"
#include "4g_if.h"
#include "Rcm_Hal.h"

#define TBOX_PM_4G_SHUTDOWN_TIME           (3300U) /* >= 3100ms(EC200x) */
#define TBOX_PM_4G_STARTUP_TIME            (2200U) /* >= 2000ms(EC200x) */
#define TBOX_PM_4G_RESET_TIME              (500U)  /* >= 300ms (EC200x) */
#define TBOX_PM_4G_DEEPRESET_SHUTDOWN_TIME (30000U)  /* 30000ms(30s) */
#define TBOX_PM_4G_DEEPRESET_POWEROFF_TIME (174000U) /* 174000ms(174s) */
#define TBOX_PM_4G_DEEPRESET_POWERON_TIME  (100U)    /* 100ms */
#define TBOX_PM_4G_DEEPRESET_STARTUP_TIME  (4000U)   /* 4000ms */
#define TBOX_PM_4G_CHECK_STARTUP_TIME      (20U)     /* 20S */
#define TBOX_PM_4G_LAST_ACTION_MAGIC_NO    (0x5A5A)
#define TBOX_PM_4G_LAST_ACTION_NAME        "PM4G_LAST_ACTION"

typedef struct
{
    UINT16 magic_no;
    UINT16 action;
}TBOX_PM_4G_MGR_LAST_ACTION;

#define TBOX_PM_4G_IS_DOING()    (((UINT8)TBOX_PM_4G_DO_NOTHING != tbox_pm_4g_do_action))

typedef enum
{
    TBOX_PM_4G_DEEPRESET_STATE_NONE,
    TBOX_PM_4G_DEEPRESET_STATE_SHUTDOWN,
    TBOX_PM_4G_DEEPRESET_STATE_WAIT_SHUTDOWN,
    TBOX_PM_4G_DEEPRESET_STATE_POWEROFF,
    TBOX_PM_4G_DEEPRESET_STATE_POWERON,
    TBOX_PM_4G_DEEPRESET_STATE_STARTUP,
    TBOX_PM_4G_DEEPRESET_STATE_WAIT_STARTUP,
    TBOX_PM_4G_DEEPRESET_STATE_FINISH
}TBOX_PM_4G_DEEPRESET_STATE;

typedef enum
{
    TBOX_PM_4G_CHECK_STARTUP_STATE_NONE = 0U,
    TBOX_PM_4G_CHECK_STARTUP_STATE_WAIT_STARTUP4G
}TBOX_PM_4G_CHECK_STARTUP_STATE;

static VOID tbox_pm_4g_do_timer_callback(VOID);
static VOID tbox_pm_4g_handle_deepreset(VOID);
static VOID tbox_pm_4g_handle_check_startup(VOID);
static VOID tbox_pm_4g_save_last_action(VOID);
static UINT16 tbox_pm_4g_load_last_action(VOID);
static VOID tbox_pm_4g_init_do_startup(VOID);

static UINT8 tbox_pm_4g_do_action;
static UINT8 tbox_pm_4g_deepreset_state;
static UINT8 tbox_pm_4g_check_startup_state;
static UINT32 tbox_pm_4g_check_startup_count;
static SemaphoreHandle_t tbox_pm_4g_mutex = NULL_PTR;
static STIMER_ID  tbox_pm_4g_do_timer_id;

INT32 tbox_pm_4g_mgr_init(VOID)
{
    tbox_pm_4g_do_action = (UINT8)TBOX_PM_4G_DO_NOTHING;
    tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_NONE;
    tbox_pm_4g_mutex = xSemaphoreCreateMutex();
    if(NULL_PTR == tbox_pm_4g_mutex)
    {
        MODULE_LOG_E(TBOXPM, "create mutex failed");
        return (INT32)TBOX_E_FAILED_INIT;
    }

    tbox_pm_4g_do_timer_id = stimer_create(STIMER_TYPE_ONCE, tbox_pm_4g_do_timer_callback);
    if(STIMER_ID_INVALID == tbox_pm_4g_do_timer_id)
    {
        vSemaphoreDelete(tbox_pm_4g_mutex);
        tbox_pm_4g_mutex = NULL_PTR;
        MODULE_LOG_E(TBOXPM, "create timer failed");
        return (INT32)TBOX_E_FAILED_INIT;
    }

    tbox_pm_4g_check_startup_count = 0U;

    tbox_pm_4g_init_do_startup();

    return (INT32)TBOX_E_OK;
}

VOID tbox_pm_4g_mgr_deinit(VOID)
{
    stimer_stop(tbox_pm_4g_do_timer_id);

    if(NULL_PTR != tbox_pm_4g_mutex)
    {
        vSemaphoreDelete(tbox_pm_4g_mutex);
        tbox_pm_4g_mutex = NULL_PTR;
    }
}

VOID tbox_pm_4g_mgr_start(VOID)
{
    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);
    tbox_pm_4g_do_action = (UINT8)TBOX_PM_4G_DO_NOTHING;
    tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_NONE;
    tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_WAIT_STARTUP4G;
    tbox_pm_4g_check_startup_count = 0U;  
    xSemaphoreGive(tbox_pm_4g_mutex);
}

VOID tbox_pm_4g_mgr_stop(VOID)
{
    tbox_pm_4g_save_last_action();
    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);
    tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_NONE;
    tbox_pm_4g_check_startup_count = 0U;
    xSemaphoreGive(tbox_pm_4g_mutex);
}

VOID tbox_pm_4g_mgr_period(VOID)
{
    tbox_pm_4g_handle_check_startup();
}

INT32 tbox_pm_4g_do_startup(VOID)
{
    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);
    if(TBOX_PM_4G_IS_DOING())
    {
        xSemaphoreGive(tbox_pm_4g_mutex);
        MODULE_LOG_E(TBOXPM, "4g is doing, action:%d", tbox_pm_4g_do_action);
        return (INT32)TBOX_E_IS_BUSY;
    }
    tbox_pm_4g_do_action = (UINT8)TBOX_PM_4G_DO_STARTUP;
    tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_NONE;
    tbox_pm_4g_check_startup_count = 0U;    
    xSemaphoreGive(tbox_pm_4g_mutex);

    if((INT32)TBOX_E_OK != stimer_start(tbox_pm_4g_do_timer_id, TBOX_PM_4G_STARTUP_TIME))
    {
        MODULE_LOG_E(TBOXPM, "start timer failed");
        return (INT32)TBOX_E_FAILED;
    }
    drv_pin_set_level(PIN_PWRKEY_MPU, 1U);

    return (INT32)TBOX_E_OK;
}

INT32 tbox_pm_4g_do_shutdown(VOID)
{
    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);
    if(TBOX_PM_4G_IS_DOING())
    {
        xSemaphoreGive(tbox_pm_4g_mutex);
        MODULE_LOG_E(TBOXPM, "4g is doing, action:%d", tbox_pm_4g_do_action);
        return (INT32)TBOX_E_IS_BUSY;
    }
    tbox_pm_4g_do_action = (UINT8)TBOX_PM_4G_DO_SHUTDOWN;
    tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_NONE;
    tbox_pm_4g_check_startup_count = 0U;    
    xSemaphoreGive(tbox_pm_4g_mutex);

    if((INT32)TBOX_E_OK != stimer_start(tbox_pm_4g_do_timer_id, TBOX_PM_4G_SHUTDOWN_TIME))
    {
        MODULE_LOG_E(TBOXPM, "start timer failed");
        return (INT32)TBOX_E_FAILED;
    }
    drv_pin_set_level(PIN_PWRKEY_MPU, 1U);

    return (INT32)TBOX_E_OK;
}

INT32 tbox_pm_4g_do_reset(VOID)
{
    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);
    if(TBOX_PM_4G_IS_DOING())
    {
        xSemaphoreGive(tbox_pm_4g_mutex);
        MODULE_LOG_E(TBOXPM, "4g is doing, action:%d", tbox_pm_4g_do_action);
        return (INT32)TBOX_E_IS_BUSY;
    }
    tbox_pm_4g_do_action = (UINT8)TBOX_PM_4G_DO_RESET;
    tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_NONE;
    tbox_pm_4g_check_startup_count = 0U;    
    xSemaphoreGive(tbox_pm_4g_mutex);

    if((INT32)TBOX_E_OK != stimer_start(tbox_pm_4g_do_timer_id, TBOX_PM_4G_RESET_TIME))
    {
        MODULE_LOG_E(TBOXPM, "start timer failed");
        return (INT32)TBOX_E_FAILED;
    }
    drv_pin_set_level(PIN_RESET_MPU, 1U);

    return (INT32)TBOX_E_OK;    
}

INT32 tbox_pm_4g_do_deepreset(VOID)
{
    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);
    if(TBOX_PM_4G_IS_DOING())
    {
        xSemaphoreGive(tbox_pm_4g_mutex);
        MODULE_LOG_E(TBOXPM, "4g is doing, action:%d", tbox_pm_4g_do_action);
        return (INT32)TBOX_E_IS_BUSY;
    }
    tbox_pm_4g_do_action = (UINT8)TBOX_PM_4G_DO_DEEPRESET;    
    tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_SHUTDOWN;
    tbox_pm_4g_handle_deepreset();
    tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_NONE;
    tbox_pm_4g_check_startup_count = 0U;    
    xSemaphoreGive(tbox_pm_4g_mutex);

    return (INT32)TBOX_E_OK; 
}

VOID tbox_pm_4g_do_timeout(VOID)
{
    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);
    switch((TBOX_PM_4G_DO_STATE)tbox_pm_4g_do_action)
    {
        case TBOX_PM_4G_DO_STARTUP:
        case TBOX_PM_4G_DO_SHUTDOWN:
            {
                drv_pin_set_level(PIN_PWRKEY_MPU, 0U);
                if((UINT8)TBOX_PM_4G_DO_STARTUP == tbox_pm_4g_do_action)
                {
                    if_4g_reset();
                    tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_WAIT_STARTUP4G;
                    tbox_pm_4g_check_startup_count = 0U;               
                }
                tbox_pm_4g_do_action = (UINT8)TBOX_PM_4G_DO_NOTHING;
            }
            break;

        case TBOX_PM_4G_DO_RESET:
            {
                drv_pin_set_level(PIN_RESET_MPU, 0U);
                if_4g_reset();
                tbox_pm_4g_do_action = (UINT8)TBOX_PM_4G_DO_NOTHING;
                tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_WAIT_STARTUP4G;
                tbox_pm_4g_check_startup_count = 0U;
                MODULE_LOG_I(TBOXPM, "reset 4g success"); 
            }
            break;

        case TBOX_PM_4G_DO_DEEPRESET:
            {
                tbox_pm_4g_handle_deepreset();
            }
            break;

        default:
            break;
    }
    xSemaphoreGive(tbox_pm_4g_mutex);
}

TBOX_PM_4G_DO_STATE tbox_pm_4g_get_do_state(VOID)
{
    TBOX_PM_4G_DO_STATE state;

    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);
    state = (TBOX_PM_4G_DO_STATE)tbox_pm_4g_do_action;
    xSemaphoreGive(tbox_pm_4g_mutex);

    return state;
}

static VOID tbox_pm_4g_do_timer_callback(VOID)
{
    tbox_pm_4g_tmout_notify();
}

static VOID tbox_pm_4g_handle_deepreset(VOID)
{
    TBOX_PM_4G_DEEPRESET_STATE state = (TBOX_PM_4G_DEEPRESET_STATE)tbox_pm_4g_deepreset_state;

    MODULE_LOG_D(TBOXPM, "handle deepreset, state:%d", state);

    switch(state)
    {
        case TBOX_PM_4G_DEEPRESET_STATE_SHUTDOWN:
            {
                drv_pin_set_level(PIN_PWRKEY_MPU, 1U);
                tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_WAIT_SHUTDOWN;
                stimer_start(tbox_pm_4g_do_timer_id, TBOX_PM_4G_SHUTDOWN_TIME);          
            }
            break;

        case TBOX_PM_4G_DEEPRESET_STATE_WAIT_SHUTDOWN:
            {
                drv_pin_set_level(PIN_PWRKEY_MPU, 0U);
                tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_POWEROFF;
                stimer_start(tbox_pm_4g_do_timer_id, TBOX_PM_4G_DEEPRESET_SHUTDOWN_TIME);
            }
            break;

        case TBOX_PM_4G_DEEPRESET_STATE_POWEROFF:
            {
                 drv_pin_set_level(PIN_POWER_MPU, 0U);
                 tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_POWERON;
                 stimer_start(tbox_pm_4g_do_timer_id, TBOX_PM_4G_DEEPRESET_POWEROFF_TIME);
            }
            break;

        case TBOX_PM_4G_DEEPRESET_STATE_POWERON:
            {
                drv_pin_set_level(PIN_POWER_MPU, 1U);
                tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_STARTUP;
                stimer_start(tbox_pm_4g_do_timer_id, TBOX_PM_4G_DEEPRESET_POWERON_TIME);
            }
            break;

        case TBOX_PM_4G_DEEPRESET_STATE_STARTUP:
            {
                 drv_pin_set_level(PIN_PWRKEY_MPU, 1U);
                 tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_WAIT_STARTUP;
                 stimer_start(tbox_pm_4g_do_timer_id, TBOX_PM_4G_STARTUP_TIME);
            }
            break;

        case TBOX_PM_4G_DEEPRESET_STATE_WAIT_STARTUP:
            {
                drv_pin_set_level(PIN_PWRKEY_MPU, 0U);
                tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_FINISH;
                stimer_start(tbox_pm_4g_do_timer_id, TBOX_PM_4G_DEEPRESET_STARTUP_TIME);
            }
            break;

        case TBOX_PM_4G_DEEPRESET_STATE_FINISH:
            {
                if_4g_reset();
                tbox_pm_4g_do_action = (UINT8)TBOX_PM_4G_DO_NOTHING;
                tbox_pm_4g_deepreset_state = (UINT8)TBOX_PM_4G_DEEPRESET_STATE_NONE;
                tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_WAIT_STARTUP4G;
                tbox_pm_4g_check_startup_count = 0U;
                MODULE_LOG_I(TBOXPM, "deepreset 4g success");
            }
            break;

        default:
            break;
    }
}

static VOID tbox_pm_4g_handle_check_startup(VOID)
{
    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);    
    TBOX_PM_4G_CHECK_STARTUP_STATE state = (TBOX_PM_4G_CHECK_STARTUP_STATE)tbox_pm_4g_check_startup_state;
    UINT32 count = tbox_pm_4g_check_startup_count;
    xSemaphoreGive(tbox_pm_4g_mutex);

    switch(state)
    {
        case TBOX_PM_4G_CHECK_STARTUP_STATE_NONE:
            break;

        case TBOX_PM_4G_CHECK_STARTUP_STATE_WAIT_STARTUP4G:
            {
                UINT8 state = if_4g_get_state();
                if(count >= TBOX_PM_4G_CHECK_STARTUP_TIME)
                {
                    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);  
                    if((UINT8)TBOX_4G_STATE_RUNNING == state ||
                       (UINT8)TBOX_4G_STATE_STOPPING == state)
                    {
                        tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_NONE;
                        tbox_pm_4g_check_startup_count = 0U;
                        xSemaphoreGive(tbox_pm_4g_mutex);
                        MODULE_LOG_I(TBOXPM, "stop 4g startup check, because 4g state:%d", state);
                        break;
                    }
                    tbox_pm_4g_check_startup_count = 0U;
                    xSemaphoreGive(tbox_pm_4g_mutex);

                    MODULE_LOG_E(TBOXPM, "wait 4g startup timeout and retry");
                    tbox_pm_4g_do_startup();
                }
                else
                {
                    xSemaphoreTake(tbox_pm_4g_mutex, portMAX_DELAY);  
                    if((UINT8)TBOX_4G_STATE_RUNNING == state ||
                       (UINT8)TBOX_4G_STATE_STOPPING == state)
                    {
                        tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_NONE;
                        tbox_pm_4g_check_startup_count = 0U;
                        xSemaphoreGive(tbox_pm_4g_mutex);
                        MODULE_LOG_I(TBOXPM, "stop 4g startup check, because 4g state:%d", state);
                        break;
                    }
                    tbox_pm_4g_check_startup_count++;
                    xSemaphoreGive(tbox_pm_4g_mutex);
                    MODULE_LOG_I(TBOXPM, "4g wait startup[%u S]", tbox_pm_4g_check_startup_count);
                }
            }
            break;

        default:
            break;
    }
}

static VOID tbox_pm_4g_save_last_action(VOID)
{
    TBOX_PM_4G_MGR_LAST_ACTION last_action;
    last_action.magic_no = TBOX_PM_4G_LAST_ACTION_MAGIC_NO;
    last_action.action = (UINT16)tbox_pm_get_sleeppost_action();
    tbox_cfg_setkv(TBOX_PM_4G_LAST_ACTION_NAME, &last_action, sizeof(TBOX_PM_4G_MGR_LAST_ACTION));
}

static UINT16 tbox_pm_4g_load_last_action(VOID)
{
    TBOX_PM_4G_MGR_LAST_ACTION last_action;
    tbox_cfg_getkv(TBOX_PM_4G_LAST_ACTION_NAME, &last_action, sizeof(TBOX_PM_4G_MGR_LAST_ACTION));
    if(last_action.magic_no != TBOX_PM_4G_LAST_ACTION_MAGIC_NO)
    {
        last_action.magic_no = TBOX_PM_4G_LAST_ACTION_MAGIC_NO;
        last_action.action = TBOX_PM_SLEEPPOST_ACTION_NONE;
        tbox_cfg_setkv(TBOX_PM_4G_LAST_ACTION_NAME, &last_action, sizeof(TBOX_PM_4G_MGR_LAST_ACTION));
        return (UINT16)TBOX_PM_SLEEPPOST_ACTION_NONE;
    }

    return last_action.action;
}

static VOID tbox_pm_4g_init_do_startup(VOID)
{
#define RCM_RESET_STATUS_POR_RST (1U << 16U)

    UINT32 reset_state = Rcm_Hal_GetResetStatus();
    Rcm_Hal_ClearResetStatus();
    if(RCM_RESET_STATUS_POR_RST == (reset_state&RCM_RESET_STATUS_POR_RST)) /*上电冷启动*/
    {
        tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_NONE;
        tbox_pm_4g_do_startup();
        MODULE_LOG_I(TBOXPM, "start up 4g");
    }
    else
    {
        UINT16 last_action = tbox_pm_4g_load_last_action();
        if(TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN == last_action) /*上次动作是关机*/
        {
            tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_NONE;
            tbox_pm_4g_do_startup();
            MODULE_LOG_I(TBOXPM, "start up 4g");            
        }
        else
        {
            tbox_pm_4g_check_startup_state = (UINT8)TBOX_PM_4G_CHECK_STARTUP_STATE_WAIT_STARTUP4G;
            tbox_pm_4g_check_startup_count = 0U;
            MODULE_LOG_I(TBOXPM, "check 4g startup");
        }
    }
}
