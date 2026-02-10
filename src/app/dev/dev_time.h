#ifndef __DEV_TIME_H__
#define __DEV_TIME_H__

#include <stdbool.h>
#include <time.h>

typedef struct {
    unsigned char  year;  /* from 2000 */
    unsigned char  month; /* from 1 */
    unsigned char  day;   /* from 1 */
    unsigned char  hour;
    unsigned char  min;
    unsigned char  sec;
    unsigned short msec;
} DEV_TIME;

time_t       dev_time_get(DEV_TIME *time);

#endif
