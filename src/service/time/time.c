#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "tbox_pm_io.h"
#include "drv_rtc.h"
#include "time_if.h"
#include "time_backup.h"
#include "time_sync.h"
#include "time_shell.h"
#include "tbox_pm_if.h"

#define DEV_RTC_POR_FLAG (0x7e8d7a8b)
#define FIXED_REBOOT_HOUR 3

int dev_rtc_set(const DEV_TIME *time);

static void dev_time_to_rtc(const DEV_TIME *dt, rtc_time_t *rtc)
{
    if (dt == NULL || rtc == NULL) {
        return;
    }
    rtc->year   = dt->year;
    rtc->month  = dt->month;
    rtc->day    = dt->day;
    rtc->hour   = dt->hour;
    rtc->minute = dt->min;
    rtc->second = dt->sec;
}

static void rtc_to_dev_time(const rtc_time_t *rtc, DEV_TIME *dt)
{
    if (rtc == NULL || dt == NULL) {
        return;
    }
    dt->year  = rtc->year;
    dt->month = rtc->month;
    dt->day   = rtc->day;
    dt->hour  = rtc->hour;
    dt->min   = rtc->minute;
    dt->sec   = rtc->second;
    dt->msec  = 0;
}

static int time_module_init(uint8_t seq);
static void time_module_task(void *param);
static void time_stop(void);
static void time_start(void);
TBOX_MODULE_FUN(TIME, time_module_init, time_stop, time_start, NULL_PTR, NULL_PTR, NULL_PTR);
TBOX_RUNLOOP_MODULE(TIME, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_INFO, TBOX_TASK_MEDIUM_STACK_SIZE, time_module_task);
TBOX_MODULE_LOADER(TIME) {}

static TBOX_ID time_module_id;

static TickType_t   basetime_tick         = 0;
static time_t       basetime_utc          = 0;
static bool         dev_time_is_set              = false;
static bool         dev_time_rtc_is_set          = false;
static bool         dev_time_rtc_is_diag         = true;
static int          dev_time_fail_cnt            = 0;
static uint8_t        dev_time_reboot_min = 0;

bool time_if_rtc_is_set(void)
{
    return dev_time_rtc_is_set;
}

bool time_if_is_set(void)
{
    return dev_time_is_set;
}

/**
 *@brief  Set device time
 *@param  time		[in]
 *@retval 0		success
 *        1        fail
 *@date   2021-11-12
 *@author vic
 */
void dev_time_set(const DEV_TIME *time)
{
    struct tm cur_time;

    memset(&cur_time, 0, sizeof(cur_time));

    if (!dev_time_check_is_valid(*time))
    {
        return;
    }

    cur_time.tm_year = time->year + 100; /* from 1900*/
    cur_time.tm_mon  = time->month - 1;  /* from 0 */
    cur_time.tm_mday = time->day;
    cur_time.tm_hour = time->hour;
    cur_time.tm_min  = time->min;
    cur_time.tm_sec  = time->sec;

    basetime_tick   = ((unsigned int)xTaskGetTickCount());
    basetime_utc    = mktime(&cur_time);
    dev_time_is_set = true;
}

/**
 *@brief  Reset device time to default.
 *@param  Mone
 *@retval None
 *@date   2021-11-12
 *@author vic
 */
void dev_time_rst(void)
{
    DEV_TIME time;

    MODULE_LOG_I(TIME, "Reset device time from flash backup");
    dev_time_get_backuptime(&time);
    time_if_set_with_source(TIME_SYNC_SOURCE_FLASH, &time);
    dev_rtc_set(&time);
}

/**
 *@brief  Get device time.
 *@param  Mone
 *@retval None
 *@date   2021-11-12
 *@author vic
 */
