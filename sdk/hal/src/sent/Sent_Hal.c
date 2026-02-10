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
 * AutoChips Inc. (C) 2024. All rights reserved.
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
 * @file Sent_Hal.c
 *
 * @brief This file provides sent integration functions.
 *
 */

/*PRQA S ALL --*/
/*PRQA S 2812 ++ */ /* Apparent: Dereference of NULL pointer. */
/*PRQA S 1503 ++ */ /* The function '%1s' is defined but is not used within this project. */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Sent_Reg.h"
#include "Sent_Hal.h"

#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
/* SENT IRQ status */
#define IRQ_RSI                 (0x1UL)
#define IRQ_RDI                 (0x2UL)
#define IRQ_RFI                 (0x4UL)
#define IRQ_TDI                 (0x8UL)
#define IRQ_TBI                 (0x10UL)
#define IRQ_FRI                 (0x20UL)
#define IRQ_FDI                 (0x40UL)
#define IRQ_NNI                 (0x80UL)
#define IRQ_NVI                 (0x100UL)
#define IRQ_CRCI                (0x200UL)
#define IRQ_WSI                 (0x400UL)
#define IRQ_SDI                 (0x800UL)
#define IRQ_SCRI                (0x1000UL)
#define IRQ_WDI                 (0x2000UL)
#define IRQ_ESI                 (0x4000UL)
#define IRQ_HFLL                (0x8000UL)
#define IRQ_FDONE               (0x10000UL)

#define IRQ_RECEIVE             (IRQ_RSI | IRQ_RDI | IRQ_SDI | IRQ_HFLL | IRQ_FDONE)
#define IRQ_TRANSMIT            (IRQ_TDI)
#define IRQ_ERROR (IRQ_RFI | IRQ_TBI | IRQ_FRI | IRQ_FDI | IRQ_NNI \
                   | IRQ_NVI | IRQ_CRCI | IRQ_WSI | IRQ_SCRI | IRQ_WDI | IRQ_ESI)

/* ============================================= TYPEDEFS ================================================ */
/*!
 * @brief ATC SENT runtime status.
 */
typedef struct
{
    Sent_MsgType SentMsg; /*!< sent message frame */
    uint32 EnabledInterrupts; /*!< interrupts control */
    Sent_RxCallbackType RxCallback; /*!< sent receive callback function */
    Sent_CallbackType TxCallback; /*!< sent transmit callback user data */
    Sent_CallbackType ErrCallback; /*!< sent channel error callback function */
} Sent_StateType;

/* =========================================== LOCAL VARIABLES ============================================== */
/* Uart state Channel config */
#if (CONFIG_SENT_CHANNEL0_ENABLE)
static Sent_StateType Sent_Channel0State;
#endif
#if (CONFIG_SENT_CHANNEL1_ENABLE)
static Sent_StateType Sent_Channel1State;
#endif

#if defined (AC7843X)
#if (CONFIG_SENT_CHANNEL2_ENABLE)
static Sent_StateType Sent_Channel2State;
#endif
#if (CONFIG_SENT_CHANNEL3_ENABLE)
static Sent_StateType Sent_Channel3State;
#endif
#endif

static Sent_StateType *const Sent_ChannelStateArray[SENT_INSTANCE_MAX] =
{
#if (CONFIG_SENT_CHANNEL0_ENABLE)
    &Sent_Channel0State,
#else
    NULL_PTR,
#endif
#if (CONFIG_SENT_CHANNEL1_ENABLE)
    &Sent_Channel1State,
#else
    NULL_PTR,
#endif

#if defined (AC7843X)
#if (CONFIG_SENT_CHANNEL2_ENABLE)
    &Sent_Channel2State,
#else
    NULL_PTR,
#endif
#if (CONFIG_SENT_CHANNEL3_ENABLE)
    &Sent_Channel3State,
#else
    NULL_PTR,
#endif
#endif
};

/* Table of base addresses for sent instances. */
static SENT_CHANNEL_Type *const SentChannelBase[SENT_INSTANCE_MAX] = SENT_BASE_PTRS;
static SENT_CTRL_Type *const SentModuleBase = SENT_CTRL;

