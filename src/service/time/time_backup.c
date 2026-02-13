#include <string.h>
#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "time_if.h"
#include "time_backup.h"

#define DEV_TIME_MAGICNO    (0xAAAB)
#define DEV_TIME_VER        (0x1230)
#define TIME_BACKUP_NAME    "TIMEBACKUP"

typedef struct
{
    uint16_t magic_no;
    uint16_t ver;
    uint8_t  time[6];
} dev_time_info_t;

#define DEV_CHECK_YEAR(year)                     (year > 37 || year < 25)
#define DEV_CHECK_MONTH(month)                   ((month < 1) || (month > 12))
#define DEV_CHECK_BIGMONTH_DAY(month, day)       (((month == 1) || (month == 3) || (month == 5) || (month == 7) || (month == 8) || (month == 10) || (month == 12)) && (day > 31))
#define DEV_CHECK_SMALLMONTH_DAY(month, day)     (((month == 4) || (month == 6) || (month == 9) || (month == 11)) && (day > 30))
#define DEV_CHECK_FEBRUARY_DAY(year, month, day) (((year % 4 != 0) && (month == 2) && (day > 28)) || ((year % 4 == 0) && (month == 2) && (day > 29)))
#define DEV_CHECK_DAY(year, month, day)          ((day < 1) || (DEV_CHECK_BIGMONTH_DAY(month, day)) || DEV_CHECK_SMALLMONTH_DAY(month, day) || DEV_CHECK_FEBRUARY_DAY(year, month, day))
#define DEV_CHECK_CLOCK(hour, min, sec)          ((hour > 23) || (min > 59) || (sec > 59))

static DEV_TIME dev_time_backuptime = {0, 0, 0, 0, 0, 0, 0};

void dev_time_load_backuptime(void)
{
    dev_time_info_t data;

    tbox_cfg_getkv(TIME_BACKUP_NAME, (uint8_t *)&data, sizeof(data));
    if (DEV_TIME_MAGICNO != data.magic_no || DEV_TIME_VER != data.ver)
    {
        data.time[0]  = 25;
        data.time[1]  = 1;
        data.time[2]  = 1;
        data.time[3]  = 0;
        data.time[4]  = 0;
        data.time[5]  = 0;
        data.magic_no = DEV_TIME_MAGICNO;
        data.ver      = DEV_TIME_VER;
        tbox_cfg_setkv(TIME_BACKUP_NAME, (uint8_t *)&data, sizeof(data));
    }

    dev_time_backuptime.year  = data.time[0];
    dev_time_backuptime.month = data.time[1];
    dev_time_backuptime.day   = data.time[2];
    dev_time_backuptime.hour  = data.time[3];
    dev_time_backuptime.min   = data.time[4];
    dev_time_backuptime.sec   = data.time[5];
    dev_time_backuptime.msec  = 0;

    if (!dev_time_check_is_valid(dev_time_backuptime))
    {
        dev_time_backuptime.year  = 25;
        dev_time_backuptime.month = 1;
        dev_time_backuptime.day   = 1;
        dev_time_backuptime.hour  = 0;
        dev_time_backuptime.min   = 0;
        dev_time_backuptime.sec   = 0;

        data.time[0]  = 25;
        data.time[1]  = 1;
        data.time[2]  = 1;
        data.time[3]  = 0;
        data.time[4]  = 0;
        data.time[5]  = 0;
        data.magic_no = DEV_TIME_MAGICNO;
        data.ver      = DEV_TIME_VER;

        tbox_cfg_setkv(TIME_BACKUP_NAME, (uint8_t *)&data, sizeof(data));
    }

    MODULE_LOG_I(TIME, "Load backup time: 20%02u-%02u-%02u %02u:%02u:%02u",
                 dev_time_backuptime.year, dev_time_backuptime.month, dev_time_backuptime.day,
                 dev_time_backuptime.hour, dev_time_backuptime.min, dev_time_backuptime.sec);
}

int dev_time_check_is_valid(DEV_TIME time)
{
    int ret = 1;

    if (DEV_CHECK_YEAR(time.year) ||
        DEV_CHECK_MONTH(time.month) ||
        DEV_CHECK_DAY(time.year, time.month, time.day) ||
        DEV_CHECK_CLOCK(time.hour, time.min, time.sec))
    {
        MODULE_LOG_E(TIME, "Invalid time: 20%02u-%02u-%02u %02u:%02u:%02u",
                     time.year, time.month, time.day, time.hour, time.min, time.sec);
        ret = 0;
    }

    return ret;
}

int time_if_check_is_valid(DEV_TIME time)
{
    return dev_time_check_is_valid(time);
}

void dev_time_get_backuptime(DEV_TIME *time)
{
    if (!dev_time_check_is_valid(dev_time_backuptime))
    {
        dev_time_backuptime.year  = 25;
        dev_time_backuptime.month = 1;
        dev_time_backuptime.day   = 1;
        dev_time_backuptime.hour  = 0;
        dev_time_backuptime.min   = 0;
        dev_time_backuptime.sec   = 0;
        dev_time_backuptime.msec  = 0;
    }

    memcpy(time, &dev_time_backuptime, sizeof(DEV_TIME));

    MODULE_LOG_D(TIME, "Get backup time: 20%02u-%02u-%02u %02u:%02u:%02u",
                 time->year, time->month, time->day, time->hour, time->min, time->sec);
}

void dev_time_set_backuptime(DEV_TIME *time)
{
    if (NULL == time)
    {
        return;
    }

    memcpy(&dev_time_backuptime, time, sizeof(DEV_TIME));
}

void dev_time_save_backuptime(void)
{
    dev_time_info_t data;

    if (!dev_time_check_is_valid(dev_time_backuptime))
    {
        dev_time_backuptime.year  = 25;
        dev_time_backuptime.month = 1;
        dev_time_backuptime.day   = 1;
        dev_time_backuptime.hour  = 0;
        dev_time_backuptime.min   = 0;
        dev_time_backuptime.sec   = 0;
        dev_time_backuptime.msec  = 0;
    }

    MODULE_LOG_I(TIME, "Save backup time: 20%02u-%02u-%02u %02u:%02u:%02u",
                 dev_time_backuptime.year, dev_time_backuptime.month, dev_time_backuptime.day,
                 dev_time_backuptime.hour, dev_time_backuptime.min, dev_time_backuptime.sec);

    data.time[0]  = dev_time_backuptime.year;
    data.time[1]  = dev_time_backuptime.month;
    data.time[2]  = dev_time_backuptime.day;
    data.time[3]  = dev_time_backuptime.hour;
    data.time[4]  = dev_time_backuptime.min;
    data.time[5]  = dev_time_backuptime.sec;
    data.magic_no = DEV_TIME_MAGICNO;
    data.ver      = DEV_TIME_VER;

    tbox_cfg_setkv(TIME_BACKUP_NAME, (uint8_t *)&data, sizeof(data));
}
