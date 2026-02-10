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
*@file Fls_Hal.c
*
*@brief This file provides sdk flash api function
*
*/

/*==============================================INCLUDE FILES=======================================*/
#include "Fls_Hal.h"
#include "AC784xx_Fls_Reg.h"
#include "OsIf_Critical.h"
#include "OsIf_Irq.h"
#include "Spm_Hal.h"
#include "Core_Hal.h"
#include "System_AC784xx.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*===========================================DEFINES AND MACROS==================================*/
#define FLASH_INVALID_ADDR               (0xFFFFFFFFU)
#define FLASH_NOT_USE_ADDR               (0x40000U)

#if defined (AC7840X)
#define FLASH_PPROT_UNIT_SIZE              (0x8000U)
#define FLASH_PPROT_MAX_UNIT               (32U)
#define FLASH_DPROT_UNIT_SIZE              (0x4000U)
#define FLASH_DPROT_CSE_UNIT_SIZE          (0x2000U)
#define FLASH_DPROT_MAX_UNIT               (8U)
#define FLASH_WPROT_BYTES                  (8U)
#elif defined (AC7842X)
#define FLASH_PPROT_UNIT_SIZE              (0x10000U)
#define FLASH_PPROT_MAX_UNIT               (32U)
#define FLASH_DPROT_UNIT_SIZE              (0x4000U)
#define FLASH_DPROT_CSE_UNIT_SIZE          (0x2000U)
#define FLASH_DPROT_MAX_UNIT               (8U)
#define FLASH_WPROT_BYTES                  (8U)
#elif defined (AC7843X)
#define FLASH_PPROT_UNIT_SIZE              (0x8000U)
#define FLASH_PPROT_MAX_UNIT               (128U)
#define FLASH_DPROT_UNIT_SIZE              (0x4000U)
#define FLASH_DPROT_CSE_UNIT_SIZE          (0x4000U)
#define FLASH_DPROT_MAX_UNIT               (16U)
#define FLASH_WPROT_BYTES                  (24U)
#endif

/*===========================================VARIABLE DECLARATIONS==================================*/
/*!< The size in byte of D-Flash memory */
static uint32 S_DFlashSize = DFLASH_BLOCK_SIZE;
/* disable write/erase timeout and enbale lvd by default */
static Flash_Config S_FlsCfg = {0U, 0U, TRUE}; //PRQA S 3218 # rule 8.9 waring*/

#if defined (AC7840X)
#if !defined (__ICCARM__)
static uint32 FLASH_StartCmd_WaitClear_Cpy[128] = {0}; //PRQA S 3218 # rule 8.9 waring*/
void (*FLASH_StartCmd_WaitClr_InRam)(void) = NULL_PTR; //PRQA S 3408 # rule 8.9 waring*/
#endif
#endif

#if defined (AC7842X)
static volatile uint8 McuLvdFlag = 0U;
#endif
/*============================================FUNCTION PROTOTYPES===================================*/

/*=======================================FUNCTIONS==================================================*/
/*PRQA S 3415 ++ #pointer will be check in DEVICE_ASSERT and not return in DEVICE_ASSERT */

/*PRQA S 3006 ++ # ues inline function */
#if defined (AC7842X)
/**
 * @brief Clear the MCU LVD interrupt status and get the current LVD state.
 *
 * This function performs the following operations:
 * 1. Clears the MCU Low Voltage Detect (LVD) interrupt status by writing '1' to bit 22
 *    of register 0xE000E284.
 * 2. Inserts two NOP instructions to ensure the clear operation takes effect.
 * 3. Reads the LVD status from bit 22 of register 0xE000E204 and returns the result.
 *
 * @note The LVD status bit is masked with (1 << 22). If the result is non-zero, it indicates
 *       that the LVD condition is present.
 *
 * @return uint32_t The current LVD status:
 *         - 0: No LVD detected.
 *         - Non-zero: LVD condition detected.
 */
static uint32 McuLvdClearAndGetStatus(void)
{
    MODIFY_REG32(*(volatile uint32 *)0xE000E284U, (uint32)(0x1U << 22U), 22U, 1U);
    ASM_KEYWORD("nop");//PRQA S 1006 # allow similar macro definitions.*/
    ASM_KEYWORD("nop");//PRQA S 1006 # allow similar macro definitions.*/
    return READ_REG32(*(volatile uint32 *)0xE000E204U) & (uint32)(0x1U << 22U);
}
#endif

#if defined (AC7840X)
#if defined (__ICCARM__)
__ramfunc void FLASH_StartCmd_WaitClear(void)
#else
__attribute__((section(".text"))) static void FLASH_StartCmd_WaitClear(void)
#endif
/*PRQA S 3006 -- */
{
    volatile uint8 timeout = 16U;
    volatile uint8 CST_Timeout = 10U;

    /* trigger command execute */
    SET_BIT32(FLASH->CST, FLASH_CST_START_Msk);
    /*wait trigger register clear finished*/
    while ((0x0U != READ_BIT32(FLASH->CST, FLASH_CST_START_Msk)) && (0x0U != CST_Timeout))
    {
        CST_Timeout = CST_Timeout - 1U;
    }

    while(timeout > 0U)
    {
        ASM_KEYWORD("nop"); //PRQA S 1006 # allow similar macro definitions.*/
        timeout = timeout - 1U;
    };
}
#endif

/**
* @brief Set cmd to start flash controller process
* @note Function ID:[DES_FLS_API_208]
* @param [in] Wait_Timeout:  flash operation timeout counter
* @return Hal_StatusType
*/
static Hal_StatusType Flash_Hal_StartCmd(uint32 Wait_Timeout)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    volatile uint32 TmpWait_Timeout = Wait_Timeout;
    /* Enable / disable timeout*/
    volatile boolean Timeout_Enable = (0U == TmpWait_Timeout) ? FALSE : TRUE;
    volatile uint8 CommandStatus = 0x0U;
