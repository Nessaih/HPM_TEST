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
 * @file Adc_Hal.h
 * @brief This file provides all adc hal api.
 */
#ifndef ADC_HAL_H
#define ADC_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Adc_Hal_Types.h"

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/**
 * @brief init ADC module about Adc_InitConfigType struct to hw
 * @note Function ID: DES_ADC_API_000
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] Config: ADC init config args
 * @return void
 */
void Adc_Hal_Init(uint8 Instance, const Adc_InitConfigType *Config);

/**
 * @brief Deinitialize ADC module hw
 * @note Function ID: DES_ADC_API_001
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @return void
 */
void Adc_Hal_Deinit(uint8 Instance);

/**
 * @brief adc Config convert args to hw
 * @note Function ID: DES_ADC_API_002
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] Config: convert Config that need to Config
 * @return void
 */
void Adc_Hal_ConfigConverter(uint8 Instance, const Adc_ConverterConfigType *Config);

/**
 * @brief adc config the Channel args to the hw.
 * @note Function ID: DES_ADC_API_003
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] Config: configuration that need to apply
 * @return void
 */
void Adc_Hal_ConfigChannel(uint8 Instance, const Adc_ChanConfigType *Config);

/**
 * @brief: Apply the Group Mode configuration to the hardware.
 * @note Function ID: DES_ADC_API_004
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] Config: configuration that need to apply to the group
 * @return: void
 */
void Adc_Hal_ConfigGroup(uint8 Instance, const Adc_GroupConfigType *Config);

/**
 * @brief adc config the amo args to the hw.
 * @note Function ID: DES_ADC_API_005
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] Config: configuration that need to apply
 * @return void
 */
void Adc_Hal_ConfigAmo(uint8 Instance, const Adc_AmoConfigType *Config);

/**
 * @brief Start software trigger regular group conversion
 * @note Function ID: DES_ADC_API_006
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @return void
 */
void Adc_Hal_SwTriggerRegularConvert(uint8 Instance);

/**
 * @brief Start software trigger inject group conversion
 * @note Function ID: DES_ADC_API_007
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @return void
 */
void Adc_Hal_SwTriggerInjectConvert(uint8 Instance);

/**
 * @brief Get the ADC convertion result for the group sequence
 * @note Function ID: DES_ADC_API_008
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] SeqIndex: ADC group seqeunce index
 * @return ADC convertion result
 */
uint16 Adc_Hal_GetSeqResult(uint8 Instance, Adc_SequenceType SeqIndex);

#ifndef ADC_SDK_NON_EXTENDED_API
/**
 * @brief Get ADC Idle flag
 * @note Function ID: DES_ADC_API_009
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @return whether ADC is idle, -TRUE: ADC is not converting -FALSE: ADC is busy converting
 */
boolean Adc_Hal_GetIdleFlag(uint8 Instance);

/**
 * @brief Get convertion complete flag
 * @note Function ID: DES_ADC_API_010
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] SeqIndex: ADC group seqeunce index
 * @return whether ADC group seqeunce index complete, -TRUE: ADC convert complete -FALSE: ADC convert not complete
 */
boolean Adc_Hal_GetConvertCompleteFlag(uint8 Instance, Adc_SequenceType SeqIndex);

/**
 * @brief Clear convertion complete flag
 * @note Function ID: DES_ADC_API_011
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] SeqIndex: ADC group seqeunce index
 * @return void
 */
void Adc_Hal_ClearConvertCompleteFlag(uint8 Instance, Adc_SequenceType SeqIndex);

/**
 * @brief Get the trigger conflict flags
 * @note Function ID: DES_ADC_API_012
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @return whether ADC trigger conflict, -TRUE: ADC trigger conflict -FALSE: ADC trigger not conflict
 */
boolean Adc_Hal_GetTriggerConflictFlag(uint8 Instance);

/**
 * @brief Clear the trigger conflict flags
 * @note Function ID: DES_ADC_API_013
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @return void
 */
void Adc_Hal_ClearTriggerConflictFlag(uint8 Instance);
#endif

