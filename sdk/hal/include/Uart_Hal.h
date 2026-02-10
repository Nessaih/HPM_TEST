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
* @brief This file provides Uart hal function extern
*/

#ifndef UART_HAL_H
#define UART_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Uart_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief  Initializes the Uart module.
 * @note Function ID: DES_UART_API_000
 * @param [in] Instance: UART hardware channel ID
 * @param [in] ChannelConfigPtr: Pointer to Uart_ChannelConfigType
 * @return void
 */
void Uart_Hal_Init(uint8 Instance, const Uart_ChannelConfigType *ChannelConfigPtr);

/**
 * @brief  Deinitializes the Uart module.
 * @note Function ID: DES_UART_API_001
 * @param [in] Instance: UART hardware channel ID
 * @return void
 */
void Uart_Hal_DeInit(uint8 Instance);

/*!
 * @brief  UART send data using non-blocking method (interrupt or DMA).
 * @note  Function ID: DES_UART_API_002
 * @param [in] Instance: UART hardware channel ID
 * @param [in] TxBuff: The tx data buffer pointer
 * @param [in] TxSize: The tx data buffer bytes size
 * @return Hal_StatusType: if successful return STATUS_SUCCESS or return STATUS_ERROR/STATUS_BUSY.
 */
Hal_StatusType Uart_Hal_SendData(uint8 Instance, const uint8 *TxBuff, uint32 TxSize);

#ifndef UART_SDK_NON_EXTENDED_API
/*!
 * @brief  UART send data using polling mode.
 * @note  Function ID: DES_UART_API_003
 * @param [in] Instance: UART hardware channel ID
 * @param [in] TxBuff: The tx data buffer pointer
 * @param [in] TxSize: The tx data buffer bytes size
 * @return Hal_StatusType: if successful return STATUS_SUCCESS or return STATUS_BUSY.
 */
Hal_StatusType Uart_Hal_SendDataPolling(uint8 Instance, const uint8 *TxBuff, uint32 TxSize);

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
Hal_StatusType Uart_Hal_SendDataBlocking(uint8 Instance, const uint8 *TxBuff, uint32 TxSize, uint32 TimeoutUs);
#endif

/*!
 * @brief  Get send status.
 * @note Function ID: DES_UART_API_008
 * @param [in] Instance: UART hardware channel ID
 * @param [out] BytesRemaining: Remaining bytes to be sent
 * @return Hal_StatusType: Transmit status
 */
Hal_StatusType Uart_Hal_GetSendStatus(uint8 Instance, uint32 *BytesRemaining);

/*!
 * @brief  Terminates an non-blocking UART transmission early.
 * @note  Function ID: DES_UART_API_010
 * @param [in] Instance: UART hardware channel ID
 * @return Hal_StatusType: return STATUS_SUCCESS if successful
 */
Hal_StatusType Uart_Hal_AbortSendingData(uint8 Instance);

/*!
 * @brief  UART receive data using non-blocking method(interrupt or DMA).
 * @note Function ID: DES_UART_API_005
 * @param [in] Instance: UART hardware channel ID
 * @param [out] RxBuff: The rx data buffer pointer
 * @param [in] RxSize: The rx data buffer bytes size
 * @return Hal_StatusType: if successful return STATUS_SUCCESS or return STATUS_ERROR/STATUS_BUSY.
 */
Hal_StatusType Uart_Hal_ReceiveData(uint8 Instance, uint8 *RxBuff, uint32 RxSize);

#ifndef UART_SDK_NON_EXTENDED_API
/*!
 * @brief  UART receive data using polling mode.
 * @note  Function ID: DES_UART_API_006
 * @param [in] Instance: UART hardware channel ID
 * @param [out] RxBuff: The rx data buffer pointer
 * @param [in] RxSize: The rx data buffer bytes size
 * @return Hal_StatusType: Receive status
 */
Hal_StatusType Uart_Hal_ReceiveDataPolling(uint8 Instance, uint8 *RxBuff, uint32 RxSize);

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
Hal_StatusType Uart_Hal_ReceiveDataBlocking(uint8 Instance, uint8 *RxBuff, uint32 RxSize, uint32 TimeoutUs);
#endif

