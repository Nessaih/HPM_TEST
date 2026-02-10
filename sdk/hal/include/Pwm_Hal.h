/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2021. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef PWM_HAL_H
#define PWM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Pwm_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief: Pwm_Hal_InitOutputMode: init pwm output mode
 * @note Function ID: DES_PWM_API_201
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Config: pwm output config pointer
 * @return: void
 */
void Pwm_Hal_InitOutputMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_OutputCfg *Config
);

/**
 * @brief: Pwm_Hal_InitOutputMode: init pwm input mode
 * @note Function ID: DES_PWM_API_202
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Config: pwm input config pointer
 * @return: void
 */
void Pwm_Hal_InitInputMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_InputCfg *Config
);

#ifndef PWM_SDK_NON_EXTENDED_API
/*!
 * @brief: Pwm_Hal_InitQuadDecoderMode: init pwm quadrature decoder mode
 * @note Function ID: DES_PWM_API_261
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Config: pwm quadrature decoder config pointer
 * @return: void
 */
void Pwm_Hal_InitQuadDecoderMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_QuadDecoderCfg *Config
);
#endif

/**
 * @brief: Pwm_Hal_DeInit: deinit pwm module
 * @note Function ID: DES_PWM_API_203
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: void
 */
void Pwm_Hal_DeInit
(
    Pwm_Hal_InstanceType Instance
);

#ifndef PWM_SDK_NON_EXTENDED_API
/**
 * @brief: Pwm_Hal_SetClockSource: Set pwm module clock source and clock psc
 * @note Function ID: DES_PWM_API_204
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] ClkSource: clock source type
 * @param[in] ClkPsc: clock prescaler
 * @return: void
 */
void Pwm_Hal_SetClockSource
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ClockSourceType ClkSource,
    uint16 ClkPsc
);

/**
 * @brief: Pwm_Hal_ResetCounter: reset module counter value
 * @note Function ID: DES_PWM_API_212
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: void
 */
void Pwm_Hal_ResetCounter
(
    Pwm_Hal_InstanceType Instance
);
#endif

/**
 * @brief: Pwm_Hal_GetCountValue: get module counter value
 * @note Function ID: DES_PWM_API_211
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint16: counter value
 */
uint16 Pwm_Hal_GetCountValue
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief: Pwm_Hal_SetMaxCountValue: Set pwm module counter max value.
 * @note Function ID: DES_PWM_API_208
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Value: max value
 * @return: void
 */
void Pwm_Hal_SetMaxCountValue
(
    Pwm_Hal_InstanceType Instance,
    uint16 Value
);

/**
 * @brief: Pwm_Hal_GetMaxCountValue: Get pwm module counter max value.
 * @note Function ID: DES_PWM_API_207
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint16:max value
 */
uint16 Pwm_Hal_GetMaxCountValue
(
    Pwm_Hal_InstanceType Instance
);

#ifndef PWM_SDK_NON_EXTENDED_API
/**
 * @brief: Pwm_Hal_SetInitCountValue: Set pwm module counter begin value.
 * @note Function ID: DES_PWM_API_210
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Value: begin value
 * @return: void
 */
void Pwm_Hal_SetInitCountValue
(
    Pwm_Hal_InstanceType Instance,
    uint16 Value
);

/**
 * @brief: Pwm_Hal_GetInitCountValue: Get pwm module counter init value.
 * @note Function ID: DES_PWM_API_209
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint16: Init value
 */
uint16 Pwm_Hal_GetInitCountValue
(
    Pwm_Hal_InstanceType Instance
);
#endif

/**
 * @brief: Pwm_Hal_SetChannelValue: Set channel value.
 * @note Function ID: DES_PWM_API_206
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Value: Channel value
 * @return: void
 */
void Pwm_Hal_SetChannelValue
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint16 Value
);

/**
 * @brief: Pwm_Hal_GetChannelValue: Get channel match value.
 * @note Function ID: DES_PWM_API_205
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @return: Channel value.
 */
uint16 Pwm_Hal_GetChannelValue
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel
);

