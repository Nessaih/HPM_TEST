#include <string.h>
#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "4g_if.h"
#include "time_if.h"
#include "time_sync.h"
#include "time_backup.h"

#define DEV_TIME_NTP_4GSYNC_TIMEOUT 125
#define DEV_TIME_NTP_RETRY_TIMEOUT  5
#define DEV_TIME_NTP_DELAY_TIMEOUT  12

typedef enum
{
    DEV_TIME_NTP_WAIT_DIALED,
    DEV_TIME_NTP_DELAY_SYNC,
    DEV_TIME_NTP_WAIT_NTP_RESP,
    DEV_TIME_NTP_WAIT_4GSYNC,
    DEV_TIME_NTP_WAIT_CCLK_RESP,
    DEV_TIME_NTP_FINISH
} DEV_TIME_NTP_SYNC_STATE;

extern int dev_rtc_set(const DEV_TIME *time);

static TIME_SYNC_SOURCE dev_time_sync_lasttype;
static UINT8            dev_time_ntp_state;
static UINT8            dev_time_ntp_4g_time_cnt;
static DEV_TIME         dev_time_sync_info;

static void dev_time_sync_ntp_resp(IF_4G_TIMINFO *info);
static void dev_time_sync_cclk_resp(IF_4G_TIMINFO *info);
static void dev_time_sync_ntp(void);
static void dev_time_sync_date_change(void);

static const char *dev_time_ntp_state_to_str(DEV_TIME_NTP_SYNC_STATE state)
{
    switch (state)
    {
    case DEV_TIME_NTP_WAIT_DIALED:
        return "WAIT_DIALED";
    case DEV_TIME_NTP_DELAY_SYNC:
        return "DELAY_SYNC";
    case DEV_TIME_NTP_WAIT_NTP_RESP:
        return "WAIT_NTP_RESP";
    case DEV_TIME_NTP_WAIT_4GSYNC:
        return "WAIT_4GSYNC";
    case DEV_TIME_NTP_WAIT_CCLK_RESP:
        return "WAIT_CCLK_RESP";
    case DEV_TIME_NTP_FINISH:
        return "FINISH";
    default:
        return "UNKNOWN";
    }
}

static void utc_to_local(UINT8 *time, INT32 timezone)
{
    INT32 hour = time[3] + timezone;

    if (hour >= 24)
    {
        hour -= 24;
        time[2]++;
    }
    else if (hour < 0)
    {
        hour += 24;
        if (time[2] > 1)
        {
            time[2]--;
        }
    }

    time[3] = (UINT8)hour;
}

void dev_time_ntp_init(void)
{
    dev_time_ntp_state       = DEV_TIME_NTP_WAIT_DIALED;
    dev_time_ntp_4g_time_cnt = 0;
    dev_time_sync_lasttype   = TIME_SYNC_SOURCE_NONE;
    memset(&dev_time_sync_info, 0, sizeof(dev_time_sync_info));
}

void dev_time_ntp_sleep(void)
{
}

void dev_time_ntp_wakeup(void)
{
    dev_time_sync_lasttype = TIME_SYNC_SOURCE_NONE;
}

