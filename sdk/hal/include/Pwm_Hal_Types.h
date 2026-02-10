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
* AutoChips Inc. (C) 2023. All rights reserved.
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

#ifndef PWM_HAL_TYPES_H
#define PWM_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*===================================================ENUMS==========================================*/
/*!
 * @brief pwm hw module enumeration
 */
typedef enum
{
    PWM_INSTANCE_0 = 0U,
    PWM_INSTANCE_1,
    PWM_INSTANCE_2,
    PWM_INSTANCE_3,
    PWM_INSTANCE_4,
    PWM_INSTANCE_5,
#if defined (AC7843X)
    PWM_INSTANCE_6,
    PWM_INSTANCE_7,
#endif
} Pwm_Hal_InstanceType;

/*!
 * @brief pwm hw channel enumeration
 */
typedef enum
{
    PWM_CHANNEL_0 = 0U,
    PWM_CHANNEL_1,
    PWM_CHANNEL_2,
    PWM_CHANNEL_3,
    PWM_CHANNEL_4,
    PWM_CHANNEL_5,
    PWM_CHANNEL_6,
    PWM_CHANNEL_7,
} Pwm_Hal_ChannelType;

/*!
 * @brief pwm hw channel pair enumeration
 */
typedef enum
{
    PWM_CHANNEL_PAIR_0 = 0U,
    PWM_CHANNEL_PAIR_1,
    PWM_CHANNEL_PAIR_2,
    PWM_CHANNEL_PAIR_3,
    PWM_CHANNEL_PAIR_NUM
} Pwm_Hal_ChannelPairType;

/*!
 * @brief pwm clock source enumeration
 */
typedef enum
{
    PWM_CLK_SOURCE_NONE = 0U, /*!< No clock selected, in effect disables the counter */
#if defined (AC7843X)
    PWM_CLK_SOURCE_BUS, /*!< Bus clock */
#else
    PWM_CLK_SOURCE_SYSTEM, /*!< System clock */
#endif
    PWM_CLK_SOURCE_FIXED_FREQUENCY, /*!< Fixed frequency clock */
    PWM_CLK_SOURCE_EXTERNAL, /*!< External clock */
} Pwm_Hal_ClockSourceType;

/*!
 * @brief pwm count mode enumeration
 */
typedef enum
{
    UP_COUNT = 0U,
    UP_DOWN_COUNT,
} Pwm_Hal_CountModeType;

/*!
 * @brief pwm output channel running mode enumeration
 */
typedef enum
{
    OUTPUT_NONE = 0U,
    OUTPUT_INDEPENDENT,
    OUTPUT_COMBINE,
    OUTPUT_COMPARE,
} Pwm_Hal_OutputChnModeType;

/*!
 * @brief pwm output channel level enumeration
 */
typedef enum
{
    PWM_LOW_LEVEL = 0U,
    PWM_HIGH_LEVEL,
} Pwm_Hal_OutputLevelType;

/*!
 * @brief pwm output channel level mode enumeration
 */
typedef enum
{
    PWM_LOW_TRUE = 0U,
    PWM_HIGH_TRUE,
} Pwm_Hal_OutputLevelModeType;

/*!
 * @brief Enumeration of pwm duty cycle types in combination center alignment mode.
 */
typedef enum
{
    PWM_DUTY_MODE_0 = 0U, /*!< CnV=Cn+1V=CNTIN,output 0% duty cycle;CnV=Cn+1V=MOD,output 100% duty cycle */
    PWM_DUTY_MODE_1 /*!< CnV=Cn+1V=CNTIN,output 100% duty cycle;CnV=Cn+1V=MOD,output 0% duty cycle */
} Pwm_Hal_CombineCenterDutyModeType;

/*!
 * @brief pwm combine channel match dir enumeration
 */
typedef enum
{
    PWM_DIR_DOWN = 0U,
    PWM_DIR_UP,
} Pwm_Hal_ChannelMatchDirType;

/*!
 * @brief pwm combine channel dead time psc enumeration
 */
typedef enum
{
    PWM_DEADTIME_DIVID_1 = 1U,
    PWM_DEADTIME_DIVID_4,
    PWM_DEADTIME_DIVID_16,
} Pwm_Hal_DeadTimePscType;