/**
 * @brief: Pwm_Hal_EnableChannelInterrupt: enable or disable pwm channel match Interrupt
 * @note Function ID: DES_PWM_API_214
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableChannelInterrupt
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief: Pwm_Hal_EnableOverflowInterrupt: enable or disable pwm module overflow Interrupt.
 * @note Function ID: DES_PWM_API_213
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableOverflowInterrupt
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

#ifndef PWM_SDK_NON_EXTENDED_API
/**
 * @brief: Pwm_Hal_EnableOverflowEvent: enable or disable pwm module overflow event.
 * @note Function ID: DES_PWM_API_215
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableOverflowEvent
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief: Pwm_Hal_EnableUnderflowEvent: enable or disable pwm module under overflow event.
 * @note Function ID: DES_PWM_API_216
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableUnderflowEvent
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);
#endif

/**
 * @brief: Pwm_Hal_GetChannelInterruptFlag: get all pwm interrupt flag
 * @note Function ID: DES_PWM_API_217
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint32: interrupt flag
 */
uint32 Pwm_Hal_GetChannelInterruptFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief: Pwm_Hal_ClearChannelInterruptFlag: clear all channel interrupt flag
 * @note Function ID: DES_PWM_API_218
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Mask: clear mask
 * @return: void
 */
void Pwm_Hal_ClearChannelInterruptFlag
(
    Pwm_Hal_InstanceType Instance,
    uint32 Mask
);

#ifndef PWM_SDK_NON_EXTENDED_API
/**
 * @brief: Pwm_Hal_GetOverflowFlag: get all pwm interrupt flag
 * @note Function ID: DES_PWM_API_219
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint32: interrupt flag
 */
uint32 Pwm_Hal_GetOverflowFlag
(
    Pwm_Hal_InstanceType Instance
);
#endif

/**
 * @brief: Pwm_Hal_ClearOverFlowFlag: clear pwm OverFlow flag
 * @note Function ID: DES_PWM_API_220
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: void
 */
void Pwm_Hal_ClearOverflowFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief: Pwm_Hal_GetOverFlowDir: get pwm Overflow direction
 * @note Function ID: DES_PWM_API_221
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint32: channel OverFlow flag
 */
uint32 Pwm_Hal_GetOverflowDir
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief: Pwm_Hal_GetAllChannelLevel: get all pwm channel level
 * @note Function ID: DES_PWM_API_222
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint32: channel level
 */
uint32 Pwm_Hal_GetAllChannelLevel
(
    Pwm_Hal_InstanceType Instance
);

#ifndef PWM_SDK_NON_EXTENDED_API
/**
 * @brief: Pwm_Hal_EnableChannelDmaRequest: Enable DMA requests for channel events
 * @note Function ID: DES_PWM_API_223
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableChannelDmaRequest
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);
#endif

/*!
 * @brief: Pwm_Hal_InitSyncConfigSet: PWM synchronization control init
 * @note Function ID: DES_PWM_API_224
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] ConfigPtr: pointer to configuration structure
 * @return: void
 */
void Pwm_Hal_InitSyncConfigSet
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_SyncCfg *ConfigPtr
);

/*!
 * @brief: Pwm_Hal_EnableSync: Enable/disable sync.
 * @note Function ID: DES_PWM_API_268
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableSync
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_EnableSyncBypass: CHnV/CNTIN/MCVR sync bypass.
 * @note Function ID: DES_PWM_API_225
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableSyncBypass
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief: Pwm_Hal_TrigSoftwareSync: start software sync trigger
 * @note Function ID: DES_PWM_API_226
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] IsTrigger: true->enable,flase->disable
 * @return: void
 */
void Pwm_Hal_TrigSoftwareSync
(
    Pwm_Hal_InstanceType Instance,
    boolean IsTrigger
);

#ifndef PWM_SDK_NON_EXTENDED_API
/*!
 * @brief: Pwm_Hal_EnableGlobalTimeBase: Enable/Disable global timer base.
 * @note Function ID: DES_PWM_API_227
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableGlobalTimeBase
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_EnableGlobalTimeBaseOutput: Global timer base output enable/disable.
 * @note Function ID: DES_PWM_API_228
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableGlobalTimeBaseOutput
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_EnableInitTrigger: Enable initialization trigger source.
 * @note Function ID: DES_PWM_API_229
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableInitTrigger
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_EnableMaxTrigger: Enable max trigger source.
 * @note Function ID: DES_PWM_API_230
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableMaxTrigger
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_EnableChannelMatchTrigger: Enable Channel match trigger source.
 * @note Function ID: DES_PWM_API_231
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableChannelMatchTrigger
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_SetTriggerRatio: Set the max/match/init trigger ratio.
 * @note Function ID: DES_PWM_API_232
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] TriggerRatio: 0 ~ 7
 * @return: void
 */