static void dev_time_sync_ntp(void)
{
    DEV_TIME_NTP_SYNC_STATE prev_state = (DEV_TIME_NTP_SYNC_STATE)dev_time_ntp_state;

    switch (dev_time_ntp_state)
    {
    case DEV_TIME_NTP_WAIT_DIALED:
        if (dev_time_sync_lasttype >= TIME_SYNC_SOURCE_NTP)
        {
            dev_time_ntp_4g_time_cnt = 0;
            dev_time_ntp_state       = DEV_TIME_NTP_FINISH;
            break;
        }
        if (TRUE == if_4g_is_downloading())
        {
            dev_time_ntp_4g_time_cnt = 0;
            break;
        }
        if (0 == dev_time_ntp_4g_time_cnt)
        {
            if (IF_4G_STATE_CONNECTED == if_4g_get_call_state(IF_4G_PUBLIC_APN))
            {
                dev_time_ntp_4g_time_cnt = DEV_TIME_NTP_DELAY_TIMEOUT;
                dev_time_ntp_state       = DEV_TIME_NTP_DELAY_SYNC;
            }
        }
        else
        {
            dev_time_ntp_4g_time_cnt--;
        }
        break;

    case DEV_TIME_NTP_DELAY_SYNC:
        if (dev_time_sync_lasttype >= TIME_SYNC_SOURCE_NTP)
        {
            dev_time_ntp_4g_time_cnt = 0;
            dev_time_ntp_state       = DEV_TIME_NTP_FINISH;
            break;
        }
        if (TRUE == if_4g_is_downloading())
        {
            dev_time_ntp_4g_time_cnt = 0;
            dev_time_ntp_state       = DEV_TIME_NTP_WAIT_DIALED;
            break;
        }
        if (0 == dev_time_ntp_4g_time_cnt)
        {
            MODULE_LOG_D(TIME, "NTP query time start");
            if_4g_ntp(IF_4G_PUBLIC_APN, (UINT8 *)"pool.ntp.org", 123, dev_time_sync_ntp_resp);
            dev_time_ntp_state = DEV_TIME_NTP_WAIT_NTP_RESP;
        }
        else
        {
            dev_time_ntp_4g_time_cnt--;
        }
        break;

    case DEV_TIME_NTP_WAIT_NTP_RESP:
        if (TRUE == if_4g_is_downloading())
        {
            dev_time_ntp_4g_time_cnt = 0;
            dev_time_ntp_state       = DEV_TIME_NTP_WAIT_DIALED;
            break;
        }
        break;

    case DEV_TIME_NTP_WAIT_4GSYNC:
        if (TRUE == if_4g_is_downloading())
        {
            dev_time_ntp_4g_time_cnt = 0;
            dev_time_ntp_state       = DEV_TIME_NTP_WAIT_DIALED;
            break;
        }
        if (0 == dev_time_ntp_4g_time_cnt)
        {
            if_4g_cclk(dev_time_sync_cclk_resp);
            dev_time_ntp_state = DEV_TIME_NTP_WAIT_CCLK_RESP;
        }
        else
        {
            dev_time_ntp_4g_time_cnt--;
        }
        break;

    case DEV_TIME_NTP_WAIT_CCLK_RESP:
        if (TRUE == if_4g_is_downloading())
        {
            dev_time_ntp_4g_time_cnt = 0;
            dev_time_ntp_state       = DEV_TIME_NTP_WAIT_DIALED;
            break;
        }
        break;

    case DEV_TIME_NTP_FINISH:
        break;

    default:
        break;
    }

    if (prev_state != (DEV_TIME_NTP_SYNC_STATE)dev_time_ntp_state)
    {
        MODULE_LOG_D(TIME,
                     "NTP state change: %s(%d) -> %s(%d)",
                     dev_time_ntp_state_to_str(prev_state),
                     prev_state,
                     dev_time_ntp_state_to_str((DEV_TIME_NTP_SYNC_STATE)dev_time_ntp_state),
                     dev_time_ntp_state);
    }
}

static void dev_time_sync_date_change(void)
{
    DEV_TIME time;

    time_if_get(&time);

    if (dev_time_ntp_state != DEV_TIME_NTP_FINISH)
    {
        return;
    }

    if ((dev_time_sync_info.year != time.year) ||
        (dev_time_sync_info.month != time.month) ||
        (dev_time_sync_info.day != time.day))
    {
        MODULE_LOG_I(TIME, "Date changed, reset NTP state");
        dev_time_ntp_state       = DEV_TIME_NTP_WAIT_DIALED;
        dev_time_ntp_4g_time_cnt = 0;
        memcpy(&dev_time_sync_info, &time, sizeof(DEV_TIME));
    }
}

void dev_time_ntp_timeout(void)
{
    dev_time_sync_date_change();
    dev_time_sync_ntp();
}

TIME_SYNC_SOURCE dev_time_ntp_get_last_type(void)
{
    return dev_time_sync_lasttype;
}

void dev_time_ntp_set_type(TIME_SYNC_SOURCE type)
{
    dev_time_sync_lasttype = type;

    if (dev_time_sync_lasttype < TIME_SYNC_SOURCE_NTP)
    {
        dev_time_ntp_state       = DEV_TIME_NTP_WAIT_DIALED;
        dev_time_ntp_4g_time_cnt = 0;
    }
}

void time_if_set_with_source(TIME_SYNC_SOURCE type, DEV_TIME *time)
{
    if (NULL == time)
    {
        MODULE_LOG_E(TIME, "Invalid parameter");
        return;
    }

    if (!dev_time_check_is_valid(*time))
    {
        return;
    }

    if (TIME_SYNC_SOURCE_GNSS == type || type >= dev_time_sync_lasttype)
    {
        MODULE_LOG_I(TIME, "Sync time from source %s: 20%02u-%02u-%02u %02u:%02u:%02u",
                     time_if_sync_source_to_str(type), time->year, time->month, time->day, time->hour, time->min, time->sec);

        if (TIME_SYNC_SOURCE_RTC != type)
        {
            dev_rtc_set(time);
        }
        dev_time_set(time);

        dev_time_sync_lasttype = type;
        dev_time_set_backuptime(time);
        memcpy(&dev_time_sync_info, time, sizeof(DEV_TIME));
    }
    else
    {
        MODULE_LOG_E(TIME, "Priority lower than last source, skip");
        return;
    }
}

