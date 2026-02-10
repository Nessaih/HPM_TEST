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

/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Pwm_Reg.h"
#include "Ckgen_Hal.h"
#include "Pwm_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"
/* =========================================== LOCAL VARIABLES ============================================== */
/* PWM overflow interrupt request info */
static const IRQn_Type PwmOverflowIrq[PWM_INSTANCE_MAX] = PWM_OVERFLOW_IRQS;

/* PWM channel interrupt request info */
static const IRQn_Type PwmChannelIrq[PWM_INSTANCE_MAX] = PWM_CHANNEL_IRQS;

/*PRQA S 3218 ++ # File scope static, 'PwmFaultIrq', is only accessed in one function.. */
/* PWM fault interrupt request info */
static const IRQn_Type PwmFaultIrq[PWM_INSTANCE_MAX] = PWM_FAULT_IRQS;

#if defined (AC7843X)
/* PWM detect interrupt request info */
static const IRQn_Type PwmDetectIrq[PWM_INSTANCE_MAX] = PWM_DETECT_IRQS;
#endif
/*PRQA S 3218 -- */

/* PWM base address */
#if defined (PWM_BASE_PTRS)
static PWM_Type *const PwmBaseAddr[PWM_INSTANCE_MAX] = PWM_BASE_PTRS;
#else
static PWM_Type *const PwmBaseAddr[PWM_INSTANCE_MAX] = {PWM0, PWM1, PWM2, PWM3, PWM4, PWM5};
#endif

/*PRQA S 0686 ++ # Array has fewer initializers than its declared size. Default initialization is applied to the remainder of the array elements. */
static boolean PwmIsCaptureMode[PWM_INSTANCE_MAX] = {FALSE};

/* PWM overflow callback pointer */
static Pwm_Hal_Callback Pwm_OverflowCallback[PWM_INSTANCE_MAX] = {NULL_PTR};

/* PWM channel callback pointer */
static Pwm_Hal_Callback Pwm_ChannelCallback[PWM_INSTANCE_MAX] = {NULL_PTR};

#ifndef PWM_SDK_NON_EXTENDED_API
/* PWM fault callback pointer */
static Pwm_Hal_Callback Pwm_FaultCallback[PWM_INSTANCE_MAX] = {NULL_PTR};

#if defined (AC7843X)
/* PWM quad phase z detect callback pointer */
static Pwm_Hal_Callback Pwm_ZDetectCallback[PWM_INSTANCE_MAX] = {NULL_PTR};
#endif
#endif
/*PRQA S 0686 -- */

/* PWM bus clock id */
static const Ckgen_BusClkIdType Pwm_ClkId[PWM_INSTANCE_MAX] =
{
    CKGEN_PWM0_BUS_CLK, CKGEN_PWM1_BUS_CLK, CKGEN_PWM2_BUS_CLK,
    CKGEN_PWM3_BUS_CLK, CKGEN_PWM4_BUS_CLK, CKGEN_PWM5_BUS_CLK
#if defined (AC7843X)
    , CKGEN_PWM6_BUS_CLK, CKGEN_PWM7_BUS_CLK
#endif
};

#if defined (AC7840X)
/* Record the clock status of each instance */
static uint16 Pwm_InstancesClkEnabledState = 0U;
#endif

/* =====================================  Functions definition  ===================================== */

/*
 * @brief Pwm_Hal_ChannelType :get combine channel 2nd channeltype
 * @note Function ID:
 * @param[in] Channel: channel number
 * @return Pwm_Hal_ChannelType:
 */
static Pwm_Hal_ChannelType Pwm_Hal_GetPairChannel
(
    uint8 Channel
)
{
    /*return channel*/
    Pwm_Hal_ChannelType HalChannel = PWM_CHANNEL_1;

    /*matching channel*/
    switch (Channel)
    {
    case 1U:
        HalChannel = PWM_CHANNEL_1;
        break;
    case 3U:
        HalChannel = PWM_CHANNEL_3;
        break;
    case 5U:
        HalChannel = PWM_CHANNEL_5;
        break;
    case 7U:
        HalChannel = PWM_CHANNEL_7;
        break;
    default:
        /* Illegal param. */
        break;
    }

    return HalChannel;
}

/**
 * @brief: Pwm_Hal_InitCommonAttr: init pwm common attributes.
 * @note Function ID:
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Config: pwm common config pointer
 * @return: void
 */
static void Pwm_Hal_InitCommonAttr
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_CommonCfg *Config
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];

    /* Set pwm module at reset state and enable bus clock. */
    (void)Ckgen_Hal_EnablePeriphClk(Pwm_ClkId[(uint8)Instance], TRUE);
    /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
    Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance), RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance), RCM_RESET_STATE_DEASSERT);
#if defined (AC7840X)
    Pwm_InstancesClkEnabledState |= ((uint16)1 << (uint8)Instance);
    /* Turning off the clocks of PWMn and PWMn-1 will cause PWMn to fail to turn on the clock (n is an odd number). */
    if (TRUE == ((uint8)Instance % 2U))
    {
        /* If PWM (Instance -1) is not in operation, turn on its clock. */
        if (0U == (Pwm_InstancesClkEnabledState & ((uint16)1 << ((uint8)Instance - 1U))))
        {
            /* Set pwm module at reset state and enable bus clock. */
            (void)Ckgen_Hal_EnablePeriphClk(Pwm_ClkId[(uint8)Instance - 1U], TRUE);
            Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance - 1U), \
                                  RCM_RESET_STATE_ASSERT);
            Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance - 1U), \
                                  RCM_RESET_STATE_DEASSERT);
        }
    }
