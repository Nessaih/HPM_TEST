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
 * @file Eio_Uart_Hal.c
 *
 * @brief This file provides eio uart integration functions.
 *
 */

/* PRQA S 0380 EOF */ /* Number of macro definitions exceeds 4095 */
/* PRQA S 0306,0303,0316,3305,0310 EOF */ /* Type conversion. */
/* PRQA S 2889 EOF */ /* More than one 'return'. */
/* PRQA S 3415 EOF */ /* with persistent side effects. */
/* PRQA S 3206 EOF */ /* Parameter not used. */
/* PRQA S 3673 EOF */ /* could be 'pointer to const'. */
/* PRQA S 1503,1505 EOF */ /* Is defined but not used. */

/* ===========================================  Includes  =========================================== */
#include "Eio_Uart_Hal.h"
#include "AC784xx_Eio_Reg.h"
#include "Eio_Common_Hal.h"
#include "Dma_Hal.h"
#include "Ckgen_Hal.h"

/* ============================================  Define  ============================================ */
/* Constraints used for baud rate computation */
#define UART_DIVIDER_MIN_VALUE  (0)
#define UART_DIVIDER_MAX_VALUE  (0xFF)

/* Shifters/Timers used for UART simulation The param[in]eter x represents the
   resourceIndex value for the current driver instance */
#define UART_TX_SHIFTER(x)      (x)
#define UART_RX_SHIFTER(x)      (x)
#define UART_SHIFTER(x)         (x)
#define UART_TX_TIMER(x)        (x)
#define UART_RX_TIMER(x)        (x)
#define UART_TIMER(x)           (x)

/* ===========================================  Typedef  ============================================ */
/*!
 * @brief Driver internal context structure
 */
typedef struct
{
    boolean InitState;                              /*!< Initialization state flag */
    uint8 EioInstance;                              /*!< EIO instance number */
    Eio_CommonStateType TxEioCommon;                /*!< Common eio drivers structure for Tx */
    Eio_CommonStateType RxEioCommon;                /*!< Common eio drivers structure for Rx */
    uint8 *RxData;                                  /*!< Receive buffer pointer */
    const uint8 *TxData;                            /*!< Transmit buffer pointer */
    uint32 TxRemainingBytes;                        /*!< Number of remaining bytes to be transferred */
    uint32 RxRemainingBytes;                        /*!< Number of remaining bytes to be received */
    uint8 TxDmaChannel;                             /*!< DMA channel number for Tx */
    uint8 RxDmaChannel;                             /*!< DMA channel number for Rx */
    Eio_UartCallbackType TxCallback;                /*!< User callback function for Tx events */
    Eio_UartCallbackType RxCallback;                /*!< User callback function for Rx events */
    Eio_UartDriverDirectionType Direction;          /*!< Driver direction: Tx or Rx */
    Eio_DriverType DriverType;                      /*!< Driver type: interrupts/polling/DMA */
    Hal_StatusType TxStatus;                        /*!< Current status of the transmitter */
    Hal_StatusType RxStatus;                        /*!< Current status of the receiver */
    boolean IsTxBusy;                               /*!< Idle/busy state of the transmitter */
    boolean IsRxBusy;                               /*!< Idle/busy state of the receiver */
    uint8 BitCount;                                 /*!< Number of bits per word */
    uint8 TxFlush;                                  /*!< Used for flushing Tx buffer before ending a transmission */
} Eio_UartStateType;
/* ==========================================  Variables  =========================================== */

static Eio_UartStateType EioUartState[EIO_MAX_SHIFTER_COUNT];

/* ====================================  Functions declaration  ===================================== */

/* =====================================  Functions definition  ===================================== */
/**
 * @brief Computes the baud rate divider for a target baud rate
 * @note Function ID: DES_UART_API_311
 * @param [in] BaudRate: The desired baud rate in hertz
 * @param [out] Divider: Pointer to baud rate divider
 * @param [in] InputClock: The frequency of the input clock
 * @return void
 */
static void Eio_Uart_Hal_ComputeBaudRateDivider(uint32 BaudRate, uint16 *Divider, uint32 InputClock)
{
    sint32 TmpDiv;

    /* Compute divider: ((input_clock / baud_rate) / 2) - 1. Round to nearest integer */
    TmpDiv = (((sint32)InputClock + (sint32)BaudRate) / (2 * (sint32)BaudRate)) - 1;
    /* Enforce upper/lower limits */
    if (TmpDiv < UART_DIVIDER_MIN_VALUE)
    {
        TmpDiv = UART_DIVIDER_MIN_VALUE;
    }
    if (TmpDiv > UART_DIVIDER_MAX_VALUE)
    {
        TmpDiv = UART_DIVIDER_MAX_VALUE;
    }

    *Divider = (uint16)TmpDiv;
}

/**
 * @brief Configures the EIO module for UART Tx
 * @note Function ID: DES_UART_API_312
 * @param [in] State: Pointer to the EIO_UART driver context structure
 * @param [in] UserConfigPtr: Pointer to the EIO_UART user configuration structure
 * @param [in] InputClock: The frequency of the input clock
 * @return void
 */
static void Eio_Uart_Hal_ConfigureTx
(
    Eio_UartStateType *State,
    const Eio_UartUserConfigType *UserConfigPtr,
    uint32 InputClock
)
{
    EIO_Type *BaseAddr;
    uint16 Divider;
    uint16 Bits;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    BaseAddr = EioBase[State->TxEioCommon.Instance];
    ResourceIndex = State->TxEioCommon.ResourceIndex;

    /* Compute divider */
    Eio_Uart_Hal_ComputeBaudRateDivider(UserConfigPtr->BaudRate, &Divider, InputClock);
    Bits = UserConfigPtr->BitCount;

    /* Configure tx shifter */
    Eio_Reg_SetShifterConfig(BaseAddr,
                            UART_TX_SHIFTER(ResourceIndex),
                            EIO_SHIFTER_START_BIT_0,
                            EIO_SHIFTER_STOP_BIT_1,
                            EIO_SHIFTER_SOURCE_PIN);
    Eio_Reg_SetShifterControl(BaseAddr,
                             UART_TX_SHIFTER(ResourceIndex),
                             EIO_SHIFTER_MODE_TRANSMIT,
                             UserConfigPtr->TxDataPin,             /* Output on tx pin */
                             EIO_PIN_POLARITY_HIGH,
                             EIO_PIN_CONFIG_OUTPUT,
                             UART_TX_TIMER(ResourceIndex),
                             EIO_TIMER_POLARITY_POSEDGE);

    /* Configure tx timer */
    Eio_Reg_SetTimerCompare(BaseAddr, UART_TX_TIMER(ResourceIndex), \
                           (uint16)((((uint16)(Bits << 1U) - 1U) << 8U) + Divider));

    Eio_Reg_SetTimerConfig(BaseAddr,
                       UART_TX_TIMER(ResourceIndex),
                       EIO_TIMER_START_BIT_ENABLED,
                       EIO_TIMER_STOP_BIT_TIM_DIS,
                       EIO_TIMER_ENABLE_TRG_HIGH,         /* Enable when Tx data is available */
                       EIO_TIMER_DISABLE_TIM_CMP,
                       EIO_TIMER_RESET_NEVER,
                       EIO_TIMER_DECREMENT_CLK_SHIFT_TMR, /* Decrement on EIO clock */
                       EIO_TIMER_INITOUT_ONE);
    Eio_Reg_SetTimerControl(BaseAddr,
                        UART_TX_TIMER(ResourceIndex),
                        (uint8)((uint8)(UART_TX_SHIFTER(ResourceIndex) << 2U) + 1U), /* Trigger on tx shifter status flag */
                        EIO_TRIGGER_POLARITY_LOW,
                        EIO_TRIGGER_SOURCE_INTERNAL,
                        0U,                                     /* Pin unused */
                        EIO_PIN_POLARITY_HIGH,
                        EIO_PIN_CONFIG_DISABLED,
                        EIO_TIMER_MODE_DISABLED);
}

