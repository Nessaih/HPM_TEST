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
 * @file AC784xx_Fls_Reg.h
 *
 * @brief This file provides extern Reg Flash api.
 */

#ifndef AC784XX_FLS_REG_H
#define AC784XX_FLS_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/*Flash error status*/
#define FLASH_STAT_ERROR_BITS (FLASH_STAT_DFDIF_Msk |\
                               FLASH_STAT_PVIOLF_Msk |\
                               FLASH_STAT_VERIFYERR_Msk |\
                               FLASH_STAT_ACCERRF_Msk |\
                               FLASH_STAT_COLLERRF_Msk)
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

/* =====================================  Functions definition  ===================================== */
/**
* @brief Set eflash lock
* @note Function ID:[DES_FLS_API_049]
* @return None
*/
LOCAL_INLINE void Flash_Reg_Lock(void)
{
    WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY2); /*cstat !MISRAC2012-Rule-11.4*/
    WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY1); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set eflash unlock
* @note Function ID:[DES_FLS_API_049]
* @return None
*/
LOCAL_INLINE void Flash_Reg_UnLock(void)
{
    WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY1); /*cstat !MISRAC2012-Rule-11.4*/
    WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY2); /*cstat !MISRAC2012-Rule-11.4*/
}

LOCAL_INLINE void Flash_Reg_ClearStatus(void)
{
    MODIFY_REG32(FLASH->STAT, FLASH_STAT_ERROR_BITS, 0U, FLASH_STAT_ERROR_BITS); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Get the can eflash controler lock status
* @note Function ID:[DES_FLS_API_050]
* @return Lock Status.
*/
LOCAL_INLINE uint8 Flash_Reg_GetLockStatusReg(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->STAT) & FLASH_STAT_LOCK_Msk) >> FLASH_STAT_LOCK_Pos);
}

/**
* @brief Set P-Flash write protection status
* @note Function ID:[DES_FLS_API_053]
* @param [in] ProtectPos: protect bits Postion
* @param [in] ProtectStatus: protect status
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetPFlashProtection(uint8 ProtectPos, uint32 ProtectStatus)
{
#if defined (AC7843X)
    if (ProtectPos == 0)
    {
        WRITE_REG32(FLASH->P0PROT0, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
    else if (ProtectPos == 1)
    {
        WRITE_REG32(FLASH->P0PROT1, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
    else if (ProtectPos == 2)
    {
        WRITE_REG32(FLASH->P1PROT0, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
    else if (ProtectPos == 3)
    {
        WRITE_REG32(FLASH->P1PROT1, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
#else
    (void)ProtectPos;
    WRITE_REG32(FLASH->PPROT, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
#endif
}

/**
* @brief Set D-Flash write protection status
* @note Function ID:[DES_FLS_API_054]
* @param [in] ProtectStatus: protect status
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetDFlashProtection(uint16 ProtectStatus)
{
    WRITE_REG32(FLASH->DPROT, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
}

/*!
 * @brief Set command complete interrupt register.
 *
 * @param [in] enable: command complete interrupt enable status
 * @return none
 */
LOCAL_INLINE void Flash_Reg_EnableCompleteIRQ(boolean enable)
{
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_CCIE_Msk, FLASH_CNFG_CCIE_Pos, enable);
}

/*!
 * @brief Set collision error interrupt register.
 *
 * @param [in] enable: collision interrupt enable status
 * @return none
 */
LOCAL_INLINE void Flash_Reg_EnableCollisionIRQ(boolean enable)
{
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_COLLE_Msk, FLASH_CNFG_COLLE_Pos, enable);
}

/*!
 * @brief Get Flash ECC 2-bit error interrupt flag.
 *
 * @return Flash ECC 2-bit error interrupt flag
 */
LOCAL_INLINE uint8 Flash_Reg_GetECCIRQFlag(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_DFDIF_Msk) >> FLASH_STAT_PVIOLF_Pos);
}