#if defined (AC7842X) || defined (AC7843X)
    volatile uint32 Timeout = 37U; /*the value is ok for check cache clear*/
#endif

#if defined (AC7842X)
/*PRQA S 2889 ++*/

    /* Check MCU LVD flag. If set, clear the interrupt status.
     * If LVD status is not active, enable LVD IRQ and clear the flag;
     * otherwise, return error status and stop flash write operation.
     */
    if (McuLvdFlag == 1U)
    {
        uint32 McuLvdSts = McuLvdClearAndGetStatus();
        if (McuLvdSts == 0U)
        {
            Core_Hal_EnableIrq(LVD_IRQn);
            McuLvdFlag = 0U;
        }
        else
        {
            return STATUS_ABORT;
        }
    }
/*PRQA S 2889 --*/
#endif

    /*trigger flash */
#if defined (AC7840X)
    OSIF_ENTER_CRITICAL(FLS_HAL_ID1);
#if defined (__ICCARM__)
    FLASH_StartCmd_WaitClear();
#else
    if (FLASH_StartCmd_WaitClr_InRam == NULL_PTR)
    {
        //PRQA S 0326 ++ # allow pointer to integral.*/
        //PRQA S 0305 ++ # allow pointer to integral.*/
        (void)System_Memcpy(FLASH_StartCmd_WaitClear_Cpy, (void *)((uint32)FLASH_StartCmd_WaitClear & 0xFFFFFFFEU),
                sizeof(FLASH_StartCmd_WaitClear_Cpy));
        FLASH_StartCmd_WaitClr_InRam = (void (*)(void))((uint32)FLASH_StartCmd_WaitClear_Cpy | 0x1U);
        //PRQA S 0305 -- # allow pointer to integral.*/
        //PRQA S 0326 -- # allow pointer to integral.*/
    }
    FLASH_StartCmd_WaitClr_InRam();
#endif
    OSIF_EXIT_CRITICAL(FLS_HAL_ID1);
#else
    Flash_Reg_TrigCtrlCmdReg();
    /*wait cache clear*/
    while (Timeout > 0U)
    {
        Timeout--; //PRQA S 3387 # allow volatile modified variables to be ++ or --.*/
    }
    /*wait trigger register clear finished*/
    while (0x0U != Flash_Reg_GetTrigCtrlCmdRegStatus())
    {
    }
#endif
    /*wait trigger flash finished*/
    while (0x0U == CommandStatus)
    {
        CommandStatus = Flash_Reg_GetCommandCompleteStatusReg();
        if (TRUE == Timeout_Enable)
        {
            TmpWait_Timeout = TmpWait_Timeout - 1U;
            if (0U == TmpWait_Timeout)
            {
                break;
            }
        }
    }
    /*Check flash status whether fail*/
    if (Flash_Reg_GetStatusReg() != 0x0U)
    {
        RetVal = STATUS_ERROR;
    }
    else if ((TRUE == Timeout_Enable) && (0U == TmpWait_Timeout))
    {
        RetVal = STATUS_TIMEOUT;
    }
    else
    {
        /*do nothing*/
    }

    return RetVal;
}

#if defined (AC7840X) || (defined (AC7842X) && !defined (FLS_SDK_NON_EXTENDED_API))
static Hal_StatusType Flash_Hal_SW_Verify(uint32 addr, uint32 size)
{
    uint32 i;
    Hal_StatusType RetVal = STATUS_SUCCESS;

    for (i = 0; i < size; i++)
    {
        if (0xFFU != *(uint8 *)(addr + i))
        {
            RetVal = STATUS_ERROR;
            break;
        }
    }
    return RetVal;
}
#endif

#if defined (AC7842X)
/*PRQA S 3673 ++ # unmodified pointer types are allowed in interrupt functions without const modification.*/
static void Flash_Hal_LVD_Callback(void *para)
/*PRQA S 3673 -- # unmodified pointer types are allowed in interrupt functions without const modification.*/
{
    (void)para;
    McuLvdFlag = 1U;
}
#endif

static void Flash_Hal_GetDflashSize(void)
{
    uint8 dFlashPartitionCode = 0U;
    dFlashPartitionCode = Flash_Reg_GetDPartition();
    if ((dFlashPartitionCode <= 0x08U) && ((dFlashPartitionCode & 1U) == 0U))
    {
        S_DFlashSize = (DFLASH_BLOCK_SIZE / 16U) * (uint32)dFlashPartitionCode;
    }
    else
    {
        S_DFlashSize = DFLASH_BLOCK_SIZE;
    }
}

/**
* @brief lock flash register write
* @note Function ID:[DES_FLS_API_205]
* @return None
*/
static void Flash_Hal_LockCtrl(void)
{
    Flash_Reg_Lock();
}

/**
* @brief unlock flash register write
* @note Function ID:[DES_FLS_API_209]
* @return Hal_StatusType
*/
static Hal_StatusType Flash_Hal_UnLockCtrl(void)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    volatile uint32 TimeoutCount = 0x0U;
    volatile uint8 CtrlLockBit;

    CtrlLockBit = Flash_Reg_GetLockStatusReg();
    while (0x1U == CtrlLockBit)
    {
        Flash_Reg_UnLock();

        if (100U < TimeoutCount)
        {
            RetVal = STATUS_TIMEOUT;
            break;
        }
        CtrlLockBit = Flash_Reg_GetLockStatusReg();
        /* PRQA S 3387 ++ # Variables inside functions that occupy a large of stack space are allowed to be defined as static global variables. */
        TimeoutCount++;
        /* PRQA S 3387 -- # Variables inside functions that occupy a large of stack space are allowed to be defined as static global variables. */
    }
    return RetVal;
}

