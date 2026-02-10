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
/**
 * @file Adc_Hal.c
 * @brief This file provides all adc hal api.
 */
/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Adc_Reg.h"
#include "AC784xx_Ctu_Reg.h"
#include "OsIf_Time.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Dma_Hal.h"
#include "Adc_Hal.h"
#include "Core_Hal.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
#define ROUND(x) ((x) >= 0 ? ((sint16)((x) + 0.5)) : ((sint16)((x) - 0.5)))
/* ============================================= TYPEDEFS ================================================ */
#ifndef ADC_SDK_NON_EXTENDED_API
/*!
 * @brief Analog ADC GE OE calibration internal input source.
 */
typedef enum
{
    ANA_GEOECAL_FLOATING = 0U,
    ANA_GEOECAL_REF_16_4,
    ANA_GEOECAL_REF_16_5,
    ANA_GEOECAL_REF_16_6,
    ANA_GEOECAL_REF_16_8,
    ANA_GEOECAL_REF_16_10,
    ANA_GEOECAL_REF_16_11,
    ANA_GEOECAL_REF_16_12,
} Ana_GeoecalVinType;
/*!
 * @brief ADC calibration structure
 */
typedef struct
{
    sint16 userGain;
    sint16 userOffset;
} adc_calibration_t;
#endif
/* =========================================== LOCAL VARIABLES ============================================== */
/*!< Table of ADC Base address */
static ADC_Type *const AdcBase[ADC_INSTANCE_MAX] = {ADC0, ADC1};
/*!< Table of ADC interrupt handler information */
static Adc_InterruptType AdcISR[ADC_INSTANCE_MAX];
/*!< Table of ADC IRQ IDS */
static const IRQn_Type AdcIrqs[ADC_INSTANCE_MAX] = ADC_IRQS;
/* Table of ADC ckgen interface clocks */
static const Ckgen_BusClkIdType AdcCkgenBusClock[ADC_INSTANCE_MAX] = {CKGEN_ADC0_BUS_CLK, CKGEN_ADC1_BUS_CLK};
/* Table of ADC soft resets */
static const Rcm_ResetIDType AdcClockReset[ADC_INSTANCE_MAX] = {RCM_RESET_ID_ADC0, RCM_RESET_ID_ADC1};

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

#ifndef ADC_SDK_NON_EXTENDED_API
/*!
 * @brief ADC self calibration function, canbe used after calibration setting is configured.
 *
 * @param[in] Instance: ADC instance number
 * @param[out] Config: calibration config result
 * @param[in] SeqIndex: ADC seqeunce
 * @return none
 */
static void ADC_SelfCalibration(uint8 Instance, adc_calibration_t *const Config, Adc_SequenceType SeqIndex);

/*!
 * @brief Config ADC0 & ADC1 interleave function.
 *
 * @param[in] Instance: ADC instance number
 * @param[in] Interleave: ADC0/1 interleave mode
 * @return none
 */

static void Adc_SetInterleave(uint8 Instance, Adc_InterleaveType Interleave);

/*!
 * @brief Get average value of ADC convert result from assigned sequence.
 *
 * @param[in] Instance: ADC instance number
 * @param[in] SeqIndex: ADC seqeunce
 * @param[in] Count: average count
 * @return averaged ADC result
 */
static float32 Adc_GetAverageValue(uint8 Instance, Adc_SequenceType SeqIndex, uint32 Count);

/**
* @brief ADC cfg dma args function.
* @param [in] Instance: Specify adc HW Unit
* @param [in] DstAddr: dma write data to the address
* @param [in] SrcAddr: dma read data from the address
* @param [in] Length: read data len
* @param [in] Callback: dma callback
* @param [in] DmaArgs: dma callback args
* @return void
*/
static void Adc_CfgDma(uint8 Instance, uint32 DstAddr, uint32 SrcAddr,
                       uint32 Length, Hal_CallbackType Callback, void *DmaArgs);
#endif

/**
* @brief ADC irq handle event process function.
* @param [in] Instance: Specify adc HW Unit
* @return void
*/
static void Adc_CommonISR(uint8 Instance);

/**
* @brief ADC deinit dma channel.
* @param [in] Instance: Specify adc HW Unit
* @return void
*/
static void Adc_DmaDeinit(uint8 Instance);

#if defined (AC7843X)
/**
* @brief ADC set internal channel ana.
* @param [in] Instance: Specify adc HW Unit
* @param [in] Channel: input channel
* @return void
*/
static void Adc_SetInternalChannelAna(uint32 Instance, Adc_InputChannelType Channel);
#elif defined (AC7842X)
/**
* @brief ADC set internal channel ana.
* @param [in] InternalChannelSrc: internal channel source select
* @return void
*/
static void Adc_SetInternalChannelAna(Adc_InternalChannelSrcType InternalChannelSrc);
#endif

