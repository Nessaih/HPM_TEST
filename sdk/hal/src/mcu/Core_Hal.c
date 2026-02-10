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
 * @file Core_Hal.c
 *
 * @brief This file provides Hal Core api.
 *
 */

/*==============================================INCLUDE FILES=======================================*/
#include "Core_Hal.h"
#include "AC784xx_Mcm_Reg.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
static const Core_ConfigType *Core_Hal_ConfigPtr;

/*============================================FUNCTION PROTOTYPES===================================*/
ISR(MCM_IRQHandler);

/*!
 * @brief Enable Irq.
 * @note Function ID: DES_CORE_API_200
 * @param[in] IrqNumber: Irq Number.
 * @return void
 */
void Core_Hal_EnableIrq(IRQn_Type IrqNumber)
{
    NVIC_ClearPendingIRQ(IrqNumber);
    NVIC_EnableIRQ(IrqNumber);
}

/*!
 * @brief Disable Irq.
 * @note Function ID: DES_CORE_API_201
 * @param[in] IrqNumber: Irq Number.
 * @return void
 */
void Core_Hal_DisableIrq(IRQn_Type IrqNumber)
{
    NVIC_DisableIRQ(IrqNumber);
}

/*!
 * @brief Get Irq Enable status.
 * @note  Function ID: DES_CORE_API_202
 * @param[in] IrqNumber: Irq Number.
 * @return boolean: Irq Enable status.
 *          FALSE: Interrupt status is not enable.
 *          TRUE: Interrupt status is enable.
 */
boolean Core_Hal_IsIrqEnable(IRQn_Type IrqNumber)
{
    /*cstat !MISRAC2012-Rule-10.5*/
    return (boolean) NVIC_GetEnableIRQ(IrqNumber);
}

/*!
 * @brief Set Irq priority.
 * @note  Function ID: DES_CORE_API_203
 * @param[in] IrqNumber: Irq Number.
 * @param[in] Priority: Irq Priority.
 * @return void
 */
void Core_Hal_SetIrqPriority(IRQn_Type IrqNumber, uint32 Priority)
{
    NVIC_SetPriority(IrqNumber, Priority);
}

/*!
 * @brief Get Irq priority.
 * @note Function ID: DES_CORE_API_204
 * @param[in] IrqNumber: Irq Number.
 * @return uint32: Irq priority
 */
uint32 Core_Hal_GetIrqPriority(IRQn_Type IrqNumber)
{
    return NVIC_GetPriority(IrqNumber);
}

/*!
 * @brief Perform software reset.
 * @note  Function ID: DES_CORE_API_205
 * @return void
 */
void Core_Hal_PerformReset(void)
{
    NVIC_SystemReset();
}

/*!
 * @brief Set pending Irq.
 * @note  Function ID: DES_CORE_API_206
 * @param[in] IrqNumber: Irq Number.
 * @return void
 */
void Core_Hal_SetPendingIrq(IRQn_Type IrqNumber)
{
    NVIC_SetPendingIRQ(IrqNumber);
}

/*!
 * @brief Clear pending Irq.
 * @note  Function ID: DES_CORE_API_207
 * @param[in] IrqNumber: Irq Number.
 * @return void
 */
void Core_Hal_ClearPendingIrq(IRQn_Type IrqNumber)
{
    NVIC_ClearPendingIRQ(IrqNumber);
}

/*!
 * @brief Get pending Irq.
 * @note  Function ID: DES_CORE_API_208
 * @param[in] IrqNumber: Irq Number.
 * @return boolean: Irq is pending or not.
 *          FALSE: Interrupt status is not active.
 *          TRUE: Interrupt status is active.
 */
boolean Core_Hal_IsIrqPending(IRQn_Type IrqNumber)
{
    return (boolean)NVIC_GetPendingIRQ(IrqNumber);
}

/*!
 * @brief Get Active Irq.
 * @note Function ID: DES_CORE_API_209
 * @param[in] IrqNumber: Irq Number.
 * @return boolean: Irq is active or not.
 *          FALSE: Interrupt status is not active.
 *          TRUE: Interrupt status is active.
 */
boolean Core_Hal_IsIrqActive(IRQn_Type IrqNumber)
{
    return (boolean)NVIC_GetActive(IrqNumber);
}

/*!
 * @brief Set Irq priority grouping.
 * @note Function ID: DES_CORE_API_210
 * @param[in] PriorityGroup: Irq Priority grouping.
 * @return void
 */
