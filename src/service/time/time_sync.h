#ifndef __TIME_SYNC_H__
#define __TIME_SYNC_H__

#include "time_if.h"

#ifdef __cplusplus
extern "C" {
#endif

void dev_time_ntp_init(void);
void dev_time_ntp_sleep(void);
void dev_time_ntp_wakeup(void);
void dev_time_ntp_timeout(void);
TIME_SYNC_SOURCE dev_time_ntp_get_last_type(void);
void dev_time_ntp_set_type(TIME_SYNC_SOURCE type);
void time_if_set_with_source(TIME_SYNC_SOURCE type, DEV_TIME *time);
TIME_SYNC_SOURCE time_if_get_sync_source(void);
const char *time_if_sync_source_to_str(TIME_SYNC_SOURCE source);

#ifdef __cplusplus
}
#endif

#endif
