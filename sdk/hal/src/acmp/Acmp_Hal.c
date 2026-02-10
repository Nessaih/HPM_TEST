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

/*!
 * @file Acmp_Hal.c
 *
 * @brief This file provides analog comparator module integration functions.
 *
 */
/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Acmp_Reg.h"
#include "Acmp_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/
/* Table of ACMP base address. */
static ACMP_Type *const Acmp_HalBase[ACMP_INSTANCE_MAX] =
{
    ACMP0,
#if defined (AC7843X)
    ACMP1
#endif
};

/* Table of ACMP IRQs. */
static const IRQn_Type Acmp_IrqId[ACMP_INSTANCE_MAX] = ACMP_IRQS;

/* Table of ACMP ckgen interface clocks */
static const Ckgen_BusClkIdType Acmp_HalBusClock[ACMP_INSTANCE_MAX] =
{
    CKGEN_ACMP0_BUS_CLK,
#if defined (AC7843X)
    CKGEN_ACMP1_BUS_CLK
#endif
};

/* Table of ACMP soft resets */
static const Rcm_ResetIDType Acmp_HalClockReset[ACMP_INSTANCE_MAX] =
{
    RCM_RESET_ID_ACMP0,
#if defined (AC7843X)
    RCM_RESET_ID_ACMP1
#endif
};

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/* ACMP interrupt callback array */
static Acmp_CallbackType Acmp_HalCallback[ACMP_INSTANCE_MAX];

/*==========================================FUNCTION PROTOTYPES=====================================*/
ISR(ACMP0_IRQHandler);

#if defined (AC7843X)
ISR(ACMP1_IRQHandler);
#endif

/*!
 * @brief Configure all ACMP comparator function with the given configuration structure.
 * @note Function ID: DES_ACMP_API_050
 * @param [in] Instance: ACMP Instance number
 * @param [in] Config: ACMP comparator configuration structure that need to apply
 * @return void.
 */
static void Acmp_Hal_ConfigComparator(uint8 Instance, const Acmp_ComparatorType *Config);

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get the current ACMP comparator configuration.
 * @note Function ID: DES_ACMP_API_051
 * @param [in] Instance: ACMP Instance number
 * @param [out] Config: ACMP comparator configuration structure that need to fill with current configuration
 * @return void.
 */
static void Acmp_Hal_GetComparatorConfig(uint8 Instance, Acmp_ComparatorType *const Config);
#endif

/*!
 * @brief Configure all DAC function with the given configuration structure.
 * @note Function ID: DES_ACMP_API_052
 * @param [in] Instance: ACMP Instance number
 * @param [in] Config: DAC configuration structure that need to apply
 * @return void.
 */
static void Acmp_Hal_ConfigDAC(uint8 Instance, const Acmp_DacType *Config);

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get the current DAC configuration.
 * @note Function ID: DES_ACMP_API_053
 * @param [in] Instance: ACMP Instance number
 * @param [out] Config: DAC configuration structure that need to fill with current configuration
 * @return void.
 */
static void Acmp_Hal_GetDACConfig(uint8 Instance, Acmp_DacType *const Config);
#endif

/*!
 * @brief Configure all input mux function with the given configuration structure.
 * @note Function ID: DES_ACMP_API_054
 * @param [in] Instance: ACMP Instance number
 * @param [in] Config: ACMP input mux configuration structure that need to apply
 * @return void.
 */
static void Acmp_Hal_ConfigMUX(uint8 Instance, const Acmp_AnmuxType *Config);

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get the current intput mux configuration.
 * @note Function ID: DES_ACMP_API_055
 * @param [in] Instance: ACMP Instance number
 * @param [out] Config: ACMP input mux configuration structure that need to fill with current configuration
 * @return void.
 */
static void Acmp_Hal_GetMUXConfig(uint8 Instance, Acmp_AnmuxType *const Config);
#endif

/*!
 * @brief Set ACMP polling mode.
 * @note Function ID: DES_ACMP_API_056
 * @param [in] Base: acmp module base
 * @param [in] Polling: polling mode
 *                      - ACMP_NONE_POLLING
 *                      - ACMP_POSITIVE_POLLING
 *                      - ACMP_NEGATIVE_POLLING
 * @return void
 */
static void Acmp_Hal_SetPollingMode(ACMP_Type *Base, const Acmp_InputPollingType Polling);

/*!
 * @brief Configure all polling function with the given configuration structure.
 * @note Function ID: DES_ACMP_API_057
 * @param[in] Instance: ACMP Instance number
 * @param[in] Config: ACMP polling mode configuration structure that need to apply
 * @return void.
 */