void Core_Hal_SetIrqPriorityGrouping(uint32 PriorityGroup)
{
    NVIC_SetPriorityGrouping(PriorityGroup);
}

/*!
 * @brief Get Irq priority grouping.
 * @note Function ID: DES_CORE_API_211
 * @return uint32: Irq priority grouping.
 */
uint32 Core_Hal_GetIrqPriorityGrouping(void)
{
    return NVIC_GetPriorityGrouping();
}

#if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined (__FPU_USED) && (__FPU_USED == 1U)))
/*!
 * @brief Get FPU error information.
 * @note Function ID: DES_CORE_API_215
 * @param[in] ErrorPtr: Pointer to where to store the error information of FPU.
 * @return Hal_StatusType: Error status.
 *               STATUS_SUCCESS: no error occurred.
 *               STATUS_ERROR:   error detected.
 */
Hal_StatusType Core_Hal_GetFpuErrorInfo(Core_FpuErrorInfoType *ErrorPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 FpuError;
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
    /* Get all float pointer exception status flags by read the MCM_MISCR register. */
    FpuError = Mcm_Reg_GetInterruptStatus() >> MCM_MISCR_FIOC_Pos;
    if (FpuError != 0U)
    {
        ReturnVal = STATUS_ERROR;

        /* Put the exception status to the structure if the pointer is not NULL. */
        if (NULL_PTR != ErrorPtr)
        {
            ErrorPtr->Err_Status = FpuError;
        }
    }
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif

    return ReturnVal;
}

/*!
 * @brief Clear FPU error information.
 * @note Function ID: DES_CORE_API_216
 * @return void
 */
/*PRQA S 3006 ++ # allows mixed use of inline assembly and C statements.*/
void Core_Hal_ClearFpuErrorInfo(void)
{
    volatile uint32 *Fpscr = (volatile uint32 *)(FPU->FPCAR + 0x40U);
    register uint32 RegFPSCR = __get_FPSCR();

    /* Clear exceptions at low 8 bits. */
    RegFPSCR &= 0xFFFFFF00U;

    /* Clear exceptions at FPSCR register. */
    __set_FPSCR(RegFPSCR);

    /* Clear exceptions at FPSCR location at stack to avoid endless FPU interrupt. */
    *Fpscr = RegFPSCR;
}
/*PRQA S 3006 -- */
#endif

#if defined(AC7840X) || defined(AC7842X)
/*!
 * @brief Get Flash Cache ECC error detail information.
 * @note Function ID: DES_CORE_API_218
 * @param[out] ErrorPtr: Pointer to where to store the error information of flash cache ECC.
 * @return Hal_StatusType: Error status.
 *               STATUS_SUCCESS: no error occurred.
 *               STATUS_ERROR:   error detected.
 */
Hal_StatusType Core_Hal_GetFlashCacheErrorInfo(Core_EccErrorInfoType *ErrorPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 ErrStatus;
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (--Timeout > 0U)
    {
        /* used for delay */
    }
#endif
    /* Get cache error status flags by read the MCM_MCPESR register. */
    ErrStatus = Mcm_Reg_GetFlashCacheEccErrorStatus();
    if (ErrStatus != 0U)
    {
        ReturnVal = STATUS_ERROR;

        /* Put the error information to the structure if the pointer is not NULL. */
        if (NULL_PTR != ErrorPtr)
        {
            ErrorPtr->Err_Status = ErrStatus;
            ErrorPtr->Err_Addr = Mcm_Reg_GetFlashCacheEccErrorAddress();
        }
    }
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif

    return ReturnVal;
}

/*!
 * @brief Clear flash Cache ECC error.
 * @note Function ID: DES_CORE_API_219
 * @return void
 */
void Core_Hal_ClearFlashCacheErrorInfo(void)
{
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (--Timeout > 0U)
    {
        /* used for delay */
    }
#endif
    Mcm_Reg_ClearFlashCacheEccError();
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
}
#endif

#if defined (AC7843X)
/*!
 * @brief Enable CPU instruction cache.
 * @note Function ID: DES_CORE_API_220
 * @param[in] En: enable state
 * @return void
 */
void Core_Hal_EnableICache(boolean En)
{
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
    if (En == TRUE)
    {
        SET_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_EN_Msk);
    }
    else
    {
        CLEAR_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_EN_Msk);
    }
    CKGEN->RCM_EN = RcmEnRegVal;
}

