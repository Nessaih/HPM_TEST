#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tbox_core.h"
#include "time_if.h"
#include "time_shell.h"
#include "tbox_cfg_if.h"
#include "drv_rtc.h"
#include "stimer.h"

static INT32 time_init(UINT8 seq);
static VOID time_stop(VOID);
static VOID time_start(VOID);
static VOID time_task(void *param);

TBOX_MODULE_FUN(TIME, time_init, time_stop, time_start, NULL_PTR, NULL_PTR, NULL_PTR);
TBOX_RUNLOOP_MODULE(TIME, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_INFO, TBOX_TASK_MEDIUM_STACK_SIZE, time_task);
TBOX_MODULE_LOADER(TIME) {}


#define DEV_CHECK_YEAR(year)                     (year > 37 || year < 25)

#define DEV_CHECK_MONTH(month)                   ((month < 1) || (month > 12))

#define DEV_CHECK_BIGMONTH_DAY(month, day)       (((month == 1) || (month == 3) || (month == 5) || (month == 7) || (month == 8) || (month == 10) || (month == 12)) && (day > 31))

#define DEV_CHECK_SMALLMONTH_DAY(month, day)     (((month == 4) || (month == 6) || (month == 9) || (month == 11)) && (day > 30))

#define DEV_CHECK_FEBRUARY_DAY(year, month, day) (((year % 4 != 0) && (month == 2) && (day > 28)) || ((year % 4 == 0) && (month == 2) && (day > 29)))

#define DEV_CHECK_DAY(year, month, day)          ((day < 1) || (DEV_CHECK_BIGMONTH_DAY(month, day)) || DEV_CHECK_SMALLMONTH_DAY(month, day) || DEV_CHECK_FEBRUARY_DAY(year, month, day))

#define DEV_CHECK_CLOCK(hour, min, sec)          ((hour > 23) || (min > 59) || (sec > 59))

#define TIME_MGR_PERIOD                           (1000 * 60 * 10)

#define TIME_PERIOD_MS                          (1000)

#define TIME_RTC_POR_FLAG  (0x7e8d7a8b)
#define TIME_FIXED_REBOOT_HOUR 3

typedef enum {
    RTC_TIME_REBOOT_MPU_UNKNOWN = 0,
    RTC_TIME_REBOOT_MPU_YES,
    RTC_TIME_REBOOT_MPU_NO,
    RTC_TIME_REBOOT_MPU_WAIT,
    RTC_TIME_REBOOT_MPU_FINISHED,
} RTC_TIME_REBOOT_MPU;

static TBOX_ID time_module_id;

static TickType_t   basetime_tick         = 0;  // 基准时刻的系统 tick（原始值）
static time_t       basetime_utc          = 0;  // 基准时刻的 UTC 时间戳（秒）
static bool         time_is_set           = false;
static bool         time_rtc_is_set       = false;
static bool         time_rtc_is_diag      = true;
static int          time_fail_cnt         = 0;
static unsigned int time_reboot_mpu       = RTC_TIME_REBOOT_MPU_UNKNOWN;
static TIME_SYNC_SOURCE time_sync_source  = TIME_SYNC_SOURCE_NONE;

static void time_mgr_info_init(void);
static void time_set(DEV_TIME time);
static void time_rst(void);
static int  time_rtc_set(DEV_TIME time);
void time_write_flash(unsigned char *data);
/* 临时函数声明，待后续实现 */
static int dev_reboot_date_is_same(void)
{
    /* TODO: 实现判断是否在同一天重启过的逻辑 */
    return 0;  /* 默认返回0，表示不是同一天 */
}

static inline uint32_t time_get_elapsed_ms(void)
{
    TickType_t current_tick = xTaskGetTickCount();
    TickType_t tick_diff = current_tick - basetime_tick;
    return pdTICKS_TO_MS(tick_diff);
}

unsigned int time_if_get_systick_ms(void)
{
    return pdTICKS_TO_MS(xTaskGetTickCount());
}

unsigned int time_if_get_systick_s(void)
{
    return pdTICKS_TO_MS(xTaskGetTickCount()) / 1000;
}

unsigned int time_if_get_basetime_tick(void)
{
    return (unsigned int)basetime_tick;
}

unsigned int time_if_get_basetime_utc_s(void)
{
    return (unsigned int)basetime_utc;
}

