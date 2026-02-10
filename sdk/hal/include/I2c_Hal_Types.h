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
 * @file I2c_Hal_Types.h
 * @brief This file provides i2c config
 */

#ifndef I2C_HAL_TYPES_H
#define I2C_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
#include "Platform_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/
#define I2C_INSTANCE_ID                      0
/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/*!
* @brief I2C 10bit address reserve mask bit macro.
*/
#define I2C_ADDEXT_PRIMARY_BYTE_FIX (0xF8U)

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */
/*!< I2c transmit Data */
typedef uint8 I2c_TransmitData;
/*!< Callback for I2C master mode */
typedef void (*I2c_Hal_CallbackType)(uint8 Instance, uint32 Event);

/**
* @brief The enum contains the I2c TX or RX
*/
enum
{
    I2C_ENABLE_RX = 0x0U, /*!< Enable I2C RX */
    I2C_ENABLE_TX /*!< Enable I2C TX */
};

/*!
 * @brief Define the enum of the events which can trigger I2C callback
 */
enum
{
    I2C_SLAVE_EVENT_RX_FULL = 0x00U, /*!< Rx buffer is full */
    I2C_SLAVE_EVENT_TX_EMPTY = 0x01U, /*!< Tx buffer is empty */
    I2C_SLAVE_EVENT_TX_REQ = 0x02U, /*!< Tx request */
    I2C_SLAVE_EVENT_RX_REQ = 0x03U, /*!< Rx request */
    I2C_SLAVE_EVENT_STOP = 0x04U, /*!< Slave detected stop */
    I2C_SLAVE_EVENT_ADDRESS_MATCH = 0x05U, /*!< Slave address match */
    I2C_MASTER_EVENT_END_TRANSFER = 0x06U, /*!< Master transferend event */
    I2C_MASTER_EVENT_PLTIE = 0x07U /*!< Master pltie event */
};

/*!
 * @brief I2C master/slave mode enum
 */
typedef enum
{
    I2C_SLAVE = 0x00U, /*!< I2C slave mode */
    I2C_MASTER = 0x01U /*!< I2C master mode */
} I2c_ModeTypes;

/*!
 * @brief I2C HW Type
 */
typedef enum
{
    HW_I2C = 0x00U,      /*!< HW I2C */
    EIO_I2C = 0x01U      /*!< EIO I2C*/
} I2c_HwTypes;

/*!
 * @brief Type of I2C transfer direction (write or read)
 */
typedef enum
{
    I2C_WRITE = 0x00U, /*!< I2C write transfer*/
    I2C_READ = 0x01U /*!< I2C read transfer*/
} I2c_Hal_DirType;

/*!
 * @brief Type of I2C transfer(based on interrupts or DMA)
 */
typedef enum
{
    I2C_USING_DMA = 0x00U, /*!< The driver will use DMA to perform I2C transfer*/
    I2C_USING_INTERRUPTS = 0x01U /*!< The driver will use interrupt to perform I2C transfer*/
} I2c_Hal_TransferType;

/*!
 * @brief I2C timecfg mode enum
 */
typedef enum
{
    I2C_SCL = 0x00U, /*!< I2C SCL mode */
    I2C_SDA_SCL = 0x01U /*!< I2C SDA/SCL mode */
} I2c_Hal_TimeCfgTypes;

typedef enum
{
    I2C_MASTER_CHANNEL_IDLE = 0U, /*!< brief Status Indication I2C channel is idle */
    I2C_MASTER_CHANNEL_BUSY_SEND, /*!< brief Status Indication send operation is ongoing */
    I2C_MASTER_CHANNEL_BUSY_RECEIVE, /*!< brief Status Indication receiving operation is ongoing */
    I2C_MASTER_CHANNEL_FINISHED, /*!< brief Status Indication operation is finished */
    I2C_MASTER_CHANNEL_ARBITRATION_LOST, /*!< brief Status Indication operation is arbitation lost */
    I2C_MASTER_CHANNEL_RECEIVE_NACK, /*!< brief Status Indication operation is receive NACK */
    I2C_MASTER_CHANNEL_ABORTED, /*!< brief Status Indication operation is aborted */
    I2C_MASTER_CHANNEL_TIMEOUT, /*!< brief Status Indication operation is timeout */
    I2C_MASTER_CHANNEL_DMA_ERROR /*!< brief Status Indication operation is Dma transmit error */
} I2c_Hal_MasterStatusType;