/*!
 * @brief Invalid CPU instruction cache.
 * @note Function ID: DES_CORE_API_221
 * @return void
 */
void Core_Hal_InvalidICache(void)
{
    volatile uint32 i = 5U;
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    SET_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_INVALID_Msk);
    while ((MCM->ICACHE_CFG & MCM_ICACHE_CFG_CLRDONE_Msk) == 0U)
    {
        if (i > 0U)
        {
             i = i-1U;
        }
        else
        {
            break;
        }
    }
    CKGEN->RCM_EN = RcmEnRegVal;
}

/*!
 * @brief Get CPU instruction cache ECC error detail information.
 * @note Function ID: DES_CORE_API_222
 * @param[out] ErrorPtr: Pointer to where to store the error information of CPU instruction cache ECC.
 * @return Hal_StatusType: Error status.
 *               STATUS_SUCCESS: no error occurred.
 *               STATUS_ERROR:   error detected.
 */
Hal_StatusType Core_Hal_GetICacheErrorInfo(Core_EccErrorInfoType *ErrorPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 ErrStatus;
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
    /* Get cache error status flags by read the MCM_DCACHE_CFG register. */
    ErrStatus = READ_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_ECCSTA_Msk) >> MCM_ICACHE_CFG_ECCSTA_Pos;
    if (ErrStatus != 0U)
    {
        ReturnVal = STATUS_ERROR;

        /* Put the error information to the structure if the pointer is not NULL. */
        if (NULL_PTR != ErrorPtr)
        {
            ErrorPtr->Err_Status = ErrStatus;
            ErrorPtr->Err_Addr = 0U; //No Address supplied.
        }
    }
    CKGEN->RCM_EN = RcmEnRegVal;

    return ReturnVal;
}

/*!
 * @brief Clear CPU instuction cache error detail information.
 * @note Function ID: DES_CORE_API_223
 * @return void
 */
void Core_Hal_ClearICacheErrorInfo(void)
{
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
    SET_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_ECCCLR_Msk);
    CKGEN->RCM_EN = RcmEnRegVal;
}

/*!
 * @brief Enable CPU Data cache.
 * @note Function ID: DES_CORE_API_224
 * @param[in] En: enable state
 * @return void
 */
void Core_Hal_EnableDCache(boolean En)
{
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    if (En == TRUE)
    {
        SET_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_EN_Msk);
    }
    else
    {
        CLEAR_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_EN_Msk);
    }
    CKGEN->RCM_EN = RcmEnRegVal;
}

/*!
 * @brief Invalid CPU Data cache.
 * @note Function ID: DES_CORE_API_225
 * @return void
 */
void Core_Hal_InvalidDCache(void)
{
    volatile uint32 i = 5U;
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    SET_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_INVALID_Msk);
    while ((MCM->DCACHE_CFG & MCM_DCACHE_CFG_CLRDONE_Msk) == 0U)
    {
        if (i > 0U)
        {
             i = i-1U;
        }
        else
        {
            break;
        }
    }

    CKGEN->RCM_EN = RcmEnRegVal;
}

/*!
 * @brief Get CPU data cache ECC error detail information.
 * @note Function ID: DES_CORE_API_226
 * @param[out] ErrorPtr: Pointer to where to store the error information of CPU data cache ECC.
 * @return Hal_StatusType: Error status.
 *               STATUS_SUCCESS: no error occurred.
 *               STATUS_ERROR:   error detected.
 */
Hal_StatusType Core_Hal_GetDCacheErrorInfo(Core_EccErrorInfoType *ErrorPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 ErrStatus;
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    /* Get cache error status flags by read the MCM_DCACHE_CFG register. */
    ErrStatus = READ_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_ECCSTA_Msk) >> MCM_DCACHE_CFG_ECCSTA_Pos;
    if (ErrStatus != 0U)
    {
        ReturnVal = STATUS_ERROR;

        /* Put the error information to the structure if the pointer is not NULL. */
        if (NULL_PTR != ErrorPtr)
        {
            ErrorPtr->Err_Status = ErrStatus;
            ErrorPtr->Err_Addr = 0U; //No Address supplied.
        }
    }
    CKGEN->RCM_EN = RcmEnRegVal;

    return ReturnVal;
}

/*!
 * @brief Clear CPU data cache error detail information.
 * @note Function ID: DES_CORE_API_227
 * @return void
 */