void Pwm_Hal_SetTriggerRatio
(
    Pwm_Hal_InstanceType Instance,
    uint8 TriggerRatio
);
#endif

/**
 * @brief: Pwm_Hal_SetChannelCaptureEdge: set input channel capture edge
 * @note Function ID: DES_PWM_API_233
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Edge: Enum Pwm_Hal_EdgeType
 * @return: void
 */
void Pwm_Hal_SetChannelCaptureEdge
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_EdgeType Edge
);

#ifndef PWM_SDK_NON_EXTENDED_API
/*!
 * @brief: Pwm_Hal_SetChannelInputFilterVal: Set channel input filter value.
 * @note Function ID: DES_PWM_API_234
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Value: filter value(0~15)
 * @return: void
 */
void Pwm_Hal_SetChannelInputFilterVal
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint8 Value
);

/*!
 * @brief: Pwm_Hal_SetInputFilterPsc: Set pwm input filter psc value.
 * @note Function ID:
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Prescaler: filter psc(0 ~ 111)
 * @return: void
 */
void Pwm_Hal_SetInputFilterPsc
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_InputFilterPscType Prescaler
);

/**
 * @brief: Pwm_Hal_EnableChannelEventReset: Does the channel event reset the counter.
 * @note Function ID: DES_PWM_API_235
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableChannelEventReset
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief: Pwm_Hal_GetHallStatus: Get pwm hall status.
 * @note Function ID: DES_PWM_API_236
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: Hall status.
 */
uint32 Pwm_Hal_GetHallStatus
(
    Pwm_Hal_InstanceType Instance
);
#endif

/**
 * @brief: Pwm_Hal_SetChannelCompareAction: Set matching action for output comparison mode
 * @note Function ID: DES_PWM_API_237
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Action: Enum Pwm_Hal_CompareActionType
 * @return: void
 */
void Pwm_Hal_SetChannelCompareAction
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_CompareActionType Action
);

#ifndef PWM_SDK_NON_EXTENDED_API
/**
 * @brief: Pwm_Hal_SetSoftControlEnableStates: Set the software output enable status for each channel.
 * @note Function ID: DES_PWM_API_238
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] EnableMask: Status masks for each channel
 * @return: void
 */
void Pwm_Hal_SetSoftControlEnableStates
(
    Pwm_Hal_InstanceType Instance,
    uint8 EnableMask
);
#endif

/**
 * @brief: Pwm_Hal_EnableChannelSoftControl: Enable channel soft control
 * @note Function ID: DES_PWM_API_239
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableChannelSoftControl
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief: Pwm_Hal_EnableCombineChannelSoftControl: enable soft control function
 * @note Function ID: DES_PWM_API_240
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\2\4\6
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableCombineChannelSoftControl
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

#ifndef PWM_SDK_NON_EXTENDED_API
/**
 * @brief: Pwm_Hal_SetSoftControlLevels: Set software output levels for all channels
 * @note Function ID: DES_PWM_API_241
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] LevelMask: software output levels for all channels
 * @return: void
 */
void Pwm_Hal_SetSoftControlLevels
(
    Pwm_Hal_InstanceType Instance,
    uint8 LevelMask
);
#endif

/**
 * @brief: Pwm_Hal_SetChannelSoftControlLevel: clear all pwm interrupt flag
 * @note Function ID: DES_PWM_API_242
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Level: Output level
 * @return: void
 */
void Pwm_Hal_SetChannelSoftControlLevel
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_OutputLevelType Level
);

#ifndef PWM_SDK_NON_EXTENDED_API
/*!
 * @brief: Pwm_Hal_SetChannelMatchDither: Set channel dither value.
 * @note Function ID: DES_PWM_API_243
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] DitherValue: Channel match dither value.
 * @return: void
 */
void Pwm_Hal_SetChannelMatchDither
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint8 DitherValue
);