/*=================================GLOBAL FUNCTION IMPLEMENTATIONS=================================*/
void Adc_Hal_Init(uint8 Instance, const Adc_InitConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    AdcISR[Instance].Callback = Config->Interrupt.Callback;
    (void)Ckgen_Hal_EnablePeriphClk(AdcCkgenBusClock[Instance], TRUE);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    /*Enable nvic*/
    Core_Hal_EnableIrq(AdcIrqs[Instance]);
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/
}

void Adc_Hal_Deinit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);

    Adc_DmaDeinit(Instance);
    /* Disable nvic */
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    Core_Hal_DisableIrq(AdcIrqs[Instance]);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(AdcCkgenBusClock[Instance], FALSE);
    Core_Hal_ClearPendingIrq(AdcIrqs[Instance]);
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/
}

void Adc_Hal_InitConverterStruct(Adc_ConverterConfigType *const Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Config->ClockDivide = ADC_CLK_DIVIDE_16;
    Config->Resolution = ADC_RESOLUTION_12BIT;
    Config->Alignment = ADC_ALIGN_RIGHT;
    Config->VoltageRef = ADC_VOLTAGEREF_VREF;
    Config->DmaEnable = FALSE;
    Config->PowerEn = TRUE;
    Config->HwAverage.Enable = FALSE;
    Config->Interleave = ADC_INTERLEAVE_DISABLE;
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

void Adc_Hal_ConfigConverter(uint8 Instance, const Adc_ConverterConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    const Ckgen_ClkIdType AdcCkgenClock[ADC_INSTANCE_MAX] = {CKGEN_ADC0_CLK, CKGEN_ADC1_CLK};
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    uint32 Freq = 0;
    Hal_StatusType ReturnValue;

    ReturnValue = Ckgen_Hal_GetFreq(AdcCkgenClock[Instance], &Freq);//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(ReturnValue == STATUS_SUCCESS);
    DEVICE_ASSERT((Freq / ((uint32)Config->ClockDivide + 1U)) <= 30000000U);//PRQA S 2842,2812 # the upper layer call guarantees that there will never be an array out of bounds.*/
    (void)ReturnValue;
    Adc_Reg_SetClockPrescaler(Base, Config->ClockDivide); /* Set clock divide */
    Adc_Reg_SetResolution(Base, Config->Resolution); /* Set Resolution */
    Adc_Reg_SetDataAlign(Base, Config->Alignment); /* Set Alignment */
    Adc_Reg_SetVoltageReference(Base, Config->VoltageRef); /* Set voltage reference */
    Adc_Reg_SetDMAEnableFlag(Base, Config->DmaEnable);
    Adc_Reg_SetAverageEnableFlag(Base, Config->HwAverage.Enable);
    Adc_Reg_SetAverageMode(Base, (uint8)Config->HwAverage.Value);
#ifndef ADC_SDK_NON_EXTENDED_API
    Adc_SetInterleave(Instance, Config->Interleave);
#endif
    Adc_Reg_SetPowerEnableFlag(Base, Config->PowerEn); /* Set power */
    OsIf_UDelay(100);
}

void Adc_Hal_InitChanStruct(Adc_ChanConfigType *const Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);
    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Config->Spt = ADC_SPT_CLK_5;
    Config->Channel = ADC_CH_DISABLE;
    Config->InterruptEn = FALSE;
    Config->SeqIndex = ADC_ISEQ_MAX;
    Config->InternalChannelSrc = ADC_INTL_CH_SRC_VDDA;
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

void Adc_Hal_ConfigChannel(uint8 Instance, const Adc_ChanConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    DEVICE_ASSERT(Config->SeqIndex < ADC_ISEQ_MAX);//PRQA S 2812 # the upper layer call guarantees that a null pointer will never appear.*/
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    Adc_InputChannelType Channel = Config->Channel;

    Adc_Reg_SetChannelSampleTime(Base, Channel, Config->Spt);
    if ((uint32)Config->SeqIndex < (uint32)ADC_ISEQ_0)
    {
        Adc_Reg_SetRegularConversionChannel(Base, Config->SeqIndex, Channel);
        Adc_Reg_SetRegularEOCInterruptEnableFlag(Base, Config->SeqIndex, Config->InterruptEn);
    }
    else
    {
        Adc_Reg_SetInjectConversionChannel(Base, Config->SeqIndex, Channel);
        Adc_Reg_SetInjectEOCInterruptEnableFlag(Base, Config->SeqIndex, Config->InterruptEn);
    }
#if defined (AC7840X) || defined (AC7843X)
    if (ADC_CH_SUPPLY == Channel)
    {
        Adc_Reg_SetSupplyChannel(Base, Config->InternalChannelSrc);
#if defined (AC7843X)
        if( ADC_INTL_CH_SRC_OSC_LDO == Config->InternalChannelSrc)
        {
            Adc_Reg_SetAMUXXoscOutputEnable(TRUE);
        }
#endif
    }
#if defined (AC7843X)
    else if ((ADC_CH_BANDGAP == Channel) || (ADC_CH_TSENSOR == Channel))
    {
        Adc_SetInternalChannelAna(Instance, Channel);
    }
    else
    {
    /* do nothing*/
    }
#endif
#elif defined (AC7842X)
    if (ADC_CH_INTERNAL == Channel)
    {
         Adc_SetInternalChannelAna(Config->InternalChannelSrc);
    }
#endif
}

void Adc_Hal_ConfigRegularGroup(uint32 Instance, const Adc_GroupConfigType* Config)
{
    ADC_Type * const Base = AdcBase[Instance];

    Adc_Reg_SetScanEnableFlag(Base, Config->ScanModeEn);
    Adc_Reg_SetContinuousEnableFlag(Base, Config->ContinuousModeEn);
    Adc_Reg_SetRegularTriggerSource(Base, Config->RegularTrigger);
    Adc_Reg_SetRegularLength(Base, Config->RegularSequenceLength);
}

void Adc_Hal_ConfigInjectGroup(uint32 Instance, const Adc_GroupConfigType* Config)
{
    ADC_Type * const Base = AdcBase[Instance];

    Adc_Reg_SetScanEnableFlag(Base, Config->ScanModeEn);
    Adc_Reg_SetIntervalEnableFlag(Base, Config->IntervalModeEn);
    Adc_Reg_SetInjectTriggerSource(Base, Config->InjectTrigger);
    Adc_Reg_SetInjectLength(Base, Config->InjectSequenceLength);
}

void Adc_Hal_DmaEnable(uint32 Instance, boolean Enable)
{
    ADC_Type * const Base = AdcBase[Instance];
    /* Set DMA enable flag */
    Adc_Reg_SetDMAEnableFlag(Base, Enable);
}

void Adc_Hal_SwTriggerRegularConvert(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/

    Adc_Reg_SoftwareStartRegularConvert(Base);
}

#ifndef ADC_SDK_NON_EXTENDED_API
void Adc_Hal_SwTriggerInjectConvert(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/

    Adc_Reg_SoftwareStartInjectConvert(Base);
}
#endif

uint16 Adc_Hal_GetSeqResult(uint8 Instance, Adc_SequenceType SeqIndex)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);
    uint16 result = 0;
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/

    if ((uint32)SeqIndex < (uint32)ADC_ISEQ_0)
    {
        result = Adc_Reg_GetRegularData(Base, SeqIndex);
    }
    else
    {
        result = Adc_Reg_GetInjectData(Base, SeqIndex);
    }

    return result;
}