/**
 * @brief Configures the EIO module for UART Rx
 * @note Function ID: DES_UART_API_313
 * @param [in] State: Pointer to the EIO_UART driver context structure
 * @param [in] UserConfigPtr: Pointer to the EIO_UART user configuration structure
 * @param [in] InputClock: The frequency of the input clock
 * @return void
 */
static void Eio_Uart_Hal_ConfigureRx(Eio_UartStateType *State,
                                      const Eio_UartUserConfigType *UserConfigPtr,
                                      uint32 InputClock)
{
    EIO_Type *BaseAddr;
    uint16 Divider;
    uint16 Bits;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    BaseAddr = EioBase[State->RxEioCommon.Instance];
    ResourceIndex = State->RxEioCommon.ResourceIndex;

    /* Compute divider. */
    Eio_Uart_Hal_ComputeBaudRateDivider(UserConfigPtr->BaudRate, &Divider, InputClock);
    Bits = UserConfigPtr->BitCount;

    /* Configure rx shifter */
    Eio_Reg_SetShifterConfig(BaseAddr,
                         UART_RX_SHIFTER(ResourceIndex),
                         EIO_SHIFTER_START_BIT_0,
                         EIO_SHIFTER_STOP_BIT_1,
                         EIO_SHIFTER_SOURCE_PIN);
    Eio_Reg_SetShifterControl(BaseAddr,
                          UART_RX_SHIFTER(ResourceIndex),
                          EIO_SHIFTER_MODE_DISABLED,
                          UserConfigPtr->RxDataPin,             /* Input from rx pin */
                          EIO_PIN_POLARITY_HIGH,
                          EIO_PIN_CONFIG_DISABLED,
                          UART_RX_TIMER(ResourceIndex),
                          EIO_TIMER_POLARITY_NEGEDGE);

    /* Configure rx timer */
    Eio_Reg_SetTimerCompare(BaseAddr, UART_RX_TIMER(ResourceIndex), \
                        (uint16)((((uint16)(Bits << 1U) - 1U) << 8U) + Divider));
    Eio_Reg_SetTimerConfig(BaseAddr,
                       UART_RX_TIMER(ResourceIndex),
                       EIO_TIMER_START_BIT_ENABLED,
                       EIO_TIMER_STOP_BIT_TIM_DIS,
                       EIO_TIMER_ENABLE_PIN_POSEDGE,         /* Enable when data is available */
                       EIO_TIMER_DISABLE_TIM_CMP,
                       EIO_TIMER_RESET_PIN_RISING,
                       EIO_TIMER_DECREMENT_CLK_SHIFT_TMR,    /* Decrement on EIO clock */
                       EIO_TIMER_INITOUT_ONE_RESET);
    Eio_Reg_SetTimerControl(BaseAddr,
                        UART_RX_TIMER(ResourceIndex),
                        0U,                                      /* Trigger unused */
                        EIO_TRIGGER_POLARITY_HIGH,
                        EIO_TRIGGER_SOURCE_EXTERNAL,
                        UserConfigPtr->RxDataPin,                   /* Input from rx pin */
                        EIO_PIN_POLARITY_LOW,
                        EIO_PIN_CONFIG_DISABLED,
                        EIO_TIMER_MODE_DISABLED);
}

/**
 * @brief End the current transfer
 * @note Function ID: DES_UART_API_314
 * @param [in] Channel: EIO UART channel number
 * @param [in] Direction: UART transfer direction (TX or RX)
 * @return void
 */
static void Eio_Uart_Hal_EndTransfer(uint8 Channel, Eio_UartDriverDirectionType Direction)
{
    EIO_Type *BaseAddr;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    uint8 DmaChannel;

    BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    if (Direction == EIO_UART_DIRECTION_TX)
    {
        ResourceIndex = EioUartState[Channel].TxEioCommon.ResourceIndex;
        DmaChannel = EioUartState[Channel].TxDmaChannel;
    }
    else
    {
        ResourceIndex = EioUartState[Channel].RxEioCommon.ResourceIndex;
        DmaChannel = EioUartState[Channel].RxDmaChannel;
    }

    /* Disable transfer engine */
    switch (EioUartState[Channel].DriverType)
    {
        case EIO_DRIVER_TYPE_INTERRUPTS:
            /* Disable interrupts for Rx / Tx shifter */
            Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << UART_SHIFTER(ResourceIndex)), FALSE);
            Eio_Reg_SetShifterErrorInterrupt(BaseAddr, (uint8)(1U << UART_SHIFTER(ResourceIndex)), FALSE);
            /* Disable timer interrupt  */
            Eio_Reg_SetTimerInterrupt(BaseAddr, (uint8)(1U << UART_SHIFTER(ResourceIndex)), FALSE);
            break;

        case EIO_DRIVER_TYPE_POLLING:
            /* Nothing to do here */
            break;

        case EIO_DRIVER_TYPE_DMA:
            /* For Tx we need to disable timer interrupt */
            Eio_Reg_SetTimerInterrupt(BaseAddr, (uint8)(1U << UART_SHIFTER(ResourceIndex)), FALSE);
            /* Stop DMA channels */
            (void)Dma_Hal_StopCh(DmaChannel);
            break;

        default:
            /* Impossible type - do nothing */
            break;
    }

    if (Direction == EIO_UART_DIRECTION_TX)
    {
        EioUartState[Channel].TxRemainingBytes = 0U;
        EioUartState[Channel].IsTxBusy = FALSE;
    }
    else
    {
        EioUartState[Channel].RxRemainingBytes = 0U;
        EioUartState[Channel].IsRxBusy = FALSE;
    }
}

/**
 * @brief Enables timers and shifters to start a transfer
 * @note Function ID: DES_UART_API_315
 * @param [in] Channel: EIO UART channel number
 * @param [in] Direction: UART transfer direction (TX or RX)
 * @return void
 */
static void Eio_Uart_Hal_EnableTransfer(uint8 Channel, Eio_UartDriverDirectionType Direction)
{
    EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    if (Direction == EIO_UART_DIRECTION_TX)
    {
        ResourceIndex = EioUartState[Channel].TxEioCommon.ResourceIndex;
    }
    else
    {
        ResourceIndex = EioUartState[Channel].RxEioCommon.ResourceIndex;
    }

    /* Enable timers and shifters */
    if (Direction == EIO_UART_DIRECTION_RX)
    {
        /* In rx mode, discard any leftover rx. data */
        if (Eio_Reg_GetShifterMode(BaseAddr, UART_TIMER(ResourceIndex)) == EIO_SHIFTER_MODE_MATCH_STORE)
        {
            Eio_Reg_SetShifterMode(BaseAddr, UART_TIMER(ResourceIndex), EIO_SHIFTER_MODE_MATCH_STORE);
        }
        else
        {
            Eio_Reg_SetShifterMode(BaseAddr, UART_TIMER(ResourceIndex), EIO_SHIFTER_MODE_RECEIVE);
        }
    }
    Eio_Reg_SetTimerMode(BaseAddr, UART_TIMER(ResourceIndex), EIO_TIMER_MODE_8BIT_BAUD);
}