/*!
 * @brief Clear Flash ECC 2-bit error interrupt flag.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ClearECCIRQFlag(void)
{
    FLASH->STAT |= FLASH_STAT_DFDIF_Msk;
}

/**
* @brief Set program or erase start address
* @note Function ID:[DES_FLS_API_055]
* @param [in] Addr: address value
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetAddressReg(uint32 Addr)
{
    WRITE_REG32(FLASH->ADDR, Addr); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set program or erase length
* @note Function ID:[DES_FLS_API_056]
* @param [in] Len: len value
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetLengthReg(uint32 Len)
{
#if defined (AC7843X)
    WRITE_REG32(FLASH->LEN, (Len == 0) ? 0 : Len - 1); /*cstat !MISRAC2012-Rule-11.4*/
#else
    WRITE_REG32(FLASH->LEN, Len); /*cstat !MISRAC2012-Rule-11.4*/
#endif
}

/**
* @brief Get abort page erase status
* @note Function ID:[DES_FLS_API_057]
* @return Abort page erase status
*/
LOCAL_INLINE uint8 Flash_Reg_GetAbortPageEraseStatusReg(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->CNFG) & FLASH_CNFG_PERABORT_Msk) >> FLASH_CNFG_PERABORT_Pos);
}

/**
* @brief Flash bort page erase
* @note Function ID:[DES_FLS_API_058]
* @return None
*/
LOCAL_INLINE void Flash_Reg_AbortPageEraseReg(void)
{
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_PERABORT_Msk, FLASH_CNFG_PERABORT_Pos, 1U); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Get the error status
* @note Function ID:[DES_FLS_API_059]
* @return Status
*/
LOCAL_INLINE uint32 Flash_Reg_GetStatusReg(void)
{
    return (READ_REG32(FLASH->STAT) & FLASH_STAT_ERROR_BITS); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set command
* @note Function ID:[DES_FLS_API_060]
* @param [in] Cmd: Command value
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetCommandReg(uint32 Cmd)
{
    WRITE_REG32(FLASH->CMD, Cmd); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Trigger the command to start
* @note Function ID:[DES_FLS_API_061]
* @return None
*/
LOCAL_INLINE void Flash_Reg_TrigCtrlCmdReg(void)
{
    WRITE_REG32(FLASH->CST, FLASH_CST_START_Msk); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Get Trigger the FLASH operation command excute status
* @note Function ID:[DES_FLS_API_062]
* @return Trig control cmd reg status
*/
LOCAL_INLINE uint8 Flash_Reg_GetTrigCtrlCmdRegStatus(void)
{
    uint8 TrigCtrlCmdRegStatus;
    TrigCtrlCmdRegStatus = (uint8)(READ_REG32(FLASH->CST) & FLASH_CST_START_Msk); /*cstat !MISRAC2012-Rule-11.4*/
    return TrigCtrlCmdRegStatus;
}

/**
* @brief Get command completed status
* @note Function ID:[DES_FLS_API_063]
* @return Command complete status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCommandCompleteStatusReg(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->STAT) & FLASH_STAT_CCIF_Msk) >> FLASH_STAT_CCIF_Pos);
}

/**
* @brief Get backdoor verify status
* @note Function ID:[DES_FLS_API_063]
* @return Backdoor verify status
*/
LOCAL_INLINE uint8 Flash_Reg_GetBackdoorStatusReg(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->STAT) & FLASH_STAT_BDKERR_Msk) >> FLASH_STAT_BDKERR_Pos);
}

/*!
 * @brief Set ECC 2-bit error interrupt register.
 *
 * @param [in] enable: ECC 2-bit error interrupt enable status
 * @return none
 */
LOCAL_INLINE void Flash_Reg_EnableECCIRQ(boolean enable)
{
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_DFDIE_Msk, FLASH_CNFG_DFDIE_Pos, enable);
}

/**
* @brief Get P-Flash write protection status
* @note Function ID:[DES_FLS_API_064]
* @param [in] ProtectPos: Get Pflash Protection status
* @return PFlash protection
*/
LOCAL_INLINE uint32 Flash_Reg_GetPFlashProtection(uint8 ProtectPos)
{
    uint32 prot;
#if defined (AC7843X)
    if (ProtectPos < 4)
    {
        prot = READ_REG32(FLASH->P0PROT0 + ProtectPos * 4);
    }
    else
    {
        prot = 0;
    }
#else
    (void)ProtectPos;
    prot = READ_REG32(FLASH->PPROT); /*cstat !MISRAC2012-Rule-11.4*/
#endif
    return prot;
}