void Core_Hal_ClearDCacheErrorInfo(void)
{
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    SET_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_ECCCLR_Msk);
    CKGEN->RCM_EN = RcmEnRegVal;
}
#endif

/**
 * @brief Get UUID from device.
 * @note  Function ID: DES_CORE_API_212
 * @param[out] uuidBuffer: UUID buffer
 * @return void
 */
void Core_Hal_GetUUID(uint32 *uuidBuffer)
{
    uint32 i;

    if (uuidBuffer != NULL_PTR)
    {
        for (i = 0U; i < 4U; i++)
        {
            uuidBuffer[i] = READ_MEM32(UUID_BASE + (i * 4U));
        }
    }
}

/**
 * @brief Get Chip ID from device.
 * @note  Function ID: DES_CORE_API_213
 * @return uint32: Chip ID
 */
uint32 Core_Hal_GetChipID(void)
{
    return (READ_MEM32(TYPEID_BASE) & 0x000FFFFFU);
}

/**
 * @brief Enable NMI, after set pinmux.
 * @note  Function ID: DES_CORE_API_214
 * @param[in] En: enable state
 * @return void
 */
void Core_Hal_EnableNMI(boolean En)
{
#if defined (AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
    Mcm_Reg_EnableNmi(En);
#if defined (AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
}

/*!
 * @brief Get the MBIST execute status.
 * @note      Function ID: DES_CORE_API_217
 * @return Core_BistStatusType: BIST execute status.
 *               BIST_NORUN: BIST not run
 *               BIST_ERROR: Threre is a BIST fault occurred.
 *               BIST_OK: BIST success.
 *               BIST_BUSY: BIST isr unning
 */
Core_BistStatusType Core_Hal_GetMBistExecStatus(void)
{
    Core_BistStatusType ReturnVal;
    uint32 Result;
#if defined (AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
    /* Get the MBIST execute result from MCM_MBIST register. */
    Result = Mcm_Reg_GetMbistStatus();
#if defined (AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
    switch (Result)
    {
    case 0U:
        ReturnVal = BIST_NORUN;
        break;
    case 1U:
        ReturnVal = BIST_OK;
        break;
    case 3U:
        ReturnVal = BIST_ERROR;
        break;
    default:
        ReturnVal = BIST_BUSY;
        break;
    }

    return ReturnVal;
}

/*!
 * @brief Initialize MCM module.
 * @note  Function ID: DES_CORE_API_228
 * @param[in] ConfigPtr: Core Configuration.
 * @return void
 */
void Core_Hal_Init(const Core_ConfigType *ConfigPtr)
{
    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
#if defined (AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;
#endif
    if (ConfigPtr != NULL_PTR)
    {
#if defined (AC7843X)
        CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
        Mcm_Reg_SetInterruptSources(ConfigPtr->Irq_Src);
#if defined (AC7843X)
        CKGEN->RCM_EN = RcmEnRegVal;
#endif
        Core_Hal_ConfigPtr = ConfigPtr;
#ifdef IRQ_CONTROL_IN_CORE_HAL
        if (ConfigPtr->Irq_Src != 0U)
        {
            Core_Hal_EnableIrq(MCM_IRQn);
        }
#endif
    }
}

/*!
 * @brief De-Initialze MCM module.
 * @note  Function ID: DES_CORE_API_229
 * @return void
 */
void Core_Hal_DeInit(void)
{
#if defined (AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
#ifdef IRQ_CONTROL_IN_CORE_HAL
    if (Core_Hal_ConfigPtr->Irq_Src != 0U)
    {
        Core_Hal_DisableIrq(MCM_IRQn);
    }
#endif

    Mcm_Reg_SetInterruptSources(0U);
#if defined (AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
}

/*!
 * @brief MCM handler.
 * @note  Function ID: DES_CORE_API_230
 * @return void
 */
ISR(MCM_IRQHandler)
{
    uint32 IntStatus;
#if defined (AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
   while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
    /* Read the interrupt status to identify wheter the interrupt is valid. */
    IntStatus = Mcm_Reg_GetInterruptStatus();
#if defined (AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
    if (IntStatus != 0U)
    {
        /* Clear Error interrupt flag, state and address. */
        Core_Hal_ClearFpuErrorInfo();

        if ((Core_Hal_ConfigPtr != NULL_PTR) && (Core_Hal_ConfigPtr->Irq_Callback != NULL_PTR))
        {
            Core_Hal_ConfigPtr->Irq_Callback((void *)&IntStatus);
        }
    }
}
