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
 * @file Core_Hal.h
 *
 * @brief This file provides extern Hal Core api.
 *
 */

#ifndef CORE_HAL_H
#define CORE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Core_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Enable Irq.
 * @note Function ID: DES_CORE_API_200
 * @param[in] IrqNumber: Irq Number.
 * @return void
 */
void Core_Hal_EnableIrq(IRQn_Type IrqNumber);

/*!
 * @brief Disable Irq.
 * @note Function ID: DES_CORE_API_201
 * @param[in] IrqNumber: Irq Number.
 * @return void
 */
void Core_Hal_DisableIrq(IRQn_Type IrqNumber);

/*!
 * @brief Get Irq Enable status.
 * @note  Function ID: DES_CORE_API_202
 * @param[in] IrqNumber: Irq Number.
 * @return boolean: Irq Enable status.
 *          FALSE: Interrupt status is not enable.
 *          TRUE: Interrupt status is enable.
 */
boolean Core_Hal_IsIrqEnable(IRQn_Type IrqNumber);

/*!
 * @brief Set Irq priority.
 * @note  Function ID: DES_CORE_API_203
 * @param[in] IrqNumber: Irq Number.
 * @param[in] Priority: Irq Priority.
 * @return void
 */
void Core_Hal_SetIrqPriority(IRQn_Type IrqNumber, uint32 Priority);

/*!
 * @brief Get Irq priority.
 * @note  Function ID: DES_CORE_API_204
 * @param[in] IrqNumber: Irq Number.
 * @return uint32: Irq priority
 */
uint32 Core_Hal_GetIrqPriority(IRQn_Type IrqNumber);

/*!
 * @brief Perform software reset.
 * @note  Function ID: DES_CORE_API_205
 * @return void
 */
void Core_Hal_PerformReset(void);

/*!
 * @brief Set pending Irq.
 * @note  Function ID: DES_CORE_API_206
 * @param[in] IrqNumber: Irq Number.
 * @return void
 */
void Core_Hal_SetPendingIrq(IRQn_Type IrqNumber);

/*!
 * @brief Clear pending Irq.
 * @note  Function ID: DES_CORE_API_207
 * @param[in] IrqNumber: Irq Number.
 * @return void
 */
void Core_Hal_ClearPendingIrq(IRQn_Type IrqNumber);

/*!
 * @brief Get pending Irq.
 * @note Function ID: DES_CORE_API_208
 * @param[in] IrqNumber: Irq Number.
 * @return boolean: Irq is pending or not.
 *          FALSE: Interrupt status is not active.
 *          TRUE: Interrupt status is active.
 */
boolean Core_Hal_IsIrqPending(IRQn_Type IrqNumber);

/*!
 * @brief Get Active Irq.
 * @note  Function ID: DES_CORE_API_209
 * @param[in] IrqNumber: Irq Number.
 * @return boolean: Irq is active or not.
 *          FALSE: Interrupt status is not active.
 *          TRUE: Interrupt status is active.
 */
boolean Core_Hal_IsIrqActive(IRQn_Type IrqNumber);

/*!
 * @brief Set Irq priority grouping.
 * @note Function ID: DES_CORE_API_210
 * @param[in] PriorityGroup: Irq Priority grouping.
 * @return void
 */
void Core_Hal_SetIrqPriorityGrouping(uint32 PriorityGroup);

/*!
 * @brief Get Irq priority grouping.
 * @note  Function ID: DES_CORE_API_211
 * @return Irq priority grouping.
 */
uint32 Core_Hal_GetIrqPriorityGrouping(void);

/**
 * @brief Get UUID from device.
 * @note  Function ID: DES_CORE_API_212
 * @param[out] uuidBuffer: UUID buffer
 * @return void
 */
void Core_Hal_GetUUID(uint32 *uuidBuffer);

/**
 * @brief Get Chip ID from device.
 * @note  Function ID: DES_CORE_API_213
 * @return uint32: Chip ID
 */
uint32 Core_Hal_GetChipID(void);

/**
 * @brief Enable NMI, after set pinmux.
 * @note  Function ID: DES_CORE_API_214
 * @param[in] En: enable state
 * @return void
 */
void Core_Hal_EnableNMI(boolean En);

#if (__FPU_PRESENT == 1U) && (__FPU_USED == 1U)
/*!
 * @brief Get FPU error information.
 * @note Function ID: DES_CORE_API_215
 * @param[in] ErrorPtr: Pointer to where to store the error information of FPU.
 * @return Hal_StatusType: Error status.
 *               STATUS_SUCCESS: no error occurred.
 *               STATUS_ERROR:   error detected.
 */
