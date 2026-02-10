
#include <stddef.h>
#include "can_config.h"

// clang-format off
#if DRV_CAN_FILTER_ENABLE
    
    #if defined(AC7840X) || defined(AC7842X)
        
        const Can_FilterParamsType can_filter_list[] = {
            {0x123U,       0U,           STD_FRAME}, 
            {0x456U,       1U,           STD_FRAME}, 
            {0x10001234UL, 2U,           EXT_FRAME}, 
            {0x678U,       0x7FFU,       STD_FRAME}, 
            {0x615U,       0x7F0U,       STD_FRAME}, 
            {0x667U,       0x7F8U,       STD_FRAME}, 
            {0x770U,       0x77FU,       STD_FRAME}, 
            {0x710U,       0x711U,       STD_FRAME}, 
            {0x720U,       0x720U,       STD_FRAME}, 
            {0x00000770U,  0x0000077FU,  EXT_FRAME}, 
            {0x00000710U,  0x0000071FU,  EXT_FRAME}, 
            {0x1FF01234UL, 0x1FFFFFFFUL, EXT_FRAME}, 
            {0x1FF56789UL, 0x1FFFFFF0UL, EXT_FRAME}, 
            {0x1FF5A987UL, 0x1FF6FFFFUL, EXT_FRAME}, 
        };
    #elif defined(AC7843X)
        
        const Can_FilterParamsType can_filter_list[] = {
            
            {0x123U,       0U,           STD_FRAME, FILTER_CLASSIC,           STORE_IN_RXBUFFER}, 
            {0x456U,       1U,           STD_FRAME, FILTER_CLASSIC,           STORE_IN_RXBUFFER}, 
            {0x10001234UL, 2U,           EXT_FRAME, FILTER_CLASSIC,           STORE_IN_RXBUFFER}, 

            {0x678U,       0x7FFU,       STD_FRAME, FILTER_CLASSIC,           STORE_IN_RXFIFO0 }, 
            {0x615U,       0x7F0U,       STD_FRAME, FILTER_CLASSIC,           STORE_IN_RXFIFO0 }, 
            {0x667U,       0x7F8U,       STD_FRAME, FILTER_CLASSIC,           STORE_IN_RXFIFO0 }, 

            {0x770U,       0x77FU,       STD_FRAME, FILTER_RANGE,             STORE_IN_RXFIFO0 }, 
            {0x710U,       0x711U,       STD_FRAME, FILTER_DUAL,              STORE_IN_RXFIFO0 }, 
            {0x720U,       0x720U,       STD_FRAME, FILTER_DUAL,              STORE_IN_RXFIFO0 }, 
            {0x00000770U,  0x0000077FU,  EXT_FRAME, FILTER_RANGE,             STORE_IN_RXFIFO0 }, 
            {0x00000710U,  0x0000071FU,  EXT_FRAME, STD_DISABLE_OR_EXT_RANGE, STORE_IN_RXFIFO0 }, 

            {0x1FF01234UL, 0x1FFFFFFFUL, EXT_FRAME, FILTER_CLASSIC,           STORE_IN_RXFIFO1 }, 
            {0x1FF56789UL, 0x1FFFFFF0UL, EXT_FRAME, FILTER_CLASSIC,           STORE_IN_RXFIFO1 }, 
            {0x1FF5A987UL, 0x1FF6FFFFUL, EXT_FRAME, FILTER_CLASSIC,           STORE_IN_RXFIFO1 }, 
        };
    #endif 

    #if defined(AC7840X) || defined(AC7842X)
    #define filter_config                                                                           \
    {                                                                                               \
        .FiltersNum                      = ARRAY_SIZE(can_filter_list),                             \
        .FiltersParamsPtr                = (Can_FilterParamsType *)can_filter_list,                 \
    }
    #elif defined(AC7843X)                                                                          \

    #define filter_config                                                                           \
    {                                                                                               \
        .FiltersNum                      = ARRAY_SIZE(can_filter_list),                             \
        .FiltersParamsPtr                = (Can_FilterParamsType *)can_filter_list,                 \
        .GlobalFilter.rejectStdRemote    = TRUE,                                                    \
        .GlobalFilter.rejectExtRemote    = TRUE,                                                    \
        .GlobalFilter.std                = REJECT_NOMATCH_ID,                                       \
        .GlobalFilter.ext                 = REJECT_NOMATCH_ID,                                       \
        .XIDAM                           = CAN_XIDAM_EIDM_Msk,                                      \
    }

    #endif // defined(AC7843X)


#endif // DRV_CAN_FILTER_ENABLE



const Can_BaudrateConfigType can_rate_list[] = {
    {.NormalBitrate = {.Presc = 5, .Seg1 = 33, .Seg2 = 4, .Sjw = 3}},
    {.NormalBitrate = {.Presc = 2, .Seg1 = 33, .Seg2 = 4, .Sjw = 3}},
    {.NormalBitrate = {.Presc = 0, .Seg1 = 50, .Seg2 = 7, .Sjw = 3}},
};

const Can_ExtendModeType can_mode_list[] = {
    CAN_EXTMODE_OFF,
    CAN_EXTMODE_LISTENING,
    CAN_EXTMODE_LOOPBACK_INTERNEL
};

const Can_HalConfigType can_init_config = {
        .EccEn                           = TRUE,
        .FdEn                            = FALSE,
        .FdIsoEn                         = TRUE,
        .RxOverWrite                     = TRUE,
        .RxFifoBufDmaConfigNum           = 0,
        .RxFifoBufDmaConfigPtr           = NULL,
        .BaudrateConfigPtr               = NULL,
#if DRV_CAN_FILTER_ENABLE
        .Filters                         = filter_config,
#else
        .Filters                         = {0},
#endif 
        .WakeupIrqCallback               = NULL,
        .IrqCallback                     = NULL,
        .CommonIrqEnMasks                = DRV_CAN_IRQ_EN_MASK,
#if defined(AC7840X)
        .BOREC                           = TRUE,
        .TxSecAmount                     = CAN_TX_SEC_ALL,
        .EccIrqEnMasks                   = 0,
#elif defined(AC7842X)
        .BOREC                           = TRUE,
        .TxSecAmount                     = CAN_TX_SEC_ALL,
        .EccIrqEnMasks                   = 0,
#elif defined(AC7843X)
        // TODO: not defined yet
        .RxBuffersNum                    = 0,
        .TxBuffersNum                    = 0,
        .RxBufferConfigPtr               = 0,
        .TxBufferConfigPtr               = 0,
        .RxBuffersInfoPtr                = 0,
        .TxBuffersInfoPtr                = 0,
        .DataSize                        = 0,
        .TxEventFifoNum                  = 0,
        .TxEventFifoWm                   = 0,
#else
    #error "Please select a chip"
#endif
    };


// clang-format on