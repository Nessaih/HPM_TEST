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
* @file Uart_Hal.c
* @brief  This file provides Uart hal function
*/
/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Uart_Reg.h"
#include "Uart_Hal.h"
#include "Dma_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"
#include "OsIf_Time.h"
#include "OsIf_Irq.h"
#include "OsIf_Critical.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
#define UART_SAMPLE_CNT_4_VALUE       (4U)
#define UART_SAMPLE_CNT_8_VALUE       (8U)
#define UART_SAMPLE_CNT_16_VALUE      (16U)

#define UART_USE_DMA_TRANSMIT_LEN_MAX (32768U)

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*!
 * @brief UART runtime state.
 */
typedef struct
{
    boolean InitState;
    const uint8 *TxBuff; /*!< The buffer of transmit data */
    uint8 *RxBuff; /*!< The buffer of received data */
    uint32 TxSize; /*!< The remaining number of bytes to be transmitted */
    uint32 RxSize; /*!< The remaining number of bytes to be received */
    boolean IsTxBusy; /*!< True if transmit is active */
    boolean IsRxBusy; /*!< True if receive is active */
    boolean IsTxBlocking; /*!< True if transmit is blocking */
    boolean IsRxBlocking; /*!< True if receive is blocking */
    Uart_PerCharConfigType BitCountPerChar; /*!< number of bits(7/8/9 bits) in a char */
    Uart_CallbackType RxCallback; /*!< Callback for data receive */
    Uart_CallbackType TxCallback; /*!< Callback for data send */
    Uart_TransferType TransferType; /*!< Type of UART transfer (interrupt or dma) */
    uint8 RxDmaChannel; /*!< DMA channel for UART receive. */
    uint8 TxDmaChannel; /*!< DMA channel for UART transmit. */
    volatile boolean RxComplete; /*!< Flag to indicate the completion of a blocking receive operation */
    volatile boolean TxComplete; /*!< Flag to indicate the completion of a blocking transmit operation */
    Hal_StatusType SendStatus; /*!< Status of last transmit */
    Hal_StatusType ReceiveStatus; /*!< Status of last receive */
} Uart_ChannelStateType;

/*===========================================VARIABLE DECLARATIONS==================================*/
/* Uart state Channel config */
#if (CONFIG_UART0_ENABLE)
static Uart_ChannelStateType Uart0_ChannelState;
#endif
#if (CONFIG_UART1_ENABLE)
static Uart_ChannelStateType Uart1_ChannelState;
#endif
#if (CONFIG_UART2_ENABLE)
static Uart_ChannelStateType Uart2_ChannelState;
#endif
#if (CONFIG_UART3_ENABLE)
static Uart_ChannelStateType Uart3_ChannelState;
#endif

#if defined (AC7843X)
#if (CONFIG_UART4_ENABLE)
static Uart_ChannelStateType Uart4_ChannelState;
#endif
#if (CONFIG_UART5_ENABLE)
static Uart_ChannelStateType Uart5_ChannelState;
#endif
#if (CONFIG_UART6_ENABLE)
static Uart_ChannelStateType Uart6_ChannelState;
#endif
#if (CONFIG_UART7_ENABLE)
static Uart_ChannelStateType Uart7_ChannelState;
#endif
#endif

static Uart_ChannelStateType *const Uart_ChannelStateArray[UART_INSTANCE_MAX] =
{
#if (CONFIG_UART0_ENABLE)
    &Uart0_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART1_ENABLE)
    &Uart1_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART2_ENABLE)
    &Uart2_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART3_ENABLE)
    &Uart3_ChannelState,
#else
    NULL_PTR,
#endif

#if defined (AC7843X)
#if (CONFIG_UART4_ENABLE)
    &Uart4_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART5_ENABLE)
    &Uart5_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART6_ENABLE)
    &Uart6_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART7_ENABLE)
    &Uart7_ChannelState,
#else
    NULL_PTR,
#endif
#endif
};

/* Table of base addresses for uart instances. */
static UART_Type *const Uart_HalBase[UART_INSTANCE_MAX] =
{
    UART0, UART1, UART2, UART3,
#if defined (AC7843X)
    UART4, UART5, UART6, UART7
#endif
};
/*PRQA S 0306 -- */

/* Table to save UART IRQ numbers. */
static const IRQn_Type Uart_IrqId[UART_INSTANCE_MAX] = UART_IRQS;

/*PRQA S 3218 ++ # Multiple functions in the SDK used.*/
static const Ckgen_ClkIdType Uart_HalClock[UART_INSTANCE_MAX] =
{
    CKGEN_UART0_CLK, CKGEN_UART1_CLK, CKGEN_UART2_CLK, CKGEN_UART3_CLK,
#if defined (AC7843X)
    CKGEN_UART4_CLK, CKGEN_UART5_CLK, CKGEN_UART6_CLK, CKGEN_UART7_CLK
#endif
};
/*PRQA S 3218 -- */

static const Ckgen_BusClkIdType Uart_HalBusClock[UART_INSTANCE_MAX] =
{
    CKGEN_UART0_BUS_CLK, CKGEN_UART1_BUS_CLK, CKGEN_UART2_BUS_CLK, CKGEN_UART3_BUS_CLK,
#if defined (AC7843X)
    CKGEN_UART4_BUS_CLK, CKGEN_UART5_BUS_CLK, CKGEN_UART6_BUS_CLK, CKGEN_UART7_BUS_CLK
#endif
};

static const Rcm_ResetIDType Uart_HalClockReset[UART_INSTANCE_MAX] =
{
    RCM_RESET_ID_UART0, RCM_RESET_ID_UART1, RCM_RESET_ID_UART2, RCM_RESET_ID_UART3,
#if defined (AC7843X)
    RCM_RESET_ID_UART4, RCM_RESET_ID_UART5, RCM_RESET_ID_UART6, RCM_RESET_ID_UART7
#endif
};

/*============================================FUNCTION PROTOTYPES===================================*/
#if (CONFIG_UART0_ENABLE)
ISR(UART0_IRQHandler);
#endif

#if (CONFIG_UART1_ENABLE)
ISR(UART1_IRQHandler);
#endif

#if (CONFIG_UART2_ENABLE)
ISR(UART2_IRQHandler);
#endif

#if (CONFIG_UART3_ENABLE)
ISR(UART3_IRQHandler);
#endif

#if defined (AC7843X)
#if (CONFIG_UART4_ENABLE)
ISR(UART4_IRQHandler);
#endif

#if (CONFIG_UART5_ENABLE)
ISR(UART5_IRQHandler);
#endif

#if (CONFIG_UART6_ENABLE)
ISR(UART6_IRQHandler);
#endif

#if (CONFIG_UART7_ENABLE)
ISR(UART7_IRQHandler);
#endif
#endif

#ifndef UART_SDK_NON_EXTENDED_API
/**
* @brief  Prepare for timeout checking.
* @note Function ID: DES_UART_API_051
* @param [out] CounterOut: Pointer to store the current counter value.
* @param [out] TicksOut: Pointer to store the number of ticks corresponding to the timeout.
* @param [in] Timeout: Timeout in microseconds.
* @return void
*/
LOCAL_INLINE void Uart_Hal_StartTimeout(uint32 *CounterOut, uint32 *TicksOut, uint32 Timeout);

/**
* @brief  Check if the timeout has elapsed.
* @note Function ID: DES_UART_API_052
* @param [inout] Counter: Pointer to counter value, which is updated with the current counter.
* @param [inout] ElapsedTicks: Pointer to store the accumulated elapsed ticks.
* @param [in] TimeoutTicks: The total number of ticks representing the timeout.
* @return TRUE if the timeout has elapsed, FALSE otherwise
*/
LOCAL_INLINE boolean Uart_Hal_CheckTimeout(uint32 *Counter, uint32 *ElapsedTicks, uint32 TimeoutTicks);
#endif

/**
* @brief  Set UART Tx buffer and enable TX interrupt.
* @note Function ID: DES_UART_API_053
* @param [in] Instance: UART hardware channel ID
* @param [in] TxBuffer: UART tx buffer base pointer
* @param [in] TxSize: UART rx buffer bytes size
* @return Hal_StatusType: operating status
*/
static Hal_StatusType Uart_Hal_StartSendDataUsingInt(uint8 Instance, const uint8 *TxBuffer, uint32 TxSize);