static void time_mgr_info_init(void)
{
    unsigned char flash_time_info[6] = {0};
    DEV_TIME      time;
    TBOX_CFG_ID   cfg_id;

    TBOX_CFG_ID_GET(DEVICETIME, cfg_id);
    tbox_cfg_read(cfg_id, flash_time_info);

    time.year  = flash_time_info[0];
    time.month = flash_time_info[1];
    time.day   = flash_time_info[2];
    time.hour  = flash_time_info[3];
    time.min   = flash_time_info[4];
    time.sec   = flash_time_info[5];
    if (!time_if_check_is_valid(time)) {
        flash_time_info[0] = 23;
        flash_time_info[1] = 1;
        flash_time_info[2] = 1;
        flash_time_info[3] = 0;
        flash_time_info[4] = 0;
        flash_time_info[5] = 0;
        tbox_cfg_write(cfg_id, flash_time_info);
    }
}

int time_if_check_is_valid(DEV_TIME time)
{
    int ret = 1;
    
    if (DEV_CHECK_YEAR(time.year) || 
        DEV_CHECK_MONTH(time.month) || 
        DEV_CHECK_DAY(time.year, time.month, time.day) || 
        DEV_CHECK_CLOCK(time.hour, time.min, time.sec)) {
            MODULE_LOG_E(TIME, "Invalid time format, year: %u, month: %u, day: %u, hour: %u, min: %u, sec: %u", time.year, time.month, time.day, time.hour, time.min, time.sec);
            ret = 0;
        }

    return ret;
}

int time_if_is_effective(DEV_TIME time)
{
    int           ret                = 0;
    unsigned char flash_time_info[6] = {0};
    TBOX_CFG_ID   cfg_id;

    TBOX_CFG_ID_GET(DEVICETIME, cfg_id);
    tbox_cfg_read(cfg_id, flash_time_info);

    if ((time.year * 12 + time.month - (flash_time_info[0] * 12 + flash_time_info[1]) >= 0) && (time.year * 12 + time.month - (flash_time_info[0] * 12 + flash_time_info[1]) <= 12))
        ret = 1;
    return ret;
}

void time_write_flash(unsigned char *data)
{
    DEV_TIME    time;
    TBOX_CFG_ID cfg_id;
    
    time.year  = data[0];
    time.month = data[1];
    time.day   = data[2];
    time.hour  = data[3];
    time.min   = data[4];
    time.sec   = data[5];
    if (time_if_check_is_valid(time)) {
        TBOX_CFG_ID_GET(DEVICETIME, cfg_id);
        tbox_cfg_write(cfg_id, data);
        MODULE_LOG_D(TIME, "write time to flash:20%02u-%02u-%02u %02u:%02u:%02u", data[0], data[1], data[2], data[3], data[4], data[5]);
    }
}

static void time_set(DEV_TIME time)
{
    struct tm cur_time;

    memset(&cur_time, 0, sizeof(cur_time));

    cur_time.tm_year = time.year + 100; /* from 1900*/
    cur_time.tm_mon  = time.month - 1;  /* from 0 */
    cur_time.tm_mday = time.day;
    cur_time.tm_hour = time.hour;
    cur_time.tm_min  = time.min;
    cur_time.tm_sec  = time.sec;

    basetime_tick = xTaskGetTickCount();
    basetime_utc  = mktime(&cur_time);
    time_is_set   = true;
}

void time_if_set_with_source(DEV_TIME time, TIME_SYNC_SOURCE source)
{
    unsigned char time_data[6];
    const char *time_sync_source_str[] = {
        "NONE",
        "RTC",
        "SHELL",
        "GNSS",
        "NTP"
    };

    if (!time_if_check_is_valid(time)) {
        MODULE_LOG_E(TIME, "Invalid time format, sync source: %d", source);
        return;
    }

    time_set(time);
    
    time_rtc_is_set = false;

    if (0 != time_rtc_set(time)) {
        MODULE_LOG_E(TIME, "Set time OK, but RTC sync failed, source: %d", source);
    }

    time_data[0] = time.year;
    time_data[1] = time.month;
    time_data[2] = time.day;
    time_data[3] = time.hour;
    time_data[4] = time.min;
    time_data[5] = time.sec;
    time_write_flash(time_data);

    time_sync_source = source;

    MODULE_LOG_I(TIME, "Time set from source %s: 20%02u-%02u-%02u %02u:%02u:%02u", 
        time_sync_source_str[source], time.year, time.month, time.day, time.hour, time.min, time.sec); 
}

