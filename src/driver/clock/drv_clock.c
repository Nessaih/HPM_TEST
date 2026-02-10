#include "macros.h"

#if defined(AC7840X)
#include "clock_config_AC7840X.h"
#elif defined(AC7842X)
#include "clock_config_AC7842X.h"
#elif defined(AC7843X)
#include "clock_config_AC7843X.h"
#else
#error "Please select the target chip"
#endif

const Ckgen_ClkTreeCfgType clock_init_cfg = {
    .ClkSrcCfgCnt = ARRAY_SIZE(CLK_VHSI),
    .ClkSrcCfgs   = CLK_VHSI,
    .pllCfg       = &CLK_SPLL,
    .XoscCfg      = &CLK_HSE,
    .LPCfg        = &CLK_LPC,
    .ExtClkCfg    = &CLK_EXT,
};


const Ckgen_ClkDistributeCfgType clock_distribute_cfg = {
    .SysClkCfgCnt = ARRAY_SIZE(CLK_MODE),
    .ClkDivCfgCnt = ARRAY_SIZE(CLK_DIV),
    .ClkMuxCfgCnt = ARRAY_SIZE(CLK_DEV),
    .SysClkCfgs   = CLK_MODE,
    .ClkDivCfgs   = CLK_DIV,
    .ClkMuxCfgs   = CLK_DEV,
    .ClkoutCfg    = &CLK_OUT,
};

int32_t drv_clock_init(void)
{
    Ckgen_Hal_InitClk(&clock_init_cfg);
    Ckgen_Hal_DistributeClk(&clock_distribute_cfg);
    return 0;
}