/**
* @brief  UART stop DMA tansfer.
* @note Function ID: DES_UART_API_054
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_StopTxDma(uint8 Instance);

/**
* @brief  Complete UART interrupt transfer.
* @note Function ID: DES_UART_API_055
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_CompleteSendDataUsingInt(uint8 Instance);

/**
* @brief  UART tx complete interrupt handler function.
* @note Function ID: DES_UART_API_056
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_TxCompleteIrqHandler(uint8 Instance);

/**
* @brief  UART DMA transfer callback function.
* @note Function ID: DES_UART_API_057
* @param [in] Param: Function parameter
* @return void
*/
static void Uart_Hal_TxDmaCallback(void *Param);

/**
* @brief  Start UART DMA Send.
* @note Function ID: DES_UART_API_058
* @param [in] Instance: UART hardware channel ID
* @param [in] TxBuffer: The tx data buffer pointer
* @param [in] TxSize: The tx data buffer bytes size
* @return Hal_StatusType. if successful return STATUS_SUCCESS
*/
static Hal_StatusType Uart_Hal_StartSendDataUsingDma(uint8 Instance, const uint8 *TxBuffer, uint32 TxSize);

/**
* @brief  Set UART Rx buffer and enable RX interrupt.
* @note Function ID: DES_UART_API_059
* @param [in] Instance: UART hardware channel ID
* @param [in] RxSize: UART rx buffer bytes size
* @param [out] RxBuffer: UART rx buffer base pointer
* @return Hal_StatusType. if successful return STATUS_SUCCESS.
*/
static Hal_StatusType Uart_Hal_StartReceiveDataUsingInt(uint8 Instance, uint8 *RxBuffer, uint32 RxSize);

/**
* @brief  UART stop DMA receive.
* @note Function ID: DES_UART_API_060
* @param [in] Instance: UART hardware channel ID
* @return void
*/
static void Uart_Hal_StopRxDma(uint8 Instance);

/**
* @brief  UART DMA receive callback function.
* @note Function ID: DES_UART_API_061
* @param [in] Param: Function parameter
* @return void
*/
static void Uart_Hal_RxDmaCallback(void *Param);

/**
* @brief  Start UART DMA receive.
* @note Function ID: DES_UART_API_062
* @param [in] Instance : UART hardware channel ID
* @param [in] RxSize: The rx data buffer bytes size
* @param [out] RxBuffer: The rx data buffer pointer
* @return Hal_StatusType. if successful return STATUS_SUCCESS or return STATUS_ERROR/STATUS_BUSY.
*/
static Hal_StatusType Uart_Hal_StartReceiveDataUsingDma(uint8 Instance, uint8 *RxBuffer, uint32 RxSize);

/**
* @brief  Complete UART interrupt receive.
* @note Function ID: DES_UART_API_063
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_CompleteReceiveDataUsingInt(uint8 Instance);

/**
* @brief  UART error interrupt handler function.
* @note Function ID: DES_UART_API_064
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_ErrIrqHandler(uint8 Instance);
/**
* @brief  UART read data from RBR register.
* @note Function ID: DES_UART_API_065
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_GetData(uint8 Instance);

/**
* @brief  UART rx not empty interrupt handler function.
* @note Function ID: DES_UART_API_066
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_RxIrqHandler(uint8 Instance);

/**
* @brief  UART write data to RBR register.
* @note Function ID: DES_UART_API_067
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_PutData(uint8 Instance);

/**
* @brief  UART tx not full interrupt handler function.
* @note Function ID: DES_UART_API_068
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_TxIrqHandler(uint8 Instance);

/**
* @brief  Initializes the UART channel state.
* @note Function ID: DES_UART_API_069
* @param [in] Instance : UART hardware channel ID.
* @param [in] ChannelConfigPtr : Pointer to the UART channel configuration structure.
* @return void
*/
static void Uart_Hal_StateInit(uint8 Instance, const Uart_ChannelConfigType *ChannelConfigPtr);

/**
* @brief  UART interrupt handler.
* @note Function ID: DES_UART_API_070
* @param [in] Instance : UART hardware channel ID
* @return void
*/
static void Uart_Hal_IRQHandler(uint8 Instance);

/*===============================GLOBAL FUNCTION  IMPLEMENTATIONS=================================*/
/**
 * @brief  Initializes the Uart module.
 * @note Function ID: DES_UART_API_000
 * @param [in] Instance: UART hardware channel ID
 * @param [in] ChannelConfigPtr: Pointer to Uart_ChannelConfigType
 * @return void
 */
void Uart_Hal_Init(uint8 Instance, const Uart_ChannelConfigType *ChannelConfigPtr)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelConfigPtr != NULL_PTR);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    /* Check if current instance is already initialized. */
    DEVICE_ASSERT(FALSE == ChannelState->InitState);

    /* Init the state struct */
    Uart_Hal_StateInit(Instance, ChannelConfigPtr);
    /* enable uart clock */
    (void)Ckgen_Hal_EnablePeriphClk(Uart_HalBusClock[Instance], TRUE);
    Rcm_Hal_SetResetState(Uart_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Uart_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    /* initialize the parameters of the UART config structure */
    (void)Uart_Hal_SetBaudRate(Instance, ChannelConfigPtr->BaudRate, ChannelConfigPtr->SampleCnt);
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/

    Uart_Reg_SetBitCountPerChar(BaseAddress, ChannelConfigPtr->BitCountPerChar);

    Uart_Reg_SetParityMode(BaseAddress, ChannelConfigPtr->ParityMode);
    Uart_Reg_SetStopBitCount(BaseAddress, ChannelConfigPtr->StopBitCount);

    /* Enable UART FIFO */
    Uart_Reg_SetFIFO(BaseAddress, TRUE);

    /* Enable UART NVIC interrupt */
    Core_Hal_EnableIrq(Uart_IrqId[Instance]);
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /* Enable the UART transmitter */
    Uart_Reg_SetTransmitterCmd(BaseAddress, TRUE);

    /* Enable the UART receiver */
    Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);

    ChannelState->InitState = TRUE;
}

/**
 * @brief  Deinitializes the Uart module.
 * @note Function ID: DES_UART_API_001
 * @param [in] Instance: UART hardware channel ID
 * @return void
 */
void Uart_Hal_DeInit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    /* Disable UART NVIC interrupt. */
    Core_Hal_DisableIrq(Uart_IrqId[Instance]);

    /* disable uart clock */
    Rcm_Hal_SetResetState(Uart_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Uart_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(Uart_HalBusClock[Instance], FALSE);

    Core_Hal_ClearPendingIrq(Uart_IrqId[Instance]);
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    ChannelState->TxComplete = FALSE;
    ChannelState->RxComplete = FALSE;
    ChannelState->InitState = FALSE;
}

/*!
 * @brief  UART send data using non-blocking method (interrupt or DMA).
 * @note Function ID: DES_UART_API_002
 * @param [in] Instance: UART hardware channel ID
 * @param [in] TxBuff: The tx data buffer pointer
 * @param [in] TxSize: The tx data buffer bytes size
 * @return Hal_StatusType: if successful return STATUS_SUCCESS or return STATUS_ERROR/STATUS_BUSY.
 */
Hal_StatusType Uart_Hal_SendData(uint8 Instance, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Hal_StatusType RetVal = STATUS_SUCCESS;
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check it's not busy transmitting data from a previous function call */
    if (TRUE == ChannelState->IsTxBusy)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        DEVICE_ASSERT((UART_USING_INTERRUPTS == ChannelState->TransferType) \
                      || (UART_USING_DMA == ChannelState->TransferType));

        /* Indicates this is a non-blocking transaction. */
        ChannelState->IsTxBlocking = FALSE;
        ChannelState->IsTxBusy = TRUE;

        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            /* Start the transmission process using interrupts */
            RetVal = Uart_Hal_StartSendDataUsingInt(Instance, TxBuff, TxSize);
        }
        else
        {
            /* Start the transmission process using DMA */
            RetVal = Uart_Hal_StartSendDataUsingDma(Instance, TxBuff, TxSize);
        }
    }

    return RetVal;
}

#ifndef UART_SDK_NON_EXTENDED_API
/*!
 * @brief  UART send data using polling mode.
 * @note Function ID: DES_UART_API_003
 * @param [in] Instance: UART hardware channel ID
 * @param [in] TxBuff: The tx data buffer pointer
 * @param [in] TxSize: The tx data buffer bytes size
 * @return Hal_StatusType: if successful return STATUS_SUCCESS or return STATUS_BUSY.
 */
