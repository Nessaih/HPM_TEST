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
 * @file Gpio_Hal.c
 *
 * @brief This file provides gpio integration functions.
 *
 */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Gpio_Hal.h"
#include "Core_Hal.h"
#include "AC784xx_Gpio_Reg.h"
/* ============================================  DEFINES AND MACROS  ============================================ */
/** @brief pin max number of current board */
#define PIN_NUM_MAX (153U)
/** @brief channel max number of current board */
#define CHANNEL_NUM_MAX (32U)
/** @brief MODIFY_REG32 redefine to MODIFY_VALUE to modify variable */
#define MODIFY_VALUE MODIFY_REG32

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */
/*!< Table of GPIO interrupt handler information */
static Port_CallbackType PortIRQConfig[PORT_INSTANCE_MAX];

/**
* @brief Table to save SPI IRQ enumeration.
*/
/*PRQA S 3218 ++ # Variables inside functions that occupy a large of stack space are allowed to be defined as static global variables.*/
static const IRQn_Type Port_HalIrqId[PORT_INSTANCE_MAX] = PORT_IRQS;
/*PRQA S 3218 -- #.*/
/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* =====================================  Functions definition  ===================================== */
ISR(PORTA_IRQHandler);
ISR(PORTB_IRQHandler);
ISR(PORTC_IRQHandler);
ISR(PORTD_IRQHandler);
ISR(PORTE_IRQHandler);

/**
 * @brief Configure the direction for a certain pin from a port.
 * @note  Function ID : DES_GPIO_API_200
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] Direction: The pin direction:
 *     - 0: corresponding pin is set to input
 *     - 1: corresponding pin is set to output
 *     - 2:corresponding pin is not set input and output
 * @return void
 */
static void Hal_SetPinDirection(uint8 PortId, uint8 ChannelId,
                                Port_Hal_DataDirectionType Direction);

/**
 * @brief The port irq handle.
 * @note  Function ID : DES_GPIO_API_201
 * @param[in] PortId: port id
 * @return void
 */
static void Gpio_Hal_IRQHandler(uint8 PortId);

/*=========================GLOBAL FUNCTION  IMPLEMENTATIONS==========================*/
void Gpio_Hal_ChannelInit(uint8 PinCount, const Gpio_Hal_SettingsConfigType Config[])
{
    uint8 i;
    uint8 ChannelId;
    uint8 PortId;

    DEVICE_ASSERT(PinCount <= PIN_NUM_MAX);
    for (i = 0U; i < PinCount; i++)
    {
        DEVICE_ASSERT(Config[i].PullConfig <= PORT_INTERNAL_PULL_DOWN_ENABLED);
        DEVICE_ASSERT(Config[i].DriveStrength <= PORT_HIGH_DRIVE_STRENGTH);
        DEVICE_ASSERT(Config[i].Mux <= PORT_MUX_ALT7);
        DEVICE_ASSERT(Config[i].IntConfig <= PORT_INT_EITHER_EDGE);
        DEVICE_ASSERT(Config[i].Direction <= GPIO_UNSPECIFIED_DIRECTION);
        DEVICE_ASSERT(Config[i].InitValue <= GPIO_MAX_LEVEL);
        DEVICE_ASSERT(Config[i].PortId <= PORT_INSTANCE_MAX);
        DEVICE_ASSERT(Config[i].ChannelId <= CHANNEL_NUM_MAX);

        PortId = Config[i].PortId;
        ChannelId = Config[i].ChannelId;

        /* if set pin as gpio,set the pin direction */
        if (PORT_MUX_AS_GPIO == Config[i].Mux)
        {
            /* Set the pin Direction and init value */
            Hal_SetPinDirection(PortId, ChannelId, Config[i].Direction);
        }

        /* if set pin output,set the InitValue */
        if (GPIO_OUTPUT_DIRECTION == Config[i].Direction)
        {
            /* Set the pin output value */
            Gpio_Hal_WritePin(PortId, ChannelId, Config[i].InitValue);
        }

        /*pull config*/
        Port_Reg_SetPullConfig(PortId, ChannelId, (uint32)Config[i].PullConfig);
        /*driver strength*/
        Port_Reg_SetDriverStrength(PortId, ChannelId, (uint8)Config[i].DriveStrength);
        /*Mux mode select*/
        Port_Reg_SetPinMux(PortId, ChannelId, (uint8)Config[i].Mux);
        /*pin lock enable/disable */
        Port_Reg_SetPinLock(PortId, ChannelId, (uint32)Config[i].PinLock);
        /*interrupt mode config */
        Port_Reg_SetInterruptMode(PortId, ChannelId, (uint32)Config[i].IntConfig);
        /*Out Put Type config */
#if defined (AC7842X) || defined (AC7843X)
        Port_Reg_SetOTYPER(PortId, ChannelId, (uint32)Config[i].OutputType);
#endif
    }
}

