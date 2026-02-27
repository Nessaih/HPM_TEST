#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_pm_if.h"
#include "tbox_pm_inner.h"
#include "tbox_pm_4g_mgr.h"
#include "tbox_pm_action.h"
#include "tbox_pm_io.h"
#include "tbox_pm_state.h"
#include "tbox_config.h"
#include "tbox_mode_if.h"
#include "tbox_cfg_if.h"
#include "stimer.h"
#include "4g_if.h"
#include "dev_time.h"
#include "driver.h"
#include "delay.h"

#define TBOX_PM_ACTION_MAX_COUNT  (3U)
#define TBOX_PM_ACTION_ABORT      (1U)
#define TBOX_PM_ACTION_NOABORT    (0U)
#define TBOX_PM_ACTION_MCURESET_MAGIC_NO (0x5A5A5A5AU)
#define TBOX_PM_ACTION_4GMCURESET_MAGIC_NO (0xA55A5A5AU)
#define TBOX_PM_4GMCUREST_CFG_NAME "4GMCURESET_COUNT"

typedef enum
{
    TBOX_PM_ACTION_NONE = 0U,
    TBOX_PM_ACTION_SHUTDOWN,
    TBOX_PM_ACTION_4G_RESET,
    TBOX_PM_ACTION_4G_DEEP_RESET,
    TBOX_PM_ACTION_4G_MCU_RESET
}TBOX_PM_ACTION_STATE;

typedef struct
{
    UINT32 magic_no;
    UINT8 data[3];
    UINT8 count;
}TBOX_PM_REBOOT_INFO;

static VOID tbox_pm_power_off(BOOL mpu_power_on);
static VOID tbox_pm_power_on(VOID);
static VOID tbox_pm_action_wakeup(VOID);
static VOID tbox_pm_action_sleep(VOID);
static VOID tbox_pm_action_listen(VOID);
static INT32 tbox_pm_action_do_shutdown(VOID);
static INT32 tbox_pm_action_do_mcureset(VOID);
static INT32 tbox_pm_action_do_4greset(VOID);
static INT32 tbox_pm_action_do_4gdeepreset(VOID);
static INT32 tbox_pm_action_do_4g_mcureset(VOID);

static UINT32 tbox_pm_action_timer_count;
static UINT8 tbox_pm_action_state;
static UINT8 tbox_pm_action_shutdown_count;
static UINT8 tbox_pm_action_4greset_count;
static UINT8 tbox_pm_action_abort_flag; /*只有shutdown和sleep支持abort*/
static TBOX_PM_REBOOT_INFO tbox_pm_4gmcu_reboot_info;

VOID tbox_pm_action_init(VOID)
{
    DEV_TIME time;

    tbox_pm_action_abort_flag = TBOX_PM_ACTION_NOABORT;
    tbox_pm_action_state = (UINT8)TBOX_PM_ACTION_NONE;
    tbox_pm_action_timer_count = 0U;
    tbox_pm_action_shutdown_count = 0U;
    tbox_pm_action_4greset_count = 0U;

    dev_time_get(&time);
    tbox_cfg_getkv(TBOX_PM_4GMCUREST_CFG_NAME, &tbox_pm_4gmcu_reboot_info, sizeof(TBOX_PM_REBOOT_INFO));
    if(TBOX_PM_ACTION_4GMCURESET_MAGIC_NO != tbox_pm_4gmcu_reboot_info.magic_no)
    {
        tbox_pm_4gmcu_reboot_info.magic_no = TBOX_PM_ACTION_4GMCURESET_MAGIC_NO;
        tbox_pm_4gmcu_reboot_info.count = 0U;
        tbox_pm_4gmcu_reboot_info.data[0] = (UINT8)time.year;
        tbox_pm_4gmcu_reboot_info.data[1] = (UINT8)time.month;
        tbox_pm_4gmcu_reboot_info.data[2] = (UINT8)time.day;
        tbox_cfg_setkv(TBOX_PM_4GMCUREST_CFG_NAME, &tbox_pm_4gmcu_reboot_info, sizeof(TBOX_PM_REBOOT_INFO));
    }
}

VOID tbox_pm_action_start(VOID)
{
    tbox_pm_action_shutdown_count = 0U;
    tbox_pm_action_4greset_count = 0U;
}

