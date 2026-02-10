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
 * @file Lin_Hal_Types.h
 *
 * @brief This file provides lin module used hardware Types.
 */

#ifndef LIN_HAL_TYPES_H
#define LIN_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/
#define LIN_MAKE_PARITY         (0U)        /*!< Make parity for PID */
#define LIN_CHECK_PARITY        (1U)        /*!< Check parity for PID */

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/
/*!
 * @brief LIN Mode.
 */
typedef enum
{
    LIN_MASTER = 0U, /*!< set the node type as MASTER */
    LIN_SLAVE = 1U, /*!< set the node type as SLAVE */
} Lin_ModeType;

/*!
 * @brief LIN break length for LIN master send.
 */
typedef enum
{
    BREAK_LENGTH_13BIT = 0U, /*!< break length 13bit */
    BREAK_LENGTH_14BIT, /*!< break length 14bit */
    BREAK_LENGTH_15BIT, /*!< break length 15bit */
    BREAK_LENGTH_16BIT, /*!< break length 16bit */
    BREAK_LENGTH_17BIT, /*!< break length 17bit */
    BREAK_LENGTH_18BIT, /*!< break length 18bit */
    BREAK_LENGTH_19BIT, /*!< break length 19bit */
    BREAK_LENGTH_20BIT, /*!< break length 20bit */
    BREAK_LENGTH_21BIT, /*!< break length 21bit */
    BREAK_LENGTH_22BIT, /*!< break length 22bit */
    BREAK_LENGTH_23BIT, /*!< break length 23bit */
    BREAK_LENGTH_24BIT, /*!< break length 24bit */
    BREAK_LENGTH_25BIT, /*!< break length 25bit */
    BREAK_LENGTH_26BIT, /*!< break length 26bit */
    BREAK_LENGTH_27BIT, /*!< break length 27bit */
    BREAK_LENGTH_28BIT /*!< break length 28bit */
} Lin_BreakLengthType;

/*!
 * @brief LIN break threshold for LIN slave detect.
 */
typedef enum
{
    BREAK_THRESHOLD_10BIT = 0U, /*!< 10-bit length */
    BREAK_THRESHOLD_11BIT = 1U, /*!< 11-bit length */
} Lin_BreakThresholdType;

/*!
 * @brief LIN event identifier enumeration define
 */
typedef enum
{
    LIN_NO_EVENT = 0x00U, /*!< No event */
    LIN_WAKEUP_SIGNAL = 0x01U, /*!< Received a wakeup signal */
    LIN_SYNC_OK = 0x02U, /*!< Sync byte is correct */
    LIN_SYNC_ERROR = 0x03U, /*!< Sync byte is error */
    LIN_PID_OK = 0x04U, /*!< PID correct */
    LIN_PID_ERROR = 0x05U, /*!< PID error */
    LIN_FRAME_ERROR = 0x06U, /*!< Frame error */
    LIN_READBACK_ERROR = 0x07U, /*!< Readback data is error */
    LIN_CHECKSUM_ERROR = 0x08U, /*!< Checksum is error */
    LIN_TX_COMPLETED = 0x09U, /*!< Send data completed */
    LIN_RX_COMPLETED = 0x0AU, /*!< Receive data completed */
    LIN_RX_OVERRUN = 0x0BU, /*!< Receive overrun flag */
    LIN_NOISE_ERROR = 0x0CU, /*!< Noise byte is error */
    LIN_TIMEOUT_ERROR = 0x0DU, /*!< Timeout error */
    LIN_BREAK_ERROR = 0x0EU,  /*!< Break error */
} Lin_EventIdType;

/*!
 * @brief LIN node state enumeration define
 */
typedef enum
{
    LIN_NODE_STATE_UNINIT = 0x00U, /*!< Uninitialized state */
    LIN_NODE_STATE_SLEEP_MODE = 0x01U, /*!< Sleep mode state */
    LIN_NODE_STATE_IDLE = 0x02U, /*!< Idle state */
    LIN_NODE_STATE_SEND_BREAK_FIELD = 0x03U, /*!< Send break field state */
    LIN_NODE_STATE_SEND_SYNC = 0x04U, /*!< Send sync byte state */
    LIN_NODE_STATE_RECV_SYNC = 0x05U, /*!< Receive sync byte state */
    LIN_NODE_STATE_SEND_PID = 0x06U, /*!< Send PID state */
    LIN_NODE_STATE_RECV_PID = 0x07U, /*!< Receive PID state */
    LIN_NODE_STATE_RECV_DATA = 0x08U, /*!< Receive data state */
    LIN_NODE_STATE_RECV_DATA_COMPLETED = 0x09U, /*!< Receive data completed state */
    LIN_NODE_STATE_SEND_DATA = 0x0AU, /*!< Send data state */
    LIN_NODE_STATE_SEND_DATA_COMPLETED = 0x0BU /*!< Send data completed state */
} Lin_NodeStateType;

