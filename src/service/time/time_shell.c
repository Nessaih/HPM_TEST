#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tbox_core.h"
#include "tbox_shell_if.h"
#include "time_if.h"
#include "drv_rtc.h"

static BaseType_t show_time(char *buf, size_t bufsz, const char *cmd)
{
    int32_t  len = 0;
    DEV_TIME time;
    TIME_SYNC_SOURCE sync_source;
    (void)cmd;

    /* 显示同步源 */
    sync_source = time_if_get_sync_source();
    len += snprintf(&buf[len], bufsz - (size_t)len, "\r\n=== Time Information ===\r\n");
    len += snprintf(&buf[len], bufsz - (size_t)len, "Sync Source : %s\r\n", time_if_sync_source_to_str(sync_source));
    
    /* 显示系统时间 */
    memset(&time, 0, sizeof(time));
    time_if_get(&time);
    len += snprintf(&buf[len], bufsz - (size_t)len, "System Time : %04u-%02u-%02u %02u:%02u:%02u.%03u\r\n", 
                    time.year + 2000U, time.month, time.day, time.hour, time.min, time.sec, time.msec);

    /* 显示RTC时间 */
    memset(&time, 0, sizeof(time));
    if (time_if_rtc_get(&time) == 0) {
        len += snprintf(&buf[len], bufsz - (size_t)len, "RTC Time    : %04u-%02u-%02u %02u:%02u:%02u\r\n", 
                        time.year + 2000U, time.month, time.day, time.hour, time.min, time.sec);
    } else {
        len += snprintf(&buf[len], bufsz - (size_t)len, "RTC Time    : Read Failed\r\n");
    }

    /* 显示系统tick(ms) */
    len += snprintf(&buf[len], bufsz - (size_t)len, "System Tick : %u ms\r\n", time_if_get_systick_ms());
    
    /* 显示RTC故障状态 */
    if (time_if_rtc_is_fault()) {
        len += snprintf(&buf[len], bufsz - (size_t)len, "RTC Status  : FAULT\r\n");
    } else {
        len += snprintf(&buf[len], bufsz - (size_t)len, "RTC Status  : OK\r\n");
    }
    
    len += snprintf(&buf[len], bufsz - (size_t)len, "========================\r\n");

    return pdFALSE;
}

static BaseType_t set_time(char *buf, size_t bufsz, const char *cmd)
{
    const char *param_ptr;
    BaseType_t  param_len;
    DEV_TIME    time;
    int32_t     year, month, day, hour, min, sec;
    int32_t     parsed;
    char        date_str[16] = {0};
    char        time_str[16] = {0};

    /* 获取日期参数 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL == param_ptr || param_len >= sizeof(date_str)) {
        snprintf(buf, bufsz, "Usage: settime <year-month-day> <hour:min:sec>\r\n"
                             "Example: settime 2026-02-05 15:46:20\r\n"
                             "         settime 26-02-05 15:46:20\r\n");
        return pdFALSE;
    }
    memcpy(date_str, param_ptr, param_len);
    date_str[param_len] = '\0';

    /* 获取时间参数 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param_len);
    if (NULL == param_ptr || param_len >= sizeof(time_str)) {
        snprintf(buf, bufsz, "Usage: settime <year-month-day> <hour:min:sec>\r\n"
                             "Example: settime 2026-02-05 15:46:20\r\n"
                             "         settime 26-02-05 15:46:20\r\n");
        return pdFALSE;
    }
    memcpy(time_str, param_ptr, param_len);
    time_str[param_len] = '\0';

    /* 解析日期：支持 2026-02-05 或 26-02-05 */
    parsed = sscanf(date_str, "%d-%d-%d", &year, &month, &day);
    if (parsed != 3) {
        snprintf(buf, bufsz, "Invalid date format: %s\r\n"
                             "Expected: year-month-day (e.g. 2026-02-05 or 26-02-05)\r\n", date_str);
        return pdFALSE;
    }

    /* 解析时间：15:46:20 */
    parsed = sscanf(time_str, "%d:%d:%d", &hour, &min, &sec);
    if (parsed != 3) {
        snprintf(buf, bufsz, "Invalid time format: %s\r\n"
                             "Expected: hour:min:sec (e.g. 15:46:20)\r\n", time_str);
        return pdFALSE;
    }

    /* 年份处理 */
    if (year < 100) {
        year = year + 2000;  /* 26 -> 2026 */
    }
    if (year < 2000 || year > 2099) {
        snprintf(buf, bufsz, "Year out of range: %d (Expected: 2000-2099 or 0-99)\r\n", year);
        return pdFALSE;
    }
    time.year = (unsigned char)(year - 2000);

    /* 填充时间结构体 */
    time.month = (unsigned char)month;
    time.day   = (unsigned char)day;
    time.hour  = (unsigned char)hour;
    time.min   = (unsigned char)min;
    time.sec   = (unsigned char)sec;
    time.msec  = 0;

    /* 验证时间格式 */
    if (!time_if_check_is_valid(time)) {
        snprintf(buf, bufsz, "Invalid time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
                 year, month, day, hour, min, sec);
        return pdFALSE;
    }

    /* 设置时间 */
    time_if_set_with_source(time, TIME_SYNC_SOURCE_SHELL);

    snprintf(buf, bufsz, "Time set successfully: %04d-%02d-%02d %02d:%02d:%02d\r\n",
             year, month, day, hour, min, sec);

    return pdFALSE;
}