/**
* @brief This API set PARTITION SIZE.
* @note Function ID:[DES_FLS_API_206]
* @param [in] CseConfig: Cse configuration info about sfe and cse key size
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_SetPartition(const Flash_CseType *CseConfig)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
#if defined (AC7840X) || defined (AC7842X)
    uint32 LowData;
    uint32 HighData;

    DEVICE_ASSERT(CseConfig != NULL_PTR);

    OSIF_ENTER_CRITICAL(FLS_HAL_ID3);
    (void)Flash_Hal_UnLockCtrl();

    /* Create CSE partition*/
    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear. */
    if (FLASH_CSESIZE_0BYTES != CseConfig->KeySize)
        /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear. */
    {
        LowData = (uint32)(0x208U | ((uint32)CseConfig->Sfe << 24U));
        HighData = (uint32)CseConfig->KeySize;
        /* Clear RDCOLERR & ACCERR & FPVIOL flag in EFLASH_MODULE status register.
        Write 1 to clear */
        Flash_Reg_ClearStatus(); /*cstat !MISRAC2012-Rule-11.4 Flash register operation*/
        /* Passing parameter to the command */
        Flash_Reg_SetCommandReg(FLASH_PROGRAM_PARTITION);

        Flash_Reg_SetLowDataReg(LowData);
        Flash_Reg_SetHighDataReg(HighData);

        RetVal = Flash_Hal_StartCmd(0U);
    }
    else /* Delete CSE Partition */
    {
        /* Clear RDCOLERR & ACCERR & FPVIOL flag in EFLASH_MODULE status register.
        Write 1 to clear */
        Flash_Reg_ClearStatus(); /*cstat !MISRAC2012-Rule-11.4 Flash register operation*/
        /* Passing parameter to the command */
        Flash_Reg_SetCommandReg(FLASH_ERASE_PARTITION);
        RetVal = Flash_Hal_StartCmd(0U);
    }
    (void)Flash_Hal_LockCtrl();
    OSIF_EXIT_CRITICAL(FLS_HAL_ID3);
    Flash_Hal_GetDflashSize();
#else
    (void)CseConfig;
#endif
    return RetVal;
}

void Flash_Hal_Init(const Flash_Config *Config)
{

    /* Use the default configuration when the Fls_Cfg is empty and uninitialized */
    if (Config != NULL_PTR)
    {
        S_FlsCfg.Write_Timeout = Config->Write_Timeout;
        S_FlsCfg.Erase_Timeout = Config->Erase_Timeout;
        S_FlsCfg.LVD_Enable = Config->LVD_Enable;
    }

    (void)Flash_Hal_UnLockCtrl();

#if defined (AC7842X)
    Spm_Hal_SetFlashLVDCallback((Hal_CallbackType)Flash_Hal_LVD_Callback);
#else
    /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear. */
    Flash_Reg_SetLVDDetection(S_FlsCfg.LVD_Enable);
    /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear. */
#endif
    (void)Flash_Hal_LockCtrl();

    Flash_Hal_GetDflashSize();
}

static Hal_StatusType Flash_Hal_Erase
(
    uint8 CommandType,
    uint32 Address
)
{
    Hal_StatusType RetVal;

    /* Check CCIF to verify the previous command is completed */
    if (Flash_Reg_GetCommandCompleteStatusReg() != 0U)
    {
        /* Clear RDCOLERR & ACCERR & FPVIOL flag in EFLASH_MODULE status register.
        Write 1 to clear */
        Flash_Reg_ClearStatus(); /*cstat !MISRAC2012-Rule-11.4 Flash register operation*/

        /* Passing parameter to the command */
        Flash_Reg_SetCommandReg(CommandType);
        if (CommandType != FLASH_ERASE_ALL_BLOCK_UNSECURE)
        {
            Flash_Reg_SetAddressReg(Address);
        }
        RetVal = Flash_Hal_StartCmd(S_FlsCfg.Erase_Timeout);
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

/**
* @brief erase write protect infomation flash area
* @note Function ID:[DES_FLS_API_212]
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_WPInfoErase(void)
{
    Hal_StatusType RetVal;
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Erase(FLASH_OPTION_PAGE_ERASE, WP_INFO_BASE);
    (void)Flash_Hal_LockCtrl();

    return RetVal;
}
#endif

/**
* @brief erase all flash area
* @note Function ID:[DES_FLS_API_213]
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_AllErase(void)
{
    Hal_StatusType RetVal;

    /*has not CSE partition can erase all block, 0xF means no cse partition*/
    if (0xfU == Flash_Reg_GetDPartition())
    {
#if defined (AC7842X)
        /* Disable Lockup reset */
        CKGEN->RCM_EN &= 0xFFFFFFFDU;
#endif
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_ERASE_ALL_BLOCK_UNSECURE, FLASH_INVALID_ADDR);
        (void)Flash_Hal_LockCtrl();
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}

/**
* @brief Write one or more unit complete flash pages into given flash sector
* @note Function ID:[DES_FLS_API_203]
* @param [in] CommandType: flash command type
* @param [in] Address: Write Address
* @param [in] Length: Data length
* @param [in] DataPtr: Data pointer
* @return Hal_StatusType
*/
static Hal_StatusType Flash_Hal_Write
(
    uint8 CommandType,
    uint32 Address,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 TmpAddress = Address;
    uint32 DataLength = Length;
    uint32 i = 0;

    /*PRQA S 0310 ++ # make sure there are no alignment issues.*/
    /*PRQA S 3305 ++ # make sure there are no alignment issues.*/
    const uint32 *Flash_SourceAddressPtr = (const uint32 *)(DataPtr);
    /*PRQA S 0310 -- # make sure there are no alignment issues.*/
    /*PRQA S 3305 -- # make sure there are no alignment issues.*/
    /* Check CCIF to verify the previous command is completed */
    while ((STATUS_SUCCESS == RetVal) && (DataLength > 0U))
    {
        if (Flash_Reg_GetCommandCompleteStatusReg() != 0x0U)
        {
            /* Clear RDCOLERR & ACCERR & FPVIOL flag in EFLASH_MODULE status register.
            Write 1 to clear */
            Flash_Reg_ClearStatus(); /*cstat !MISRAC2012-Rule-11.4 Flash register operation*/

            /* Passing parameter to the command */
            Flash_Reg_SetCommandReg(CommandType);
            if (CommandType != FLASH_SECURITY_BY_PASS)
            {
                Flash_Reg_SetAddressReg(TmpAddress);
            }

            Flash_Reg_SetLowDataReg(Flash_SourceAddressPtr[i]);
            Flash_Reg_SetHighDataReg(Flash_SourceAddressPtr[i + 1U]);

            /* It is time to do word programming */
            RetVal = Flash_Hal_StartCmd(S_FlsCfg.Write_Timeout);

            /* start internal erase/program sequence */
            if (CommandType != FLASH_SECURITY_BY_PASS)
            {
                /*This may compare with */
                TmpAddress += PFLASH_WRITE_UNIT_SIZE;
            }
            DataLength -= PFLASH_WRITE_UNIT_SIZE;
            i += 2U;
        }
        else
        {
            RetVal = STATUS_ERROR;
        }
    }

    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

/**
* @brief write flash OTP area
* @note Function ID:[DES_FLS_API_222]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
*/
#if defined (AC7843X)
Hal_StatusType Flash_Hal_WriteOtp
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    return STATUS_SUCCESS;
}
#else
Hal_StatusType Flash_Hal_WriteOtp
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal;
    DEVICE_ASSERT((Offset + Length) <= 2048U);

    uint32 WriteAddress = OTP_INFO_BASE + Offset;
    (void)Flash_Hal_UnLockCtrl();

    RetVal = Flash_Hal_Write(FLASH_PROGRAM_ONCE, WriteAddress, Length, DataPtr);

    (void)Flash_Hal_LockCtrl();
    return RetVal;
}
#endif