VOID tbox_pm_action_stop(VOID)
{
    tbox_pm_action_state = (UINT8)TBOX_PM_ACTION_NONE;
    tbox_pm_action_timer_count = 0U;
}

VOID tbox_pm_action_period(VOID)
{
    MODULE_LOG_I(TBOXPM, "pm action period, action state:%d, timer count:%d", tbox_pm_action_state, tbox_pm_action_timer_count);

    if(tbox_pm_action_timer_count > 0U)
    {
        tbox_pm_action_timer_count--;
    }

    switch ((TBOX_PM_ACTION_STATE)tbox_pm_action_state)
    {
        case TBOX_PM_ACTION_SHUTDOWN:
            if(tbox_pm_action_timer_count > 0)
            {
                return;
            }
            if(TBOX_PM_ACTION_ABORT == tbox_pm_action_abort_flag)
            {
                tbox_pm_state_stop();
                tbox_pm_action_abort_flag = TBOX_PM_ACTION_NOABORT;
            }
            else
            {
                tbox_pm_action_shutdown_count++;
                tbox_pm_action_sleep();
            }
            tbox_pm_action_state = (UINT8)TBOX_PM_ACTION_NONE;
            break;

        case TBOX_PM_ACTION_4G_RESET:
        case TBOX_PM_ACTION_4G_DEEP_RESET:
            if(0U == tbox_pm_action_timer_count || 
              TBOX_PM_4G_DO_NOTHING == tbox_pm_4g_get_do_state())
            {
                tbox_pm_action_timer_count = 0U;
                tbox_pm_action_4greset_count++;
                tbox_pm_action_state = (UINT8)TBOX_PM_ACTION_NONE;
            }
            break;

        case TBOX_PM_ACTION_4G_MCU_RESET:
            if(0U == tbox_pm_action_timer_count || 
              TBOX_PM_4G_DO_NOTHING == tbox_pm_4g_get_do_state())
            {
                tbox_pm_action_timer_count = 0U;
                tbox_pm_4gmcu_reboot_info.count++;
                tbox_cfg_setkv(TBOX_PM_4GMCUREST_CFG_NAME, &tbox_pm_4gmcu_reboot_info, sizeof(TBOX_PM_REBOOT_INFO));
                tbox_pm_action_state = (UINT8)TBOX_PM_ACTION_NONE;

                tbox_pm_reboot(TBOX_PM_REBOOT_MCU);
            }
            break;

        default:
            break;
    }
}

VOID tbox_pm_abort_action(VOID)
{
    tbox_pm_action_abort_flag = TBOX_PM_ACTION_ABORT;
}

VOID tbox_pm_clear_abort_action(VOID)
{
    tbox_pm_action_abort_flag = TBOX_PM_ACTION_NOABORT;
}

VOID tbox_pm_force_shutdown(VOID)
{
    tbox_pm_action_sleep();
}

VOID tbox_pm_force_sleep(VOID)
{
    tbox_pm_action_listen();
}

INT32 tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION type)
{
    if((UINT8)TBOX_PM_ACTION_NONE != tbox_pm_action_state)
    {
        MODULE_LOG_W(TBOXPM, "pm action is not none, state:%d", tbox_pm_action_state);
        return (INT32)TBOX_E_IS_BUSY;
    }

    MODULE_LOG_I(TBOXPM, "pm do action, type:%d", type);

    INT32 ret = (INT32)TBOX_E_OK;
    tbox_pm_action_abort_flag = TBOX_PM_ACTION_NOABORT;
    switch(type)
    {
        case TBOX_PM_SLEEPPOST_ACTION_SLEEP:
            tbox_pm_action_listen();
            break;

        case TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN:
            tbox_pm_action_do_shutdown();
            break;

        case TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G:
            ret = tbox_pm_action_do_4greset();
            break;

        case TBOX_PM_SLEEPPOST_ACTION_DEEPREBOOT_4G:
            ret = tbox_pm_action_do_4gdeepreset();
           break;
        
        case TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU:
            ret = tbox_pm_action_do_mcureset();
            break;
        
        case TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU:
            ret = tbox_pm_action_do_4g_mcureset();
            break;

        default:
            break;
    }

    return ret;
}

