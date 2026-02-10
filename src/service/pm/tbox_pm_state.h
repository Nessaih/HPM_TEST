#ifndef TBOX_PM_STATE_H
#define TBOX_PM_STATE_H

#ifdef _cplusplus
extern "C" {
#endif

typedef enum
{
    TBOX_PM_STATE_RUNNING = 0U,
    TBOX_PM_STATE_SLEEP_PRE_CHECK,
    TBOX_PM_STATE_SLEEP_POST_CHECK,
    TBOX_PM_STATE_DOACITON,
    TBOX_PM_STATE_FINISH,
    TBOX_PM_STATE_MAX
}TBOX_PM_STATE;

INT32 tbox_pm_state_init(VOID);
VOID tbox_pm_state_stop(VOID);
VOID tbox_pm_state_timeout(VOID);
TBOX_PM_STATE tbox_pm_state_get(VOID);

#ifdef _cplusplus
}
#endif

#endif /* TBOX_PM_STATE_H */