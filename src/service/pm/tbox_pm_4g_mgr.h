#ifndef TBOX_PM_4G_MGR_H
#define TBOX_PM_4G_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

#define TBOX_PM_4G_STARTUP_MAX_TIME  2500U    /*2500ms*/
#define TBOX_PM_4G_SHUTDOWN_MAX_TIME 3500U    /*3500ms*/
#define TBOX_PM_4G_RESET_MAX_TIME    500U     /*500ms*/
#define TBOX_PM_4G_DEEPRESET_MAX_TIME 210000U /*210000ms(210s)*/

typedef enum
{
    TBOX_PM_4G_DO_NOTHING = 0,
    TBOX_PM_4G_DO_STARTUP,
    TBOX_PM_4G_DO_SHUTDOWN,
    TBOX_PM_4G_DO_RESET,
    TBOX_PM_4G_DO_DEEPRESET,
}TBOX_PM_4G_DO_STATE;

INT32 tbox_pm_4g_mgr_init(VOID);
VOID tbox_pm_4g_mgr_deinit(VOID);
VOID tbox_pm_4g_mgr_start(VOID);
VOID tbox_pm_4g_mgr_stop(VOID);
VOID tbox_pm_4g_mgr_stop_check_startup(VOID);
VOID tbox_pm_4g_mgr_period(VOID);
INT32 tbox_pm_4g_do_startup(VOID);
INT32 tbox_pm_4g_do_shutdown(VOID);
INT32 tbox_pm_4g_do_reset(VOID);
INT32 tbox_pm_4g_do_deepreset(VOID);
VOID tbox_pm_4g_do_timeout(VOID);
TBOX_PM_4G_DO_STATE tbox_pm_4g_get_do_state(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_PM_4G_MGR_H */