/*!
 * @brief PWM output compare mode enumeration.
 */
typedef enum
{
    PWM_NONE_OUTPUT = 0U, /*!< No output */
    PWM_TOGGLE_OUTPUT, /*!< Toggle output */
    PWM_CLEAR_OUTPUT, /*!< Clear output */
    PWM_SET_OUTPUT /*!< Set output */
} Pwm_Hal_CompareActionType;

/*!
* @brief PWM channel output polarity active enumeration.
*/
typedef enum
{
    PWM_ACTIVE_POLARITY_HIGH = 0U, /*!< The channel output polarity is active high */
    PWM_ACTIVE_POLARITY_LOW /*!< The channel output polarity is active low */
} Pwm_Hal_ActivePolarityType;

/*!
 * @brief pwm detect edge enumeration
 */
typedef enum
{
    NONE_EDGE = 0U,
    RISING_EDGE_DETECT,
    FALLING_EDGE_DETECT,
    BOTH_EDGES_DETECT,
} Pwm_Hal_EdgeType;

/*!
 * @brief pwm input channel running mode enumeration
 */
typedef enum
{
    INPUT_NONE = 0U,
    INPUT_SINGLE,
    INPUT_DUAL,
} Pwm_Hal_InputChnModeType;

/*!
 * @brief input event psc
 */
typedef enum
{
    INPUT_EVENT_PSC_1 = 0U,
    INPUT_EVENT_PSC_2,
    INPUT_EVENT_PSC_4,
    INPUT_EVENT_PSC_8,
} Pwm_Hal_InputEventPscType;

/*!
 * @brief input filter psc
 */
typedef enum
{
    INPUT_FILTER_PSC_1 = 0U,
    INPUT_FILTER_PSC_2,
    INPUT_FILTER_PSC_4,
    INPUT_FILTER_PSC_8,
    INPUT_FILTER_PSC_16,
    INPUT_FILTER_PSC_32,
    INPUT_FILTER_PSC_64,
    INPUT_FILTER_PSC_128,
    INPUT_FILTER_PSC_256,
    INPUT_FILTER_PSC_1024,
    INPUT_FILTER_PSC_2048,
    INPUT_FILTER_PSC_4096,
} Pwm_Hal_InputFilterPscType;

/*!
 * @brief pwm measure type enumeration
 */
typedef enum
{
    PWM_POSITIVE_PLUSE_WIDTH_MEASURE = 0U,
    PWM_NEGATIVE_PLUSE_WIDTH_MEASURE,
    PWM_RISING_EDGE_PERIOD_MEASURE,
    PWM_FALLING_EDGE_PERIOD_MEASURE,
    PWM_BOTH_EDGE_DUTY_CYCLE_MEASURE
} Pwm_Hal_DualInputMeasureType;

/*!
 * @brief pwm continuous mode enumeration
 */
typedef enum
{
    PWM_INPUTCAP_ONESHOT = 0U,
    PWM_INPUTCAP_CONTINUOUS,
} Pwm_Hal_DualInputContinuousModeType;

/*!
 * @brief pwm sync trigger enumeration
 */
typedef enum
{
    PWM_SYNC_TRIGGER_SOFTWARE = 0U,
    PWM_SYNC_TRIGGER_HARDWARE,
} Pwm_Hal_SyncTriggerMethodType;

/*!
 * @brief pwm sync mode enumeration
 */
typedef enum
{
    PWM_SYNC_MODE_LEGACY = 0U,
    PWM_SYNC_MODE_ENHANCED,
} Pwm_Hal_SyncModeType;

/*!
 * @brief PWM fault input pin enumeration.
 */
typedef enum
{
    PWM_FAULT_INPUT_0 = 0U, /*!< PWM fault input input id 0 */
    PWM_FAULT_INPUT_1, /*!< PWM fault input input id 1 */
    PWM_FAULT_INPUT_2, /*!< PWM fault input input id 2 */
    PWM_FAULT_INPUT_3, /*!< PWM fault input input id 3 */
    PWM_FAULT_INPUT_MAX /*!< Invalid fault input id */
} Pwm_Hal_FaultInputIdType;