Hal_StatusType Uart_Hal_SendDataPolling(uint8 Instance, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    const UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/
    Hal_StatusType RetVal = STATUS_SUCCESS;

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check the validity of the parameters */
    DEVICE_ASSERT((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar) \
                  || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar) \
                  || (0U == (TxSize & 1U)));

    /* Check driver is not busy transmitting data from a previous asynchronous call */
    if (TRUE == ChannelState->IsTxBusy)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        ChannelState->TxSize = TxSize;
        ChannelState->TxBuff = TxBuff;
        while (ChannelState->TxSize > 0U)
        {
            while (FALSE == Uart_Reg_GetStatusFlag(BaseAddress, UART_TX_DATA_NOT_FULL))
            {
                /* Do nothing */
            }

            Uart_Hal_PutData(Instance);

            if ((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar) \
                    || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar))
            {
                ++ChannelState->TxBuff;
                --ChannelState->TxSize;
            }
            else
            {
                ChannelState->TxBuff += 2U;
                ChannelState->TxSize -= 2U;
            }
        }

        ChannelState->SendStatus = STATUS_SUCCESS;
    }

    return RetVal;
}

/*!
 * @brief  UART send data using blocking method, which will not return until the
 *        transmit is complete.
 * @note Function ID: DES_UART_API_004
 * @param [in] Instance: UART hardware channel ID
 * @param [in] TxBuff: The tx data buffer pointer
 * @param [in] TxSize: The tx data buffer bytes size
 * @param [in] TimeoutUs: Maximum time in microseconds to wait for the transmission
 * @return Hal_StatusType: Transmit status
 */
Hal_StatusType Uart_Hal_SendDataBlocking(uint8 Instance, const uint8 *TxBuff, uint32 TxSize, uint32 TimeoutUs)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/
    uint32 Counter;
    uint32 Ticks;
    uint32 Elapse = 0U;
    boolean IsTimeOut = FALSE;
    Hal_StatusType RetVal = STATUS_SUCCESS;

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Indicates this is a blocking transaction. */
    ChannelState->IsTxBlocking = TRUE;
    ChannelState->TxComplete = FALSE;

    DEVICE_ASSERT((UART_USING_INTERRUPTS == ChannelState->TransferType) \
                  || (UART_USING_DMA == ChannelState->TransferType));

    if (UART_USING_INTERRUPTS == ChannelState->TransferType)
    {
        /* Start the transmission process using interrupts */
        RetVal = Uart_Hal_StartSendDataUsingInt(Instance, TxBuff, TxSize);
    }
    else
    {
        /* Start the transmission process using DMA */
        RetVal = Uart_Hal_StartSendDataUsingDma(Instance, TxBuff, TxSize);
    }

    if (STATUS_SUCCESS == RetVal)
    {
        /* Wait until the transmit is complete. */
        Uart_Hal_StartTimeout(&Counter, &Ticks, TimeoutUs);

        while ((FALSE == ChannelState->TxComplete) && (FALSE == IsTimeOut))
        {
            IsTimeOut = Uart_Hal_CheckTimeout(&Counter, &Elapse, Ticks);
        }

        /* Finish the transmission if timeout expired */
        if (TRUE == IsTimeOut)
        {
            ChannelState->IsTxBlocking = FALSE;
            ChannelState->SendStatus = STATUS_TIMEOUT;

            if (UART_USING_INTERRUPTS == ChannelState->TransferType)
            {
                Uart_Hal_CompleteSendDataUsingInt(Instance);
            }
            else
            {
                Uart_Hal_StopTxDma(Instance);
            }
        }
    }

    return ChannelState->SendStatus;
}
#endif

/*!
 * @brief  Get send status.
 * @note Function ID: DES_UART_API_008
 * @param [in] Instance: UART hardware channel ID
 * @param [out] BytesRemaining: Remaining bytes to be sent
 * @return Hal_StatusType: Transmit status
 */
Hal_StatusType Uart_Hal_GetSendStatus(uint8 Instance, uint32 *BytesRemaining)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    const Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    if (BytesRemaining != NULL_PTR)
    {
        if (TRUE == ChannelState->IsTxBusy)
        {
            /* Fill in the bytes not transferred yet. */
            if (UART_USING_INTERRUPTS == ChannelState->TransferType)
            {
                /* In interrupt communication, the remaining bytes are
                   retrieved from the state structure */
                *BytesRemaining = ChannelState->TxSize;;
            }
            else
            {
                /* In DMA communication, the remaining bytes are retrieved
                   from the current DMA channel */
                *BytesRemaining = ChannelState->TxSize - Dma_Hal_GetTransBytes(ChannelState->TxDmaChannel);
            }
        }
        else
        {
            *BytesRemaining = 0U;
        }
    }

    return ChannelState->SendStatus;
}

/*!
 * @brief  Terminates an non-blocking UART transmission early.
 * @note Function ID: DES_UART_API_010
 * @param [in] Instance: UART hardware channel ID
 * @return Hal_StatusType: return STATUS_SUCCESS if successful
 */
Hal_StatusType Uart_Hal_AbortSendingData(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check if a transfer is running. */
    if (TRUE == ChannelState->IsTxBusy)
    {
        /* Update the tx status */
        ChannelState->SendStatus = STATUS_UART_ABORTED;

        /* Stop the running transfer. */
        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            Uart_Hal_CompleteSendDataUsingInt(Instance);
        }
        else
        {
            Uart_Hal_StopTxDma(Instance);
        }
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief  UART receive data using non-blocking method(interrupt or DMA).
 * @note Function ID: DES_UART_API_005
 * @param [in] Instance: UART hardware channel ID
 * @param [out] RxBuff: The rx data buffer pointer
 * @param [in] RxSize: The rx data buffer bytes size
 * @return Hal_StatusType: if successful return STATUS_SUCCESS or return STATUS_ERROR/STATUS_BUSY.
 */
Hal_StatusType Uart_Hal_ReceiveData(uint8 Instance, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Hal_StatusType RetVal = STATUS_SUCCESS;
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check it's not busy receiving data from a previous function call */
    if (TRUE == ChannelState->IsRxBusy)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        DEVICE_ASSERT((UART_USING_INTERRUPTS == ChannelState->TransferType) \
                      || (UART_USING_DMA == ChannelState->TransferType));

        /* Indicates this is a non-blocking transaction. */
        ChannelState->IsRxBlocking = FALSE;
        ChannelState->IsRxBusy = TRUE;

        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            /* Start the reception process using interrupts */
            RetVal = Uart_Hal_StartReceiveDataUsingInt(Instance, RxBuff, RxSize);
        }
        else
        {
            /* Start the reception process using DMA */
            RetVal = Uart_Hal_StartReceiveDataUsingDma(Instance, RxBuff, RxSize);
        }
    }

    return RetVal;
}

#ifndef UART_SDK_NON_EXTENDED_API
/*!
 * @brief  UART receive data using polling mode.
 * @note Function ID: DES_UART_API_006
 * @param [in] Instance: UART hardware channel ID
 * @param [out] RxBuff: The rx data buffer pointer
 * @param [in] RxSize: The rx data buffer bytes size
 * @return Hal_StatusType: Receive status
 */
