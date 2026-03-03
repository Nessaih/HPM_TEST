#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_pm_io.h"
#include "tbox_cfg_if.h"

#include "hpm_mgr.h"
#include "hpm_socket.h"
#include "hpm_ota_tbox.h"

#define HPM_MGR_ACC_OFF_STOP_COUNT (60UL) // 60s

typedef enum
{
    HPM_MGR_STATUS_IDLE = 0,
    HPM_MGR_STATUS_RUNNING = 1,
    HPM_MGR_STATUS_WAIT_STOP = 2,
    HPM_MGR_STATUS_STOPPED = 3,
    HPM_MGR_STATUS_MAX
} hpm_mgr_status_e;

static VOID hpm_mgr_in_idle(VOID);
static VOID hpm_mgr_in_running(VOID);
static VOID hpm_mgr_in_wait_stop(VOID);
static VOID hpm_mgr_in_stopped(VOID);

static hpm_mgr_status_e hpm_mgr_status;
static UINT32 hpm_mgr_acc_off_count;

static VOID (*hpm_mgr_handle[HPM_MGR_STATUS_MAX])() = {
    hpm_mgr_in_idle,
    hpm_mgr_in_running,
    hpm_mgr_in_wait_stop,
    hpm_mgr_in_stopped,
};

INT32 hpm_mgr_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        hpm_mgr_status = HPM_MGR_STATUS_IDLE;
        hpm_mgr_acc_off_count = 0;
        break;

    case MODULE_INIT_SEQ_STORAGE:
        break;

    case MODULE_INIT_SEQ_MODULE:
        break;

    default:
        break;
    }

    return 0;
}

BOOL hpm_mgr_allow_sleep(VOID)
{
    if ((HPM_MGR_STATUS_IDLE == hpm_mgr_status) ||
        (HPM_MGR_STATUS_STOPPED == hpm_mgr_status))
    {
        return TRUE;
    }
    return FALSE;
}

VOID hpm_mgr_process(VOID)
{
    hpm_mgr_handle[hpm_mgr_status]();
}

VOID hpm_mgr_wakeup(VOID)
{
    hpm_mgr_status = HPM_MGR_STATUS_IDLE;
    hpm_mgr_acc_off_count = 0;
}

static BOOL hpm_mgr_allowed_start(VOID)
{
    BOOL allowed = FALSE;
    if (TRUE == tbox_pm_io_acc_is_active())
    {
        allowed = TRUE;
    }
    else
    {
        UINT32 wake_source = tbox_pm_io_get_wakesrc();

        /*RTC唤醒*/
        if (0 != (wake_source & (1U << PM_WAKE_SOURCE_RTC)))
        {
            allowed = TRUE;
        }

        /*拆除唤醒*/
        if (0 != (wake_source & (1U << PM_WAKE_SOURCE_REMOVED)))
        {
            allowed = TRUE;
        }

        /*开盖唤醒*/
        if (0 != (wake_source & (1U << PM_WAKE_SOURCE_LIGHT)))
        {
            allowed = TRUE;
        }
    }
    return allowed;
}

static VOID hpm_mgr_in_idle(VOID)
{
    if (TRUE == hpm_mgr_allowed_start())
    {
        MODULE_LOG_I(HPM, "allowed start");
        hpm_socket_start();
        hpm_mgr_status = HPM_MGR_STATUS_RUNNING;
        hpm_mgr_acc_off_count = 0;
    }
}

static VOID hpm_mgr_in_running(VOID)
{
    if (TRUE == tbox_pm_io_acc_is_active())
    {
        hpm_mgr_acc_off_count = 0;
        if (TRUE == hpm_socket_in_idle())
        {
            hpm_socket_start();
        }
    }
    else
    {
        hpm_mgr_acc_off_count++;
    }

    MODULE_LOG_I(HPM, "acc off count %d", hpm_mgr_acc_off_count);

    if (hpm_mgr_acc_off_count >= HPM_MGR_ACC_OFF_STOP_COUNT)
    {
        UINT32 sleep_delay = 75UL;
        TBOX_CFG_ID cfg_id;
        TBOX_CFG_ID_GET(SLEEPDELAY, cfg_id);
        tbox_cfg_read(cfg_id, &sleep_delay);
        if (sleep_delay <= HPM_MGR_ACC_OFF_STOP_COUNT)
        {
            hpm_mgr_acc_off_count = 0;
        }
        else
        {
            hpm_mgr_acc_off_count = sleep_delay - hpm_mgr_acc_off_count;
        }
        MODULE_LOG_I(HPM, "begin to stop connect,sleep seconds %d", hpm_mgr_acc_off_count);
        hpm_socket_stop();
        hpm_mgr_status = HPM_MGR_STATUS_WAIT_STOP;
    }
}

static VOID hpm_mgr_in_wait_stop(VOID)
{
    if (TRUE == tbox_pm_io_acc_is_active())
    {
        hpm_mgr_acc_off_count = 0;
    }

    if (hpm_mgr_acc_off_count > 0)
    {
        hpm_mgr_acc_off_count--;
        return;
    }

    if (FALSE == hpm_socket_in_idle())
    {
        MODULE_LOG_I(HPM, "socket not idle, force stop.");
        hpm_socket_force_stop();
    }
    else
    {
        MODULE_LOG_I(HPM, "socket stop completed.");
    }
    hpm_mgr_status = HPM_MGR_STATUS_STOPPED;
}

static VOID hpm_mgr_in_stopped(VOID)
{
    if (TRUE == tbox_pm_io_acc_is_active())
    {
        MODULE_LOG_I(HPM, "acc active, begin to start connect.");
        hpm_socket_start();
        hpm_mgr_acc_off_count = 0;
        hpm_mgr_status = HPM_MGR_STATUS_RUNNING;
    }
}