#ifndef ADC_SDK_NON_EXTENDED_API
void Adc_Hal_InitGroupStruct(Adc_GroupConfigType *const Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Config->RegularSequenceLength = 0U;
    Config->RegularTrigger = ADC_TRIGG_SRC_SW;
    Config->InjectSequenceLength = 0U;
    Config->InjectTrigger = ADC_TRIGG_SRC_SW;
    /* Conversion mode */
    Config->ScanModeEn = FALSE;
    Config->ContinuousModeEn = FALSE;
    Config->RegularDiscontinuousModeEn = FALSE;
    Config->InjectDiscontinuousModeEn = FALSE;
    Config->InjectAutoModeEn = FALSE;
    Config->IntervalModeEn = FALSE;
    Config->RegularDiscontinuousNum = 0U;
    Config->DmaCallback = NULL_PTR;
    Config->DmaArgs = NULL_PTR;
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

void Adc_Hal_ConfigGroup(uint8 Instance, const Adc_GroupConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    /* Set adc mode */
    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Adc_Reg_SetScanEnableFlag(Base, Config->ScanModeEn);
    Adc_Reg_SetContinuousEnableFlag(Base, Config->ContinuousModeEn);
    Adc_Reg_SetRegularDiscontinuousEnableFlag(Base, Config->RegularDiscontinuousModeEn);
    Adc_Reg_SetInjectDiscontinuousEnableFlag(Base, Config->InjectDiscontinuousModeEn);
    Adc_Reg_SetInjectAutoEnableFlag(Base, Config->InjectAutoModeEn);
    Adc_Reg_SetIntervalEnableFlag(Base, Config->IntervalModeEn);
    Adc_Reg_SetRegularDiscontinuousNum(Base, Config->RegularDiscontinuousNum);

    /* Set adc regular*/
    Adc_Reg_SetRegularTriggerSource(Base, Config->RegularTrigger);
    Adc_Reg_SetRegularLength(Base, Config->RegularSequenceLength);

    /* Set adc inject*/
    Adc_Reg_SetInjectTriggerSource(Base, Config->InjectTrigger);
    Adc_Reg_SetInjectLength(Base, Config->InjectSequenceLength);
    if (TRUE == Adc_Reg_GetDMAEnableFlag(Base))
    {
        DEVICE_ASSERT(0U != Config->DmaDstAddr);
        Adc_CfgDma(Instance, Config->DmaDstAddr, (uint32)&Base->RDR[0], \
                   (uint32)Config->RegularSequenceLength * 2U, Config->DmaCallback, Config->DmaArgs);
    }
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

void Adc_Hal_InitAMOStruct(Adc_AmoConfigType *const Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Config->AmoTriggerMode = ADC_AMO_TRIGGER_LEVEL;

    Config->AmoInterruptEn = FALSE;

    Config->AmoRegularEn = FALSE;
    Config->AmoInjectEn = FALSE;
    Config->AmoSingleModeEn = FALSE;
    Config->AmoSingleChannel = ADC_CH_0;

    Config->AmoUpThreshold = 1U;
    Config->AmoLowThreshold = 0U;
    Config->AmoUpOffset = 0U;
    Config->AmoLowOffset = 0U;
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

void Adc_Hal_ConfigAmo(uint8 Instance, const Adc_AmoConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    DEVICE_ASSERT(Config->AmoUpThreshold > Config->AmoLowThreshold);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/

    Adc_Reg_SetAMOTriggerMode(Base, Config->AmoTriggerMode);
    Adc_Reg_SetAMORegularMode(Base, Config->AmoRegularEn);
    Adc_Reg_SetAMOInjectMode(Base, Config->AmoInjectEn);
    Adc_Reg_SetAMOSingleChannelMode(Base, Config->AmoSingleModeEn);
    Adc_Reg_SetAMOSingleChannel(Base, Config->AmoSingleChannel);

    Adc_Reg_SetAMOThreshold(Base, Config->AmoUpThreshold, Config->AmoLowThreshold);
    Adc_Reg_SetAMOOffset(Base, Config->AmoUpOffset, Config->AmoLowOffset);
    Adc_Reg_SetAMOInterrupt(Base, Config->AmoInterruptEn);
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

boolean Adc_Hal_GetIdleFlag(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    return (0U != (Adc_Reg_GetSTRFlag(Base) & ADC_STR_IDLE_Msk)) ? TRUE : FALSE;
}

boolean Adc_Hal_GetConvertCompleteFlag(uint8 Instance, Adc_SequenceType SeqIndex)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    boolean Status = FALSE;

    if ((uint32)SeqIndex < (uint32)ADC_ISEQ_0)
    {
        Status = Adc_Reg_GetRegularEOCFlag(Base, SeqIndex);
    }
    else
    {
        Status = Adc_Reg_GetInjectEOCFlag(Base, SeqIndex);
    }

    return Status;
}

void Adc_Hal_ClearConvertCompleteFlag(uint8 Instance, Adc_SequenceType SeqIndex)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/

    if ((uint32)SeqIndex < (uint32)ADC_ISEQ_0)
    {
        Adc_Reg_ClearRegularEOCFlag(Base, SeqIndex);
    }
    else
    {
        Adc_Reg_ClearInjectEOCFlag(Base, SeqIndex);
    }
}

boolean Adc_Hal_GetTriggerConflictFlag(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/

    return (0U != (Adc_Reg_GetSTRFlag(Base) & ADC_STR_COVCFT_Msk)) ? TRUE : FALSE;
}

void Adc_Hal_ClearTriggerConflictFlag(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/

    Adc_Reg_ClearSTRFlag(Base, ADC_STR_COVCFT_Msk);
}

uint8 Adc_Hal_GetParityVal(uint8 Instance, Adc_SequenceType SeqIndex)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    uint8 ret = 0U;

    if ((uint32)SeqIndex < (uint32)ADC_ISEQ_0)
    {
        ret = Adc_Reg_GetRegularParityVal(Base, SeqIndex);
    }
    else
    {
        ret = Adc_Reg_GetInjectParityVal(Base, SeqIndex);
    }
    return ret;
}

float32 Adc_Hal_ConvertToTemperature(const uint16 Value, const float32 RefVoltage)
{
    float32 Temperature = 0.0f;
    float32 Slop = 1.862f, VolTempBase = 673.0f, TempBase = 25.0f;

    Temperature = ((VolTempBase - ((float32)Value / 4096.0f * RefVoltage)) / Slop) + TempBase;

    return Temperature;
}

void Adc_Hal_SetInjectOffset(uint8 Instance, Adc_SequenceType SeqIndex, uint16 Offset)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT((SeqIndex > ADC_RSEQ_31) && (SeqIndex < ADC_ISEQ_MAX));
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/

    Adc_Reg_SetInjectOffset(Base, (uint8)SeqIndex - (uint8)ADC_ISEQ_0, Offset);
}

