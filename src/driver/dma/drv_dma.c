#include <stdint.h>
#include "AC784xx_Dma_Reg.h"
#include "Dma_Hal.h"
#include "dma_config.h"
#include "macros.h"

static const Dma_ChannelConfigType channel_cfg[] = DRV_DMA_CHANNEL_CONFIG_TABLE;

int32_t drv_dma_init(void)
{
    Dma_ConfigType dma_cfg;

    dma_cfg.ChannelCnt = (uint8_t)ARRAY_SIZE(channel_cfg);
    dma_cfg.ChannelCfg = channel_cfg;
    Dma_Hal_Init(&dma_cfg);
    Dma_Reg_SetChannelDebug(0, TRUE);
    Dma_Reg_SetChannelDebug(1, TRUE);

    return 0;
}

int32_t drv_dma_deinit(void)
{
    Dma_Hal_Deinit();
    return 0;
}

int32_t drv_dma_wake(void)
{
    drv_dma_init();
    return 0;
}

int32_t drv_dma_sleep(void)

{
    drv_dma_deinit();
    return 0;
}