/* Table to save SENT IRQ numbers. */
static const IRQn_Type SentIrqId[SENT_INSTANCE_MAX] = SENT_IRQS;

#ifndef SENT_SDK_NON_EXTENDED_API
/*PRQA S 3218 ++ */ /* File scope static, '%1s', is only accessed in one function. */
/* Default sent channel configuration. */
static const Sent_ChannelConfigType DefaultSentChannelConfig =
{
    .WatchdogTimerLimit = 0,
    .IoControl =
    {
        .InputInvertEnable = FALSE,
        .OutputInvertEnable = FALSE,
        .GlitchFilterDepth = 2U,
        .Input = SENT_INPUT_DATA_SELECT_0,
        .TriggerSource = SENT_EXTERNAL_TRIGGER_0,
    },
    .RxControl =
    {
        .PausePulseEnable = FALSE,
        .AlternateCRCEnable = FALSE,
        .StatusIncludedInCRC = FALSE,
        .SerialDataEnable = FALSE,
        .SerialCRCEnable = FALSE,
        .FastCRCEnable = FALSE,
        .FrameCheckMode = SENT_FRAME_CHECK_PAST_SYNC_PULSE,
        .FrameLength = 6U,
        .ZeroAugumentInCRCDisable = FALSE,
        .SerialMsgType = SENT_SHORT_SERIAL_MSG,
        .DriftErrorDisable = FALSE,
        .FDFL = FALSE,
    },
    .TickUnit = 3E-6F,
    .EnabledInterrupts = 0xffffffffU,
    .NibblePointer =
    {
        .Nibble7Position = SENT_NIBBLE_POSITION_7,
        .Nibble6Position = SENT_NIBBLE_POSITION_6,
        .Nibble5Position = SENT_NIBBLE_POSITION_5,
        .Nibble4Position = SENT_NIBBLE_POSITION_4,
        .Nibble3Position = SENT_NIBBLE_POSITION_3,
        .Nibble2Position = SENT_NIBBLE_POSITION_2,
        .Nibble1Position = SENT_NIBBLE_POSITION_1,
        .Nibble0Position = SENT_NIBBLE_POSITION_0,
    },
    .RxCallback = NULL_PTR,
    .TxCallback = NULL_PTR,
    .ErrCallback = NULL_PTR,
};

/* Default sent module configuration. */
static const Sent_ModuleConfigType DefaultSentModuleConfig =
{
    .FractionDivider = 1U,
    .TimeStampPreDivider = 0U,
};
/*PRQA S 3218 -- */
#endif

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Initializes the SENT channel IO configuration.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] SentChannelIoConfig: sent channel IO configuration
 * @return void
 */
static void Sent_Hal_InitChannelIoCtrl(Sent_ChannelIdType ChannelId, const Sent_IoCtrlType *SentChannelIoConfig);

/*!
 * @brief Initializes the SENT channel Rx configuration.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] SentChannelRxConfig: sent channel Rx configuration
 * @return void
 */
static void Sent_Hal_InitChannelRxCtrl(Sent_ChannelIdType ChannelId, const Sent_ReceiveCtrlType *SentChannelRxConfig);

/*!
 * @brief Initializes the SENT channel nibble order.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] SentNibbleOrder: sent channel nibble order
 * @return void
 */
static void Sent_Hal_InitChannelNibbleOrder(Sent_ChannelIdType ChannelId,
        const Sent_NibblePointerType *SentNibbleOrder);

/*!
 * @brief Initializes the SENT channel module diver and channel divider.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] TickUnit: sent channel tick time,unit:seconds
 * @return void
 */
static void Sent_Hal_InitChannelTickTime(Sent_ChannelIdType ChannelId, float32 TickUnit);

/*!
 * @brief SENT interrupt handler.
 *
 * @param[in] ChannelId: SENT channel ID
 * @return void
 */
static void Sent_Hal_IrqHandler(Sent_ChannelIdType ChannelId);

