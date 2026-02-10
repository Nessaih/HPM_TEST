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
* @file Lin_Hal.h
* @brief This file provides extern Hal lin api.
*/

#ifndef LIN_HAL_H
#define LIN_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Lin_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Hal Initialize a LIN channel.
 * @note Function ID: DES_LIN_API_000
 * @param [in] Instance: LIN module Instance.
 * @param [in] ChannelConfigPtr: LIN Channel Config pointer.
 * @return void
 */
void Lin_Hal_Init
(
    uint8 Instance,
    const Lin_ChannelConfigType *ChannelConfigPtr
);

#ifndef LIN_SDK_NON_EXTENDED_API
/**
 * @brief Deinitializes the Lin module.
 * @note Function ID: DES_LIN_API_001
 * @param [in] Instance: LIN module Instance
 * @return void
 */
void Lin_Hal_DeInit
(
    uint8 Instance
);
#endif

/**
 * @brief Lin channel go to sleep.
 * @note Function ID: DES_LIN_API_005
 * @param [in] Instance: LIN module Instance.
 * @return Hal_StatusType: return STATUS_SUCCESS if successful, STATUS_BUSY if busy.
 */
Hal_StatusType Lin_Hal_GoToSleepMode
(
    uint8 Instance
);

/**
 * @brief Lin channel go to idle state.
 * @note Function ID: DES_LIN_API_007
 * @param [in] Instance: LIN module Instance.
 * @return void
 */
void Lin_Hal_GoToIdleState
(
    uint8 Instance
);

/**
 * @brief Send LIN wakeup signal
 * @note Function ID: DES_LIN_API_006
 * @param [in] Instance: LIN module Instance.
 * @return Hal_StatusType: return STATUS_SUCCESS if successful, STATUS_BUSY if busy.
 */
Hal_StatusType Lin_Hal_SendWakeupSignal
(
    uint8 Instance
);

/**
 * @brief Process identifier parity.
 * @note Function ID: DES_LIN_API_003
 * @param [in] Pid: ID or Pid.
 * @param [in] Type: Specifies the operation type:
                - LIN_MAKE_PARITY : Compute the parity bits and return the PID
                - LIN_CHECK_PARITY: Check the parity bits     return PID if correct,otherwise return 0xFF.
 * @return uint8: If successful, return PID, otherwise return 0xFF.
 */
uint8 Lin_Hal_ProcessParity
(
    uint8 Pid,
    uint8 Type
);

/**
 * @brief Send LIN frame data.
 * @note Function ID: DES_LIN_API_002
 * @param [in] Instance: LIN module Instance
 * @param [in] PduInfo: Pointer to Lin_PduType containing the PID,Checksum model,
                     Response type, Data Length and SDU data
 * @return Hal_StatusType:if successful return STATUS_SUCCESS or return STATUS_ERROR/STATUS_BUSY.
 */
Hal_StatusType Lin_Hal_SendFrameData
(
    uint8 Instance,
    const Lin_PduType *PduInfo
);

/**
 * @brief This function is abort lin transfer.
 * @note Function ID: DES_LIN_API_004
 * @param [in] Instance: LIN module Instance.
 * @return void
 */
void Lin_Hal_AbortTransferData
(
    uint8 Instance
);

/**
 * @brief Gets the status of the LIN driver when Channel is operating.
 * @note Function ID: DES_LIN_API_008
 * @param [in] Instance: LIN module Instance.
 * @param [out] LinSduPtr: Pointer to the buffer where the received SDU(Service Data Unit) will be stored.
 * @return Lin_StatusType: Lin current channel status
 */
Lin_StatusType Lin_Hal_GetStatus
(
    uint8 Instance,
    uint8 *LinSduPtr
);

#ifndef LIN_SDK_NON_EXTENDED_API
/*!
 * @brief Set timeout counter for timer interrupt
 * @note Function ID: DES_LIN_API_017
 * @param [in] Instance: LIN module instance
 * @param [in] Timeout: timeout counter
 * @return void
 */
void Lin_Hal_SetTimeoutCounter
(
    uint8 Instance,
    uint32 Timeout
);

/*!
 * @brief Timer interrupt callback function
 * @note Function ID: DES_LIN_API_018
 * @param [in] Instance: LIN module instance
 * @return void
 */
void Lin_Hal_TimeoutService
(
    uint8 Instance
);
#endif

#ifdef __cplusplus
}
#endif
#endif
/*============================================EOF===================================================*/