#endif
    /*PRQA S 4342 -- */
    /*PRQA S 4394 -- */

    /* Set counter related register. */
    Pwm_Reg_ResetCounter(Base);
    Pwm_Reg_SetInitCountValue(Base, Config->MinCount);
    Pwm_Reg_SetMaxCountValue(Base, Config->MaxCount);
    Pwm_Reg_SetPeriodDither(Base, Config->PeriodDither);

    /* Set the number of overflow times to wait for the next CNTOF setting */
    Pwm_Reg_SetCntOverflowFreq(Base, Config->OverflowFreq);

    /* DMA Setting */
    Pwm_Reg_EnableOverflowDmaReq(Base, Config->EnableOverflowDmaReq);
    Pwm_Reg_EnableDmaTransfer(Base, Config->EnableDmaTransLen);
    Pwm_Reg_SetDmaTransferLen(Base, Config->DmaTransLen);

    Core_Hal_EnableIrq(PwmChannelIrq[(uint8)Instance]);
    if (TRUE == Config->EnableOverflowInterrupt)
    {
        Core_Hal_EnableIrq(PwmOverflowIrq[(uint8)Instance]);
    }
    else
    {
        Core_Hal_DisableIrq(PwmOverflowIrq[(uint8)Instance]);
    }
    Pwm_Reg_EnableOverflowInterrupt(Base, Config->EnableOverflowInterrupt);
    Pwm_Reg_EnableOverflowEvent(Base, Config->EnableOverflowEvent);
    Pwm_Reg_EnableUnderflowEvent(Base, Config->EnableUnderflowEvent);
    Pwm_OverflowCallback[(uint8)Instance] = Config->OverflowCallback;
    Pwm_ChannelCallback[(uint8)Instance] = Config->ChannelCallback;

    /*PRQA S 2812 -- */
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */
    if (NULL_PTR != Config)
    {
        PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
        const Pwm_Hal_OutputChannelCfg *ChCfg;
        uint32 ChannelDitherValue[2U] = {0U, 0U};
        Pwm_Hal_ChannelType Combine2ndChannel = PWM_CHANNEL_1;
        Pwm_Hal_ChannelPairType ChPair = PWM_CHANNEL_PAIR_0;

        Pwm_Hal_InitCommonAttr(Instance, Config->CommonCfg.BaseCfg);

        /* Set module up count or up-down count mode*/
        Pwm_Reg_SetCountMode(Base, Config->CommonCfg.CountMode);

        /* trigger function init */
        Pwm_Reg_EnableInitTrigger(Base, Config->CommonCfg.EnableInitTrigger);
        Pwm_Reg_EnableMaxTrigger(Base, Config->CommonCfg.EnableMaxTrigger);
        Pwm_Reg_SetTriggerRatio(Base, Config->CommonCfg.TriggerRatio);

        Pwm_Reg_SetCombineCenterDutyType(Base, Config->CommonCfg.CombineCenterDutyMode);

        for (uint8 i = 0U; i < Config->ChannelNum; i++)
        {
            ChCfg = &Config->ChannelCfg[i];

            switch (ChCfg->ChannelMode)
            {
            case OUTPUT_INDEPENDENT:
                Pwm_Reg_SetChannelOutputInitLevel(Base, ChCfg->Channel, ChCfg->IndependentChnCfg->InitLevel);
                Pwm_Reg_SetChannelMSR(Base, ChCfg->Channel, 0x02U);
                Pwm_Reg_SetChannelELSR(Base, ChCfg->Channel, (0x01U << (uint32)ChCfg->IndependentChnCfg->LevelMode));
                Pwm_Reg_EnableChannelDmaReq(Base, ChCfg->Channel, ChCfg->IndependentChnCfg->EnableChnEventDmaReq);
                Pwm_Reg_SetChannelPolarity(Base, ChCfg->Channel, ChCfg->IndependentChnCfg->ActivePolarity);
                Pwm_Reg_EnableChannelMatchTrigger(Base, ChCfg->Channel, ChCfg->IndependentChnCfg->EnableMatchTrigger);
                Pwm_Reg_EnableChannelInterrupt(Base, ChCfg->Channel, ChCfg->IndependentChnCfg->EnableInterrupt);

                Pwm_Reg_SetChannelValue(Base, ChCfg->Channel, ChCfg->IndependentChnCfg->ChnValue);
                ChannelDitherValue[(uint8)ChCfg->Channel / 4U] |= (\
                        ((uint32)ChCfg->IndependentChnCfg->ChnDither & PWM_DITHER0_C0DHR_Msk) << \
                        (((uint32)ChCfg->Channel % 4U) * PWM_DITHER_VALUE_WIDTH));

                break;
            case OUTPUT_COMBINE:
                /* Check the channel is even number */
                DEVICE_ASSERT(0U == ((uint8)ChCfg->Channel % 2U));

                Combine2ndChannel = Pwm_Hal_GetPairChannel((uint8)ChCfg->Channel + 1U);
                /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
                /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
                ChPair = (Pwm_Hal_ChannelPairType)((uint8)ChCfg->Channel >> 0x01U);
                /*PRQA S 4342 -- */
                /*PRQA S 4394 -- */

                /* Set init output level */
                Pwm_Reg_SetChannelOutputInitLevel(Base, ChCfg->Channel, \
                                                  ChCfg->ChnPairCfg->Ch1stInitLevel);
                Pwm_Reg_SetChannelOutputInitLevel(Base, Combine2ndChannel, \
                                                  ChCfg->ChnPairCfg->Ch2ndInitLevel);

                Pwm_Reg_SetChannelELSR(Base, ChCfg->Channel, (0x01U << (uint32)ChCfg->ChnPairCfg->LevelMode));

                Pwm_Reg_EnableChannelCombine(Base, ChPair, TRUE);
                Pwm_Reg_EnablePairChSymmetric(Base, ChPair, ChCfg->ChnPairCfg->EnableSymmetric);
                Pwm_Reg_EnablePairChComplement(Base, ChPair, ChCfg->ChnPairCfg->EnableChComplementation);

                /* Set channels match dir */
                Pwm_Reg_SetChannelMatchDir(Base, ChCfg->Channel, ChCfg->ChnPairCfg->Ch1stMatchDir);
                Pwm_Reg_SetChannelMatchDir(Base, Combine2ndChannel, ChCfg->ChnPairCfg->Ch2ndMatchDir);

                /* Enable deadtime */
                Pwm_Reg_EnablePairChDeadtime(Base, ChPair, ChCfg->ChnPairCfg->EnableDeadTime);
                Pwm_Reg_SetPairChDeadtime(Base, ChPair, \
                                          ChCfg->ChnPairCfg->TimePsc, \
                                          ChCfg->ChnPairCfg->DeadTimeValue);

                Pwm_Reg_EnableChannelDmaReq(Base, ChCfg->Channel, ChCfg->ChnPairCfg->EnableCh1stEventDmaReq);
                Pwm_Reg_EnableChannelDmaReq(Base, Combine2ndChannel, ChCfg->ChnPairCfg->EnableCh2ndEventDmaReq);

                Pwm_Reg_SetChannelPolarity(Base, ChCfg->Channel, ChCfg->ChnPairCfg->Ch1stActivePolarity);
                Pwm_Reg_SetChannelPolarity(Base, Combine2ndChannel, ChCfg->ChnPairCfg->Ch2ndActivePolarity);

                /* enable or disable channel match trigger */
                Pwm_Reg_EnableChannelMatchTrigger(Base, ChCfg->Channel, \
                                                  ChCfg->ChnPairCfg->EnableCh1stMatchTrigger);
                Pwm_Reg_EnableChannelMatchTrigger(Base, Combine2ndChannel, \
                                                  ChCfg->ChnPairCfg->EnableCh2ndMatchTrigger);

                /* enable or disable channel Interrupt */
                Pwm_Reg_EnableChannelInterrupt(Base, ChCfg->Channel, ChCfg->ChnPairCfg->EnableCh1stInterrupt);
                Pwm_Reg_EnableChannelInterrupt(Base, Combine2ndChannel, ChCfg->ChnPairCfg->EnableCh2ndInterrupt);

                /* Set channels match value */
                Pwm_Reg_SetChannelValue(Base, ChCfg->Channel, ChCfg->ChnPairCfg->Ch1stValue);
                Pwm_Reg_SetChannelValue(Base, Combine2ndChannel, ChCfg->ChnPairCfg->Ch2ndValue);
                ChannelDitherValue[(uint8)ChCfg->Channel / 4U] |= (\
                        ((uint32)ChCfg->ChnPairCfg->Ch1stDither & PWM_DITHER0_C0DHR_Msk) << \
                        (((uint32)ChCfg->Channel % 4U) * PWM_DITHER_VALUE_WIDTH));
                ChannelDitherValue[(uint8)ChCfg->Channel / 4U] |= (\
                        ((uint32)ChCfg->ChnPairCfg->Ch2ndDither & PWM_DITHER0_C0DHR_Msk) << \
                        ((((uint32)ChCfg->Channel % 4U) + 1U) * PWM_DITHER_VALUE_WIDTH));

                break;
            case OUTPUT_COMPARE:
                Pwm_Reg_SetChannelOutputInitLevel(Base, ChCfg->Channel, ChCfg->CompareChnCfg->InitLevel);
                Pwm_Reg_SetChannelMSR(Base, ChCfg->Channel, 0x01U);
                Pwm_Reg_SetChannelELSR(Base, ChCfg->Channel, (uint8)ChCfg->CompareChnCfg->Action);
                Pwm_Reg_EnableChannelInterrupt(Base, ChCfg->Channel, ChCfg->CompareChnCfg->EnableInterrupt);
                Pwm_Reg_SetChannelPolarity(Base, ChCfg->Channel, ChCfg->CompareChnCfg->ActivePolarity);
                Pwm_Reg_EnableChannelMatchTrigger(Base, ChCfg->Channel, ChCfg->CompareChnCfg->EnableMatchTrigger);
                Pwm_Reg_EnableChannelDmaReq(Base, ChCfg->Channel, ChCfg->CompareChnCfg->EnableChnEventDmaReq);
                Pwm_Reg_SetChannelValue(Base, ChCfg->Channel, ChCfg->CompareChnCfg->ChnValue);

                break;
            default:
                /* Useless param. */
                break;
            }
        }
        Pwm_Reg_SetChannelMatchDitherReg(Base, 0U, ChannelDitherValue[0U]);
        Pwm_Reg_SetChannelMatchDitherReg(Base, 1U, ChannelDitherValue[1U]);

        Pwm_Reg_EnableOutputInit(Base, Config->CommonCfg.InitOutput);

        Pwm_Reg_SetClockPsc(Base, Config->CommonCfg.BaseCfg->Prescaler);
        Pwm_Reg_SetClockSource(Base, Config->CommonCfg.BaseCfg->ClockSource);
    }
    /*PRQA S 2812 -- */
}