Hal_StatusType Uart_Hal_ReceiveDataPolling(uint8 Instance, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    uint8 TmpByte = 0U;
    boolean IsError = FALSE;
    Hal_StatusType RetVal = STATUS_SUCCESS;
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check the validity of the parameters */
    DEVICE_ASSERT((ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR) \
                  || (0U == (RxSize & 1U)));

    /* Check driver is not busy receiving data from a previous asynchronous call */
    if (TRUE == ChannelState->IsRxBusy)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        /* Enable the UART receiver */
        Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);
        ChannelState->RxSize = RxSize;
        ChannelState->RxBuff = RxBuff;

        while (ChannelState->RxSize > 0U)
        {
            while (FALSE == Uart_Reg_GetStatusFlag(BaseAddress, UART_RX_DATA_READY))
            {
                /* Do nothing */
            }

            Uart_Hal_GetData(Instance);

            if ((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar) \
                    || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar))
            {
                ++ChannelState->RxBuff;
                --ChannelState->RxSize;
            }
            else
            {
                ChannelState->RxBuff += 2U;
                ChannelState->RxSize -= 2U;
            }

            /* Check for errors on received data */
            if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_FRAME_ERR))
            {
                RetVal = STATUS_UART_FRAMING_ERROR;
                /* Disable the UART receiver */
                Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
                /* Clear the flag */
                Uart_Reg_ClearStatusFlag(BaseAddress, UART_FRAME_ERR);
                IsError = TRUE;
            }
            else if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_NOISE_ERR))
            {
                RetVal = STATUS_UART_NOISE_ERROR;
                /* Disable the UART receiver */
                Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
                /* Clear the flag */
                Uart_Reg_ClearStatusFlag(BaseAddress, UART_NOISE_ERR);
                IsError = TRUE;
            }
            else if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_PARITY_ERR))
            {
                RetVal = STATUS_UART_PARITY_ERROR;
                /* Disable the UART receiver */
                Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
                /* Clear the flag */
                Uart_Reg_ClearStatusFlag(BaseAddress, UART_PARITY_ERR);
                IsError = TRUE;
            }
            else if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_RX_OVERRUN))
            {
                RetVal = STATUS_UART_RX_OVERRUN;
                /* Disable the UART receiver */
                Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
                /* Clear the flag */
                Uart_Reg_ClearStatusFlag(BaseAddress, UART_RX_OVERRUN);
                IsError = TRUE;
            }
            else
            {
                /* No error condition - avoid MISRA violation */
            }

            if (TRUE == IsError)
            {
                break;
            }
        }

        ChannelState->ReceiveStatus = RetVal;

        if (STATUS_SUCCESS == RetVal)
        {
            /* Disable the UART receiver */
            Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
        }

        /* Read dummy to clear RX data ready flag */
        Uart_Reg_GetChar(BaseAddress, &TmpByte);
    }

    return RetVal;
}

/*!
 * @brief  UART receive data using blocking method, which will not return until
 *        the receive is complete.
 * @note  Function ID: DES_UART_API_007
 * @param [in] Instance: UART hardware channel ID
 * @param [out] RxBuff: The rx data buffer pointer
 * @param [in] RxSize: The rx data buffer bytes size
 * @param [in] TimeoutUs: Maximum time in microseconds to wait for the reception to complete
 * @return Hal_StatusType: The receive status
 */
Hal_StatusType Uart_Hal_ReceiveDataBlocking(uint8 Instance, uint8 *RxBuff, uint32 RxSize, uint32 TimeoutUs)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/
    uint32 Counter;
    uint32 Ticks;
    uint32 Elapse = 0U;
    boolean IsTimeOut = FALSE;
    Hal_StatusType RetVal = STATUS_SUCCESS;

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Indicates this is a blocking transaction. */
    ChannelState->IsRxBlocking = TRUE;
    ChannelState->RxComplete = FALSE;

    DEVICE_ASSERT((UART_USING_INTERRUPTS == ChannelState->TransferType) \
                  || (UART_USING_DMA == ChannelState->TransferType));

    if (UART_USING_INTERRUPTS == ChannelState->TransferType)
    {
        /* Start the reception process using interrupts */
        RetVal = Uart_Hal_StartReceiveDataUsingInt(Instance, RxBuff, RxSize);
    }
    else
    {
        /* Start the reception process using DMA */
        RetVal = Uart_Hal_StartReceiveDataUsingDma(Instance, RxBuff, RxSize);
    }

    if (STATUS_SUCCESS == RetVal)
    {
        /* Wait until the receive is complete. */
        Uart_Hal_StartTimeout(&Counter, &Ticks, TimeoutUs);

        while ((FALSE == ChannelState->RxComplete) && (FALSE == IsTimeOut))
        {
            IsTimeOut = Uart_Hal_CheckTimeout(&Counter, &Elapse, Ticks);
        }

        /* Finish the reception if timeout expired */
        if (TRUE == IsTimeOut)
        {
            ChannelState->IsRxBlocking = FALSE;
            ChannelState->ReceiveStatus = STATUS_TIMEOUT;

            if (UART_USING_INTERRUPTS == ChannelState->TransferType)
            {
                Uart_Hal_CompleteReceiveDataUsingInt(Instance);
            }
            else
            {
                Uart_Hal_StopRxDma(Instance);
            }
        }
    }

    return ChannelState->ReceiveStatus;
}
#endif

/*!
 * @brief  Get receive status.
 * @note Function ID: DES_UART_API_009
 * @param [in] Instance: UART hardware channel ID
 * @param [out] BytesRemaining: Remaining bytes to receive
 * @return Hal_StatusType: Receive status
 */
Hal_StatusType Uart_Hal_GetReceiveStatus(uint8 Instance, uint32 *BytesRemaining)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    const Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    if (BytesRemaining != NULL_PTR)
    {
        if (TRUE == ChannelState->IsRxBusy)
        {
            /* Fill in the bytes transferred. */
            if (UART_USING_INTERRUPTS == ChannelState->TransferType)
            {
                /* In interrupt-based communication, the remaining bytes are retrieved
                   from the state structure */
                *BytesRemaining = ChannelState->RxSize;
            }
            else
            {
                /* In DMA communication, the remaining bytes are retrieved
                   from the current DMA channel */
                *BytesRemaining = ChannelState->RxSize - Dma_Hal_GetTransBytes(ChannelState->RxDmaChannel);
            }
        }
        else
        {
            *BytesRemaining = 0;
        }
    }

    return ChannelState->ReceiveStatus;
}

/*!
 * @brief  Terminates a non-blocking receive early.
 * @note Function ID: DES_UART_API_011
 * @param [in] Instance: UART hardware channel ID
 * @return Hal_StatusType. if successful return STATUS_SUCCESS
 */
Hal_StatusType Uart_Hal_AbortReceivingData(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check if a transfer is running. */
    if (TRUE == ChannelState->IsRxBusy)
    {
        /* Update the rx status */
        ChannelState->ReceiveStatus = STATUS_UART_ABORTED;

        /* Stop the running transfer. */
        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            Uart_Hal_CompleteReceiveDataUsingInt(Instance);
        }
        else
        {
            Uart_Hal_StopRxDma(Instance);
        }
    }

    return STATUS_SUCCESS;
}

/*PRQA S 1505 ++ */ /* external function in sdk project */
/**
 * @brief  Set UART baud rate.
 * @note Function ID: DES_UART_API_012
 * @param [in] Instance: UART hardware channel ID
 * @param [in] DesiredBaudRate: The desired baudrate to be set
 * @param [in] SampleCnt: The sample count value to be used
 * @return Hal_StatusType: The status of the operation STATUS_SUCCESS or STATUS_BUSY
 */
Hal_StatusType Uart_Hal_SetBaudRate(uint8 Instance, uint32 DesiredBaudRate, Uart_SampleCntType SampleCnt)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(DesiredBaudRate > 0U);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    uint32 SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
    uint32 UartSourceClock;
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    float32 Divisor;
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];
    Hal_StatusType RetVal = STATUS_SUCCESS;/* function return value */

    /* Check if there is an ongoing transfer */
    if ((ChannelState->IsTxBusy == TRUE) || (ChannelState->IsRxBusy == TRUE))
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        (void)Ckgen_Hal_GetFreq(Uart_HalClock[Instance], &UartSourceClock);
        /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

        /* Check if current instance is clock gated off. */
        DEVICE_ASSERT(UartSourceClock > 0U);

        /* Determine sampleCntValue based on sampleCntReg */
        switch (SampleCnt)
        {
        case UART_SMP_CNT4:
            SampleCntValue = UART_SAMPLE_CNT_4_VALUE;
            break;
        case UART_SMP_CNT8:
            SampleCntValue = UART_SAMPLE_CNT_8_VALUE;
            break;
        case UART_SMP_CNT16:
            SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
            break;
        default:
            SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
            break;
        }

        /* Check if the desired baud rate can be configured with the current protocol clock. */
        DEVICE_ASSERT(UartSourceClock >= (DesiredBaudRate * SampleCntValue));

        /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        Divisor = ((float32)UartSourceClock / (float32)(DesiredBaudRate * SampleCntValue));
        /*PRQA S 4394 -- */

        /* program the sampleCntReg value */
        Uart_Reg_SetSampleCounter(BaseAddress, (uint32)SampleCnt);

        /* write the divisor value */
        Uart_Reg_SetBaudRateDivisor(BaseAddress, Divisor);
    }

    return RetVal;
}
/*PRQA S 1505 -- */ /* external function in sdk project */

#ifndef UART_SDK_NON_EXTENDED_API
/*!
 * @brief  Get UART baudrate.
 * @note Function ID: DES_UART_API_013
 * @param [in] Instance: UART hardware channel ID
 * @param [out] ConfiguredBaudRate: Return the UART configured baudrate
 * @return void
 */