/*!
 * @brief SENT Rx interrpt handler function.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] InterruptStatus: Current status of the interrupt
 * @return void
 */
static void Sent_Hal_RxIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus);

/*!
 * @brief SENT Tx interrpt handler function.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] InterruptStatus: Current status of the interrupt
 * @return void
 */
static void Sent_Hal_TxIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus);

/*!
 * @brief SENT error interrpt handler function.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] InterruptStatus: Current status of the interrupt
 * @return void
 */
static void Sent_Hal_ErrIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus);

/*===============================GLOBAL FUNCTION  IMPLEMENTATIONS=================================*/
#ifndef SENT_SDK_NON_EXTENDED_API
/*!
 * @brief Initializes the SENT channel configuration structure with default values.
 *
 * @param[out] SentChannelConfig: Initializes a configuration structure received from the application
 *                                with default values
 * @return void
 */
void Sent_Hal_GetDefaultChannelConfig(Sent_ChannelConfigType *SentChannelConfig)
{
    *SentChannelConfig = DefaultSentChannelConfig;
}

/*!
 * @brief Initializes the SENT module configuration structure with default values.
 *
 * @param[out] SentModuleConfig: Initializes a configuration structure received from the application
 *                               with default values
 * @return void
 */
void Sent_Hal_GetDefaultModuleConfig(Sent_ModuleConfigType *SentModuleConfig)
{
    *SentModuleConfig = DefaultSentModuleConfig;
}
#endif

/*!
 * @brief initializes the SENT module
 *
 * @param[in] SentModuleConfig: sent module config pointer
 * @return void
 */