/**
 * @brief: Pwm_Hal_InitInputMode: init pwm input mode
 * @note Function ID: DES_PWM_API_202
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Config: pwm input config pointer
 * @return: void
 */
void Pwm_Hal_InitInputMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_InputCfg *Config
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */
    if (NULL_PTR != Config)
    {
        PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
        const Pwm_Hal_InputChannelCfg *ChCfg;
        Pwm_Hal_ChannelType Combine2ndChannel = PWM_CHANNEL_1;
        Pwm_Hal_ChannelPairType ChPair = PWM_CHANNEL_PAIR_0;

        Pwm_Hal_InitCommonAttr(Instance, Config->CommonCfg.BaseCfg);

        /* Set module up count or up-down count mode*/
        Pwm_Reg_SetCountMode(Base, UP_COUNT);

        if (TRUE != Config->CommonCfg.EnableHall)
        {
            PwmIsCaptureMode[(uint8)Instance] = TRUE;
        }

        for (uint8 i = 0; i < Config->ChannelNum; i++)
        {
            ChCfg = &Config->ChannelCfg[i];

            if (INPUT_SINGLE == ChCfg->ChannelMode)
            {
                Pwm_Reg_SetChannelELSR(Base, ChCfg->Channel, (uint8)ChCfg->CaptureEdge);
                Pwm_Reg_SetChannelMSR(Base, ChCfg->Channel, 0U);
                Pwm_Reg_SetCaptureEventPsc(Base, ChCfg->Channel, ChCfg->EventPsc);
                Pwm_Reg_EnableChannelEventReset(Base, ChCfg->Channel, ChCfg->EnableCounterReset);
                Pwm_Reg_EnableChannelDmaReq(Base, ChCfg->Channel, ChCfg->EnableChnEventDmaReq);
                Pwm_Reg_EnableChannelInterrupt(Base, ChCfg->Channel, ChCfg->EnableInterrupt);
            }
            else if (INPUT_DUAL == ChCfg->ChannelMode)
            {
                /* Check the channel is even number */
                DEVICE_ASSERT(0U == ((uint8)ChCfg->Channel % 2U));

                Combine2ndChannel = Pwm_Hal_GetPairChannel((uint8)ChCfg->Channel + 1U);
                /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
                /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
                ChPair = (Pwm_Hal_ChannelPairType)((uint8)ChCfg->Channel >> 1U);
                /*PRQA S 4342 -- */
                /*PRQA S 4394 -- */
                /* Set measure type */
                switch (ChCfg->DualInputMeasureType)
                {
                case PWM_POSITIVE_PLUSE_WIDTH_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, ChCfg->Channel, 1U);
                    Pwm_Reg_SetChannelELSR(Base, Combine2ndChannel, 2U);
                    break;

                case PWM_NEGATIVE_PLUSE_WIDTH_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, ChCfg->Channel, 2U);
                    Pwm_Reg_SetChannelELSR(Base, Combine2ndChannel, 1U);
                    break;

                case PWM_RISING_EDGE_PERIOD_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, ChCfg->Channel, 1U);
                    Pwm_Reg_SetChannelELSR(Base, Combine2ndChannel, 1U);
                    break;

                case PWM_FALLING_EDGE_PERIOD_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, ChCfg->Channel, 2U);
                    Pwm_Reg_SetChannelELSR(Base, Combine2ndChannel, 2U);
                    break;

                case PWM_BOTH_EDGE_DUTY_CYCLE_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, ChCfg->Channel, 3U);
                    Pwm_Reg_SetChannelELSR(Base, Combine2ndChannel, 3U);
                    break;

                default:
                    /* Illegal param. */
                    break;
                }

                Pwm_Reg_SetChannelMSR(Base, ChCfg->Channel, (uint8)ChCfg->DualInputContinuousMode);
                Pwm_Reg_SetChannelMSR(Base, Combine2ndChannel, (uint8)ChCfg->DualInputContinuousMode);
                Pwm_Reg_SetCaptureEventPsc(Base, Combine2ndChannel, ChCfg->EventPsc);
                Pwm_Reg_EnableChannelDmaReq(Base, Combine2ndChannel, ChCfg->EnableChnEventDmaReq);

                Pwm_Reg_EnablePairChDualEdgeCapture(Base, ChPair, TRUE);
                Pwm_Reg_SetPairChDualEdgeCapture(Base, ChPair, TRUE);

                if (PWM_BOTH_EDGE_DUTY_CYCLE_MEASURE == ChCfg->DualInputMeasureType)
                {
                    Pwm_Reg_EnableChannelEventReset(Base, ChCfg->Channel, ChCfg->EnableCounterReset);
                    Pwm_Reg_EnableChannelEventReset(Base, Combine2ndChannel, ChCfg->EnableCounterReset);
                    Pwm_Reg_EnableChannelInterrupt(Base, ChCfg->Channel, ChCfg->EnableInterrupt);
                }
                else
                {
                    Pwm_Reg_EnableChannelEventReset(Base, Combine2ndChannel, ChCfg->EnableCounterReset);
                    /* Just set channel n+1 interrupt */
                    Pwm_Reg_EnableChannelInterrupt(Base, Combine2ndChannel, ChCfg->EnableInterrupt);
                    if (TRUE == ChCfg->EnablePulseWidthMeasure)
                    {
                        Pwm_Reg_EnableChannelEventReset(Base, ChCfg->Channel, ChCfg->EnableCounterReset);
                    }
                }
                Pwm_Reg_EnablePairPulseWidthMeasurement(Base, ChPair, ChCfg->EnablePulseWidthMeasure);

                if (TRUE == Config->CommonCfg.EnableHall)
                {
                    /* config CH2 - phaze C */
                    /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
                    Pwm_Reg_EnablePairChDualEdgeCapture(Base, (Pwm_Hal_ChannelPairType)1U, TRUE);
                    Pwm_Reg_SetPairChDualEdgeCapture(Base, (Pwm_Hal_ChannelPairType)1U, TRUE);
                    /*PRQA S 4342 -- */

                    Pwm_Reg_EnableChannelEventReset(Base, ChCfg->Channel, ChCfg->EnableCounterReset);
                    Pwm_Reg_EnableChannelInterrupt(Base, ChCfg->Channel, ChCfg->EnableInterrupt);

                }
                Pwm_Reg_EnableHall(Base, Config->CommonCfg.EnableHall);
            }
            else
            {
                /* reserve */
            }

            /* Enable filtering for input channels */
            if (PWM_CHANNEL_4 > ChCfg->Channel)
            {
                Pwm_Reg_SetChannelInputFilterVal(Base, ChCfg->Channel, ChCfg->FilterValue);
            }
        }

        Pwm_Reg_SetInputFilterPsc(Base, Config->CommonCfg.FilterPsc);

        /* Enable clock */
        Pwm_Reg_SetClockPsc(Base, Config->CommonCfg.BaseCfg->Prescaler);
        Pwm_Reg_SetClockSource(Base, Config->CommonCfg.BaseCfg->ClockSource);
    }
    /*PRQA S 2812 -- */
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];

    Pwm_Hal_InitCommonAttr(Instance, Config->BaseCfg);

    Pwm_Reg_SetInputFilterPsc(Base, Config->FilterPsc);
    /* Set polarity for Phase A and Phase B */
    Pwm_Reg_SetQuadPhaseAPolarity(Base, Config->PhaseAConfig.Polarity);
    Pwm_Reg_SetQuadPhaseBPolarity(Base, Config->PhaseBConfig.Polarity);
    Pwm_Reg_SetQuadPhaseZPolarity(Base, Config->PhaseZConfig.Polarity);
    Pwm_Reg_EnableQuadPhaseZResetCnt(Base, Config->EnablePhaseZReset);
