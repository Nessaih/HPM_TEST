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
 * @file Gpio_Hal.h
 *
 * @brief This file provides extern Hal Gpio api.
 *
 */

#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*===============================================  INCLUDE FILES  ============================================*/
#include "Gpio_Hal_Types.h"
/* ============================================  DEFINES AND MACROS  ============================================ */
/** @brief PORTA of instances module */
#define PORTID_A          (0u)
/** @brief PORTB of instances module */
#define PORTID_B          (1u)
/** @brief PORTC of instances module */
#define PORTID_C          (2u)
/** @brief PORTD of instances module */
#define PORTID_D          (3u)
/** @brief PORTE of instances module */
#define PORTID_E          (4u)
/* ============================================= TYPEDEFS ================================================ */

/* ========================================== LOCAL VARIABLES =========================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/**
 * @brief Initializes the gpio with the given configuration structure.
 * @note  Function ID : DES_PORT_API_202
 * @note  Service ID : NA
 * @param[in] PinCount: The number of configured pins in structure
 * @param[in] Config: The configuration structure
 * @return void
 */
void Gpio_Hal_ChannelInit(uint8 PinCount, const Gpio_Hal_SettingsConfigType Config[]);

/**
 * @brief Configures the internal pull-up/down resistor.
 * @note  Function ID : DES_PORT_API_203
 * @note  Service ID : NA
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] PullConfig: The pull pull-up/down resistor configuration
 * @return void
 */
void Gpio_Hal_SetPullSel(uint8 PortId, uint8 ChannelId, Port_Hal_PullConfigType PullConfig);

/**
 * @brief Configures the pin Mux.
 * @note  Function ID : DES_PORT_API_204
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] Mux: Pin Mux selection
 * @return void
 */
void Gpio_Hal_SetMuxMode(uint8 PortId, uint8 ChannelId, Port_Hal_MuxType Mux);

/**
 * @brief Configures the port pin interrupt/DMA request.
 * @note  Function ID : DES_PORT_API_205
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] IntConfig: Interrupt configuration
 * @return function execution status 0 means SUCCESS,other means fail
 */
Hal_StatusType Gpio_Hal_SetPinIntSel(uint8 PortId, uint8 ChannelId, Port_Hal_InterruptConfigType IntConfig);

/**
 * @brief Gets the current port pin interrupt/DMA request configuration.
 * @note  Function ID : DES_PORT_API_206
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @return Interrupt configuration
 */
Port_Hal_InterruptConfigType Gpio_Hal_GetPinIntSel(uint8 PortId, uint8 ChannelId);

/**
 * @brief Enables digital filter for digital pin Mux.
 * @note  Function ID : DES_PORT_API_207
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] En: Digital Filter enable/disalbe
 *                 0:diable
 *                 1:enable
 * @return void
 */
#ifndef GPIO_SDK_NON_EXTENDED_API
void Gpio_Hal_EnableDigitalFilter(uint8 PortId, uint8 ChannelId, boolean En);
#endif

/**
 * @brief Configures digital filter for port with given configuration.
 * @note  Function ID : DES_PORT_API_208
 * @param[in] PortId: port id
 * @param[in] Config: The digital filter configuration struct
 * @return void
 */
void Gpio_Hal_ConfigDigitalFilter(uint8 PortId, const Port_Hal_DigitalFilterCfgType *Config);

/**
 * @brief Get the pins directions configuration for a pin.
 * @note  Function ID : DES_PORT_API_209
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @return the pin direction input or onput
 */
#ifndef GPIO_SDK_NON_EXTENDED_API
Port_Hal_DataDirectionType Gpio_Hal_GetPinDirection(uint8 PortId, uint8 ChannelId);
#endif

/**
 * @brief Get the pins directions configuration for a port.
 * @note  Function ID : DES_PORT_API_210
 * @param[in] PortId: port id
 * @return GPIO directions: Each bit represents one pin (LSB is pin 0, MSB is
 *     pin 31). For each bit:
 *     - 0: corresponding pin is set to input
 *     - 1: corresponding pin is set to output
 */
Gpio_Hal_ChannelType Gpio_Hal_GetPinsDirection(uint8 PortId);

/**
 * @brief Configure the direction for a certain pin from a port.
 * @note  Function ID : DES_PORT_API_211
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] Direction: The pin direction:
 *     - GPIO_INPUT_DIRECTION: corresponding pin is set to input
 *     - GPIO_OUTPUT_DIRECTION: corresponding pin is set to output
 *     - GPIO_UNSPECIFIED_DIRECTION:corresponding pin is not set input and output
 * @return void
 */
void Gpio_Hal_SetPinDirection(uint8 PortId, uint8 ChannelId, Port_Hal_DataDirectionType Direction);

/**
 * @brief Set the pins directions configuration for a port.
 * @note  Function ID : DES_PORT_API_212
 * @param[in] PortId: Port id
 * @param[in] PinsDir: Pin mask where each bit represents one pin (LSB
 *         is pin 0, MSB is pin 31). For each bit:
 *     - 0: corresponding pin is set to input
 *     - 1: corresponding pin is set to output
 * @return void
 */
#ifndef GPIO_SDK_NON_EXTENDED_API
void Gpio_Hal_SetPinsDirection(uint8 PortId, uint32 PinsDir);
#endif

/**
 * @brief Write a pin of a port with a given value.
 * @note  Function ID : DES_PORT_API_213
 * @param[in] PortId: Port id
 * @param[in] ChannelId: port channel id
 * @param[in] Value: Pin value to be written
 *         - 0: corresponding pin is set to low
 *         - 1: corresponding pin is set to high
 * @return void
 */
