#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_pm_io.h"
#include "gnss_if.h"
#include "time_if.h"
#include "analog_if.h"
#include "can_if.h"
#include "hpm_dev.h"
#include "hpm_pack.h"
#include "hpm_data.h"
#include "hpm_session.h"
#include "hpm_param_fetch.h"
#include "tbox_cfg_if.h"
#include "hpm_cfg.h"

#define HPM_DEV_WAIT_FIX (50U) // 50s

typedef enum
{
    HPM_DATA_LOCATION = 0x01,
    HPM_DATA_FAULT = 0x02,
    HPM_DATA_CFG_FACTH = 0x03,
} hpm_dev_data_t;

static VOID hpm_dev_handle_init(VOID);
static VOID hpm_dev_handle_accon(VOID);
static VOID hpm_dev_handle_accoff(VOID);
static VOID hpm_dev_handle_idle(VOID);
static VOID hpm_dev_handle_wakeup(VOID);

static VOID hpm_dev_status_set(hpm_dev_status_e status);
static hpm_dev_status_e hpm_dev_status_get(VOID);

static VOID (*hpm_dev_handler[HPM_DEV_STATUS_MAX])(VOID) = {
    hpm_dev_handle_init,
    hpm_dev_handle_accon,
    hpm_dev_handle_accoff,
    hpm_dev_handle_idle,
    hpm_dev_handle_wakeup,
};

static hpm_dev_status_e hpm_dev_status;
static UINT32 hpm_dev_tick;
static SemaphoreHandle_t hpm_dev_mutex;

VOID hpm_dev_init(UINT8 seq)
{
    if (seq == MODULE_INIT_SEQ_MODULE)
    {
        hpm_dev_status = HPM_DEV_STATUS_INIT;
        hpm_dev_tick = 0;
        hpm_dev_mutex = xSemaphoreCreateMutex();
    }
}

VOID hpm_dev_deinit(void)
{
    hpm_dev_status = HPM_DEV_STATUS_MAX;
    hpm_dev_tick = 0;
    if (NULL_PTR != hpm_dev_mutex)
    {
        vSemaphoreDelete(hpm_dev_mutex);
    }
}

VOID hpm_dev_wakeup(void)
{
    hpm_dev_status_set(HPM_DEV_STATUS_WAKEUP);
    hpm_dev_tick = time_if_get_systick_ms();
}

VOID hpm_dev_sleep(void)
{
    hpm_dev_status_set(HPM_DEV_STATUS_INIT);
}

VOID hpm_dev_process(void)
{
    hpm_dev_status_e status = hpm_dev_status_get();
    if (status < HPM_DEV_STATUS_MAX)
    {
        hpm_dev_handler[status]();
    }
}

static UINT32 hpm_get_devinfo(void)
{
    UINT32 devinfo = 0;

    // TODO for more dev info

    // bit 0: acc active
    if (tbox_pm_io_acc_is_active())
    {
        devinfo |= (1U << 0);
    }

    // bit 8: lte ant status
    ant_status_t lte_ant_status = analog_lte_ant_status();
    if (ANT_OPEN == lte_ant_status)
    {
        devinfo |= (1U << 8);
    }
    else if (ANT_SHORT == lte_ant_status)
    {
        devinfo |= (1U << 9);
    }

    // bit 9: gps ant status
    ant_status_t gps_ant_status = analog_gps_ant_status();
    if (ANT_OPEN == gps_ant_status)
    {
        devinfo |= (1U << 10);
    }
    else if (ANT_SHORT == gps_ant_status)
    {
        devinfo |= (1U << 11);
    }

    for (UINT8 i = 0; i < DRV_CAN_INS_COUNT; i++)
    {
        uint8_t can_status = can_if_state_get(i);
        switch (can_status)
        {
        case CAN_INSTANCE_BUSY:
            devinfo |= (1U << (12 + 2 * i));
            break;

        case CAN_INSTANCE_ERROR:
            devinfo |= (2U << (12 + 2 * i));
            break;

        case CAN_INSTANCE_OFF:
            devinfo |= (3U << (12 + 2 * i));
            break;
        default:
            break;
        }
    }

    return devinfo;
}

