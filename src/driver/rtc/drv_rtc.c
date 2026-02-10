#include <string.h>
#include "api_drv.h"
#include "drv_log.h"

#define RTC_ADDR   0x51

#define dec2bcd(d) (uint8_t)((((d) / 10U) << 4U) | ((d) % 10U))
#define bcd2dec(b) (uint8_t)(((((uint8_t)(b)) >> 4U) * 10U) + (((uint8_t)(b)) & 0x0FU))

rtc_status_t drv_rtc_init(void)
{
    uint8_t data = 0;
    int32_t ret;

    
    ret = drv_i2c_write(RTC_ADDR, 0x00, &data, 1);
    if (0 != ret) {
        DRV_LOG_E(DRVRTC, "init failed");
        return RTC_STATUS_ERROR;
    }

    
    ret = drv_i2c_write(RTC_ADDR, 0x0D, &data, 1);
    if (0 != ret) {
        DRV_LOG_E(DRVRTC, "init failed");
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_SUCCESS;
}

rtc_status_t drv_rtc_deinit(void)
{
    return RTC_STATUS_SUCCESS;
}

rtc_status_t drv_rtc_set_time(const rtc_time_t *time)
{
    int32_t ret;
    uint8_t buf[7];

    if (NULL == time) {
        DRV_LOG_E(DRVRTC, "invalid parameter");
        return RTC_STATUS_INVALID_PARAMETER;
    }

    buf[0] = 0x7FU & dec2bcd(time->second);
    buf[1] = 0x7FU & dec2bcd(time->minute);
    buf[2] = 0x3FU & dec2bcd(time->hour);
    buf[3] = 0x3FU & dec2bcd(time->day);
    buf[4] = 0;
    buf[5] = 0x1FU & dec2bcd(time->month);
    buf[6] = 0xFFU & dec2bcd(time->year);

    ret = drv_i2c_write(RTC_ADDR, 2, buf, (uint16_t)sizeof(buf));

    if (0 != ret) {
        DRV_LOG_E(DRVRTC, "set time failed");
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_SUCCESS;
}

rtc_status_t drv_rtc_get_time(rtc_time_t *time)
{
    int32_t ret;
    uint8_t buf[7];

    if (NULL == time) {
        DRV_LOG_E(DRVRTC, "invalid parameter");
        return RTC_STATUS_INVALID_PARAMETER;
    }

    ret = drv_i2c_read(RTC_ADDR, 2, buf, (uint16_t)sizeof(buf));
    if (0 != ret) {
        DRV_LOG_E(DRVRTC, "read time failed");
        return RTC_STATUS_ERROR;
    }

    time->second = bcd2dec(buf[0] & 0x7FU);
    time->minute = bcd2dec(buf[1] & 0x7FU);
    time->hour   = bcd2dec(buf[2] & 0x3FU);
    time->day    = bcd2dec(buf[3] & 0x3FU);
    time->month  = bcd2dec(buf[5] & 0x1FU);
    time->year   = bcd2dec(buf[6] & 0xFFU);

    if ((buf[0] & 0x80U) != 0U) {
        DRV_LOG_W(DRVRTC, "low voltage had been detected");
        return RTC_STATUS_VL_DETECTED;
    }

    return RTC_STATUS_SUCCESS;
}

rtc_status_t drv_rtc_set_alarm(const rtc_time_t *time)
{

    int32_t ret;
    uint8_t buf[3];

    
    buf[0] = 0x00;
    ret    = drv_i2c_write(RTC_ADDR, 1, &buf[0], 1);
    if (0 != ret) {
        DRV_LOG_E(DRVRTC, "stop alarm failed");
        return RTC_STATUS_ERROR;
    }

    if (NULL == time) {
        DRV_LOG_W(DRVRTC, "alarm stoped, but not set");
        return RTC_STATUS_ALARM_STOPED;
    }

    buf[0] = 0x7FU & dec2bcd(time->minute);
    buf[1] = 0x7FU & dec2bcd(time->hour);
    buf[2] = 0x7FU & dec2bcd(time->day);
    ret    = drv_i2c_write(RTC_ADDR, 9, buf, (uint16_t)sizeof(buf));

    if (0 != ret) {
        DRV_LOG_E(DRVRTC, "set alarm failed");
        return RTC_STATUS_ERROR;
    }

    
    buf[0] = 0x02;
    ret    = drv_i2c_write(RTC_ADDR, 1, &buf[0], 1);
    if (0 != ret) {
        DRV_LOG_E(DRVRTC, "start alarm failed");
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_SUCCESS;
}