/*!
 * @brief Define type for checksum type of the frame.
 */
typedef enum
{
    LIN_ENHANCED_CS, /*!< Enhanced checksum mode */
    LIN_CLASSIC_CS /*!< Classic checksum mode */
} Lin_FrameCsModelType;

/*!
 * @brief Define type for response type of the frame.
 */
typedef enum
{
    LIN_FRAMERESPONSE_TX, /*!< Response is generated from this node.*/
    LIN_FRAMERESPONSE_RX, /*!< Response is generated from another node and is relevant for this node.*/
    LIN_FRAMERESPONSE_IGNORE /*!< Response is generated from one slave to another slave.*/
} Lin_FrameResponseType;

/*!
 * @brief Define a range of transfer status
 */
typedef enum
{
    LIN_NOT_OK = 0, /*!< Development or production error occurred.*/
    LIN_TX_OK, /*!< Successful transmission.*/
    LIN_TX_BUSY, /*!< Ongoing transmission (Header or Response).*/
    LIN_TX_HEADER_ERROR, /*!< Erroneous header transmission such as:
                                     - Mismatch between sent and read
                                       back data
                                     - Identifier parity error
                                     - Physical bus error.*/
    LIN_TX_ERROR, /*!< Erroneous transmission such as:
                                     - Mismatch between sent and read back data
                                     - Physical bus error.*/
    LIN_RX_OK, /*!< Reception of correct response.*/
    LIN_RX_BUSY, /*!< Ongoing reception: at least one response byte has been received, but the checksum byte has not been received.*/
    LIN_RX_ERROR, /*!< Erroneous reception such as:
                                     - Framing error
                                     - Overrun error
                                     - Checksum error.*/
    LIN_RX_NO_RESPONSE, /*!< No response byte has been received so far.*/
    LIN_OPERATIONAL, /*!< Normal operation:
                                     - The related LIN channel is ready to transmit next header
                                     - No data from previous frame available (e.g. after initialization).*/
    LIN_CH_SLEEP /*!< Sleep mode operation;
                                     - In this mode wake-up detection from slave nodes is enabled.*/
} Lin_StatusType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*PRQA S 3630 ++ # the defined external structure or union does not need to be hidden.*/
/*!
 * @brief LIN channel info structure
 */
typedef struct
{
    uint8 *RxBuff; /*!< The buffer of received data */
    uint8 CurrentPid; /*!< Current PID */
    Lin_EventIdType CurrentEventId; /*!< Current event ID */
} Lin_ChannelInfoType;

/*!
 * @brief LIN callback function
 */
typedef void (* Lin_CallbackType)(uint8 Instance, Lin_ChannelInfoType *LinInfo);

/*!
 * @brief Configuration struction of the LIN driver
 */
typedef struct
{
    uint32 BaudRate; /*!< LIN Baudrate value */
    boolean AutoBaudEnable; /*!< Autobaud function enable */
    Lin_ModeType ModeType; /*!< Node mode as Master or Slave */
    Lin_BreakLengthType BreakLength; /*!< LIN break length for master */
    Lin_BreakThresholdType BreakThreshold; /*!< LIN break detect threshold for slave */
    Lin_CallbackType Callback; /*!< Callback funtion */
} Lin_ChannelConfigType;

/*!
 * @brief LIN frame type used to provide PID,checksum model, data length and SDU pointer
 */
typedef struct
{
    uint8 Pid; /*!< LIN frame identifier.*/
    Lin_FrameCsModelType Cs; /*!< Checksum model type.*/
    Lin_FrameResponseType Drc; /*!< Response type.*/
    uint8 Dl; /*!< Data length.*/
    uint8 *SduPtr; /*!< Pointer to Sdu(Servie Data Unit).*/
} Lin_PduType;
/*PRQA S 3630 -- # the defined external structure or union does not need to be hidden.*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif
#endif
/*============================================EOF===================================================*/