time_t time_if_get(DEV_TIME *time)
{
    unsigned int tick;
    time_t       sec;
    struct tm   *cur_time;

    tick     = ((unsigned int)xTaskGetTickCount());
    sec      = basetime_utc + (tick - basetime_tick) / pdMS_TO_TICKS(1000);
    cur_time = localtime(&sec);
    if (NULL == cur_time)
    {
        dev_time_get_backuptime(time);
        return sec;
    }

    if (time)
    {
        time->year  = cur_time->tm_year - 100;
        time->month = cur_time->tm_mon + 1;
        time->day   = cur_time->tm_mday;
        time->hour  = cur_time->tm_hour;
        time->min   = cur_time->tm_min;
        time->sec   = cur_time->tm_sec;
        time->msec  = pdTICKS_TO_MS(tick - basetime_tick) % 1000;
    }
    return sec;
}

/**
 *@brief  Set RTC time
 *@param  time		[in]
 *@retval 0		success
 *        1        fail
 *@date   2021-11-12
 *@author vic
 */
int dev_rtc_set(const DEV_TIME *time)
{
    int ret = 1;
    rtc_time_t rtc;

    if (!dev_time_check_is_valid(*time)) {
        dev_time_rtc_is_set = false;
        return ret;
    }

    dev_time_to_rtc(time, &rtc);
    if (drv_rtc_set_time(&rtc) == RTC_STATUS_SUCCESS) {
        dev_time_rtc_is_set = true;
        ret = 0;
    } else {
        dev_time_rtc_is_set = false;
    }
    return ret;
}

time_t dev_time_get_utc(DEV_TIME *time)
{
    unsigned int tick;
    time_t       sec;
    struct tm   *cur_time;

    tick     = ((unsigned int)xTaskGetTickCount());
    sec      = basetime_utc + (tick - basetime_tick) / pdMS_TO_TICKS(1000) - 8 * 3600;
    cur_time = localtime(&sec);
    if (NULL == cur_time)
    {
        dev_time_get_backuptime(time);
        return sec;
    }

    if (time)
    {
        time->year  = cur_time->tm_year - 100;
        time->month = cur_time->tm_mon + 1;
        time->day   = cur_time->tm_mday;
        time->hour  = cur_time->tm_hour;
        time->min   = cur_time->tm_min;
        time->sec   = cur_time->tm_sec;
        time->msec  = pdTICKS_TO_MS(tick - basetime_tick) % 1000;
    }
    return sec;
}

/**
 *@brief  Get RTC time
 *@param  time		[in]
 *@retval 0		success
 *        1        fail
 *@date   2021-11-12
 *@author vic
 */
int time_if_rtc_get(DEV_TIME *time)
{
    rtc_time_t rtc;

    if (time == NULL) {
        return 1;
    }
    if (drv_rtc_get_time(&rtc) != RTC_STATUS_SUCCESS) {
        return 1;
    }
    rtc_to_dev_time(&rtc, time);
    return 0;
}