/**
 * @brief Forcefully stops the current transfer
 * @note Function ID: DES_UART_API_316
 * @param [in] Channel: EIO UART channel number
 * @param [in] Direction: UART transfer direction (TX or RX)
 * @return void
 */
static void Eio_Uart_Hal_StopTransfer(uint8 Channel, Eio_UartDriverDirectionType Direction)
{
    uint8 ResourceIndex;
    EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];

    if (Direction == EIO_UART_DIRECTION_TX)
    {
        ResourceIndex = EioUartState[Channel].TxEioCommon.ResourceIndex;
    }
    else
    {
        ResourceIndex = EioUartState[Channel].RxEioCommon.ResourceIndex;
    }

    /* Disable and re-enable timers and shifters to reset them */
    Eio_Reg_SetTimerMode(BaseAddr, UART_TIMER(ResourceIndex), EIO_TIMER_MODE_DISABLED);
    Eio_Reg_SetShifterMode(BaseAddr, UART_TIMER(ResourceIndex), EIO_SHIFTER_MODE_DISABLED);
    
    /* Clear any leftover error flags */
    Eio_Reg_ClearShifterErrorStatus(BaseAddr, UART_SHIFTER(ResourceIndex));

    /* End the transfer */
    Eio_Uart_Hal_EndTransfer(Channel, Direction);

    /* Re-enable shifter for Tx, to ensure correct idle state */
    if (Direction == EIO_UART_DIRECTION_TX)
    {
        /* In tx mode restore start bit in case it was changed for end of transmission detection */
        Eio_Reg_SetShifterStartBit(BaseAddr, UART_TX_SHIFTER(ResourceIndex), EIO_SHIFTER_START_BIT_0);
        Eio_Reg_SetShifterMode(BaseAddr, UART_TX_SHIFTER(ResourceIndex), EIO_SHIFTER_MODE_TRANSMIT);
    }
}

/**
 * @brief Reads data received by the module
 * @note Function ID: DES_UART_API_317
 * @param [in] Channel: EIO UART channel number
 * @return void
 */
static void Eio_Uart_Hal_ReadData(uint8 Channel)
{
    DEVICE_ASSERT(EioUartState[Channel].RxRemainingBytes > 0U);
    DEVICE_ASSERT(EioUartState[Channel].RxData != NULL_PTR);

    uint32 Data;
    EIO_Type const *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    uint8 ResourceIndex = EioUartState[Channel].RxEioCommon.ResourceIndex;

    /* Read data from shifter buffer */
    if (Eio_Reg_GetShifterMode(BaseAddr, UART_RX_SHIFTER(ResourceIndex)) == EIO_SHIFTER_MODE_MATCH_STORE)
    {
        Data = Eio_Reg_ReadShifterBuffer(BaseAddr, UART_RX_SHIFTER(ResourceIndex), EIO_SHIFTER_RW_MODE_BIT_SWAP);
        Data >>= 32U - (uint32)(EioUartState[Channel].BitCount);
    }
    else
    {
        Data = Eio_Reg_ReadShifterBuffer(BaseAddr, UART_RX_SHIFTER(ResourceIndex), EIO_SHIFTER_RW_MODE_NORMAL);
        Data >>= 32U - (uint32)(EioUartState[Channel].BitCount);
    }
    if (EioUartState[Channel].BitCount <= 8U)
    {
        *(uint8 *)EioUartState[Channel].RxData = (uint8)Data;
        /* Update rx buffer pointer and remaining bytes count */
        EioUartState[Channel].RxData++;
        EioUartState[Channel].RxRemainingBytes -= 1U;
    }
    else
    {
        /* For more than 8 bits per word 2 bytes are needed */
        *(uint16 *)EioUartState[Channel].RxData = (uint16)Data;
        /* Update rx buffer pointer and remaining bytes count */
        EioUartState[Channel].RxData = &EioUartState[Channel].RxData[2U];
        EioUartState[Channel].RxRemainingBytes -= 2U;
    }
}

/**
 * @brief Writes data to be transmitted by the module
 * @note Function ID: DES_UART_API_318
 * @param [in] Channel: EIO UART channel number
 * @return void
 */
static void Eio_Uart_Hal_WriteData(uint8 Channel)
{
    DEVICE_ASSERT(EioUartState[Channel].TxData != NULL_PTR);

    uint32 Data;
    EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    uint8 ResourceIndex = EioUartState[Channel].TxEioCommon.ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    if (EioUartState[Channel].TxRemainingBytes == 0U)
    {
        /* Done transmitting */
        return;
    }
    /* Read data from user buffer and update tx buffer pointer and remaining bytes count */
    if (EioUartState[Channel].BitCount <= 8U)
    {
        Data = (uint32)(*(const uint8 *)EioUartState[Channel].TxData);
        EioUartState[Channel].TxData++;
        EioUartState[Channel].TxRemainingBytes -= 1U;
    }
    else
    {
        /* For more than 8 bits per word 2 bytes are needed */
        Data = (uint32)(*(const uint16 *)EioUartState[Channel].TxData);
        EioUartState[Channel].TxData = &EioUartState[Channel].TxData[2U];
        EioUartState[Channel].TxRemainingBytes -= 2U;
    }

    Eio_Reg_WriteShifterBuffer(BaseAddr, UART_TX_SHIFTER(ResourceIndex), Data, EIO_SHIFTER_RW_MODE_NORMAL);
}

/**
 * @brief Check status of the UART transmission
 * @note Function ID: DES_UART_API_319
 * @param [in] StateStruct: Pointer to the EIO_UART driver context structure
 * @return void
 */