/*!
 * @brief PWM channel fault control mode enumeration.
 */
typedef enum
{
    PWM_FAULT_CTRL_NONE = 0U, /*!< No Fault control */
    PWM_FAULT_CTRL_MANUAL_EVEN, /*!< Fault control is enabled for even channels and manual fault clearing */
    PWM_FAULT_CTRL_MANUAL_ALL, /*!< Fault control is enabled for all channels and manual fault clearing */
    PWM_FAULT_CTRL_AUTO /*!< Fault control is enabled for all channels and automatic fault clearing */
} Pwm_Hal_FaultCtrlModeType;

/*!
 * @brief PWM quadrature decode mode enumeration.
 */
typedef enum
{
    PWM_QUAD_PHASE_ENCODE = 0U, /*!< Phase encoding mode */
    PWM_QUAD_COUNT_DIR_ENCODE /*!< Counter and direction encoding mode */
} Pwm_Hal_QuadModeType;

/*!
 * @brief PWM quadrature phase polarity enumeration.
 */
typedef enum
{
    PWM_QUAD_PHASE_NORMAL = 0U, /*!< Phase input signal is not inverted */
    PWM_QUAD_PHASE_INVERT /*!< Phase input signal is inverted */
} Pwm_Hal_QuadPhasePolarityType;

#if defined (AC7843X)
/*!
 * @brief PWM quadrature Z index reset mode enumeration.
 */
typedef enum
{
    PWM_QUAD_RESET_AT_NEXT_VAILD_EDGE = 0U, /*!< Counter reset at next vaild edge */
    PWM_QUAD_RESET_IMMEDIATELY = 1U /*!< Counter reset immediately */
} Pwm_Hal_QuadPhaseZResetMode;
#endif

/*!
 * @brief pwm debug mode enumeration.
 */
typedef enum
{
    PWM_DEBUG_OFF = 0U, /*!< No effect for counter, channel output, CNTIN/MOD/CnV/Cn+1V buffer and registers */
    PWM_DEBUG_OUTPUT_PREVIOUS, /*!< Counter stopped, channel output remains previous value, write CNTIN/MOD/CnV/Cn+1V buffer and immediately update to registersn */
    PWM_DEBUG_OUTPUT_HIZ /*!< Counter stopped, channel output remains high resistance value, write CNTIN/MOD/CnV/Cn+1V buffer and immediately update to registersn */
} Pwm_Hal_DebugModeType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
typedef void (*Pwm_Hal_Callback)(Pwm_Hal_InstanceType Instance, uint32 Status, const void *UserInfo);

/*PRQA S 3630 ++ # The implementation of this struct/union type should be hidden. */
/*!
 * @brief pwm common base config struct
 */
typedef struct
{
    Pwm_Hal_ClockSourceType ClockSource;
    uint16 Prescaler;
    uint16 MinCount;
    uint16 MaxCount;
    uint8 PeriodDither;
    boolean EnableDmaTransLen;
    uint8 DmaTransLen;
    boolean EnableOverflowEvent;
    boolean EnableUnderflowEvent;
    boolean EnableOverflowInterrupt;
    boolean EnableOverflowDmaReq;
    uint8 OverflowFreq;
    Pwm_Hal_Callback OverflowCallback;
    Pwm_Hal_Callback ChannelCallback;
} Pwm_Hal_CommonCfg;

/*!
 * @brief pwm independent channel config struct
 */
typedef struct
{
    uint16 ChnValue;
    uint8 ChnDither;
    boolean EnableMatchTrigger;
    boolean EnableInterrupt;
    boolean EnableChnEventDmaReq;
    Pwm_Hal_OutputLevelType InitLevel;
    Pwm_Hal_ActivePolarityType ActivePolarity;
    Pwm_Hal_OutputLevelModeType LevelMode;
} Pwm_Hal_IndependentChnCfg;

/*!
 * @brief pwm combine mode config struct
 */