void Uart_Hal_GetBaudRate(uint8 Instance, uint32 *ConfiguredBaudRate)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(ConfiguredBaudRate != NULL_PTR);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_SampleCntType SampleCntReg = UART_SMP_CNT4;
    uint32 SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
    float32 Divisor = 0.0F;
    uint32 UartSourceClock = 0U;
    const UART_Type *BaseAddress = Uart_HalBase[Instance];

    /* Get the UART clock */
    (void)Ckgen_Hal_GetFreq(Uart_HalClock[Instance], &UartSourceClock);
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /*PRQA S 4342 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    SampleCntReg = (Uart_SampleCntType)Uart_Reg_GetSampleCounter(BaseAddress);
    /*PRQA S 4342 -- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    /* Determine sampleCntValue based on sampleCntReg */
    switch (SampleCntReg)
    {
    case UART_SMP_CNT4:
        SampleCntValue = UART_SAMPLE_CNT_4_VALUE;
        break;
    case UART_SMP_CNT8:
        SampleCntValue = UART_SAMPLE_CNT_8_VALUE;
        break;
    case UART_SMP_CNT16:
        SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
        break;
    default:
        SampleCntValue = UART_SAMPLE_CNT_4_VALUE;
        break;
    }
    Divisor = Uart_Reg_GetBaudRateDivisor(BaseAddress);

    if (ConfiguredBaudRate != NULL_PTR)
    {
        /*PRQA S 4395 ++ # allow float types to be converted to unsigned.*/
        *ConfiguredBaudRate = (uint32)((float32)UartSourceClock / ((float32)SampleCntValue * Divisor));
        /*PRQA S 4395 -- */
    }
}
#endif

/**
 * @brief  Enable or disable the UART idle interrupt.
 * @note Function ID: DES_UART_API_019
 * @param [in] Instance: UART hardware channel ID
 * @param [in] IsEnable: Use TRUE to enable and FALSE to disable the idle interrupt
 * @return void
 */
void Uart_Hal_SetIdleInterrupt(uint8 Instance, boolean IsEnable)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Uart_Reg_SetIdleInterrupt(BaseAddress, IsEnable);
}

#ifndef UART_SDK_NON_EXTENDED_API
/**
 * @brief  Configure data match interrupt.
 * @note Function ID: DES_UART_API_016
 * @param [in] Instance: UART hardware channel ID
 * @param [in] Data: The data value to match against received data
 * @param [in] IsEnable: Use TRUE to enable and FALSE to disable data match
 * @return void
 */
void Uart_Hal_SetDataMatch(uint8 Instance, uint16 Data, boolean IsEnable)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Uart_Reg_SetDataMatch(BaseAddress, Data, IsEnable);
}

/**
 * @brief  Configure address filter.
 * @note Function ID: DES_UART_API_015
 * @param [in] Instance: UART hardware channel ID
 * @param [in] Addr: The address value to be used for filtering incoming data
 * @param [in] IsEnable: Use TRUE to enable and FALSE to disable address filter
 * @return void
 */
void Uart_Hal_SetAddrFilter(uint8 Instance, uint16 Addr, boolean IsEnable)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Uart_Reg_SetAddrFilter(BaseAddress, Addr, IsEnable);
}

/**
 * @brief  Enable or disable the UART match interrupt.
 * @note Function ID: DES_UART_API_020
 * @param [in] Instance: UART hardware channel ID
 * @param [in] IsEnable: Use TRUE to enable and FALSE to disable match interrupt
 * @return void
 */
void Uart_Hal_SetMatchInterrupt(uint8 Instance, boolean IsEnable)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Uart_Reg_SetIntMode(BaseAddress, UART_INT_DATA_MATCH, IsEnable);
}

/**
 * @brief  Configure the RTS/CTS flow control.
 * @note Function ID: DES_UART_API_014
 * @param [in] Instance: UART hardware channel ID
 * @param [in] RtsCts: Configuration for RTS/CTS flow control
 * @return void
 */
void Uart_Hal_SetCtsRts(uint8 Instance, Uart_RtsCtsType RtsCts)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Uart_Reg_SetCtsRts(BaseAddress, RtsCts);
}

/**
 * @brief  Configure RS485 settings.
 * @note Function ID: DES_UART_API_018
 * @param [in] Instance: UART hardware channel ID
 * @param [in] Config: Config Pointer to the RS485 configuration structure
 * @return void
 */
void Uart_Hal_SetRS485(uint8 Instance, const Uart_RS485ConfigType *Config)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Uart_Reg_SetRS485(BaseAddress, Config->Enable);
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
    Uart_Reg_SetRS485Invpol(BaseAddress, Config->InvpolEnable);
    Uart_Reg_SetRS485Delay(BaseAddress, Config->DelayEnable);
    Uart_Reg_SetRS485DelayCnt(BaseAddress, Config->DelayCnt);
    Uart_Reg_SetRS485GuardTime(BaseAddress, Config->GuardTime);
    Uart_Reg_SetRS485GuardEnable(BaseAddress, (Config->GuardTime > 0U) ? TRUE : FALSE);
}

/**
 * @brief  Configure IrDA settings.
 * @note Function ID: DES_UART_API_017
 * @param [in] Instance: UART hardware channel ID
 * @param [in] Config: Config Pointer to the IrDA configuration structure
 * @return void
 */
void Uart_Hal_SetIrDA(uint8 Instance, const Uart_IrDAConfigType *Config)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    Uart_Reg_SetIrDA(BaseAddress, Config->Enable);
    Uart_Reg_SetIrDARxWidth(BaseAddress, Config->RxWidth);
    Uart_Reg_SetIrDATxWidth(BaseAddress, Config->TxWidth);

    if (TRUE == Config->Enable)
    {
        if (TRUE == Config->TxMode)
        {
            /* Enable the tranceiver */
            Uart_Reg_SetTransmitterCmd(BaseAddress, TRUE);
            /* Disable the receiver */
            Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
        }
        else
        {
            /* Disable the tranceiver */
            Uart_Reg_SetTransmitterCmd(BaseAddress, FALSE);
            /* Enable the receiver */
            Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);
        }
    }
    else
    {
        /* Enable the tranceiver */
        Uart_Reg_SetTransmitterCmd(BaseAddress, TRUE);
        /* Enable the receiver */
        Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);
    }
}
#endif

#if (CONFIG_UART0_ENABLE)
/**
 * @brief  UART0 Interrupt Handler Function
 * @note Function ID: DES_UART_API_021
 * @return void
 */
ISR(UART0_IRQHandler)
{
    Uart_Hal_IRQHandler(0U);
}
#endif

#if (CONFIG_UART1_ENABLE)
/**
 * @brief  UART1 Interrupt Handler Function
 * @note Function ID: DES_UART_API_022
 * @return void
 */
ISR(UART1_IRQHandler)
{
    Uart_Hal_IRQHandler(1U);
}
#endif

#if (CONFIG_UART2_ENABLE)

/**
 * @brief  UART2 Interrupt Handler Function
 * @note Function ID: DES_UART_API_023
 * @return void
 */
ISR(UART2_IRQHandler)
{
    Uart_Hal_IRQHandler(2U);
}
#endif

#if (CONFIG_UART3_ENABLE)
/**
 * @brief  UART3 Interrupt Handler Function
 * @note Function ID: DES_UART_API_024
 * @return void
 */
ISR(UART3_IRQHandler)
{
    Uart_Hal_IRQHandler(3U);
}
#endif

#if defined (AC7843X)
#if (CONFIG_UART4_ENABLE)
/**
 * @brief  UART4 Interrupt Handler Function
 * @note Function ID: DES_UART_API_025
 * @return void
 */
ISR(UART4_IRQHandler)
{
    Uart_Hal_IRQHandler(4U);
}
#endif

#if (CONFIG_UART5_ENABLE)
/**
 * @brief  UART5 Interrupt Handler Function
 * @note Function ID: DES_UART_API_026
 * @return void
 */
ISR(UART5_IRQHandler)
{
    Uart_Hal_IRQHandler(5U);
}
#endif

#if (CONFIG_UART6_ENABLE)

/**
 * @brief  UART6 Interrupt Handler Function
 * @note Function ID: DES_UART_API_027
 * @return void
 */
ISR(UART6_IRQHandler)
{
    Uart_Hal_IRQHandler(6U);
}
#endif

#if (CONFIG_UART7_ENABLE)
/**
 * @brief  UART7 Interrupt Handler Function
 * @note Function ID: DES_UART_API_028
 * @return void
 */