void Adc_Hal_ConfigAutoCalibration(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    Adc_GroupConfigType CaliGroupConfig;
    Adc_ConverterConfigType caliConvConfig;
    Adc_ChanConfigType caliChConfig;
    Adc_SequenceType seq = ADC_RSEQ_0;
    /* x1 & x2 are calibration voltage value, and y1 & y2 are the corresponding adc code */
    adc_calibration_t caliConfig;
    ADC_Type *Base = NULL_PTR;

    if (Instance < ADC_INSTANCE_MAX)
    {
        Base = AdcBase[Instance];
    }
    caliConfig.userGain = 0;
    caliConfig.userOffset = 0;
    /* Config ADC analog register to generate ge oe calibration signal */
    Adc_Reg_SetGEOEVINEnableFlag(Instance, TRUE);

    /* config ADC convertion mode to mode 1 */
    Adc_Hal_InitConverterStruct(&caliConvConfig);

    caliConvConfig.ClockDivide = ADC_CLK_DIVIDE_16;
    caliConvConfig.Alignment = ADC_ALIGN_RIGHT;
    caliConvConfig.DmaEnable = FALSE;
    caliConvConfig.PowerEn = TRUE;
    caliConvConfig.VoltageRef = ADC_VOLTAGEREF_VREF;
    Adc_Hal_ConfigConverter(Instance, &caliConvConfig);

    Adc_Hal_InitGroupStruct(&CaliGroupConfig);
    CaliGroupConfig.ScanModeEn = FALSE;
    CaliGroupConfig.ContinuousModeEn = FALSE;
    CaliGroupConfig.RegularDiscontinuousModeEn = FALSE;
    CaliGroupConfig.InjectDiscontinuousModeEn = FALSE;
    CaliGroupConfig.IntervalModeEn = FALSE;
    CaliGroupConfig.RegularTrigger = ADC_TRIGG_SRC_SW;
    Adc_Hal_ConfigGroup(Instance, &CaliGroupConfig);

    /* Disable calibration caculation logic */
    Adc_Reg_SetCalibrationEnableFlag(Base, FALSE);

    /* Config convertion channel */
    Adc_Hal_InitChanStruct(&caliChConfig);

    caliChConfig.Channel = ADC_CH_GEOE_CAL;
    caliChConfig.InterruptEn = FALSE;
    caliChConfig.Spt = ADC_SPT_CLK_185;
    caliChConfig.SeqIndex = seq;
    Adc_Hal_ConfigChannel(Instance, &caliChConfig);
    OsIf_UDelay(100);
    /* calibrate external channel */
    ADC_SelfCalibration(Instance, &caliConfig, seq);

    Adc_Reg_SetGainOffset0Value(Base, caliConfig.userGain, caliConfig.userOffset);

    /* using external calibration parameter for internal channel */
    Adc_Reg_SetGainOffset1Value(Base, caliConfig.userGain, caliConfig.userOffset);

    /* Restore ADC ANA setting */
    Adc_Reg_SetGEOEVINEnableFlag(Instance, FALSE);
    /* Enable calibration caculation after GE OE is ready */
    Adc_Reg_SetCalibrationEnableFlag(Base, TRUE);
}