static void Acmp_Hal_ConfigPollingMode(uint8 Instance, const Acmp_PollingModeType *Config);

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get ACMP polling mode.
 * @note Function ID: DES_ACMP_API_058
 * @param [in] Base: acmp module base
 * @return current polling mode
 *                      - ACMP_NONE_POLLING
 *                      - ACMP_POSITIVE_POLLING
 *                      - ACMP_NEGATIVE_POLLING
 */
static Acmp_InputPollingType Acmp_Hal_GetPollingMode(const ACMP_Type *Base);

/*!
 * @brief Get the current polling mode configuration.
 * @note Function ID: DES_ACMP_API_059
 * @param[in] Instance: ACMP Instance number
 * @param[out] Config: ACMP polling mode configuration structure that need to fill with current configuration
 * @return void.
 */
static void Acmp_Hal_GetPollingModeConfig(uint8 Instance, Acmp_PollingModeType *const Config);
#endif

/*!
 * @brief ACMP interrupt service routine. ACMP interrupt Status flags will be cleared here.
 *
 * @param[in] Instance: ACMP Instance number
 * @return void
 */
static void Acmp_Hal_IRQHandler(uint8 Instance);

/*=================================GLOBAL FUNCTION IMPLEMENTATIONS=================================*/
#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get default configuration structure.
 * @note Function ID: DES_ACMP_API_008
 * @param [out] Config: ACMP configuration structure
 * @return void
 */
void Acmp_Hal_GetDefaultConfig(Acmp_ModuleType *Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    if (Config != NULL_PTR)
    {
        Config->Comparator.InterruptEn = FALSE;
        Config->Comparator.OutputTrigger = ACMP_BOTH_EDGES;
        Config->Comparator.OutputSelect = ACMP_COUT;
        Config->Comparator.FilterEnable = FALSE;
        Config->Comparator.FilterSampleCount = 0U;
        Config->Comparator.LpfBandwidth = ACMP_LPF_500KHZ;
        Config->Comparator.InverterEnable = FALSE;
        Config->Comparator.PinState = FALSE;
        Config->Comparator.HysteresisMode = ACMP_HYS_FALLING_EDGE;
        Config->Comparator.HysteresisLevel = ACMP_LEVEL_HYS_10MV;
        Config->Comparator.UsingLSIEnable = FALSE;
        Config->Comparator.WindowModeEnable = FALSE;
        Config->Comparator.ClockDivide = ACMP_FLT_DIVIDE_1;
        Config->Comparator.Callback = NULL_PTR;
        Config->Comparator.PowerEnable = TRUE;

        Config->Mux.PositiveInputMux = ACMP_EXTERNAL_CH1;
        Config->Mux.NegativeInputMux = ACMP_DAC_OUTPUT;

        Config->Dac.VoltageReferenceSource = ACMP_DAC_VDD;
        Config->Dac.Voltage = (0xFFU >> 1U);
        Config->Dac.State = TRUE;
#if defined (AC7842X) || defined (AC7843X)
        Config->Dac.OutToPin = FALSE;
        Config->Dac.BufferEnable = FALSE;
#endif

        Config->PollingMode.Mode = ACMP_NONE_POLLING;
        Config->PollingMode.PollingClockDivide = ACMP_CLK_DIVIDE_256;
        Config->PollingMode.PollingSequence = 0U;
        Config->PollingMode.HallOutputEnable = 0U;
        Config->PollingMode.HallAOutputCh = ACMP_EXTERNAL_CH0;
        Config->PollingMode.HallBOutputCh = ACMP_EXTERNAL_CH0;
        Config->PollingMode.HallCOutputCh = ACMP_EXTERNAL_CH0;
    }
}

/*!
 * @brief Reset all ACMP registers. Need to be called after ACMP clock is applied and reset is deasserted.
 * @note Function ID: DES_ACMP_API_009
 * @param [in] Instance: ACMP Instance number
 * @return void
 */
void Acmp_Hal_Reset(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    IRQn_Type Irq = Acmp_IrqId[Instance];

    /*!< Disable ACMP interrupt */
    Core_Hal_DisableIrq(Irq);
    Core_Hal_ClearPendingIrq(Irq);

    Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/
}
#endif

/*!
 * @brief Configure all comparator function with the given configuration structure.
 * @note Function ID: DES_ACMP_API_000
 * @param [in] Instance: ACMP Instance number
 * @param [in] Config: ACMP configuration structure that need to apply
 * @return void
 */
