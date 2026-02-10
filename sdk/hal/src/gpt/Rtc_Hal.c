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
 * @file Rtc_Hal.c
 *
 * @brief rtc hal source file.
 */
/*==============================================INCLUDE FILES=======================================*/
#include "Rtc_Hal.h"
#include "Ckgen_Hal.h"
#include "Core_Hal.h"
#include "AC784xx_Rtc_Reg.h"
#include "OsIf_Time.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/** @brief  The value(none zero) in order to clear TIF flag*/
#define CLEAR_TIF_VALUE   (0x01U)

#if defined (AC7840X)
/** @brief  Value setting for SPM LSI32K clock.*/
#define RTC_CTRL_CLK_STB_LSI32K 0x10000UL

/** @brief  Value setting for SPM RTC_CLKIN clock.*/
#define RTC_CTRL_CLK_STB_CLKIN  0x20000UL
#endif

/*===================================================ENUMS==========================================*/

/*!
 * @brief RTC state type
 */
typedef enum
{
    RTC_UNINITED = 0x0U, /**< Un initialization status. */
    RTC_IDLE, /**< RTC has been initialized and is idle */
    RTC_PAUSED, /**< RTC is paused.*/
    RTC_RUNNING, /**< RTC is running.*/
} Rtc_StateType;
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/** @brief Variable for pre-set counter.*/
static uint32 TcValue = 1U;

/** @brief Variable for store alarm value.*/
static uint32 AlarmTime = 0U;

/** @brief Variable for store pointer to interrupt callback function.*/
static Rtc_Hal_CallbackType RtcIsrCallback = NULL_PTR;
/** @brief Variable of RTC state.*/
static Rtc_StateType RtcStatus = RTC_UNINITED;

/** @brief  RTC interrupt enable flags(bits).*/
static uint32 RtcInterruptBits = RTC_INT_PRESCALER_EN;

/** @brief RTC output configuration..*/
static Rtc_ClockOutputType RtcOutputCfg = RTC_OUTPUT_DISABLED;
/*=============================FUNCTION PROTOTYPES==================================*/
/**
* @brief Clear RTC interrupt status register.
* @note Function ID: DES_GPT_API_621
* @param [in] Flags: Status bits of RTC interupt.
* @return void
*/
static void Rtc_Hal_ClearIntFlags(uint32 Flags);

/**
* @brief Set output register base on output config setting..
* @note Function ID: DES_GPT_API_622
* @param [in] OutputCfg: Config bits of RTC output.
* @return void
*/
static void Rtc_Hal_ConfigOutputReg(Rtc_ClockOutputType OutputCfg);

/**
* @brief Set interrupt  hardware base on interrupt configuration.
* @note Function ID: DES_GPT_API_623
* @param [in] InterruptBits: Configuration bits of RTC interrupt.
* @return void
*/
static void Rtc_Hal_ConfigInterrupt(uint32 InterruptBits);

/*=========================GLOBAL FUNCTION  IMPLEMENTATIONS==========================*/
Hal_StatusType Rtc_Hal_Init(Rtc_ClockSourceType Clk)
{
    Hal_StatusType Res;/* function return value */

    if (RTC_UNINITED == RtcStatus)/* rtc is RTC_UNINITED, can initialize rtc  */
    {
        Res = Ckgen_Hal_EnablePeriphClk(CKGEN_RTC_BUS_CLK, TRUE);
        /* Clock source is valid. */
        if (RTC_CLOCK_INVALID != Clk)
        {
            uint32 Ctrl = Rtc_Reg_ReadCTRL();
            Ctrl &= ~(RTC_CTRL_CLKSEL_Msk); /* Clear CLKSEL bits*/
#if defined (AC7840X)
            switch (Clk)
            {
            case RTC_CLOCK_STB_LSI32K:
                Ctrl |= RTC_CTRL_CLK_STB_LSI32K;
                break;
            case RTC_CLOCK_STB_CLKIN:
                Ctrl |= RTC_CTRL_CLK_STB_CLKIN;
                break;
            default:
                /* Not need operation.*/
                break;/*RTC clock source is from ckgen.*/
            }
#elif defined (AC7842X) || defined (AC7843X)
            Rtc_Reg_Reset(); // Ensure RTC is stopped before setting RTC Clock.
            /*Set CLKSEL base on clock source.*/
            Ctrl |= (uint32)Clk << 16U;
#endif
            Rtc_Reg_WriteCTRL(Ctrl); /* AC7840x Bit 16 & 17 can only be written once. Set it before SW reseet!!!*/
        }
        /* enable rtc bus clock success */
        if (STATUS_SUCCESS == Res)
        {
            AlarmTime = 0U;
            RtcStatus = RTC_IDLE;
        }
    }
    else
    {
        Res = STATUS_ERROR;
    }

    return Res;
}