ISR(UART7_IRQHandler)
{
    Uart_Hal_IRQHandler(7U);
}
#endif
#endif
/*================================STATIC  FUNCTION  IMPLEMENTATIONS=================================*/
#ifndef UART_SDK_NON_EXTENDED_API
LOCAL_INLINE void Uart_Hal_StartTimeout(uint32 *CounterOut, uint32 *TicksOut, uint32 Timeout)
{
    *TicksOut = OsIf_MicrosToTicks(Timeout);
    *CounterOut = OsIf_GetCounter();
}

LOCAL_INLINE boolean Uart_Hal_CheckTimeout(uint32 *Counter, uint32 *ElapsedTicks, uint32 TimeoutTicks)
{
    uint32 CurrentElapsedTicks = OsIf_GetElapsed(Counter);
    *ElapsedTicks += CurrentElapsedTicks;
    return ((*ElapsedTicks >= TimeoutTicks) ? TRUE : FALSE);
}
#endif

static Hal_StatusType Uart_Hal_StartSendDataUsingInt(uint8 Instance, const uint8 *TxBuffer, uint32 TxSize)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    DEVICE_ASSERT((ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR) \
                  || (0U == (TxSize & 1U)));

    ChannelState->TxBuff = TxBuffer;
    ChannelState->TxSize = TxSize;
    ChannelState->SendStatus = STATUS_BUSY;

    /* Enable tx not full interrupt */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_NOT_FULL, TRUE);

    return STATUS_SUCCESS;
}

static void Uart_Hal_StopTxDma(uint8 Instance)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    /* Disable tx DMA requests for the current instance */
    Uart_Reg_SetTxDmaCmd(BaseAddress, FALSE);
    /* Stop the dma channel */
    (void)Dma_Hal_StopCh(ChannelState->TxDmaChannel);
    /* Disable transmit complete interrupt */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_COMPLETE, FALSE);
    /* current transmission status is STATUS_BUSY */
    if (STATUS_BUSY == ChannelState->SendStatus)
    {
        /* If the transfer is completed, update the transmit status */
        ChannelState->SendStatus = STATUS_SUCCESS;
    }
    /* Update the internal busy flag */
    ChannelState->IsTxBusy = FALSE;
    ChannelState->TxSize = 0U;

    if (TRUE == ChannelState->IsTxBlocking) /* Mark the transmission as complete */
    {
        ChannelState->TxComplete = TRUE;
    }
}

static void Uart_Hal_CompleteSendDataUsingInt(uint8 Instance)
{
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    UART_Type *BaseAddress = Uart_HalBase[Instance];

    if (STATUS_BUSY == ChannelState->SendStatus)
    {
        /* If the transfer is completed, update the transmit status */
        ChannelState->SendStatus = STATUS_SUCCESS;
    }
    else
    {
        /* If the transfer is aborted or timed out, disable tx not full interrupt */
        Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_NOT_FULL, FALSE);
    }
    /* Disable transmit complete interrupt */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_COMPLETE, FALSE);

    /* Update the internal busy flag */
    ChannelState->IsTxBusy = FALSE;

    if (TRUE == ChannelState->IsTxBlocking) /* Mark the transmission as complete */
    {
        ChannelState->TxComplete = TRUE;
    }
}

static void Uart_Hal_TxCompleteIrqHandler(uint8 Instance)
{
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];

    if (0U == ChannelState->TxSize)/* current transmission size is 0, means transaction complete */
    {
        if (UART_USING_INTERRUPTS == ChannelState->TransferType)/* Complete the interrupt based transfer */
        {
            Uart_Hal_CompleteSendDataUsingInt(Instance);
        }
        else/* Complete the DMA based transfer */
        {
            Uart_Hal_StopTxDma(Instance);
        }
        if (NULL_PTR != ChannelState->TxCallback)/* Invoke callback if there is one */
        {
            ChannelState->TxCallback(Instance, UART_EVENT_END_TRANSFER);
        }
    }
}

/*PRQA S 3673 ++ # unmodified pointer types are allowed in interrupt functions without const modification.*/
static void Uart_Hal_TxDmaCallback(void *Param)
/*PRQA S 3673 -- # unmodified pointer types are allowed in interrupt functions without const modification.*/
{
    /*PRQA S 0316 ++ # interrupt functions allow conversion of a pointer to void to a pointer to an object.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    uint32 Instance = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    if (DMA_ERROR_EVENT == (DmaInfo->DmaEvent & DMA_ERROR_EVENT))/* Check if the DMA transfer completed with errors */
    {
        /* Update the status */
        ChannelState->SendStatus = STATUS_ERROR;
        /* Stop the transfer */
        Uart_Hal_StopTxDma((uint8)Instance);
        /* Notify the application that an error occurred */
        if (ChannelState->TxCallback != NULL_PTR)
        {
            ChannelState->TxCallback((uint8)Instance, UART_EVENT_ERROR);
        }
    }
    else if (DMA_FINISH_EVENT == (DmaInfo->DmaEvent & DMA_FINISH_EVENT))/* DMA transfer completed without errors */
    {
        ChannelState->TxSize = 0U;

        /* Invoke callback if there is one */
        if (ChannelState->TxCallback != NULL_PTR)
        {
            /* Allow the user to provide a new buffer, for continuous transmission */
            ChannelState->TxCallback((uint8)Instance, UART_EVENT_TX_EMPTY);
        }
        /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
        /* Enable the TX COMPLETE interrupt to check end of transfer */
        Uart_Reg_SetIntMode(Uart_HalBase[Instance], UART_INT_TX_COMPLETE, TRUE);
        /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/
    }
    else
    {
        /* do nothing */
    }
}

static Hal_StatusType Uart_Hal_StartSendDataUsingDma(uint8 Instance, const uint8 *TxBuffer, uint32 TxSize)
{
    DEVICE_ASSERT(TxSize < UART_USE_DMA_TRANSMIT_LEN_MAX);

    Hal_StatusType DmaRetValue;/* config dma status */
    Hal_StatusType RetValue;/* function return value */
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    Dma_TransferUnitType DmaTransferSize;
    Dma_TransferConfigType TransferConfig;
    uint32 UartInstance;

    /* Check it's not busy transmitting data from a previous function call */
    /* Enable tx DMA requests */
    Uart_Reg_SetTxDmaCmd(BaseAddress, TRUE);

    /* Update state structure */
    ChannelState->TxBuff = TxBuffer;
    ChannelState->TxSize = TxSize;
    ChannelState->IsTxBusy = TRUE;
    ChannelState->SendStatus = STATUS_BUSY;

    /* Set DMA channel transfer size is DMA_TRANSFER_UNIT_2B*/
    if (UART_9_BITS_PER_CHAR == ChannelState->BitCountPerChar)
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_2B;
    }
    else/* Set DMA channel transfer size is DMA_TRANSFER_UNIT_1B*/
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_1B;
    }

    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = FALSE;
    TransferConfig.SrcUnit = DmaTransferSize;
    TransferConfig.DestUnit = DmaTransferSize;
    TransferConfig.Type = DMA_TRANSFER_MEM2PERIPH;
    TransferConfig.SrcStartAddr = (uint32)TxBuffer;
    TransferConfig.DestStartAddr = (uint32)(&(BaseAddress->RBR));
    TransferConfig.SrcOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
    TransferConfig.DestOffset = 0U;
    TransferConfig.Length = (uint16)TxSize;
    TransferConfig.Callback = (Hal_CallbackType)Uart_Hal_TxDmaCallback;
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    UartInstance = Instance;
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    TransferConfig.UserArgs = (void *)(UartInstance);
    /*PRQA S 0326 -- */
    /*PRQA S 2985 ++ # considered an invalid operation, it is actually meaningful.*/
    TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + \
                                (TransferConfig.Length / (1UL << (uint8)TransferConfig.SrcUnit) * \
                                 TransferConfig.SrcOffset);
    TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + \
                                 (TransferConfig.Length / (1UL << (uint8)TransferConfig.DestUnit) * \
                                  TransferConfig.DestOffset);
    /*PRQA S 2985 -- # considered an invalid operation, it is actually meaningful.*/
    /* Configure the DMA transfer control */
    DmaRetValue = Dma_Hal_ConfigCh(ChannelState->TxDmaChannel, &TransferConfig);
    if (STATUS_SUCCESS == DmaRetValue)/* config dma success */
    {
        /* Start the DMA channel */
        (void)Dma_Hal_StartCh(ChannelState->TxDmaChannel);

        /* Enable the UART transmitter */
        Uart_Reg_SetTransmitterCmd(BaseAddress, TRUE);
        RetValue = STATUS_SUCCESS;
    }
    else/* config dma fail */
    {
        RetValue = STATUS_ERROR;
    }

    return RetValue;
}