static void Eio_Uart_Hal_CheckStatusTx(void *StateStruct)
{
    DEVICE_ASSERT(StateStruct != NULL_PTR);

    EIO_Type *BaseAddr;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    Eio_CommonStateType const *State;
    uint32 Channel;

    State = (Eio_CommonStateType *)StateStruct;
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    Channel = (uint32)(State->UserArgs);
    /*PRQA S 0326 -- */
    BaseAddr = EioBase[State->Instance];
    ResourceIndex = State->ResourceIndex;

    /* No need to check for Tx underflow since timer is controlled by the shifter status flag */
    /* Check for transfer end */
    if (EioUartState[Channel].TxRemainingBytes == 0U)
    {
        if (TRUE == Eio_Reg_GetTimerStatus(BaseAddr, UART_TX_TIMER(ResourceIndex)))
        {
            /* Clear timer status */
            Eio_Reg_ClearTimerStatus(BaseAddr, UART_TX_TIMER(ResourceIndex));
            EioUartState[Channel].TxFlush--;
            if (EioUartState[Channel].TxFlush == 0U)
            {
                /* Done flushing the Tx buffer, end transfer */
                /* Record success if there was no error */
                if (EioUartState[Channel].TxStatus == STATUS_BUSY)
                {
                    EioUartState[Channel].TxStatus = STATUS_SUCCESS;
                }
                Eio_Uart_Hal_StopTransfer((uint8)Channel, EIO_UART_DIRECTION_TX);
                /* Call callback to announce the end transfer event to the user */
                if (EioUartState[Channel].TxCallback != NULL_PTR)
                {
                    EioUartState[Channel].TxCallback((uint8)Channel, EIO_UART_EVENT_END_TRANSFER);
                }
            }
            else if (TRUE == Eio_Reg_GetShifterStatus(BaseAddr, UART_TX_SHIFTER(ResourceIndex)))
            {
                /* txFlush == 1, but last byte was already transferred from buffer to shifter. There is a
                   danger that the transmission is over and we end up never reporting the end event.
                   To avoid this, send one extra dummy byte */
                /* Set start bit to 1 and send an 0xFF byte, this way the line will appear idle */
                Eio_Reg_SetShifterStartBit(BaseAddr, UART_TX_SHIFTER(ResourceIndex), EIO_SHIFTER_START_BIT_1);
                Eio_Reg_WriteShifterBuffer(BaseAddr, UART_TX_SHIFTER(ResourceIndex), 0xFFFFFFFFU,
                        EIO_SHIFTER_RW_MODE_NORMAL);
            }
            else
            {
                /* txFlush == 1, and last byte was not yet transferred from buffer to shifter.
                   No need to do anything, just wait for the next timer event. */
            }
        }
    }
    /* Check if transmitter needs more data */
    else if ((TRUE == Eio_Reg_GetShifterStatus(BaseAddr, UART_TX_SHIFTER(ResourceIndex))) &&
            (EioUartState[Channel].TxRemainingBytes > 0U))
    {
        Eio_Uart_Hal_WriteData((uint8)Channel);
        if (EioUartState[Channel].TxRemainingBytes == 0U)
        {
            /* Out of data, call callback to allow user to provide a new buffer */
            if (EioUartState[Channel].TxCallback != NULL_PTR)
            {
                EioUartState[Channel].TxCallback((uint8)Channel, EIO_UART_EVENT_TX_EMPTY);
            }
        }
        if (EioUartState[Channel].TxRemainingBytes == 0U)
        {
            /* No more data, transmission will stop after the last bytes are sent.
               The timer event will indicate when the send is complete */
            /* Clear any previous timer events */
            Eio_Reg_ClearTimerStatus(BaseAddr, UART_TX_TIMER(ResourceIndex));
            if (EioUartState[Channel].DriverType == EIO_DRIVER_TYPE_INTERRUPTS)
            {
                /* Transmission completed; disable interrupt */
                Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << UART_TX_SHIFTER(ResourceIndex)), FALSE);
                /* Enable timer interrupt to ensure that transfer is completed */
                Eio_Reg_SetTimerInterrupt(BaseAddr, (uint8)(1U << UART_TX_TIMER(ResourceIndex)), TRUE);
            }
        }
    }
    else
    {
        /* No relevant events - nothing to do */
    }
}

/**
 * @brief Check status of the UART reception
 * @note Function ID: DES_UART_API_320
 * @param [in] StateStruct: Pointer to the EIO_UART driver context structure
 * @return void
 */
static void Eio_Uart_Hal_CheckStatusRx(void *StateStruct)
{
    DEVICE_ASSERT(StateStruct != NULL_PTR);

    EIO_Type *BaseAddr;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    Eio_CommonStateType const *State;
    uint32 Channel;

    State = (Eio_CommonStateType *)StateStruct;
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    Channel = (uint32)(State->UserArgs);
    /*PRQA S 0326 -- */
    BaseAddr = EioBase[State->Instance];
    ResourceIndex = State->ResourceIndex;

    /* Check for errors */
    if (TRUE == Eio_Reg_GetShifterErrorStatus(BaseAddr, UART_RX_SHIFTER(ResourceIndex)))
    {
        Eio_Uart_Hal_ReadData((uint8)Channel);
        EioUartState[Channel].RxStatus = STATUS_UART_RX_OVERRUN;
        Eio_Reg_ClearShifterErrorStatus(BaseAddr, UART_RX_SHIFTER(ResourceIndex));
        EioUartState[Channel].RxRemainingBytes = 0U;
        /* Continue processing events */
    }
    /* Check if data was received */
    else if (TRUE == Eio_Reg_GetShifterStatus(BaseAddr, UART_RX_SHIFTER(ResourceIndex)))
    {
        Eio_Uart_Hal_ReadData((uint8)Channel);
        if (EioUartState[Channel].RxRemainingBytes == 0U)
        {
            /* Out of data, call callback to allow user to provide a new buffer */
            if (EioUartState[Channel].RxCallback != NULL_PTR)
            {
                EioUartState[Channel].RxCallback((uint8)Channel, EIO_UART_EVENT_RX_FULL);
            }
        }
    }
    else
    {
        /* No events - nothing to do */
    }
    /* Check if transfer is over */
    if (EioUartState[Channel].RxRemainingBytes == 0U)
    {
        /* Record success if there was no error */
        if (EioUartState[Channel].RxStatus == STATUS_BUSY)
        {
            EioUartState[Channel].RxStatus = STATUS_SUCCESS;
        }
        /* Discard any leftover rx. data */
        Eio_Reg_ClearShifterStatus(BaseAddr, UART_RX_SHIFTER(ResourceIndex));
        /* End transfer */
        Eio_Uart_Hal_StopTransfer((uint8)Channel, EIO_UART_DIRECTION_RX);
        /* Call callback to announce the event to the user */
        if (EioUartState[Channel].RxCallback != NULL_PTR)
        {
            EioUartState[Channel].RxCallback((uint8)Channel, EIO_UART_EVENT_END_TRANSFER);
        }
    }
}

/**
 * @brief Check status of the UART transfer
 * @note Function ID: DES_UART_API_321
 * @param [in] Channel: EIO UART channel number
 * @param [in] Direction: UART transfer direction (TX or RX)
 * @return void
 */
static void Eio_Uart_Hal_CheckStatus(uint8 Channel, Eio_UartDriverDirectionType Direction)
{
    if (Direction == EIO_UART_DIRECTION_TX)
    {
        /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
        Eio_Uart_Hal_CheckStatusTx(&EioUartState[Channel].TxEioCommon);
    }
    else
    {
        Eio_Uart_Hal_CheckStatusRx(&EioUartState[Channel].RxEioCommon);
        /*PRQA S 2842 -- */
    }
}

/**
 * @brief Function called at the end of a DMA Tx transfer
 * @note Function ID: DES_UART_API_322
 * @param [in] Param: Pointer to DMA callback information
 * @return void
 */