Hal_StatusType Rtc_Hal_DeInit(void)
{
    Hal_StatusType Res = STATUS_ERROR;
    /* RTC is IDLE*/
    if (RTC_IDLE == RtcStatus)
    {
        (void)Ckgen_Hal_EnablePeriphClk(CKGEN_RTC_BUS_CLK, FALSE);
        RtcStatus = RTC_UNINITED;
        Res = STATUS_SUCCESS;
    }
    return Res;
}

Hal_StatusType Rtc_Hal_Start(uint32 Unit)
{
    Hal_StatusType Res = STATUS_SUCCESS;/* function return value */
    /* RtcStatus is RTC_STOPPED or RTC_CONFIGURED, can enable rtc*/
    if (RTC_IDLE == RtcStatus)
    {
        Rtc_Reg_Reset();
        uint32 Value = Rtc_Reg_ReadSR();
        Value |= RTC_SR_TAF_Msk; // Force to write Alarm register in Rtc_Hal_ClearIntFlags.
        Value |= RTC_SR_TOF_Msk; // Force to write TC register in Rtc_Hal_ClearIntFlags.
        Rtc_Hal_ClearIntFlags(Value);
        Rtc_Reg_WritePSR(Unit);
        Rtc_Hal_ConfigOutputReg(RtcOutputCfg);
        Rtc_Hal_ConfigInterrupt(RtcInterruptBits);
        Rtc_Reg_Enable(TRUE);
        RtcStatus = RTC_RUNNING;
    }
    else if (RTC_RUNNING == RtcStatus)/* RtcStatus is RTC_RUNNING, do nothing and return success*/
    {
        Res = STATUS_SUCCESS;
    }
    else if (RTC_PAUSED == RtcStatus)/* RtcStatus is RTC_PAUSED, restart.*/
    {
        Rtc_Reg_Enable(TRUE);
        RtcStatus = RTC_RUNNING;
        Res = STATUS_SUCCESS;
    }
    else
    {
        Res = STATUS_ERROR;
    }

    return Res;
}

Hal_StatusType Rtc_Hal_Stop(void)
{
    Hal_StatusType Res = STATUS_SUCCESS;/* function return value */
    /* RtcStatus is RTC_RUNNING , can disable rtc*/
    if ((RTC_RUNNING == RtcStatus) || (RTC_PAUSED == RtcStatus))
    {
        Rtc_Reg_Enable(FALSE);
        Rtc_Hal_ConfigInterrupt(0U); /* Disable RTC all inerrupts.*/
        RtcStatus = RTC_IDLE;
        AlarmTime = 0U; /* Reset alarm time.*/
        TcValue = 0U;
        Rtc_Reg_Reset(); /* Reset hardware register to default value(For SRS). */
    }
    else if (RTC_IDLE == RtcStatus)/* RtcStatus is RTC_IDLE, do nothing and return success*/
    {
        Res = STATUS_SUCCESS;
    }
    else
    {
        Res = STATUS_ERROR;
    }
    return Res;
}

Hal_StatusType Rtc_Hal_Pause(void)
{
    Hal_StatusType Res = STATUS_SUCCESS;/* function return value */
    /* RtcStatus is RTC_RUNNING , can disable rtc*/
    if (RTC_RUNNING == RtcStatus)
    {
        Rtc_Reg_Enable(FALSE);
        RtcStatus = RTC_PAUSED;
    }
    else
    {
        Res = STATUS_ERROR;
    }
    return Res;
}

uint32 Rtc_Hal_GetCurrentValue(void)
{
    uint32 Value = 0;
    /* RtcStatus not equal to RTC_UNINITED, can get value */
    if (RTC_UNINITED != RtcStatus)
    {
        Value = Rtc_Reg_ReadTC();
    }
    return Value;
}

void Rtc_Hal_SetCurrentValue(uint32 Value)
{
    TcValue = Value;
    if (RTC_PAUSED == RtcStatus)
    {
        Rtc_Reg_WriteTC(TcValue);
    }
}

