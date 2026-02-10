#ifndef __CLOCK_CONFIG_AC7843X_H__
#define __CLOCK_CONFIG_AC7843X_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "Ckgen_Hal.h"

const Ckgen_ClkSrcCfgType CLK_VHSI[] = {
    {
     .Enable       = TRUE,
     .EnableInVLPS = FALSE,
     .OutputDiv1   = 2U,
     .OutputDiv2   = 1U,
     .Clk          = CKGEN_HSI_CLK,
     },
    {
     .Enable       = TRUE,
     .EnableInVLPS = FALSE,
     .OutputDiv1   = 2U,
     .OutputDiv2   = 1U,
     .Clk          = CKGEN_VHSI_CLK,
     }
};

const Ckgen_PllCfgType CLK_SPLL = {
    .Enable     = TRUE,
    .EnableLD   = TRUE,
    .OutputDiv1 = 2U,
    .OutputDiv2 = 2U,
    .ClkSrc     = CKGEN_HSE_CLK,
    .AdcSpllDiv = 2U,
    .DlySel     = CKGEN_PLL_LD_DLY_SEL_256CYCLE,
    .PreDiv     = CKGEN_PLL_PRE_DIV_1U,
    .FbkDiv     = 90U,
    .PosDiv     = CKGEN_PLL_POS_DIV_4U,

};

const Ckgen_XoscClkCfgType CLK_HSE = {
    .Enable        = TRUE,
    .EnableBypass  = FALSE,
    .EnableMonitor = TRUE,
    .OutputDiv1    = 2U,
    .OutputDiv2    = 1U,
    .Freq          = 8000000UL,
};

const Ckgen_LPClkCfgType CLK_LPC = {

    .LSIClkSrc = CKGEN_LSI_128K_CLK,
};

const Ckgen_ExternalClkFreqCfgType CLK_EXT = {
    .RtcClkInFreq   = 0U,
    .PwmExtClk0Freq = 0U,
    .PwmExtClk1Freq = 0U,
    .PwmExtClk2Freq = 0U,
};

const Ckgen_ClkoutCfgType CLK_OUT = {
    .Enable = FALSE,
    .Div    = CKGEN_CLKOUT_DIV_BY_2,
    .ClkSrc = CKGEN_SPLL_DIV2_CLK,
};

const Ckgen_ClkDivCfgType CLK_DIV[] = {
    {.Div = 3,  .Clk = CKGEN_CAN0_CLK},
    {.Div = 3,  .Clk = CKGEN_CAN1_CLK},
    {.Div = 3,  .Clk = CKGEN_CAN2_CLK},
    {.Div = 3,  .Clk = CKGEN_CAN3_CLK},
    {.Div = 3,  .Clk = CKGEN_CAN4_CLK},
    {.Div = 3,  .Clk = CKGEN_CAN5_CLK},
    {.Div = 1U, .Clk = CKGEN_PCT_CLK },
    {.Div = 1U, .Clk = CKGEN_TPIU_CLK}
};

const Ckgen_ClkMuxCfgType CLK_DEV[] = {

    {.Clk = CKGEN_TIMER_CLK, .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_PCT_CLK,   .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_EIO_CLK,   .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_I2C0_CLK,  .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_I2C1_CLK,  .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_I2C2_CLK,  .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_ADC0_CLK,  .ClkSrc = CKGEN_ADC_SPLLDIV_CLK},
    {.Clk = CKGEN_ADC1_CLK,  .ClkSrc = CKGEN_ADC_SPLLDIV_CLK},
    {.Clk = CKGEN_SPI0_CLK,  .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_SPI1_CLK,  .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_SPI2_CLK,  .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_SPI3_CLK,  .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_SPI4_CLK,  .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_CAN0_CLK,  .ClkSrc = CKGEN_SYS_CLK        },
    {.Clk = CKGEN_CAN1_CLK,  .ClkSrc = CKGEN_SYS_CLK        },
    {.Clk = CKGEN_CAN2_CLK,  .ClkSrc = CKGEN_SYS_CLK        },
    {.Clk = CKGEN_CAN3_CLK,  .ClkSrc = CKGEN_SYS_CLK        },
    {.Clk = CKGEN_CAN4_CLK,  .ClkSrc = CKGEN_SYS_CLK        },
    {.Clk = CKGEN_CAN5_CLK,  .ClkSrc = CKGEN_SYS_CLK        },
    {.Clk = CKGEN_UART0_CLK, .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_UART1_CLK, .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_UART2_CLK, .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_UART3_CLK, .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_UART4_CLK, .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_UART5_CLK, .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_UART6_CLK, .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_UART7_CLK, .ClkSrc = CKGEN_SPLL_DIV2_CLK  },
    {.Clk = CKGEN_PWM0_CLK,  .ClkSrc = CKGEN_SPLL_DIV1_CLK  },
    {.Clk = CKGEN_PWM1_CLK,  .ClkSrc = CKGEN_SPLL_DIV1_CLK  },
    {.Clk = CKGEN_PWM2_CLK,  .ClkSrc = CKGEN_SPLL_DIV1_CLK  },
    {.Clk = CKGEN_PWM3_CLK,  .ClkSrc = CKGEN_SPLL_DIV1_CLK  },
    {.Clk = CKGEN_PWM4_CLK,  .ClkSrc = CKGEN_SPLL_DIV1_CLK  },
    {.Clk = CKGEN_PWM5_CLK,  .ClkSrc = CKGEN_SPLL_DIV1_CLK  },
    {.Clk = CKGEN_PWM6_CLK,  .ClkSrc = CKGEN_SPLL_DIV1_CLK  },
    {.Clk = CKGEN_PWM7_CLK,  .ClkSrc = CKGEN_SPLL_DIV1_CLK  },
};

const Ckgen_SysClkCfgType CLK_MODE[] = {
    {
     .Mode   = CKGEN_SYS_CLK_MODE_RUN,
     .ClkSrc = CKGEN_SPLL_CLK,
     .SysDiv = 1U,
     .BusDiv = 2U,
     },
    {
     .Mode   = CKGEN_SYS_CLK_MODE_VLPR,
     .ClkSrc = CKGEN_HSI_CLK,
     .SysDiv = 1U,
     .BusDiv = 2U,
     }
};