static void Eio_Uart_Hal_EndDmaTxTransfer(void *Param)
{
    /*PRQA S 0316 ++ # interrupt functions allow conversion of a pointer to void to a pointer to an object.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    uint32 Channel = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */

    EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];

    if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))
    {
        /* DMA error, stop transfer */
        EioUartState[Channel].TxStatus = STATUS_ERROR;
        Eio_Uart_Hal_StopTransfer((uint8)Channel, EIO_UART_DIRECTION_TX);
        /* Call callback to announce the end transfer event to the user */
        if (EioUartState[Channel].TxCallback != NULL_PTR)
        {
            EioUartState[Channel].TxCallback((uint8)Channel, EIO_UART_EVENT_END_TRANSFER);
        }
    }
    else
    {
        /* Call callback to allow user to provide a new buffer */
        if (EioUartState[Channel].TxCallback != NULL_PTR)
        {
            EioUartState[Channel].TxCallback((uint8)Channel, EIO_UART_EVENT_TX_EMPTY);
        }
        
        EioUartState[Channel].TxRemainingBytes = 0U;
        /* No more data to transmit, transmission will stop */
        /* Enable timer interrupt to let IRQ ensure that transfer is completed */
        Eio_Reg_ClearTimerStatus(BaseAddr, UART_TX_TIMER(EioUartState[Channel].TxEioCommon.ResourceIndex));
        Eio_Reg_SetTimerInterrupt(BaseAddr,
                (uint8)(1U << UART_TX_TIMER(EioUartState[Channel].TxEioCommon.ResourceIndex)), TRUE);
    }
}

/**
 * @brief Function called at the end of a DMA Rx transfer
 * @note Function ID: DES_UART_API_323
 * @param [in] Param: Pointer to DMA callback information
 * @return void
 */
static void Eio_Uart_Hal_EndDmaRxTransfer(void *Param)
{
    /*PRQA S 0316 ++ # interrupt functions allow conversion of a pointer to void to a pointer to an object.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    uint32 Channel = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */

    if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))
    {
        /* DMA error, stop transfer */
        EioUartState[Channel].RxStatus = STATUS_ERROR;
        /* Stop the transfer */
        Eio_Uart_Hal_StopTransfer((uint8)Channel, EIO_UART_DIRECTION_RX);
        /* Notify the application that an error occurred */
        if (NULL_PTR != EioUartState[Channel].RxCallback)
        {
            EioUartState[Channel].RxCallback((uint8)Channel, EIO_UART_EVENT_ERROR);
        }
    }

    /* Return if an error occurred; error cases are treated by the interrupt handler */
    if ((0U != (DmaInfo->DmaEvent & DMA_FINISH_EVENT)) && (EioUartState[Channel].RxStatus == STATUS_BUSY))
    {
        /* Stop the reception */
        Eio_Uart_Hal_StopTransfer((uint8)Channel, EIO_UART_DIRECTION_RX);
        EioUartState[Channel].RxStatus = STATUS_SUCCESS;
        EioUartState[Channel].RxRemainingBytes = 0U;
        /* Invoke the callback to notify the end of the transfer */
        if (NULL_PTR != EioUartState[Channel].RxCallback)
        {
            EioUartState[Channel].RxCallback((uint8)Channel, EIO_UART_EVENT_END_TRANSFER);
        }
    }
    else/* ReceiveStatus not STATUS_BUSY */
    {
        /* do nothing */
    }
}

/**
 * @brief Computes the address of the register used for DMA Tx transfer
 * @note Function ID: DES_UART_API_324
 * @param [in] Channel: EIO UART channel number
 * @return The address of the register
 */
static uint32 Eio_Uart_Hal_ComputeTxRegAddr(uint8 Channel)
{
    uint32 Addr;

    const EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    uint8 Shifter = UART_TX_SHIFTER(EioUartState[Channel].TxEioCommon.ResourceIndex);
    Addr = (uint32)(&(BaseAddr->SHIFTBUF[Shifter]));

    return Addr;
}

/**
 * @brief Computes the address of the register used for DMA Rx transfer
 * @note Function ID: DES_UART_API_325
 * @param [in] Channel: EIO UART channel number
 * @return The address of the register
 */
static uint32 Eio_Uart_Hal_ComputeRxRegAddr(uint8 Channel)
{
    uint32 Addr;
    uint32 ByteCount;

    const EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    uint8 Shifter = UART_RX_SHIFTER(EioUartState[Channel].RxEioCommon.ResourceIndex);

    if (EioUartState[Channel].BitCount <= 8U)
    {
        ByteCount = 1U;
    }
    else
    {
        ByteCount = 2U;
    }

    if (Eio_Reg_GetShifterMode(BaseAddr, Shifter) == EIO_SHIFTER_MODE_MATCH_STORE)
    {
        Addr = (uint32)(&(BaseAddr->SHIFTBUFBIS[Shifter])) + (sizeof(uint32) - ByteCount);
    }
    else
    {
        Addr = (uint32)(&(BaseAddr->SHIFTBUF[Shifter])) + (sizeof(uint32) - ByteCount);
    }

    return Addr;
}

/**
 * @brief Starts a Tx DMA transfer
 * @note Function ID: DES_UART_API_326
 * @param [in] Channel: EIO UART channel number
 * @return Error or success status returned by API
 */
static Hal_StatusType Eio_Uart_Hal_StartTxDmaTransfer(uint8 Channel)
{
    Hal_StatusType RetCode;
    Dma_TransferUnitType DmaTransferSize;
    Dma_TransferConfigType TransferConfig;

    EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    uint8 ResourceIndex = EioUartState[Channel].TxEioCommon.ResourceIndex;
    uint32 UartChannel;

    /* Configure the transfer control descriptor for the previously allocated channel */
    if (EioUartState[Channel].BitCount <= 8U)
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_1B;
    }
    else
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_2B;
    }

    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = FALSE;
    TransferConfig.SrcUnit = DmaTransferSize;
    TransferConfig.DestUnit = DmaTransferSize;
    TransferConfig.Type = DMA_TRANSFER_MEM2PERIPH;
    TransferConfig.SrcStartAddr = (uint32)EioUartState[Channel].TxData;
    TransferConfig.DestStartAddr = Eio_Uart_Hal_ComputeTxRegAddr(Channel);
    TransferConfig.SrcOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
    TransferConfig.DestOffset = 0U;
    TransferConfig.Length = (uint16)EioUartState[Channel].TxRemainingBytes;
    TransferConfig.Callback = (Hal_CallbackType)Eio_Uart_Hal_EndDmaTxTransfer;
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    UartChannel = Channel;
    TransferConfig.UserArgs = (void *)(UartChannel);
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
    RetCode = Dma_Hal_ConfigCh(EioUartState[Channel].TxDmaChannel, &TransferConfig);

    if (STATUS_SUCCESS == RetCode)/* config dma success */
    {
        /* Enable EIO DMA requests */
        Eio_Reg_SetShifterDMARequest(BaseAddr, (uint8)(1U << UART_TX_SHIFTER(ResourceIndex)), TRUE);
        /* Start tx DMA channel */
        (void)Dma_Hal_StartCh(EioUartState[Channel].TxDmaChannel);
    }

    return RetCode;
}

/**
 * @brief Starts a Rx DMA transfer
 * @note Function ID: DES_UART_API_327
 * @param [in] Channel: EIO UART channel number
 * @return Error or success status returned by API
 */