static void dev_time_sync_ntp_resp(IF_4G_TIMINFO *info)
{
    if (NULL == info)
    {
        return;
    }

    if (1 == info->is_valid)
    {
        dev_time_ntp_4g_time_cnt = DEV_TIME_NTP_4GSYNC_TIMEOUT;
        dev_time_ntp_state       = DEV_TIME_NTP_WAIT_4GSYNC;
    }
    else
    {
        MODULE_LOG_E(TIME, "NTP response failed");
        //dev_time_ntp_4g_time_cnt = DEV_TIME_NTP_RETRY_TIMEOUT;
        //dev_time_ntp_state       = DEV_TIME_NTP_WAIT_DIALED;

        /*为节省流量，ntp一天最多执行一次*/
        dev_time_ntp_4g_time_cnt = 0;
        dev_time_ntp_state       = DEV_TIME_NTP_FINISH;
    }
}

static void dev_time_sync_cclk_resp(IF_4G_TIMINFO *info)
{
    INT32    timezone = 8;
    UINT8    local_time[6] = {0};
    DEV_TIME time;
    TBOX_CFG_ID cfg_id;

    if (NULL == info)
    {
        return;
    }

    if (1 == info->is_valid)
    {
        memset(&time, 0, sizeof(time));
        memcpy(local_time, info->time, 6);

        TBOX_CFG_ID_GET(TIMEZONE, cfg_id);
        if (TBOX_E_OK == tbox_cfg_read(cfg_id, &timezone))
        {
            utc_to_local(local_time, timezone);
        }

        time.year  = local_time[0];
        time.month = local_time[1];
        time.day   = local_time[2];
        time.hour  = local_time[3];
        time.min   = local_time[4];
        time.sec   = local_time[5];

        if (dev_time_check_is_valid(time))
        {
            time_if_set_with_source(TIME_SYNC_SOURCE_NTP, &time);
            dev_time_ntp_state = DEV_TIME_NTP_FINISH;
            MODULE_LOG_I(TIME, "NTP set time: 20%02u-%02u-%02u %02u:%02u:%02u, timezone: %d",
                         time.year, time.month, time.day, time.hour, time.min, time.sec, timezone);
        }
        else
        {
            dev_time_ntp_state = DEV_TIME_NTP_FINISH;
            MODULE_LOG_E(TIME, "Invalid NTP time: 20%02u-%02u-%02u %02u:%02u:%02u, timezone: %d",
                         time.year, time.month, time.day, time.hour, time.min, time.sec, timezone);
        }
    }
    else
    {
        MODULE_LOG_E(TIME, "CCLK response failed");
		//dev_time_ntp_4g_time_cnt = DEV_TIME_NTP_RETRY_TIMEOUT;
        //dev_time_ntp_state       = DEV_TIME_NTP_WAIT_DIALED;

        /*为节省流量，ntp一天最多执行一次*/
        dev_time_ntp_4g_time_cnt = 0;
        dev_time_ntp_state       = DEV_TIME_NTP_FINISH;
    }
}

TIME_SYNC_SOURCE time_if_get_sync_source(void)
{
    return dev_time_sync_lasttype;
}

const char *time_if_sync_source_to_str(TIME_SYNC_SOURCE source)
{
    switch (source)
    {
    case TIME_SYNC_SOURCE_NONE:
        return "NONE";
    case TIME_SYNC_SOURCE_RTC:
        return "RTC";
    case TIME_SYNC_SOURCE_SHELL:
        return "SHELL";
    case TIME_SYNC_SOURCE_GNSS:
        return "GNSS";
    case TIME_SYNC_SOURCE_NTP:
        return "NTP";
    case TIME_SYNC_SOURCE_FLASH:
        return "FLASH";
    case TIME_SYNC_SOURCE_TSP:
        return "TSP";
    default:
        return "UNKNOWN";
    }
}

void time_if_update_backuptime(const DEV_TIME *time)
{
    dev_time_set_backuptime((DEV_TIME *)time);
}
