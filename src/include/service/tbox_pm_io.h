#ifndef TBOX_PM_IO_H
#define TBOX_PM_IO_H

#include "tbox_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    PM_WAKE_SOURCE_ON,
    PM_WAKE_SOURCE_ACC,
    PM_WAKE_SOURCE_RTC,
    PM_WAKE_SOURCE_PWD,
    PM_WAKE_SOURCE_CAN,
    PM_WAKE_SOURCE_ANT,
    PM_WAKE_SOURCE_MOV,
    PM_WAKE_SOURCE_RING,
    PM_WAKE_SOURCE_LIGHT,
    PM_WAKE_SOURCE_REMOVED,
    PM_WAKE_SOURCE_MAX
} pm_wake_soruce_t;

UINT32 tbox_pm_io_get_wakesrc(VOID);
BOOL   tbox_pm_io_mainpower_is_active(VOID);
BOOL   tbox_pm_io_acc_is_active(VOID);
BOOL   tbox_pm_io_fcw_is_active(VOID);
BOOL   tbox_pm_io_doh_is_active(VOID);
BOOL   tbox_pm_io_dol_is_active(VOID);
BOOL   tbox_pm_io_is_removed(VOID);
VOID   tbox_pm_io_mpu_power_on(BOOL enable);
VOID   tbox_pm_io_mpu_sleep(VOID);
VOID   tbox_pm_io_mpu_wake(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_PM_IO_H */