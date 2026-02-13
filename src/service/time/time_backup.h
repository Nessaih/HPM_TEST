#ifndef __TIME_BACKUP_H__
#define __TIME_BACKUP_H__

#include "time_if.h"

#ifdef __cplusplus
extern "C" {
#endif

void dev_time_load_backuptime(void);
void dev_time_set(const DEV_TIME *time);
int dev_time_check_is_valid(DEV_TIME time);
void dev_time_get_backuptime(DEV_TIME *time);
void dev_time_set_backuptime(DEV_TIME *time);
void dev_time_save_backuptime(void);

#ifdef __cplusplus
}
#endif

#endif
