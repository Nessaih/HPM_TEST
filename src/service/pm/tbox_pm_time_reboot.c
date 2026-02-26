#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "tbox_pm_if.h"
#include "time_if.h"
#include "tbox_pm_io.h"
#include "tbox_pm_time_reboot.h"

#define TBOX_PM_TIME_STATE_IDLE (0UL)
#define TBOX_PM_TIME_STATE_START (1UL)
#define TBOX_PM_TIME_STATE_FINISH (2UL)

unsigned short tbox_pm_time_reboot_state;
static SemaphoreHandle_t tbox_pm_time_reboot_mutex = NULL_PTR;

static VOID tbox_pm_time_state_set(unsigned short state)
{
    xSemaphoreTake(tbox_pm_time_reboot_mutex, portMAX_DELAY);
    tbox_pm_time_reboot_state = state;
    xSemaphoreGive(tbox_pm_time_reboot_mutex);
}

static unsigned short tbox_pm_time_state_get(VOID)
{
    unsigned short state = TBOX_PM_TIME_STATE_IDLE;
    xSemaphoreTake(tbox_pm_time_reboot_mutex, portMAX_DELAY);
    state = tbox_pm_time_reboot_state;
    xSemaphoreGive(tbox_pm_time_reboot_mutex);
    return state;
}

static VOID tbox_pm_time_reboot_handle_idle(VOID)
{
    /* ACC关闭，主电激活时，才会触发半夜重启*/
    if (TRUE == tbox_pm_io_acc_is_active())
    {
        return;
    }

    if (FALSE == tbox_pm_io_mainpower_is_active())
    {
        return;
    }

    /* 如果重启使能，才会触发重启*/
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(RTCENABLE, cfg_id);
    int enable = 0; /* 默认为使能*/
    tbox_cfg_read(cfg_id, (void*)&enable);
    if (0 == enable)
    {
        MODULE_LOG_W(TBOXPM, "time reboot disable");
        return;
    }

    /* 已经重启过，无需再次重启*/
    if (TRUE == tbox_pm_is_reboot_same_date())
    {
        MODULE_LOG_W(TBOXPM, "time reboot same date");
        return;
    }

    /* 只有在固定时间点才会触发重启*/
    DEV_TIME time;
    time_if_get(&time);
    unsigned short reboot_min = time_if_reboot_time();
    unsigned short cur_min = time.hour * 60 + time.min;

    if (cur_min < reboot_min || (time.hour != (reboot_min / 60)))
    {
        return;
    }

    UINT32 wake_source = tbox_pm_io_get_wakesrc();
    if ((wake_source & (1U << PM_WAKE_SOURCE_RTC)) != 0U)
    {
        MODULE_LOG_E(TBOXPM, "time reboot rtc wake source");
        tbox_pm_time_state_set(TBOX_PM_TIME_STATE_START);
    }
    else
    {
        MODULE_LOG_E(TBOXPM, "time reboot wake source %u", wake_source);
        tbox_pm_time_state_set(TBOX_PM_TIME_STATE_START);
    }
}

VOID tbox_pm_time_reboot_handle_start(VOID)
{
    if (TBOX_E_OK != tbox_pm_reboot(TBOX_PM_REBOOT_4G_MCU))
    {
        tbox_pm_time_state_set(TBOX_PM_TIME_STATE_IDLE);
    }
    else
    {
        MODULE_LOG_W(TBOXPM, "time reboot 4g and mcu start");
        tbox_pm_time_state_set(TBOX_PM_TIME_STATE_FINISH);
    }
}

static VOID tbox_pm_time_reboot_handle_finish(VOID)
{
    /* do nothing */
}

VOID tbox_pm_time_reboot_init(VOID)
{
    tbox_pm_time_reboot_state = TBOX_PM_TIME_STATE_IDLE;
    tbox_pm_time_reboot_mutex = xSemaphoreCreateMutex();
    if (NULL_PTR == tbox_pm_time_reboot_mutex)
    {
        MODULE_LOG_E(TBOXPM, "time reboot mutex create failed");
    }
}

VOID tbox_pm_time_reboot_stop(VOID)
{
    tbox_pm_time_state_set(TBOX_PM_TIME_STATE_IDLE);
}

VOID tbox_pm_time_reboot_period(VOID)
{
    unsigned short state = tbox_pm_time_state_get();
    switch (state)
    {
    case TBOX_PM_TIME_STATE_IDLE:
        tbox_pm_time_reboot_handle_idle();
        break;
    case TBOX_PM_TIME_STATE_START:
        tbox_pm_time_reboot_handle_start();
        break;
    case TBOX_PM_TIME_STATE_FINISH:
        tbox_pm_time_reboot_handle_finish();
        break;
    default:
        break;
    }
}