/**
* @brief Get the error status
* @note Function ID:[DES_FLS_API_065]
* @return DFlash protection
*/
LOCAL_INLINE uint32 Flash_Reg_GetDFlashProtection(void)
{
    return (READ_REG32(FLASH->DPROT) & FLASH_DPROT_DPROT_Msk); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set low 32-bit data
* @note Function ID:[DES_FLS_API_066]
* @param [in] Data: Data write to regsiter
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetLowDataReg(uint32 Data)
{
    WRITE_REG32(FLASH->DATA0, Data); /*cstat !MISRAC2012-Rule-11.4*/
}

/*!
 * @brief get low 32-bit data register value.
 *
 * @return Low 32-bit data register value
 */
LOCAL_INLINE uint32 Flash_Reg_GetLowDataReg(void)
{
    return (READ_REG32(FLASH->DATA0));
}

/*!
 * @brief Get high 32-bit data register value.
 *
 * @return High 32-bit data register value
 */
LOCAL_INLINE uint32 Flash_Reg_GetHighDataReg(void)
{
    return (READ_REG32(FLASH->DATA1));
}

/**
* @brief Set high 32-bit data
* @note Function ID:[DES_FLS_API_067]
* @param [in] Data: Data write to regsiter
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetHighDataReg(uint32 Data)
{
    WRITE_REG32(FLASH->DATA1, Data); /*cstat !MISRAC2012-Rule-11.4*/
}

#if defined (AC7843X)
LOCAL_INLINE uint8 Flash_Reg_GetDPartition(void)
{
    return 0xFU;
}

LOCAL_INLINE uint8 Flash_Reg_GetEPartition(void)
{
    return 0xFU;
}
LOCAL_INLINE uint8 Flash_Reg_GetCSEPartition(void)
{
    return 0xFU;
}

/*!
 * @brief Get command completed status of all flash bank, exclude D-Flash bank1 when HSM exist.
 *
 * @return 1: Command is completed
 *         0: Command is not completed
 */
LOCAL_INLINE uint8 Flash_Reg_GetEFCmdCompleteStatus(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_EFCCIF_Msk) >> FLASH_STAT_EFCCIF_Pos);
}

/*!
 * @brief Clear command complete interrupt register.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ClearCompleteIRQ(void)
{
    FLASH->STAT |= FLASH_STAT_CMDONOFF_Msk;
}
#else
/**
* @brief Get DFLASH Partition
* @note Function ID:[DES_FLS_API_067]
* @return PFLASH PARTITION SIZE
*/
LOCAL_INLINE uint8 Flash_Reg_GetDPartition(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->PART) & FLASH_PART_DPART_Msk) >> FLASH_PART_DPART_Pos);
}

/**
* @brief Get FlexRAM Partition
* @note Function ID:[DES_FLS_API_067]
* @return DFLASH PARTITION SIZE
*/
LOCAL_INLINE uint8 Flash_Reg_GetEPartition(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->PART) & FLASH_PART_EPART_Msk) >> FLASH_PART_EPART_Pos);
}

/**
* @brief Get CSE Partition
* @note Function ID:[DES_FLS_API_067]
* @return CSE PARTITION SIZE
*/
LOCAL_INLINE uint8 Flash_Reg_GetCSEPartition(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->PART) & FLASH_PART_CSEKEYSIZE_Msk) >> FLASH_PART_CSEKEYSIZE_Pos);
}

/*!
 * @brief Get FlexRAM ECC 1-bit error flag.
 *
 * @return Flash FlexRAM ECC 1-bit error interrupt flag
 */
LOCAL_INLINE uint8 FLASH_GetFRAMECC1BitFlag(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_FRAMECC1F_Msk) >> FLASH_STAT_FRAMECC1F_Pos);
}

/*!
 * @brief Clear FlexRAM ECC 1-bit error flag.
 *
 * @return none
 */
LOCAL_INLINE void FLASH_ClearFRAMECC1BitFlag(void)
{
    FLASH->STAT |= FLASH_STAT_FRAMECC1F_Msk;
}

/*!
 * @brief Get FlexRAM ECC 2-bit error flag.
 *
 * @return Flash FlexRAM ECC 2-bit error interrupt flag
 */
