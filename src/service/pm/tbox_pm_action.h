#ifndef TBOX_PM_ACTION_H
#define TBOX_PM_ACTION_H

#ifdef __cplusplus
extern "C" {
#endif

VOID tbox_pm_action_init(VOID);
VOID tbox_pm_action_start(VOID);
VOID tbox_pm_action_stop(VOID);
VOID tbox_pm_action_period(VOID);
VOID tbox_pm_abort_action(VOID);
VOID tbox_pm_clear_abort_action(VOID);
VOID tbox_pm_force_shutdown(VOID);
VOID tbox_pm_force_sleep(VOID);
INT32 tbox_pm_action_do(TBOX_PM_SLEEPPOST_ACTION type);
BOOL tbox_pm_action_is_cando(TBOX_PM_SLEEPPOST_ACTION type);
VOID tbox_pm_action_cleanresetinfo(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_PM_ACTION_H */