#ifndef __TIME_IF_H__
#define __TIME_IF_H__

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char  year;  /* from 2000 */
    unsigned char  month; /* from 1 */
    unsigned char  day;   /* from 1 */
    unsigned char  hour;
    unsigned char  min;
    unsigned char  sec;
    unsigned short msec;
} DEV_TIME;

typedef enum {
    TIME_SYNC_SOURCE_NONE = 0,  // 未设置
    TIME_SYNC_SOURCE_FLASH,     // Flash备份
    TIME_SYNC_SOURCE_RTC,       // RTC硬件
    TIME_SYNC_SOURCE_NTP,       // NTP网络时间
    TIME_SYNC_SOURCE_TSP,       // TSP平台校时
    TIME_SYNC_SOURCE_GNSS,      // GNSS
    TIME_SYNC_SOURCE_SHELL,     // Shell手动设置
} TIME_SYNC_SOURCE;

/* Time get/set functions */
extern time_t       time_if_get(DEV_TIME *time);
extern int          time_if_rtc_get(DEV_TIME *time);
extern void         time_if_set_with_source(TIME_SYNC_SOURCE type, DEV_TIME *time);
extern TIME_SYNC_SOURCE time_if_get_sync_source(void);
extern bool         time_if_rtc_is_set(void);
extern bool         time_if_is_set(void);

/*Time reboot time*/
extern unsigned short time_if_reboot_time(void);

/* RTC alarm functions */
extern int          time_if_set_rtcwake(unsigned char day, unsigned char hour, unsigned char min);

/* System tick functions */
extern unsigned int time_if_get_systick_ms(void);
extern unsigned int time_if_get_systick_s(void);
extern unsigned int time_if_get_basetime_tick(void);
extern unsigned int time_if_get_basetime_utc_s(void);

/* Validation functions */
extern int          time_if_check_is_valid(DEV_TIME time);
extern bool         time_if_rtc_is_fault(void);

/* Utility functions */
extern const char  *time_if_sync_source_to_str(TIME_SYNC_SOURCE source);

extern void         time_if_update_backuptime(const DEV_TIME *time);

#ifdef __cplusplus
}
#endif

#endif /* __TIME_IF_H__ */