void Acmp_Hal_Init(uint8 Instance, const Acmp_ModuleType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    if (Config != NULL_PTR)
    {
        /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
        (void)Ckgen_Hal_EnablePeriphClk(Acmp_HalBusClock[Instance], TRUE);
        Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
        /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

        Acmp_Hal_ConfigComparator(Instance, &Config->Comparator);
        Acmp_Hal_ConfigDAC(Instance, &Config->Dac);
        Acmp_Hal_ConfigMUX(Instance, &Config->Mux);
        Acmp_Hal_ConfigPollingMode(Instance, &Config->PollingMode);
    }
}

/*!
 * @brief Reset ACMP module and close the clock source.
 * @note Function ID: DES_ACMP_API_001
 * @param [in] Instance: ACMP Instance number
 * @return void
 */
void Acmp_Hal_Deinit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    IRQn_Type Irq = Acmp_IrqId[Instance];

    /*!< Disable ACMP interrupt */
    Core_Hal_DisableIrq(Irq);

    Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(Acmp_HalBusClock[Instance], FALSE);

    Core_Hal_ClearPendingIrq(Irq);
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/
}

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get the current ACMP configuration.
 * @note Function ID: DES_ACMP_API_001
 * @param [in] Instance: ACMP Instance number
 * @param [out] Config: ACMP configuration structure that need to fill with current configuration
 * @return void.
 */
void Acmp_Hal_GetConfigAll(uint8 Instance, Acmp_ModuleType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    if (Config != NULL_PTR)
    {
        Acmp_Hal_GetComparatorConfig(Instance, &Config->Comparator);
        Acmp_Hal_GetDACConfig(Instance, &Config->Dac);
        Acmp_Hal_GetMUXConfig(Instance, &Config->Mux);
        Acmp_Hal_GetPollingModeConfig(Instance, &Config->PollingMode);
    }
}
#endif

/*!
 * @brief Get ACMP normal mode output data.
 * @note Function ID: DES_ACMP_API_002
 * @param [in] Instance: ACMP Instance number
 * @return output data.
 */
uint8 Acmp_Hal_GetOutputData(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    uint8 Data = 0U;
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Data = Acmp_Reg_GetOutputData(Base);

    return Data;
}

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get ACMP output Status flags.
 * @note Function ID: DES_ACMP_API_004
 * @param[in] Instance: ACMP Instance number
 * @return output Status flags.
 */
uint8 Acmp_Hal_GetOutputFlags(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    uint8 Flags = 0U;
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Flags = Acmp_Reg_GetOutputStatus(Base);

    return Flags;
}

/*!
 * @brief Clear ACMP output flags Status.
 * @note Function ID: DES_ACMP_API_005
 * @param[in] Instance: ACMP Instance number
 * @return void.
 */
void Acmp_Hal_ClearOutputFlags(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Acmp_Reg_ClearOutputStatus(Base);
}
#endif

/*!
 * @brief Get polling mode compare data.
 * @note Function ID: DES_ACMP_API_003
 * @param[in] Instance: ACMP Instance number
 * @return polling mode input channel compare data.
 */
uint16 Acmp_Hal_GetPollingData(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    uint16 Data = 0U;
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Data = Acmp_Reg_GetPollingData(Base);

    return Data;
}

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get polling mode Status flags.
 * @note Function ID: DES_ACMP_API_006
 * @param[in] Instance: ACMP Instance number
 * @return polling mode input channel Status.
 */
uint16 Acmp_Hal_GetPollingFlags(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    uint16 Flags = 0U;
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Flags = Acmp_Reg_GetPollingStatus(Base);

    return Flags;
}

/*!
 * @brief Clear polling mode Status flags.
 * @note Function ID: DES_ACMP_API_007
 * @param[in] Instance: ACMP Instance number
 * @return void.
 */
void Acmp_Hal_ClearInputFlags(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Acmp_Reg_ClearPollingStatus(Base);
}
#endif

/**
* @brief ACMP0 interrupt handler
* @note Function ID: DES_ACMP_API_010
* @return void
*/
ISR(ACMP0_IRQHandler)
{
    Acmp_Hal_IRQHandler(0U);
}

#if defined (AC7843X)
/**
* @brief ACMP1 interrupt handler
* @note Function ID: DES_ACMP_API_010
* @return void
*/
ISR(ACMP1_IRQHandler)
{
    Acmp_Hal_IRQHandler(1U);
}
#endif