LOCAL_INLINE uint8 FLASH_GetFRAMECC2BitFlag(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_FRAMECC2F_Msk) >> FLASH_STAT_FRAMECC2F_Pos);
}

/*!
 * @brief Clear FlexRAM ECC 2-bit error flag.
 *
 * @return none
 */
LOCAL_INLINE void FLASH_ClearFRAMECC2BitFlag(void)
{
    FLASH->STAT |= FLASH_STAT_FRAMECC2F_Msk;
}
#endif

/**
* @brief Set LVD Detection
* @note Function ID:[DES_FLS_API_068]
* @param [in] IsEnable: True enable LVD Detection, false disable rtc module
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetLVDDetection(boolean IsEnable)
{
    /*cstat !MISRAC2012-Rule-10.5 !MISRAC2012-Rule-11.4*/
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LVDDETEN_Msk, FLASH_CNFG_LVDDETEN_Pos,
                 (uint32)IsEnable);
}

/**
* @brief Get Ecc Error Address Info
* @note Function ID:[DES_FLS_API_068]
* @return None
*/
LOCAL_INLINE uint32 Flash_Reg_ECCErrorAddr(void)
{
    return READ_REG32(FLASH->DFADR); /*cstat !MISRAC2012-Rule-10.5 !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set LVD Detection
* @note Function ID:[DES_FLS_API_068]
* @param [in] IsEnable: True enable LVD Detection, false disable rtc module
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetEMDFIE(boolean IsEnable)
{
    /*cstat !MISRAC2012-Rule-10.5 !MISRAC2012-Rule-11.4*/
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_EMDFIE_Msk, FLASH_CNFG_EMDFIE_Pos, (uint32)IsEnable);
}

#if defined (AC7842X) || defined (AC7843X)
/**
* @brief Get Swap status
* @note Function ID:[DES_FLS_API_068]
* @return None
*/
LOCAL_INLINE uint32 Flash_Reg_GetSwapStatus(void)
{
    /*cstat !MISRAC2012-Rule-10.5 !MISRAC2012-Rule-11.4*/
    return READ_REG32(FLASH->SWAPSTAT) & FLASH_SWAP_FLAG_Msk;
}
#endif

/*!
 * @brief Get the flash read protection status.
 *
 * @return 2b'11: Read protection is disabled
 *         2b'00/2b'01/2b'10: Read protection is enabled
 */
LOCAL_INLINE uint8 Flash_Reg_GetReadProtectStatus(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((FLASH->SEC & FLASH_SEC_RDPROT_Msk) >> FLASH_SEC_RDPROT_Pos);
}

/*!
 * @brief Get the backdoor key verify enable status.
 *
 * @return 2b'11: Backdoor key can not disable read protection
 *         2b'00/2b'01/2b'10: Backdoor key can enable read protection
 */
LOCAL_INLINE uint8 Flash_Reg_GetBackdoorKeyStatus(void)
{
    return (uint8)((FLASH->SEC & FLASH_SEC_BDKEN_Msk) >> FLASH_SEC_BDKEN_Pos);
}

/*!
 * @brief Clear Flash ECC 2-bit error address.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ClearECCErrorAddress(void)
{
    FLASH->DFADR = 0;
}

/*!
 * @brief Get collision error interrupt flag.
 *
 * @return Collision error interrupt flag
 */
LOCAL_INLINE uint8 Flash_Reg_GetCollIRQFlag(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_COLLERRF_Msk) >> FLASH_STAT_COLLERRF_Pos);
}

/*!
 * @brief Clear collision error interrupt flag.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ClearCollIRQFlag(void)
{
    FLASH->STAT |= FLASH_STAT_COLLERRF_Msk;
}

/*!
 * @brief force ECC 2-bit error detected, code should run in SRAM, usually used it for test.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ForceECCErrorDetect(void)
{
    FLASH->CNFG |= FLASH_CNFG_EMDFIE_Msk;
}

/*!
 * @brief Get flash CNFG register value.
 *
 * @return CNFG register value
 */
LOCAL_INLINE uint32 Flash_Reg_GetCnfg(void)
{
    return FLASH->CNFG;
}
/*cstat +MISRAC2012-Rule-10.5*/
#ifdef __cplusplus
}
#endif
#endif
/*============================================EOF==========================================*/