#if defined (AC7843X)
    Pwm_Reg_SetQuadPhaseZResetMode(Base, Config->PhaseZResetMode);
    Pwm_Reg_EnableQuadPhaseZInterrupt(Base, Config->EnablePhaseZInterrupt);
    Pwm_ZDetectCallback[(uint8)Instance] = Config->ZDetectCallback;

    if (TRUE == Config->EnablePhaseZInterrupt)
    {
        Core_Hal_EnableIrq(PwmDetectIrq[Instance]);
    }
    else
    {
        Core_Hal_DisableIrq(PwmDetectIrq[Instance]);
    }
#endif

    /* phaseA corresponding channel 0 */
    Pwm_Reg_SetChannelInputFilterVal(Base, PWM_CHANNEL_0, Config->PhaseAConfig.FilterValue);

    /* phaseB corresponding channel 1 */
    Pwm_Reg_SetChannelInputFilterVal(Base, PWM_CHANNEL_1, Config->PhaseBConfig.FilterValue);

    /* phaseZ corresponding channel 2 */
    Pwm_Reg_SetChannelInputFilterVal(Base, PWM_CHANNEL_2, Config->PhaseZConfig.FilterValue);

    Pwm_Reg_SetQuadEncodeMode(Base, Config->Mode);
    Pwm_Reg_EnableQuadDecoder(Base, Config->EnableQuad);

    Pwm_Reg_SetClockPsc(Base, Config->BaseCfg->Prescaler);
    Pwm_Reg_SetClockSource(Base, Config->BaseCfg->ClockSource);

    /*PRQA S 2812 -- */
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    if ((uint32)Instance < PWM_INSTANCE_MAX)
    {
        Core_Hal_DisableIrq(PwmOverflowIrq[Instance]);
        Core_Hal_DisableIrq(PwmChannelIrq[Instance]);
        Core_Hal_DisableIrq(PwmFaultIrq[Instance]);
#if defined (AC7843X)
        Core_Hal_DisableIrq(PwmDetectIrq[Instance]);
#endif

        /* Set pwm module at reset state and disable bus clock. */
        /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
        Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance), RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance), RCM_RESET_STATE_DEASSERT);
        (void)Ckgen_Hal_EnablePeriphClk(Pwm_ClkId[(uint8)Instance], FALSE);