/*==================================STATIC  FUNCTION  IMPLEMENTATIONS=================================*/
static void Acmp_Hal_ConfigComparator(uint8 Instance, const Acmp_ComparatorType *Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    IRQn_Type Irq = Acmp_IrqId[Instance];

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Acmp_Reg_SetInterruptEnableFlag(Base, Config->InterruptEn);
    Acmp_Reg_SetTriggerMode(Base, Config->OutputTrigger);
    Acmp_Reg_SetOutputSelection(Base, Config->OutputSelect);
    Acmp_Reg_SetFilterEnableFlag(Base, Config->FilterEnable);
    Acmp_Reg_SetFilterLength(Base, Config->FilterSampleCount);
    Acmp_Reg_SetFilterClockPrescaler(Base, Config->ClockDivide);
    Acmp_Reg_SetLowPassFilter(Base, Config->LpfBandwidth);
    Acmp_Reg_SetInvertMode(Base, Config->InverterEnable);
    Acmp_Reg_SetOutputPinEnableFlag(Base, Config->PinState);
    Acmp_Reg_SetHysteresisMode(Base, Config->HysteresisMode);
    Acmp_Reg_SetHysteresisVoltage(Base, Config->HysteresisLevel);
    /* Before changing clock source, should disable the ACMP power first */
    Acmp_Reg_SetUsingLSIEnableFlag(Base, Config->UsingLSIEnable);
#if defined (AC7840X)
    Acmp_Reg_SetLPEnableFlag(Config->UsingLSIEnable);
#elif defined (AC7842X)
    Acmp_Reg_SetLPEnableFlag(Base, Config->UsingLSIEnable);
#endif
    Acmp_Reg_SetWindowMode(Base, Config->WindowModeEnable);
    Acmp_Reg_SetEnableFlag(Base, Config->PowerEnable);

    Acmp_HalCallback[Instance] = Config->Callback;
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /* Config ACMP interrupt */
    if (TRUE == Config->InterruptEn)
    {
        Core_Hal_EnableIrq(Irq);
    }
    else
    {
        Core_Hal_DisableIrq(Irq);
        Core_Hal_ClearPendingIrq(Irq);
    }
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

#ifndef ACMP_SDK_NON_EXTENDED_API
static void Acmp_Hal_GetComparatorConfig(uint8 Instance, Acmp_ComparatorType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Config->InterruptEn = Acmp_Reg_GetInterruptEnableFlag(Base);
    Config->OutputTrigger = Acmp_Reg_GetTriggerMode(Base);
    Config->OutputSelect = Acmp_Reg_GetOutputSelection(Base);
    Config->FilterEnable = Acmp_Reg_GetFilterEnableFlag(Base);
    Config->FilterSampleCount = Acmp_Reg_GetFilterLength(Base);
    Config->ClockDivide = Acmp_Reg_GetFilterClockPrescaler(Base);
    Config->LpfBandwidth = Acmp_Reg_GetLowPassFilter(Base);
    Config->InverterEnable = Acmp_Reg_GetInvertMode(Base);
    Config->PinState = Acmp_Reg_GetOutputPinEnableFlag(Base);
    Config->HysteresisMode = Acmp_Reg_GetHysteresisMode(Base);
    Config->HysteresisLevel = Acmp_Reg_GetHysteresisVoltage(Base);
    Config->UsingLSIEnable = Acmp_Reg_GetUsingLSIEnableFlag(Base);
    Config->WindowModeEnable = Acmp_Reg_GetWindowMode(Base);
    Config->Callback = Acmp_HalCallback[Instance];
    Config->PowerEnable = Acmp_Reg_GetEnableFlag(Base);
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}
#endif

static void Acmp_Hal_ConfigDAC(uint8 Instance, const Acmp_DacType *Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Acmp_Reg_SetDACReference(Base, Config->VoltageReferenceSource);
    Acmp_Reg_SetDACOutput(Base, Config->Voltage);
    Acmp_Reg_SetDACEnableFlag(Base, Config->State);
#if defined (AC7842X) || defined (AC7843X)
    Acmp_Reg_SetDACOutputEnableFlag(Base, Config->OutToPin);
    Acmp_Reg_SetDACBufferEnableFlag(Base, Config->BufferEnable);
#endif
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

#ifndef ACMP_SDK_NON_EXTENDED_API
static void Acmp_Hal_GetDACConfig(uint8 Instance, Acmp_DacType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Config->VoltageReferenceSource = Acmp_Reg_GetDACReference(Base);
    Config->Voltage = Acmp_Reg_GetDACOutput(Base);
    Config->State = Acmp_Reg_GetDACEnableFlag(Base);
#if defined (AC7842X) || defined (AC7843X)
    Config->OutToPin = Acmp_Reg_GetDACOutputEnableFlag(Base);
    Config->BufferEnable = Acmp_Reg_GetDACBufferEnableFlag(Base);
#endif
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}
#endif

static void Acmp_Hal_ConfigMUX(uint8 Instance, const Acmp_AnmuxType *Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Acmp_Reg_SetPositiveInputPin(Base, Config->PositiveInputMux);
    Acmp_Reg_SetNegativeInputPin(Base, Config->NegativeInputMux);
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

#ifndef ACMP_SDK_NON_EXTENDED_API
static void Acmp_Hal_GetMUXConfig(uint8 Instance, Acmp_AnmuxType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Config->PositiveInputMux = Acmp_Reg_GetPositiveInputPin(Base);
    Config->NegativeInputMux = Acmp_Reg_GetNegativeInputPin(Base);
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}
#endif

static void Acmp_Hal_SetPollingMode(ACMP_Type *Base, const Acmp_InputPollingType Polling)
{
    boolean PositivePolling = FALSE;

    /* Not using polling mode */
    if (ACMP_NONE_POLLING == Polling)
    {
        Acmp_Reg_SetPositivePollingEnableFlag(Base, FALSE);
        Acmp_Reg_SetNegativePollingEnableFlag(Base, FALSE);
    }
    else
    {
        /* Use positive polling */
        if (ACMP_POSITIVE_POLLING == Polling)
        {
            PositivePolling = TRUE;
        }
        else /* Use Negative polling */
        {
            PositivePolling = FALSE;
        }
        Acmp_Reg_SetPositivePollingEnableFlag(Base, PositivePolling);
        Acmp_Reg_SetNegativePollingEnableFlag(Base, (TRUE == PositivePolling) ? FALSE : TRUE);
    }
}

static void Acmp_Hal_ConfigPollingMode(uint8 Instance, const Acmp_PollingModeType *Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Acmp_Hal_SetPollingMode(Base, Config->Mode);

    if (ACMP_NONE_POLLING != Config->Mode)
    {
        Acmp_Reg_SetPollingFreqDiv(Base, Config->PollingClockDivide);
        Acmp_Reg_SetPollingSequence(Base, Config->PollingSequence);
        Acmp_Reg_SetHallOutputEnableFlag(Base, Config->HallOutputEnable);
        Acmp_Reg_SetHallOutputA(Base, Config->HallAOutputCh);
        Acmp_Reg_SetHallOutputB(Base, Config->HallBOutputCh);
        Acmp_Reg_SetHallOutputC(Base, Config->HallCOutputCh);
    }
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}

#ifndef ACMP_SDK_NON_EXTENDED_API
static Acmp_InputPollingType Acmp_Hal_GetPollingMode(const ACMP_Type *Base)
{
    Acmp_InputPollingType Polling = ACMP_NONE_POLLING;
    boolean Positive = Acmp_Reg_GetPositivePollingEnableFlag(Base);
    boolean Negative = Acmp_Reg_GetNegativePollingEnableFlag(Base);

    /* Use positive polling */
    if ((TRUE == Positive) && (FALSE == Negative))
    {
        Polling = ACMP_POSITIVE_POLLING;
    }
    else if ((FALSE == Positive) && (TRUE == Negative)) /* Use Negative polling */
    {
        Polling = ACMP_NEGATIVE_POLLING;
    }
    else /* Not using polling mode */
    {
        Polling = ACMP_NONE_POLLING;
    }

    return Polling;
}

static void Acmp_Hal_GetPollingModeConfig(uint8 Instance, Acmp_PollingModeType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Config->PollingClockDivide = Acmp_Reg_GetPollingFreqDiv(Base);
    Config->Mode = Acmp_Hal_GetPollingMode(Base);
    Config->PollingSequence = Acmp_Reg_GetPollingSequence(Base);
    Config->HallOutputEnable = Acmp_Reg_GetHallOutputEnableFlag(Base);
    Config->HallAOutputCh = Acmp_Reg_GetHallOutputA(Base);
    Config->HallBOutputCh = Acmp_Reg_GetHallOutputB(Base);
    Config->HallCOutputCh = Acmp_Reg_GetHallOutputC(Base);
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
}
#endif

static void Acmp_Hal_IRQHandler(uint8 Instance)
{
    ACMP_Type *Base = Acmp_HalBase[Instance];
    uint32 state = 0U;

    /* Get all interrupt Status */
    state = Acmp_Reg_GetInterruptStatus(Base);

    /* Clear all interrtup Status */
    Acmp_Reg_ClearInterruptStatus(Base, state);

    /* Call the customized interrtup handler function */
    if (NULL_PTR != Acmp_HalCallback[Instance])
    {
        Acmp_HalCallback[Instance](Instance, state);
    }
}
/* =============================================  EOF  ============================================== */
