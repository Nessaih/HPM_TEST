#ifndef TBOX_PM_IF_H
#define TBOX_PM_IF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    TBOX_PM_REBOOT_NONE = 0U,
    TBOX_PM_REBOOT_MCU,
    TBOX_PM_REBOOT_4G,
    TBOX_PM_DEEPREBOOT_4G,
    TBOX_PM_REBOOT_4G_MCU
}TBOX_PM_REBOOT_TYPE;

typedef enum
{
    TBOX_PM_WAKEUP_BY_KEY = 0U,
    TBOX_PM_WAKEUP_BY_PERIPHERAL,
    TBOX_PM_WAKEUP_BY_TIMER,
    TBOX_PM_WAKEUP_BY_4G
}TBOX_PM_WAKEUP_TYPE;

typedef enum
{
    TBOX_PM_SLEEPPOST_ACTION_NONE = 0U,
    TBOX_PM_SLEEPPOST_ACTION_SLEEP,
    TBOX_PM_SLEEPPOST_ACTION_SHUTDOWN,    
    TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G,
    TBOX_PM_SLEEPPOST_ACTION_DEEPREBOOT_4G,
    TBOX_PM_SLEEPPOST_ACTION_REBOOT_MCU,
    TBOX_PM_SLEEPPOST_ACTION_REBOOT_4G_MCU
}TBOX_PM_SLEEPPOST_ACTION;

INT32 tbox_pm_reboot(TBOX_PM_REBOOT_TYPE type);
BOOL  tbox_pm_is_reboot_finish(TBOX_PM_REBOOT_TYPE type);
INT32 tbox_pm_wakeup(TBOX_PM_WAKEUP_TYPE type);
VOID  tbox_pm_fctsleep(VOID);
VOID  tbox_pm_4g_startup(VOID);
VOID  tbox_pm_4g_shutdown(VOID);
TBOX_PM_SLEEPPOST_ACTION tbox_pm_get_sleeppost_action(VOID);
BOOL  tbox_pm_is_reboot_same_date(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_PM_IF_H */