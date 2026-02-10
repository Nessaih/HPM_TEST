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
 * @file Eio_Uart_Hal.h
 *
 * @brief This file provides eio uart integration functions interface.
 *
 */

#ifndef EIO_UART_HAL_H
#define EIO_UART_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Eio_Uart_Hal_Types.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */
/**
 * @brief Initialize the EIO_UART driver
 * @note Function ID: DES_UART_API_300
 * @param [in] Channel: EIO UART channel number
 * @param [in] UserConfigPtr: Pointer to the EIO_UART user configuration structure
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_Init(uint8 Channel, const Eio_UartUserConfigType *UserConfigPtr);

/**
 * @brief De-initialize the EIO_UART driver
 * @note Function ID: DES_UART_API_301
 * @param [in] Channel: EIO UART channel number
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_Deinit(uint8 Channel);

/**
 * @brief Set the baud rate
 * @note Function ID: DES_UART_API_302
 * @param [in] Channel: EIO UART channel number
 * @param [in] BaudRate: The desired baud rate in hertz
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_SetBaudRate(uint8 Channel, uint32 BaudRate);

/**
 * @brief Get the currently configured baud rate
 * @note Function ID: DES_UART_API_303
 * @param [in] Channel: EIO UART channel number
 * @param [out] BaudRate: The current baud rate in hertz
 * @return void
 */
void Eio_Uart_Hal_GetBaudRate(uint8 Channel, uint32 *BaudRate);

/**
 * @brief Perform a non-blocking UART transmission
 * @note Function ID: DES_UART_API_304
 * @param [in] Channel: EIO UART channel number
 * @param [in] TxBuff: Pointer to the data to be transferred
 * @param [in] TxSize: Length in bytes of the data to be transferred
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_SendData(uint8 Channel, const uint8 *TxBuff, uint32 TxSize);

/**
 * @brief Perform a non-blocking UART reception
 * @note Function ID: DES_UART_API_305
 * @param [in] Channel: EIO UART channel number
 * @param [in] RxBuff: Pointer to the receive buffer
 * @param [in] RxSize: Length in bytes of the data to be received
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_ReceiveData(uint8 Channel, uint8 *RxBuff, uint32 RxSize);

/**
 * @brief Aborts a non-blocking UART transmission
 * @note Function ID: DES_UART_API_306
 * @param [in] Channel: EIO UART channel number
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_AbortSendingData(uint8 Channel);

/**
 * @brief Aborts a non-blocking UART reception
 * @note Function ID: DES_UART_API_307
 * @param [in] Channel: EIO UART channel number
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_AbortReceivingData(uint8 Channel);

/**
 * @brief Get the status of the current non-blocking UART transmission
 * @note Function ID: DES_UART_API_308
 * @param [in] Channel: EIO UART channel number
 * @param [out] BytesRemaining: The remaining number of bytes to be transferred
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_GetSendStatus(uint8 Channel, uint32 *BytesRemaining);

/**
 * @brief Get the status of the current non-blocking UART reception
 * @note Function ID: DES_UART_API_309
 * @param [in] Channel: EIO UART channel number
 * @param [out] BytesRemaining: The remaining number of bytes to be transferred
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Uart_Hal_GetReceiveStatus(uint8 Channel, uint32 *BytesRemaining);

/**
 * @brief Returns default configuration structure for EIO_UART
 * @note Function ID: DES_UART_API_310
 * @param [out] UserConfigPtr: Pointer to the EIO_UART user configuration structure
 * @return void
 */
void Eio_Uart_Hal_GetDefaultConfig(Eio_UartUserConfigType *UserConfigPtr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EIO_UART_HAL_H */
/* =============================================  EOF  ============================================== */