ADC_Type *Adc_Hal_GetBase(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    return AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
}
static void ADC_SelfCalibration(uint8 Instance, adc_calibration_t *const Config, Adc_SequenceType SeqIndex)
{
    DEVICE_ASSERT(NULL_PTR != Config);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);

    float32 y1 = 0.0f, y2 = 0.0f, x1 = 0.0f, x2 = 0.0f, ge = 0.0f, oe = 0.0f;
    const uint32 avgCount = 16U;

    /* Get calibration value */
    Adc_Reg_SetGEOEVIN(Instance, (uint8)ANA_GEOECAL_REF_16_4);
    x1 = 4.0f / 16.0f; /* x1 from GEOEVIN value */

    y1 = Adc_GetAverageValue(Instance, SeqIndex, avgCount);

    Adc_Reg_SetGEOEVIN(Instance, (uint8)ANA_GEOECAL_REF_16_12);
    x2 = 12.0f / 16.0f; /* x2 from GEOEVIN value */

    y2 = Adc_GetAverageValue(Instance, SeqIndex, avgCount);

    /* Caculate calibration GE OE value */
    ge = (((4095.0f * (x2 - x1)) / ((y2 - y1) - 1.0f))) * 4096.0f;
    oe = (((y2 * x1) - (y1 * x2)) * 4096.0f) / (y2 - y1);

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    /*PRQA S 4395,3396,1802 ++ # allow float types to be converted to unsigned.*/
    Config->userGain = (sint16)ROUND((float64)ge);
    Config->userOffset = (sint16)ROUND((float64)oe);
    /*PRQA S 4395,3396,1802 -- # allow float types to be converted to unsigned.*/
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

