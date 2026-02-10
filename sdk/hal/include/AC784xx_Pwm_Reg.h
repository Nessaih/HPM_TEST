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

/*!
 * @file AC784xx_Pwm_Reg.h
 *
 * @brief This file provides pwm hardware integration interface.
 *
 */

#ifndef AC784XX_PWM_REG_H
#define AC784XX_PWM_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Pwm_Hal_Types.h"
#include "Device_Register.h"

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

LOCAL_INLINE PWM_Type *Pwm_Reg_GetBase(Pwm_Hal_InstanceType Instance)
{
    /* PRQA S 0306 ++ */ /* Cast between a pointer to object and an integral type. */
#if defined (PWM_BASE_PTRS)
    PWM_Type *const PwmBaseAddr[PWM_INSTANCE_MAX] = PWM_BASE_PTRS;
#else
    PWM_Type *const PwmBaseAddr[PWM_INSTANCE_MAX] = {PWM0, PWM1, PWM2, PWM3, PWM4, PWM5};
#endif
    /* PRQA S 0306 -- */ /* Cast between a pointer to object and an integral type. */

    PWM_Type *Base = PwmBaseAddr[Instance];

    return Base;
}

LOCAL_INLINE void Pwm_Reg_SetClockSource(PWM_Type *Base, Pwm_Hal_ClockSourceType ClkSource)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CLKSRC_Msk, PWM_INIT_CLKSRC_Pos, ClkSource);
}

LOCAL_INLINE void Pwm_Reg_SetClockPsc(PWM_Type *Base, uint16 ClkPsc)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CLKPSC_Msk, PWM_INIT_CLKPSC_Pos, ClkPsc);
}

LOCAL_INLINE void Pwm_Reg_SetCountMode(PWM_Type *Base, Pwm_Hal_CountModeType Mode)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CNTMODE_Msk, PWM_INIT_CNTMODE_Pos, Mode);
}

LOCAL_INLINE void Pwm_Reg_SetMaxCountValue(PWM_Type *Base, uint16 Value)
{
    WRITE_REG32(Base->MCVR, Value);
}

LOCAL_INLINE uint16 Pwm_Reg_GetMaxCountValue(const PWM_Type *Base)
{
    uint32 Value = READ_BIT32(Base->MCVR, PWM_MCVR_MCVR_Msk);

    return (uint16)Value;
}

LOCAL_INLINE void Pwm_Reg_SetInitCountValue(PWM_Type *Base, uint16 Value)
{
    WRITE_REG32(Base->CNTIN, Value);
}

LOCAL_INLINE uint16 Pwm_Reg_GetInitCountValue(const PWM_Type *Base)
{
    uint32 Value = READ_BIT32(Base->CNTIN, PWM_CNTIN_CNTINIT_Msk);

    return (uint16)Value;
}

LOCAL_INLINE void Pwm_Reg_ResetCounter(PWM_Type *Base)
{
    WRITE_REG32(Base->CNT, 0UL);
}

LOCAL_INLINE uint16 Pwm_Reg_GetCountValue
(
    const PWM_Type *Base
)
{
    uint32 Flag = READ_BIT32(Base->CNT, PWM_CNT_COUNT_Msk);

    return (uint16)Flag;
}

LOCAL_INLINE void Pwm_Reg_SetPeriodDither(PWM_Type *Base, uint8 Value)
{
    MODIFY_REG32(Base->DITHER2, PWM_DITHER2_PDHR_Msk, PWM_DITHER2_PDHR_Pos, Value);
}

LOCAL_INLINE void Pwm_Reg_SetCntOverflowFreq(PWM_Type *Base, uint8 Freq)
{
    MODIFY_REG32(Base->CONF, PWM_CONF_CNTOFNUM_Msk, PWM_CONF_CNTOFNUM_Pos, Freq);
}

LOCAL_INLINE void Pwm_Reg_EnableOverflowDmaReq(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_OFUFDMAEN_Msk, PWM_INIT_OFUFDMAEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableDmaTransfer(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->DMACTRL, PWM_DMACTRL_TRANS_EN_Msk, PWM_DMACTRL_TRANS_EN_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetDmaTransferLen(PWM_Type *Base, uint8 Len)
{
    MODIFY_REG32(Base->DMACTRL, PWM_DMACTRL_TRANS_LEN_Msk, PWM_DMACTRL_TRANS_LEN_Pos, Len);
}

LOCAL_INLINE void Pwm_Reg_EnableOutputInit(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_INIT_Msk, PWM_FUNCSEL_INIT_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableChannelDmaReq(PWM_Type *Base, Pwm_Hal_ChannelType Channel, boolean Enable)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_DMAEN_Msk, PWM_CH0SCR_DMAEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetChannelMSR(PWM_Type *Base, Pwm_Hal_ChannelType Channel, uint8 Value)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, \
                 PWM_CH0SCR_MSR0_Msk | PWM_CH0SCR_MSR1_Msk, \
                 PWM_CH0SCR_MSR0_Pos, \
                 ((uint32)Value & 0x03UL));
}

LOCAL_INLINE void Pwm_Reg_SetChannelELSR(PWM_Type *Base, Pwm_Hal_ChannelType Channel, uint8 Value)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, \
                 PWM_CH0SCR_ELSR0_Msk | PWM_CH0SCR_ELSR1_Msk, \
                 PWM_CH0SCR_ELSR0_Pos, \
                 ((uint32)Value & 0x03UL));
}

