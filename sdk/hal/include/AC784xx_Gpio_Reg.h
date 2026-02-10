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
 * @file AC784xx_Gpio_Reg.h
 *
 * @brief This file provides gpio hardware integration interface.
 *
 */

/* PRQA S ALL -- */
/* PRQA S 0306 ++ */ /* Cast between a pointer to volatile object and an integral type. */

#ifndef AC784XX_GPIO_REG_H
#define AC784XX_GPIO_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/**
 * @brief Set the Pin control register.
 * @note  Function ID : DES_GPIO_API_301
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] Value: port Value to set
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetPCR(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->PCR[ChannelId], Value);
}

/**
 * @brief Get the Pin control register.
 * @note  Function ID : DES_GPIO_API_302
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @return uint32
 */
LOCAL_INLINE uint32 Port_Reg_GetPCR(const uint8 PortId, uint32 ChannelId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->PCR[ChannelId]);
}

/**
 * @brief Set the up/down pull of pin.
 * @note  Function ID : DES_GPIO_API_303
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] Value: up/down pull Value to set
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetPullConfig(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], (PORT_PCR_PU_Msk | PORT_PCR_PD_Msk), 0U, Value);
}

/**
 * @brief Set the driver strength of pin.
 * @note  Function ID : DES_GPIO_API_304
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] Value: driver strength Value to set
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetDriverStrength(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_DSE_Msk, PORT_PCR_DSE_Pos, Value);
}

/**
 * @brief Set the Pin mux mode.
 * @note  Function ID : DES_GPIO_API_305
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] Value: mux mode Value to set
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetPinMux(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_MUX_Msk, PORT_PCR_MUX_Pos, Value);
}

/**
 * @brief Set the Pin lock.
 * @note  Function ID : DES_GPIO_API_306
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] Value: lock Value to set
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetPinLock(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_LK_Msk, PORT_PCR_LK_Pos, Value);
}

/**
 * @brief Set the Pin interrupt mode .
 * @note  Function ID : DES_GPIO_API_307
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] Value: interrupt mode Value to set
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetInterruptMode(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (uint32)(0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_IRQC_Msk, PORT_PCR_IRQC_Pos, Value);
}

/**
 * @brief Clear the port control interrupt status flag register.
 * @note  Function ID : DES_GPIO_API_308
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @return void
 */
LOCAL_INLINE void Port_Reg_ClearInterruptFlag(uint8 PortId, uint32 ChannelId)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_ISF_Msk, PORT_PCR_ISF_Pos, 1U);
}

/**
 * @brief get the port control interrupt status flag register.
 * @note  Function ID : DES_GPIO_API_335
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @return uint32
 */
LOCAL_INLINE uint32 Port_Reg_GetInterruptFlag(uint8 PortId, uint32 ChannelId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->PCR[ChannelId]);
}

/**
 * @brief Set the port interrupt status flag register.
 * @note  Function ID : DES_GPIO_API_309
 * @param[in] PortId: port id
 * @param[in] Value: Each bit represents one Pin clear interrupt status clear(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31)
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetISFR(uint8 PortId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->ISFR, Value);
}

/**
 * @brief Get the port interrupt status flag register.
 * @note  Function ID : DES_GPIO_API_310
 * @param[in] PortId: port id
 * @return uint32  Each bit represents one Pin interrupt status interrupted(1) or no interrupt(0) (LSB is Pin 0, MSB is Pin 31)
 */
LOCAL_INLINE uint32 Port_Reg_GetISFR(const uint8 PortId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->ISFR);
}

/**
 * @brief Set the port digital filter enable register.
 * @note  Function ID : DES_GPIO_API_311
 * @param[in] PortId: port id
 * @param[in] ChannelId: port channel id
 * @param[in] En: enable/disable digital filter
 * @return void
 */
LOCAL_INLINE void Port_Reg_EnableDigitalFilter(uint8 PortId, uint32 ChannelId, boolean En)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->DFER, ((uint32)1U << ChannelId), ChannelId, En);
}

/**
 * @brief Set the port digital filter enable register.
 * @note  Function ID : DES_GPIO_API_312
 * @param[in] PortId: port id
 * @param[in] Value: Each bit represents one Pin digital filer enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31)
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetDFER(uint8 PortId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->DFER, Value);
}

/**
 * @brief Get the port digital filter enable register.
 * @note  Function ID : DES_GPIO_API_313
 * @param[in] PortId: port id
 * @return uint32 Each bit represents one Pin digital filer enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 */
LOCAL_INLINE uint32 Port_Reg_GetDFER(uint8 PortId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->DFER);
}

/**
 * @brief Set the port digital filter clock register.
 * @note  Function ID : DES_GPIO_API_314
 * @param[in] PortId: port id
 * @param[in] Value: port digital filter clock Value to set
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetDFCR(uint8 PortId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->DFCR, Value);
}

/**
 * @brief Get the port digital filter clock register.
 * @note  Function ID : DES_GPIO_API_315
 * @param[in] PortId: port id
 * @return uint32
 */
LOCAL_INLINE uint32 Port_Reg_GetDFCR(uint8 PortId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->DFCR);
}

/**
 * @brief Set the port digital filter width register.
 * @note  Function ID : DES_GPIO_API_316
 * @param[in] PortId: port id
 * @param[in] Value: port digital filter width Value to set
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetDFWR(uint8 PortId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->DFWR, Value);
}

/**
 * @brief Get the port digital filter width register.
 * @note  Function ID : DES_GPIO_API_317
 * @param[in] PortId: port id
 * @return uint32 digital filter width
 */
