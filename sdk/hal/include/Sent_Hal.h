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
 * @file Sent_Hal.h
 *
 * @brief This file provides sent integration functions interface.
 *
 */

#ifndef SENT_HAL_H
#define SENT_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Sent_Hal_Types.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
#ifndef SENT_SDK_NON_EXTENDED_API
/*!
 * @brief Initializes the SENT channel configuration structure with default values.
 *
 * @param[out] SentChannelConfig: Initializes a configuration structure received from the application
 *                                with default values
 * @return void
 */
void Sent_Hal_GetDefaultChannelConfig(Sent_ChannelConfigType *SentChannelConfig);

/*!
 * @brief Initializes the SENT module configuration structure with default values.
 *
 * @param[out] SentModuleConfig: Initializes a configuration structure received from the application
 *                               with default values
 * @return void
 */
void Sent_Hal_GetDefaultModuleConfig(Sent_ModuleConfigType *SentModuleConfig);
#endif

/*!
 * @brief initializes the SENT module
 *
 * @param[in] SentModuleConfig: sent module config pointer
 * @return void
 */
void Sent_Hal_Init(const Sent_ModuleConfigType *SentModuleConfig);

/*!
 * @brief deinitializes the SENT module
 *
 * @return void
 */
void Sent_Hal_Deinit(void);

/*!
 * @brief Set the SENT channel configuration
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] SentChannelConfig: sent channel config pointer
 * @return void
 */
void Sent_Hal_ConfigChannel(Sent_ChannelIdType ChannelId, const Sent_ChannelConfigType *SentChannelConfig);

/*!
 * @brief enable/disable SENT channel
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] Enable: enable or disbale
 * @return void
 */
void Sent_Hal_EnableChannel(Sent_ChannelIdType ChannelId, boolean Enable);

/*!
 * @brief SENT get channel state
 *
 * @param[in] ChannelId: sent channel ID
 * @return channel status
 */
Sent_ChannelStatusType Sent_Hal_GetChannelStatus(Sent_ChannelIdType ChannelId);

/*!
 * @brief Initializes the SENT channel Tx configuration.
 *
 * @param[in] ChannelId: sent channel ID
 * @param[in] SentChannelTxConfig: sent channel Tx configuration
 * @return void
 */
void Sent_Hal_InitChannelTxCtrl(Sent_ChannelIdType ChannelId, const Sent_TransmitCtrlType *SentChannelTxConfig);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* SENT_HAL_H */

/* =============================================  EOF  ============================================== */