void Gpio_Hal_SetPullSel(uint8 PortId, uint8 ChannelId, Port_Hal_PullConfigType PullConfig)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    /*pull config*/
    Port_Reg_SetPullConfig(PortId, ChannelId, (uint32)PullConfig);
}

void Gpio_Hal_SetMuxMode(uint8 PortId, uint8 ChannelId, Port_Hal_MuxType Mux)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);
    DEVICE_ASSERT(Mux <= PORT_MUX_ALT7);

    Port_Reg_SetPinMux(PortId, ChannelId, (uint32)Mux);
}

Hal_StatusType Gpio_Hal_SetPinIntSel(uint8 PortId, uint8 ChannelId, Port_Hal_InterruptConfigType IntConfig)
{
    Hal_StatusType Ret = STATUS_ERROR;

    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    /*check config value whether in range*/
    if (((IntConfig <= PORT_INT_EITHER_EDGE) && (IntConfig >= PORT_INT_RISING_EDGE))
            || (IntConfig <= PORT_DMA_EITHER_EDGE))
    {
        /*PRQA S 2842,2792 ++ #.invalid pointer value*/
        Port_Reg_SetISFR(PortId, (uint32)1UL << ChannelId);
        Port_Reg_SetInterruptMode(PortId, ChannelId, (uint32)IntConfig);
        Core_Hal_EnableIrq(Port_HalIrqId[PortId]);
        /*PRQA S 2842,2792 -- */
        Ret = STATUS_SUCCESS;
    }

    return Ret;
}

Port_Hal_InterruptConfigType Gpio_Hal_GetPinIntSel(uint8 PortId, uint8 ChannelId)
{
    uint32 RegValue;

    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    RegValue = Port_Reg_GetPCR(PortId, ChannelId);
    RegValue = ((RegValue >> PORT_PCR_IRQC_Pos) & 0xFU);

    /*PRQA S 4394,4342 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    return (Port_Hal_InterruptConfigType)RegValue;
    /*PRQA S 4394,4342 -- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
}

#ifndef GPIO_SDK_NON_EXTENDED_API
void Gpio_Hal_EnableDigitalFilter(uint8 PortId, uint8 ChannelId, boolean En)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    Port_Reg_EnableDigitalFilter(PortId, ChannelId, En);
}
#endif

void Gpio_Hal_ConfigDigitalFilter(uint8 PortId, const Port_Hal_DigitalFilterCfgType *Config)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    if (NULL_PTR != Config)
    {
        /*digital filter select clock*/
        Port_Reg_SetDFCR(PortId, (uint32)Config->Clock);

        /*set digital filter width*/
        Port_Reg_SetDFWR(PortId, Config->Width);

        /*set digital filter enable*/
        Port_Reg_SetDFER(PortId, Config->PinMask);
    }
}