LOCAL_INLINE uint32 Port_Reg_GetDFWR(uint8 PortId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->DFWR);
}

/**
 * @brief write the port output data register
 * @note  Function ID : DES_GPIO_API_318
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin output Value high(1) or low(0) (LSB is Pin 0, MSB is Pin 31).
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_SetPODR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PODR, Value);
}

/**
 * @brief Get the port output data register
 * @note  Function ID : DES_GPIO_API_319
 * @param[in] GpioId: Gpio id
 * @return uint32 Each bit represents one Pin output Value high(1) or low(0) (LSB is Pin 0, MSB is Pin 31).
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPODR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PODR);
}

/**
 * @brief Set the port set output data register
 * @note  Function ID : DES_GPIO_API_320
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin set Value set high(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_SetPSOR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PSOR, Value);
}

/**
 * @brief Get the port set output data register
 * @note  Function ID : DES_GPIO_API_321
 * @param[in] GpioId: Gpio id
 * @return uint32 Each bit represents one Pin set Value set high(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPSOR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PSOR);
}

/**
 * @brief Set the port clear output data register
 * @note  Function ID : DES_GPIO_API_322
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin reset Value reset(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_SetPROR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PROR, Value);
}

/**
 * @brief Get the port clear output data register
 * @note  Function ID : DES_GPIO_API_323
 * @param[in] GpioId: Gpio id
 * @return uint32 Each bit represents one Pin reset Value reset(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPROR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PROR);
}

/**
 * @brief Set the port toggle output data register
 * @note  Function ID : DES_GPIO_API_324
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin toggle Value toggle(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_SetPIOR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PIOR, Value);
}

/**
 * @brief Get the port toggle output data register
 * @note  Function ID : DES_GPIO_API_325
 * @param[in] GpioId: Gpio id
 * @return uint32 Each bit represents one Pin toggle Value toggle(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPIOR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PIOR);
}

/**
 * @brief Set the port input data register
 * @note  Function ID : DES_GPIO_API_326
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin input Value high(1) or low(0) (LSB is Pin 0, MSB is Pin 31).
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_SetPIDR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PIDR, Value);
}

/**
 * @brief Get the port input data register
 * @note  Function ID : DES_GPIO_API_327
 * @param[in] GpioId: Gpio id
 * @return uint32 Each bit represents one Pin input Value high(1) or low(0) (LSB is Pin 0, MSB is Pin 31).
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPIDR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PIDR);
}

/**
 * @brief Set the port ouput enable register
 * @note  Function ID : DES_GPIO_API_328
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin output enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_SetPOER(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->POER, Value);
}

/**
 * @brief Get the port output enable register
 * @note  Function ID : DES_GPIO_API_329
 * @param[in] GpioId: Gpio id
 * @return uint32 Each bit represents one Pin output enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPOER(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->POER);
}

/**
 * @brief Set the port input enable register
 * @note  Function ID : DES_GPIO_API_330
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin input enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_SetPIER(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PIER, Value);
}

/**
 * @brief Get the port input enable register
 * @note  Function ID : DES_GPIO_API_331
 * @param[in] GpioId: Gpio id
 * @return uint32 Each bit represents one Pin input enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPIER(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PIER);
}

/**
 * @brief write the port input enable register
 * @note  Function ID : DES_GPIO_API_332
 * @param[in] GpioId: Gpio id
 * @param[in] Value: 1 means enable , 0 means disable
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_ModifyPIER(uint8 GpioId, uint32 ChannelId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    MODIFY_REG32(Base->PIER, ((uint32)1U << ChannelId), ChannelId, Value);
}

/**
 * @brief write the port output enable register
 * @note  Function ID : DES_GPIO_API_333
 * @param[in] GpioId: Gpio id
 * @param[in] ChannelId: channel id of port
 * @param[in] Value: 1 means enable , 0 means disable
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_ModifyPOER(uint8 GpioId, uint32 ChannelId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    MODIFY_REG32(Base->POER, ((uint32)1U << ChannelId), ChannelId, Value);
}

/**
 * @brief write the port output data register
 * @note  Function ID : DES_GPIO_API_334
 * @param[in] GpioId: Gpio id
 * @param[in] ChannelId: channel id of port
 * @param[in] Value: 1 means high level , 0 means low level
 * @return void
 */
LOCAL_INLINE void Gpio_Reg_ModifyPODR(uint8 GpioId, uint32 ChannelId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    MODIFY_REG32(Base->PODR, ((uint32)1U << ChannelId), ChannelId, Value);
}

#if defined (AC7842X) || defined (AC7843X)
/**
 * @brief Set the port output type register
 * @param[in] GpioId: Gpio id
* @param[in] ChannelId: channel id of port
 * @param[in] Value:  0 means push-pull , 1 means open-drain.
 * @return void
 */
LOCAL_INLINE void Port_Reg_SetOTYPER(uint8 GpioId, uint32 ChannelId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    MODIFY_REG32(Base->OTYPER, ((uint32)1U << ChannelId), ChannelId, Value);
}
#endif

/* =====================================  Functions definition  ===================================== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC784XX_GPIO_REG_H*/

/* =============================================  EOF  ============================================== */
