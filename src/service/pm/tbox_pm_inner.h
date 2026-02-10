#ifndef TBOX_PM_INNER_H
#define TBOX_PM_INNER_H

#ifdef __cplusplus
extern "C" {
#endif

VOID tbox_pm_start(VOID);
VOID tbox_pm_stop(VOID);
VOID tbox_pm_4g_tmout_notify(VOID);

INT32 tbox_pm_io_init(VOID);
VOID  tbox_pm_io_sleep(UINT32 set_wake_src);
VOID  tbox_pm_io_wake(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_PM_INNER_H */