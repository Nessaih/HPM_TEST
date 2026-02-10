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
 * @file Pbr_Hal.h
 *
 * @brief pbr hal define.
 */

#ifndef PBR_HAL_H
#define PBR_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */
/*!
 * @brief pbr master id
 */
typedef enum
{
    PBR_MASTER_CORE = 0x0U,
    PBR_MASTER_DEBUGGER,
    PBR_MASTER_DMA,
#if defined (AC7843X)
    PBR_MASTER_HSM,
#endif /* AC7843X */
    PBR_MASTER_MAX
} Pbr_MasterIdType;

/*!
 * @brief pbr peripheral id
 */
typedef enum
{
    PBR_PERIPH_PWM0 = 0U,
    PBR_PERIPH_PWM1,
    PBR_PERIPH_PWM2,
    PBR_PERIPH_PWM3,
    PBR_PERIPH_PWM4,
    PBR_PERIPH_PWM5,
#if defined (AC7843X)
    PBR_PERIPH_PWM6,
    PBR_PERIPH_PWM7,
#endif /* AC7843X */
    PBR_PERIPH_CRC = 8U,
    PBR_PERIPH_GPIO,
    PBR_PERIPH_PDT0,
    PBR_PERIPH_PDT1,
    PBR_PERIPH_MPU,
    PBR_PERIPH_EIM,
    PBR_PERIPH_SMU,
    PBR_PERIPH_CKGEN = 32U,
    PBR_PERIPH_RESERVE33,
    PBR_PERIPH_FLASH,
    PBR_PERIPH_ADC0,
    PBR_PERIPH_ADC1,
    PBR_PERIPH_ACMP0,
#if defined (AC7843X)
    PBR_PERIPH_CTU,
#endif /* AC7843X */
#if defined (AC7840X) || defined (AC7842X)
    PBR_PERIPH_CTU,
    PBR_PERIPH_CAN0,
    PBR_PERIPH_CAN1 = 40U,
    PBR_PERIPH_CAN2,
    PBR_PERIPH_CAN3,
#endif /* AC7840X AC7842X */
#if defined (AC7842X)
    PBR_PERIPH_CAN4,
    PBR_PERIPH_CAN5,
#endif /* AC7842X */
    PBR_PERIPH_SPM = 45U,
    PBR_PERIPH_RTC,
    PBR_PERIPH_EIO,
    PBR_PERIPH_WDG = 48U,
    PBR_PERIPH_EWDG,
    PBR_PERIPH_SPI0,
    PBR_PERIPH_SPI1,
    PBR_PERIPH_SPI2,
#if defined (AC7842X)
    PBR_PERIPH_SPI3,
#endif /* AC7842X */
    PBR_PERIPH_I2C0,
#if defined (AC7842X)
    PBR_PERIPH_I2C1,
#endif /* AC7842X */
#if defined (AC7843X)
    PBR_PERIPH_ACMP1 = 56U,
    PBR_PERIPH_SPI3,
    PBR_PERIPH_SPI4,
    PBR_PERIPH_I2C1,
    PBR_PERIPH_I2C2,
#endif /* AC7843X */
    PBR_PERIPH_TIMER = 64U,
    PBR_PERIPH_DMA,
    PBR_PERIPH_UART0,
    PBR_PERIPH_UART1,
    PBR_PERIPH_UART2,
    PBR_PERIPH_UART3,
    PBR_PERIPH_PCT = 72U,
    PBR_PERIPH_CMU,
#if defined (AC7842X)
    PBR_PERIPH_SENT = 77U,
#endif /* AC7842X */
#if defined (AC7843X)
    PBR_PERIPH_MRAM0 = 80U,
    PBR_PERIPH_MRAM1,
    PBR_PERIPH_UART4,
    PBR_PERIPH_UART5,
    PBR_PERIPH_UART6,
    PBR_PERIPH_UART7,
    PBR_PERIPH_CAN0,
    PBR_PERIPH_CAN1,
    PBR_PERIPH_CAN2 = 88U,
    PBR_PERIPH_CAN3,
    PBR_PERIPH_CAN4,
    PBR_PERIPH_CAN5,
    PBR_PERIPH_CAN_CTRL,
    PBR_PERIPH_SENT,
#endif /* AC7843X */
    PBR_PERIPH_MAX
} Pbr_PeriphIdType;

/*PRQA S 3630 ++ # use in different file in the future*/
/*!
 * @brief pbr master configure structure
 */
typedef struct
{
    uint8 Value;/* register value */
    Pbr_MasterIdType MasterId;
} Pbr_MasterCfgType;

/*!
 * @brief pbr peripheral configure structure
 */
typedef struct
{
    uint8 Value;/* register value */
    Pbr_PeriphIdType PeriphId;
} Pbr_PeriphCfgType;

/*!
 * @brief pbr configure structure
 */
typedef struct
{
    uint8 MasterCfgCnt;
    uint8 PeriphCfgCnt;
    const Pbr_MasterCfgType *MasterCfg;
    const Pbr_PeriphCfgType *PeriphCfg;
} Pbr_CfgType;
/*PRQA S 3630 -- # use in different file in the future*/
/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief Initialize pbr master and peripheral configuration
 * @note Function ID: DES_MCL_API_301
 * @param[in] Cfg: the pointer to the Pbr_CfgType structure
 * @return void
 */
void Pbr_Hal_Init(const Pbr_CfgType *Cfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* PBR_HAL_H */
/* =============================================  EOF  ============================================== */
