#ifndef __TBOX_PM_TIME_REBOOT_H__
#define __TBOX_PM_TIME_REBOOT_H__

/**
 * @brief 半夜重启逻辑
 * 1. ACC关闭，主电激活
 * 2. 3:00 - 4:00 之间
 * 3. 每天仅触发一次
 * 4. 非RTC唤醒时也需要重启，不重启会错过3:00 - 4:00 之间的RTC唤醒
 * 5. 3:00 正常工作，休眠时也进入重启流程
 */

#ifdef _cplusplus
extern "C"
{
#endif

VOID tbox_pm_time_reboot_init(VOID);
VOID tbox_pm_time_reboot_period(VOID);
VOID tbox_pm_time_reboot_stop(VOID);

#ifdef _cplusplus
}
#endif

#endif /* __TBOX_PM_TIME_REBOOT_H__ */