#ifndef GPIO_SDK_NON_EXTENDED_API
Port_Hal_DataDirectionType Gpio_Hal_GetPinDirection(uint8 PortId, uint8 ChannelId)
{
    Gpio_Hal_ChannelType RegValue;
    Port_Hal_DataDirectionType Ret;

    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    if ((PortId < PORT_INSTANCE_MAX) && (ChannelId < PORT_PCR_COUNT))
    {
        RegValue = Gpio_Reg_GetPOER(PortId);

        /*PRQA S 4394,4342 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        Ret = (Port_Hal_DataDirectionType)READ_BIT(RegValue, ChannelId);
        /*PRQA S 4394,4342 -- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    }
    else
    {
        Ret = GPIO_UNSPECIFIED_DIRECTION;
    }

    return Ret;
}
#endif

Gpio_Hal_ChannelType Gpio_Hal_GetPinsDirection(uint8 PortId)
{
    Gpio_Hal_ChannelType RegValue;

    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    RegValue = Gpio_Reg_GetPOER(PortId);

    return RegValue;
}

void Gpio_Hal_SetPinDirection(uint8 PortId, uint8 ChannelId, Port_Hal_DataDirectionType Direction)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);
    DEVICE_ASSERT(Direction <= GPIO_UNSPECIFIED_DIRECTION);

    Hal_SetPinDirection(PortId, ChannelId, Direction);
}

#ifndef GPIO_SDK_NON_EXTENDED_API
void Gpio_Hal_SetPinsDirection(uint8 PortId, uint32 PinsDir)
{
    uint32 i;
    uint32 RegInputEnable;
    uint32 RegOutputEnable;

    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);

    /*get current input and output direction status of GpioId*/
    RegInputEnable = Gpio_Reg_GetPIER(PortId);
    RegOutputEnable = Gpio_Reg_GetPOER(PortId);
    /*traverse all channels of port */
    for (i = 0U; i < PORT_PCR_COUNT; i++)
    {
        /* check the bit is input or output */
        if (0U != (BIT_SHIFT(i) & PinsDir))
        {
            /*set corresponding bit(pin) to output direction enable*/
            /* PRQA S 2985 ++ #considered an invalid operation, it is actually meaningful*/
            MODIFY_VALUE(RegOutputEnable, BIT_SHIFT(i), i, 1U);
            /*set corresponding bit(pin) to input direction disable*/
            MODIFY_VALUE(RegInputEnable, BIT_SHIFT(i), i, 0U);
        }
        else
        {
            /*set corresponding bit(pin) to output direction disable*/
            MODIFY_VALUE(RegOutputEnable, BIT_SHIFT(i), i, 0U);
            /*set corresponding bit(pin) to input direction enable*/
            MODIFY_VALUE(RegInputEnable, BIT_SHIFT(i), i, 1U);
            /* PRQA S 2985 -- */
        }
    }
    /*set port output and input direction*/
    Gpio_Reg_SetPIER(PortId, RegInputEnable);
    Gpio_Reg_SetPOER(PortId, RegOutputEnable);
}
#endif

void Gpio_Hal_WritePin(uint8 PortId, uint8 ChannelId, Gpio_Hal_LevelType Value)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);
    DEVICE_ASSERT(Value < GPIO_MAX_LEVEL);

    if ((PortId < PORT_INSTANCE_MAX) && (ChannelId < PORT_PCR_COUNT))
    {
        if (GPIO_LOW_LEVEL == Value)
        {
            Gpio_Reg_SetPROR(PortId, GPIO_PROR_Msk((ChannelId)));
        }
        else
        {
            Gpio_Reg_SetPSOR(PortId, GPIO_PSOR_Msk((ChannelId)));
        }
    }
}

void Gpio_Hal_WritePins(uint8 PortId, Gpio_Hal_ChannelType Pins)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);

    Gpio_Reg_SetPODR(PortId, Pins);
}