static BaseType_t set_rtc_wake(char *buf, size_t bufsz, const char *cmd)
{
    const char *param_ptr;
    BaseType_t  param_len;
    rtc_time_t  alarm;
    int32_t     day, hour, min;
    int32_t     parsed;
    char        day_str[8] = {0};
    char        time_str[16] = {0};
    rtc_status_t ret;

    /* 获取第一个参数 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL == param_ptr) {
        snprintf(buf, bufsz, "Usage: setrtcwake <day> <hour:min> or setrtcwake off\r\n"
                             "Example: setrtcwake 5 14:30\r\n"
                             "         setrtcwake off\r\n");
        return pdFALSE;
    }

    /* 检查是否是关闭命令 */
    if (param_len == 3 && strncmp(param_ptr, "off", 3) == 0) {
        ret = drv_rtc_set_alarm(NULL);
        if (ret == RTC_STATUS_SUCCESS || ret == RTC_STATUS_ALARM_STOPED) {
            snprintf(buf, bufsz, "RTC wake alarm stopped\r\n");
        } else {
            snprintf(buf, bufsz, "Failed to stop RTC wake alarm (status: %d)\r\n", ret);
        }
        return pdFALSE;
    }

    /* 获取日期参数 */
    if (param_len >= sizeof(day_str)) {
        snprintf(buf, bufsz, "Invalid day parameter\r\n");
        return pdFALSE;
    }
    memcpy(day_str, param_ptr, param_len);
    day_str[param_len] = '\0';

    /* 获取时间参数 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param_len);
    if (NULL == param_ptr || param_len >= sizeof(time_str)) {
        snprintf(buf, bufsz, "Usage: setrtcwake <day> <hour:min>\r\n"
                             "Example: setrtcwake 5 14:30\r\n");
        return pdFALSE;
    }
    memcpy(time_str, param_ptr, param_len);
    time_str[param_len] = '\0';

    /* 解析日期 */
    parsed = sscanf(day_str, "%d", &day);
    if (parsed != 1) {
        snprintf(buf, bufsz, "Invalid day format: %s (Expected: 1-31)\r\n", day_str);
        return pdFALSE;
    }

    /* 解析时间：14:30 */
    parsed = sscanf(time_str, "%d:%d", &hour, &min);
    if (parsed != 2) {
        snprintf(buf, bufsz, "Invalid time format: %s (Expected: hour:min, e.g. 14:30)\r\n", time_str);
        return pdFALSE;
    }

    /* 验证参数范围 */
    if (day < 1 || day > 31) {
        snprintf(buf, bufsz, "Day out of range: %d (Expected: 1-31)\r\n", day);
        return pdFALSE;
    }
    if (hour < 0 || hour > 23) {
        snprintf(buf, bufsz, "Hour out of range: %d (Expected: 0-23)\r\n", hour);
        return pdFALSE;
    }
    if (min < 0 || min > 59) {
        snprintf(buf, bufsz, "Minute out of range: %d (Expected: 0-59)\r\n", min);
        return pdFALSE;
    }

    /* 填充RTC alarm结构体 */
    memset(&alarm, 0, sizeof(alarm));
    alarm.day    = (uint8_t)day;
    alarm.hour   = (uint8_t)hour;
    alarm.minute = (uint8_t)min;

    /* 设置唤醒时间 */
    ret = drv_rtc_set_alarm(&alarm);
    if (ret == RTC_STATUS_SUCCESS) {
        snprintf(buf, bufsz, "RTC wake alarm set: day=%d, %02d:%02d\r\n", day, hour, min);
    } else {
        snprintf(buf, bufsz, "Failed to set RTC wake alarm (status: %d)\r\n", ret);
    }

    return pdFALSE;
}

TBOX_SHELL_DEFINE(showtime, "show time information", 0, show_time);
TBOX_SHELL_DEFINE(settime, "set system time: <year-month-day> <hour:min:sec>", 2, set_time);
TBOX_SHELL_DEFINE(setrtcwake, "set RTC wake alarm: <day> <hour:min> or off", -1, set_rtc_wake);

int time_shell_init(void)
{
    TBOX_SHELL_REGISTER(showtime);
    TBOX_SHELL_REGISTER(settime);
    TBOX_SHELL_REGISTER(setrtcwake);
    
    return 0;
}
