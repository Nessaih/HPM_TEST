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
 * AutoChips Inc. (C) 2022. All rights reserved.
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
/******************************************************************************
* SPECIFICATION(S) : specification of DIO Driver, AUTOSAR Release 4.4.0
******************************************************************************/
/**

 * @file Crc_Hal_Types.h
 *
 * @brief This file provides extern Crc macro enum and structure info for hal/mcal.
 *
 */
#ifndef CRC_HAL_TYPES_H
#define CRC_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/** @brief Support for caculate method. caculate method by define.*/
#define    CRC_TABLE_16_BYTE_MODE          0x0U
#define    CRC_TABLE_256_BYTE_MODE         0x1U
#define    CRC_RUNTIME_MODE                0x2U
#define    CRC_HARDWARE_MODE               0x3U

/**************************CRC initial value**************************/
/** @brief Definition of the initial value of the SAE J1850 CRC8 */
#define CRC_INITIAL_VALUE8    0xFFU

/** @brief Definition of the initial value of the CRC8 on polynom 0x2F */
#define CRC_INITIAL_VALUE8H2F 0xFFU

/** @brief Definition of the initial value of crc16 */
#define CRC_INITIAL_VALUE16   0xFFFFU

/** @brief Definition of the initial value of crc16 */
#define CRC_INITIAL_VALUE16ARC   (0x0U)

/** @brief Definition of the initial value of crc32 */
#define CRC_INITIAL_VALUE32   0xFFFFFFFFU

/** @brief Definition of the initial value of crc64 */
#define CRC_INITIAL_VALUE64   0xFFFFFFFFFFFFFFFFULL

/** @brief DATA register is data */
#define CRC_DATA_IS_DATA (0x0U)
/** @brief DATA register is Seed */
#define CRC_DATA_IS_SEED (0x1U)
/*===================================================ENUMS==========================================*/
/** @brief CRC protocol enum */
typedef enum
{
    CRC_PROTOCOL_16BIT = 0x00U, /*!< CRC Protocol 16bit mode */
    CRC_PROTOCOL_32BIT, /*!< CRC Protocol 32bit mode */
} Crc_ProtocolType;

/** @brief CRC transpose enum */
typedef enum
{
    CRC_TRANSPOSE_NONE = 0x00U, /*!< CRC write in without tranpose */
    CRC_TRANSPOSE_BITS, /*!< CRC write in with bits transpose */
    CRC_TRANSPOSE_BITS_BYTES, /*!< CRC write in with bits and bytes transpose */
    CRC_TRANSPOSE_BYTES /*!< CRC write in with bytes transpose */
} Crc_TransposeType;

/**
 * @brief CRC configuration structure
 *
 * This structure holds the configuration settings for the crc
 */
/*PRQA S 3630 ++ # will be used in the future.*/
typedef struct
{
    Crc_ProtocolType Protocol; /*!< CRC 16/32 protocol type */
    Crc_TransposeType WriteTranspose; /*!< CRC write in transpose type */
    Crc_TransposeType ReadTranspose; /*!< CRC read out transpose type */
    boolean FinalXOR; /*!< Enable/disable result XOR */
    uint32 Poly; /*!< CRC polynomial */
    uint32 Seed; /*!< CRC Seed*/
} Crc_ConfigType;
/*PRQA S 3630 -- # will be used in the future.*/
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif

#endif