/**
* @brief get flash error status and clear
* @note Function ID:[DES_FLS_API_223]
* @param [out] Addr: ECC error address
* @return error status
*/
uint32 Flash_Hal_ErrorStatusGetAndClear(uint32 *Addr)
{
    uint32 RetVal;

    DEVICE_ASSERT(NULL_PTR != Addr);

    RetVal = Flash_Reg_GetStatusReg();
    /*clear error status*/
    Flash_Reg_ClearStatus();
    /*get ecc error address*/
    if (FLASH_STAT_DFDIF_Msk == (RetVal & FLASH_STAT_DFDIF_Msk))
    {
        /*PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear. */
        *Addr = Flash_Reg_ECCErrorAddr();
        /*PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear. */
    }
    return RetVal;
}
#endif
/**
* @brief verify input address
* @note Function ID:[DES_FLS_API_215]
* @param [in] CommandType: flash command type
* @param [in] Address: Write Address
* @param [in] Length: Data length
* @return Hal_StatusType
*/
static Hal_StatusType Flash_Hal_Verify
(
    uint8 CommandType,
    uint32 Address,
    uint32 Length
)
{
    Hal_StatusType RetVal;

    /*Check flash whether process complete*/
    if (0x0U == Flash_Reg_GetCommandCompleteStatusReg())
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        Flash_Reg_ClearStatus(); /*cstat !MISRAC2012-Rule-11.4 Flash register operation */

        Flash_Reg_SetCommandReg(CommandType);
        if (CommandType != FLASH_VERIFY_ALL_BLOCK)
        {
            Flash_Reg_SetAddressReg(Address);
            Flash_Reg_SetLengthReg(Length >> 3U); /*one means eight address length, so need divide 8 */
        }

        RetVal = Flash_Hal_StartCmd(0U);
    }
    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API
/**
* @brief verify all flash area
* @note Function ID:[DES_FLS_API_216]
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_AllVerify(void)
{
    Hal_StatusType RetVal = STATUS_ERROR;

#if defined (AC7840X) || defined (AC7843X)
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Verify(FLASH_VERIFY_ALL_BLOCK, FLASH_INVALID_ADDR, 8U);
    (void)Flash_Hal_LockCtrl();
#endif
#if defined (AC7840X) || defined (AC7842X)
    if (STATUS_ERROR == RetVal)
    {
        /* software to verify */
        RetVal = Flash_Hal_SW_Verify(PFLASH_BASE, PFLASH_BLOCK_SIZE);
        if (STATUS_SUCCESS == RetVal)
        {
            RetVal = Flash_Hal_SW_Verify(DFLASH_BASE, S_DFlashSize);
        }
    }
#endif

    return RetVal;
}
#endif
/**
* @brief enable read protect function
* @note Function ID:[DES_FLS_API_217]
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_ReadProtectSet(void)
{
    uint32 Val[2U];
    Hal_StatusType RetVal;

    /*enable Read protect*/
    Val[0] = (READ_MEM32(COMP_CTRL_BASE) & 0xFCU);
    Val[1] = 0xFFU;
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, COMP_CTRL_BASE, 8U, (uint8 *)Val);
    (void)Flash_Hal_LockCtrl();

    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