typedef struct
{
    uint16 Ch1stValue;
    uint8 Ch1stDither;
    boolean EnableCh1stMatchTrigger;
    boolean EnableCh1stInterrupt;
    boolean EnableCh1stEventDmaReq;
    Pwm_Hal_OutputLevelType Ch1stInitLevel;
    Pwm_Hal_ActivePolarityType Ch1stActivePolarity;
    Pwm_Hal_ChannelMatchDirType Ch1stMatchDir;
    uint16 Ch2ndValue;
    uint8 Ch2ndDither;
    boolean EnableCh2ndMatchTrigger;
    boolean EnableCh2ndInterrupt;
    boolean EnableCh2ndEventDmaReq;
    Pwm_Hal_OutputLevelType Ch2ndInitLevel;
    Pwm_Hal_ActivePolarityType Ch2ndActivePolarity;
    Pwm_Hal_ChannelMatchDirType Ch2ndMatchDir;
    Pwm_Hal_OutputLevelModeType LevelMode;
    boolean EnableChComplementation;
    boolean EnableSymmetric;
    boolean EnableDeadTime;
    uint16 DeadTimeValue;
    Pwm_Hal_DeadTimePscType TimePsc;
} Pwm_Hal_CombineChnPairCfg;

/*!
 * @brief pwm compare mode config struct
 */
typedef struct
{
    uint16 ChnValue;
    boolean EnableMatchTrigger;
    boolean EnableInterrupt;
    boolean EnableChnEventDmaReq;
    Pwm_Hal_OutputLevelType InitLevel;
    Pwm_Hal_ActivePolarityType ActivePolarity;
    Pwm_Hal_CompareActionType Action;
} Pwm_Hal_CompareModeChnCfg;

/*!
 * @brief pwm output mode common config struct
 */
typedef struct
{
    Pwm_Hal_CommonCfg *BaseCfg;
    Pwm_Hal_CountModeType CountMode;
    boolean EnableInitTrigger;
    boolean EnableMaxTrigger;
    uint8 TriggerRatio;
    boolean InitOutput;
    Pwm_Hal_CombineCenterDutyModeType CombineCenterDutyMode;
} Pwm_Hal_OutputCommonCfg;

/*!
 * @brief pwm output channel config struct
 */
typedef struct
{
    Pwm_Hal_ChannelType Channel;
    Pwm_Hal_OutputChnModeType ChannelMode;
    Pwm_Hal_IndependentChnCfg *IndependentChnCfg;
    Pwm_Hal_CombineChnPairCfg *ChnPairCfg;
    Pwm_Hal_CompareModeChnCfg *CompareChnCfg;
} Pwm_Hal_OutputChannelCfg;

/*!
 * @brief pwm output mode config struct
 */
typedef struct
{
    Pwm_Hal_OutputCommonCfg CommonCfg;
    Pwm_Hal_OutputChannelCfg *ChannelCfg;
    uint8 ChannelNum;
} Pwm_Hal_OutputCfg;

/*!
 * @brief pwm input mode common config struct
 */
typedef struct
{
    Pwm_Hal_CommonCfg *BaseCfg;
    boolean EnableHall;
    Pwm_Hal_InputFilterPscType FilterPsc;
} Pwm_Hal_InputCommonCfg;

/*!
 * @brief pwm input channel config struct
 */
typedef struct
{
    Pwm_Hal_ChannelType Channel;
    Pwm_Hal_InputChnModeType ChannelMode;
    Pwm_Hal_EdgeType CaptureEdge;
    Pwm_Hal_InputEventPscType EventPsc;
    Pwm_Hal_DualInputContinuousModeType DualInputContinuousMode;
    boolean EnablePulseWidthMeasure;
    Pwm_Hal_DualInputMeasureType DualInputMeasureType;
    boolean EnableCounterReset;
    boolean EnableInterrupt;
    boolean EnableChnEventDmaReq;
    uint8 FilterValue;
} Pwm_Hal_InputChannelCfg;

/*!
 * @brief pwm input mode config struct
 */
typedef struct
{
    Pwm_Hal_InputCommonCfg CommonCfg;
    Pwm_Hal_InputChannelCfg *ChannelCfg;
    uint8 ChannelNum;
} Pwm_Hal_InputCfg;

/*!
 * @brief PWM sync configuration structure.
 * Please don't use software and hardware trigger simultaneously
 */