LOCAL_INLINE void Pwm_Reg_SetChannelValue(PWM_Type *Base, Pwm_Hal_ChannelType Channel, uint16 Value)
{
    WRITE_REG32((Base)->CHANNELS[Channel].CHnV, Value);
}

LOCAL_INLINE uint16 Pwm_Reg_GetChannelValue(const PWM_Type *Base, Pwm_Hal_ChannelType Channel)
{
    uint32 Value = READ_BIT32((Base)->CHANNELS[Channel].CHnV, PWM_CH0V_CHCVAL_Msk);

    return (uint16)Value;
}

LOCAL_INLINE void Pwm_Reg_SetChannelMatchDitherReg(PWM_Type *Base, uint8 DitherRegNum, uint32 RegValue)
{
    if (0U == DitherRegNum)
    {
        WRITE_REG32(Base->DITHER0, RegValue);
    }
    else
    {
        WRITE_REG32(Base->DITHER1, RegValue);
    }
}

LOCAL_INLINE void Pwm_Reg_SetChannelMatchDither(PWM_Type *Base, Pwm_Hal_ChannelType Channel, uint8 DitherValue)
{
    if (PWM_CHANNEL_4 > Channel)
    {
        MODIFY_REG32(Base->DITHER0, \
                     (uint32)PWM_DITHER0_C0DHR_Msk << (PWM_DITHER_VALUE_WIDTH * ((uint32)Channel % 4U)), \
                     (uint32)PWM_DITHER_VALUE_WIDTH * ((uint32)Channel % 4U), \
                     DitherValue);
    }
    else
    {
        MODIFY_REG32(Base->DITHER1, \
                     (uint32)PWM_DITHER0_C0DHR_Msk << (PWM_DITHER_VALUE_WIDTH * ((uint32)Channel % 4U)), \
                     (uint32)PWM_DITHER_VALUE_WIDTH * ((uint32)Channel % 4U), \
                     DitherValue);
    }

}

