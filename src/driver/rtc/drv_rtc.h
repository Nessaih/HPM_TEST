#ifndef __DRV_RTC_H__
#define __DRV_RTC_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t year;  // not valid for alarm clock
    uint8_t month; // not valid for alarm clock
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second; // not valid for alarm clock
} rtc_time_t;

typedef enum {
    RTC_STATUS_SUCCESS,
    RTC_STATUS_VL_DETECTED,
    RTC_STATUS_INVALID_PARAMETER,
    RTC_STATUS_ALARM_STOPED,
    RTC_STATUS_ERROR,

} rtc_status_t;

extern rtc_status_t drv_rtc_init(void);
extern rtc_status_t drv_rtc_deinit(void);
extern rtc_status_t drv_rtc_set_time(const rtc_time_t *time);
extern rtc_status_t drv_rtc_get_time(rtc_time_t *time);
extern rtc_status_t drv_rtc_set_alarm(unsigned char day, unsigned char hour, unsigned char min);
extern bool         drv_rtc_is_startup_pre(void);


#endif //__DRV_RTC_H__