/*!
 * @brief  Get receive status.
 * @note Function ID: DES_UART_API_009
 * @param [in] Instance: UART hardware channel ID
 * @param [out] BytesRemaining: Remaining bytes to receive
 * @return Hal_StatusType: Receive status
 */
Hal_StatusType Uart_Hal_GetReceiveStatus(uint8 Instance, uint32 *BytesRemaining);

/*!
 * @brief  Terminates a non-blocking receive early.
 * @note Function ID: DES_UART_API_011
 * @param [in] Instance: UART hardware channel ID
 * @return Hal_StatusType: return STATUS_SUCCESS if successful
 */
Hal_StatusType Uart_Hal_AbortReceivingData(uint8 Instance);

/**
 * @brief  Set UART baud rate.
 * @note Function ID: DES_UART_API_012
 * @param [in] Instance: UART hardware channel ID
 * @param [in] DesiredBaudRate: The desired baudrate to be set
 * @param [in] SampleCnt: The sample count value to be used
 * @return Hal_StatusType: The status of the operation STATUS_SUCCESS or STATUS_BUSY
 */
Hal_StatusType Uart_Hal_SetBaudRate(uint8 Instance, uint32 DesiredBaudRate, Uart_SampleCntType SampleCnt);

/**
 * @brief  Enable or disable the UART idle interrupt.
 * @note Function ID: DES_UART_API_019
 * @param [in] Instance: UART hardware channel ID
 * @param [in] IsEnable: Use TRUE to enable and FALSE to disable the idle interrupt
 * @return void
 */
void Uart_Hal_SetIdleInterrupt(uint8 Instance, boolean IsEnable);

#ifndef UART_SDK_NON_EXTENDED_API
/*!
 * @brief  Get UART baudrate.
 * @note Function ID: DES_UART_API_013
 * @param [in] Instance: UART hardware channel ID
 * @param [out] ConfiguredBaudRate: Return the UART configured baudrate
 * @return void
 */
void Uart_Hal_GetBaudRate(uint8 Instance, uint32 *ConfiguredBaudRate);

/**
 * @brief  Configure data match interrupt.
 * @note Function ID: DES_UART_API_016
 * @param [in] Instance: UART hardware channel ID
 * @param [in] Data: The data value to match against received data
 * @param [in] IsEnable: Use TRUE to enable and FALSE to disable data match
 * @return void
 */
void Uart_Hal_SetDataMatch(uint8 Instance, uint16 Data, boolean IsEnable);

/**
 * @brief  Configure address filter.
 * @note Function ID: DES_UART_API_015
 * @param [in] Instance: UART hardware channel ID
 * @param [in] Addr: The address value to be used for filtering incoming data
 * @param [in] IsEnable: Use TRUE to enable and FALSE to disable address filter
 * @return void
 */
void Uart_Hal_SetAddrFilter(uint8 Instance, uint16 Addr, boolean IsEnable);

/**
 * @brief  Enable or disable the UART match interrupt.
 * @note Function ID: DES_UART_API_020
 * @param [in] Instance: UART hardware channel ID
 * @param [in] IsEnable: Use TRUE to enable and FALSE to disable match interrupt
 * @return void
 */
void Uart_Hal_SetMatchInterrupt(uint8 Instance, boolean IsEnable);

/**
 * @brief  Configure the RTS/CTS flow control.
 * @note Function ID: DES_UART_API_014
 * @param [in] Instance: UART hardware channel ID
 * @param [in] RtsCts: Configuration for RTS/CTS flow control
 * @return void
 */
void Uart_Hal_SetCtsRts(uint8 Instance, Uart_RtsCtsType RtsCts);

/**
 * @brief  Configure RS485 settings.
 * @note Function ID: DES_UART_API_018
 * @param [in] Instance: UART hardware channel ID
 * @param [in] Config: Config Pointer to the RS485 configuration structure
 * @return void
 */
void Uart_Hal_SetRS485(uint8 Instance, const Uart_RS485ConfigType *Config);

/**
 * @brief  Configure IrDA settings.
 * @note Function ID: DES_UART_API_017
 * @param [in] Instance: UART hardware channel ID
 * @param [in] Config: Config Pointer to the IrDA configuration structure
 * @return void
 */
void Uart_Hal_SetIrDA(uint8 Instance, const Uart_IrDAConfigType *Config);
#endif

#ifdef __cplusplus
}
#endif

#endif
/*============================================EOF===================================================*/