typedef enum
{
    I2C_SLAVE_CHANNEL_IDLE = 0U, /*!< brief Status Indication I2C channel is idle */
    I2C_SLAVE_CHANNEL_BUSY_SEND, /*!< brief Status Indication send operation is ongoing */
    I2C_SLAVE_CHANNEL_BUSY_RECEIVE, /*!< brief Status Indication receiving operation is ongoing */
    I2C_SLAVE_CHANNEL_BUSY_TRANSMIT, /*!< brief Status Indication Transmit operation is ongoing */
    I2C_SLAVE_CHANNEL_FINISHED, /*!< brief Status Indication operation is finished */
    I2C_SLAVE_CHANNEL_TX_EMPTY, /*!< brief Status Indication an error is tx empty */
    I2C_SLAVE_CHANNEL_TX_UNDERRUN, /*!< brief Status Indication an error is under run */
    I2C_SLAVE_CHANNEL_RX_OVERRUN, /*!< brief Status Indication an error is over run */
    I2C_SLAVE_CHANNEL_ABORTED, /*!< brief Status Indication operation is aborted */
    I2C_SLAVE_CHANNEL_DMA_ERROR /*!< brief Status Indication operation is Dma transmit error */
} I2c_Hal_SlaveStatusType;

typedef enum
{
    I2C_CHANNEL_IDLE = 0U, /*!< brief Status Indication I2C channel is idle */
    I2C_CHANNEL_BUSY_TRANSMIT, /*!< brief Status Indication Transmit operation is ongoing */
    I2C_CHANNEL_FINISHED, /*!< brief Status Indication operation is finished */
    I2C_CHANNEL_ERROR_PRESENT, /*!< brief Status Indication an error is present */
    I2C_CHANNEL_ABORTED_SUCCESS /*!< brief Status Indication operation abort success */
} I2c_Hal_ChannelStatusType;

/*PRQA S 3630 ++ # the defined external struct or union doesnot need to be hidden. */
/*!
 * @brief Configuration structure that the user needs to set
 */
typedef struct
{
    uint8 TxDmaChannel;     /*!<Tx Channel number for DMA */
    uint8 RxDmaChannel;     /*!<Rx Channel number for DMA */
    uint8 SdaPin;           /*!< Eio pin to use as I2C SDA pin */
    uint8 SclPin;           /*!< Eio pin to use as I2C SCL pin */
    uint16 SlaveAddress;    /*!< Slave address, 7-bit or 10-bit */
    uint16 PinLow;          /*!< Pinlow * Clock */
    uint32 BaudRate;        /*!< Baud rate */
    boolean SendStop;       /*!< Specifies if STOP condition must be generated after current transfer*/
    boolean SclSdaLowEn;    /*!< PltieEn enable*/
    boolean SyncEn;         /*!< Enable/disable SCL sync */
    boolean Arbitration;    /*!< Enable/disable arbitration */
    I2c_Hal_TransferType TransferType; /*!< Type of I2C transfer, DMA or interrupt */
    I2c_Hal_TimeCfgTypes TimeCfgTypes; /*!< Type of I2C TimeCfg, SCL or SCL/SDA */
} I2c_Hal_MasterConfigType;

/*!
 * @brief Slave configuration structure
 */
typedef struct
{
    uint8 TxDmaChannel; /*!<Tx Channel number for DMA */
    uint8 RxDmaChannel; /*!<Rx Channel number for DMA */
    uint16 SlaveAddress; /*!< Slave address, 7-bit or 10-bit */
    boolean StretchEn; /*!< Enable/disable slave SCL stretch*/
    boolean WakeupEn; /*!< Enable/disable slave address match wakeup */
    boolean Is10bitAddr; /*!< Select 7-bit or 10-bit slave address */
    boolean slaveListening; /*!< Slave mode (always listening or on demand only) */
    I2c_Hal_TransferType TransferType; /*!< Type of I2C transfer, DMA or interrupt */
} I2c_Hal_SlaveConfigType;

/**
 * @brief DataTransmitType
 */
typedef struct
{
    uint16 SlaveAddress; /*!< Slave address, 7-bit or 10-bit */
    I2c_Hal_DirType DirType; /*!< Type of I2C transfer direction */
    boolean SendStop; /*!< Specifies if STOP condition must be generated after current transfer*/
    I2c_TransmitData *DataBufferPtr;
    uint32 DataLength;
    boolean Is10bitAddr; /*!< i2c slave addr is 10bit or not */
} DataTransmitType;

/*!
 * @brief I2C configuration structure
 */
typedef struct
{
    I2c_HwTypes I2cType;                            /*!< Type of I2C mode*/
    I2c_ModeTypes I2cMode;                          /*!< Type of I2C mode*/
    I2c_Hal_CallbackType Callback;                  /*!< callback function */
    const I2c_Hal_MasterConfigType *MasterConfigPtr;/*!< Master internal context structure */
    const I2c_Hal_SlaveConfigType *SlaveConfigPtr;  /*!< Slave internal context structure */
} I2c_Hal_ChannelConfigType;
/*PRQA S 3630 -- */

#ifdef __cplusplus
}
#endif

#endif
/*============================================EOF===================================================*/
