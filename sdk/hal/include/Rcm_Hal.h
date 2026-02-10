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
 * @file Rcm_Hal.h
 *
 * @brief This file provides extern Hal Rcm api.
 *
 */

#ifndef RCM_HAL_H
#define RCM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* endif of __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/*!< rcm reset filter max value     */
#define RESET_FILTER_MAX_VALUE (127UL)

/** @brief rcm irq control internal(STD_ON) or external(STD_OFF)*/
#define RCM_IRQ_CONTROL_INTERNAL           (STD_ON)
/*===================================================ENUMS==========================================*/

/*!
 * @brief Reset delay time enumeration.
 */
typedef enum
{
    RCM_RESET_DELAY_8_128KHz = 0U, /*!< Reset delay 8   cycles * 128K     */
    RCM_RESET_DELAY_32_128KHz, /*!< Reset delay 32  cycles * 128K     */
    RCM_RESET_DELAY_128_128KHz, /*!< Reset delay 128 cycles * 128K     */
    RCM_RESET_DELAY_512_128KHz /*!< Reset delay 512 cycles * 128K     */
} Rcm_DelayType;

typedef enum
{

    RCM_RESET_ID_UART0 = 0U, /*!< Soft reset UART0                  */
    RCM_RESET_ID_UART1 = 1U, /*!< Soft reset UART1                  */
    RCM_RESET_ID_UART2 = 2U, /*!< Soft reset UART2                  */
    RCM_RESET_ID_UART3 = 3U, /*!< Soft reset UART3                  */
#if defined (AC7843X)
    RCM_RESET_ID_UART4 = 4U, /*!< Soft reset Uart4                  */
    RCM_RESET_ID_UART5 = 5U, /*!< Soft reset Uart5                  */
    RCM_RESET_ID_UART6 = 6U, /*!< Soft reset Uart6                  */
    RCM_RESET_ID_UART7 = 7U, /*!< Soft reset Uart7                  */
#endif /* endif of AC7843X */
#if defined (AC7840X) || defined (AC7842X)
    RCM_RESET_ID_SPI0 = 6U, /*!< Soft reset SPI0                   */
    RCM_RESET_ID_SPI1 = 7U, /*!< Soft reset SPI1                   */
    RCM_RESET_ID_SPI2 = 8U, /*!< Soft reset SPI2                   */
#endif /* endif of AC7840X AC7842X */
#if defined (AC7842X)
    RCM_RESET_ID_SPI3 = 24U, /*!< Soft reset SPI3                   */
#elif defined (AC7843X) /* endif of AC7842X */
    RCM_RESET_ID_SPI0 = 8U, /*!< Soft reset SPI0                   */
    RCM_RESET_ID_SPI1 = 9U, /*!< Soft reset SPI1                   */
    RCM_RESET_ID_SPI2 = 10U, /*!< Soft reset SPI2                   */
    RCM_RESET_ID_SPI3 = 11U, /*!< Soft reset SPI3                   */
    RCM_RESET_ID_SPI4 = 12U, /*!< Soft reset SPI4                   */
#endif /* endif of AC7843X */
#if defined (AC7840X) || defined (AC7842X)
    RCM_RESET_ID_I2C0 = 9U, /*!< Soft reset I2C0                   */
#endif /* endif of AC7840X AC7842X */
#if defined (AC7842X)
    RCM_RESET_ID_I2C1 = 10U, /*!< Soft reset SPI3                   */
#elif defined (AC7843X) /* endif of AC7842X */
    RCM_RESET_ID_I2C0 = 16U, /*!< Soft reset I2C0                   */
    RCM_RESET_ID_I2C1 = 17U, /*!< Soft reset I2C1                   */
    RCM_RESET_ID_I2C2 = 18U, /*!< Soft reset I2C2                   */
#endif /* endif of AC7843X */
#if defined (AC7840X) || defined (AC7842X)
    RCM_RESET_ID_PCT = 11U, /*!< Soft reset PCT                    */
    RCM_RESET_ID_PWM0 = 15U, /*!< Soft reset PWM0                   */
    RCM_RESET_ID_PWM1 = 16U, /*!< Soft reset PWM1                   */
    RCM_RESET_ID_PWM2 = 17U, /*!< Soft reset PWM2                   */
    RCM_RESET_ID_PWM3 = 18U, /*!< Soft reset PWM3                   */
    RCM_RESET_ID_PWM4 = 19U, /*!< Soft reset PWM4                   */
    RCM_RESET_ID_PWM5 = 20U, /*!< Soft reset PWM5                   */
#elif defined (AC7843X) /* endif of AC7840X AC7842X */
    RCM_RESET_ID_PCT = 20U, /*!< Soft reset PCT                    */
    RCM_RESET_ID_PWM0 = 24U, /*!< Soft reset PWM0                   */
    RCM_RESET_ID_PWM1 = 25U, /*!< Soft reset PWM1                   */
    RCM_RESET_ID_PWM2 = 26U, /*!< Soft reset PWM2                   */
    RCM_RESET_ID_PWM3 = 27U, /*!< Soft reset PWM3                   */
    RCM_RESET_ID_PWM4 = 28U, /*!< Soft reset PWM4                   */
    RCM_RESET_ID_PWM5 = 29U, /*!< Soft reset PWM5                   */
    RCM_RESET_ID_PWM6 = 30U, /*!< Soft reset PWM6                   */
    RCM_RESET_ID_PWM7 = 31U, /*!< Soft reset PWM7                   */
#endif /* endif of AC7843X */
    RCM_RESET_ID_DMA = 34U, /*!< Soft reset DMA                    */
    RCM_RESET_ID_GPIO = 37U, /*!< Soft reset GPIO                   */
    RCM_RESET_ID_WDG = 38U, /*!< Soft reset WDG                    */
    RCM_RESET_ID_EWDG = 39U, /*!< Soft reset EWDG                   */
    RCM_RESET_ID_CRC = 40U, /*!< Soft reset CRC                    */
    RCM_RESET_ID_CAN0 = 41U, /*!< Soft reset CAN0                   */
    RCM_RESET_ID_CAN1 = 42U, /*!< Soft reset CAN1                   */
    RCM_RESET_ID_CAN2 = 43U, /*!< Soft reset CAN2                   */
    RCM_RESET_ID_CAN3 = 44U, /*!< Soft reset CAN3                   */
#if defined (AC7843X) || defined (AC7842X)
    RCM_RESET_ID_CAN4 = 45U, /*!< Soft reset CAN4                   */
    RCM_RESET_ID_CAN5 = 46U, /*!< Soft reset CAN5                   */
#endif /* endif of AC7843X AC7842X */
#if defined (AC7843X)
    RCM_RESET_ID_CAN_CTRL = 47U, /*!< Soft reset CAN CTRL               */
#endif /* endif of AC7843X */
    RCM_RESET_ID_CTU = 65U, /*!< Soft reset CTU                    */
#if defined (AC7840X) ||  defined (AC7842X)
    RCM_RESET_ID_ACMP0 = 72U, /*!< Soft reset ACMP                  */
#elif defined (AC7843X) /* endif of AC7840X AC7842X */
    RCM_RESET_ID_ACMP0 = 71U, /*!< Soft reset ACMP                   */
    RCM_RESET_ID_ACMP1 = 72U, /*!< Soft reset ACMP                   */
#endif /* endif of AC7843X */
    RCM_RESET_ID_PDT0 = 73U, /*!< Soft reset PDT0                   */
    RCM_RESET_ID_PDT1 = 74U, /*!< Soft reset PDT1                   */
    RCM_RESET_ID_ADC0 = 75U, /*!< Soft reset ADC0                   */
    RCM_RESET_ID_ADC1 = 76U, /*!< Soft reset ADC1                   */
    RCM_RESET_ID_TIMER = 77U, /*!< Soft reset TIMER                  */
    RCM_RESET_ID_EIO = 78U, /*!< Soft reset EIO                    */
#if defined (AC7843X) || defined (AC7842X)
    RCM_RESET_ID_SENT = 79U, /*!< Soft reset SENT                   */
#endif /* endif of AC7843X AC7842X */
    RCM_RESET_ID_MAX
} Rcm_ResetIDType;

