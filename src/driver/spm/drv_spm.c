#include <stdint.h>
#include "Spm_Hal.h"

static void spm_callback(void *args);

// clang-format off
static uint32_t             spm_ack_status;
static const Spm_ConfigType spm_cfg = {
    .SleepTimeoutAction  = SPM_INTERRUPT,
    .LvdLevel            = SPM_HIGH,
    .LvrLevel            = SPM_HIGH,
    .StandbyWakeupSource = 0x00U,
    .SpmCallback         = spm_callback,
    .LvdCallback         = NULL_PTR,
#if defined(AC7842X) || defined(AC7843X)
    .StandbyAction       = SPM_STANDBY_RESET,
    .StbCallback         = NULL_PTR,
#endif
};
// clang-format on

static void spm_callback(void *args)
{
    spm_ack_status = *(uint32_t *)args;
}

int32_t drv_spm_init(void)
{
    Spm_Hal_Init(&spm_cfg);
    return 0;
}

int32_t drv_spm_sleep(void)
{
    return Spm_Hal_SetPowerMode(SPM_MODE_VLPS);
}

void drv_spm_clr_status(void)
{
    spm_ack_status = 0;
}

uint32_t drv_spm_get_status(void)
{
    return spm_ack_status;
}