static Hal_StatusType Eio_Uart_Hal_StartRxDmaTransfer(uint8 Channel)
{
    Hal_StatusType RetCode;
    Dma_TransferUnitType DmaTransferSize;
    Dma_TransferConfigType TransferConfig;
    uint32 UartChannel;

    EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    uint8 ResourceIndex = EioUartState[Channel].RxEioCommon.ResourceIndex;

    /* Configure the transfer control descriptor for the previously allocated channel */
    if (EioUartState[Channel].BitCount <= 8U)
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_1B;
    }
    else
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_2B;
    }

    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = FALSE;
    TransferConfig.SrcUnit = DmaTransferSize;
    TransferConfig.DestUnit = DmaTransferSize;
    TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
    TransferConfig.SrcStartAddr = Eio_Uart_Hal_ComputeRxRegAddr(Channel);
    TransferConfig.DestStartAddr = (uint32)EioUartState[Channel].RxData;
    TransferConfig.SrcOffset = 0U;
    TransferConfig.DestOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
    TransferConfig.Length = (uint16)EioUartState[Channel].RxRemainingBytes;
    TransferConfig.Callback = (Hal_CallbackType)Eio_Uart_Hal_EndDmaRxTransfer;
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    UartChannel = Channel;
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    TransferConfig.UserArgs = (void *)(UartChannel);
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
    RetCode = Dma_Hal_ConfigCh(EioUartState[Channel].RxDmaChannel, &TransferConfig);

    if (STATUS_SUCCESS == RetCode)
    {
        /* Enable EIO DMA requests */
        Eio_Reg_SetShifterDMARequest(BaseAddr, (uint8)(1U << UART_RX_SHIFTER(ResourceIndex)), TRUE);
        /* Start rx DMA channel */
        (void)Dma_Hal_StartCh(EioUartState[Channel].RxDmaChannel);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Initialize the EIO_UART driver
 * @note Function ID: DES_UART_API_300
 * @param [in] Channel: EIO UART channel number
 * @param [in] UserConfigPtr: Pointer to the EIO_UART user configuration structure
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_Init(uint8 Channel, const Eio_UartUserConfigType *UserConfigPtr)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    DEVICE_ASSERT(UserConfigPtr != NULL_PTR);

    uint32 InputClock;
    uint32 EioChannel = Channel;
    Hal_StatusType RetCode = STATUS_SUCCESS;
    EIO_Type *BaseAddr;

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    BaseAddr = EioBase[UserConfigPtr->EioInstance];
    /*PRQA S 2812 -- */

    Eio_Hal_InitDevice(UserConfigPtr->EioInstance);

    /* Get the protocol clock frequency */
    (void)Ckgen_Hal_GetFreq(Eio_HalClock[UserConfigPtr->EioInstance], &InputClock);
    DEVICE_ASSERT(InputClock > 0U);

    /* Instruct the resource allocator that we need one shifter/timer */
    if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
    {
        /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
        EioUartState[Channel].TxEioCommon.ResourceCount = 1U;
        /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
        EioUartState[Channel].TxEioCommon.UserArgs = (void *)EioChannel;
        /*PRQA S 0326 -- */
        RetCode = Eio_Hal_InitDriver(UserConfigPtr->EioInstance, &EioUartState[Channel].TxEioCommon);
    }

    if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U)
    {
        EioUartState[Channel].RxEioCommon.ResourceCount = 1U;
        /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
        EioUartState[Channel].RxEioCommon.UserArgs = (void *)EioChannel;
        /*PRQA S 0326 -- */
        RetCode = Eio_Hal_InitDriver(UserConfigPtr->EioInstance, &EioUartState[Channel].RxEioCommon);
    }

    if (RetCode == STATUS_SUCCESS)
    {
        /* Initialize driver-specific context structure */
        EioUartState[Channel].EioInstance = UserConfigPtr->EioInstance;
        EioUartState[Channel].RxData = NULL_PTR;
        EioUartState[Channel].TxData = NULL_PTR;
        EioUartState[Channel].TxRemainingBytes = 0U;
        EioUartState[Channel].RxRemainingBytes = 0U;
        EioUartState[Channel].TxCallback = UserConfigPtr->TxCallback;
        EioUartState[Channel].RxCallback = UserConfigPtr->RxCallback;
        EioUartState[Channel].DriverType = UserConfigPtr->DriverType;
        EioUartState[Channel].Direction = UserConfigPtr->Direction;
        EioUartState[Channel].TxStatus = STATUS_IDLE;
        EioUartState[Channel].RxStatus = STATUS_IDLE;
        EioUartState[Channel].IsTxBusy = FALSE;
        EioUartState[Channel].IsRxBusy = FALSE;
        EioUartState[Channel].BitCount = UserConfigPtr->BitCount;

        if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
        {
            if (EioUartState[Channel].DriverType == EIO_DRIVER_TYPE_DMA)
            {
                /* Enable EIO DMA requests */
                Eio_Reg_SetShifterDMARequest(BaseAddr,
                        (uint8)(1U << UART_TX_SHIFTER(EioUartState[Channel].TxEioCommon.ResourceIndex)), TRUE);
            }
            /* Configure device for UART Tx mode */
            Eio_Uart_Hal_ConfigureTx(&EioUartState[Channel], UserConfigPtr, InputClock);
        }

        if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U)
        {
            if (EioUartState[Channel].DriverType == EIO_DRIVER_TYPE_DMA)
            {
                /* Enable EIO DMA requests */
                Eio_Reg_SetShifterDMARequest(BaseAddr,
                        (uint8)(1U << UART_RX_SHIFTER(EioUartState[Channel].RxEioCommon.ResourceIndex)), TRUE);
            }
            /* Configure device for UART Rx mode */
            Eio_Uart_Hal_ConfigureRx(&EioUartState[Channel], UserConfigPtr, InputClock);
        }

        /* Set up transfer engine */
        switch (EioUartState[Channel].DriverType)
        {
            case EIO_DRIVER_TYPE_INTERRUPTS:
                if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
                {
                    EioUartState[Channel].TxEioCommon.Isr = Eio_Uart_Hal_CheckStatusTx;
                }

                if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U)
                {
                    EioUartState[Channel].RxEioCommon.Isr = Eio_Uart_Hal_CheckStatusRx;
                }
                break;

            case EIO_DRIVER_TYPE_POLLING:
                /* Nothing to do here, Eio_Uart_Hal_GetStatus() will handle the transfer */
                break;

            case EIO_DRIVER_TYPE_DMA:
                /* Store DMA channel number */
                EioUartState[Channel].TxDmaChannel = UserConfigPtr->TxDmaChannel;
                EioUartState[Channel].RxDmaChannel = UserConfigPtr->RxDmaChannel;

                /* For Tx we will still need interrupt to signal end of transfer */
                if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U)
                {
                    EioUartState[Channel].TxEioCommon.Isr = Eio_Uart_Hal_CheckStatusTx;
                }
                break;

            default:
                /* Impossible type - do nothing */
                break;
        }

        EioUartState[Channel].InitState = TRUE;
        /*PRQA S 2842 -- */
    }

    return RetCode;
}

/**
 * @brief De-initialize the EIO_UART driver
 * @note Function ID: DES_UART_API_301
 * @param [in] Channel: EIO UART channel number
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_Deinit(uint8 Channel)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(TRUE == EioUartState[Channel].InitState);

    Hal_StatusType RetCode = STATUS_SUCCESS;

    /* Check if driver is busy */
    if ((EioUartState[Channel].IsTxBusy == TRUE) || (EioUartState[Channel].IsRxBusy == TRUE))
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        Eio_Hal_DeinitDriver(&EioUartState[Channel].TxEioCommon);
        Eio_Hal_DeinitDriver(&EioUartState[Channel].RxEioCommon);
        Eio_Hal_DeinitDevice(EioUartState[Channel].EioInstance);
    }
    /*PRQA S 2842 -- */

    return RetCode;
}