Hal_StatusType Core_Hal_GetFpuErrorInfo(Core_FpuErrorInfoType *ErrorPtr);

/*!
 * @brief Clear FPU error information.
 * @note Function ID: DES_CORE_API_216
 * @return void
 */
void Core_Hal_ClearFpuErrorInfo(void);

#endif

/*!
 * @brief Get the MBIST execute status.
 * @note Function ID: DES_CORE_API_217
 * @return Core_BistStatusType: BIST execute status.
 *               BIST_NORUN: BIST not run
 *               BIST_ERROR: Threre is a BIST fault occurred.
 *               BIST_OK: BIST success.
 *               BIST_BUSY: BIST isr unning
 */
Core_BistStatusType Core_Hal_GetMBistExecStatus(void);

#if defined(AC7840X) || defined(AC7842X)
/*!
 * @brief Get Flash Cache ECC error detail information.
 * @note Function ID: DES_CORE_API_218
 * @param[out] ErrorPtr: Pointer to where to store the error information of flash cache ECC.
 * @return Hal_StatusType: Error status.
 *               STATUS_SUCCESS: no error occurred.
 *               STATUS_ERROR:   error detected.
 */
Hal_StatusType Core_Hal_GetFlashCacheErrorInfo(Core_EccErrorInfoType *ErrorPtr);

/*!
 * @brief Clear flash Cache ECC error.
 * @note Function ID: DES_CORE_API_219
 * @return void
 */
void Core_Hal_ClearFlashCacheErrorInfo(void);

#endif /* AC7840X AC7842X */

#if defined (AC7843X)
/*!
 * @brief Enable CPU instruction cache.
 * @note Function ID: DES_CORE_API_220
 * @param[in] En: enable state
 * @return void
 */
void Core_Hal_EnableICache(boolean En);

/*!
 * @brief Invalid CPU instruction cache.
 * @note Function ID: DES_CORE_API_221
 * @return void
 */
void Core_Hal_InvalidICache(void);

/*!
 * @brief Get CPU instruction cache ECC error detail information.
 * @note Function ID: DES_CORE_API_222
 * @param[out] ErrorPtr: Pointer to where to store the error information of CPU instruction cache ECC.
 * @return Hal_StatusType: Error status.
 *               STATUS_SUCCESS: no error occurred.
 *               STATUS_ERROR:   error detected.
 */
Hal_StatusType Core_Hal_GetICacheErrorInfo(Core_EccErrorInfoType *ErrorPtr);

/*!
 * @brief Clear CPU instuction cache error detail information.
 * @note Function ID: DES_CORE_API_223
 * @return void
 */
void Core_Hal_ClearICacheErrorInfo(void);

/*!
 * @brief Enable CPU Data cache.
 * @note Function ID: DES_CORE_API_224
 * @param[in] En: enable state
 * @return void
 */
void Core_Hal_EnableDCache(boolean En);

/*!
 * @brief Invalid CPU Data cache.
 * @note Function ID: DES_CORE_API_225
 * @return void
 */
void Core_Hal_InvalidDCache(void);

/*!
 * @brief Get CPU data cache ECC error detail information.
 * @note Function ID: DES_CORE_API_226
 * @param[out] ErrorPtr: Pointer to where to store the error information of CPU data cache ECC.
 * @return Hal_StatusType: Error status.
 *               STATUS_SUCCESS: no error occurred.
 *               STATUS_ERROR:   error detected.
 */
Hal_StatusType Core_Hal_GetDCacheErrorInfo(Core_EccErrorInfoType *ErrorPtr);

/*!
 * @brief Clear CPU data cache error detail information.
 * @note Function ID: DES_CORE_API_227
 * @return void
 */
void Core_Hal_ClearDCacheErrorInfo(void);
#endif /* AC7843X */

/*!
 * @brief Initialize MCM module.
 * @note Function ID: DES_CORE_API_228
 * @param[in] ConfigPtr: Core Configuration.
 * @return void
 */
void Core_Hal_Init(const Core_ConfigType *ConfigPtr);

/*!
 * @brief De-Initialze MCM module.
 * @note Function ID: DES_CORE_API_229
 * @return void
 */
void Core_Hal_DeInit(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* CORE_HAL_H */