static void Adc_SetInterleave(uint8 Instance, Adc_InterleaveType Interleave)
{
#if defined (AC7840X)
    if (0U == Instance)
    {
        if (ADC_INTERLEAVE_DISABLE == Interleave)
        {
            /* turn off RG_MODE */
            Adc_Reg_SetAdc1Interleave(FALSE);
        }
        else
        {
            Ctu_Reg_SetAdc0Inter((uint8)Interleave);
            /* enable ADC1 RG_MODE for ADC1 interleave */
            Adc_Reg_SetAdc1Interleave(TRUE);
            if (ADC_INTERLEAVE_0 == Interleave)
            {
                Adc_Reg_SetAdc1InterleaveChannel(8);
            }
            else
            {
                Adc_Reg_SetAdc1InterleaveChannel(9);
            }
        }
    }
    else
    {
        if (ADC_INTERLEAVE_DISABLE == Interleave)
        {
            /* turn off RG_MODE */
            Adc_Reg_SetAdc0Interleave(FALSE);
        }
        else
        {
            Ctu_Reg_SetAdc1Inter((uint8)Interleave);
            /* enable ADC1 RG_MODE for ADC1 interleave */
            Adc_Reg_SetAdc0Interleave(TRUE);
            if (ADC_INTERLEAVE_0 == Interleave)
            {
                Adc_Reg_SetAdc0InterleaveChannel(4);
            }
            else
            {
                Adc_Reg_SetAdc0InterleaveChannel(5);
            }
        }
    }
#elif defined (AC7842X) || defined (AC7843X)
    if (ADC_INTERLEAVE_DISABLE != Interleave)
    {
        if (0U == Instance)
        {
           Ctu_Reg_SetAdc0Inter((uint32)Interleave);
        }
        else
        {
           Ctu_Reg_SetAdc1Inter((uint32)Interleave);
        }
    }
#endif
}

static float32 Adc_GetAverageValue(uint8 Instance, Adc_SequenceType SeqIndex, uint32 Count)
{
    DEVICE_ASSERT(Count > 0U);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);

    float32 Sum = 0.0f;
    uint32 i = 0U;
    uint16 Value = 0U;

    for (i = 0; i < Count; i++)
    {
        Adc_Hal_SwTriggerRegularConvert(Instance);

        /* Wait for convertion complete */
        while (TRUE != Adc_Hal_GetConvertCompleteFlag(Instance, SeqIndex))
        {
            ;
        }

        Value = Adc_Hal_GetSeqResult(Instance, SeqIndex);
        Sum += (float32)Value;
    }

    return (Sum / (float32)Count);
}

static void Adc_CfgDma(uint8 Instance, uint32 DstAddr, uint32 SrcAddr,
                       uint32 Length, Hal_CallbackType Callback, void *DmaArgs)
{
    Hal_StatusType Status;
    Dma_TransferConfigType TransferConfig;
    uint8 ChannelId = 0;

    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = TRUE;
    TransferConfig.SrcUnit = DMA_TRANSFER_UNIT_2B;
    TransferConfig.DestUnit = DMA_TRANSFER_UNIT_2B;
    TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
    TransferConfig.SrcOffset = 4U;
    TransferConfig.DestOffset = 2U;
    TransferConfig.SrcStartAddr = SrcAddr;
    TransferConfig.DestStartAddr = DstAddr;
    TransferConfig.Length = (uint16)Length;
    /*PRQA S 1891++ # unsigned type(unsigned char) is allowed.*/
    TransferConfig.SrcEndAddr = SrcAddr + \
                                ((Length / (1U << (uint8)TransferConfig.SrcUnit)) * TransferConfig.SrcOffset);
    TransferConfig.DestEndAddr = DstAddr + \
                                ((Length / (1U << (uint8)TransferConfig.DestUnit)) * TransferConfig.DestOffset);
    /*PRQA S 1891-- # unsigned type(unsigned char) is allowed.*/
    TransferConfig.Callback = Callback;
    TransferConfig.UserArgs = DmaArgs;
    if (NULL_PTR != Callback)
    {
        TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    }
    else
    {
        TransferConfig.IrqSrc = (uint8)DMA_IRQ_NONE;
    }

    /*PRQA S 4394,4342 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    ChannelId = Dma_Hal_GetChIdByReqSrc((Dma_RequestSourceType)((uint32)DMA_REQ_ADC0 + Instance));
    /*PRQA S 4394,4342 -- # convert unsigned to enum to ensure that there will be no problems in the current code.*/

    /* Configure the DMA transfer control */
    Status = Dma_Hal_ConfigCh(ChannelId, &TransferConfig);
    if (STATUS_SUCCESS == Status)/* config dma success */
    {
        /* Start the DMA channel */
        (void)Dma_Hal_StartCh(ChannelId);
    }
    DEVICE_ASSERT(STATUS_SUCCESS == Status);
}
#endif