#if defined (AC7840X)
        if (1U == ((uint8)Instance % 2U))
        {
            /* If PWM (Instance -1) is not in operation, turn off its clock. */
            if (0U == (Pwm_InstancesClkEnabledState & ((uint16)1 << ((uint8)Instance - 1U))))
            {
                Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance - 1U), \
                                      RCM_RESET_STATE_ASSERT);
                Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance - 1U), \
                                      RCM_RESET_STATE_DEASSERT);
                (void)Ckgen_Hal_EnablePeriphClk(Pwm_ClkId[(uint8)Instance - 1U], FALSE);
            }
        }
        Pwm_InstancesClkEnabledState &= (~((uint16)1 << (uint8)Instance));
#endif
        /*PRQA S 4342 -- */
        /*PRQA S 4394 -- */
        PwmIsCaptureMode[(uint8)Instance] = FALSE;

        Core_Hal_ClearPendingIrq(PwmOverflowIrq[Instance]);
        Core_Hal_ClearPendingIrq(PwmChannelIrq[Instance]);
        Core_Hal_ClearPendingIrq(PwmFaultIrq[Instance]);
#if defined (AC7843X)
        Core_Hal_ClearPendingIrq(PwmDetectIrq[Instance]);
#endif
    }
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetClockPsc(Base, ClkPsc);
    Pwm_Reg_SetClockSource(Base, ClkSource);
}

/**
 * @brief: Pwm_Hal_ResetCounter: reset module counter value
 * @note Function ID: DES_PWM_API_212
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: void
 */
void Pwm_Hal_ResetCounter
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ResetCounter(Base);
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint16 Value = Pwm_Reg_GetCountValue(Base);

    return Value;
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetMaxCountValue(Base, Value);
}

/**
 * @brief: Pwm_Hal_GetMaxCountValue: Get pwm module counter max value.
 * @note Function ID: DES_PWM_API_207
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint16:max value
 */
uint16 Pwm_Hal_GetMaxCountValue
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint16 Value = Pwm_Reg_GetMaxCountValue(Base);

    return Value;
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetInitCountValue(Base, Value);
}

/**
 * @brief: Pwm_Hal_GetInitCountValue: Get pwm module counter init value.
 * @note Function ID: DES_PWM_API_209
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint16: Init value
 */
uint16 Pwm_Hal_GetInitCountValue
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint16 Value = Pwm_Reg_GetInitCountValue(Base);

    return Value;
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelValue(Base, Channel, Value);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint16 ChV = Pwm_Reg_GetChannelValue(Base, Channel);

    return ChV;
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Mask = (0x1UL << (uint8)Channel);
    Pwm_Reg_ClearChannelEventFlag(Base, Mask);
    Pwm_Reg_ClearChannelInterruptFlag(Base, Channel);
    Pwm_Reg_EnableChannelInterrupt(Base, Channel, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableOverflowInterrupt(Base, Enable);
    if (TRUE == Enable)
    {
        Core_Hal_EnableIrq(PwmOverflowIrq[(uint8)Instance]);
    }
    else
    {
        Core_Hal_DisableIrq(PwmOverflowIrq[(uint8)Instance]);
    }
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableOverflowEvent(Base, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableUnderflowEvent(Base, Enable);
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Flag = Pwm_Reg_GetChannelEventFlag(Base);

    return Flag;
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearChannelEventFlag(Base, Mask);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Flag = Pwm_Reg_GetOverflowFlag(Base);

    return Flag;
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearOverflowFlag(Base);
}

/**
 * @brief: Pwm_Hal_GetOverFlowDir: get pwm Overflow direction
 * @note Function ID: DES_PWM_API_221
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint32: channel OverFlow flag
 */
uint32 Pwm_Hal_GetOverflowDir
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Flag = Pwm_Reg_GetOverflowDir(Base);

    return Flag;
}

/**
 * @brief: Pwm_Hal_GetAllChannelLevel: get all pwm channel level
 * @note Function ID: DES_PWM_API_222
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint32: channel level
 */