void Gpio_Hal_WritePin(uint8 PortId, uint8 ChannelId, Gpio_Hal_LevelType Value);

/**
 * @brief Write all pins of a port.
 * @note  Function ID : DES_PORT_API_214
 * @param[in] PortId: Port id
 * @param[in] Pins: Pin mask to be written
 *         - 0: corresponding pin is set to low
 *         - 1: corresponding pin is set to high
 * @return void
 */
void Gpio_Hal_WritePins(uint8 PortId, Gpio_Hal_ChannelType Pins);

/**
 * @brief Get the current output level configuration from a pin.
 * @note  Function ID : DES_PORT_API_219
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @return the pin output level
 */
#ifndef GPIO_SDK_NON_EXTENDED_API
Gpio_Hal_LevelType Gpio_Hal_GetPinOutputLevel(uint8 PortId, uint8 ChannelId);
#endif

/**
 * @brief Get the current output level configuration from a port.
 * @note  Function ID : DES_PORT_API_220
 * @param[in] PortId: Port id
 * @return GPIO outputs level configuration: Each bit represents one pin
 *      (LSB is pin 0, MSB is pin 31). For each bit:
 *      - 0: corresponding pin is set to low
 *      - 1: corresponding pin is set to high
 */
Gpio_Hal_ChannelType Gpio_Hal_GetPinsOutputLevel(uint8 PortId);

/**
 * @brief Toggle a pin output level.
 * @note  Function ID : DES_PORT_API_217
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @return void
 */
#ifndef GPIO_SDK_NON_EXTENDED_API
void Gpio_Hal_TogglePin(uint8 PortId, uint8 ChannelId);
#endif

/**
 * @brief Toggle pins output level.
 * @note  Function ID : DES_PORT_API_218
 * @param[in] PortId: PortId id
 * @param[in] Pins: Pin mask of bits to be toggled.  Each bit represents one pin (LSB
 *         is pin 0, MSB is pin 31). For each bit:
 *         - 0: corresponding pin is unaffected
 *         - 1: corresponding pin is toggled
 * @return void
 */
void Gpio_Hal_TogglePins(uint8 PortId, Gpio_Hal_ChannelType Pins);

/**
 * @brief Read a pin input level.
 * @note  Function ID : DES_PORT_API_215
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @return GPIO inputs level
 */
#ifndef GPIO_SDK_NON_EXTENDED_API
Gpio_Hal_LevelType Gpio_Hal_ReadPin(uint8 PortId, uint8 ChannelId);
#endif

/**
 * @brief Read pins input level.
 * @note  Function ID : DES_PORT_API_216
 * @param[in] PortId: PortId id
 * @return GPIO inputs level: Each bit represents one pin (LSB is pin 0, MSB is pin
 *      31). For each bit:
 *      - 0: corresponding pin is read as low
 *      - 1: corresponding pin is read as high
 */
Gpio_Hal_ChannelType Gpio_Hal_ReadPins(uint8 PortId);

/**
 * @brief Set a pin to HighZ.
 * @note  Function ID : DES_PORT_API_221
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] En: HighZ enable/disalbe
 *                 0:diable
 *                 1:enable
 * @return void
 */
#ifndef GPIO_SDK_NON_EXTENDED_API
void Gpio_Hal_SetHighZ(uint8 PortId, uint8 ChannelId, boolean En);
#endif

/**
 * @brief Install GPIO interrupt callback function.
 * @note  Function ID : DES_PORT_API_222
 * @param[in] PortId: GPIO PortId
 * @param[in] Function: The GPIO interrupt callback function to be installed
 * @return void
 */
void Gpio_Hal_InstallCallback(uint8 PortId, const Port_CallbackType Function);

/**
 * @brief Clear Gpio interrupt status.
 * @note  Function ID : DES_PORT_API_223
 * @param[in] PortId: Gpio PortId
 * @param[in] ChannelId: Gpio channel id
 * @return void
 */
void Gpio_Hal_ClearIntStatus(uint8 PortId, uint8 ChannelId);

#ifndef GPIO_SDK_NON_EXTENDED_API
/**
 * @brief Get Gpio interrupt status.
 * @note  Function ID : DES_PORT_API_224
 * @param[in] PortId: Gpio PortId
 * @param[in] ChannelId: Gpio channel id
 * @return Int status
 */
uint8 Gpio_Hal_GetIntStatus(uint8 PortId, uint8 ChannelId);

/**
 * @brief Clear Gpio Pins.
 * @note  Function ID : DES_PORT_API_225
 * @param[in] PortId: Gpio PortId
 * @param[in] Pins: Pin mask of bits to be Clear. Each bit represents one pin (LSB
 *         is pin 0, MSB is pin 31). For each bit:
 *         - 0: corresponding pin is unaffected
 *         - 1: corresponding pin is toggled
 * @return void
 */
void Gpio_Hal_ClearPins(uint8 PortId, Gpio_Hal_ChannelType Pins);

/**
 * @brief Set Gpio Pins.
 * @note  Function ID : DES_PORT_API_226
 * @param[in] PortId: Gpio PortId
 * @param[in] Pins: Pin mask of bits to be Set. Each bit represents one pin (LSB
 *         is pin 0, MSB is pin 31). For each bit:
 *         - 0: corresponding pin is unaffected
 *         - 1: corresponding pin is toggled
 * @return void
 */
void Gpio_Hal_SetPins(uint8 PortId, Gpio_Hal_ChannelType Pins);
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* GPIO_HAL_H */
/*============================================EOF===================================================*/