/**
* @brief disable/enable write protect
* @note Function ID:[DES_FLS_API_238]
* @param [in] En: disable/enable write protect configuration
* @param [in] Data: pflash/dflash write protect config value
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_WriteProtectSet(boolean En, const uint8 *Data)
{
    Hal_StatusType RetVal;

    if (TRUE == En)
    {
        DEVICE_ASSERT(Data != NULL_PTR);
        /*enable write protect */
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, WP_INFO_BASE, FLASH_WPROT_BYTES, Data);
        (void)Flash_Hal_LockCtrl();
    }
    else
    {
        /*disable write protect */
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_OPTION_PAGE_ERASE, WP_INFO_BASE);
        (void)Flash_Hal_LockCtrl();
    }

    return RetVal;
}
#endif
/**
* @brief set backdoor key
* @note Function ID:[DES_FLS_API_233]
* @param [in] Data:  backdoor key
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_BackdoorSet(const uint8 *Data)
{
    uint32 Val[2U];
    Hal_StatusType RetVal;

    Val[0] = (READ_MEM32(COMP_CTRL_BASE) & 0xCFU);
    Val[1] = 0xFFU;
    (void)Flash_Hal_UnLockCtrl();
    /*enable backdoor */
    RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, COMP_CTRL_BASE, 8U, (uint8 *)Val);
    if (STATUS_SUCCESS == RetVal)
    {
        /*set backdoor key*/
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, BACKDOOR_KEY_BASE, 8U, Data);
    }
    (void)Flash_Hal_LockCtrl();

    return RetVal;
}

/**
* @brief verify backdoor key
* @note Function ID:[DES_FLS_API_234]
* @param [in] Data:  backdoor key
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_BackdoorVerify(const uint8 *Data)
{
    Hal_StatusType RetVal = STATUS_ERROR;
    DEVICE_ASSERT(Data != NULL_PTR);

    /*backdoor whether enable ,enable can verify,disable return error*/
    if ((READ_MEM32(COMP_CTRL_BASE) & 0x30U) != 0x30U)
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Write(FLASH_SECURITY_BY_PASS, FLASH_INVALID_ADDR, 8U, Data);
        (void)Flash_Hal_LockCtrl();
        if ((RetVal == STATUS_SUCCESS) && (Flash_Reg_GetBackdoorStatusReg() != 0U))
        {
            RetVal = STATUS_ERROR;
        }
    }
    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

/**
* @brief erase cse info area
* @note Function ID:[DES_FLS_API_235]
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_EraseCseInfo(void)
{
    Hal_StatusType RetVal = STATUS_BUSY;
#if defined (AC7840X) || defined (AC7842X)
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, CSE_SECRET_KEY_BASE);
    (void)Flash_Hal_LockCtrl();
#endif
    return RetVal;
}

/**
* @brief program cse info
* @note Function ID:[DES_FLS_API_236]
* @param [in] Addr:  info address
* @param [in] Length:  data length
* @param [in] Data:  cse info
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_ProgramCseInfo
(
    uint32 Addr,
    uint32 Length,
    const uint8 *Data
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
#if defined (AC7840X) || defined (AC7842X)
    uint32 TmpAddr = Addr;
    uint32 TmpLength = Length;
    uint32 i = 0;
    /*PRQA S 0310 ++ # make sure there are no alignment issues.*/
    /*PRQA S 3305 ++ # make sure there are no alignment issues.*/
    const uint32 *DataPtr = (const uint32 *)Data;
    /*PRQA S 3305 -- # make sure there are no alignment issues.*/
    /*PRQA S 0310 -- # make sure there are no alignment issues.*/
    DEVICE_ASSERT(0U == (TmpLength % PFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT((TmpAddr >= CSE_SECRET_KEY_BASE) && (TmpAddr <= CSE_LOCK_BASE));
    DEVICE_ASSERT(NULL_PTR != Data);

    (void)Flash_Hal_UnLockCtrl();
    while ((TmpLength > 0U) && (STATUS_SUCCESS == RetVal))
    {
        /* Check if command is completed by CCIF */
        if (Flash_Reg_GetCommandCompleteStatusReg() != 0U)
        {
            /* Write 1 to clear COLLERR & ACCERR & FPVIOL flag in FLASH status register */
            Flash_Reg_ClearStatus();

            /* Set flash execute command */
            Flash_Reg_SetCommandReg(FLASH_PROGRAM_CSE);
            Flash_Reg_SetAddressReg(TmpAddr);

            Flash_Reg_SetLowDataReg(DataPtr[i]);
            Flash_Reg_SetHighDataReg(DataPtr[i + 1U]);

            /* Calling FLASH start command sequence function to execute the command */
            RetVal = Flash_Hal_StartCmd(0U);

            /* Update program address for next operation */
            TmpAddr += PFLASH_WRITE_UNIT_SIZE;
            /* Update size for next operation */
            TmpLength -= PFLASH_WRITE_UNIT_SIZE;
            /* Increase the data source pointer by 8 */
            i += 2U;
        }
        else
        {
            RetVal = STATUS_BUSY;
        }
    }
    (void)Flash_Hal_LockCtrl();
#endif
    return RetVal;
}

#if defined (AC7842X) || defined (AC7843X)
/**
* @brief swap flash program bank
* @note Function ID:[DES_FLS_API_248]
* @param [in] En: 0 -> swapA  1 -> swapB
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_Swap(boolean En)
{
    uint32 Data[4U];
    Hal_StatusType RetVal;

    /* program swap location */
    if (En == TRUE)
    {
        Data[0] = 0x00U;
        Data[1] = 0x00U;
        Data[2] = 0x00U;
        Data[3] = 0x00U;
        (void)Flash_Hal_UnLockCtrl();
        /* config swap location */
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, SWAP_INFO_BASE, 16U, (uint8 *)Data);
        (void)Flash_Hal_LockCtrl();
    }
    else /* erase swap location */
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, SWAP_INFO_BASE);
        (void)Flash_Hal_LockCtrl();
    }

    return RetVal;
}
#else
/**
* @brief swap flash program bank
* @note Function ID:[DES_FLS_API_248]
* @param [in] En: 0 -> swapA  1 -> swapB
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_Swap(boolean En)
{
    (void)En;
    return STATUS_SUCCESS;
}
#endif
#endif

/**
* @brief Read one or more bytes from given flash info
* @note Function ID:[DES_FLS_API_210]
* @param [in] Offset: info address offset
* @param [in] Length: Flash read length
* @param [in] DataPtr: source data buffer address
* @return None
*/
void Flash_Hal_InfoRead
(
    uint32 Offset,
    uint32 Length,
    uint8 *DataPtr
)
{
    uint32 TargetAddress;
    uint32 DataLength = Length;
    uint8 *TmpDataPtr = DataPtr;

    DEVICE_ASSERT(Offset < FLASH_INFO_ADDR_SIZE);
    DEVICE_ASSERT((Offset + Length) <= FLASH_INFO_ADDR_SIZE);
    DEVICE_ASSERT(DataPtr != NULL_PTR);
    TargetAddress = FLASH_INFO_ADDR_BASE + Offset;

    /* End of read pointer */
    while (0x0U < DataLength)
    {
        /* read flash location */
        *TmpDataPtr = *(uint8 *)TargetAddress;
        TmpDataPtr++;
        TargetAddress++;
        DataLength -= 0x1U;
    }
}