BOOL tbox_pm_action_is_cando(TBOX_PM_SLEEPPOST_ACTION type)
{
    if((UINT8)TBOX_PM_ACTION_NONE != tbox_pm_action_state)
    {
        return FALSE;
    }

    INT32 ret = TRUE;
    switch(type)
    {
        case TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G:
        case TBOX_PM_SLEEPPOST_ACTION_DEEPREBOOT_4G:
            {
                if(tbox_pm_action_4greset_count >= 2*TBOX_PM_ACTION_MAX_COUNT)
                {
                    MODULE_LOG_W(TBOXPM, "the reset 4g count has reached max:%d", 2*TBOX_PM_ACTION_MAX_COUNT);
                    ret = FALSE;
                }
            }
            break;
                    
        case TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU:
            /*不限制MCU重启次数*/
            break;
        
        case TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU:
            {
                DEV_TIME time;
                dev_time_get(&time);
                if(time.year == tbox_pm_4gmcu_reboot_info.data[0] &&
                    time.month == tbox_pm_4gmcu_reboot_info.data[1] &&
                    time.day == tbox_pm_4gmcu_reboot_info.data[2] &&
                    tbox_pm_4gmcu_reboot_info.count >= TBOX_PM_ACTION_MAX_COUNT)
                {
                    MODULE_LOG_W(TBOXPM, "the reset mcu&4g count has reached max:%d in one day", TBOX_PM_ACTION_MAX_COUNT);
                    ret = FALSE;
                }
            }
            break;

        default:
            break;
    }

    return (BOOL)ret;
}

VOID tbox_pm_action_cleanresetinfo(VOID)
{
    DEV_TIME time;
    dev_time_get(&time);    
    tbox_pm_action_4greset_count = 0U;
    tbox_pm_action_shutdown_count = 0U;

    tbox_pm_4gmcu_reboot_info.magic_no = TBOX_PM_ACTION_4GMCURESET_MAGIC_NO;
    tbox_pm_4gmcu_reboot_info.count = 0U;
    tbox_pm_4gmcu_reboot_info.data[0] = (UINT8)time.year;
    tbox_pm_4gmcu_reboot_info.data[1] = (UINT8)time.month;
    tbox_pm_4gmcu_reboot_info.data[2] = (UINT8)time.day;
    tbox_cfg_setkv(TBOX_PM_4GMCUREST_CFG_NAME, &tbox_pm_4gmcu_reboot_info, sizeof(TBOX_PM_REBOOT_INFO));
}

static VOID tbox_pm_power_off(BOOL mpu_power_on)
{
    drv_pin_set_level(PIN_POWER_5V0,  0U);
    drv_pin_set_level(PIN_POWER_3V3,  0U);
    drv_pin_set_level(PIN_POWER_1V8,  0U);
    drv_pin_set_level(PIN_POWER_BT,   0U);
    drv_pin_set_level(PIN_POWER_485,  0U);
    drv_pin_set_level(PIN_ENABLE_CAN, 0U);
    drv_pin_set_level(PIN_RESET_GNSS, 0U);

    tbox_pm_io_mpu_power_on(mpu_power_on);
}

static VOID tbox_pm_power_on(VOID)
{
    drv_pin_set_level(PIN_POWER_5V0,  1U);
    drv_pin_set_level(PIN_POWER_3V3,  1U);
    drv_pin_set_level(PIN_POWER_1V8,  1U);
    drv_pin_set_level(PIN_POWER_BT,   1U);
    drv_pin_set_level(PIN_POWER_485,  1U);
    drv_pin_set_level(PIN_ENABLE_CAN, 1U);
    drv_pin_set_level(PIN_RESET_GNSS, 1U);

    tbox_pm_io_mpu_power_on(TRUE);
}