/**
 * @brief Initialize converter config struct default args
 * @note Function ID: DES_ADC_API_014
 * @note Service ID: none
 * @param [out] Config: converter config structure that need to be initialized
 * @return none
 */
void Adc_Hal_InitConverterStruct(Adc_ConverterConfigType *const Config);

#ifndef ADC_SDK_NON_EXTENDED_API
/**
 * @brief Initialize adc amo config struct default args
 * @note Function ID: DES_ADC_API_015
 * @note Service ID: none
 * @param [out] Config: adc amo Config structure that need to be initialized
 * @return none
 */
void Adc_Hal_InitGroupStruct(Adc_GroupConfigType *const Config);
#endif

/**
 * @brief Initialize adc channel config struct default args
 * @note Function ID: DES_ADC_API_016
 * @note Service ID: none
 * @param [out] Config: adc channel Config structure that need to be initialized
 * @return none
 */
void Adc_Hal_InitChanStruct(Adc_ChanConfigType *const Config);

#ifndef ADC_SDK_NON_EXTENDED_API
/*!
 * @brief initialize Analog Monitor config structure
 * @note Function ID: DES_ADC_API_017
 * @note Service ID: none
 * @param [out] Config: pointer to the config that need to be initialized
 * @return none
 */
void Adc_Hal_InitAMOStruct(Adc_AmoConfigType *const Config);
#endif

/**
 * @brief: Apply the Regular Group Mode configuration to the hardware.
 * @note Function ID: DES_ADC_API_18
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] Config: configuration that need to apply to the group
 * @return: void
 */
void Adc_Hal_ConfigRegularGroup(uint32 Instance, const Adc_GroupConfigType* Config);

/**
 * @brief: Apply the Inject Group Mode configuration to the hardware.
 * @note Function ID: DES_ADC_API_19
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] Config: configuration that need to apply to the group
 * @return: void
 */
void Adc_Hal_ConfigInjectGroup(uint32 Instance, const Adc_GroupConfigType* Config);

/**
 * @brief: Enable adc-dma control.
 * @note Function ID: DES_ADC_API_20
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] Enable: Enable flag.
 * @return: void
 */
void Adc_Hal_DmaEnable(uint32 Instance, boolean Enable);

#ifndef ADC_SDK_NON_EXTENDED_API
/**
 * @brief Get ADC group seqeunce index parity value
 * @note Function ID: DES_ADC_API_21
 * @note Service ID: none
 * @param[in] Instance: ADC Instance number
 * @param[in] SeqIndex ADC group seqeunce index
 * @return ADC group seqeunce index parity value
 */
uint8 Adc_Hal_GetParityVal(uint8 Instance, Adc_SequenceType SeqIndex);

/**
* @brief Get the adc base address
* @note Function ID: DES_ADC_API_022
* @note Service ID: none
* @param[in] Instance: ADC Instance number
* @return adc base address ptr
*/
ADC_Type *Adc_Hal_GetBase(uint8 Instance);

/**
* @brief Config autocalibration
* @note Function ID: DES_ADC_API_023
* @note Service ID: none
* @param[in] Instance: ADC Instance number
* @return adc base address ptr
*/
void Adc_Hal_ConfigAutoCalibration(uint8 Instance);

/**
* @brief Set the adc inject group offset
* @note Function ID: DES_ADC_API_024
* @note Service ID: none
* @param[in] Instance: ADC Instance number
* @param[in] SeqIndex: ADC group seqeunce index
* @param[in] Offset: ADC inject group offset value
* @return void
*/
void Adc_Hal_SetInjectOffset(uint8 Instance, Adc_SequenceType SeqIndex, uint16 Offset);

/*!
 * @brief Convert ADC value of T-Sensor to Temperature.
 * @note Function ID: DES_ADC_API_025
 * @note Service ID: none
 * @param[in] Value: ADC conversion result of internal T-Sensor
 * @param[in] RefVoltage: reference voltage of ADC, in mV
 * @return temperature value in celsius scale
 */
float32 Adc_Hal_ConvertToTemperature(const uint16 Value, const float32 RefVoltage);
#endif
#ifdef __cplusplus
}
#endif
#endif /*ADC_HAL_H*/

/* =============================================  EOF  ============================================== */