uint32 Pwm_Hal_GetAllChannelLevel
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Levels = Pwm_Reg_GetAllChannelLevel(Base);

    return Levels;
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelDmaReq(Base, Channel, Enable);
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(NULL_PTR != ConfigPtr);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */

    uint8 i = 0U;
    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];

    /* Set synchronization trigger mode */
    Pwm_Reg_SetPOLTriggerMode(Base, ConfigPtr->TriggerType, TRUE);
    Pwm_Reg_SetCHOSWCRSyncTriggerMode(Base, ConfigPtr->TriggerType, TRUE);
    Pwm_Reg_SetINVCRSyncTriggerMode(Base, ConfigPtr->TriggerType, TRUE);
    Pwm_Reg_SetOMCRSyncTriggerMode(Base, ConfigPtr->TriggerType, TRUE);
    Pwm_Reg_SetMCVRSyncMode(Base, ConfigPtr->TriggerType, TRUE);

    Pwm_Reg_EnablePOLSync(Base, ConfigPtr->EnablePolaritySync);
    Pwm_Reg_EnableCHOSWCRSync(Base, ConfigPtr->EnableSwOutputCtrlSync);
    Pwm_Reg_EnableINVCRSync(Base, ConfigPtr->EnableDualChannelInvertSync);
    Pwm_Reg_EnableOutputMaskSync(Base, ConfigPtr->EnableOutputMaskSync);
    Pwm_Reg_EnableCNTINSync(Base, ConfigPtr->EnableCounterInitSync);

    for (i = 0U; i < (uint8)PWM_CHANNEL_PAIR_NUM; i++)
    {
        /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
        Pwm_Reg_EnablePairChValueSync(Base, (Pwm_Hal_ChannelPairType)i, ConfigPtr->EnablePairedChnValSync[i]);
        /*PRQA S 4342 -- */
    }

    if ((FALSE == ConfigPtr->EnableMaxLoadingPoint) && (FALSE == ConfigPtr->EnableMinLoadingPoint))
    {
        /* update immediately when no load point is set */
        Pwm_Reg_SetCNTSyncTriggerMode(Base, ConfigPtr->TriggerType, TRUE);
    }
    else
    {
        Pwm_Reg_SetCNTSyncTriggerMode(Base, ConfigPtr->TriggerType, FALSE);
    }
    Pwm_Reg_EnableMaxLoadingPointSync(Base, ConfigPtr->EnableMaxLoadingPoint);
    Pwm_Reg_EnableMinLoadingPointSync(Base, ConfigPtr->EnableMinLoadingPoint);

    Pwm_Reg_EnableSyncHwTriggerSrc(Base, 0U, ConfigPtr->EnableHwSync0);
    Pwm_Reg_EnableSyncHwTriggerSrc(Base, 1U, ConfigPtr->EnableHwSync1);
    Pwm_Reg_EnableSyncHwTriggerSrc(Base, 2U, ConfigPtr->EnableHwSync2);
    Pwm_Reg_DisableHwTriggerSyncAfterTrigged(Base, ConfigPtr->DisableHwTriggerAfterTrigged);

    Pwm_Reg_SetSyncType(Base, 0U);
    Pwm_Reg_EnableSync(Base, ConfigPtr->EnableSync);
    Pwm_Reg_EnableSyncBYP(Base, ConfigPtr->EnableSyncBypass);

    /*PRQA S 2812 -- */

    Pwm_Reg_SetSyncMode(Base, PWM_SYNC_MODE_ENHANCED);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableSync(Base, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableSyncBYP(Base, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_TrigSoftwareSync(Base, IsTrigger);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableGlobalTimeBase(Base, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableGlobalTimeBaseOutput(Base, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableInitTrigger(Base, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableMaxTrigger(Base, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelMatchTrigger(Base, Channel, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetTriggerRatio(Base, TriggerRatio);
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelELSR(Base, Channel, (uint8)Edge);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelInputFilterVal(Base, Channel, Value);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetInputFilterPsc(Base, Prescaler);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelEventReset(Base, Channel, Enable);
}

/**
 * @brief: Pwm_Hal_GetHallStatus: Get pwm hall status.
 * @note Function ID: DES_PWM_API_236
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: Hall status.
 */
uint32 Pwm_Hal_GetHallStatus
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Status = Pwm_Reg_GetHallStatus(Base);

    return Status;
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];

    Pwm_Reg_SetChannelELSR(Base, Channel, (uint8)Action);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetSoftControlEnableStatesMask(Base, EnableMask);
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelSoftControl(Base, Channel, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint8 Mask = (uint8)Pwm_Reg_ReadChannelSoftControlState(Base);
    if (TRUE == Enable)
    {
        Mask = (Mask | (0x03U << (uint8)Channel));
    }
    else
    {
        Mask = (Mask & (~(0x03U << (uint8)Channel)));
    }
    Pwm_Reg_SetSoftControlEnableStatesMask(Base, Mask);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetSoftControlLevelMask(Base, LevelMask);
}
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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelSoftControlLevel(Base, Channel, Level);
}

#ifndef PWM_SDK_NON_EXTENDED_API
/*!
 * @brief: Pwm_Hal_SetChannelMatchDither: Set channel dither value.
 * @note Function ID: DES_PWM_API_243
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @param[in] Channel: PWM_CHANNEL_0\1\2\3\4\5\6\7
 * @param[in] DitherValue: Channel match dither value (0 ~ 31).
 * @return: void
 */
void Pwm_Hal_SetChannelMatchDither
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint8 DitherValue
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelMatchDither(Base, Channel, DitherValue);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetPeriodDither(Base, MaxCountDitherValue);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelOutputMask(Base, Channel, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetOutputMask(Base, Mask);
}

/**
 * @brief: Pwm_Hal_GetOutputMask: get pwm moudel channel output status
 * @note Function ID: DES_PWM_API_246
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: uint32: channel output state
 */
uint32 Pwm_Hal_GetOutputMask
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Mask = Pwm_Reg_GetOutputMask(Base);

    return Mask;
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelPolarity(Base, Channel, Polarity);
}

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
 *            - 0 ~ 1023
 * @return void
 */
void Pwm_Hal_SetDeadtime
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    Pwm_Hal_DeadTimePscType Prescaler,
    uint16 Value
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetPairChDeadtime(Base, ChannelPair, Prescaler, Value);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnablePairChSymmetric(Base, ChannelPair, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnablePairChInvert(Base, ChannelPair, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(NULL_PTR != ConfigPtr);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint8 FaultInputId = (uint8)PWM_FAULT_INPUT_0;

    if (TRUE == ConfigPtr->EnableInterrupt)
    {
        Core_Hal_EnableIrq(PwmFaultIrq[Instance]);
    }
    else
    {
        Core_Hal_DisableIrq(PwmFaultIrq[Instance]);
    }
    Pwm_Reg_EnableFaultInterrupt(Base, ConfigPtr->EnableInterrupt);
    Pwm_FaultCallback[(uint8)Instance] = ConfigPtr->FaultCallback;

    /* Control the output channels. */
    Pwm_Reg_EnableFaultHizOutput(Base, ConfigPtr->EnableHiz);
    for (uint8 i = 0U; i < (uint8)PWM_CHANNEL_PAIR_NUM; i++)
    {
        /*PRQA S 4394 ++ # A composite expression of 'essentially unsigned' type is being cast to a different type category. */
        /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
        Pwm_Reg_EnablePairChFaultCtrl(Base, (Pwm_Hal_ChannelPairType)i, ConfigPtr->EnableChannelOutputCtrl[i]);
        /*PRQA S 4342 -- */
        /*PRQA S 4394 -- */
    }

    Pwm_Reg_SetFaultMode(Base, ConfigPtr->FaultCtrlMode);
    Pwm_Reg_SetInputFilterPsc(Base, ConfigPtr->FilterPsc);
    Pwm_Reg_SetFaultInputFilterValue(Base, ConfigPtr->FilterValue);

#if defined (AC7843X) || defined (AC7842X)
    Pwm_Reg_RecoverZeroOrFullDutyAfterFaultCleared(Base, TRUE);
#endif

    for (FaultInputId = (uint8)PWM_FAULT_INPUT_0; FaultInputId < (uint8)PWM_FAULT_INPUT_MAX; FaultInputId++)
    {
        /*PRQA S 4394 ++ # A composite expression of 'essentially unsigned' type is being cast to a different type category. */
        /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
        Pwm_Reg_EnableFaultInputFilter(Base, (Pwm_Hal_FaultInputIdType)FaultInputId, \
                                       ConfigPtr->FaultPinCfg[FaultInputId].EnableFaultFilter);
        Pwm_Reg_SetFaultInputPolarity(Base, (Pwm_Hal_FaultInputIdType)FaultInputId, \
                                      ConfigPtr->FaultPinCfg[FaultInputId].FaultPolarity);
        Pwm_Reg_EnableFaultPinInput(Base, (Pwm_Hal_FaultInputIdType)FaultInputId, \
                                    ConfigPtr->FaultPinCfg[FaultInputId].EnableFaultInput);
        /*PRQA S 4342 -- */
        /*PRQA S 4394 -- */
    }

    /*PRQA S 2812 -- */
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableFaultPinInput(Base, FaultInputId, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableFaultInputFilter(Base, FaultInputId, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetFaultInputPolarity(Base, FaultInputId, ActivePolarity);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetFaultPinFlag(Base, FaultInputId);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearFaultPinFlag(Base, FaultInputId);
}

/*!
 * @brief: Pwm_Hal_GetFaultFlag: Get the OR value of each fault input pin flag.
 * @note Function ID: DES_PWM_API_258
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return The OR value of each fault input pin flag
 */
uint32 Pwm_Hal_GetFaultFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetFaultFlag(Base);
}

/*!
 * @brief: Pwm_Hal_ClearFaultFlag: Clear the OR value of each fault input pin flag.
 * @note Function ID: DES_PWM_API_259
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return void
 */
void Pwm_Hal_ClearFaultFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearFaultFlag(Base);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelHizOutput(Base, Channel, Enable);
}

/*!
 * @brief: Pwm_Hal_GetQuadCountingDir: Get the current quadrature decoding count direction.
 * @note Function ID: DES_PWM_API_262
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return counting direction
 */
uint32 Pwm_Hal_GetQuadCountingDir
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetQuadCountDir(Base);
}

/*!
 * @brief: Pwm_Hal_GetQuadOverflowDir: Get the quadrature timer overflow direction.
 * @note Function ID: DES_PWM_API_263
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return timer overflow direction
 */
uint32 Pwm_Hal_GetQuadOverflowDir
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetQuadOverflowDir(Base);
}

/*!
 * @brief: Pwm_Hal_GetQuadPhaseZFlag: Get the phaseZ Status.
 * @note Function ID: DES_PWM_API_264
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return phaseZ Status
 */
uint32 Pwm_Hal_GetQuadPhaseZFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetQuadPhaseZFlag(Base);
}

/*!
 * @brief: Pwm_Hal_ClearQuadPhaseZFlag: Clear the phaseZ Status.
 * @note Function ID: DES_PWM_API_265
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return void
 */
void Pwm_Hal_ClearQuadPhaseZFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearQuadPhaseZFlag(Base);
}