static VOID tbox_pm_action_wakeup(VOID)
{
    UINT32 wake_source = 0U;

    /*上电*/
    tbox_pm_power_on();
    /*清除唤醒源*/
    tbox_pm_io_wake();
    /*启动驱动模块*/
    driver_wake();
    /*启动后处理模块*/
    xTaskResumeAll();
    vTaskDelay(pdMS_TO_TICKS(100U));
    tbox_core_start(FALSE);
    vTaskDelay(pdMS_TO_TICKS(200U));
    tbox_stimer_start();
    tbox_pm_start();
    /*唤醒电源模块*/
    wake_source = tbox_pm_io_get_wakesrc();
    if((wake_source & (1 << PM_WAKE_SOURCE_ACC)) != 0U)
    {
        tbox_pm_wakeup(TBOX_PM_WAKEUP_BY_KEY);
    }
    else if((wake_source & (1 << PM_WAKE_SOURCE_RTC)) != 0U)
    {
        tbox_pm_wakeup(TBOX_PM_WAKEUP_BY_TIMER);
    }
    else if((wake_source & (1 << PM_WAKE_SOURCE_RING)) != 0U)
    {
        tbox_pm_wakeup(TBOX_PM_WAKEUP_BY_4G);
    }
    else if(0u != wake_source)
    {
        tbox_pm_wakeup(TBOX_PM_WAKEUP_BY_PERIPHERAL);
    }
    else
    {
        tbox_pm_wakeup(TBOX_PM_WAKEUP_BY_TIMER);
    }

    CHAR *wakeup_log = "enter running mode.....\r\n";
    tbox_log_raw_output(wakeup_log, strlen(wakeup_log));
}

static VOID tbox_pm_action_sleep(VOID)
{
    TBOX_MODE_TYPE mode = TBOX_MODE_NORMAL;
    tbox_mode_get(&mode);
    CHAR *sleep_log = "enter sleep mode......\r\n";
    tbox_log_raw_output(sleep_log, strlen(sleep_log));
    /*停止后处理模块*/
    tbox_pm_stop();
    tbox_stimer_stop();
    vTaskDelay(pdMS_TO_TICKS(100U));
    tbox_core_stop(FALSE);
    vTaskDelay(pdMS_TO_TICKS(200U));
    vTaskSuspendAll();
    /*停止驱动模块*/
    driver_sleep();
    /*设置唤醒源*/
    UINT32 wake_source;
    if(TBOX_MODE_FACTORY == mode ||
       TBOX_MODE_UNDERVOLTAGE == mode)
    {
        /*工厂模式和低电压模式，只保留ACC唤醒*/
        wake_source = 0U;
        wake_source |= (1 << PM_WAKE_SOURCE_ACC);
    }
    else
    {
        /*4G模块关机，不要RING唤醒*/
        wake_source = 0xFFFFFFFFU;
        wake_source &= ~(1 << PM_WAKE_SOURCE_RING);
    }
    tbox_pm_io_sleep(wake_source);
    /*关闭电源*/
    tbox_pm_power_off(FALSE);
    /*进入休眠模式*/
    drv_spm_sleep();
    /*唤醒处理*/
    tbox_pm_action_wakeup();
}

static VOID tbox_pm_action_listen(VOID)
{
    TBOX_MODE_TYPE mode = TBOX_MODE_NORMAL;
    tbox_mode_get(&mode);
    CHAR *sleep_log = "enter listen mode......\r\n";
    tbox_log_raw_output(sleep_log, strlen(sleep_log));
    /*停止后处理模块*/
    tbox_pm_stop();
    tbox_stimer_stop();
    vTaskDelay(pdMS_TO_TICKS(100U));
    tbox_core_stop(FALSE);
    vTaskDelay(pdMS_TO_TICKS(100U));
    vTaskSuspendAll();
    /*停止驱动模块*/
    driver_sleep();
    /*设置唤醒源*/
    UINT32 wake_source;
    if(TBOX_MODE_FACTORY == mode ||
       TBOX_MODE_UNDERVOLTAGE == mode)
    {
        /*工厂模式和低电压模式，只保留ACC唤醒*/
        wake_source = 0U;
        wake_source |= (1 << PM_WAKE_SOURCE_ACC);
    }
    else
    {
        wake_source = 0xFFFFFFFFU;
    }
    tbox_pm_io_sleep(wake_source);
    /*关闭电源*/
    tbox_pm_power_off(TRUE);
    /*进入休眠模式*/
    drv_spm_sleep();
    /*唤醒处理*/
    tbox_pm_action_wakeup();
}