static INT32 hpm_get_position_data(UINT8 *buf)
{
    INT32 len = 0;
    UINT8 *data_len = buf;
    buf[len++] = 0;
    buf[len++] = 0;

    UINT8 status = 0;
    GNSS_POSITION_DATA pos;
    gnss_get_position(&pos);

    if (GNSS_POS_STATE_FIX != gnss_get_fix_state())
    {
        status = 0x01;
    }
    status |= (pos.is_north ? 0 : 1U << 1);
    status |= (pos.is_east ? 0 : 1U << 2);

    buf[len++] = status;

    UINT32 longitude = (UINT32)(pos.longitude * 1000000);
    buf[len++] = (UINT8)(longitude >> 24);
    buf[len++] = (UINT8)(longitude >> 16);
    buf[len++] = (UINT8)(longitude >> 8);
    buf[len++] = (UINT8)(longitude >> 0);

    UINT32 latitude = (UINT32)(pos.latitude * 1000000);
    buf[len++] = (UINT8)(latitude >> 24);
    buf[len++] = (UINT8)(latitude >> 16);
    buf[len++] = (UINT8)(latitude >> 8);
    buf[len++] = (UINT8)(latitude >> 0);

    UINT16 speed = (UINT16)(gnss_get_speed() * 10);
    buf[len++] = (UINT8)(speed >> 8);
    buf[len++] = (UINT8)(speed >> 0);

    UINT8 direction = (UINT8)(gnss_get_direction() / 2);
    buf[len++] = direction;

    UINT16 altitude = (UINT16)(gnss_get_altitude() + 1000);
    buf[len++] = (UINT8)(altitude >> 8);
    buf[len++] = (UINT8)(altitude >> 0);

    // acctime
    UINT32 acctime = hpm_cfg_get_run_acc_time();
    buf[len++] = (UINT8)(acctime >> 24);
    buf[len++] = (UINT8)(acctime >> 16);
    buf[len++] = (UINT8)(acctime >> 8);
    buf[len++] = (UINT8)(acctime >> 0);

    // odometer
    UINT32 odometer = hpm_cfg_get_run_gps_odo();
    buf[len++] = (UINT8)(odometer >> 24);
    buf[len++] = (UINT8)(odometer >> 16);
    buf[len++] = (UINT8)(odometer >> 8);
    buf[len++] = (UINT8)(odometer >> 0);

    UINT32 devinfo = hpm_get_devinfo();
    buf[len++] = (UINT8)(devinfo >> 24);
    buf[len++] = (UINT8)(devinfo >> 16);
    buf[len++] = (UINT8)(devinfo >> 8);
    buf[len++] = (UINT8)(devinfo >> 0);

    UINT16 main_vol = (UINT16)(analog_pwr_vtg() / 10);
    buf[len++] = (UINT8)(main_vol >> 8);
    buf[len++] = (UINT8)(main_vol >> 0);

    UINT16 bat_vol = (UINT16)(analog_bat_vtg() / 10);
    buf[len++] = (UINT8)(bat_vol >> 8);
    buf[len++] = (UINT8)(bat_vol >> 0);

    data_len[0] = (UINT8)((len - 2) >> 8);
    data_len[1] = (UINT8)((len - 2) >> 0);

    return len;
}

static INT32 hpm_get_can_data(UINT8 *buf, INT32 remain_size)
{
    INT32 len = 0;
    buf[len++] = HPM_DATA_CFG_FACTH;
    DEV_TIME time;
    time_if_get(&time);
    buf[len++] = (UINT8)(time.year);
    buf[len++] = (UINT8)(time.month);
    buf[len++] = (UINT8)(time.day);
    buf[len++] = (UINT8)(time.hour);
    buf[len++] = (UINT8)(time.min);
    buf[len++] = (UINT8)(time.sec);

    remain_size -= len;

    INT32 ret = hpm_param_fetch_report(buf + len, remain_size);

    if (ret < 0)
    {
        MODULE_LOG_E(HPM, "can data overflow");
        return -1;
    }

    len += ret;

    return len;
}

static INT32 hpm_get_location_data(UINT8 *buf)
{
    INT32 len = 0;
    buf[len++] = HPM_DATA_LOCATION;
    DEV_TIME time;
    time_if_get(&time);
    buf[len++] = (UINT8)(time.year);
    buf[len++] = (UINT8)(time.month);
    buf[len++] = (UINT8)(time.day);
    buf[len++] = (UINT8)(time.hour);
    buf[len++] = (UINT8)(time.min);
    buf[len++] = (UINT8)(time.sec);

    len += hpm_get_position_data(buf + len);

    return len;
}

static INT32 hpm_make_report_pack(UINT8 *data)
{
    UINT8 count = 0;
    UINT16 len = 0;
    len += hpm_sesion_get_data_seq(data + len);
    UINT8 *ptr_count = &data[len];
    data[len++] = 0x00;
    len += hpm_get_location_data(data + len);
    count++;

    INT32 remain_size = HPM_PACK_BUFF_LEN - len;
    INT32 ret = hpm_get_can_data(data + len, remain_size);
    if (ret < 0)
    {
        MODULE_LOG_E(HPM, "get can data failed.");
    }
    else
    {
        count++;
        len += ret;
    }

    *ptr_count = count;
    return len;
}

VOID hpm_dev_report(VOID)
{
    uint8_t *buf = mempool_alloc(HPM_PACK_BUFF_LEN);
    if (NULL_PTR == buf)
    {
        MODULE_LOG_E(HPM, "malloc data buf failed");
        return;
    }

    INT32 len = hpm_make_report_pack(buf);
    if (len <= 0)
    {
        MODULE_LOG_E(HPM, "get real data failed");
        mempool_free(buf);
        return;
    }

    if (len > 0)
    {
        MODULE_LOG_DUMP(HPM, "hpm dev report data", buf, len);
        hpm_data_save_to_realtm_list(HPM_CMD_LIVE_DATA, buf, len);
    }
    mempool_free(buf);
}