void dev_time_retry(void)
{
    // int32_t           ret_reboot;
    // uint32_t          reboot_switch;
    DEV_TIME        time;
    uint32_t          flag;
    // TBOX_CFG_ID cfg_id;

    if (!dev_time_is_set)
    {
        drv_rtc_init();
        TBOX_CFG_ID cfg_id;
        TBOX_CFG_ID_GET(RTCPOR, cfg_id);
        tbox_cfg_read(cfg_id, (unsigned char *)&flag);
        if (DEV_RTC_POR_FLAG != flag && (!drv_rtc_is_startup_pre()))
        {
            dev_time_rst();
            flag = DEV_RTC_POR_FLAG;
            TBOX_CFG_ID cfg_id;
            TBOX_CFG_ID_GET(RTCPOR, cfg_id);
            tbox_cfg_write(cfg_id, (unsigned char *)&flag);
        }
        else
        {
            if (0 == time_if_rtc_get(&time) && dev_time_check_is_valid(time))
            {
                time_if_set_with_source(TIME_SYNC_SOURCE_RTC, &time);
            }
            else
            {
                dev_time_rst();
            }
        }
    }
    else
    {
        time_if_get(&time);
        if (!dev_time_rtc_is_set)
        {   
            if(dev_time_check_is_valid(time))
            {
                dev_rtc_set(&time);
            }
            else
            {
                dev_time_get_backuptime(&time);
                dev_rtc_set(&time);
            }
        }
#if 0
        // key_on turn off, main power on, 3:00--4:00, once a day*/
        if(!tbox_pm_io_acc_is_active()
          && !tbox_pm_io_mainpower_is_active()
          && FIXED_REBOOT_HOUR == time.hour)
        {

            TBOX_CFG_ID_GET(RTCENABLE, cfg_id);
            ret_reboot = tbox_cfg_read(cfg_id, (uint8_t *)&reboot_switch);
            if((0 == ret_reboot) || (0 == reboot_switch))
            {
                return;
            }

            if(1 == dev_reboot_date_is_same())
            {

                return;
            }

            if((tbox_pm_io_get_wakesrc() & (1 << PM_WAKE_SOURCE_RTC)) > 0)
            {
                MODULE_LOG_I(TIME, "timer reboot 4g[rtcwake]...");
                /* TODO: 适配重启函数 */ // dev_do_reboot(DEV_REBOOT_MCU_4G);
            }
            else if(dev_time_reboot_min == time.min)
            {
                MODULE_LOG_I(TIME, "timer reboot 4g...");
                /* TODO: 适配重启函数 */ // dev_do_reboot(DEV_REBOOT_MCU_4G);
            }
            else
            {
                /*NONE*/
            }
        }
#endif
    }
}

unsigned int time_if_get_basetime_tick(void)
{
    return basetime_tick;
}

unsigned int time_if_get_basetime_utc_s(void)
{
    return basetime_utc;
}

