#ifndef __DRV_DMA_H__
#define __DRV_DMA_H__

#include <stdint.h>

int32_t drv_dma_init(void);
int32_t drv_dma_deinit(void);
int32_t drv_dma_wake(void);
int32_t drv_dma_sleep(void);

#endif //__DRV_DMA_H__