#ifndef GPIO_SDK_NON_EXTENDED_API
Gpio_Hal_LevelType Gpio_Hal_ReadPin(uint8 PortId, uint8 ChannelId)
{
    uint32 RegValue;
    Gpio_Hal_LevelType Ret;

    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    if ((PortId < PORT_INSTANCE_MAX) && (ChannelId < PORT_PCR_COUNT))
    {
        RegValue = Gpio_Reg_GetPIDR(PortId);

        /*PRQA S 4342 ++ # The value of an expression should not be cast to an inappropriate essential type . */
        /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        Ret = (Gpio_Hal_LevelType)READ_BIT(RegValue, ChannelId);
        /*PRQA S 4394 -- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        /*PRQA S 4342 -- # The value of an expression should not be cast to an inappropriate essential type . */
    }
    else
    {
        Ret = GPIO_MAX_LEVEL;
    }
    return Ret;
}
#endif

Gpio_Hal_ChannelType Gpio_Hal_ReadPins(uint8 PortId)
{
    uint32 RegValue;

    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);

    RegValue = Gpio_Reg_GetPIDR(PortId);

    return RegValue;
}

#ifndef GPIO_SDK_NON_EXTENDED_API
void Gpio_Hal_TogglePin(uint8 PortId, uint8 ChannelId)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    if ((PortId < PORT_INSTANCE_MAX) && (ChannelId < PORT_PCR_COUNT))
    {
        Gpio_Reg_SetPIOR(PortId, GPIO_PIOR_Msk((ChannelId)));
    }
}
#endif

void Gpio_Hal_TogglePins(uint8 PortId, Gpio_Hal_ChannelType Pins)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);

    Gpio_Reg_SetPIOR(PortId, Pins);
}

#ifndef GPIO_SDK_NON_EXTENDED_API
Gpio_Hal_LevelType Gpio_Hal_GetPinOutputLevel(uint8 PortId, uint8 ChannelId)
{
    uint32 RegValue;

    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);

    RegValue = Gpio_Reg_GetPODR(PortId);

    /*PRQA S 4394,4342 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    return (Gpio_Hal_LevelType)READ_BIT(RegValue, ChannelId);
    /*PRQA S 4394,4342 -- # convert unsigned to enum to ensure that there will be no problems in the current code.*/
}
#endif

Gpio_Hal_ChannelType Gpio_Hal_GetPinsOutputLevel(uint8 PortId)
{
    Gpio_Hal_ChannelType RegValue;

    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);

    RegValue = Gpio_Reg_GetPODR(PortId);

    return RegValue;
}

#ifndef GPIO_SDK_NON_EXTENDED_API
void Gpio_Hal_SetHighZ(uint8 PortId, uint8 ChannelId, boolean En)
{
    uint32 RegValue;
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    if ((PortId < PORT_INSTANCE_MAX) && (ChannelId < PORT_PCR_COUNT))
    {
        /* Set pinmux to GPIO */
        Port_Reg_SetPinMux(PortId, ChannelId, (uint32)PORT_MUX_AS_GPIO);

        /* Disable pull-up and pull-down */
        Port_Reg_SetPullConfig(PortId, ChannelId, (uint32)PORT_INTERNAL_PULL_NOT_ENABLED);

        /* Disable output */
        Gpio_Reg_ModifyPOER(PortId, ChannelId, 0U);

        /* Disable or enable input */
        RegValue = Gpio_Reg_GetPIER(PortId);
        /* Check enable or not */
        if (TRUE == En)
        {
            /* PRQA S 2985 ++ #considered an invalid operation, it is actually meaningful*/
            MODIFY_VALUE(RegValue, BIT_SHIFT(ChannelId), ChannelId, 0U);
            /* PRQA S 2985 -- */
        }
        else
        {
            MODIFY_VALUE(RegValue, BIT_SHIFT(ChannelId), ChannelId, 1U);
        }
        Gpio_Reg_SetPIER(PortId, RegValue);
    }
}
#endif