/**
 * @brief Set the baud rate
 * @note Function ID: DES_UART_API_302
 * @param [in] Channel: EIO UART channel number
 * @param [in] BaudRate: The desired baud rate in hertz
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_SetBaudRate(uint8 Channel, uint32 BaudRate)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(TRUE == EioUartState[Channel].InitState);
    DEVICE_ASSERT(BaudRate > 0U);

    Hal_StatusType RetCode = STATUS_SUCCESS;
    uint16 Divider;
    uint32 InputClock;
    uint8 TxResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    uint8 RxResourceIndex;
    uint16 Bits = EioUartState[Channel].BitCount;

    EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    TxResourceIndex = EioUartState[Channel].TxEioCommon.ResourceIndex;
    RxResourceIndex = EioUartState[Channel].RxEioCommon.ResourceIndex;

    /* Check if driver is busy */
    if ((EioUartState[Channel].IsTxBusy == TRUE) || (EioUartState[Channel].IsRxBusy == TRUE))
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        /* Get the protocol clock frequency */
        (void)Ckgen_Hal_GetFreq(Eio_HalClock[EioUartState[Channel].EioInstance], &InputClock);
        DEVICE_ASSERT(InputClock > 0U);

        /* Compute divider */
        Eio_Uart_Hal_ComputeBaudRateDivider(BaudRate, &Divider, InputClock);

        /* Configure tx/rx timer */
        if (((uint8)EioUartState[Channel].Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
        {
            Eio_Reg_SetTimerCompare(BaseAddr, UART_TX_TIMER(TxResourceIndex),
                     (uint16)((((uint16)(Bits << 1U) - 1U) << 8U) + Divider));
        }
        if (((uint8)EioUartState[Channel].Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U)
        {
            Eio_Reg_SetTimerCompare(BaseAddr, UART_RX_TIMER(RxResourceIndex),
                     (uint16)((((uint16)(Bits << 1U) - 1U) << 8U) + Divider));
        }
    }
    /*PRQA S 2842 -- */

    return RetCode;
}

/**
 * @brief Get the currently configured baud rate
 * @note Function ID: DES_UART_API_303
 * @param [in] Channel: EIO UART channel number
 * @param [out] BaudRate: The current baud rate in hertz
 * @return void
 */
void Eio_Uart_Hal_GetBaudRate(uint8 Channel, uint32 *BaudRate)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(TRUE == EioUartState[Channel].InitState);
    DEVICE_ASSERT(BaudRate != NULL_PTR);

    uint32 InputClock;
    uint16 Divider;
    uint16 TimerCmp;
    uint8 TxResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    uint8 RxResourceIndex;

    const EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    TxResourceIndex = EioUartState[Channel].TxEioCommon.ResourceIndex;
    RxResourceIndex = EioUartState[Channel].RxEioCommon.ResourceIndex;

    /* Get the protocol clock frequency */
    (void)Ckgen_Hal_GetFreq(Eio_HalClock[EioUartState[Channel].EioInstance], &InputClock);
    DEVICE_ASSERT(InputClock > 0U);

    /* Get the currently configured divider */
    if (((uint8)EioUartState[Channel].Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
    /*PRQA S 2842 -- */
    {
        TimerCmp = Eio_Reg_GetTimerCompare(BaseAddr, UART_TX_TIMER(TxResourceIndex));
    }
    else
    {
        TimerCmp = Eio_Reg_GetTimerCompare(BaseAddr, UART_RX_TIMER(RxResourceIndex));
    }
    Divider = (uint16)(TimerCmp & 0x00FFU);

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    /* Compute baud rate: input_clock / (2 * (divider + 1)). Round to nearest integer */
    *BaudRate = (InputClock + (uint32)Divider + 1U) / (2U * ((uint32)Divider + 1U));
    /*PRQA S 2812 -- */
}

/**
 * @brief Perform a non-blocking UART transmission
 * @note Function ID: DES_UART_API_304
 * @param [in] Channel: EIO UART channel number
 * @param [in] TxBuff: Pointer to the data to be transferred
 * @param [in] TxSize: Length in bytes of the data to be transferred
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_SendData(uint8 Channel, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(TRUE == EioUartState[Channel].InitState);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    DEVICE_ASSERT(((uint8)EioUartState[Channel].Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U);
    /* If 2 bytes per word are needed, then the size must be even */
    DEVICE_ASSERT((EioUartState[Channel].BitCount <= 8U) || ((TxSize & 1U) == 0U));

    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    Hal_StatusType RetCode = STATUS_SUCCESS;

    EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    ResourceIndex = EioUartState[Channel].TxEioCommon.ResourceIndex;

    /* Check if driver is busy */
    if (TRUE == EioUartState[Channel].IsTxBusy)
    {
        RetCode =  STATUS_BUSY;
    }
    else
    {
        EioUartState[Channel].TxData = (const uint8 *)TxBuff;
        EioUartState[Channel].TxRemainingBytes = TxSize;
        EioUartState[Channel].TxStatus = STATUS_BUSY;
        EioUartState[Channel].IsTxBusy = TRUE;
        /* Number of bytes to flush after the last byte is copied in the tx shifter buffer */
        EioUartState[Channel].TxFlush = (uint8)((TxSize == 1U) ? 1U : 2U);

        /* Enable timers and shifters */
        Eio_Uart_Hal_EnableTransfer(Channel, EIO_UART_DIRECTION_TX);
        /* Enable transfer engine */
        switch (EioUartState[Channel].DriverType)
        /*PRQA S 2842 -- */
        {
            case EIO_DRIVER_TYPE_INTERRUPTS:
                /* Enable interrupts for Tx shifter */
                Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << UART_TX_SHIFTER(ResourceIndex)), TRUE);
                Eio_Reg_SetShifterErrorInterrupt(BaseAddr, (uint8)(1U << UART_TX_SHIFTER(ResourceIndex)), TRUE);
                break;

            case EIO_DRIVER_TYPE_POLLING:
                /* Call Eio_Uart_Hal_CheckStatus once to send the first byte */
                Eio_Uart_Hal_CheckStatus(Channel, EIO_UART_DIRECTION_TX);
                break;

            case EIO_DRIVER_TYPE_DMA:
                RetCode = Eio_Uart_Hal_StartTxDmaTransfer(Channel);
                break;

            default:
                /* Impossible type - do nothing */
                break;
        }
    }

    return RetCode;
}

