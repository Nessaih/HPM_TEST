#ifndef __HPM_DEV_H__
#define __HPM_DEV_H__

typedef enum
{
    HPM_DEV_STATUS_INIT = 0,
    HPM_DEV_STATUS_ACCON = 1,
    HPM_DEV_STATUS_ACCOFF = 2,
    HPM_DEV_STATUS_IDLE = 3,
    HPM_DEV_STATUS_WAKEUP = 4,
    HPM_DEV_STATUS_MAX
} hpm_dev_status_e;

typedef enum
{
    HPM_DEV_EVENT_NONE = 0,
    HPM_DEV_EVENT_SHELL = 1,
    HPM_DEV_EVENT_ACCOFF = 2,
    HPM_DEV_EVENT_ACCON = 3,
    HPM_DEV_EVENT_CYCLE = 4,
    HPM_DEV_EVENT_CALL = 5,
    HPM_DEV_EVENT_WAKEUP = 6,
} hpm_dev_event_e;

VOID hpm_dev_init(UINT8 seq);
VOID hpm_dev_deinit(void);
VOID hpm_dev_wakeup(void);
VOID hpm_dev_sleep(void);
VOID hpm_dev_process(void);
VOID hpm_dev_handle_event(hpm_dev_event_e event);

#endif