void Gpio_Hal_InstallCallback(uint8 PortId, const Port_CallbackType Function)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    /*PRQA S 2842 ++ #.*/
    PortIRQConfig[PortId] = Function;
    /*PRQA S 2842 -- #.*/
}

void Gpio_Hal_ClearIntStatus(uint8 PortId, uint8 ChannelId)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);
    Port_Reg_ClearInterruptFlag(PortId, ChannelId);
}

#ifndef GPIO_SDK_NON_EXTENDED_API
uint8 Gpio_Hal_GetIntStatus(uint8 PortId, uint8 ChannelId)
{
    uint32 RegValue;
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    RegValue = Port_Reg_GetInterruptFlag(PortId, ChannelId);
    return (uint8)READ_BIT(RegValue, PORT_PCR_ISF_Pos);
}

void Gpio_Hal_ClearPins(uint8 PortId, Gpio_Hal_ChannelType Pins)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    Gpio_Reg_SetPROR(PortId, Pins);
}

void Gpio_Hal_SetPins(uint8 PortId, Gpio_Hal_ChannelType Pins)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    Gpio_Reg_SetPSOR(PortId, Pins);
}
#endif

ISR(PORTA_IRQHandler)
{
    Gpio_Hal_IRQHandler(PORTID_A);
}

ISR(PORTB_IRQHandler)
{
    Gpio_Hal_IRQHandler(PORTID_B);
}

ISR(PORTC_IRQHandler)
{
    Gpio_Hal_IRQHandler(PORTID_C);
}

ISR(PORTD_IRQHandler)
{
    Gpio_Hal_IRQHandler(PORTID_D);
}

ISR(PORTE_IRQHandler)
{
    Gpio_Hal_IRQHandler(PORTID_E);
}

/*========================STATIC  FUNCTION  IMPLEMENTATIONS========================*/
static void Hal_SetPinDirection(uint8 PortId, uint8 ChannelId,
                                Port_Hal_DataDirectionType Direction)
{
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelId < PORT_PCR_COUNT);

    switch (Direction)
    {
    case GPIO_INPUT_DIRECTION:
        /*set the pin to input direction enable*/
        Gpio_Reg_ModifyPIER(PortId, ChannelId, 1U);
        /*set the pin to output direction disable*/
        Gpio_Reg_ModifyPOER(PortId, ChannelId, 0U);
        break;
    case GPIO_OUTPUT_DIRECTION:
        /*set the pin to input direction disable*/
        Gpio_Reg_ModifyPIER(PortId, ChannelId, 0U);
        /*set the pin to output direction enable*/
        Gpio_Reg_ModifyPOER(PortId, ChannelId, 1U);
        break;
    default:
        /*default set the pin to high Z status*/
        Gpio_Reg_ModifyPIER(PortId, ChannelId, 0U);
        Gpio_Reg_ModifyPOER(PortId, ChannelId, 0U);
        break;
    }
}

static void Gpio_Hal_IRQHandler(uint8 PortId)
{
    uint32 Status;
#if defined (AC7840X)
    uint32 ChipId = Core_Hal_GetChipID();
#endif /* AC7840X */
    DEVICE_ASSERT(PortId < PORT_INSTANCE_MAX);

    /*clear port interrupt*/
    Status = Port_Reg_GetISFR(PortId);
    Port_Reg_SetISFR(PortId, Status);
#if defined (AC7840X)
    /* PA5 reset pin. */
    if (PortId == 0U && (Status & 0x20U) && (0x08U != ChipId) && (0x09U != ChipId))
    {
        Core_Hal_PerformReset();
    }
#endif /* AC7840X */
    /*PRQA S 2842 ++ #.*/
    /* Check PortIRQConfig not null */
    if (PortIRQConfig[PortId] != NULL_PTR)
    {
        PortIRQConfig[PortId](PortId, Status);
    }
    /*PRQA S 2842 -- #.*/
}

/* =============================================  EOF  ============================================== */
