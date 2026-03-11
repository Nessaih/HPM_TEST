#ifndef HPM_STATUS_H
#define HPM_STATUS_H

INT32 hpm_status_init(UINT8 seq);
VOID hpm_status_wakeup(VOID);
VOID hpm_status_sleep(VOID);
VOID hpm_status_process(VOID);

#endif