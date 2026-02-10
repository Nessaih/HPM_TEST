#ifndef __DMA_CONFIG_H__
#define __DMA_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off

#define DRV_DMA_CHANNEL_MASK                                     0x0FU



#define DRV_DMA_DEFFINE(channel, index, priority, source)                           \
    {                                                                               \
        (DRV_DMA_CHANNEL_MASK) & (channel),                                         \
        (DRV_DMA_CHANNEL_MASK) & (index),                                           \
        (Dma_ChannelPriorityType)(priority),                                        \
        (Dma_RequestSourceType)  (source)                                           \
    }


#define DRV_DMA_CHANNEL_CONFIG_TABLE                                                \
    {                                                                               \
        DRV_DMA_DEFFINE(0U, 0U, DMA_CHANNEL_PRIORITY_MEDIUM, DMA_REQ_SPI0_RX),      \
        DRV_DMA_DEFFINE(1U, 1U, DMA_CHANNEL_PRIORITY_MEDIUM, DMA_REQ_SPI0_TX),      \
        DRV_DMA_DEFFINE(2U, 2U, DMA_CHANNEL_PRIORITY_MEDIUM, DMA_REQ_SPI1_RX),      \
        DRV_DMA_DEFFINE(3U, 3U, DMA_CHANNEL_PRIORITY_MEDIUM, DMA_REQ_SPI1_TX),      \
    }

// clang-format on

#ifdef __cplusplus
}
#endif

#endif //__DMA_CONFIG_H__