void Sent_Hal_Init(const Sent_ModuleConfigType *SentModuleConfig)
{
    DEVICE_ASSERT(SentModuleConfig != NULL_PTR);
    DEVICE_ASSERT(SentModuleConfig->FractionDivider > 0U);
    uint8 Idx;

    (void)Ckgen_Hal_EnablePeriphClk(CKGEN_SENT_BUS_CLK, TRUE);
    Rcm_Hal_SetResetState(RCM_RESET_ID_SENT, RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(RCM_RESET_ID_SENT, RCM_RESET_STATE_DEASSERT);

    Sent_Reg_SetModuleClkDivider(SentModuleBase, SentModuleConfig->FractionDivider);
    Sent_Reg_SetModuleTimeStampDivider(SentModuleBase, SentModuleConfig->TimeStampPreDivider);

    /* Enable SENT NVIC interrupt */
    for (Idx = 0U; Idx < SENT_INSTANCE_MAX; Idx++)
    {
        Core_Hal_EnableIrq(SentIrqId[Idx]);
    }
}

/*!
 * @brief deinitializes the SENT module
 *
 * @return void
 */
void Sent_Hal_Deinit(void)
{
    uint8 Idx;

    /* Disable SENT clock */
    Rcm_Hal_SetResetState(RCM_RESET_ID_SENT, RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(RCM_RESET_ID_SENT, RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(CKGEN_SENT_BUS_CLK, FALSE);

    /* Disable SENT NVIC interrupt. */
    for (Idx = 0U; Idx < SENT_INSTANCE_MAX; Idx++)
    {
        Core_Hal_DisableIrq(SentIrqId[Idx]);
        Core_Hal_ClearPendingIrq(SentIrqId[Idx]);
    }
}

/*!
 * @brief Set the SENT channel configuration
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] SentChannelConfig: sent channel config pointer
 * @return void
 */
void Sent_Hal_ConfigChannel(Sent_ChannelIdType ChannelId,
                            const Sent_ChannelConfigType *SentChannelConfig)
{
    DEVICE_ASSERT(SentChannelConfig != NULL_PTR);
    DEVICE_ASSERT(Sent_ChannelStateArray[ChannelId] != NULL_PTR);

    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];
    Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];

    /* disable channel */
    Sent_Hal_EnableChannel(ChannelId, FALSE);

    /* Set SENT channel config */
    Sent_Reg_SetWatchdogLimit(ChannelBase, SentChannelConfig->WatchdogTimerLimit);
    Sent_Hal_InitChannelIoCtrl(ChannelId, &(SentChannelConfig->IoControl));
    Sent_Hal_InitChannelRxCtrl(ChannelId, &(SentChannelConfig->RxControl));

    Sent_Hal_InitChannelTickTime(ChannelId, SentChannelConfig->TickUnit);
    Sent_Reg_EnableIrq(ChannelBase, SentChannelConfig->EnabledInterrupts);
    Sent_Hal_InitChannelNibbleOrder(ChannelId, &(SentChannelConfig->NibblePointer));

    SentChannelState->EnabledInterrupts = SentChannelConfig->EnabledInterrupts;
    SentChannelState->RxCallback = SentChannelConfig->RxCallback;
    SentChannelState->TxCallback = SentChannelConfig->TxCallback;
    SentChannelState->ErrCallback = SentChannelConfig->ErrCallback;
}

/*!
 * @brief enable/disable SENT channel
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] Enable: enable or disbale
 * @return void
 */
void Sent_Hal_EnableChannel(Sent_ChannelIdType ChannelId, boolean Enable)
{
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    Sent_Reg_EnableChannel(ChannelBase, Enable);
}

/*!
 * @brief Initializes the SENT channel Tx configuration.
 *
 * @param[in] ChannelId: sent channel Id
 * @param[in] SentChannelTxConfig: sent channel Tx configuration
 * @return void
 */
void Sent_Hal_InitChannelTxCtrl(Sent_ChannelIdType ChannelId, const Sent_TransmitCtrlType *SentChannelTxConfig)
{
    DEVICE_ASSERT(SentChannelTxConfig != NULL_PTR);
    uint32 params = 0;
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    params = ((uint32)(SentChannelTxConfig->PulseLength) & SENT_CHANNEL_SCR_PLEN_Msk);
    params |= (((uint32)(SentChannelTxConfig->PulseDelayLength) << SENT_CHANNEL_SCR_DEL_Pos) \
               & SENT_CHANNEL_SCR_DEL_Msk);
    params |= (((uint32)(SentChannelTxConfig->Mode) << SENT_CHANNEL_SCR_TRIG_Pos) & SENT_CHANNEL_SCR_TRIG_Msk);
    params |= (((uint32)(SentChannelTxConfig->TimeBase) << SENT_CHANNEL_SCR_BASE_Pos) & SENT_CHANNEL_SCR_BASE_Msk);

    Sent_Reg_SetSPCParams(ChannelBase, params);
}

/*!
 * @brief SENT get channel state
 *
 * @param[in] ChannelId: sent channel Id
 * @return channel status
 */
Sent_ChannelStatusType Sent_Hal_GetChannelStatus(Sent_ChannelIdType ChannelId)
{
    const SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    return Sent_Reg_GetChannelStatus(ChannelBase);
}

/*PRQA S 3408 ++ */ /*  IRQHandler is used in startup.s */
/*!
 * @brief SENT channel 0 interrupts handler
 *
 * @return void
 */
ISR(SENT_Channel0_IRQHandler)
{
    Sent_Hal_IrqHandler(SENT_CHANNEL_0);
}

/*!
 * @brief SENT channel 1 interrupts handler
 *
 * @return void
 */
ISR(SENT_Channel1_IRQHandler)
{
    Sent_Hal_IrqHandler(SENT_CHANNEL_1);
}

#if defined (AC7843X)
/*!
 * @brief SENT channel 2 interrupts handler
 *
 * @return void
 */
ISR(SENT_Channel2_IRQHandler)
{
    Sent_Hal_IrqHandler(SENT_CHANNEL_2);
}

/*!
 * @brief SENT channel 3 interrupts handler
 *
 * @return void
 */
ISR(SENT_Channel3_IRQHandler)
{
    Sent_Hal_IrqHandler(SENT_CHANNEL_3);
}
#endif
/*PRQA S 3408 -- */
/*================================STATIC  FUNCTION  IMPLEMENTATIONS=================================*/
/*!
 * @brief Initializes the SENT channel IO configuration.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] SentChannelIoConfig: sent channel IO configuration
 * @return void
 */
static void Sent_Hal_InitChannelIoCtrl(Sent_ChannelIdType ChannelId, const Sent_IoCtrlType *SentChannelIoConfig)
{
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    Sent_Reg_EnableInputInvert(ChannelBase, SentChannelIoConfig->InputInvertEnable);
    Sent_Reg_EnableOutputInvert(ChannelBase, SentChannelIoConfig->OutputInvertEnable);
    Sent_Reg_SetGlitchFilterDepth(ChannelBase, SentChannelIoConfig->GlitchFilterDepth);
    Sent_Reg_SelectInputDataSource(ChannelBase, SentChannelIoConfig->Input);
    Sent_Reg_SelectExternalTriggerSource(ChannelBase, SentChannelIoConfig->TriggerSource);
}

/*!
 * @brief Initializes the SENT channel Rx configuration.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] SentChannelRxConfig: sent channel Rx configuration
 * @return void
 */
static void Sent_Hal_InitChannelRxCtrl(Sent_ChannelIdType ChannelId, const Sent_ReceiveCtrlType *SentChannelRxConfig)
{
    DEVICE_ASSERT(SentChannelRxConfig != NULL_PTR);

    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    Sent_Reg_EnablePausePulse(ChannelBase, SentChannelRxConfig->PausePulseEnable);
    Sent_Reg_EnableAlternateCRC(ChannelBase, SentChannelRxConfig->AlternateCRCEnable);
    Sent_Reg_EnableStatusNibbleInCRC(ChannelBase, SentChannelRxConfig->StatusIncludedInCRC);
    Sent_Reg_EnableSerialMsgProcess(ChannelBase, SentChannelRxConfig->SerialDataEnable);
    Sent_Reg_EnableSerialMsgCRC(ChannelBase, SentChannelRxConfig->SerialCRCEnable);
    Sent_Reg_EnableFastMsgCRC(ChannelBase, SentChannelRxConfig->FastCRCEnable);
    Sent_Reg_SetFrameCheckMode(ChannelBase, SentChannelRxConfig->FrameCheckMode);
    Sent_Reg_SetFrameLength(ChannelBase, SentChannelRxConfig->FrameLength);
    Sent_Reg_EnableCRCAugumentZero(ChannelBase, SentChannelRxConfig->ZeroAugumentInCRCDisable);
    Sent_Reg_SetSerialMsgType(ChannelBase, SentChannelRxConfig->SerialMsgType);
    Sent_Reg_DisableDriftError(ChannelBase, SentChannelRxConfig->DriftErrorDisable);
    Sent_Reg_EnableFDFLMode(ChannelBase, SentChannelRxConfig->FDFL);
}

/*!
 * @brief Initializes the SENT channel module diver and channel divider.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] TickUnit: sent channel tick time,unit:seconds
 * @return void
 */
static void Sent_Hal_InitChannelTickTime(Sent_ChannelIdType ChannelId, float32 TickUnit)
{
    uint32 SentSourceClock = 0U;
    float32 SendModuleDivClock, Tmp;
    uint32 Pdiv, Fdiv;

    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    /* Get the SENT clock as configured in the clock manager */
    (void)Ckgen_Hal_GetFreq(CKGEN_BUS_CLK, &SentSourceClock);

    SendModuleDivClock = (float32)(SentSourceClock) / (float32)(Sent_Reg_GetModuleClkDivider(SentModuleBase));

    Pdiv = 1U;
    Tmp = (SendModuleDivClock * (float32)56U * TickUnit) / (float32)Pdiv;
    Fdiv = (uint32)Tmp;

    /* DIV max: 49100; min: todo */
    while (Fdiv >= 49100U)
    {
        Pdiv = Pdiv << 1U;
        Tmp = (SendModuleDivClock * (float32)56U * TickUnit) / (float32)Pdiv;
        Fdiv = (uint32)Tmp;
    }

    Sent_Reg_SetChannelPreDivider(ChannelBase, (uint16)(Pdiv - 1U));
    Sent_Reg_SetChannelFractionalDivider(ChannelBase, (uint16)Fdiv);
}

/*!
 * @brief Initializes the SENT channel nibble order.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] SentNibbleOrder: sent channel nibble order
 * @return void
 */
static void Sent_Hal_InitChannelNibbleOrder(Sent_ChannelIdType ChannelId,
        const Sent_NibblePointerType *SentNibbleOrder)
{
    DEVICE_ASSERT(SentNibbleOrder != NULL_PTR);

    uint32 View;
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    View = (uint32)(SentNibbleOrder->Nibble0Position)
           | ((uint32)(SentNibbleOrder->Nibble1Position) << 4U)
           | ((uint32)(SentNibbleOrder->Nibble2Position) << 8U)
           | ((uint32)(SentNibbleOrder->Nibble3Position) << 12U)
           | ((uint32)(SentNibbleOrder->Nibble4Position) << 16U)
           | ((uint32)(SentNibbleOrder->Nibble5Position) << 20U)
           | ((uint32)(SentNibbleOrder->Nibble6Position) << 24U)
           | ((uint32)(SentNibbleOrder->Nibble7Position) << 28U);

    Sent_Reg_SetDataNibbleView(ChannelBase, View);
}

/*!
 * @brief SENT interrupt handler.
 *
 * @param[in] ChannelId: SENT channel ID
 * @return void
 */
static void Sent_Hal_IrqHandler(Sent_ChannelIdType ChannelId)
{
    uint32 InterruptStatus, EnabledInterrupts;
    const Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    InterruptStatus = Sent_Reg_GetIrqStatus(ChannelBase);
    EnabledInterrupts = SentChannelState->EnabledInterrupts;

    if (((EnabledInterrupts & (uint32)IRQ_RECEIVE) != 0U) && ((InterruptStatus & (uint32)IRQ_RECEIVE) != 0U))
    {
        Sent_Hal_RxIrqHandler(ChannelId, InterruptStatus);
    }

    if (((EnabledInterrupts & (uint32)IRQ_TRANSMIT) != 0U) && ((InterruptStatus & (uint32)IRQ_TRANSMIT) != 0U))
    {
        Sent_Hal_TxIrqHandler(ChannelId, InterruptStatus);
    }

    if (((EnabledInterrupts & (uint32)IRQ_ERROR) != 0U) && ((InterruptStatus & (uint32)IRQ_ERROR) != 0U))
    {
        Sent_Hal_ErrIrqHandler(ChannelId, InterruptStatus);
    }

    Sent_Reg_ClearIrqStatus(ChannelBase, InterruptStatus);
}

/*!
 * @brief SENT Rx interrpt handler function.
 *
 * @param[in] ChannelId: sent channel ID
 * @return void
 */
static void Sent_Hal_RxIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus)
{
    const SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];
    const SENT_CTRL_Type *ModuleBase = SentModuleBase;
    Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];
    uint32 Event = 0U;

    SentChannelState->SentMsg.FifoStatus.FrameDataFifoAvail = Sent_Reg_GetFifoAvailable(ChannelBase);
    SentChannelState->SentMsg.FifoStatus.FrameDataFifoEmpty = (boolean)(Sent_Reg_GetFifoEmptyStatus(ChannelBase));
    SentChannelState->SentMsg.FifoStatus.FrameDataFifoFull = (boolean)(Sent_Reg_GetFifoFullStatus(ChannelBase));
    SentChannelState->SentMsg.FifoStatus.FrameDataFifoHalfFull = (boolean)(Sent_Reg_GetFifoHalfFullStatus(ChannelBase));

    if ((InterruptStatus & IRQ_RDI) != 0U)
    {
        SentChannelState->SentMsg.DataFrame.Data = Sent_Reg_GetFrameDataNibbles(ChannelBase);
        SentChannelState->SentMsg.DataFrame.FrameDataCRC = Sent_Reg_GetFrameCRCNibble(ChannelBase);
        SentChannelState->SentMsg.DataFrame.StatusNibble = Sent_Reg_GetFrameStatusNibble(ChannelBase);
        SentChannelState->SentMsg.DataFrame.TimeStamp = Sent_Reg_GetCurrentTimeStamp(ModuleBase);
        SentChannelState->SentMsg.DataFrame.TotalFrameLength = Sent_Reg_GetFrameLengthCounter(ChannelBase);
    }

    if ((InterruptStatus & IRQ_SDI) != 0U)
    {
        SentChannelState->SentMsg.SerialFrame.ConfigBit = Sent_Reg_GetEnhancedConfigType(ChannelBase);
        SentChannelState->SentMsg.SerialFrame.SerialMsgCRC = Sent_Reg_GetSerialMsgCRC(ChannelBase);
        SentChannelState->SentMsg.SerialFrame.SerialMsgData = Sent_Reg_GetSerialMsgData(ChannelBase);
        SentChannelState->SentMsg.SerialFrame.SerialMsgID = Sent_Reg_GetSerialMsgID(ChannelBase);
    }

    if (SentChannelState->RxCallback != NULL_PTR)
    {
        if ((InterruptStatus & IRQ_RSI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_SUCCESS;
        }

        if ((InterruptStatus & IRQ_RDI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_DATA;
        }

        if ((InterruptStatus & IRQ_SDI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_SERIAL_MSG;
        }

        if ((InterruptStatus & IRQ_HFLL) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_FIFO_HALF_FULL;
        }

        if ((InterruptStatus & IRQ_FDONE) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_FRAME_DONE;
        }

        SentChannelState->RxCallback(ChannelId, Event, &SentChannelState->SentMsg);
    }
}

/*!
 * @brief SENT Tx interrpt handler function.
 *
 * @param[in] ChannelId: sent channel ID
 * @return void
 */
static void Sent_Hal_TxIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus)
{
    const Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];
    uint32 Event = 0;

    if (SentChannelState->TxCallback != NULL_PTR)
    {
        if ((InterruptStatus & IRQ_TDI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_TRANSMIT_DATA;
        }

        SentChannelState->TxCallback(ChannelId, Event);
    }
}

/*!
 * @brief SENT error interrpt handler function.
 *
 * @param[in] ChannelId: sent channel ID
 * @return void
 */
static void Sent_Hal_ErrIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus)
{
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];
    const Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];
    uint32 Event = 0;

    if ((InterruptStatus & IRQ_RFI) != 0U)
    {
        Sent_Reg_EnableFlushFifo(ChannelBase, TRUE);
        Sent_Reg_EnableFlushFifo(ChannelBase, FALSE);
        Event |= (uint32)SENT_EVENT_ERROR_RECEIVE_FIFO_OVERFLOW;
    }

    if (SentChannelState->ErrCallback != NULL_PTR)
    {
        if ((InterruptStatus & IRQ_TBI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_TRANSMIT_BUFFER_UNDERFLOW;
        }

        if ((InterruptStatus & IRQ_FRI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_FREQUENCY_RANGE;
        }

        if ((InterruptStatus & IRQ_FDI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_FREQUENCY_DRIFT;
        }

        if ((InterruptStatus & IRQ_NNI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_NIBBLE_NUMBER;
        }

        if ((InterruptStatus & IRQ_NVI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_NIBBLE_DATA;
        }

        if ((InterruptStatus & IRQ_CRCI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_NIBBLE_CRC;
        }

        if ((InterruptStatus & IRQ_WSI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_SHORT_START_BIT;
        }

        if ((InterruptStatus & IRQ_SCRI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_SERIAL_CRC;
        }

        if ((InterruptStatus & IRQ_WDI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_WATCH_DOG;
        }

        if ((InterruptStatus & IRQ_ESI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_ENHANCED_START_BIT;
        }

        SentChannelState->ErrCallback(ChannelId, Event);
    }
}

/* =============================================  EOF  ============================================== */