VOID hpm_dev_handle_event(hpm_dev_event_e event)
{
    switch (event)
    {
    case HPM_DEV_EVENT_SHELL:
        MODULE_LOG_I(HPM, "shell trig report");
        break;
    case HPM_DEV_EVENT_ACCOFF:
        MODULE_LOG_I(HPM, "accoff trig report");
        break;
    case HPM_DEV_EVENT_ACCON:
        MODULE_LOG_I(HPM, "accon trig report");
        break;
    case HPM_DEV_EVENT_CYCLE:
        MODULE_LOG_I(HPM, "cycle trig report");
        break;
    case HPM_DEV_EVENT_CALL:
        MODULE_LOG_I(HPM, "call trig report");
        break;
    case HPM_DEV_EVENT_WAKEUP:
        MODULE_LOG_I(HPM, "wakeup trig report");
        break;
    default:
        break;
    }
    hpm_dev_tick = time_if_get_systick_ms();
    hpm_dev_report();
}

static VOID hpm_dev_status_set(hpm_dev_status_e status)
{
    switch (status)
    {
    case HPM_DEV_STATUS_INIT:
        MODULE_LOG_I(HPM, "hpm init status");
        break;
    case HPM_DEV_STATUS_ACCON:
        MODULE_LOG_I(HPM, "hpm accon status");
        break;
    case HPM_DEV_STATUS_ACCOFF:
        MODULE_LOG_I(HPM, "hpm accoff status");
        break;
    case HPM_DEV_STATUS_IDLE:
        MODULE_LOG_I(HPM, "hpm idle status");
        break;
    case HPM_DEV_STATUS_WAKEUP:
        MODULE_LOG_I(HPM, "hpm wakeup status");
        break;
    default:
        break;
    }
    xSemaphoreTake(hpm_dev_mutex, portMAX_DELAY);
    hpm_dev_status = status;
    xSemaphoreGive(hpm_dev_mutex);
}

static hpm_dev_status_e hpm_dev_status_get(VOID)
{
    hpm_dev_status_e status = HPM_DEV_STATUS_INIT;
    xSemaphoreTake(hpm_dev_mutex, portMAX_DELAY);
    status = hpm_dev_status;
    xSemaphoreGive(hpm_dev_mutex);
    return status;
}

static VOID hpm_dev_handle_init(VOID)
{
    if (0 == hpm_dev_tick)
    {
        hpm_dev_tick = time_if_get_systick_ms();
    }

    if (tbox_pm_io_acc_is_active())
    {
        hpm_dev_status_set(HPM_DEV_STATUS_ACCON);
    }
}

static VOID hpm_dev_handle_accon(VOID)
{
    if (!tbox_pm_io_acc_is_active())
    {
        hpm_dev_status_set(HPM_DEV_STATUS_ACCOFF);
        return;
    }

    if ((time_if_get_systick_ms() - hpm_dev_tick) >= (UINT32)(HPM_DEV_WAIT_FIX * 1000))
    {
        hpm_dev_handle_event(HPM_DEV_EVENT_ACCON);
        hpm_dev_status_set(HPM_DEV_STATUS_IDLE);
    }
}

static VOID hpm_dev_handle_accoff(VOID)
{
    if (tbox_pm_io_acc_is_active())
    {
        hpm_dev_status_set(HPM_DEV_STATUS_ACCON);
        return;
    }

    if ((time_if_get_systick_ms() - hpm_dev_tick) >= (UINT32)(HPM_DEV_WAIT_FIX * 1000))
    {
        hpm_dev_handle_event(HPM_DEV_EVENT_ACCOFF);
        hpm_dev_status_set(HPM_DEV_STATUS_INIT);
    }
}

static VOID hpm_dev_handle_idle(VOID)
{
    if (!tbox_pm_io_acc_is_active())
    {
        hpm_dev_handle_event(HPM_DEV_EVENT_ACCOFF);
        hpm_dev_status_set(HPM_DEV_STATUS_INIT);
        return;
    }

    UINT32 interval = 60;
    hpm_cfg_get_report_intv(&interval, sizeof(interval));
    if ((time_if_get_systick_ms() - hpm_dev_tick + 50) >= (UINT32)(interval * 1000))
    {
        hpm_dev_handle_event(HPM_DEV_EVENT_CYCLE);
    }
}

static VOID hpm_dev_handle_wakeup(VOID)
{
    if (tbox_pm_io_acc_is_active())
    {
        hpm_dev_status_set(HPM_DEV_STATUS_ACCON);
        return;
    }

    if ((time_if_get_systick_ms() - hpm_dev_tick) >= (UINT32)(HPM_DEV_WAIT_FIX * 1000))
    {
        hpm_dev_handle_event(HPM_DEV_EVENT_WAKEUP);
        hpm_dev_status_set(HPM_DEV_STATUS_INIT);
    }
}
