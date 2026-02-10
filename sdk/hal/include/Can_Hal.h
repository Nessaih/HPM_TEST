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

 * @file Can_Hal.h
 *
 * @brief This file provides extern Can Hal API implement.
 *
 */

#ifndef CAN_HAL_H
#define CAN_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Can_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/**
 * @brief Initialize a can hw unit
 * @note Function ID: DES_CAN_API_001
 * @note Service ID: none
 * @param [in] Instance: Specify CAN HW Unit
 * @param [in] ConfigPtr: Config args
 * @return void
 */
void Can_Hal_Init(uint8 Instance, const Can_HalConfigType *ConfigPtr);

/**
* @brief Deinitializes a can hw unit
* @note Function ID: DES_CAN_API_002
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @return void
*/
void Can_Hal_Deinit(uint8 Instance);

/**
* @brief Set work state for a can hw unit
* @note Function ID: DES_CAN_API_003
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [in] State: The can work state to be set
* @return Hal_StatusType. if successful return STATUS_SUCCESS or return STATUS_ERROR
*/
Hal_StatusType Can_Hal_SetControllerState(uint8 Instance, Can_HalStateType State);

/**
* @brief Get work state for a can hw unit
* @note Function ID: DES_CAN_API_004
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @return Can_HalStateType. return The can currernt work state
*/
Can_HalStateType Can_Hal_GetControllerState(uint8 Instance);

/**
* @brief Write send frame information to hw and send it
* @note Function ID: DES_CAN_API_005
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [in] BufferId: Send buffer id
* @param [in] MessagePtr: Send can frame information
* @return Can_HalStateType. if successful return STATUS_SUCCESS or return STATUS_BUSY
*/
Hal_StatusType Can_Hal_WriteTxBuffer(uint8 Instance, uint8 BufferId, const Can_MessageInfoType *MessagePtr);

/**
* @brief Read receive frame information from hw
* @note Function ID: DES_CAN_API_006
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [in] BufferId: Read buffer id
* @param [out] MessagePtr: Read can frame information
* @return Can_HalStateType. if successful return STATUS_SUCCESS or return STATUS_ERROR
*/
Hal_StatusType Can_Hal_ReadRxBuffer(uint8 Instance, uint8 BufferId, Can_MessageInfoType *MessagePtr);

/**
* @brief Get sent status
* @note Function ID: DES_CAN_API_007
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [in] BufferId: Read buffer id
* @return Can_HalStateType. if sent successful return STATUS_SUCCESS or return STATUS_ERROR
*/
Hal_StatusType Can_Hal_GetTxStatus(uint8 Instance, uint8 BufferId);

/**
* @brief Get received status
* @note Function ID: DES_CAN_API_008
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [in] BufferId: Read buffer id
* @return Can_HalStateType. if received successful return STATUS_SUCCESS or return STATUS_ERROR
*/
Hal_StatusType Can_Hal_GetRxStatus(uint8 Instance, uint8 BufferId);

/**
* @brief Get the number of frames the can sent errors
* @note Function ID: DES_CAN_API_009
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @return the number of frames the can sent errors
*/
uint8 Can_Hal_GetTxErrorCount(uint8 Instance);

/**
* @brief Get the number of frames the can received errors
* @note Function ID: DES_CAN_API_010
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @return the number of frames the can received errors
*/
uint8 Can_Hal_GetRxErrorCount(uint8 Instance);

/**
* @brief Get can error state
* @note Function ID: DES_CAN_API_011
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @return Can_DevErrorStateType. can error state
*/
Can_DevErrorStateType Can_Hal_GetErrorState(uint8 Instance);

/**
* @brief Get can errors information
* @note Function ID: DES_CAN_API_012
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @return can errors mask information
*/
uint32 Can_Hal_GetErrorsInfo(uint8 Instance);

#ifndef CAN_SDK_NON_EXTENDED_API
/**
* @brief Set can timestamp
* @note Function ID: DES_CAN_API_013
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [in] Mode: Can_ExtendModeType
* @return void
*/
void Can_Hal_ConfigExtendMode(uint8 Instance, Can_ExtendModeType Mode);