/*!
 * @brief: PWM_DRV_SetMaxCountDitherValue: Set mod dither value.
 * @note Function ID: DES_PWM_API_244
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] MaxCountDitherValue: mod dither value (0 ~ 31)
 * @return: void
 */
void Pwm_Hal_SetPeriodDither
(
    Pwm_Hal_InstanceType Instance,
    uint8 MaxCountDitherValue
);

/**
 * @brief: Pwm_Hal_EnableChannelOutputMask: Shielding channel output or not
 * @note Function ID: DES_PWM_API_245
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableChannelOutputMask
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief: Pwm_Hal_SetOutputMask: Set the mask status for all channels
 * @note Function ID: DES_PWM_API_247
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Mask: The shielding status of all channels
 * @return: void
 */
void Pwm_Hal_SetOutputMask
(
    Pwm_Hal_InstanceType Instance,
    uint8 Mask
);

/**
 * @brief: Pwm_Hal_GetOutputMask: get pwm moudel channel output status
 * @note Function ID: DES_PWM_API_246
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint32: channel output state
 */
uint32 Pwm_Hal_GetOutputMask
(
    Pwm_Hal_InstanceType Instance
);

/*!
 * @brief: Pwm_Hal_SetChannelPolarity: Set channel polarity.
 * @note Function ID: DES_PWM_API_248
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Polarity: Output polarity.
 * @return void
 */
void Pwm_Hal_SetChannelPolarity
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_ActivePolarityType Polarity
);

/*!
 * @brief: Pwm_Hal_SetDeadtime: Set deadtime prescaler & value.
 * @note Function ID: DES_PWM_API_249
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] ChannelPair: PWM_CHANNEL_PAIR_0\1\2\3
 * @param[in] Prescaler: prescaler divider
 *            - PWM_DEADTIME_DIVID_1
 *            - PWM_DEADTIME_DIVID_4
 *            - PWM_DEADTIME_DIVID_16
 * @param[in] Value: inserted value
 *            - 0 ~ 63
 * @return void
 */
void Pwm_Hal_SetDeadtime
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    Pwm_Hal_DeadTimePscType Prescaler,
    uint16 Value
);

/*!
 * @brief: Pwm_Hal_EnableChannelPairSymmetric: Set pair channel symmetric.
 * @note Function ID: DES_PWM_API_250
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] ChannelPair: PWM_CHANNEL_PAIR_0\1\2\3
 * @param[in] Enable: TRUE/FALSE
 * @return void
 */
void Pwm_Hal_EnableChannelPairSymmetric
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_EnableChannelPairInvert: Set channel inverting control.
 * @note Function ID: DES_PWM_API_251
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] ChannelPair: PWM_CHANNEL_PAIR_0\1\2\3
 * @param[in] Enable: TRUE/FALSE
 * @return void
 */
void Pwm_Hal_EnableChannelPairInvert
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_InitFaultControl: Init fault control.
 *         Only applicable to PWM modulation output mode, called after PWM_Init().
 * @note Function ID: DES_PWM_API_252
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] ConfigPtr: Pointer to configuration structure
 * @return void
 */
void Pwm_Hal_InitFaultControl
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_FaultCfg *ConfigPtr
);

/*!
 * @brief: Pwm_Hal_EnableFaultPinInput: Enable this fault input pin.
 * @note Function ID: DES_PWM_API_253
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] FaultInputId: PWM_FAULT_INPUT_0\1\2\3
 * @param[in] Enable: TRUE/FALSE
 * @return void
 */
void Pwm_Hal_EnableFaultPinInput
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_EnableFaultInputFilter: Enable the filtering function of the fault input pin.
 * @note Function ID: DES_PWM_API_254
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] FaultInputId: PWM_FAULT_INPUT_0\1\2\3
 * @param[in] Enable: TRUE/FALSE
 * @return void
 */
void Pwm_Hal_EnableFaultInputFilter
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_SetFaultInputPolarity: Set fault input polarity.
 * @note Function ID: DES_PWM_API_255
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] FaultInputId: PWM_FAULT_INPUT_0\1\2\3
 * @param[in] ActivePolarity: PWM_ACTIVE_POLARITY_HIGH / PWM_ACTIVE_POLARITY_LOW
 * @return void
 */
void Pwm_Hal_SetFaultInputPolarity
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId,
    Pwm_Hal_ActivePolarityType ActivePolarity
);