/**
* @brief Write one or more unit from given flash info
* @note Function ID:[DES_FLS_API_211]
* @param [in] Offset: info address offset
* @param [in] Length: Flash read length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_InfoWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal;
    uint32 TargetAddress;

    DEVICE_ASSERT(Offset < FLASH_INFO_ADDR_SIZE);
    DEVICE_ASSERT((Offset + Length) <= FLASH_INFO_ADDR_SIZE);
    DEVICE_ASSERT(0U == (Offset % PFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % PFLASH_WRITE_UNIT_SIZE));
    TargetAddress = FLASH_INFO_ADDR_BASE + Offset;

    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, TargetAddress, Length, DataPtr);
    (void)Flash_Hal_LockCtrl();
    return RetVal;
}

#ifndef FLS_SDK_NON_EXTENDED_API

/**
* @brief Erase one page from given flash info
* @note Function ID:[DES_FLS_API_204]
* @param [in] Offset: info address offset
* @return Hal_StatusType
*/
Hal_StatusType Flash_Hal_InfoErase(uint32 Offset)
{
    Hal_StatusType RetVal;
    uint32 TargetAddress;

    DEVICE_ASSERT(Offset < FLASH_INFO_ADDR_SIZE);
    DEVICE_ASSERT(0U == (Offset % PFLASH_WRITE_UNIT_SIZE));
    TargetAddress = FLASH_INFO_ADDR_BASE + Offset;

    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, TargetAddress);
    (void)Flash_Hal_LockCtrl();
    return RetVal;
}
#endif

#ifdef PFLASH_ENABLE
Hal_StatusType PFlash_Hal_PageErase(uint32 Offset)
{
    Hal_StatusType RetVal;
    uint32 TargetAddr;

    DEVICE_ASSERT(Offset < PFLASH_BLOCK_SIZE);
    DEVICE_ASSERT(0U == (Offset % PFLASH_PAGE_SIZE));

    /*PRQA S 2986 ++ # considered an invalid operation, it is actually meaningful.*/
    TargetAddr = PFLASH_BASE + Offset;
    /*PRQA S 2986 -- # considered an invalid operation, it is actually meaningful.*/
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, TargetAddr);
    (void)Flash_Hal_LockCtrl();
    return RetVal;
}

/**
* @brief erase pflash area with block erase
* @note Function ID:[DES_FLS_API_219]
* @return Hal_StatusType
*/
/*PRQA S 3408 ++ # ues inline function */
Hal_StatusType PFlash_Hal_BlockErase(void)
{
    Hal_StatusType RetVal;

    /*has not CSE partition can erase all block, 0xF means no cse partition*/
    if (0xfU == Flash_Reg_GetDPartition())
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_ERASE_BLOCK, PFLASH_BASE);
        (void)Flash_Hal_LockCtrl();
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}
/*PRQA S 3408 -- */

/**
* @brief write pflash area with page program
* @note Function ID:[DES_FLS_API_220]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
*/
Hal_StatusType PFlash_Hal_PageWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal;
    DEVICE_ASSERT(Length <= PFLASH_PAGE_SIZE);
    DEVICE_ASSERT(Offset < PFLASH_BLOCK_SIZE);
    DEVICE_ASSERT(0U == (Offset % PFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % PFLASH_WRITE_UNIT_SIZE));
    /*PRQA S 2986 ++ # considered an invalid operation, it is actually meaningful.*/
    uint32 WriteAddress = PFLASH_BASE + Offset;
    /*PRQA S 2986 -- # considered an invalid operation, it is actually meaningful.*/
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, WriteAddress, Length, DataPtr);
    (void)Flash_Hal_LockCtrl();
    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