typedef struct
{
    Pwm_Hal_SyncTriggerMethodType TriggerType; /*!< Synchronization trigger mode */
    boolean EnableHwSync0; /*!< Enable/disable hardware sync trigger source 0 */
    boolean EnableHwSync1; /*!< Enable/disable hardware sync trigger source 1 */
    boolean EnableHwSync2; /*!< Enable/disable hardware sync trigger source 2 */
    boolean DisableHwTriggerAfterTrigged; /*!< Available only for hardware trigger */
    boolean EnableCounterInitSync; /*!< Enable/disable CNTIN sync */
    boolean EnableSwOutputCtrlSync; /*!< Enable/disable CHOSWCR sync */
    boolean EnableDualChannelInvertSync; /*!< Enable/disable INVCR sync */
    boolean EnableOutputMaskSync; /*!< Enable/disable OMCR sync */
    boolean EnablePolaritySync; /*!< Enable/disable CHOPOLCR sync */
    boolean EnablePairedChnValSync[PWM_CHANNEL_PAIR_NUM]; /*!< Enable/disable dual channel CHV sync */
    boolean EnableMaxLoadingPoint; /*!< Enable/disable maximum loading point */
    boolean EnableMinLoadingPoint; /*!< Enable/disable minimum loading point */
    boolean EnableSyncBypass; /*!< Enable/disable Synchronization bypass */
    boolean EnableSync; /*!< PWM Enable/disable */
} Pwm_Hal_SyncCfg;

/*!
 * @brief PWM Fault channel configuration structure.
 */
typedef struct
{
    boolean EnableFaultInput; /*!< Fault input channel state */
    boolean EnableFaultFilter; /*!< Fault channel filter state */
    Pwm_Hal_ActivePolarityType FaultPolarity; /*!< Fault channel input polarity active */
} Pwm_Hal_FaultPinCfg;

/*!
 * @brief PWM Fault configuration structure.
 */
typedef struct
{
    Pwm_Hal_FaultCtrlModeType FaultCtrlMode; /*!< Fault mode */
    Pwm_Hal_FaultPinCfg FaultPinCfg[PWM_FAULT_INPUT_MAX]; /*!< Fault input channels configuration */
    uint8 FilterValue; /*!< Fault filter value */
    Pwm_Hal_InputFilterPscType FilterPsc; /*!< Fault filter psc */
    boolean EnableChannelOutputCtrl[PWM_CHANNEL_PAIR_NUM]; /*!< Fault control channel output state */
    boolean EnableInterrupt; /*!< Enable PWM fault interrupt */
    boolean EnableHiz; /*!< Enable PWM fault Hiz Output */
    Pwm_Hal_Callback FaultCallback;
} Pwm_Hal_FaultCfg;

/*!
 * @brief PWM quadrature decoder phase input configuration structure.
 */
typedef struct
{
    Pwm_Hal_QuadPhasePolarityType Polarity; /*!< PhaseA Polarity */
    uint8 FilterValue; /*!< Filter value */
} Pwm_Hal_QuadPhaseCfg;

/*!
 * @brief PWM quadrature configuration structure.
 */
typedef struct
{
    Pwm_Hal_CommonCfg *BaseCfg; /*!< PWM common config */
    Pwm_Hal_QuadModeType Mode; /*!< PWM quadrature decode mode */
    Pwm_Hal_QuadPhaseCfg PhaseAConfig; /*!< PhaseA config */
    Pwm_Hal_QuadPhaseCfg PhaseBConfig; /*!< PhaseB config */
    Pwm_Hal_QuadPhaseCfg PhaseZConfig; /*!< PhaseZ config */
    boolean EnableQuad; /*!< Enable quadrature decode mode */
    Pwm_Hal_InputFilterPscType FilterPsc; /*!< Input Capture Filter psc */
    boolean EnablePhaseZReset; /*!< Reset the counter when Z index event is detected> */
#if defined (AC7843X)
    Pwm_Hal_QuadPhaseZResetMode PhaseZResetMode; /*!< Reset mode when Z index event is detected > */
    boolean EnablePhaseZInterrupt; /*!< Enable/disabled interrupt when z index event is detected > */
    Pwm_Hal_Callback ZDetectCallback;
#endif
} Pwm_Hal_QuadDecoderCfg;
/*PRQA S 3630 -- */

#endif
