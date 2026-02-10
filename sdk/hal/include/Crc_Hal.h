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

 * @file Crc_Hal.h
 *
 * @brief This file provides extern Crc Hal API.
 *
 */
#ifndef CRC_HAL_H
#define CRC_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Crc_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief This function initializes the driver
 * @note  Function ID : DES_CRC_API_201
 * @param[in] Instance: CRC Hardware Device instance.
 * @param[in] ConfigPtr: Pointer to a selected configuration structure
 * @return void.
 */
void Crc_Hal_Init
(
    uint8 Instance,
    const Crc_ConfigType *ConfigPtr
);

/**
* @brief This function deinitializes the driver
* @note  Function ID : DES_CRC_API_206
* @return void
*/
void Crc_Hal_Deinit(void);

/**
 * @brief Appends a block of bytes to the current CRC calculation
 * @note  Function ID : DES_CRC_API_202
 * @param[in] Instance: CRC Hardware Device instance.
 * @param[in] DataPtr: The Pointer to the data array
 * @param[in] Length: Number of the data array
 * @return CRC calculate result.
 */
uint32 Crc_Hal_CalculateCRC
(
    uint8 Instance,
    const uint8 *DataPtr,
    uint32 Length
);

/**
 * @brief Appends a block of bytes to the current CRC calculation using dma
 * @note  Function ID : DES_CRC_API_203
 * @param[in] Channel: Dma Channel Id
 * @param[in] DataPtr: The Pointer to the data array
 * @param[in] Length: Number of the data array
 * @param[in] Params: Dma callback parametes delive to callback (not used currently)
 * @return call dma success or error.
 */
Hal_StatusType Crc_Hal_DmaCalculateCRC
(
    uint8 Channel,
    const uint8 *DataPtr,
    uint32 Length,
    const void *Params
);

/**
 * @brief Gets the current result of the CRC32/CRC16 calculation
 * @note  Function ID : DES_CRC_API_204
 * @param[in] Instance: The CRC instance number
 * @return Result of CRC32/CRC16 calculation
 */
uint32 Crc_Hal_GetCRCResult
(
    uint8 Instance
);

/*!
 * @brief Sets seed value for CRC module.
 * @note  Function ID : DES_CRC_API_214
 * @param[in] Instance: The CRC instance number
 * @param[in] Seed: New seed data for CRC module
 * @return void
 */
void Crc_Hal_SetSeed(uint8 Instance, uint32 Seed);

/**
 * @brief Gets the configuration structure of the CRC module currently.
 * @note  Function ID : DES_CRC_API_205
 * @param[in] Instance: The CRC instance number
 * @param[out] ConfigPtr: Pointer to structure of CRC configuration
 * @return The result of execution
 *       - STATUS_SUCCESS: Operation was successful
 *       - STATUS_ERROR: Operation was successful
 */
Hal_StatusType Crc_Hal_GetConfig
(
    uint8 Instance,
    Crc_ConfigType *ConfigPtr
);

/**
 * @brief CRC8 caculate function with software.
 * @note  Function ID : DES_CRC_API_207
 * @param[in] Crc_DataPtr: The pointer to data block.
 * @param[in] Crc_Length: caculate crc data length
 * @param[in] Crc_StartValue8: The CRC8 start value
 * @param[in]  Crc_IsFirstCall: check it is first call or not.
 * @param[in]  Mode: Crc Calculate Mode
 *                -CRC_TABLE_16_BYTE_MODE
 *                -CRC_TABLE_256_BYTE_MODE
 *                -CRC_RUNTIME_MODE
 * @return Crc8 result.
 */
uint8 Crc_Hal_CalculateCRC8
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint8 Crc_StartValue8,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief CRC8H2F caculate function with software.
 * @note  Function ID : DES_CRC_API_208
 * @param[in] Crc_DataPtr: The pointer to data block.
 * @param[in] Crc_Length: caculate crc data length
 * @param[in] Crc_StartValue8H2F: The CRC8H2F start value
 * @param[in]  Crc_IsFirstCall: check it is first call or not.
 * @param[in]  Mode: Crc Calculate Mode
 *                -CRC_TABLE_16_BYTE_MODE
 *                -CRC_TABLE_256_BYTE_MODE
 *                -CRC_RUNTIME_MODE
 * @return Crc8H2F result.
 */