TIME_SYNC_SOURCE time_if_get_sync_source(void)
{
    return time_sync_source;
}

static void time_rst(void)
{  
    DEV_TIME      time;
    unsigned char flash_time_info[6] = {0};
    TBOX_CFG_ID   cfg_id;

    TBOX_CFG_ID_GET(DEVICETIME, cfg_id);
    tbox_cfg_read(cfg_id, flash_time_info);

    time.year  = flash_time_info[0];
    time.month = flash_time_info[1];
    time.day   = flash_time_info[2];
    time.hour  = flash_time_info[3];
    time.min   = flash_time_info[4];
    time.sec   = flash_time_info[5];
    time.msec  = 0;
    time_if_set_with_source(time, TIME_SYNC_SOURCE_RTC);
}

time_t time_if_get(DEV_TIME *time)
{
    uint32_t   elapsed_ms;
    time_t     current_utc;
    struct tm *cur_time;

    elapsed_ms  = time_get_elapsed_ms();
    current_utc = basetime_utc + (elapsed_ms / 1000);
    cur_time    = localtime(&current_utc);

    if (time && cur_time) {
        time->year  = cur_time->tm_year - 100;
        time->month = cur_time->tm_mon + 1;
        time->day   = cur_time->tm_mday;
        time->hour  = cur_time->tm_hour;
        time->min   = cur_time->tm_min;
        time->sec   = cur_time->tm_sec;
        time->msec  = elapsed_ms % 1000;
    }
    return current_utc;
}

static int time_rtc_set(DEV_TIME time)
{
    int        ret = 0;
    rtc_time_t rtc = {0};

    if (time_rtc_is_set) {
        MODULE_LOG_E(TIME, "RTC is already set");
        ret = 0;
    } else {
        rtc.year   = time.year;
        rtc.month  = time.month;
        rtc.day    = time.day;
        rtc.hour   = time.hour;
        rtc.minute = time.min;
        rtc.second = time.sec;

        ret = drv_rtc_set_time(&rtc);
    }
    if (0 == ret)
        time_rtc_is_set = true;
    else
        time_rtc_is_set = false;

    return ret;
}

int time_if_rtc_get(DEV_TIME *time)
{
    int        ret;
    rtc_time_t rtc = {0};

    ret = drv_rtc_get_time(&rtc);

    if (RTC_STATUS_SUCCESS == ret) {
        time->year  = rtc.year;
        time->month = rtc.month;
        time->day   = rtc.day;
        time->hour  = rtc.hour;
        time->min   = rtc.minute;
        time->sec   = rtc.second;
        time->msec  = 0;
        return 0;
    }

    if (RTC_STATUS_VL_DETECTED == ret) {
        time->year  = rtc.year;
        time->month = rtc.month;
        time->day   = rtc.day;
        time->hour  = rtc.hour;
        time->min   = rtc.minute;
        time->sec   = rtc.second;
        time->msec  = 0;
        MODULE_LOG_D(TIME, "RTC voltage loss detected");
        return 0;
    } 

    return -1;
}

DEV_TIME rtc_time_init(void)
{   
    int          ret;
    unsigned int flag;
    DEV_TIME     time;
    TBOX_CFG_ID  cfg_id;

    TBOX_CFG_ID_GET(RTCPOR, cfg_id);
    tbox_cfg_read(cfg_id, &flag);
    
    if (TIME_RTC_POR_FLAG == flag) {
        ret = time_if_rtc_get(&time);

        MODULE_LOG_D(TIME, "time: %04d-%02d-%02d %02d:%02d:%02d", time.year + 2000, time.month, time.day, time.hour, time.min, time.sec);

        if (0 == ret && time_if_check_is_valid(time) && time_if_is_effective(time)) {
            time.msec = 0;
            time_if_set_with_source(time, TIME_SYNC_SOURCE_RTC);
        } else {
            time_rst();
            flag = TIME_RTC_POR_FLAG;
            tbox_cfg_write(cfg_id, &flag);
        }
    }
    return time;
}
int time_if_init(void)
{
    drv_rtc_init();
    rtc_time_init();
    time_mgr_info_init();

    return 0;
}

void time_if_clear_reboot_flag(void)
{
    time_reboot_mpu = RTC_TIME_REBOOT_MPU_FINISHED;
}

bool time_if_get_reboot_flag(void)
{
    return ((RTC_TIME_REBOOT_MPU_YES == time_reboot_mpu) || (RTC_TIME_REBOOT_MPU_WAIT == time_reboot_mpu));
}

