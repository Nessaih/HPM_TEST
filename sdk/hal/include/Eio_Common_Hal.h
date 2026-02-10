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
 * @file Eio_Common_Hal.h
 *
 * @brief This file provides eio comm integration functions interfaces.
 *
 */

#ifndef EIO_COMMON_HAL_H
#define EIO_COMMON_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================  Includes  =========================================== */
#include "Eio_Hal_Types.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */
/* Table of base addresses for EIO instances. */
extern EIO_Type * const EioBase[EIO_INSTANCE_COUNT];

extern const Ckgen_ClkIdType Eio_HalClock[EIO_INSTANCE_COUNT];

extern const Dma_RequestSourceType EioDMASrc[EIO_INSTANCE_COUNT][EIO_MAX_SHIFTER_COUNT];

/* ====================================  Functions declaration  ===================================== */
/**
 * @brief Initializes the EIO device
 * @note Function ID: DES_EIO_API_004
 * @param [in] Instance: EIO peripheral instance number
 * @return void
 */
void Eio_Hal_InitDevice(uint8 Instance);

/**
 * @brief De-initializes the EIO device
 * @note Function ID: DES_EIO_API_005
 * @param [in] Instance: EIO peripheral instance number
 * @return void
 */
void Eio_Hal_DeinitDevice(uint8 Instance);

/**
 * @brief Resets the EIO device
 * @note Function ID: DES_EIO_API_006
 * @param [in] Instance: EIO peripheral instance number
 * @return void
 */
void Eio_Hal_Reset(uint8 Instance);

/**
 * @brief Initializes an instance of EIO driver
 * @note Function ID: DES_EIO_API_007
 * @param [in] Instance: EIO peripheral instance number
 * @param [in] Driver: Pointer to the EIO common driver context structure
 * @return Error or success status returned by API
 */
Hal_StatusType Eio_Hal_InitDriver(uint8 Instance, Eio_CommonStateType *Driver);

/**
 * @brief De-initializes an instance of EIO driver
 * @note Function ID: DES_EIO_API_008
 * @param [in] Driver: Pointer to the EIO common driver context structure
 * @return void
 */
void Eio_Hal_DeinitDriver(const Eio_CommonStateType *Driver);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EIO_COMMON_HAL_H */

/* =============================================  EOF  ============================================== */