Hal_StatusType Rtc_Hal_SetAlarm(uint32 Value)
{
    AlarmTime = Value; /* Save for next RTC start.*/

    /* RTC is running.*/
    if (RTC_RUNNING == RtcStatus)
    {
        /* Tips: RTC HW needs 2 clocks time(Max:62.5 us in LSI32K)  to clear status. */
        /* Avoid  setting alarm register before HW finishs clearing  IRQ status.*/
        OsIf_UDelay(100);
        Rtc_Reg_WriteAlarm(Value);
    }
    return STATUS_SUCCESS;
}

void Rtc_Hal_InstallCallback(const Rtc_Hal_CallbackType Func)
{
    RtcIsrCallback = Func;
}

void Rtc_Hal_EnableInterrupt(uint32 InterruptBits)
{
    /* RTC interrupt config is different from previous config.*/
    if ((InterruptBits & RTC_INT_ALL_EN) != RtcInterruptBits)
    {
        /* Apply interrupt setting if RTC is running.*/
        if (RTC_RUNNING == RtcStatus)
        {
            uint32 Value = Rtc_Reg_ReadSR();
            Rtc_Hal_ClearIntFlags(Value);
            Rtc_Hal_ConfigInterrupt(InterruptBits);
        }
        RtcInterruptBits = InterruptBits & RTC_INT_ALL_EN;
    }
}

Hal_StatusType Rtc_Hal_SetConfig(Rtc_ClockOutputType OutputCfg)
{
    /* Output setting is different from previous config.*/
    if (RtcOutputCfg != OutputCfg)
    {
        /* Change output setting if RTC is running.*/
        if (RTC_RUNNING == RtcStatus)
        {
            Rtc_Hal_ConfigOutputReg(OutputCfg);
        }
        RtcOutputCfg = OutputCfg;
    }
    return STATUS_SUCCESS;
}