#if defined (AC7843X)
/**
* @brief write pflash area with section program
* @note Function ID:[DES_FLS_API_221]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
*/
Hal_StatusType PFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    return STATUS_SUCCESS;
}
#else
/**
* @brief write pflash area with section program
* @note Function ID:[DES_FLS_API_221]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
*/
Hal_StatusType PFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_ERROR;
    uint32 Len = Length;
    uint32 Addr = FLEXRAM_BASE;
    const uint8 *DPtr = DataPtr;

    DEVICE_ASSERT(Length <= PFLASH_PAGE_SIZE);
    DEVICE_ASSERT(Offset < PFLASH_BLOCK_SIZE);
    DEVICE_ASSERT(0U == (Offset % PFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % PFLASH_WRITE_UNIT_SIZE));

    /*PRQA S 2986 ++ # considered an invalid operation, it is actually meaningful.*/
    uint32 WriteAddress = PFLASH_BASE + Offset;
    /*PRQA S 2986 -- # considered an invalid operation, it is actually meaningful.*/
    /*if true means CSE not use flex ram*/
    if (0xFU == Flash_Reg_GetEPartition())
    {
        (void)Flash_Hal_UnLockCtrl();
        /*set flex ram*/
        Flash_Reg_SetCommandReg(FLASH_SET_FLEXRAM);
        Flash_Reg_SetAddressReg(0xFFU);
        RetVal = Flash_Hal_StartCmd(0U);
        /*set data to flex ram*/
        while (Len > 0U)
        {
            *(uint8 *)Addr = *DPtr;
            DPtr++;
            Addr++;
            Len--;
        }

        if (STATUS_SUCCESS == RetVal)
        {
            Addr = FLEXRAM_BASE;
            Flash_Reg_SetLengthReg(Length / PFLASH_WRITE_UNIT_SIZE);
            RetVal = Flash_Hal_Write(FLASH_PROGRAM_SECTION, WriteAddress, PFLASH_WRITE_UNIT_SIZE, (uint8 *)Addr);
        }

        (void)Flash_Hal_LockCtrl();
    }
    return RetVal;
}
#endif
#endif
/**
* @brief verify pflash area with section verify
* @note Function ID:[DES_FLS_API_237]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash verify length
* @return Hal_StatusType
*/
Hal_StatusType PFlash_Hal_SectionVerify
(
    uint32 Offset,
    uint32 Length
)
{
    Hal_StatusType RetVal;
    DEVICE_ASSERT((Offset + Length) <= PFLASH_BLOCK_SIZE);

    /*PRQA S 2986 ++ # considered an invalid operation, it is actually meaningful.*/
    uint32 WriteAddress = PFLASH_BASE + Offset;
    /*PRQA S 2986 -- # considered an invalid operation, it is actually meaningful.*/
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Verify(FLASH_VERIFY_SECTION, WriteAddress, Length);
    (void)Flash_Hal_LockCtrl();
#if defined (AC7840X)
    if (STATUS_ERROR == RetVal)
    {
        /* software to verify */
        RetVal = Flash_Hal_SW_Verify(WriteAddress, Length);
    }
#endif
    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

/**
* @brief verify pflash area with block verify
* @note Function ID:[DES_FLS_API_224]
* @return Hal_StatusType
*/
Hal_StatusType PFlash_Hal_BlockVerify(void)
{
    Hal_StatusType RetVal;

    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Verify(FLASH_VERIFY_BLOCK, PFLASH_BASE, 8U);
    (void)Flash_Hal_LockCtrl();
#if defined (AC7840X)
    if (STATUS_ERROR == RetVal)
    {
        /* software to verify */
        RetVal = Flash_Hal_SW_Verify(PFLASH_BASE, PFLASH_BLOCK_SIZE);
    }
#endif

    return RetVal;
}
#endif
/**
* @brief Read one or more bytes from given pflash
* @note Function ID:[DES_FLS_API_204]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash read length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
*/
Hal_StatusType PFlash_Hal_Read
(
    uint32 Offset,
    uint32 Length,
    uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 ReadAddress;
    uint32 DataLength = Length;
    uint8 *TmpDataPtr = DataPtr;

    DEVICE_ASSERT((Offset + Length) <= PFLASH_BLOCK_SIZE);
    DEVICE_ASSERT(DataPtr != NULL_PTR);

    /*PRQA S 2986 ++ # considered an invalid operation, it is actually meaningful.*/
    ReadAddress = PFLASH_BASE + Offset;
    /*PRQA S 2986 -- # considered an invalid operation, it is actually meaningful.*/
    /* End of read pointer */
    while (0x0U < DataLength)
    {
        /* read flash location */
        *TmpDataPtr = *(uint8 *)ReadAddress;
        TmpDataPtr++; /*cstat !MISRAC2012-Rule-17.8 The value saved in the pointer needs to be transfered*/
        ReadAddress++;
        DataLength -= 0x1U;
    }

    return RetVal;
}
#endif

#ifdef DFLASH_ENABLE
Hal_StatusType DFlash_Hal_PageErase(uint32 Offset)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 TargetAddr;
    FlashDeviceType DevLock;

    DEVICE_ASSERT(Offset <= S_DFlashSize);
    DEVICE_ASSERT(0U == (Offset % DFLASH_PAGE_SIZE));

    TargetAddr = DFLASH_BASE + Offset;

    DevLock = System_FlsDeviceTryLock(FLS_DEV);
    if (DevLock != FLS_DEV)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, TargetAddr);
        (void)Flash_Hal_LockCtrl();
        (void)System_FlsDeviceUnlock(FLS_DEV);
    }

    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