uint8 Crc_Hal_CalculateCRC8H2F
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint8 Crc_StartValue8H2F,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief CRC16 caculate function with software.
 * @note  Function ID : DES_CRC_API_209
 * @param[in] Crc_DataPtr: The pointer to data block.
 * @param[in] Crc_Length: caculate crc data length
 * @param[in] Crc_StartValue16: The CRC16 start value.
 * @param[in]  Crc_IsFirstCall: check it is first call or not.
 * @param[in]  Mode: Crc Calculate Mode
 *                -CRC_TABLE_16_BYTE_MODE
 *                -CRC_TABLE_256_BYTE_MODE
 *                -CRC_RUNTIME_MODE
 * @return CRC16 result.
 */
uint16 Crc_Hal_CalculateCRC16
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint16 Crc_StartValue16,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief CRC16ARC caculate function with software.
 * @note  Function ID : DES_CRC_API_210
 * @param[in] Crc_DataPtr: The pointer to data block.
 * @param[in] Crc_Length: caculate crc data length
 * @param[in] Crc_StartValue16: The CRC16ARC start value.
 * @param[in]  Crc_IsFirstCall: check it is first call or not.
 * @param[in]  Mode: Crc Calculate Mode
 *                -CRC_TABLE_16_BYTE_MODE
 *                -CRC_TABLE_256_BYTE_MODE
 *                -CRC_RUNTIME_MODE
 * @return CRC16ARC result.
 */
uint16 Crc_Hal_CalculateCRC16ARC
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint16 Crc_StartValue16,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief CRC32 caculate function with software.
 * @note  Function ID : DES_CRC_API_211
 * @param[in] Crc_DataPtr: The pointer to data block.
 * @param[in] Crc_Length: caculate crc data length
 * @param[in] Crc_StartValue32: The CRC32 start value.
 * @param[in]  Crc_IsFirstCall: check it is first call or not.
 * @param[in]  Mode: Crc Calculate Mode
 *                -CRC_TABLE_16_BYTE_MODE
 *                -CRC_TABLE_256_BYTE_MODE
 *                -CRC_RUNTIME_MODE
 * @return CRC32 result.
 */
uint32 Crc_Hal_CalculateCRC32
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint32 Crc_StartValue32,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief CRC32P4 caculate function with software.
 * @note  Function ID : DES_CRC_API_212
 * @param[in] Crc_DataPtr: The pointer to data block.
 * @param[in] Crc_Length: caculate crc data length
 * @param[in] Crc_StartValue32: The CRC32P4 start value.
 * @param[in]  Crc_IsFirstCall: check it is first call or not.
 * @param[in]  Mode: Crc Calculate Mode
 *                -CRC_TABLE_16_BYTE_MODE
 *                -CRC_TABLE_256_BYTE_MODE
 *                -CRC_RUNTIME_MODE
 * @return CRC32P4 result.
 */
uint32 Crc_Hal_CalculateCRC32P4
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint32 Crc_StartValue32,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief CRC64 caculate function with software.
 * @note  Function ID : DES_CRC_API_213
 * @param[in] Crc_DataPtr: The pointer to data block.
 * @param[in] Crc_Length: caculate crc data length
 * @param[in] Crc_StartValue64: The CRC64 start value.
 * @param[in]  Crc_IsFirstCall: check it is first call or not.
 * @param[in]  Mode: Crc Calculate Mode
 *                -CRC_TABLE_16_BYTE_MODE
 *                -CRC_TABLE_256_BYTE_MODE
 *                -CRC_RUNTIME_MODE
 * @return CRC64 result.
 */
uint64 Crc_Hal_CalculateCRC64
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint64 Crc_StartValue64,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/* ======================================  Functions definition  ==================================== */

/*==================================================================================================
 *                                        GLOBAL FUNCTIONS
==================================================================================================*/
#ifdef __cplusplus
}
#endif

#endif /* CRC_HAL_H */
