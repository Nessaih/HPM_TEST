#ifndef __CAN_CONIG_H__
#define __CAN_CONIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "Can_Hal.h"
#include "macros.h"


#define DRV_CAN_FILTER_ENABLE 0
#define DRV_CAN_BUSERR_ENABLE 1
#define DRV_CAN_WAKEUP_ENABLE 1

#define DRV_CAN_RATE_250K     0
#define DRV_CAN_RATE_500K     1
#define DRV_CAN_RATE_1000K    2

// clang-format off
#if DRV_CAN_BUSERR_ENABLE
#define DRV_CAN_IRQ_EN_MASK               CAN_COMMON_IRQ_EN_MASKS
#else
#define DRV_CAN_IRQ_EN_MASK               (                                                     \
                                            CAN_CTRL1_EIE_Msk   |                               \
                                            CAN_CTRL1_TSIE_Msk  |                               \
                                            CAN_CTRL1_TPIE_Msk  |                               \
                                            CAN_CTRL1_RAFIE_Msk |                               \
                                            CAN_CTRL1_RFIE_Msk  |                               \
                                            CAN_CTRL1_ROIE_Msk  |                               \
                                            CAN_CTRL1_RIE_Msk                                   \
                                            )
#endif
// clang-format on

extern const Can_BaudrateConfigType can_rate_list[];
extern const Can_ExtendModeType     can_mode_list[];
extern const Can_HalConfigType      can_init_config;

#ifdef __cplusplus
}
#endif

#endif 