LOCAL_INLINE void Pwm_Reg_EnableOverflowInterrupt(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CNTOIE_Msk, PWM_INIT_CNTOIE_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableChannelInterrupt(PWM_Type *Base, Pwm_Hal_ChannelType Channel, boolean Enable)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_CHIE_Msk, PWM_CH0SCR_CHIE_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableOverflowEvent(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_COFE_Msk, PWM_INIT_COFE_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableUnderflowEvent(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CUFE_Msk, PWM_INIT_CUFE_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetChannelOutputInitLevel
(
    PWM_Type *Base,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_OutputLevelType Level
)
{
    MODIFY_REG32(Base->OUTINIT, (uint32)PWM_OUTINIT_CH0OIV_Msk << ((uint8)Channel), ((uint8)Channel), Level);
}

LOCAL_INLINE void Pwm_Reg_EnableChannelCombine(PWM_Type *Base, Pwm_Hal_ChannelPairType ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, ((uint32)PWM_MODESEL_PAIR0COMBINEN_Msk << ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnablePairChSymmetric(PWM_Type *Base, Pwm_Hal_ChannelPairType ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, (uint32)PWM_MODESEL_PAIR0SYMEN_Msk << ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 PWM_MODESEL_PAIR0SYMEN_Pos + ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnablePairChComplement(PWM_Type *Base, Pwm_Hal_ChannelPairType ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, ((uint32)PWM_MODESEL_PAIR0COMPEN_Msk << ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 PWM_MODESEL_PAIR0COMPEN_Pos + ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnablePairChValueSync(PWM_Type *Base, Pwm_Hal_ChannelPairType ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, (uint32)PWM_MODESEL_PAIR0SYNCEN_Msk << ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH),
                 PWM_MODESEL_PAIR0SYNCEN_Pos + ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnablePairChDeadtime(PWM_Type *Base, Pwm_Hal_ChannelPairType ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, ((uint32)PWM_MODESEL_PAIR0DTEN_Msk << ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (PWM_MODESEL_PAIR0DTEN_Pos + ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetPairChDeadtime
(
    PWM_Type *Base,
    Pwm_Hal_ChannelPairType ChannelPair,
    Pwm_Hal_DeadTimePscType Psc,
    uint16 Value
)
{
    if ((uint8)ChannelPair < 2U)
    {
        MODIFY_REG32(Base->DTSET0, \
                     ((uint32)PWM_DTSET0_DTPSC0_Msk << ((uint32)ChannelPair * PWM_DTSET_CONFIG_WIDTH)), \
                     (PWM_DTSET0_DTPSC0_Pos + ((uint32)ChannelPair * PWM_DTSET_CONFIG_WIDTH)), \
                     (uint32)Psc);
        MODIFY_REG32(Base->DTSET0, \
                     ((uint32)PWM_DTSET0_DTVAL0_Msk << ((uint32)ChannelPair * PWM_DTSET_CONFIG_WIDTH)), \
                     (PWM_DTSET0_DTVAL0_Pos + ((uint32)ChannelPair * PWM_DTSET_CONFIG_WIDTH)), \
                     (uint32)Value);
    }
    else
    {
        MODIFY_REG32(Base->DTSET1, \
                     ((uint32)PWM_DTSET1_DTPS2_Msk << (((uint32)ChannelPair - 2UL) * PWM_DTSET_CONFIG_WIDTH)), \
                     (PWM_DTSET1_DTPS2_Pos + (((uint32)ChannelPair - 2UL) * PWM_DTSET_CONFIG_WIDTH)), \
                     Psc);
        MODIFY_REG32(Base->DTSET1, \
                     ((uint32)PWM_DTSET1_DTVAL2_Msk << (((uint32)ChannelPair - 2UL) * PWM_DTSET_CONFIG_WIDTH)), \
                     (PWM_DTSET1_DTVAL2_Pos + (((uint32)ChannelPair - 2UL) * PWM_DTSET_CONFIG_WIDTH)), \
                     (uint32)Value);
    }
}

LOCAL_INLINE void Pwm_Reg_EnablePairChInvert(PWM_Type *Base, Pwm_Hal_ChannelPairType ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->INVCR, (uint32)PWM_INVCR_PAIR0INVEN_Msk << (uint8)ChannelPair, (uint8)ChannelPair,
                 (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetCombineCenterDutyType(PWM_Type *Base, Pwm_Hal_CombineCenterDutyModeType DutyMode)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_PDYTPEN_Msk, PWM_FUNCSEL_PDYTPEN_Pos, DutyMode);
}

LOCAL_INLINE void Pwm_Reg_SetChannelMatchDir(PWM_Type *Base, Pwm_Hal_ChannelType Channel,
        Pwm_Hal_ChannelMatchDirType Dir)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_DIR_Msk, PWM_CH0SCR_DIR_Pos, Dir);
}

LOCAL_INLINE void Pwm_Reg_EnablePairPulseWidthMeasurement(PWM_Type *Base, Pwm_Hal_ChannelPairType ChannelPair,
        boolean State)
{
    MODIFY_REG32(Base->FUNCSEL, (uint32)PWM_FUNCSEL_CHP0PNWEN_Msk << (uint32)ChannelPair,
                 PWM_FUNCSEL_CHP0PNWEN_Pos + (uint32)ChannelPair, State);
}

LOCAL_INLINE void Pwm_Reg_EnableHall(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_HALLEN_Msk, PWM_FUNCSEL_HALLEN_Pos, (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE uint32 Pwm_Reg_GetHallStatus(const PWM_Type *Base)
{
    uint32 Status = READ_BIT32((Base)->STR, PWM_STR_HALL_STATUS_Msk);
    Status = Status >> PWM_STR_HALL_STATUS_Pos;

    return Status;
}

LOCAL_INLINE void Pwm_Reg_EnableInitTrigger(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->EXTTRIG, PWM_EXTTRIG_INITTRIGEN_Msk, PWM_EXTTRIG_INITTRIGEN_Pos, (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableMaxTrigger(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->EXTTRIG, PWM_EXTTRIG_MAXTRIGEN_Msk, PWM_EXTTRIG_MAXTRIGEN_Pos, (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableChannelMatchTrigger(PWM_Type *Base, Pwm_Hal_ChannelType Channel, boolean State)
{
    MODIFY_REG32(Base->EXTTRIG, (uint32)PWM_EXTTRIG_CH0TRIG_Msk << ((uint8)Channel), ((uint8)Channel), (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetTriggerRatio(PWM_Type *Base, uint8 Ratio)
{
    DEVICE_ASSERT(Ratio <= 0x07U);
    MODIFY_REG32(Base->EXTTRIG, PWM_EXTTRIG_TRIGRATIO_Msk, PWM_EXTTRIG_TRIGRATIO_Pos, Ratio);
}

LOCAL_INLINE uint32 Pwm_Reg_GetChannelTriggerFlag(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32(Base->EXTTRIG, PWM_EXTTRIG_TRIGF_Msk);
    Flag = Flag >> PWM_EXTTRIG_TRIGF_Pos;

    return Flag;
}

LOCAL_INLINE void Pwm_Reg_ClearChannelTriggerFlag(PWM_Type *Base)
{
#if defined (AC7843X)
    SET_BIT32(Base->EXTTRIG, PWM_EXTTRIG_TRIGF_Msk);
#else
    CLEAR_BIT32(Base->EXTTRIG, PWM_EXTTRIG_TRIGF_Msk);
#endif
}

LOCAL_INLINE void Pwm_Reg_EnableChannelOutputMask(PWM_Type *Base, Pwm_Hal_ChannelType Channel, boolean Enable)
{
    MODIFY_REG32(Base->OMCR, (uint32)PWM_OMCR_CH0OMEN_Msk << (uint8)Channel, (uint8)Channel, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetOutputMask(PWM_Type *Base, uint8 Mask)
{
    MODIFY_REG32(Base->OMCR, PWM_OMCR_CH_Msk, PWM_OMCR_CH0OMEN_Pos, Mask);
}

LOCAL_INLINE uint32 Pwm_Reg_GetOutputMask(const PWM_Type *Base)
{
    uint32 Mask = READ_BIT32(Base->OMCR, PWM_OMCR_CH_Msk);

    return Mask;
}

LOCAL_INLINE void Pwm_Reg_SetChannelPolarity(PWM_Type *Base, Pwm_Hal_ChannelType Channel,
        Pwm_Hal_ActivePolarityType Polarity)
{
    MODIFY_REG32(Base->CHOPOLCR, (uint32)PWM_CHOPOLCR_CH0POL_Msk << ((uint8)Channel), ((uint8)Channel), Polarity);
}

LOCAL_INLINE uint32 Pwm_Reg_GetAllChannelLevel(const PWM_Type *Base)
{
    uint32 Levels = READ_BIT32(Base->STR, PWM_STR_CH_INPUT_Msk);
    Levels = Levels >> PWM_STR_CH0STS_Pos;

    return Levels;
}

LOCAL_INLINE uint32 Pwm_Reg_GetOverflowFlag(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32(Base->INIT, PWM_INIT_CNTOF_Msk);
    Flag = Flag >> PWM_INIT_CNTOF_Pos;

    return Flag;
}

LOCAL_INLINE void Pwm_Reg_ClearOverflowFlag(PWM_Type *Base)
{
#if defined (AC7843X)
    uint32 Value = READ_REG32(Base->INIT);
    WRITE_REG32(Base->INIT, Value | PWM_INIT_CNTOF_Msk | PWM_INIT_OFDIR_Msk);
#else
    CLEAR_BIT32(Base->INIT, PWM_INIT_CNTOF_Msk);
#endif
}

LOCAL_INLINE uint32 Pwm_Reg_GetOverflowDir(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32(Base->INIT, PWM_INIT_OFDIR_Msk);
    Flag = Flag >> PWM_INIT_OFDIR_Pos;

    return Flag;
}

LOCAL_INLINE uint32 Pwm_Reg_GetChannelEventFlag(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32(Base->STR, PWM_STR_CH_EVENT_Msk);

    return Flag;
}

LOCAL_INLINE void Pwm_Reg_ClearChannelEventFlag(PWM_Type *Base, uint32 mask)
{
#if defined (AC7843X)
    /* All status bits of this register are cleared by writing 1 */
    WRITE_REG32(Base->STR, mask);
#else
    CLEAR_BIT32(Base->STR, mask);
#endif
}

LOCAL_INLINE uint32 Pwm_Reg_GetChannelInterruptFlag(const PWM_Type *Base, Pwm_Hal_ChannelType Channel)
{
    uint32 Flag = READ_BIT32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_CHIF_Msk);
    Flag = Flag >> PWM_CH0SCR_CHIF_Pos;

    return Flag;
}

LOCAL_INLINE void Pwm_Reg_ClearChannelInterruptFlag(PWM_Type *Base, Pwm_Hal_ChannelType Channel)
{
#if defined (AC7843X)
    SET_BIT32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_CHIF_Msk);
#else
    CLEAR_BIT32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_CHIF_Msk);
#endif
}

LOCAL_INLINE void Pwm_Reg_EnableChannelSoftControl
(
    PWM_Type *Base,
    Pwm_Hal_ChannelType Channel,
    boolean State
)
{
    MODIFY_REG32(Base->CHOSWCR, ((uint32)PWM_CHOSWCR_CH0SWEN_Msk << (uint32)Channel), (uint32)Channel, (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetSoftControlEnableStatesMask
(
    PWM_Type *Base,
    uint8 Mask
)
{
    MODIFY_REG32(Base->CHOSWCR, PWM_CHOSWCR_CHNSWEN_Msk, PWM_CHOSWCR_CH0SWEN_Pos, Mask);
}

LOCAL_INLINE uint32 Pwm_Reg_ReadChannelSoftControlState
(
    const PWM_Type *Base
)
{
    uint32 Flag = READ_BIT32(Base->CHOSWCR, PWM_CHOSWCR_CHNSWEN_Msk);

    return Flag;
}

LOCAL_INLINE void Pwm_Reg_SetChannelSoftControlLevel
(
    PWM_Type *Base,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_OutputLevelType Level
)
{
    MODIFY_REG32(Base->CHOSWCR, \
                 ((uint32)PWM_CHOSWCR_CH0SWCV_Msk << (uint32)Channel), \
                 (PWM_CHOSWCR_CH0SWCV_Pos + (uint32)Channel), \
                 (uint32)Level);
}

LOCAL_INLINE void Pwm_Reg_SetSoftControlLevelMask(PWM_Type *Base, uint8 Mask)
{
    MODIFY_REG32(Base->CHOSWCR, PWM_CHOSWCR_CHNSWCV_Msk, PWM_CHOSWCR_CH0SWCV_Pos, Mask);
}

LOCAL_INLINE void Pwm_Reg_EnableSync(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_PWMSYNCEN_Msk, PWM_FUNCSEL_PWMSYNCEN_Pos, (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetSyncType(PWM_Type *Base, uint8 Type)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_PWMSYNC_Msk, PWM_FUNCSEL_PWMSYNC_Pos, Type);
}

LOCAL_INLINE void Pwm_Reg_SetSyncMode(PWM_Type *Base, Pwm_Hal_SyncModeType Mode)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SYNCMODE_Msk, PWM_SYNCONF_SYNCMODE_Pos, Mode);
}

LOCAL_INLINE void Pwm_Reg_EnableSyncBYP(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_PWM_SYNC_BYPEN_Msk, PWM_SYNC_PWM_SYNC_BYPEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableSyncHwTriggerSrc(PWM_Type *Base, uint8 HwTrigNum, boolean Enable)
{
    DEVICE_ASSERT(3U > HwTrigNum);
    MODIFY_REG32(Base->SYNC, (uint32)PWM_SYNC_TRIG0_Msk << HwTrigNum, PWM_SYNC_TRIG0_Pos + HwTrigNum, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_TrigSoftwareSync(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_SWSYNC_Msk, PWM_SYNC_SWSYNC_Pos, (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableMaxLoadingPointSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_MAXSYNCP_Msk, PWM_SYNC_MAXSYNCP_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableMinLoadingPointSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_MINSYNCP_Msk, PWM_SYNC_MINSYNCP_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnablePOLSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_PWM_SYNCPOL_Msk, PWM_SYNC_PWM_SYNCPOL_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableOutputMaskSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_OMSYNCP_Msk, PWM_SYNC_OMSYNCP_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_ChooseBufferOrRegisterToRead(PWM_Type *Base, boolean ChooseReg)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_PWM_BFRGCHSEN_Msk, PWM_SYNC_PWM_BFRGCHSEN_Pos, (FALSE != ChooseReg) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_DisableHwTriggerSyncAfterTrigged(PWM_Type *Base, boolean Disable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_HWTRIGMODE_Msk, PWM_SYNCONF_HWTRIGMODE_Pos, (TRUE == Disable) ? 0U : 1U);
}

LOCAL_INLINE void Pwm_Reg_EnableCNTINSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_CNTINC_Msk, PWM_SYNCONF_CNTINC_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableINVCRSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_INVC_Msk, PWM_SYNCONF_INVC_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableCHOSWCRSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SWOC_Msk, PWM_SYNCONF_SWOC_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetPOLTriggerMode(PWM_Type *Base, Pwm_Hal_SyncTriggerMethodType Mode, boolean Enable)
{
    if (PWM_SYNC_TRIGGER_SOFTWARE == Mode)
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SWPOL_Msk, PWM_SYNCONF_SWPOL_Pos, (FALSE != Enable) ? 1U : 0U);
    }
    else
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_HWPOL_Msk, PWM_SYNCONF_HWPOL_Pos, (FALSE != Enable) ? 1U : 0U);
    }
}

LOCAL_INLINE void Pwm_Reg_SetCHOSWCRSyncTriggerMode(PWM_Type *Base, Pwm_Hal_SyncTriggerMethodType Mode, boolean Enable)
{
    if (PWM_SYNC_TRIGGER_SOFTWARE == Mode)
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SWVSWSYNC_Msk, PWM_SYNCONF_SWVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
    else
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SWVHWSYNC_Msk, PWM_SYNCONF_SWVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
}

LOCAL_INLINE void Pwm_Reg_SetINVCRSyncTriggerMode(PWM_Type *Base, Pwm_Hal_SyncTriggerMethodType Mode, boolean Enable)
{
    if (PWM_SYNC_TRIGGER_SOFTWARE == Mode)
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_INVSWSYNC_Msk, PWM_SYNCONF_INVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
    else
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_INVHWSYNC_Msk, PWM_SYNCONF_INVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
}

LOCAL_INLINE void Pwm_Reg_SetOMCRSyncTriggerMode(PWM_Type *Base, Pwm_Hal_SyncTriggerMethodType Mode, boolean Enable)
{
    if (PWM_SYNC_TRIGGER_SOFTWARE == Mode)
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_OMVSWSYNC_Msk, PWM_SYNCONF_OMVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
    else
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_OMVHWSYNC_Msk, PWM_SYNCONF_OMVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
}

LOCAL_INLINE void Pwm_Reg_SetMCVRSyncMode(PWM_Type *Base, Pwm_Hal_SyncTriggerMethodType Mode, boolean Enable)
{
    if (PWM_SYNC_TRIGGER_SOFTWARE == Mode)
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_PWMSVSWSYNC_Msk, PWM_SYNCONF_PWMSVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
    else
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_PWMSVHWSYNC_Msk, PWM_SYNCONF_PWMSVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
}

LOCAL_INLINE void Pwm_Reg_SetCNTSyncTriggerMode(PWM_Type *Base, Pwm_Hal_SyncTriggerMethodType Mode, boolean Enable)
{
    if (PWM_SYNC_TRIGGER_SOFTWARE == Mode)
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_CNTVSWSYNC_Msk, PWM_SYNCONF_CNTVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
    else
    {
        MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_CNTVHWSYNC_Msk, PWM_SYNCONF_CNTVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
    }
}

LOCAL_INLINE void Pwm_Reg_EnableGlobalTimeBase(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->CONF, PWM_CONF_GTBEEN_Msk, PWM_CONF_GTBEEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableGlobalTimeBaseOutput(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->CONF, PWM_CONF_GTBEOUT_Msk, PWM_CONF_GTBEOUT_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetInputFilterPsc(PWM_Type *Base, Pwm_Hal_InputFilterPscType Psc)
{
    MODIFY_REG32(Base->CAPFILTER, PWM_CAPFILTER_CAPFPSC_Msk, PWM_CAPFILTER_CAPFPSC_Pos, (uint32)Psc);
}

LOCAL_INLINE void Pwm_Reg_SetChannelInputFilterVal(PWM_Type *Base, Pwm_Hal_ChannelType Channel, uint8 Value)
{
    DEVICE_ASSERT(PWM_CHANNEL_4 > Channel);
    MODIFY_REG32(Base->CAPFILTER, (uint32)PWM_CAPFILTER_CH0CAPFVAL_Msk << (((uint8)Channel) * PWM_CAPFILTER_WIDTH), \
                 (((uint8)Channel) * PWM_CAPFILTER_WIDTH), Value);
}

LOCAL_INLINE void Pwm_Reg_SetCaptureEventPsc
(
    PWM_Type *Base,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_InputEventPscType EventPsc
)
{
    MODIFY_REG32(Base->CONF, \
                 (PWM_CONF_EVENT0_PSC_Msk << ((uint8)Channel * PWM_CONF_EVENTPSC_WIDTH)), \
                 (PWM_CONF_EVENT0_PSC_Pos + ((uint8)Channel * PWM_CONF_EVENTPSC_WIDTH)), \
                 (uint32)EventPsc);
}

LOCAL_INLINE void Pwm_Reg_EnableChannelEventReset
(
    PWM_Type *Base,
    Pwm_Hal_ChannelType Channel,
    boolean State
)
{
    MODIFY_REG32(Base->CHANNELS[(uint32)Channel].CHnSCR, \
                 PWM_CH0SCR_CHRSTEN_Msk, \
                 PWM_CH0SCR_CHRSTEN_Pos, \
                 (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnablePairChDualEdgeCapture
(
    PWM_Type *Base,
    Pwm_Hal_ChannelPairType ChannelPair,
    boolean State
)
{
    MODIFY_REG32(Base->MODESEL, \
                 ((uint32)PWM_MODESEL_PAIR0DECAPEN_Msk << ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (PWM_MODESEL_PAIR0DECAPEN_Pos + ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetPairChDualEdgeCapture
(
    PWM_Type *Base,
    Pwm_Hal_ChannelPairType ChannelPair,
    boolean State
)
{
    MODIFY_REG32(Base->MODESEL, \
                 ((uint32)PWM_MODESEL_PAIR0DECAP_Msk << ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (PWM_MODESEL_PAIR0DECAP_Pos + ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (FALSE != State) ? 1U : 0U);
}

LOCAL_INLINE boolean Pwm_Reg_IsPairChDualEdgeCaptureEnabled
(
    const PWM_Type *Base,
    Pwm_Hal_ChannelPairType ChannelPair
)
{
    uint32 Flag = READ_BIT32(Base->MODESEL, \
                             ((uint32)PWM_MODESEL_PAIR0DECAPEN_Msk << ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)));
    Flag = Flag >> (PWM_MODESEL_PAIR0DECAPEN_Pos + ((uint8)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH));

    return (0U != Flag) ? TRUE : FALSE;
}

LOCAL_INLINE void Pwm_Reg_SetFaultMode(PWM_Type *Base, Pwm_Hal_FaultCtrlModeType Mode)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_FAULTMODE_Msk, PWM_FUNCSEL_FAULTMODE_Pos, Mode);
}

LOCAL_INLINE void Pwm_Reg_EnableFaultInputFilter(PWM_Type *Base, Pwm_Hal_FaultInputIdType FaultInputId, boolean Enable)
{
    MODIFY_REG32(Base->FFAFER, (uint32)PWM_FFAFER_FF0EN_Msk << ((uint8)FaultInputId),
                 PWM_FFAFER_FF0EN_Pos + ((uint8)FaultInputId), (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetFaultInputFilterValue(PWM_Type *Base, uint8 Value)
{
    MODIFY_REG32(Base->FFAFER, PWM_FFAFER_FFVAL_Msk, PWM_FFAFER_FFVAL_Pos, Value);
}

LOCAL_INLINE void Pwm_Reg_EnableFaultPinInput(PWM_Type *Base, Pwm_Hal_FaultInputIdType FaultInputId, boolean Enable)
{
    MODIFY_REG32(Base->FFAFER, (uint32)PWM_FFAFER_FER0EN_Msk << ((uint8)FaultInputId), ((uint8)FaultInputId),
                 (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_SetFaultInputPolarity(PWM_Type *Base, Pwm_Hal_FaultInputIdType FaultInputId,
        Pwm_Hal_ActivePolarityType Polarity)
{
    MODIFY_REG32(Base->FLTPOL, (uint32)PWM_FLTPOL_FLT0POL_Msk << ((uint8)FaultInputId), ((uint8)FaultInputId), Polarity);
}

LOCAL_INLINE void Pwm_Reg_EnableFaultInterrupt(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_FAULTIE_Msk, PWM_FUNCSEL_FAULTIE_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableFaultHizOutput(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_FAULTHIZEN_Msk, PWM_FUNCSEL_FAULTHIZEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableChannelHizOutput(PWM_Type *Base, Pwm_Hal_ChannelType Channel, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_CH0HIZEN_Msk << ((uint8)Channel),
                 PWM_FUNCSEL_CH0HIZEN_Pos + ((uint8)Channel), (FALSE != Enable) ? 1U : 0U);
}

#if defined (AC7843X) || defined (AC7842X)
LOCAL_INLINE void Pwm_Reg_RecoverZeroOrFullDutyAfterFaultCleared(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_FAULTCTRLEN_Msk, PWM_FUNCSEL_FAULTCTRLEN_Pos, (FALSE != Enable) ? 1U : 0U);
}
#endif

LOCAL_INLINE void Pwm_Reg_EnablePairChFaultCtrl(PWM_Type *Base, Pwm_Hal_ChannelPairType ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, (uint32)PWM_MODESEL_PAIR0FAULTEN_Msk << ((uint32)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH),
                 PWM_MODESEL_PAIR0FAULTEN_Pos + ((uint32)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE uint32 Pwm_Reg_GetFaultPinFlag(const PWM_Type *Base, Pwm_Hal_FaultInputIdType FaultInputId)
{
    uint32 Flag = READ_BIT32((Base)->FDSR, ((uint32)PWM_FDSR_FAULTDF0_Msk << (uint32)FaultInputId));
    Flag = Flag >> (PWM_FDSR_FAULTDF0_Pos + (uint32)FaultInputId);

    return Flag;
}

LOCAL_INLINE void Pwm_Reg_ClearFaultPinFlag(PWM_Type *Base, Pwm_Hal_FaultInputIdType FaultInputId)
{
    DEVICE_ASSERT(PWM_FAULT_INPUT_MAX > FaultInputId);

#if defined (AC7843X)
    MODIFY_REG32(Base->FDSR, (PWM_FDSR_FAULTDF_Msk | PWM_FDSR_WPEN_Msk | PWM_FDSR_FAULTDF3_Msk | \
                              PWM_FDSR_FAULTDF2_Msk | PWM_FDSR_FAULTDF1_Msk | PWM_FDSR_FAULTDF0_Msk), \
                 (uint32)FaultInputId, 1UL);
#else
    CLEAR_BIT32((Base)->FDSR, ((uint32)PWM_FDSR_FAULTDF0_Msk << (uint8)FaultInputId));
#endif
}

LOCAL_INLINE uint32 Pwm_Reg_GetFaultFlag(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32((Base)->FDSR, PWM_FDSR_FAULTDF_Msk);
    Flag = Flag >> (PWM_FDSR_FAULTDF_Pos);

    return Flag;
}

LOCAL_INLINE void Pwm_Reg_ClearFaultFlag(PWM_Type *Base)
{
#if defined (AC7843X)
    MODIFY_REG32(Base->FDSR, (PWM_FDSR_FAULTDF_Msk | PWM_FDSR_WPEN_Msk | PWM_FDSR_FAULTDF3_Msk | \
                              PWM_FDSR_FAULTDF2_Msk | PWM_FDSR_FAULTDF1_Msk | PWM_FDSR_FAULTDF0_Msk), \
                 PWM_FDSR_FAULTDF_Pos, 1UL);
#else
    CLEAR_BIT32((Base)->FDSR, PWM_FDSR_FAULTDF_Msk);
#endif
}

LOCAL_INLINE uint32 Pwm_Reg_GetFaultInputStatus(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32((Base)->FDSR, PWM_FDSR_FAULTIN_Msk);
    Flag = Flag >> (PWM_FDSR_FAULTIN_Pos);

    return Flag;
}

LOCAL_INLINE void Pwm_Reg_SetQuadEncodeMode(PWM_Type *Base, Pwm_Hal_QuadModeType Mode)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_QUADMODE_Msk, PWM_QDI_QUADMODE_Pos, Mode);
}

LOCAL_INLINE void Pwm_Reg_SetQuadPhaseAPolarity(PWM_Type *Base, Pwm_Hal_QuadPhasePolarityType Polarity)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHAPOL_Msk, PWM_QDI_PHAPOL_Pos, Polarity);
}

LOCAL_INLINE void Pwm_Reg_SetQuadPhaseBPolarity(PWM_Type *Base, Pwm_Hal_QuadPhasePolarityType Polarity)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHBPOL_Msk, PWM_QDI_PHBPOL_Pos, Polarity);
}

LOCAL_INLINE void Pwm_Reg_SetQuadPhaseZPolarity(PWM_Type *Base, Pwm_Hal_QuadPhasePolarityType Polarity)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHZPOL_Msk, PWM_QDI_PHZPOL_Pos, Polarity);
}

LOCAL_INLINE void Pwm_Reg_EnableQuadPhaseZResetCnt(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHZRSTEN_Msk, PWM_QDI_PHZRSTEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE void Pwm_Reg_EnableQuadDecoder(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_QDIEN_Msk, PWM_QDI_QDIEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

LOCAL_INLINE uint32 Pwm_Reg_GetQuadCountDir(const PWM_Type *Base)
{
    uint32 DirFlag = READ_BIT32((Base)->QDI, PWM_QDI_QUADIR_Msk);
    DirFlag = DirFlag >> (PWM_QDI_QUADIR_Pos);

    return DirFlag;
}

LOCAL_INLINE uint32 Pwm_Reg_GetQuadOverflowDir(const PWM_Type *Base)
{
    uint32 DirFlag = READ_BIT32((Base)->QDI, PWM_QDI_CNTOFDIR_Msk);
    DirFlag = DirFlag >> (PWM_QDI_CNTOFDIR_Pos);

    return DirFlag;
}

LOCAL_INLINE uint32 Pwm_Reg_GetQuadPhaseZFlag(const PWM_Type *Base)
{
    uint32 Status = READ_BIT32((Base)->QDI, PWM_QDI_PHZSTS_Msk);
    Status = Status >> (PWM_QDI_PHZSTS_Pos);

    return Status;
}

LOCAL_INLINE void Pwm_Reg_ClearQuadPhaseZFlag(PWM_Type *Base)
{
#if defined (AC7843X)
    SET_BIT32((Base)->QDI, PWM_QDI_PHZSTS_Msk);
#else
    CLEAR_BIT32((Base)->QDI, PWM_QDI_PHZSTS_Msk);
#endif
}

#if defined (AC7843X)
LOCAL_INLINE void Pwm_Reg_SetQuadPhaseZResetMode(PWM_Type *Base, Pwm_Hal_QuadPhaseZResetMode Mode)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHZRSTMODE_Msk, PWM_QDI_PHZRSTMODE_Pos, Mode);
}

LOCAL_INLINE void Pwm_Reg_EnableQuadPhaseZInterrupt(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_DETECTIRQEN_Msk, PWM_QDI_DETECTIRQEN_Pos, (FALSE != Enable) ? 1U : 0U);
}
#endif

LOCAL_INLINE void Pwm_Reg_EnableWriteProtection(PWM_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
#if defined (AC7843X)
        MODIFY_REG32(Base->FDSR, (PWM_FDSR_FAULTDF_Msk | PWM_FDSR_WPEN_Msk | PWM_FDSR_FAULTDF3_Msk | \
                                  PWM_FDSR_FAULTDF2_Msk | PWM_FDSR_FAULTDF1_Msk | PWM_FDSR_FAULTDF0_Msk), \
                     PWM_FDSR_WPEN_Pos, 1UL);
#else
        MODIFY_REG32(Base->FDSR, PWM_FDSR_WPEN_Msk, PWM_FDSR_WPEN_Pos, 1U);
#endif
    }
    else
    {
        MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_WPDIS_Msk, PWM_FUNCSEL_WPDIS_Pos, 1U);
    }
}

LOCAL_INLINE void Pwm_Reg_SetDebugMode(PWM_Type *Base, Pwm_Hal_DebugModeType Mode)
{
    MODIFY_REG32(Base->DITHER2, PWM_DITHER2_DEBUGCTRL_Msk, PWM_DITHER2_DEBUGCTRL_Pos, Mode);
}

#endif /* AC784XX_PWM_REG_H */