#if defined (AC7843X)
static void Adc_SetInternalChannelAna(uint32 Instance, Adc_InputChannelType Channel)
{
    Adc_Reg_SetAMUXBufEnable(TRUE);
    Adc_Reg_SetAMUXOutputEnable(TRUE);
    if (ADC_CH_BANDGAP == Channel)
    {
        if(1U == Instance)
        {
            Adc_Reg_SetBG1OutputValue((uint8)ANA_OUTPUT_SELECT_BG1);
            Adc_Reg_SetAmuxInternalChannel(ANA_CHANNEL_AMUX_BG1);
        }
        else
        {
            Adc_Reg_SetBG0OutputValue((uint8)ANA_OUTPUT_SELECT_BG0);
            Adc_Reg_SetAmuxInternalChannel(ANA_CHANNEL_AMUX_BG0);
        }
    }
    else if (ADC_CH_TSENSOR == Channel)
    {
        if(1U == Instance)
        {
            Adc_Reg_SetTsensor1OutputValue((uint8)ANA_OUTPUT_SELECT_TSENSOR_ON);
            Adc_Reg_SetAmuxInternalChannel(ANA_CHANNEL_AMUX_TSENSOR1);
        }
        else
        {
            Adc_Reg_SetTsensor0OutputValue((uint8)ANA_OUTPUT_SELECT_TSENSOR_ON);
        }
        Adc_Reg_SetAmuxInternalChannel(ANA_CHANNEL_AMUX_TSENSOR0);
    }
    else
    {
        /*do nothing*/
    }
}
#elif defined (AC7842X)
static void Adc_SetInternalChannelAna(Adc_InternalChannelSrcType InternalChannelSrc)
{
    if ((ADC_INTL_CH_SRC_VDDA == InternalChannelSrc) || (ADC_INTL_CH_SRC_VDD == InternalChannelSrc))
    {
        Adc_Reg_SetAMUXBufEnable(FALSE);
        Adc_Reg_SetAMUXBufBypassEnable(TRUE);
    }
    else
    {
        Adc_Reg_SetAMUXBufEnable(TRUE);
        Adc_Reg_SetAMUXBufBypassEnable(FALSE);
        if (ADC_INTL_CH_SRC_DIGLDO_SUPPLY == InternalChannelSrc)
        {
            Adc_Reg_SetDigitalLDOOutputEnable(TRUE);
            Adc_Reg_SetDigitalLDOEnable(TRUE);
        }
        else if (ADC_INTL_CH_SRC_FLHLDO_SUPPLY == InternalChannelSrc)
        {
            Adc_Reg_SetFlashLDOOutputEnable(TRUE);
        }
        else if (ADC_INTL_CH_SRC_BG0 == InternalChannelSrc)
        {
            Adc_Reg_SetBG0OutputValue((uint8)ANA_OUTPUT_SELECT_BG0);
        }
        else if (ADC_INTL_CH_SRC_BG1 == InternalChannelSrc)
        {
            Adc_Reg_SetBG1OutputValue((uint8)ANA_OUTPUT_SELECT_BG1);
        }
        else if (ADC_INTL_CH_SRC_TSENSOR0 == InternalChannelSrc)
        {
            Adc_Reg_SetTsensor0OutputValue((uint8)ANA_OUTPUT_SELECT_TSENSOR_ON);
        }
        else if (ADC_INTL_CH_SRC_TSENSOR1 == InternalChannelSrc)
        {
             Adc_Reg_SetTsensor1OutputValue((uint8)ANA_OUTPUT_SELECT_TSENSOR_ON);
        }
        else if (ADC_INTL_CH_SRC_XOSC == InternalChannelSrc)
        {
             Adc_Reg_SetAMUXXoscOutputEnable(TRUE);
        }
        else
        {
            /* Do nothing */
        }
    }
    Adc_Reg_SetAMUXOutputEnable(TRUE);
    Adc_Reg_SetAmuxInternalChannel(InternalChannelSrc);
}
#endif

