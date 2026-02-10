#ifndef TBOX_4G_POWER_H
#define TBOX_4G_POWER_H

#include "4g_if.h"

typedef enum
{
    POWER_4G_SLP_SLEEP = IF_4G_SLP_TYPE_SLEEP,
    POWER_4G_SLP_SHUTDOWN = IF_4G_SLP_TYPE_SHUTDOWN,
    POWER_4G_SLP_SHEEL_SLEEP = IF_4G_SLP_TYPE_SHELL_SLEEP,
    POWER_4G_SLP_SHEEL_SHUWDOWN = IF_4G_SLP_TYPE_SHELL_SHUTDOWN,
    POWER_4G_SLP_UNKNOWN
}POWER_4G_SLEEP_TYPE;

void power_4g_init(void);

void power_4g_reset(void);

void power_4g_periodic(void);

int power_4g_reg_depend_fun(IF_4G_SLEEP_DEPEND_FUN fun);

void power_4g_sleep(void);

void power_4g_wakeup(void);

void powersupply_periodic(void);

#endif /* TBOX_4G_POWER_H */