/*!
 * @brief: Pwm_Hal_GetDetectEventFlag: Check Z index detect event status.
 * @note Function ID:
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return Z index event status
 */
uint32 Pwm_Hal_GetDetectEventFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetQuadPhaseZFlag(Base);
}

/*!
 * @brief: Pwm_Hal_ClearDetectEventFlag: Clear Z index detect event status.
 * @note Function ID:
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return void
 */
void Pwm_Hal_ClearDetectEventFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearQuadPhaseZFlag(Base);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableWriteProtection(Base, Enable);
}

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
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetDebugMode(Base, Mode);
}
#endif

/**
* @brief: Pwm_Hal_Overflow_IrqHandler: overflow Interrupt handle
* @note Function ID:
* @param[in] Instance: Enum Pwm_Hal_InstanceType
* @return: void
*/
static void Pwm_Hal_Overflow_IrqHandler
(
    Pwm_Hal_InstanceType Instance
)
{
    /*overflow event flag*/
    uint32 Flag;
    Flag = Pwm_Hal_GetOverflowDir(Instance);
    Pwm_Hal_ClearOverflowFlag(Instance);
    /*overflow callback is not null*/
    if (NULL_PTR != Pwm_OverflowCallback[(uint8)Instance])
    {
        Pwm_OverflowCallback[Instance](Instance, Flag, (void *) NULL_PTR);
    }
}