static  INT32 tbox_pm_action_do_shutdown(VOID)
{
    if(tbox_pm_action_shutdown_count >= TBOX_PM_ACTION_MAX_COUNT)
    {
        MODULE_LOG_E(TBOXPM, "shutdown count has reached max:%d", TBOX_PM_ACTION_MAX_COUNT);
        return (INT32)TBOX_E_FAILED;
    }

    /*进行关机操作后，经过30秒模块才真正关机(EC200x)*/
    tbox_pm_4g_do_shutdown();
    tbox_pm_action_state = (UINT8)TBOX_PM_ACTION_SHUTDOWN;
    tbox_pm_action_timer_count = (TBOX_PM_4G_SHUTDOWN_MAX_TIME+31000U)/1000U; 

    return (INT32)TBOX_E_OK;   
}

static INT32 tbox_pm_action_do_mcureset(VOID)
{
    /*不对MCU重启添加条件限制*/
    NVIC_SystemReset();

    return (INT32)TBOX_E_OK;
}

static INT32 tbox_pm_action_do_4greset(VOID)
{
    if(tbox_pm_action_4greset_count >= 2*TBOX_PM_ACTION_MAX_COUNT)
    {
        MODULE_LOG_E(TBOXPM, "4g reset count has reached max:%d", 2*TBOX_PM_ACTION_MAX_COUNT);
        return (INT32)TBOX_E_FAILED;
    }

    tbox_pm_4g_do_reset();
    tbox_pm_action_state = (UINT8)TBOX_PM_ACTION_4G_RESET;
    tbox_pm_action_timer_count = (TBOX_PM_4G_RESET_MAX_TIME+1000U)/1000U; 

    return (INT32)TBOX_E_OK;
}

static INT32 tbox_pm_action_do_4gdeepreset(VOID)
{
    if(tbox_pm_action_4greset_count >= TBOX_PM_ACTION_MAX_COUNT)
    {
        MODULE_LOG_E(TBOXPM, "4g reset count has reached max:%d", TBOX_PM_ACTION_MAX_COUNT);
        return (INT32)TBOX_E_FAILED;
    }

    tbox_pm_4g_do_deepreset();
    tbox_pm_action_state = (UINT8)TBOX_PM_ACTION_4G_DEEP_RESET;
    tbox_pm_action_timer_count = (TBOX_PM_4G_DEEPRESET_MAX_TIME+1000U)/1000U;
    return (INT32)TBOX_E_OK;         
}

static INT32 tbox_pm_action_do_4g_mcureset(VOID)
{
    DEV_TIME time;
    dev_time_get(&time);

    if(time.year != tbox_pm_4gmcu_reboot_info.data[0] ||
       time.month != tbox_pm_4gmcu_reboot_info.data[1] ||
       time.day != tbox_pm_4gmcu_reboot_info.data[2])
    {
        tbox_pm_4gmcu_reboot_info.count = 0U;
        tbox_pm_4gmcu_reboot_info.data[0] = (UINT8)time.year;
        tbox_pm_4gmcu_reboot_info.data[1] = (UINT8)time.month;
        tbox_pm_4gmcu_reboot_info.data[2] = (UINT8)time.day;
        tbox_cfg_setkv(TBOX_PM_4GMCUREST_CFG_NAME, &tbox_pm_4gmcu_reboot_info, sizeof(TBOX_PM_REBOOT_INFO));
    }
    else
    {
        if(tbox_pm_4gmcu_reboot_info.count >= TBOX_PM_ACTION_MAX_COUNT)
        {
            MODULE_LOG_E(TBOXPM, "4g mcu reset count has reached max:%d", TBOX_PM_ACTION_MAX_COUNT);
            return (INT32)TBOX_E_FAILED;
        }
    }

    tbox_pm_4g_do_deepreset();
    tbox_pm_action_state = (UINT8)TBOX_PM_ACTION_4G_MCU_RESET;
    tbox_pm_action_timer_count = (TBOX_PM_4G_DEEPRESET_MAX_TIME+1000U)/1000U; 
    return (INT32)TBOX_E_OK;        
}

BOOL tbox_pm_is_reboot_same_date(void)
{
    DEV_TIME time;
    dev_time_get(&time);

    if (time.year == tbox_pm_4gmcu_reboot_info.data[0] &&
        time.month == tbox_pm_4gmcu_reboot_info.data[1] &&
        time.day == tbox_pm_4gmcu_reboot_info.data[2])
    {
        if (tbox_pm_4gmcu_reboot_info.count > 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}
