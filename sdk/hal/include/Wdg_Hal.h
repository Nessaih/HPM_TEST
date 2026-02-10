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
 * @file Wdg_Hal.h
 *
 * @brief This file provides extern WDG HAL API.
 *
 */

#ifndef WDG_HAL_H
#define WDG_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*===============================================  INCLUDE FILES  ============================================*/
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/*!
 * @brief WDG Clock source type
 */
typedef enum
{
    WDG_CLOCK_LSI = 0U, /*!< WDG uses LSI as clock source.*/
    WDG_CLOCK_HSI, /*!< WDG uses HSI as clock source.*/
    WDG_CLOCK_HSE, /*!< WDG uses HSE as clock source.*/
    WDG_CLOCK_BUS, /*!< WDG uses Bus Clock as clock source.*/
    WDG_CLOCK_INVALID, /*!< Invalid clock source.*/
} Wdg_ClockSourceType;

/** @brief  Prescaler enable config for WDG clock.*/
#define WDG_CONFIG_PRESCALER_EN 0x01U

/** @brief  Enable WDG in Debug mode.*/
#define WDG_CONFIG_DBG_EN       0x02U

/** @brief  Enable WDG fast test mode.*/
#define WDG_CONFIG_FAST_TEST    0x04U

/** @brief  Enable WDG in Low Power mode.*/
#define WDG_CONFIG_LP_EN        0x08U

/** @brief  Enable WDG window mode.*/
#define WDG_CONFIG_WIN_EN       0x10U

/** @brief  Enable all configuraiton of WDG.*/
#define WDG_CONFIG_ALL (WDG_CONFIG_PRESCALER_EN | WDG_CONFIG_DBG_EN | \
                        WDG_CONFIG_FAST_TEST | WDG_CONFIG_LP_EN | WDG_CONFIG_WIN_EN)

/*!
 * @brief Defines the  configuration structure for WDG HAL
 */
typedef struct
{
    Wdg_ClockSourceType ClockSource; /*!< Clock Source Type*/
    uint32 WindowValue; /*!< Window Value in ms*/
    uint32 TimeoutValue; /*!< Timeout Value in ms*/
    uint8 Config; /*!< WDG configuration. It is conbinations (bitwise OR) of WDG_CONFIG_xxx.*/
} Wdg_HalConfigType;

/* ========================================== LOCAL VARIABLES =========================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**  @brief Initializes WDG hardware.
 *   @note Function ID: DES_WDG_API_201
 *   @param [in] ConfigPtr   : Pointer to WDG hardware setting. It can't be NULL_PTR.
 *   @return void
 *   @see struct Wdg_HalConfigType
 */
void Wdg_Hal_Init(const Wdg_HalConfigType *ConfigPtr);

/**  @brief De-initializes (disable) WDG hardware.
 *   @note Function ID: DES_WDG_API_202
 *   @return void
 */
void Wdg_Hal_DeInit(void);

/**  @brief Feed WDG hardware.  Its counter will be reset.
 *   @note Function ID: DES_WDG_API_203
 *   @return void
 */
void Wdg_Hal_Feed(void);

#ifndef WDG_SDK_NON_EXTENDED_API
/*!
 * @brief Install WDG interrupt callback function.
 * @note Function ID: DES_WDG_API_204
 * @param [in] Func: The pointer to  callback function to be installed
 * @param [in] Args: The parameter of the callback function
 * @return void
 */
void Wdg_Hal_InstallCallback
(
    const Hal_CallbackType Func,
    void *Args
);
#endif
/*!
 * @brief Get WDG freqency of Clock source in KHz.
 * @note Function ID: DES_WDG_API_206
 * @param [in] ClockSource:  ClockSource of WDG
 * @return The frequency(in KHz) of WDG.
 * @see  Wdg_ClockSourceType
 */
uint32 Wdg_Hal_GetFrequency(Wdg_ClockSourceType ClockSource);

#ifndef WDG_SDK_NON_EXTENDED_API
/**
 * @brief  Enable WDG interrupt.
 * @note Function ID: DES_WDG_API_207
 * @note Service ID: none
 * @param [in] Enable: TRUE: Enable WDG interrupt, FALSE(0):  Disable WDG interrupt.
 * @return void
 */
void Wdg_Hal_EnableInterrupt(boolean Enable);
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* WDG_HAL_H */
/*============================================EOF===================================================*/