/*!
 * @brief: Pwm_Hal_GetFaultPinFlag: Get fault pin detection flag.
 * @note Function ID: DES_PWM_API_256
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] FaultInputId: PWM_FAULT_INPUT_0\1\2\3
 * @return Fault pin detection flag.
 */
uint32 Pwm_Hal_GetFaultPinFlag
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId
);

/*!
 * @brief: Pwm_Hal_ClearFaultPinFlag: Clear fault pin detection flag.
 * @note Function ID: DES_PWM_API_257
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] FaultInputId: PWM_FAULT_INPUT_0\1\2\3
 * @return void
 */
void Pwm_Hal_ClearFaultPinFlag
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId
);

/*!
 * @brief: Pwm_Hal_GetFaultFlag: Get the OR value of each fault input pin flag.
 * @note Function ID: DES_PWM_API_258
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return The OR value of each fault input pin flag
 */
uint32 Pwm_Hal_GetFaultFlag
(
    Pwm_Hal_InstanceType Instance
);

/*!
 * @brief: Pwm_Hal_ClearFaultFlag: Clear the OR value of each fault input pin flag.
 * @note Function ID: DES_PWM_API_259
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return void
 */
void Pwm_Hal_ClearFaultFlag
(
    Pwm_Hal_InstanceType Instance
);

/*!
 * @brief: Pwm_Hal_SetChannelHizOutput: Enable or disable channel high-Z output
 * @note Function ID: DES_PWM_API_260
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] Enable: TRUE/FALSE
 * @return: void
 */
void Pwm_Hal_EnableChannelHizOutput
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_GetQuadCountingDir: Get the current quadrature decoding count direction.
 * @note Function ID: DES_PWM_API_262
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return counting direction
 */
uint32 Pwm_Hal_GetQuadCountingDir
(
    Pwm_Hal_InstanceType Instance
);

/*!
 * @brief: Pwm_Hal_GetQuadOverflowDir: Get the quadrature timer overflow direction.
 * @note Function ID: DES_PWM_API_263
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return timer overflow direction
 */
uint32 Pwm_Hal_GetQuadOverflowDir
(
    Pwm_Hal_InstanceType Instance
);

/*!
 * @brief: Pwm_Hal_GetQuadPhaseZFlag: Get the phaseZ Status.
 * @note Function ID: DES_PWM_API_264
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return phaseZ Status
 */
uint32 Pwm_Hal_GetQuadPhaseZFlag
(
    Pwm_Hal_InstanceType Instance
);

/*!
 * @brief: Pwm_Hal_ClearQuadPhaseZFlag: Clear the phaseZ Status.
 * @note Function ID: DES_PWM_API_265
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return void
 */
void Pwm_Hal_ClearQuadPhaseZFlag
(
    Pwm_Hal_InstanceType Instance
);

/*!
 * @brief: Pwm_Hal_GetDetectEventFlag: Check Z index detect event status.
 * @note Function ID:
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return Z index event status
 */
uint32 Pwm_Hal_GetDetectEventFlag
(
    Pwm_Hal_InstanceType Instance
);

/*!
 * @brief: Pwm_Hal_ClearDetectEventFlag: Clear Z index detect event status.
 * @note Function ID:
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return void
 */
void Pwm_Hal_ClearDetectEventFlag
(
    Pwm_Hal_InstanceType Instance
);

/*!
 * @brief: Pwm_Hal_EnableWriteProtection: Enable write protection.
 * @note Function ID: DES_PWM_API_266
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Enable: TRUE/FALSE
 * @return void
 */
void Pwm_Hal_EnableWriteProtection
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/*!
 * @brief: Pwm_Hal_SetDebugMode: Set debug mode.
 * @note Function ID: DES_PWM_API_267
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Mode: pwm debug mode
 *            - PWM_DEBUG_MODE_NO_EFFECT
 *            - PWM_DEBUG_MODE_COUNTER_STOPPED_OUTPUT_PREVIOUS
 *            - PWM_DEBUG_MODE_COUNTER_STOPPED_OUTPUT_HIGH
 * @return void
 */
void Pwm_Hal_SetDebugMode
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_DebugModeType Mode
);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
/* =============================================  EOF  ============================================== */
#endif   /* PWM_HAL_H */