/**
* @brief Config can timestamp function
* @note Function ID: DES_CAN_API_014
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [in] tsConfigPtr: timestamp config args
* @return void
*/
void Can_Hal_ConfigTimeStamp(uint8 Instance, const Can_TimeStampType *tsConfigPtr);

#if defined (AC7840X) || defined (AC7842X)
/**
* @brief Get the can send buffer sent complete timestamp value
* @note Function ID: DES_CAN_API_015
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [in] BufferId: Sent buffer id
* @return can receive frame timestamp value
*/
uint32 Can_Hal_GetTxTimeStamp(uint8 Instance, uint8 BufferId);
#endif
#endif

/**
* @brief Get the can base address
* @note Function ID: DES_CAN_API_016
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @return can base address ptr
*/
CAN_Type *Can_Hal_GetBase(uint8 Instance);

/**
* @brief abort tx transmit
* @note Function ID: DES_CAN_API_017
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [in] BufferId: Read buffer id
* @return Can_HalStateType. STATUS_SUCCESS: abort success, STATUS_ERROR: can not abort(transmit completed)
*/
Hal_StatusType Can_Hal_AbortTransmit(uint8 Instance, uint8 BufferId);

#ifndef CAN_SDK_NON_EXTENDED_API
#if defined (AC7843X)
/**
* @brief Read an element from tx event fifo
* @note Function ID: DES_CAN_API_018
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param [out] Msg: tx event fifo element
* @return Can_HalStateType. if read successful return STATUS_SUCCESS or return STATUS_ERROR
*/
Hal_StatusType Can_Hal_ReadTxEvent(uint8 Instance, Can_MsgEventType *Msg);
#endif
#endif

/**
* @brief config send frame info to hw.
* @note Function ID: DES_CAN_API_019
* @note Service ID: none
* @param[in] Instance: CAN module Instance
* @param[in] BufferIndex: rx buffer index
* @param[in] InfoPtr - a non-null pointer pointing to the frame information to be transmitted
* @return void
*/
void Can_Hal_SetMsgInfo(uint8 Instance, uint8 BufferIndex, const Can_MessageInfoType *InfoPtr);

#if defined (AC7840X) || defined (AC7842X)
/**
* @brief start can transmission action.
* @note Function ID: DES_CAN_API_020
* @note Service ID: none
* @param [in] Instance: Specify CAN HW Unit
* @param[in] BufferId - transmit bufferId
* @return void
*/
void Can_Hal_StartTransmit(uint8 Instance, uint8 BufferId);
#endif

#ifndef CAN_SDK_NON_EXTENDED_API
/**
 * @brief Get the CAN message information from reveive buffer.
 * @note Function ID: DES_CAN_API_021
 * @note Service ID: none
 * @param[out] InfoPtr: pointer to message information
 * @param[in] Buf: pointer to receive buffer
 * @return void
 */
void Can_Hal_GetMsgInfo(Can_MessageInfoType *InfoPtr, const uint32 *Buf);

/**
 * @brief Can start next dma transfer.
 * @note Function ID: DES_CAN_API_022
 * @note Service ID: none
 * @param[in] Instance: Specify CAN HW Unit
 * @param[in] FifoId: Can HW Unit rx fifoid
 * @param[in] DmaDstAddr: The dma dest addresss
 * @return void
 */
void Can_Hal_StartNextDma(uint8 Instance, uint8 FifoId, uint32 DmaDstAddr);

#if defined (AC7840X) || defined (AC7842X)
/*!
 * @brief Set amount for transmit secondary buffer.
 * @note Function ID: DES_CAN_API_023
 * @note Service ID: none
 * @param[in] Instance: CAN module instance
 * @param[in] amount: transmit secondary buffer amount
 * @return none
 */
void Can_Hal_SetTxSecAmount(uint8 Instance, Can_TxSecAmountType amount);
#endif
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