/**
* @brief: Pwm_Hal_Channel_IrqHandler: channel Interrupt handle
* @note Function ID:
* @param[in] Instance: Enum Pwm_Hal_InstanceType
* @return: void
*/
static void Pwm_Hal_Channel_IrqHandler
(
    Pwm_Hal_InstanceType Instance
)
{
    /*channel event flag*/
    uint32 Flag;
    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Flag = Pwm_Hal_GetChannelInterruptFlag(Instance);

    /*channel event is true*/
    if (0UL != (Flag & PWM_STR_CH_EVENT_Msk))
    {
        if (TRUE == PwmIsCaptureMode[Instance])
        {
            /* The interrupt flag should be cleared by the user code */

            /* If the interrupt flag of all channels is directly cleared in ISR, the interrupt flag of the first edge of
               the low-frequency dual channel capture mode may be mistakenly cleared by the ISR of the high-frequency
               capture mode channel before the second edge capture is completed */
        }
        /* other mode */
        else
        {
            Pwm_Reg_ClearChannelEventFlag(Base, Flag);
        }
    }
    /*channel callback is not null*/
    if (NULL_PTR != Pwm_ChannelCallback[Instance])
    {
        Pwm_ChannelCallback[Instance](Instance, Flag, (void *) NULL_PTR);
    }
}

#ifndef PWM_SDK_NON_EXTENDED_API
/**
* @brief: Pwm_Hal_Fault_IrqHandler: fault Interrupt handle
* @note Function ID:
* @param[in] Instance: Enum Pwm_Hal_InstanceType
* @return: void
*/
static void Pwm_Hal_Fault_IrqHandler
(
    Pwm_Hal_InstanceType Instance
)
{
    /* store channel status and clear */
    uint32 Flag = Pwm_Hal_GetFaultFlag(Instance);
    if (TRUE == Flag)
    {
        /* clear fault flag will affect the output behavior, so it is oprated by the user*/
    }

    if (NULL_PTR != Pwm_FaultCallback[Instance])
    {
        Pwm_FaultCallback[(uint8)Instance](Instance, Flag, (void *) NULL_PTR);
    }
}

#if defined (AC7843X)
/*!
 * @brief: Pwm_Hal_Detect_IrqHandler: phase z detect Interrupt handle
 * @note Function ID:
 * @param[in] Instance: Enum Pwm_Hal_InstanceType
 * @return: void
 */
static void Pwm_Hal_Detect_IrqHandler
(
    Pwm_Hal_InstanceType Instance
)
{
    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];

    uint32 Flag = Pwm_Reg_GetQuadPhaseZFlag(Base);
    if (Flag != 0U)
    {
        Pwm_Reg_ClearQuadPhaseZFlag(Base);
    }

    if (NULL_PTR != Pwm_ZDetectCallback[Instance])
    {
        /* callback */
        Pwm_ZDetectCallback[Instance](Instance, Flag, (void *) NULL_PTR);
    }
}
#endif
#endif

/*PRQA S 3408 ++ # IRQHandler is used in startup.s */

/**
* @brief: PWM0_Overflow_IRQHandler: PWM0 overflow Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM0_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_0);
}

/**
* @brief: PWM1_Overflow_IRQHandler: PWM1 overflow Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM1_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_1);
}

/**
* @brief: PWM2_Overflow_IRQHandler: PWM2 overflow Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM2_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_2);
}

/**
* @brief: PWM3_Overflow_IRQHandler: PWM3 overflow Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM3_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_3);
}

/**
* @brief: PWM4_Overflow_IRQHandler: PWM4 overflow Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM4_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_4);
}

/**
* @brief: PWM5_Overflow_IRQHandler: PWM5 overflow Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM5_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_5);
}

#if defined (AC7843X)
/**
* @brief: PWM6_Overflow_IRQHandler: PWM6 overflow Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM6_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_6);
}

/**
* @brief: PWM7_Overflow_IRQHandler: PWM7 overflow Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM7_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_7);
}
#endif

/**
* @brief: PWM0_Channel_IRQHandler: PWM0 channel Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM0_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_0);
}

/**
* @brief: PWM1_Channel_IRQHandler: PWM1 channel Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM1_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_1);
}

/**
* @brief: PWM2_Channel_IRQHandler: PWM2 channel Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM2_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_2);
}

/**
* @brief: PWM3_Channel_IRQHandler: PWM3 channel Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM3_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_3);
}

/**
* @brief: PWM4_Channel_IRQHandler: PWM4 channel Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM4_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_4);
}

/**
* @brief: PWM5_Channel_IRQHandler: PWM5 channel Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM5_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_5);
}

#if defined (AC7843X)
/**
* @brief: PWM6_Channel_IRQHandler: PWM6 channel Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM6_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_6);
}

/**
* @brief: PWM7_Channel_IRQHandler: PWM7 channel Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM7_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_7);
}
#endif

#ifndef PWM_SDK_NON_EXTENDED_API
/**
* @brief: PWM0_Fault_IRQHandler: PWM0 fault Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM0_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_0);
}

/**
* @brief: PWM1_Fault_IRQHandler: PWM1 fault Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM1_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_1);
}

/**
* @brief: PWM2_Fault_IRQHandler: PWM2 fault Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM2_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_2);
}

/**
* @brief: PWM3_Fault_IRQHandler: PWM3 fault Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM3_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_3);
}

/**
* @brief: PWM4_Fault_IRQHandler: PWM4 fault Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM4_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_4);
}

/**
* @brief: PWM5_Fault_IRQHandler: PWM5 fault Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM5_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_5);
}

#if defined (AC7843X)
/**
* @brief: PWM6_Fault_IRQHandler: PWM6 fault Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM6_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_6);
}

/**
* @brief: PWM7_Fault_IRQHandler: PWM7 fault Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM7_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_7);
}
#endif

#if defined (AC7843X)
/**
* @brief: PWM0_Detect_IRQHandler: PWM0 detect Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM0_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_0);
}

/**
* @brief: PWM1_Detect_IRQHandler: PWM1 detect Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM1_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_1);
}

/**
* @brief: PWM2_Detect_IRQHandler: PWM2 detect Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM2_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_2);
}

/**
* @brief: PWM3_Detect_IRQHandler: PWM3 detect Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM3_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_3);
}

/**
* @brief: PWM4_Detect_IRQHandler: PWM4 detect Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM4_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_4);
}

/**
* @brief: PWM5_Detect_IRQHandler: PWM5 detect Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM5_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_5);
}

/**
* @brief: PWM6_Detect_IRQHandler: PWM6 detect Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM6_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_6);
}

/**
* @brief: PWM7_Detect_IRQHandler: PWM7 detect Interrupt handle
* @note Function ID:
* @return: void
*/
void PWM7_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_7);
}
#endif
#endif

/*PRQA S 3408 -- */

/* =============================================  EOF  ============================================== */