/**
 * @brief Perform a non-blocking UART reception
 * @note Function ID: DES_UART_API_305
 * @param [in] Channel: EIO UART channel number
 * @param [in] RxBuff: Pointer to the receive buffer
 * @param [in] RxSize: Length in bytes of the data to be received
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_ReceiveData(uint8 Channel, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(TRUE == EioUartState[Channel].InitState);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    DEVICE_ASSERT(((uint8)EioUartState[Channel].Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U);
    /* If 2 bytes per word are needed, then the size must be even */
    DEVICE_ASSERT((EioUartState[Channel].BitCount <= 8U) || ((RxSize & 1U) == 0U));

    Hal_StatusType RetCode = STATUS_SUCCESS;
    EIO_Type *BaseAddr = EioBase[EioUartState[Channel].EioInstance];
    uint8 ResourceIndex = EioUartState[Channel].RxEioCommon.ResourceIndex;

    /* Check if driver is busy */
    if (TRUE == EioUartState[Channel].IsRxBusy)
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        EioUartState[Channel].RxData = RxBuff;
        EioUartState[Channel].RxRemainingBytes = RxSize;
        EioUartState[Channel].RxStatus = STATUS_BUSY;
        EioUartState[Channel].IsRxBusy = TRUE;

        /* Enable timers and shifters */
        Eio_Uart_Hal_EnableTransfer(Channel, EIO_UART_DIRECTION_RX);
        /* Enable transfer engine */
        switch (EioUartState[Channel].DriverType)
        /*PRQA S 2842 -- */
        {
            case EIO_DRIVER_TYPE_INTERRUPTS:
                /* Enable interrupts for Rx shifter */
                Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << UART_RX_SHIFTER(ResourceIndex)), TRUE);
                Eio_Reg_SetShifterErrorInterrupt(BaseAddr, (uint8)(1U << UART_RX_SHIFTER(ResourceIndex)), TRUE);
                break;

            case EIO_DRIVER_TYPE_POLLING:
                /* Call Eio_Uart_Hal_CheckStatus once to send the first byte */
                Eio_Uart_Hal_CheckStatus(Channel, EIO_UART_DIRECTION_RX);
                break;

            case EIO_DRIVER_TYPE_DMA:
                if (STATUS_SUCCESS != Eio_Uart_Hal_StartRxDmaTransfer(Channel))
                {
                    RetCode = STATUS_ERROR;
                }
                break;

            default:
                /* Impossible type - do nothing */
                break;
        }
    }

    return RetCode;
}

/**
 * @brief Aborts a non-blocking UART transmission
 * @note Function ID: DES_UART_API_306
 * @param [in] Channel: EIO UART channel number
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_AbortSendingData(uint8 Channel)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(TRUE == EioUartState[Channel].InitState);
    /* Check if driver is busy */
    if (EioUartState[Channel].IsTxBusy == TRUE)
    {
        EioUartState[Channel].TxStatus = STATUS_UART_ABORTED;
        Eio_Uart_Hal_StopTransfer(Channel, EIO_UART_DIRECTION_TX);
    }
    /*PRQA S 2842 -- */

    return STATUS_SUCCESS;
}

/**
 * @brief Aborts a non-blocking UART reception
 * @note Function ID: DES_UART_API_307
 * @param [in] Channel: EIO UART channel number
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_AbortReceivingData(uint8 Channel)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    DEVICE_ASSERT(TRUE == EioUartState[Channel].InitState);
    /* Check if driver is busy */
    if (EioUartState[Channel].IsRxBusy == TRUE)
    {
        EioUartState[Channel].RxStatus = STATUS_UART_ABORTED;
        Eio_Uart_Hal_StopTransfer(Channel, EIO_UART_DIRECTION_RX);
    }
    /*PRQA S 2842 -- */

    return STATUS_SUCCESS;
}

/**
 * @brief Get the status of the current non-blocking UART transmission
 * @note Function ID: DES_UART_API_308
 * @param [in] Channel: EIO UART channel number
 * @param [out] BytesRemaining: The remaining number of bytes to be transferred
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_GetSendStatus(uint8 Channel, uint32 *BytesRemaining)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);

    uint32 RemainingBytes;

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    /* Initialize with the actual remaining bytes count */
    RemainingBytes = EioUartState[Channel].TxRemainingBytes;

    if (TRUE == EioUartState[Channel].IsTxBusy)
    {
        switch (EioUartState[Channel].DriverType)
        {
            case EIO_DRIVER_TYPE_POLLING:
                /* In polling mode advance the UART transfer here */
                Eio_Uart_Hal_CheckStatus(Channel, EIO_UART_DIRECTION_TX);
                break;

            case EIO_DRIVER_TYPE_DMA:
                /* In DMA mode just update the remaining count.
                DO NOT write State->RemainingBytes directly !!! */
                RemainingBytes = EioUartState[Channel].TxRemainingBytes -
                        Dma_Hal_GetTransBytes(EioUartState[Channel].TxDmaChannel);
                break;

            default:
                /* Nothing */
                break;
        }
    }

    if (BytesRemaining != NULL_PTR)
    {
        *BytesRemaining = RemainingBytes;
    }

    return EioUartState[Channel].TxStatus;
    /*PRQA S 2842 -- */
}

/**
 * @brief Get the status of the current non-blocking UART reception
 * @note Function ID: DES_UART_API_309
 * @param [in] Channel: EIO UART channel number
 * @param [out] BytesRemaining: The remaining number of bytes to be transferred
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_GetReceiveStatus(uint8 Channel, uint32 *BytesRemaining)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);

    uint32 RemainingBytes;

    /*PRQA S 2842 ++ # the upper layer call guarantees that there will never be an array out of bounds.*/
    /* Initialize with the actual remaining bytes count */
    RemainingBytes = EioUartState[Channel].RxRemainingBytes;

    if (TRUE == EioUartState[Channel].IsRxBusy)
    {
        switch (EioUartState[Channel].DriverType)
        {
            case EIO_DRIVER_TYPE_POLLING:
                /* In polling mode advance the UART transfer here */
                Eio_Uart_Hal_CheckStatus(Channel, EIO_UART_DIRECTION_RX);
                break;

            case EIO_DRIVER_TYPE_DMA:
                /* In DMA mode just update the remaining count.
                DO NOT write State->RemainingBytes directly !!! */
                RemainingBytes = EioUartState[Channel].RxRemainingBytes -
                        Dma_Hal_GetTransBytes(EioUartState[Channel].RxDmaChannel);
                break;

            default:
                /* Nothing */
                break;
        }
    }

    if (BytesRemaining != NULL_PTR)
    {
        *BytesRemaining = RemainingBytes;
    }

    return EioUartState[Channel].RxStatus;
    /*PRQA S 2842 -- */
}

/**
 * @brief Returns default configuration structure for EIO_UART
 * @note Function ID: DES_UART_API_310
 * @param [out] UserConfigPtr: Pointer to the EIO_UART user configuration structure
 * @return void
 */
void Eio_Uart_Hal_GetDefaultConfig(Eio_UartUserConfigType *UserConfigPtr)
{
    DEVICE_ASSERT(UserConfigPtr != NULL_PTR);

    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    UserConfigPtr->EioInstance = 0U;
    UserConfigPtr->DriverType = EIO_DRIVER_TYPE_INTERRUPTS;
    UserConfigPtr->BaudRate = 115200U;
    UserConfigPtr->BitCount = 8U;
    UserConfigPtr->Direction = EIO_UART_DIRECTION_BOTH;
    UserConfigPtr->TxDataPin = 0U;
    UserConfigPtr->RxDataPin = 1U;
    UserConfigPtr->TxCallback = NULL_PTR;
    UserConfigPtr->RxCallback = NULL_PTR;
    UserConfigPtr->TxDmaChannel = 255U;
    UserConfigPtr->RxDmaChannel = 255U;
    /*PRQA S 2812 -- */
}

/*============================================EOF===================================================*/