uint32 Rtc_Hal_GetWorkFreq(void)
{
    uint32 Freq = 0U;
    /* RTC has been initialized.*/
    if (RTC_UNINITED != RtcStatus)
    {
#if defined (AC7840X)
        const Ckgen_ClkIdType ClkReg2CkgenId[4U] = {CKGEN_RTC_CLK, CKGEN_LSI_32K_CLK,
                                    CKGEN_RTC_CLKIN, CKGEN_RTC_CLK
                                   };
#elif defined (AC7842X) || defined (AC7843X)
        const Ckgen_ClkIdType ClkReg2CkgenId[4U] = {CKGEN_HSE_DIV1_CLK, CKGEN_VHSI_DIV1_CLK,
                                    CKGEN_LSI_32K_CLK, CKGEN_RTC_CLKIN
                                   };
#endif
        uint32 RtcClk = (Rtc_Reg_ReadCTRL() & RTC_CTRL_CLKSEL_Msk) >> 16U;
        RtcClk =  (uint32) ClkReg2CkgenId[RtcClk];
        /*PRQA S 4342 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        (void)Ckgen_Hal_GetFreq((Ckgen_ClkIdType)RtcClk, &Freq);
        /*PRQA S 4342-- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    }

    return Freq;
}

uint32 Rtc_Hal_GetPrescalerValue(void)
{
    uint32 Value = 0;
    /* RtcStatus not equal to RTC_UNINITED, can get value */
    if (RTC_UNINITED != RtcStatus)
    {
        Value = Rtc_Reg_ReadPSC();
    }
    return Value;
}

void RTC_IRQHandler(void)
{
    uint32 IrqStatus = Rtc_Reg_ReadSR();
    uint32 Event = 0U;

    /* RTC_SR_POF_Msk is or RTC_SR_TIF_Msk isn't set. */
    if ((0U != (IrqStatus & RTC_SR_POF_Msk)) || (0U != (IrqStatus & RTC_SR_TIF_Msk)))
    {
        Event = RtcInterruptBits & RTC_INT_PRESCALER_EN;
    }
    if (0U != (IrqStatus & (RTC_SR_TOF_Msk|RTC_SR_TIF_Msk)))
    {
        Event |= RtcInterruptBits & (RTC_INT_TIMER_EN | RTC_INT_INVALID_EN);
        Rtc_Reg_Enable(FALSE); /* Stop RTC. (For clear IRQ status) */
    }
    if (0U != (IrqStatus & RTC_SR_TAF_Msk))
    {
        /* if an alarm is enable, the status is assigned */
        Event |= RtcInterruptBits & RTC_INT_ALARM_EN;
    }
    Rtc_Hal_ClearIntFlags(IrqStatus|RTC_SR_POF_Msk); /* Clear all interupt status flags.*/

    /* Time interrupt status flag is set */
    if (0U != (IrqStatus & (RTC_SR_TOF_Msk|RTC_SR_TIF_Msk)))
    {
        Rtc_Hal_ClearIntFlags(RTC_SR_POF_Msk);/* Deleting will result in the inability to generate prescaler IRQ.*/
        Rtc_Reg_Enable(TRUE); /* Re-start RTC */
    }
    if ((NULL_PTR != RtcIsrCallback) && (0U != Event))
    {
        RtcIsrCallback(Event);
    }

}

/*========================STATIC  FUNCTION  IMPLEMENTATIONS========================*/
static void Rtc_Hal_ClearIntFlags(uint32 Flags)
{
    /* Neet to clear alarm interrupt status.*/
    if (0U != (RTC_SR_TAF_Msk & Flags))
    {
        /* Write Alarm register to clear alarm interrupt status.*/
        Rtc_Reg_WriteAlarm(AlarmTime);
    }
    /* Neet to clear TOF and TIF*/
    if ((0U != (RTC_SR_TOF_Msk & Flags)) || (0U != (RTC_SR_TIF_Msk & Flags)))
    {
        if (0U == TcValue)
        {
            Rtc_Reg_WriteTC(CLEAR_TIF_VALUE); /* Write non-zero value to clear RTC timer interrupt status and TIF.*/
        }
        Rtc_Reg_WriteTC(TcValue); /* Re-set TC */
    }
    /* Neet to clear POF*/
    if (0U != (RTC_SR_POF_Msk & Flags))
    {
        uint32 Value = Rtc_Reg_ReadSR() | RTC_SR_POF_Msk;
        Rtc_Reg_WriteSR(Value); /* Clear prescaler interrupt status. */
    }
}

static void Rtc_Hal_ConfigOutputReg(Rtc_ClockOutputType OutputCfg)
{
    uint32 Value = Rtc_Reg_ReadCTRL();
    /* RTC output is enabled.*/
    if (RTC_OUTPUT_DISABLED != OutputCfg)
    {
        Value |= RTC_CTRL_COE_Msk;

        /* RTC output is RTC clock.*/
        if (RTC_OUTPUT_CLOCK == OutputCfg)
        {
            Value |= RTC_CTRL_COS_Msk;
        }
    }
    else /* RTC output is disabled.*/
    {
        Value &= ~RTC_CTRL_COE_Msk;
    }
    Rtc_Reg_WriteCTRL(Value);
}

static void Rtc_Hal_ConfigInterrupt(uint32 InterruptBits)
{
    uint32 Value;

    Value = Rtc_Reg_ReadCTRL();
    /* Clear All interupt bits*/
    Value &= ~(RTC_CTRL_POIE_Msk | RTC_CTRL_TOIE_Msk | RTC_CTRL_TAIE_Msk | RTC_CTRL_TIIE_Msk);

    /* Need to restart RTC in ISR.*/
    Value |= RTC_CTRL_TOIE_Msk;
    /* Alarm interrupt is enabled.*/
    if (RTC_INT_ALARM_EN == (InterruptBits & RTC_INT_ALARM_EN))
    {
        Value |= RTC_CTRL_TAIE_Msk;
    }
    /* Prescaler overflow interrupt is enabled.*/
    if (RTC_INT_PRESCALER_EN == (InterruptBits & RTC_INT_PRESCALER_EN))
    {
        Value |= RTC_CTRL_POIE_Msk;
    }
    /* Invalid interrupt is enabled.*/
    if (RTC_INT_INVALID_EN == (InterruptBits & RTC_INT_INVALID_EN))
    {
        Value |= RTC_CTRL_TIIE_Msk;
    }
    /* if one of the irq is set to the enable , the RTC irq needs to be enabled*/
    if (0U != InterruptBits)
    {
        Core_Hal_ClearPendingIrq(RTC_IRQn);
        Core_Hal_EnableIrq(RTC_IRQn);
    }
    else
    {
        Core_Hal_DisableIrq(RTC_IRQn);
        Core_Hal_ClearPendingIrq(RTC_IRQn);
    }
    Rtc_Reg_WriteCTRL(Value);
}

/* =============================================  EOF  ============================================== */