static Hal_StatusType Uart_Hal_StartReceiveDataUsingInt
(
    uint8 Instance,
    uint8 *RxBuffer,
    uint32 RxSize
)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    DEVICE_ASSERT((ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR) \
                  || (0U == (RxSize & 1U)));

    /* Initialize the module driver state struct to indicate transfer in progress
    * and with the buffer and byte count data. */
    ChannelState->RxBuff = RxBuffer;
    ChannelState->RxSize = RxSize;
    ChannelState->ReceiveStatus = STATUS_BUSY;
    /* Enable the receiver */
    Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);
    /* Enable error interrupts TRUE */
    Uart_Reg_SetErrorInterrupts(BaseAddress, TRUE);
    /* Enable receive data not empty interrupt */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_RX_NOT_EMPTY, TRUE);

    return STATUS_SUCCESS;
}

static void Uart_Hal_StopRxDma
(
    uint8 Instance
)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    uint8 TmpByte = 0U;

    /* Disable receiver */
    Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
    /* Disable error interrupts */
    Uart_Reg_SetErrorInterrupts(BaseAddress, FALSE);
    /* Disable rx DMA requests */
    Uart_Reg_SetRxDmaCmd(BaseAddress, FALSE);
    /* Read dummy to clear RX data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Read dummy to clear RX data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Stop the DMA channel */
    (void)Dma_Hal_StopCh(ChannelState->RxDmaChannel);

    /* Signal the synchronous completion object. */
    if (TRUE == ChannelState->IsRxBlocking)/* modify variable IsRxBlocking */
    {
        ChannelState->RxComplete = TRUE;
        ChannelState->IsRxBlocking = FALSE;
    }

    /* If status is STATUS_BUSY, update the internal driver status */
    if (STATUS_BUSY == ChannelState->ReceiveStatus)
    {
        ChannelState->ReceiveStatus = STATUS_SUCCESS;
    }

    /* Update the information of the module driver state */
    ChannelState->IsRxBusy = FALSE;
    ChannelState->RxSize = 0U;
}

/*PRQA S 3673 ++ # unmodified pointer types are allowed in interrupt functions without const modification.*/
static void Uart_Hal_RxDmaCallback(void *Param)
/*PRQA S 3673 -- # unmodified pointer types are allowed in interrupt functions without const modification.*/
{
    /*PRQA S 0316 ++ # interrupt functions allow conversion of a pointer to void to a pointer to an object.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    uint32 Instance = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    if (DMA_ERROR_EVENT == (DmaInfo->DmaEvent & DMA_ERROR_EVENT))/* current status is error */
    {
        /* Update the status */
        ChannelState->ReceiveStatus = STATUS_ERROR;
        /* Stop the transfer */
        Uart_Hal_StopRxDma((uint8)Instance);
        /* Notify the application that an error occurred */
        if (NULL_PTR != ChannelState->RxCallback)/* RxCallback not null pointer, callback to user */
        {
            ChannelState->RxCallback((uint8)Instance, UART_EVENT_ERROR);
        }
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress);
    }

    /* Return if an error occurred; error cases are treated by the interrupt handler */
    if ((DMA_FINISH_EVENT == (DmaInfo->DmaEvent & DMA_FINISH_EVENT)) && (ChannelState->ReceiveStatus == STATUS_BUSY))
    {
        /* Stop the reception */
        Uart_Hal_StopRxDma((uint8)Instance);
        /* Invoke the callback to notify the end of the transfer */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback((uint8)Instance, UART_EVENT_END_TRANSFER);
        }
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress);
    }
    else/* ReceiveStatus not STATUS_BUSY */
    {
        /* do nothing */
    }

    return;
}

static Hal_StatusType Uart_Hal_StartReceiveDataUsingDma(uint8 Instance, uint8 *RxBuffer, uint32 RxSize)
{
    DEVICE_ASSERT(RxSize < UART_USE_DMA_TRANSMIT_LEN_MAX);

    Hal_StatusType RetValue;
    Hal_StatusType DmaRetValue;
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    Dma_TransferUnitType DmaTransferSize;
    uint8 TmpByte = 0U;
    uint32 UartInstance;

    /* Enable rx DMA requests for the current instance */
    Uart_Reg_SetRxDmaCmd(BaseAddress, TRUE);
    /* Read dummy to clear RX data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Read dummy to clear RX data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Clear the flags */
    Uart_Reg_ClearErrorFlags(BaseAddress);
    /* Set DMA channel transfer size */
    if (UART_9_BITS_PER_CHAR == ChannelState->BitCountPerChar)
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_2B;
    }
    else
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_1B;
    }
    Dma_TransferConfigType TransferConfig;
    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = FALSE;
    TransferConfig.SrcUnit = DmaTransferSize;
    TransferConfig.DestUnit = DmaTransferSize;
    TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
    TransferConfig.SrcStartAddr = (uint32)(&(BaseAddress->RBR));
    TransferConfig.DestStartAddr = (uint32)RxBuffer;
    TransferConfig.SrcOffset = 0U;
    TransferConfig.DestOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
    TransferConfig.Length = (uint16)RxSize;
    TransferConfig.Callback = (Hal_CallbackType)Uart_Hal_RxDmaCallback;
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    UartInstance = Instance;
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    TransferConfig.UserArgs = (void *)(UartInstance);
    /*PRQA S 0326 -- */
    /*PRQA S 2985 ++ # considered an invalid operation, it is actually meaningful.*/
    TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + \
                                (TransferConfig.Length / (1UL << (uint8)TransferConfig.SrcUnit) * \
                                 TransferConfig.SrcOffset);
    TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + \
                                 (TransferConfig.Length / (1UL << (uint8)TransferConfig.DestUnit) * \
                                  TransferConfig.DestOffset);
    /*PRQA S 2985 -- # considered an invalid operation, it is actually meaningful.*/
    /* Configure the DMA transfer control */
    DmaRetValue = Dma_Hal_ConfigCh(ChannelState->RxDmaChannel, &TransferConfig);
    if (STATUS_SUCCESS == DmaRetValue)/* config dma channel is STATUS_SUCCESS */
    {
        /* Start the DMA channel */
        (void)Dma_Hal_StartCh(ChannelState->RxDmaChannel);
        /* Update the state structure */
        ChannelState->RxBuff = RxBuffer;
        ChannelState->RxSize = RxSize;
        ChannelState->ReceiveStatus = STATUS_BUSY;

        /* Enable error interrupts */
        Uart_Reg_SetErrorInterrupts(BaseAddress, TRUE);
        /* Enable the receiver */
        Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);
        RetValue = STATUS_SUCCESS;
    }
    else/* config dma channel fail */
    {
        RetValue = STATUS_ERROR;
    }

    return RetValue;
}

static void Uart_Hal_CompleteReceiveDataUsingInt(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    uint8 TmpByte = 0U;
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /* Disable receiver */
    Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
    /* Disable error interrupts */
    Uart_Reg_SetErrorInterrupts(BaseAddress, FALSE);
    /* Read dummy to clear rx data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Disable receive data not full interrupt. */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_RX_NOT_EMPTY, FALSE);
    if (TRUE == ChannelState->IsRxBlocking)/* Signal the synchronous completion object. */
    {
        ChannelState->RxComplete = TRUE;
        ChannelState->IsRxBlocking = FALSE;
    }
    /* Update the information of the module driver state */
    ChannelState->IsRxBusy = FALSE;
    if (STATUS_BUSY == ChannelState->ReceiveStatus)
    {
        ChannelState->ReceiveStatus = STATUS_SUCCESS;
    }
}

static void Uart_Hal_ErrIrqHandler(uint8 Instance)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    boolean IsError = FALSE;

    /* Handle receive overrun interrupt */
    if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_RX_OVERRUN))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_RX_OVERRUN;
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress);
        IsError = TRUE;
    }

    /* Handle parity error interrupt */
    if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_PARITY_ERR))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_PARITY_ERROR;
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress);
        IsError = TRUE;
    }

    /* Handle framing error interrupt */
    if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_FRAME_ERR))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_FRAMING_ERROR;
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress);
        IsError = TRUE;
    }

    /* Handle break error interrupt */
    if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_BREAK_ERR))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_BREAK_ERROR;
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress);
        IsError = TRUE;
    }

    /* Handle noise error interrupt */
    if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_NOISE_ERR))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_NOISE_ERROR;
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress);
        IsError = TRUE;
    }

    if (TRUE == IsError)
    {
        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            /* Complete transfer (disable rx logic) */
            Uart_Hal_CompleteReceiveDataUsingInt(Instance);
        }
        else
        {
            /* Complete the transfer (stop DMA channel) */
            Uart_Hal_StopRxDma(Instance);
        }

        /* Invoke callback if there is one */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback(Instance, UART_EVENT_ERROR);
        }
    }
}