/**
* @brief erase dflash area with block erase
* @note Function ID:[DES_FLS_API_226]
* @return Hal_StatusType
*/
Hal_StatusType DFlash_Hal_BlockErase(void)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;

    /*has not CSE partition can erase all block, 0xF means no cse partition*/
    if (0xfU == Flash_Reg_GetDPartition())
    {
        DevLock = System_FlsDeviceTryLock(FLS_DEV);
        if (DevLock != FLS_DEV)
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            (void)Flash_Hal_UnLockCtrl();
            RetVal = Flash_Hal_Erase(FLASH_ERASE_BLOCK, DFLASH_BASE);
            (void)Flash_Hal_LockCtrl();
            (void)System_FlsDeviceUnlock(FLS_DEV);
        }
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}
#endif
/**
* @brief write dflash area with page program
* @note Function ID:[DES_FLS_API_227]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
*/
Hal_StatusType DFlash_Hal_PageWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;
    DEVICE_ASSERT(0U == (Offset % DFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % DFLASH_WRITE_UNIT_SIZE));

    uint32 WriteAddress = DFLASH_BASE + Offset;
    if ((Offset + Length) > S_DFlashSize)
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        DevLock = System_FlsDeviceTryLock(FLS_DEV);
        if (DevLock != FLS_DEV)
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            (void)Flash_Hal_UnLockCtrl();
            RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, WriteAddress, Length, DataPtr);
            (void)Flash_Hal_LockCtrl();
            (void)System_FlsDeviceUnlock(FLS_DEV);
        }
    }
    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

/**
* @brief write dflash area with section program
* @note Function ID:[DES_FLS_API_228]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
*/
#if defined (AC7843X)
Hal_StatusType DFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    return STATUS_SUCCESS;
}
#else
Hal_StatusType DFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;
    uint32 Len = Length;
    uint32 Addr = FLEXRAM_BASE;
    const uint8 *DPtr = DataPtr;
    DEVICE_ASSERT(Length <= DFLASH_PAGE_SIZE);
    DEVICE_ASSERT(0U == (Offset % DFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % DFLASH_WRITE_UNIT_SIZE));
    (void)DataPtr;
    uint32 WriteAddress = DFLASH_BASE + Offset;
    if ((Offset + Length) > S_DFlashSize)
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        DevLock = System_FlsDeviceTryLock(FLS_DEV);
        if (DevLock != FLS_DEV)
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            (void)Flash_Hal_UnLockCtrl();
            /*set flex ram*/
            Flash_Reg_SetCommandReg(FLASH_SET_FLEXRAM);
            Flash_Reg_SetAddressReg(0xFFU);
            RetVal = Flash_Hal_StartCmd(0U);
            /*set data to flex ram*/
            while (Len > 0U)
            {
                *(uint8 *)Addr = *DPtr;
                DPtr++;
                Addr++;
                Len--;
            }

            if (STATUS_SUCCESS == RetVal)
            {
                Flash_Reg_SetLengthReg(Length / DFLASH_WRITE_UNIT_SIZE);
                RetVal = Flash_Hal_Write(FLASH_PROGRAM_SECTION, WriteAddress,
                                     DFLASH_WRITE_UNIT_SIZE, (uint8 *)FLEXRAM_BASE);
            }

            (void)Flash_Hal_LockCtrl();
            (void)System_FlsDeviceUnlock(FLS_DEV);
        }
    }
    return RetVal;
}
#endif
#endif
/**
* @brief verify dflash area with section verify
* @note Function ID:[DES_FLS_API_229]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash verify length
* @return Hal_StatusType
*/
Hal_StatusType DFlash_Hal_SectionVerify
(
    uint32 Offset,
    uint32 Length
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;
    if ((Offset + Length) > S_DFlashSize)
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        uint32 WriteAddress = DFLASH_BASE + Offset;

        DevLock = System_FlsDeviceTryLock(FLS_DEV);
        if (DevLock != FLS_DEV)
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            (void)Flash_Hal_UnLockCtrl();
            RetVal = Flash_Hal_Verify(FLASH_VERIFY_SECTION, WriteAddress, Length);
            (void)Flash_Hal_LockCtrl();
#if defined (AC7840X)
            if (STATUS_ERROR == RetVal)
            {
                /* software to verify */
                RetVal = Flash_Hal_SW_Verify(WriteAddress, Length);
            }
#endif
            (void)System_FlsDeviceUnlock(FLS_DEV);
        }
    }

    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

/**
* @brief verify dflash area with block verify
* @note Function ID:[DES_FLS_API_230]
* @return Hal_StatusType
*/
Hal_StatusType DFlash_Hal_BlockVerify(void)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;

    DevLock = System_FlsDeviceTryLock(FLS_DEV);
    if (DevLock != FLS_DEV)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Verify(FLASH_VERIFY_BLOCK, DFLASH_BASE, 8U);
        (void)Flash_Hal_LockCtrl();
#if defined (AC7840X)
        if (STATUS_ERROR == RetVal)
        {
            /* software to verify */
            RetVal = Flash_Hal_SW_Verify(DFLASH_BASE, S_DFlashSize);
        }
#endif
        (void)System_FlsDeviceUnlock(FLS_DEV);
    }

    return RetVal;
}
#endif
/**
* @brief Read one or more bytes from given dflash
* @note Function ID:[DES_FLS_API_231]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash read length
* @param [in] DataPtr: source data buffer address
* @return None
*/
Hal_StatusType DFlash_Hal_Read
(
    uint32 Offset,
    uint32 Length,
    uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 ReadAddress;
    FlashDeviceType DevLock;
    uint32 DataLength = Length;
    uint8 *TmpDataPtr = DataPtr;
    DEVICE_ASSERT((Offset + Length) <= S_DFlashSize);
    DEVICE_ASSERT(DataPtr != NULL_PTR);

    ReadAddress = DFLASH_BASE + Offset;
    DevLock = System_FlsDeviceTryLock(FLS_DEV);
    if (DevLock != FLS_DEV)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        /* End of read pointer */
        while (0x0U < DataLength)
        {
            /* read flash location */
            *TmpDataPtr = *(volatile uint8 *)ReadAddress;
            TmpDataPtr++;
            ReadAddress++;
            DataLength -= 0x1U;
        }
        (void)System_FlsDeviceUnlock(FLS_DEV);
    }

    return RetVal;
}
#endif