void dev_time_sleep(void)
{
    int           ret_wake, ret_reboot;
    int           fixed_min, curr_min;
    int           val;
    uint8_t         reboot_switch;
    time_t        sec;
    struct tm    *alarm_time;
    struct tm    *reboot_time;
    DEV_TIME      time;

    dev_time_save_backuptime();

    dev_time_is_set      = false;
    dev_time_rtc_is_diag = false;

    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(RTCINTV, cfg_id);
    ret_wake   = tbox_cfg_read(cfg_id, (uint8_t *)&val);
    TBOX_CFG_ID_GET(RTCENABLE, cfg_id);
    ret_reboot = tbox_cfg_read(cfg_id, (uint8_t *)&reboot_switch);
    val &= 0x7FFFFFFF;

    time_if_get(&time);
    sec  = basetime_utc + (((unsigned int)xTaskGetTickCount()) - basetime_tick) / pdMS_TO_TICKS(1000);
    fixed_min = FIXED_REBOOT_HOUR * 60 + dev_time_reboot_min;
    curr_min  = time.hour * 60 + time.min;

    if (ret_reboot != TBOX_E_OK || 0 == reboot_switch)
    {
        if (ret_wake != TBOX_E_OK || 0 == val)
        {
            drv_rtc_set_alarm(0, 0, 0);
            return;
        }
        else
        {
            sec        = sec + val * 60; /* the unit of val is minute */
            alarm_time = localtime(&sec);
            if (NULL == alarm_time)
            {
                return;
            }
            drv_rtc_set_alarm(alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
            MODULE_LOG_I(TIME, "1WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
        }
    }
    else
    {
        if (ret_wake != TBOX_E_OK || 0 == val)
        {
            if (time.hour < FIXED_REBOOT_HOUR)
            {
                reboot_time = localtime(&sec);
                if (NULL == reboot_time)
                {
                    return;
                }
                drv_rtc_set_alarm(reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                MODULE_LOG_I(TIME, "2RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
            }
            else if (time.hour == FIXED_REBOOT_HOUR)
            {
                if (time.min < dev_time_reboot_min)
                {
                    if(tbox_pm_is_reboot_same_date())
                    {
                        sec = sec + 24*60*60;
                    }
                    reboot_time = localtime(&sec);
                    if (NULL == reboot_time)
                    {
                        return;
                    }
                    drv_rtc_set_alarm(reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                    MODULE_LOG_I(TIME, "3RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                }
                else
                {
                    sec         = sec + 24 * 60 * 60;
                    reboot_time = localtime(&sec);
                    if (NULL == reboot_time)
                    {
                        return;
                    }
                    drv_rtc_set_alarm(reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                    MODULE_LOG_I(TIME, "4RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                }
            }
            else
            {
                sec         = sec + 24 * 60 * 60;
                reboot_time = localtime(&sec);
                if (NULL == reboot_time)
                {
                    return;
                }
                drv_rtc_set_alarm(reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                MODULE_LOG_I(TIME, "5RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
            }
        }
        else
        {
            if (time.hour < FIXED_REBOOT_HOUR)
            {
                if ((fixed_min - curr_min) > val)
                {
                    sec        = sec + val * 60; /* the unit of val is minute */
                    alarm_time = localtime(&sec);
                    if (NULL == alarm_time)
                    {
                        return;
                    }
                    drv_rtc_set_alarm(alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                    MODULE_LOG_I(TIME, "6WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                }
                else
                {
                    if(tbox_pm_is_reboot_same_date())
                    {
                        sec = sec + val*60;
                        alarm_time  = localtime(&sec);
                        if (NULL == alarm_time)
                        {
                            return;
                        }
                        drv_rtc_set_alarm(alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                        MODULE_LOG_I(TIME, "9WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday,alarm_time->tm_hour,alarm_time->tm_min);
                    }
                    else
                    {
                        reboot_time = localtime(&sec);
                        if (NULL == reboot_time)
                        {
                            return;
                        }
                        drv_rtc_set_alarm(reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                        MODULE_LOG_I(TIME, "7RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                    }
                }
            }
            else if (time.hour == FIXED_REBOOT_HOUR)
            {
                if (time.min < dev_time_reboot_min)
                {
                    if ((dev_time_reboot_min - time.min + 24 * 60) > val)
                    {
                        sec        = sec + val * 60; /* the unit of val is minute */
                        alarm_time = localtime(&sec);
                        if (NULL == alarm_time)
                        {
                            return;
                        }
                        drv_rtc_set_alarm(alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                        MODULE_LOG_I(TIME, "8WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                    }
                    else
                    {
                        sec         = sec + 24 * 60 * 60;
                        reboot_time = localtime(&sec);
                        if (NULL == reboot_time)
                        {
                            return;
                        }
                        drv_rtc_set_alarm(reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                        MODULE_LOG_I(TIME, "9RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                    }
                }
                else
                {
                      if ((fixed_min - curr_min + 24 * 60) > val)
                      {
                          sec        = sec + val * 60; /* the unit of val is minute */
                          alarm_time = localtime(&sec);
                          if (NULL == alarm_time)
                          {
                              return;
                          }
                          drv_rtc_set_alarm(alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                          MODULE_LOG_I(TIME, "01WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                      }
                      else
                      {
                          reboot_time = localtime(&sec);
                          if (NULL == reboot_time)
                          {
                              return;
                          }
                          drv_rtc_set_alarm(reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                          MODULE_LOG_I(TIME, "02RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                      }
                }
            }
            else
            {
                if ((fixed_min - curr_min + 24 * 60) > val)
                {
                    sec        = sec + val * 60; /* the unit of val is minute */
                    alarm_time = localtime(&sec);
                    if (NULL == alarm_time)
                    {
                        return;
                    }
                    drv_rtc_set_alarm(alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                    MODULE_LOG_I(TIME, "03WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                }
                else
                {
                    sec         = sec + 24 * 60 * 60;
                    reboot_time = localtime(&sec);
                    if (NULL == reboot_time)
                    {
                        return;
                    }
                    drv_rtc_set_alarm(reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                    MODULE_LOG_I(TIME, "04RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, FIXED_REBOOT_HOUR, dev_time_reboot_min);
                }
            }
        }
    }
}

unsigned short time_if_reboot_time(void)
{
    unsigned short reboot_time = (FIXED_REBOOT_HOUR * 60) + dev_time_reboot_min;
    return reboot_time;
}

void dev_time_wakeup(void)
{
    dev_time_fail_cnt    = 0;
    dev_time_rtc_is_diag = true;
    dev_time_ntp_wakeup();
}

void dev_time_wakeup_checktime(void)
{
    dev_time_retry();
}

void dev_time_diag(void)
{
    int      ret;
    DEV_TIME time;

    if (!dev_time_rtc_is_diag)
        return;

    ret = time_if_rtc_get(&time);
    if (0 != ret)
        dev_time_fail_cnt++;
    else
        dev_time_fail_cnt = 0;
}

bool time_if_rtc_is_fault(void)
{
    if (dev_time_fail_cnt >= 5)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool dev_time_is_set_flag(void)
{
    return dev_time_is_set;
}

static void time_stop(void)
{
    dev_time_sleep();
    tbox_module_set_state(time_module_id, TBOX_MODULE_STATE_STOP);
}

static void time_start(void)
{
    dev_time_wakeup();
    tbox_module_set_state(time_module_id, TBOX_MODULE_STATE_START);
}

static int time_module_init(uint8_t seq)
{
    unsigned int flag;
    DEV_TIME     time;
    TBOX_CFG_ID  cfg_id;
    unsigned int retry = 3;

    switch (seq) {
    case MODULE_INIT_SEQ_OS:
        while (retry--) {
            if (drv_rtc_init() == RTC_STATUS_SUCCESS) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        break;
    
    case MODULE_INIT_SEQ_STORAGE:
        dev_time_load_backuptime();
        break;
    
    case MODULE_INIT_SEQ_MODULE:
        TBOX_CFG_ID_GET(RTCPOR, cfg_id);
        tbox_cfg_read(cfg_id, (unsigned char *)&flag);
        
        if (DEV_RTC_POR_FLAG == flag || drv_rtc_is_startup_pre())
        {
            if (0 == time_if_rtc_get(&time) && dev_time_check_is_valid(time))
            {
                time_if_set_with_source(TIME_SYNC_SOURCE_RTC, &time);
            }
            else
            {
                dev_time_rst();
                flag = DEV_RTC_POR_FLAG;
                tbox_cfg_write(cfg_id, (unsigned char *)&flag);
            }
        }

        srand(time.sec);
        dev_time_reboot_min = rand() % 60;
        MODULE_LOG_D(TIME, "reboot min:%d", dev_time_reboot_min);
        
        time_shell_init();
        GET_TBOX_MODULE_ID(TIME, time_module_id);
        tbox_module_set_state(time_module_id, TBOX_MODULE_STATE_START);
        break;
    
    default:
        break;
    }
    
    return TBOX_E_OK;
}

static void time_module_task(void *param)
{
    TickType_t last_wake = xTaskGetTickCount();
    (void)param;
    
    for (;;) {
        if (tbox_module_get_state(time_module_id) != TBOX_MODULE_STATE_START) {
            // MODULE_LOG_D(TIME, "task stopped, return");
            vTaskDelay(pdMS_TO_TICKS(100));  // 模块未启动时休眠
            continue;
        }
        
        dev_time_retry();
        dev_time_diag();
        dev_time_ntp_timeout();
        
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));  /* 1秒周期 */
    }
}

unsigned int time_if_get_systick_ms(void)
{
    return pdTICKS_TO_MS(xTaskGetTickCount());
}

unsigned int time_if_get_systick_s(void)
{
    return pdTICKS_TO_MS(xTaskGetTickCount()) / 1000;
}

int time_if_set_rtcwake(unsigned char day, unsigned char hour, unsigned char min)
{
    if (day == 0 && hour == 0 && min == 0)
    {
        return drv_rtc_set_alarm(0, 0, 0);
    }
    return drv_rtc_set_alarm(day, hour, min);
}