void time_if_retry(void)
{
    int          ret;
    DEV_TIME     time;
    unsigned int flag;
    TBOX_CFG_ID  cfg_id;

    if (!time_is_set) {
        drv_rtc_init();
        TBOX_CFG_ID_GET(RTCPOR, cfg_id);
        tbox_cfg_read(cfg_id, &flag);
        
        if (TIME_RTC_POR_FLAG != flag) {
            time_rst();
            flag = TIME_RTC_POR_FLAG;
            tbox_cfg_write(cfg_id, &flag);
        } else {
            ret = time_if_rtc_get(&time);
            if (0 == ret && time_if_check_is_valid(time) && time_if_is_effective(time)) {
                time.msec = 0;
                time_if_set_with_source(time, TIME_SYNC_SOURCE_RTC);
            } else {
                time_rst();
                flag = TIME_RTC_POR_FLAG;
                tbox_cfg_write(cfg_id, &flag);
            }
        }
    }
}

void time_set_wakeup_time(void)
{
    int          ret_wake, ret_reboot;
    unsigned int fixed_min, curr_min;
    unsigned int val;
    unsigned int reboot_switch;
    uint32_t     elapsed_ms;
    time_t       current_utc;
    struct tm   *alarm_time;
    struct tm   *reboot_time;
    DEV_TIME     time;
    rtc_time_t   alarm;
    unsigned int time_reboot_min;

    /* 读取唤醒间隔配置（RTCINTV，单位：分钟） */
    TBOX_CFG_ID cfg_id;
    TBOX_CFG_ID_GET(RTCINTV, cfg_id);
    ret_wake = tbox_cfg_read(cfg_id, &val);
    if (ret_wake != TBOX_E_OK) {
        val = 180;  /* 默认180分钟 */
    }
    
    /* 读取RTC使能配置（RTCENABLE，0=禁用，1=启用） */
    TBOX_CFG_ID_GET(RTCENABLE, cfg_id);
    ret_reboot = tbox_cfg_read(cfg_id, &reboot_switch);
    if (ret_reboot != TBOX_E_OK) {
        reboot_switch = 1;  /* 默认启用 */
    }
    
    time_if_rtc_get(&time);

    elapsed_ms  = time_get_elapsed_ms();
    current_utc = basetime_utc + (elapsed_ms / 1000);

    srand(time.sec);
    time_reboot_min = rand() % 60;

    fixed_min = TIME_FIXED_REBOOT_HOUR * 60 + time_reboot_min;
    curr_min  = time.hour * 60 + time.min;

    if (ret_reboot < 0 || 0 == reboot_switch) {
        if (ret_wake < 0 || 0 == val) {
            // drv_rtc_stop_alarm();
            drv_rtc_set_alarm(NULL);
            return;
        } else {
            current_utc = current_utc + val * 60; /* the unit of val is minute */
            alarm_time  = localtime(&current_utc);

            memset(&alarm, 0, sizeof(alarm));
            alarm.day    = alarm_time->tm_mday;
            alarm.hour   = alarm_time->tm_hour;
            alarm.minute = alarm_time->tm_min;
            drv_rtc_set_alarm(&alarm);

            MODULE_LOG_I(TIME, "1WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
        }
    } else {
        if (ret_wake < 0 || 0 == val) {
            if (time.hour < TIME_FIXED_REBOOT_HOUR) {
                reboot_time = localtime(&current_utc);

                memset(&alarm, 0, sizeof(alarm));
                alarm.day    = reboot_time->tm_mday;
                alarm.hour   = TIME_FIXED_REBOOT_HOUR;
                alarm.minute = time_reboot_min;
                drv_rtc_set_alarm(&alarm);

                MODULE_LOG_I(TIME, "2RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, TIME_FIXED_REBOOT_HOUR, time_reboot_min);
            } else if (time.hour == TIME_FIXED_REBOOT_HOUR) {
                if (time.min < time_reboot_min) {
                    if (dev_reboot_date_is_same()) {
                        current_utc = current_utc + 24 * 60 * 60;
                    }
                    reboot_time = localtime(&current_utc);

                    memset(&alarm, 0, sizeof(alarm));
                    alarm.day    = reboot_time->tm_mday;
                    alarm.hour   = TIME_FIXED_REBOOT_HOUR;
                    alarm.minute = time_reboot_min;
                    drv_rtc_set_alarm(&alarm);

                    MODULE_LOG_I(TIME, "3RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, TIME_FIXED_REBOOT_HOUR, time_reboot_min);
                } else {
                    current_utc = current_utc + 24 * 60 * 60;
                    reboot_time = localtime(&current_utc);

                    memset(&alarm, 0, sizeof(alarm));
                    alarm.day    = reboot_time->tm_mday;
                    alarm.hour   = TIME_FIXED_REBOOT_HOUR;
                    alarm.minute = time_reboot_min;
                    drv_rtc_set_alarm(&alarm);

                    MODULE_LOG_I(TIME, "4RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, TIME_FIXED_REBOOT_HOUR, time_reboot_min);
                }
            } else {
                current_utc = current_utc + 24 * 60 * 60;
                reboot_time = localtime(&current_utc);

                memset(&alarm, 0, sizeof(alarm));
                alarm.day    = reboot_time->tm_mday;
                alarm.hour   = TIME_FIXED_REBOOT_HOUR;
                alarm.minute = time_reboot_min;
                drv_rtc_set_alarm(&alarm);

                MODULE_LOG_I(TIME, "5RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, TIME_FIXED_REBOOT_HOUR, time_reboot_min);
            }
        } else {
            if (time.hour < TIME_FIXED_REBOOT_HOUR) {
                if ((fixed_min - curr_min) > val) {
                    current_utc = current_utc + val * 60; /* the unit of val is minute */
                    alarm_time  = localtime(&current_utc);

                    memset(&alarm, 0, sizeof(alarm));
                    alarm.day    = alarm_time->tm_mday;
                    alarm.hour   = alarm_time->tm_hour;
                    alarm.minute = alarm_time->tm_min;
                    drv_rtc_set_alarm(&alarm);

                    MODULE_LOG_I(TIME, "6WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                } else {
                    if (dev_reboot_date_is_same()) {
                        current_utc = current_utc + val * 60;
                        alarm_time  = localtime(&current_utc);

                        memset(&alarm, 0, sizeof(alarm));
                        alarm.day    = alarm_time->tm_mday;
                        alarm.hour   = alarm_time->tm_hour;
                        alarm.minute = alarm_time->tm_min;
                        drv_rtc_set_alarm(&alarm);

                        MODULE_LOG_I(TIME, "9WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                    } else {
                        reboot_time = localtime(&current_utc);

                        memset(&alarm, 0, sizeof(alarm));
                        alarm.day    = reboot_time->tm_mday;
                        alarm.hour   = TIME_FIXED_REBOOT_HOUR;
                        alarm.minute = time_reboot_min;
                        drv_rtc_set_alarm(&alarm);

                        MODULE_LOG_I(TIME, "7RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, TIME_FIXED_REBOOT_HOUR, time_reboot_min);
                    }
                }
            } else if (time.hour == TIME_FIXED_REBOOT_HOUR) {
                if (time.min < time_reboot_min) {
                    if ((time_reboot_min - time.min + 24 * 60) > val) {
                        current_utc = current_utc + val * 60; /* the unit of val is minute */
                        alarm_time  = localtime(&current_utc);

                        memset(&alarm, 0, sizeof(alarm));
                        alarm.day    = alarm_time->tm_mday;
                        alarm.hour   = alarm_time->tm_hour;
                        alarm.minute = alarm_time->tm_min;
                        drv_rtc_set_alarm(&alarm);

                        MODULE_LOG_I(TIME, "8WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                    } else {
                        current_utc = current_utc + 24 * 60 * 60;
                        reboot_time = localtime(&current_utc);

                        memset(&alarm, 0, sizeof(alarm));
                        alarm.day    = reboot_time->tm_mday;
                        alarm.hour   = TIME_FIXED_REBOOT_HOUR;
                        alarm.minute = time_reboot_min;
                        drv_rtc_set_alarm(&alarm);

                        MODULE_LOG_I(TIME, "9RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, TIME_FIXED_REBOOT_HOUR, time_reboot_min);
                    }
                } else {
                    if ((fixed_min - curr_min + 24 * 60) > val) {
                        current_utc = current_utc + val * 60; /* the unit of val is minute */
                        alarm_time  = localtime(&current_utc);

                        memset(&alarm, 0, sizeof(alarm));
                        alarm.day    = alarm_time->tm_mday;
                        alarm.hour   = alarm_time->tm_hour;
                        alarm.minute = alarm_time->tm_min;
                        drv_rtc_set_alarm(&alarm);

                        MODULE_LOG_I(TIME, "01WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                    } else {
                        reboot_time = localtime(&current_utc);

                        memset(&alarm, 0, sizeof(alarm));
                        alarm.day    = reboot_time->tm_mday;
                        alarm.hour   = TIME_FIXED_REBOOT_HOUR;
                        alarm.minute = time_reboot_min;
                        drv_rtc_set_alarm(&alarm);

                        MODULE_LOG_I(TIME, "02RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, TIME_FIXED_REBOOT_HOUR, time_reboot_min);
                    }
                }
            } else {
                if ((fixed_min - curr_min + 24 * 60) > val) {
                    current_utc = current_utc + val * 60; /* the unit of val is minute */
                    alarm_time  = localtime(&current_utc);

                    memset(&alarm, 0, sizeof(alarm));
                    alarm.day    = alarm_time->tm_mday;
                    alarm.hour   = alarm_time->tm_hour;
                    alarm.minute = alarm_time->tm_min;
                    drv_rtc_set_alarm(&alarm);

                    MODULE_LOG_I(TIME, "03WakeUpTime->day:%d,hour:%d,min:%d", alarm_time->tm_mday, alarm_time->tm_hour, alarm_time->tm_min);
                } else {
                    current_utc = current_utc + 24 * 60 * 60;
                    reboot_time = localtime(&current_utc);

                    memset(&alarm, 0, sizeof(alarm));
                    alarm.day    = reboot_time->tm_mday;
                    alarm.hour   = TIME_FIXED_REBOOT_HOUR;
                    alarm.minute = time_reboot_min;
                    drv_rtc_set_alarm(&alarm);

                    MODULE_LOG_I(TIME, "04RebootTime->day:%d,hour:%d,min:%d", reboot_time->tm_mday, TIME_FIXED_REBOOT_HOUR, time_reboot_min);
                }
            }
        }
    }
}

void time_if_sleep(void)
{
    time_is_set      = false;
    time_rtc_is_diag = false;

    time_set_wakeup_time();
}

void time_if_wakeup(void)
{
    time_fail_cnt    = 0;
    time_rtc_is_diag = true;
    time_reboot_mpu  = RTC_TIME_REBOOT_MPU_UNKNOWN;
}

void time_if_diag(void)
{
    int      ret;
    DEV_TIME time;

    if (!time_rtc_is_diag) {
        return;
    }

    ret = time_if_rtc_get(&time);

    if (0 != ret) {
        time_fail_cnt++;
    } else {
        time_fail_cnt = 0;
    }
}

bool time_if_rtc_is_fault(void)
{
    if (time_fail_cnt >= 5) {
        return true;
    } else {
        return false;
    }
}

int time_if_set_rtcwake(const rtc_time_t *alarm_time)
{
    rtc_status_t ret;
    
    ret = drv_rtc_set_alarm(alarm_time);
    
    if (ret == RTC_STATUS_SUCCESS) {
        return 0;
    } else if (ret == RTC_STATUS_ALARM_STOPED) {
        return 0;  // 停止alarm也视为成功
    } else {
        return -1; // 其他错误返回-1
    }
}

static void time_task(void *param)
{
    TickType_t last_wake = xTaskGetTickCount();
    (void)param;

    for (;;) {
        if(tbox_module_get_state(time_module_id) != TBOX_MODULE_STATE_START) {
            MODULE_LOG_D(TIME, "task stopped, return");
            return;
        }

        time_if_retry();
        time_if_diag();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(TIME_PERIOD_MS));
    }
}

static INT32 time_init(UINT8 seq)
{
    switch (seq) {
        case  MODULE_INIT_SEQ_OS:
            break;
        
        case  MODULE_INIT_SEQ_STORAGE:
            break;
        
        case  MODULE_INIT_SEQ_MODULE:
            time_if_init();
            time_shell_init();
            GET_TBOX_MODULE_ID(TIME, time_module_id);
            tbox_module_set_state(time_module_id, TBOX_MODULE_STATE_START);
            break;

        default:
            return TBOX_E_INVALID_PARAM;
    }
    return TBOX_E_OK;
}

static VOID time_stop(VOID)
{
    time_if_sleep();
    tbox_module_set_state(time_module_id, TBOX_MODULE_STATE_STOP);
}

static VOID time_start(VOID)
{
    time_if_wakeup();
}

