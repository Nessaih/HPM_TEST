#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tbox_cfg_if.h"
#include "tbox_pm_io.h"
#include "dev_time.h"

static unsigned int dev_time_tick             = 0;
static unsigned int dev_time_sec              = 0;


time_t dev_time_get(DEV_TIME *time)
{
    unsigned int tick;
    time_t       sec;
    struct tm   *cur_time;

    tick     = xTaskGetTickCount() / 10;
    sec      = dev_time_sec + (tick - dev_time_tick) / 100;
    cur_time = localtime(&sec);

    if (time && cur_time) {
        time->year  = cur_time->tm_year - 100;
        time->month = cur_time->tm_mon + 1;
        time->day   = cur_time->tm_mday;
        time->hour  = cur_time->tm_hour;
        time->min   = cur_time->tm_min;
        time->sec   = cur_time->tm_sec;
        time->msec  = ((tick - dev_time_tick) % 100) * 10;
    }
    return sec;
}