static void Adc_DmaDeinit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    uint8 ChannelId = 0;

    /*PRQA S 4342-- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    /*PRQA S 4394 -- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    if (TRUE == Adc_Reg_GetDMAEnableFlag(Base))
    {
        /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        /*PRQA S 4342 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        ChannelId = Dma_Hal_GetChIdByReqSrc((Dma_RequestSourceType)((uint32)DMA_REQ_ADC0 + Instance));
        (void)Dma_Hal_StopCh(ChannelId);
        (void)Dma_Hal_EnableChIrq(ChannelId, (uint8)DMA_IRQ_NONE);
    }
}

static void Adc_CommonISR(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    Adc_InterruptInfoType Info = {0U};
    uint32 Event;
    const uint32 AmoMask = ADC_STR_AAMO_Msk | ADC_STR_NAMO_Msk | ADC_STR_AMO_Msk;
    /* take snapshot of current regular & injection group length */
    uint32 RegularLen = Adc_Reg_GetRegularLength(Base);
    uint32 InjectLen = Adc_Reg_GetInjectLength(Base);
    uint32 i;
    Adc_SequenceType Seq;

    /* Restrict RegularLen form above ADC_REGULAR_SEQ_NUM. No need for injection group for now */
    if (RegularLen > ADC_REGULAR_SEQ_NUM)
    {
        RegularLen = ADC_REGULAR_SEQ_NUM;
    }
    /* store device Status */
    Event = Adc_Reg_GetSTRFlag(Base);
    Event &= AmoMask;
    /* clear device Status, and clear AMO State only */
    Adc_Reg_ClearSTRFlag(Base, AmoMask);
    Info.Instance = Instance;
    /* call AMO interrupt handler */
    if ((Event > 0U) && (NULL_PTR != AdcISR[Instance].Callback))//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    {
        Info.Event = Event;
        /* Callback */
        AdcISR[Instance].Callback(&Info);//PRQA S 2842 # the upper layer call guarantees that there will never be an array out of bounds.*/
    }
    /* Process EOC interrupt */
    Event = 0U;
    for (i = 0U; i < InjectLen; i++)
    {
        /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        /*PRQA S 4342 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        Seq = (Adc_SequenceType)((uint32)ADC_ISEQ_0 + i);
        /*PRQA S 4394 -- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        /*PRQA S 4342 -- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        boolean IEOCCtrlFlag, IEOCFlag;
        IEOCCtrlFlag = Adc_Reg_GetInjectEOCInterruptEnableFlag(Base, Seq);
        IEOCFlag = Adc_Reg_GetInjectEOCFlag(Base, Seq);
        if (((boolean)TRUE == IEOCCtrlFlag) && ((boolean)TRUE == IEOCFlag))
        {
            /* clear EOC Status */
            Adc_Reg_ClearInjectEOCFlag(Base, Seq);
            /* save Event information */
            Info.sequence = Seq;
            Event = (uint32)ADC_EVENT_IEOC;
            break;
        }
    }
    /* call EOC interrupt handler */
    if (((uint32)ADC_EVENT_IEOC == Event)\
            && (NULL_PTR != AdcISR[Instance].Callback))//PRQA S 2842,2843 # the upper layer call guarantees that there will never be an array out of bounds.*/
    {
        Info.Event = Event;
        AdcISR[Instance].Callback(&Info);//PRQA S 2842,2843 # the upper layer call guarantees that there will never be an array out of bounds.*/
    }
    Event = 0U;
    for (i = 0U; i < RegularLen; i++)
    {
        Seq = (Adc_SequenceType)i;//PRQA S 4342  # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        boolean EOCCtrlFlag, EOCFlag;
        EOCCtrlFlag = Adc_Reg_GetRegularEOCInterruptEnableFlag(Base, Seq);
        EOCFlag = Adc_Reg_GetRegularEOCFlag(Base, Seq);
        if (((boolean)TRUE == EOCCtrlFlag) && ((boolean)TRUE == EOCFlag))
        {
            /* clear EOC Status */
            Adc_Reg_ClearRegularEOCFlag(Base, Seq);
            /* save Event information */
            Info.sequence = Seq;
            Event = (uint32)ADC_EVENT_EOC;
            break;
        }
    }
    /* call EOC interrupt handler */
    if (((uint32)ADC_EVENT_EOC == Event)\
            && (NULL_PTR != AdcISR[Instance].Callback))//PRQA S 2842,2843 # the upper layer call guarantees that there will never be an array out of bounds.*/
    {
        Info.Event = Event;
        AdcISR[Instance].Callback(&Info);//PRQA S 2842,2843 # the upper layer call guarantees that there will never be an array out of bounds.*/
    }
}

/**
 * @brief ADC0 Irq.
 * @return void
 */
ISR(ADC0_IRQHandler)//PRQA S 1503,3408 # macros function.*/
{
    Adc_CommonISR(0U);
}

/**
 * @brief ADC1 Irq.
 * @return void
 */
ISR(ADC1_IRQHandler)//PRQA S 1503,3408 # macros function.*/
{
    Adc_CommonISR(1U);
}

/* =============================================  EOF  ============================================== */
