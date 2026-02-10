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
 * @file Acmp_Hal.h
 *
 * @brief This file provides analog comparator module integration functions interfaces.
 *
 */

#ifndef ACMP_HAL_H
#define ACMP_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Acmp_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get default configuration structure.
 * @note Function ID: DES_ACMP_API_008
 * @param [out] Config: ACMP configuration structure
 * @return void
 */
void Acmp_Hal_GetDefaultConfig(Acmp_ModuleType *Config);

/*!
 * @brief Reset all ACMP registers. Need to be called after ACMP clock is applied and reset is deasserted.
 * @note Function ID: DES_ACMP_API_009
 * @param [in] Instance: ACMP Instance number
 * @return void
 */
void Acmp_Hal_Reset(uint8 Instance);
#endif

/*!
 * @brief Configure all comparator function with the given configuration structure.
 * @note Function ID: DES_ACMP_API_000
 * @param [in] Instance: ACMP Instance number
 * @param [in] Config: ACMP configuration structure that need to apply
 * @return void
 */
void Acmp_Hal_Init(uint8 Instance, const Acmp_ModuleType *const Config);

/*!
 * @brief Reset ACMP module and close the clock source.
 * @note Function ID: DES_ACMP_API_001
 * @param [in] Instance: ACMP Instance number
 * @return void
 */
void Acmp_Hal_Deinit(uint8 Instance);

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get the current ACMP configuration.
 * @note Function ID: DES_ACMP_API_001
 * @param [in] Instance: ACMP Instance number
 * @param [out] Config: ACMP configuration structure that need to fill with current configuration
 * @return void.
 */
void Acmp_Hal_GetConfigAll(uint8 Instance, Acmp_ModuleType *const Config);
#endif

/*!
 * @brief Get ACMP normal mode output data.
 * @note Function ID: DES_ACMP_API_002
 * @param [in] Instance: ACMP Instance number
 * @return output data.
 */
uint8 Acmp_Hal_GetOutputData(uint8 Instance);

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get ACMP output Status flags.
 * @note Function ID: DES_ACMP_API_004
 * @param[in] Instance: ACMP Instance number
 * @return output Status flags.
 */
uint8 Acmp_Hal_GetOutputFlags(uint8 Instance);

/*!
 * @brief Clear ACMP output flags Status.
 * @note Function ID: DES_ACMP_API_005
 * @param[in] Instance: ACMP Instance number
 * @return void.
 */
void Acmp_Hal_ClearOutputFlags(uint8 Instance);
#endif

/*!
 * @brief Get polling mode compare data.
 * @note Function ID: DES_ACMP_API_003
 * @param[in] Instance: ACMP Instance number
 * @return polling mode input channel compare data.
 */
uint16 Acmp_Hal_GetPollingData(uint8 Instance);

#ifndef ACMP_SDK_NON_EXTENDED_API
/*!
 * @brief Get polling mode Status flags.
 * @note Function ID: DES_ACMP_API_006
 * @param[in] Instance: ACMP Instance number
 * @return polling mode input channel Status.
 */
uint16 Acmp_Hal_GetPollingFlags(uint8 Instance);

/*!
 * @brief Clear polling mode Status flags.
 * @note Function ID: DES_ACMP_API_007
 * @param[in] Instance: ACMP Instance number
 * @return void.
 */
void Acmp_Hal_ClearInputFlags(uint8 Instance);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ACMP_HAL_H */

/* =============================================  EOF  ============================================== */