static void Uart_Hal_GetData(uint8 Instance)
{
    UART_Type const *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];
    uint16 Data = 0U;
    uint8 *RxBuff = ChannelState->RxBuff;

    if ((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar)
            || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar)) /* 7-bits or 8-bits */
    {
        /* Receive the data */
        Uart_Reg_GetChar(BaseAddress, RxBuff);
    }
    else /* 9-bits */
    {
        /* Receive the data */
        Uart_Reg_GetChar9(BaseAddress, &Data);

        /* Write the least significant bits to the receive buffer */
        *RxBuff = (uint8)(Data & 0xFFU);
        ++RxBuff;
        /* Write the ninth bit to the subsequent byte in the rx buffer */
        *RxBuff = (uint8)(Data >> 8U);
    }
}

static void Uart_Hal_RxIrqHandler(uint8 Instance)
{
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    /* Get data and put in receive buffer  */
    Uart_Hal_GetData(Instance);

    /* Update the internal state */
    if (ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR)
    {
        ++ChannelState->RxBuff;
        --ChannelState->RxSize;
    }
    else
    {
        ChannelState->RxBuff = &ChannelState->RxBuff[2];
        ChannelState->RxSize -= 2U;
    }

    /* Check if this was the last byte in the current buffer */
    if (0U == ChannelState->RxSize)
    {
        /* Invoke callback if there is one (callback may reset the rx buffer
           for continuous reception) */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback(Instance, UART_EVENT_RX_FULL);
        }
    }

    /* Finish reception if this was the last byte received */
    if (0U == ChannelState->RxSize)
    {
        /* Complete transfer (disable rx logic) */
        Uart_Hal_CompleteReceiveDataUsingInt(Instance);

        /* Invoke callback if there is one */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback(Instance, UART_EVENT_END_TRANSFER);
        }
    }
}

static void Uart_Hal_PutData(uint8 Instance)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];
    const uint8 *TxBuff = ChannelState->TxBuff;
    uint16 Data;

    if ((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar)
            || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar)) /* 7-bits or 8-bits */
    {
        /* Transmit the data */
        Uart_Reg_PutChar(BaseAddress, *TxBuff);
    }
    else /* 9-bit per char */
    {
        /* Create a 16-bits integer from two bytes */
        Data = (uint16)(*TxBuff);
        ++TxBuff;
        Data |= (uint16)(((uint16)(*TxBuff)) << 8U);

        /* Transmit the data */
        Uart_Reg_PutChar9(BaseAddress, Data);
    }
}

static void Uart_Hal_TxIrqHandler(uint8 Instance)
{
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    boolean IsLastByte = FALSE;

    /* Check if there are any more bytes to send */
    if (ChannelState->TxSize > 0U)
    {
        /* Check if it is the last character */
        if (ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR)
        {
            if (1U == ChannelState->TxSize)
            {
                IsLastByte = TRUE;
            }
        }
        else
        {
            if (2U == ChannelState->TxSize)
            {
                IsLastByte = TRUE;
            }
        }

        /* When TC interrupt is enabled, suspend all interrupts */
        if (TRUE == IsLastByte)
        {
            OSIF_ENTER_CRITICAL(UART_HAL_ID1);
        }
        /* Transmit the data */
        Uart_Hal_PutData(Instance);

        /* enable tx complete interrupt */
        if (TRUE == IsLastByte)
        {
            Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_COMPLETE, TRUE);
            OSIF_EXIT_CRITICAL(UART_HAL_ID1);
        }

        /* Update the internal state */
        if (ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR)
        {
            ++ChannelState->TxBuff;
            --ChannelState->TxSize;
        }
        else
        {
            ChannelState->TxBuff = &ChannelState->TxBuff[2];
            ChannelState->TxSize -= 2U;
        }

        /* Check if this was the last byte in the current buffer */
        if (0U == ChannelState->TxSize)
        {
            /* Invoke callback if there is one (callback may reset the tx buffer
               for continuous transmission)*/
            if (NULL_PTR != ChannelState->TxCallback)
            {
                ChannelState->TxCallback(Instance, UART_EVENT_TX_EMPTY);
            }

            /* If there's no new data, disable tx not full interrupt */
            Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_NOT_FULL, FALSE);
        }
    }
}

static void Uart_Hal_StateInit(uint8 Instance, const Uart_ChannelConfigType *ChannelConfigPtr)
{
    /* lin hal channel config */
    Uart_ChannelStateType *StatePtr = Uart_ChannelStateArray[Instance];

    /* Init Uart State */
    StatePtr->IsTxBusy = FALSE;
    StatePtr->IsRxBusy = FALSE;
    StatePtr->IsRxBlocking = FALSE;
    StatePtr->IsTxBlocking = FALSE;
    StatePtr->TxComplete = FALSE;
    StatePtr->RxComplete = FALSE;
    StatePtr->TxSize = 0U;
    StatePtr->RxSize = 0U;
    /* Save the transfer information for runtime retrieval */
    StatePtr->TransferType = ChannelConfigPtr->TransferType;
    StatePtr->BitCountPerChar = ChannelConfigPtr->BitCountPerChar;

    StatePtr->RxDmaChannel = ChannelConfigPtr->RxDmaChannel;
    StatePtr->TxDmaChannel = ChannelConfigPtr->TxDmaChannel;

    /* initialize SendStatus and receiveStatus */
    StatePtr->SendStatus = STATUS_IDLE;
    StatePtr->ReceiveStatus = STATUS_IDLE;

    /* Set the Callback from config struct */
    StatePtr->RxCallback = ChannelConfigPtr->RxCallback;
    StatePtr->TxCallback = ChannelConfigPtr->TxCallback;
}

static void Uart_Hal_IRQHandler(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # the upper layer call guarantees that there will never be an array out of bounds.*/

    /* Handle error interrupt */
    if (TRUE == Uart_Reg_IsErrorInterruptEnable(BaseAddress))
    {
        Uart_Hal_ErrIrqHandler(Instance);
    }

    /* Handle receive not empty interrupt */
    if (TRUE == Uart_Reg_GetIntMode(BaseAddress, UART_INT_RX_NOT_EMPTY))
    {
        if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_RX_DATA_READY))
        {
            Uart_Hal_RxIrqHandler(Instance);
        }
    }

    /* Handle transmit not full interrupt */
    if (TRUE == Uart_Reg_GetIntMode(BaseAddress, UART_INT_TX_NOT_FULL))
    {
        if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_TX_DATA_NOT_FULL))
        {
            Uart_Hal_TxIrqHandler(Instance);
        }
    }

    /* Handle transmit complete interrupt */
    if (TRUE == Uart_Reg_GetIntMode(BaseAddress, UART_INT_TX_COMPLETE))
    {
        if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_TX_COMPLETE))
        {
            Uart_Hal_TxCompleteIrqHandler(Instance);
        }
    }

    /* Handle receive data match interrupt */
    if (TRUE == Uart_Reg_GetIntMode(BaseAddress, UART_INT_DATA_MATCH))
    {
        if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_DATA_MATCH))
        {
            Uart_Reg_ClearStatusFlag(BaseAddress, UART_DATA_MATCH);
            /* Invoke callback if there is one */
            if (ChannelState->RxCallback != NULL_PTR)
            {
                ChannelState->RxCallback(Instance, UART_EVENT_DATA_MATCH);
            }
        }
    }

    /* Handle idle line interrupt */
    if (TRUE == Uart_Reg_GetIntMode(BaseAddress, UART_INT_IDLE_LINE))
    {
        if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_IDLE_LINE))
        {
            Uart_Reg_ClearStatusFlag(BaseAddress, UART_IDLE_LINE);
            /* Invoke callback if there is one */
            if (ChannelState->RxCallback != NULL_PTR)
            {
                ChannelState->RxCallback(Instance, UART_EVENT_IDLE_LINE);
            }
        }
    }
}

/*============================================EOF===================================================*/
