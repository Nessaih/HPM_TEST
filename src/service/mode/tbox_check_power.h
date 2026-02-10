#ifndef TBOX_CHECK_POWER_H
#define TBOX_CHECK_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

VOID tbox_check_power_init(VOID);
VOID tbox_check_power_start(VOID);
VOID tbox_check_power_stop(VOID);
VOID tbox_check_period(VOID);
BOOL tbox_check_power_isundervoltage(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_CHECK_POWER_H */