typedef enum
{
    RCM_RESET_STATE_ASSERT = 0U, /*!< Assert                           */
    RCM_RESET_STATE_DEASSERT = 1U, /*!< Deassert                         */
} Rcm_ResetStateType;
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*PRQA S 3630 ++ # use in different file in the future*/
/*!
 * @brief Rcm module configuration structure.
 */
typedef struct
{
    uint32 InterruptSource; /*!< sources can generate interrupt before reset */
    uint32 ResetSource; /*!< sources can generate reset                  */
    uint8 FilterValue; /*!< filter value of external reset pid          */
    Hal_CallbackType RcmCallback; /*!< callback function                         */
    Rcm_DelayType DelayTime; /*!< delay time before reset                     */
} Rcm_ConfigType;
/*PRQA S 3630 -- # use in different file in the future*/
/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Initialize reset module.
 * @note  Function ID: DES_MCU_API_401
 * @param[in] ConfigPtr: Rcm configuration.
 * @return void
 */
void Rcm_Hal_Init(const Rcm_ConfigType *ConfigPtr);

/*!
 * @brief Get reset statue.
 * @note  Function ID: DES_MCU_API_402
 * @return value of reset and interrupt status
 *         - BIT0:RCM_RESET_STATUS_POR_RST
 *         - BIT1:RCM_RESET_STATUS_LVR_RST
 *         - BIT2:RCM_RESET_STATUS_SW_RST
 *         - BIT3:RCM_RESET_STATUS_LOCKUP_RST
 *         - BIT4:RCM_RESET_STATUS_ACK_ERR_RST
 *         - BIT5:RCM_RESET_STATUS_WDG_RST
 *         - BIT7:RCM_RESET_STATUS_XOSC_LOSS_RST
 *         - BIT8:RCM_RESET_STATUS_PLL_UNLOCK_RST
 *         - BIT9:RCM_RESET_STATUS_VHSI_LOSS_RST
 *         - BIT10:RCM_RESET_STATUS_EXT_RST
 *         - BIT11:RCM_RESET_STATUS_SMU_ERR_RST
 *         - BIT12:RCM_RESET_STATUS_ECC2_ERR_RST
 */
uint32 Rcm_Hal_GetResetStatus(void);

/*!
 * @brief Clear All reset status.
 * @note  Function ID:DES_MCU_API_403
 * @return void
 */
void Rcm_Hal_ClearResetStatus(void);

/*!
 * @brief Assert or deassert the reset.
 * @note  Function ID:DES_MCU_API_404
 * @param[in] ResetId: Reset ID
 * @param[in] ResetState: Reset state, assert or deassert
 * @return void
 */
void Rcm_Hal_SetResetState(Rcm_ResetIDType ResetId, Rcm_ResetStateType ResetState);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* RCM_HAL_H */
/*============================================EOF===================================================*/
