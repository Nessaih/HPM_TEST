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
 * AutoChips Inc. (C) 2022. All rights reserved.
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
 * @file Eep_Hal.c
 * @brief This file provides EEP integration functions.
 *
 */

/* ===========================================  INCLUDE FILES  =========================================== */

#include "Device_Register.h"
#include "Eep_Hal.h"
#include "Fls_Hal.h"
#include "AC784xx_Fls_Reg.h"
#include "System_AC784xx.h"

#if !defined(CONFIG_EEP_DRV_ENABLE) || defined (AC7842X) || defined (AC7842E)  || defined (AC7843X)
/* ============================================  DEFINES AND MACROS  ============================================ */
/*!
 * @brief group size define rule:
 * 1. Total EEP size (Byte) = EEP SRAM size
 * 2. The min EEP_GROUP_SIZE = EEP_SIZE (word) * 2 * EepFlashMinProgByteNum
 * 3. EEP_FLASH_SIZE = EEP_GROUP_SIZE * Total group
 */

/** @brief EEP flash erased value */
#define EEP_FLASH_ERASED_VALUE          (0XFFFFFFFFU)

/** @brief EEP flash operate group flag */
#define EEP_GROUP_FLAG_VALUE            (0x00000000U)

/** @brief EEP block max number */
#define EEP_BLOCK_MAX_NUM               (8U)

/** @brief EEP group max number */
#define EEP_GROUP_MAX_NUM               (4U)

/** @brief EEP flash max program unit size */
#define EEP_FLASH_MAX_UNIT_SIZE         (16U)

/** @brief The EEP driver each RAM block base address. */
#define EEP_RAM_BLOCK_BASE(blk) (EepConfigPtr->EepRamBaseAddr + \
                                 ((uint32)(blk) * (uint32)(EepConfigPtr->EepBlockSize)))

/** @brief The EEP driver each RAM block end address. */
#define EEP_RAM_BLOCK_END(blk) (EepConfigPtr->EepRamBaseAddr + \
                                (((uint32)(blk) + 1U) * (uint32)(EepConfigPtr->EepBlockSize)))

/** @brief The EEP driver each group logic base address. */
#define EEP_GROUP_LOGIC_ADDR_START(blk) ((uint32)(blk) * (uint32)(EepConfigPtr->EepBlockSize))

/** @brief The EEP driver each group logic end address. */
#define EEP_GROUP_LOGIC_ADDR_END(blk) (((uint32)(blk) + 1U) * (uint32)(EepConfigPtr->EepBlockSize))

/** @brief The EEP ram address to logic address. */
#define EEP_RAM_ADDR_TO_LOGIC(addr) ((addr) - EepConfigPtr->EepRamBaseAddr)

/** @brief The EEP ram address to logic address. */
#define EEP_LOGIC_ADDR_TO_RAM(addr) ((addr) + EepConfigPtr->EepRamBaseAddr)

/** @brief The EEP ram address to logic address. */
#define EEP_LOGIC_ADDR_FROM_FLASH(addr) ((addr)& 0xfffU)

/** @brief The EEP driver each flash group base address. */
#define EEP_FLASH_GROUP_BASE(blk, grp) (EepConfigPtr->EepFlashBaseAddr + \
                                ((((uint32)(blk) * (uint32)(EepConfigPtr->EepGroupNum)) + (grp)) * EepFlashGroupSize))

/** @brief The EEP driver each flash group end address. */
#define EEP_FLASH_GROUP_END(blk, grp) (EepConfigPtr->EepFlashBaseAddr + \
                            ((((uint32)(blk) * (uint32)(EepConfigPtr->EepGroupNum)) + (grp) + 1U) * EepFlashGroupSize))

#define ALIGN_UP(val, align)            (((val) + (align) - 1U) & (~((align) - 1U)))
#define ALIGN_DOWN(val, align)          ((val) & (~(align) - 1U))

#define EEP_XOR_PATTERN                 0x5aa5U
/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */
/** @brief The EEP driver a flash group size in bytes. */
static uint32 EepFlashGroupSize = (uint32)EEP_FLASH_GROUP_SIZE;
/** @brief The EEP driver each flash program minimum size in bytes. */
static uint32 EepFlashMinProgByteNum = (uint32)DFLASH_WRITE_UNIT_SIZE;
/** @brief The EEP driver each flash program data size in bytes. */
static uint32 EepFlashDataByteNum = (uint32)(DFLASH_WRITE_UNIT_SIZE >> 1U);
/** @brief The EEP driver RAM block numbers. */
static uint16 EepBlockNum = EEP_BLOCK_MAX_NUM;
/** @brief The EEP driver current flash program address of each block. */
static uint32 CurFlashOperatAddr[EEP_BLOCK_MAX_NUM];
/** @brief The EEP driver current flash program block. */
static uint8 CurFlashGroup[EEP_BLOCK_MAX_NUM];
/** @brief The EEP driver last flash program block. */
static uint8 LastFlashGroup[EEP_BLOCK_MAX_NUM];
/** @brief The EEP driver configuration pointer. */
static const Eep_Hal_ConfigType *EepConfigPtr = NULL_PTR;
/** @brief The EEP driver status. */
static Eep_Hal_StatusType Eep_Status = EEP_HAL_UNINIT;
/** @brief Start address of the EEP flash area (inclusive) */
static uint32 EepFlashAddressStart;
/** @brief End address of the EEP flash area (exclusive) */
static uint32 EepFlashAddressEnd;

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* ======================================  Functions define  ======================================== */
/*!
 * @brief Fill memory with a specified value
 * @param[out] s Destination memory address
 * @param[in] c Value to fill
 * @param[in] count Number of bytes to fill
 * @note Function ID: DES_EEP_API_208
 */
static void Eep_Hal_Memset(uint8 *s, uint8 c, uint32 count)
{
    uint8 *ss = s;
    uint32 cnt = count;

    while (0x0U != cnt)
    {
        *ss = c;
        ss++;
        cnt--;
    }
}

/*!
 * @brief Copy memory between buffers
 * @param[out] dst Destination buffer address
 * @param[in] src Source buffer address
 * @param[in] count Number of bytes to copy
 * @note Function ID: DES_EEP_API_209
 */
static void Eep_Hal_Memcpy(uint8 *dst, const uint8 *src, uint32 count)
{
    const uint8 *s = src;
    uint8 *d = dst;
    uint32 cnt = count;

    /* Simple, byte oriented memcpy. */
    while (0x0U != cnt)
    {
        *d = *s;
        d++;
        s++;
        cnt--;
    }
}

#ifdef ATC_DEVICE_ASSERT
/*!
 * @brief Validate EEP storage configuration
 * @return Configuration validity status
 * @note Function ID: DES_EEP_API_210
 * @details Verifies:
 * - RAM/Flash address ranges when CSE partition is enabled
 * - Full memory region accessibility when CSE is disabled
 */
static Hal_StatusType Eep_Hal_InitCheck(void)
{
    Hal_StatusType Ret = STATUS_ERROR;
    uint32 RamBase = EepConfigPtr->EepRamBaseAddr;
    uint32 RamEnd = RamBase + EepConfigPtr->EepSize;
    uint32 FlashBase = EepConfigPtr->EepFlashBaseAddr;
    uint32 FlashEnd = FlashBase + (EepConfigPtr->EepGroupNum * EepFlashGroupSize * EepBlockNum);

    /* FlexRAM is used by CSE */
    if (0U != Flash_Reg_GetCSEPartition())
    {
        /* FlexRam 64KB and DFlash last 64KB are used by CSE, the remain partition could be used by EEP. */
        if ((RamBase >= SRAM_L_BASE) && (RamEnd <= (SRAM_U_END + 1U)) \
                && (FlashBase >= DFLASH_BASE) && (FlashEnd <= (DFLASH_BASE + 0x10000U)))
        {
            Ret = STATUS_SUCCESS;
        }
    }
    else
    {
        /* SRAM L/U and FlexRam, all DFlash could be used by EEP. */
#if defined (AC7843X)
        if (((RamBase >= SRAM_L_BASE) && (RamEnd <= (SRAM_U_END + 1U))) \
                && (FlashBase >= DFLASH_BASE) && (FlashEnd <= (DFLASH_END + 1U)))
#else
        if ((((RamBase >= SRAM_L_BASE) && (RamEnd <= (SRAM_U_END + 1U))) \
                || ((RamBase >= FLEXRAM_BASE) && (RamEnd <= (FLEXRAM_END + 1U)))) \
                && (FlashBase >= DFLASH_BASE) && (FlashEnd <= (DFLASH_END + 1U)))
#endif
        {
            Ret = STATUS_SUCCESS;
        }
    }

    return Ret;
}
#endif

/*!
 * @brief Erase Flash page with verification
 * @param[in] pageAddress Start address of Flash page to erase
 * @return Operation status (STATUS_SUCCESS/STATUS_ERROR)
 * @note Function ID: DES_EEP_API_207
 * @warning Must be executed within flash operation critical section
 * @remark Operation sequence:
 * 1. Verify page erase status
 * 2. Perform erase if verification fails
 * 3. Re-verify after erase operation
 */
static Hal_StatusType Eep_Hal_EraseFlash(uint32 pageAddress)
{
    Hal_StatusType status = STATUS_ERROR;

    if ((pageAddress >= EepFlashAddressStart) && (pageAddress <= (EepFlashAddressEnd - EepFlashGroupSize)))
    {
        const uint32 pagesPerGroup = EepFlashGroupSize / DFLASH_PAGE_SIZE;

        /* Erase and verify next group */
        for (uint32 pageIndex = 0; pageIndex < pagesPerGroup; pageIndex++)
        {
            const uint32 pageBase = pageAddress + (pageIndex * DFLASH_PAGE_SIZE);

            status = DFlash_Hal_SectionVerify(pageBase - DFLASH_BASE, DFLASH_PAGE_SIZE);
            if (STATUS_SUCCESS == status)
            {
                continue;
            }

            status = DFlash_Hal_PageErase(pageBase - DFLASH_BASE);
            if (STATUS_SUCCESS == status)
            {
                status = DFlash_Hal_SectionVerify(pageBase - DFLASH_BASE, DFLASH_PAGE_SIZE);
            }

            if (STATUS_SUCCESS != status)
            {
                break;
            }
        }
    }

    return status;
}

/*!
 * @brief Write data to Flash with EEP mapping
 * @param[in,out] FlashAddr Pointer to current Flash address (auto-incremented)
 * @param[in] Addr Virtual EEP address
 * @param[in] Size Data length in bytes
 * @param[in] Flush Operation mode:
 *        - 0: Normal write
 *        - 1: Force write all valid data
 * @param[in] updateFlag Write mode selector:
 *        - 0: Combined address+data write
 *        - 1: Direct data write
 * @return Operation status
 * @note Function ID: DES_EEP_API_211
 * @remark Supports two write modes:
 * - Combined mode: Pack 32-bit address + 32-bit data per Flash phrase
 * - Direct mode: Write raw data blocks
 */
static Hal_StatusType Eep_Hal_WriteDataToDflash(volatile uint32 *FlashAddr, uint32 Addr, uint16 Size,
                        uint32 Flush, uint32 updateFlag)
{
    Hal_StatusType Ret = STATUS_ERROR;
    uint32 CurAddr = Addr;
    uint16 CurSize = ALIGN_UP(Size, 4U);

    DEVICE_ASSERT((Addr % 4U) == 0U); //Alignment 4Byte
    if ((*FlashAddr >= EepFlashAddressStart) && ((*FlashAddr + CurSize) <= EepFlashAddressEnd))
    {
        if (updateFlag != 0U)
        {
            Ret = DFlash_Hal_PageWrite(*FlashAddr - DFLASH_BASE, CurSize,  (const uint8 *)CurAddr);
        }
        else
        {
            uint32 NewCombineData[EEP_FLASH_MAX_UNIT_SIZE] = {0};
            uint32 j = 0U;
            Ret = STATUS_SUCCESS;

            for (uint16 i = 0U; i < CurSize; i += 4U)
            {
                uint32 tmp;
                tmp = *(volatile uint32 *)CurAddr;
                if ((Flush == 0U) || ((Flush == 1U) && (tmp != EEP_FLASH_ERASED_VALUE)))
                {
#if defined (AC7842X) || defined (AC7842E)  || defined (AC7843X)
                    NewCombineData[j] = (EEP_RAM_ADDR_TO_LOGIC(CurAddr) << 16U) | (tmp >> 16U);
                    NewCombineData[j + 1U] = (tmp << 16U) | (EEP_RAM_ADDR_TO_LOGIC(CurAddr) ^ EEP_XOR_PATTERN);
#else
                    NewCombineData[j] = EEP_RAM_ADDR_TO_LOGIC(CurAddr);
                    NewCombineData[j + 1U] = tmp;
#endif
                    j += 2U;
                }
                CurAddr += 4U;

                if (j == EEP_FLASH_MAX_UNIT_SIZE)
                {
                    Ret = DFlash_Hal_PageWrite(*FlashAddr - DFLASH_BASE, j * 4U,  (const uint8 *)NewCombineData);
                    if (Ret != STATUS_SUCCESS)
                    {
                        break;
                    }
                    *FlashAddr += j * 4U;
                    j = 0U;
                }
            }

            if ((j != 0U) && (Ret == STATUS_SUCCESS))
            {
                Ret = DFlash_Hal_PageWrite(*FlashAddr - DFLASH_BASE, j * 4U,  (const uint8 *)NewCombineData);
                *FlashAddr += j * 4U;
            }
        }
    }

    return Ret;
}

/*!
 * @brief Retrieve group flag value
 * @param[in] Block Memory block index
 * @param[in] Group Flash group index
 * @param[in] high Flag portion selector:
 *        - 0: Lower 32-bit flag
 *        - 1: Upper 32-bit flag
 * @param[out] Flag A pointer used to store the read flag value.
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_212
 */
static Hal_StatusType Eep_Hal_GetGroupFlagVal(uint8 Block, uint8 Group, uint32 High, uint32 *Flag)
{
    uint32 FlashAddr = (EEP_FLASH_GROUP_BASE(Block, Group) + EepFlashGroupSize - EepFlashMinProgByteNum);

    FlashAddr = (High != 0U) ? (FlashAddr + EepFlashDataByteNum) : FlashAddr;
    return DFlash_Hal_Read((FlashAddr - DFLASH_BASE), sizeof(uint32), (uint8 *)Flag);
}

/*!
 * @brief Locate valid data start address
 * @param[in] Block Memory block index
 * @param[in] Group Flash group index
 * @param[in,out] GroupOperateAddr Valid data start address pointer
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_213
 * @remark Reverse search algorithm:
 * 1. Traverse from group end to start
 * 2. Determine boundary at first non-erased value
 */
static Hal_StatusType Eep_Hal_GetGroupValidDataStartAddr(uint8 Block, uint8 Group, volatile uint32 *GroupOperateAddr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    /* return group 0 start address if valid address was not found.*/
    *GroupOperateAddr = EEP_FLASH_GROUP_BASE(Block, Group);

    /* find valid address reverse.*/
    uint32 Addr = EEP_FLASH_GROUP_BASE(Block, Group) + EepFlashGroupSize - (2U * EepFlashMinProgByteNum);
    for (; Addr >= EEP_FLASH_GROUP_BASE(Block, Group); Addr -= EepFlashMinProgByteNum)
    {
        /* Check value was erased  */
        uint32 Data;
        Ret = DFlash_Hal_Read((Addr - DFLASH_BASE), sizeof(uint32), (uint8 *)&Data);
        if ((EEP_FLASH_ERASED_VALUE != Data) || (Ret != STATUS_SUCCESS))
        {
            if (Ret == STATUS_SUCCESS)
            {
                *GroupOperateAddr = Addr + EepFlashMinProgByteNum;
            }
            break;
        }
    }

    return Ret;
}

/*!
 * @brief Calculate valid flash group count and update pointers
 * @param[in] Block Memory block index
 * @param[out] ValidCnt a pointer used to store GroupValidCnt
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_214
 * @remark Validation criteria:
 * - FlagL != Erased value (0xFFFFFFFF)
 * - FlagH == Group flag value (0x00000000)
 */
static Hal_StatusType Eep_Hal_UpdateOperateAddr(uint8 Block, uint8 *ValidCnt)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    Hal_StatusType Ret1 = STATUS_SUCCESS;
    uint8 GroupValidCnt = 0U;
    uint8 FirstValidIndex = 0U;
    uint8 FlashGroupCur = 0U;
    uint8 FlashGroupLast = 0U;
    uint32 FlagL, FlagH;

    for (uint8 i = 0U; i < EepConfigPtr->EepGroupNum; i++)
    {
        Ret = Eep_Hal_GetGroupFlagVal(Block, i, 0, &FlagL);
        Ret1 = Eep_Hal_GetGroupFlagVal(Block, i, 1, &FlagH);

        if ((Ret != STATUS_SUCCESS) || (Ret1 != STATUS_SUCCESS))
        {
            Ret = (Ret != STATUS_SUCCESS) ? Ret : Ret1;
            break;
        }

        if ((EEP_FLASH_ERASED_VALUE != FlagL) &&
            (EEP_GROUP_FLAG_VALUE == FlagH))
        {
            GroupValidCnt++;

            if (GroupValidCnt == 1U) /* Normal case */
            {
                FlashGroupCur = i;
                FlashGroupLast = i;
                FirstValidIndex = i;
            }
            else if (GroupValidCnt == 2U)
            {
                if ((i == (EEP_GROUP_MAX_NUM - 1U)) &&
                    (FirstValidIndex == 0U))
                {
                    FlashGroupLast = i;
                }
                else
                {
                    FlashGroupCur = i;
                }
            }
            else /* GroupValidCnt > 2 */
            {
                /* report error */
                FlashGroupCur  = 0U;
                FlashGroupLast = 0U;
            }
        }
    }

    CurFlashGroup[Block] = FlashGroupCur;
    LastFlashGroup[Block] = FlashGroupLast;
    *ValidCnt = GroupValidCnt;

    return Ret;
}

/*!
 * @brief Load flash data to RAM mirror
 * @param[in] Block Target RAM block index
 * @param[in] Group Source flash group index
 * @param[in] OperateAddr Flash group operation boundary address
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_215
 * @remark Performs:
 * 1. Iterates through flash group addresses
 * 2. Validates address mapping between flash and RAM
 * 3. Copies valid data entries to RAM
 * @details For each flash entry:
 * - Checks 4-byte alignment validity
 * - Verifies RAM address range
 * - Executes direct memory copy when conditions met
 */
static Hal_StatusType Eep_Hal_LoadDataToRam(uint8 Block, uint8 Group, uint32 OperateAddr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    /*PRQA S 0771 ++*/
    for (uint32 Addr = EEP_FLASH_GROUP_BASE(Block, Group); Addr < OperateAddr; Addr += EepFlashMinProgByteNum)
    {
#if defined (AC7842X) || defined (AC7842E)  || defined (AC7843X)
        uint32 Temp1, Temp2, Data;
        uint32 LogicAddr;
        uint32 LogicAddrCheck;

        Ret = DFlash_Hal_Read((Addr - DFLASH_BASE), sizeof(uint32), (uint8 *)&Temp1);
        if (Ret == STATUS_SUCCESS)
        {
            Ret = DFlash_Hal_Read((Addr + EepFlashDataByteNum - DFLASH_BASE), sizeof(uint32), (uint8 *)&Temp2);
        }

        if (Ret != STATUS_SUCCESS)
        {
            break;
        }

        LogicAddr = Temp1 >> 16U;
        Data = (Temp1 << 16U)|(Temp2 >> 16U);
        LogicAddrCheck = Temp2 & 0xffffU;

        if ((LogicAddr >= EEP_GROUP_LOGIC_ADDR_START(Block)) &&
            (LogicAddr < EEP_GROUP_LOGIC_ADDR_END(Block)) && ((LogicAddr % 4U) == 0U) &&
            (LogicAddrCheck == (LogicAddr ^ EEP_XOR_PATTERN)))
        {
            *(volatile uint32 *)(EEP_LOGIC_ADDR_TO_RAM(LogicAddr)) = Data;
        }
        else
        {
            continue;
        }
#else
        uint32 Temp, Data;

        Ret = DFlash_Hal_Read((Addr - DFLASH_BASE), sizeof(uint32), (uint8 *)&Temp);
        Temp = EEP_LOGIC_ADDR_FROM_FLASH(Temp);
        if (Ret != STATUS_SUCCESS)
        {
            break;
        }

        /* Check the temp address is valid or not, maybe the temp is erased && addr 4byte aligned */
        /*PRQA S 1840 ++*/
        if ((Temp >= EEP_GROUP_LOGIC_ADDR_START(Block)) &&
            (Temp < EEP_GROUP_LOGIC_ADDR_END(Block)) && ((Temp % 4) == 0U))
        /*PRQA S 1840 --*/
        {
            Ret = DFlash_Hal_Read((Addr + EepFlashDataByteNum - DFLASH_BASE), sizeof(uint32), (uint8 *)&Data);
            if (Ret != STATUS_SUCCESS)
            {
                break;
            }
            *(volatile uint32 *)(EEP_LOGIC_ADDR_TO_RAM(Temp)) = Data;
        }
        else
        {
            continue;
        }
#endif
    }
    /*PRQA S 0771 --*/
    return Ret;
}

/*!
 * @brief Initialize RAM with flash data
 * @param[in] Block Memory block index
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_216
 * @remark Operation sequence:
 * 1. Clear RAM block with 0xFF
 * 2. Load data from both current and last groups
 * 3. Update operation address pointer
 */
static Hal_StatusType Eep_Hal_SortRamData(uint8 Block)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint8 CurFlashGroupIndex = CurFlashGroup[Block];
    uint8 LastFlashGroupIndex = LastFlashGroup[Block];

    Eep_Hal_Memset((uint8 *)(EEP_RAM_BLOCK_BASE(Block)), 0xFFU, EepConfigPtr->EepBlockSize);

    if (CurFlashGroupIndex != LastFlashGroupIndex)
    {
        uint32 LastGroupOperateAddr = 0U;

        Ret = Eep_Hal_GetGroupValidDataStartAddr(Block, LastFlashGroupIndex, &LastGroupOperateAddr);
        if (Ret == STATUS_SUCCESS)
        {
            Ret = Eep_Hal_LoadDataToRam(Block, LastFlashGroupIndex, LastGroupOperateAddr);
        }
    }

    if (Ret == STATUS_SUCCESS)
    {
        Ret = Eep_Hal_GetGroupValidDataStartAddr(Block, CurFlashGroupIndex, &CurFlashOperatAddr[Block]);
        if (Ret == STATUS_SUCCESS)
        {
            Ret = Eep_Hal_LoadDataToRam(Block, CurFlashGroupIndex, CurFlashOperatAddr[Block]);
        }
    }

    return Ret;
}

/*!
 * @brief Update EEP group indexes and synchronize data
 * @param[in] Block Memory block index
 * @return Operation status
 * @note Function ID: DES_EEP_API_217
 * @remark Handles three scenarios:
 * - First initialization of group flags
 * - Normal single group operation
 * - Group rotation with data migration
 */
static Hal_StatusType Eep_Hal_UpdateAll(uint8 Block)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint8 GroupValidCnt;

    Ret = Eep_Hal_UpdateOperateAddr(Block, &GroupValidCnt);
    if (Ret == STATUS_SUCCESS)
    {
        Ret = Eep_Hal_SortRamData(Block);
    }

    if (Ret == STATUS_SUCCESS)
    {
        if (GroupValidCnt == 0U)/* First used eep write operate flag in the first group */
        {
            const uint32 Flag[2] = {0U, 0U};
            volatile uint32 operateFlagAddr = (EEP_FLASH_GROUP_BASE(Block,CurFlashGroup[Block]) +
                                        EepFlashGroupSize - EepFlashMinProgByteNum);
            /*PRQA S 0309 ++*/
            Ret = Eep_Hal_WriteDataToDflash(&operateFlagAddr, (uint32)Flag, (uint16)EepFlashMinProgByteNum, 0, 1);
            /*PRQA S 0309 --*/
        }
        else if (GroupValidCnt == 1U) /* Normal case */
        {
            Ret = STATUS_SUCCESS;
        }
        else if (GroupValidCnt == 2U)
        {
            /* indicate copy fail last time from the previous group to the next group
            * then it should copy again and erase the previous group, GroupValidCnt=2 so index never = 1 at least
            */
            uint32 CurAddr;
            uint16 CurSize;

            /* at start, restore from start to Block end */
            if (EEP_FLASH_GROUP_BASE(Block, CurFlashGroup[Block]) == CurFlashOperatAddr[Block])
            {
                CurAddr = EEP_RAM_BLOCK_BASE(Block);
                CurSize = EepConfigPtr->EepBlockSize;
            }
            else /* at middle, restore from the valid ram address to Block end */
            {
                uint32 VaildFlashOperatAddr = CurFlashOperatAddr[Block] - EepFlashMinProgByteNum;

                CurAddr = EEP_RAM_BLOCK_BASE(Block);
                /* find vaild ram address in current group */
                /*PRQA S 0771 ++*/
                for (; VaildFlashOperatAddr >= EEP_FLASH_GROUP_BASE(Block, CurFlashGroup[Block]);
                        VaildFlashOperatAddr -= EepFlashMinProgByteNum)
                {
#if defined (AC7842X) || defined (AC7842E)  || defined (AC7843X)
                    uint32 Temp1, Temp2;
                    uint32 LogicAddr;
                    uint32 LogicAddrCheck;

                    Ret = DFlash_Hal_Read((VaildFlashOperatAddr - DFLASH_BASE), sizeof(uint32), (uint8 *)&Temp1);
                    if (Ret == STATUS_SUCCESS)
                    {
                        Ret = DFlash_Hal_Read((VaildFlashOperatAddr + EepFlashDataByteNum - DFLASH_BASE),
                                               sizeof(uint32), (uint8 *)&Temp2);
                    }

                    if (Ret != STATUS_SUCCESS)
                    {
                        break;
                    }

                    LogicAddr = Temp1 >> 16U;
                    LogicAddrCheck = Temp2 & 0xffffU;

                    if ((LogicAddr >= EEP_GROUP_LOGIC_ADDR_START(Block)) &&
                        (LogicAddr < EEP_GROUP_LOGIC_ADDR_END(Block)) && ((LogicAddr % 4U) == 0U) &&
                        (LogicAddrCheck == (LogicAddr ^ EEP_XOR_PATTERN)))
                    {
                        CurAddr += EepFlashDataByteNum;
                        break;
                    }
#else
                    Ret = DFlash_Hal_Read((VaildFlashOperatAddr - DFLASH_BASE), sizeof(uint32), (uint8 *)&CurAddr);

                    CurAddr = EEP_LOGIC_ADDR_TO_RAM(EEP_LOGIC_ADDR_FROM_FLASH(CurAddr));
                    if(Ret != STATUS_SUCCESS)
                    {
                        break;
                    }
                    if ((CurAddr >= EEP_RAM_BLOCK_BASE(Block)) &&
                        /*PRQA S 1840 ++*/
                        (CurAddr < EEP_RAM_BLOCK_END(Block)) && ((CurAddr % 4) == 0U))
                        /*PRQA S 1840 --*/
                    {
                        CurAddr += EepFlashDataByteNum;
                        break;
                    }
#endif
                }
                /*PRQA S 0771 --*/
                if (VaildFlashOperatAddr < EEP_FLASH_GROUP_BASE(Block, CurFlashGroup[Block]))
                {
                    CurAddr = EEP_RAM_BLOCK_BASE(Block);
                }

                CurSize = (uint16)(EEP_RAM_BLOCK_END(Block) - CurAddr);
            }

            if (Ret == STATUS_SUCCESS)
            {
                Ret = Eep_Hal_WriteDataToDflash(&CurFlashOperatAddr[Block], CurAddr, CurSize, 1, 0);
                if (STATUS_SUCCESS == Ret)
                {
                    Ret = Eep_Hal_EraseFlash(EEP_FLASH_GROUP_BASE(Block, LastFlashGroup[Block]));
                }
            }
        }
        else /*  Three or more Valid groups are not supported */
        {
            Ret = STATUS_UNSUPPORTED;
        }
    }

    return Ret;
}

/*!
 * @brief Atomic write operation unit
 * @param[in] Block Memory block index
 * @param[in] Addr Starting address in EEP space
 * @param[in] Data Data buffer pointer
 * @param[in] Size Data length in bytes
 * @return Operation status
 * @note Function ID: DES_EEP_API_218
 * @remark Key operations:
 * 1. Address alignment handling
 * 2. Flash space availability check
 * 3. Group switching when space exhausted
 */
static Hal_StatusType Eep_Hal_WriteUnit(uint8 Block, uint32 Addr, const uint8 *Data, uint16 Size)
{
    Hal_StatusType Ret;
    uint32 CurAddr = Addr;
    uint16 CurSize = Size;

    DEVICE_ASSERT((0U != EepFlashMinProgByteNum) && (0U != EepFlashDataByteNum));

    /* Update to RAM */
    Eep_Hal_Memcpy((uint8 *)CurAddr, Data, Size);

    /* Update to Dflash */
    uint8 CurFlashGroupIndex = CurFlashGroup[Block];
    uint16 AdditionalSize;
    uint16 LeftSize;

    AdditionalSize = (uint16)(CurAddr % EepFlashDataByteNum); /* Align down by EepFlashDataByteNum */
    if (AdditionalSize != 0U)
    {
        CurAddr -= AdditionalSize;
        CurSize += AdditionalSize;
    }

    LeftSize = (uint16)(EEP_FLASH_GROUP_BASE(Block, CurFlashGroupIndex) + EepFlashGroupSize
                        - CurFlashOperatAddr[Block] - EepFlashMinProgByteNum) / 2U;

    /* check available space is enough, every word combine with write address will use 8 FLASH address */
    if (LeftSize >= CurSize) /* enough */
    {
        Ret = Eep_Hal_WriteDataToDflash(&CurFlashOperatAddr[Block], CurAddr, CurSize, 0, 0);
    }
    else /* not enough */
    {
        /* switch group */
        uint8 NextFlsGroup = (uint8)(((uint32)CurFlashGroupIndex + 1U) % (EepConfigPtr->EepGroupNum));

        if (NULL_PTR != EepConfigPtr->Callback)
        {
            EepConfigPtr->Callback();
        }

        /* Erase and verify next group */
        Ret = Eep_Hal_EraseFlash(EEP_FLASH_GROUP_BASE(Block, NextFlsGroup));
        if (STATUS_SUCCESS == Ret)
        {
            /* write group operate flag (64-bit 0) to the next group*/
            uint32 Flag[2] = {0U, 0U};
            Ret  = Eep_Hal_GetGroupFlagVal(Block, CurFlashGroupIndex, 0, &Flag[0]);
            if (STATUS_SUCCESS == Ret)
            {
                 Flag[0] += 1U;
            }
            volatile uint32 operateFlagAddr = EEP_FLASH_GROUP_BASE(Block, NextFlsGroup) +
                                        EepFlashGroupSize - EepFlashMinProgByteNum;
                /*PRQA S 0309 ++*/
            Ret = Eep_Hal_WriteDataToDflash(&operateFlagAddr, (uint32)Flag, (uint16)EepFlashMinProgByteNum, 0, 1);
                /*PRQA S 0309 --*/

            /* write data to the next group */
            if (STATUS_SUCCESS == Ret)
            {
                /* write all the SRAM data to the next group, NEED optimization */
                CurFlashOperatAddr[Block] = EEP_FLASH_GROUP_BASE(Block, NextFlsGroup);
                CurAddr = EEP_RAM_BLOCK_BASE(Block);

                Ret = Eep_Hal_WriteDataToDflash(&CurFlashOperatAddr[Block], CurAddr,
                                                (uint16)EepConfigPtr->EepBlockSize, 1, 0);
                /* update pointers and erase the previous group */
                if (STATUS_SUCCESS == Ret)
                {
                    LastFlashGroup[Block] = CurFlashGroupIndex;
                    CurFlashGroup[Block] = NextFlsGroup;

                    /* erase the previous group */
                    Ret = Eep_Hal_EraseFlash(EEP_FLASH_GROUP_BASE(Block, CurFlashGroupIndex));
                }
            }
        }
    }

    return Ret;
}

/*!
 * @brief Main EEP write interface
 * @param[in] Addr Virtual EEP address (offset within configured space)
 * @param[in] Data Pointer to source data buffer
 * @param[in] Size Data length in bytes
 * @return Operation status (STATUS_SUCCESS/STATUS_ERROR)
 * @note Function ID: DES_EEP_API_202
 * @warning Address range must be within configured EEP size
 * @remark Implements:
 * 1. Address validation
 * 2. Block-wise write operations
 * 3. Status tracking
 */
Hal_StatusType Eep_Hal_Write(uint32 Addr, const uint8 *Data, uint16 Size)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint32 CurAddr;
    uint16 CurSize = Size;
    const uint8 *CurData = (const uint8 *)Data;
    uint8 Block;

    DEVICE_ASSERT(NULL_PTR != EepConfigPtr);
    DEVICE_ASSERT(NULL_PTR != Data);
    DEVICE_ASSERT(EepConfigPtr->EepSize >= (Addr + Size));
    DEVICE_ASSERT(0U != EepConfigPtr->EepBlockSize);

    Ret = (NULL_PTR == EepConfigPtr) ? STATUS_ERROR :
          ((EepConfigPtr->EepSize < (Addr + Size)) ? STATUS_ERROR :
           ((EepConfigPtr->EepBlockSize == 0U) ? STATUS_ERROR : STATUS_SUCCESS));
    Ret = (NULL_PTR == Data) ? STATUS_ERROR : Ret;

    if (Ret == STATUS_SUCCESS)
    {
        /*PRQA S 2985 ++ # considered an invalid operation, it is actually meaningful.*/
        CurAddr = Addr + EEP_RAM_BLOCK_BASE(0);
        /*PRQA S 2985 -- # considered an invalid operation, it is actually meaningful.*/
        Block = (uint8)(Addr / EepConfigPtr->EepBlockSize);

        Eep_Status = EEP_HAL_BUSY;

        /* not cross RAM units. */
        for (uint16 RemainSize = Size; RemainSize > 0U; RemainSize -= CurSize)
        {
            CurSize = (uint16)(EEP_RAM_BLOCK_END(Block) - CurAddr);
            if (CurSize > RemainSize)
            {
                CurSize = RemainSize;
            }

            Ret = Eep_Hal_WriteUnit(Block, CurAddr, CurData, CurSize);
            if (Ret != STATUS_SUCCESS)
            {
                break;
            }
            /* PRQA S 0488 ++*/
            CurAddr += CurSize;
            CurData += CurSize;
            /* PRQA S 0488 --*/
            Block++;
        }

        if (Ret == STATUS_SUCCESS)
        {
            Eep_Status = EEP_HAL_IDLE;
        }
        else
        {
            Eep_Status = EEP_HAL_TIMEOUT;
        }
    }

    return Ret;
}

/*!
 * @brief Main EEP read interface
 * @param[in] Addr Virtual EEP address
 * @param[out] Data Data buffer pointer
 * @param[in] Size Data length in bytes
 * @return Operation status
 * @note Function ID: DES_EEP_API_201
 * @remark Direct memory read from RAM mirror
 */
Hal_StatusType Eep_Hal_Read(uint32 Addr, uint8 *Data, uint16 Size)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    DEVICE_ASSERT(NULL_PTR != EepConfigPtr);
    DEVICE_ASSERT(NULL_PTR != Data);
    DEVICE_ASSERT(EepConfigPtr->EepSize >= (Addr + Size));

    Ret = (NULL_PTR == EepConfigPtr) ? STATUS_ERROR :
          ((EepConfigPtr->EepSize < (Addr + Size)) ? STATUS_ERROR :
           ((NULL_PTR == Data) ? STATUS_ERROR : STATUS_SUCCESS));

    if (Ret == STATUS_SUCCESS)
    {
        /*PRQA S 2985 ++ # considered an invalid operation, it is actually meaningful.*/
        Eep_Hal_Memcpy(Data, (const uint8 *)(EEP_RAM_BLOCK_BASE(0) + Addr), Size);
        /*PRQA S 2985 -- # considered an invalid operation, it is actually meaningful.*/
    }

    return Ret;
}

/*!
 * @brief Internal EEP data refresh mechanism
 * @return Operation status (STATUS_SUCCESS/STATUS_ERROR)
 * @note Function ID: DES_EEP_API_219
 * @remark Performs multi-block refresh by:
 * 1. Iterating through all configured memory blocks
 * 2. Updating group indexes and data synchronization
 * 3. Maintaining data consistency across flash and RAM
 */
static Hal_StatusType Eep_Hal_InternalRefresh(void)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    DEVICE_ASSERT(NULL_PTR != EepConfigPtr);

    for (uint8 Block = 0U; Block < EepBlockNum; Block++)
    {
        Ret = Eep_Hal_UpdateAll(Block);
        if (Ret != STATUS_SUCCESS)
        {
            break;
        }
    }

    return Ret;
}

/*!
 * @brief Erase entire EEP storage space
 * @return Operation status (STATUS_SUCCESS/STATUS_ERROR)
 * @note Function ID: DES_EEP_API_203
 * @warning Requires EEP_SDK_NON_EXTENDED_API undefined
 * @remark Performs:
 * 1. Flash sector erase
 * 2. RAM mirror refresh
 * 3. Status management
 */
Hal_StatusType Eep_Hal_Erase(void)
{
    Hal_StatusType Ret;

    DEVICE_ASSERT(NULL_PTR != EepConfigPtr);
    DEVICE_ASSERT(0x0U != EepConfigPtr->EepBlockSize);

    Ret = (NULL_PTR == EepConfigPtr) ? STATUS_ERROR :
          ((EepConfigPtr->EepBlockSize == 0U) ? STATUS_ERROR : STATUS_SUCCESS);

    if (STATUS_SUCCESS == Ret)
    {
        Eep_Status = EEP_HAL_BUSY;

        /* erase dflash */
        for (uint16 i = 0U; i < (EepConfigPtr->EepGroupNum * EepBlockNum); i++)
        {
            Ret = Eep_Hal_EraseFlash(EepConfigPtr->EepFlashBaseAddr + (i * EepFlashGroupSize));
            if (Ret != STATUS_SUCCESS)
            {
                break;
            }
        }
    }

    /* erase RAM */
    if (STATUS_SUCCESS == Ret)
    {
        Ret = Eep_Hal_InternalRefresh();
    }

    if (Ret == STATUS_SUCCESS)
    {
        Eep_Status = EEP_HAL_IDLE;
    }
    else
    {
        Eep_Status = EEP_HAL_TIMEOUT;
    }

    return Ret;
}

/*!
 * @brief Initialize EEP driver
 * @param[in] ConfigPtr Pointer to configuration structure
 * @return Initialization status
 * @note Function ID: DES_EEP_API_200
 * @warning Configuration must persist after initialization
 * @remark Performs:
 * 1. Parameter validation
 * 2. Memory mapping setup
 * 3. Initial data synchronization
 */
Hal_StatusType Eep_Hal_Init(const Eep_Hal_ConfigType *ConfigPtr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    /* Save EEP driver configuration. */
    EepConfigPtr = ConfigPtr;

    /* PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    DEVICE_ASSERT((ConfigPtr->EepSize % EEP_MIN_SIZE) == 0U);
    DEVICE_ASSERT(ConfigPtr->EepSize <= EEP_MAX_SIZE);

    DEVICE_ASSERT(ConfigPtr->EepBlockSize > 0U);
    DEVICE_ASSERT(ConfigPtr->EepBlockSize < (EepFlashGroupSize / 2U));
    DEVICE_ASSERT((ConfigPtr->EepBlockSize % EepFlashDataByteNum) == 0U);

    DEVICE_ASSERT(ConfigPtr->EepGroupNum > 1U);
    DEVICE_ASSERT(ConfigPtr->EepGroupNum <= EEP_GROUP_MAX_NUM);

    DEVICE_ASSERT((EepFlashGroupSize % DFLASH_PAGE_SIZE) == 0U);
#ifdef ATC_DEVICE_ASSERT
    DEVICE_ASSERT(STATUS_SUCCESS == Eep_Hal_InitCheck());
#endif

    /* PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/

    if (ConfigPtr == NULL_PTR)
    {
        Ret = STATUS_ERROR;
    }
    else
    {
        Ret = ((ConfigPtr->EepSize % EEP_MIN_SIZE) != 0U) ? STATUS_ERROR :
              ((ConfigPtr->EepSize > EEP_MAX_SIZE) ? STATUS_ERROR : Ret);
        Ret = (ConfigPtr->EepBlockSize >= (EepFlashGroupSize / 2U)) ? STATUS_ERROR :
              (((ConfigPtr->EepBlockSize % EepFlashDataByteNum) != 0U) ? STATUS_ERROR : Ret);
        Ret = (ConfigPtr->EepGroupNum < 1U) ? STATUS_ERROR :
              ((ConfigPtr->EepGroupNum > EEP_GROUP_MAX_NUM) ? STATUS_ERROR : Ret);

        /*PRQA S 2832 ++ # the upper layer call guarantees that devision by zero will never appear.*/
        EepBlockNum = (ConfigPtr->EepSize + ConfigPtr->EepBlockSize - 1U) / (ConfigPtr->EepBlockSize);
        /*PRQA S 2832 -- # the upper layer call guarantees that devision by zero will never appear.*/
        DEVICE_ASSERT(EepBlockNum <= EEP_BLOCK_MAX_NUM);

        Ret = (EepBlockNum > EEP_BLOCK_MAX_NUM) ? STATUS_ERROR :
              (((EepFlashGroupSize % DFLASH_PAGE_SIZE) != 0U) ? STATUS_ERROR : Ret);

        EepFlashAddressStart = EepConfigPtr->EepFlashBaseAddr;
        EepFlashAddressEnd = EepConfigPtr->EepFlashBaseAddr +
                 (uint32)(EepBlockNum * EepFlashGroupSize * EepConfigPtr->EepGroupNum);
    }

    if (Ret == STATUS_SUCCESS)
    {
        Ret = Eep_Hal_InternalRefresh();

        Eep_Status = (Ret == STATUS_SUCCESS) ? EEP_HAL_INITIALIZED : EEP_HAL_UNINIT;
    }

    return Ret;
}

/*!
 * @brief Refresh EEP data from flash
 * @return Operation status
 * @note Function ID: DES_EEP_API_205
 * @remark Internal mechanism for:
 * 1. Data consistency recovery
 * 2. Post-erase reinitialization
 * 3. Multi-block synchronization
 */
#ifndef EEP_SDK_NON_EXTENDED_API
Hal_StatusType Eep_Hal_Refresh(void)
{
    return Eep_Hal_InternalRefresh();
}
#endif

/*!
 * @brief Retrieve EEP driver status
 * @return Current driver state
 * @note Function ID: DES_EEP_API_204
 * @retval EEP_HAL_UNINIT Driver not initialized
 * @retval EEP_HAL_INITIALIZED Initialized but inactive
 * @retval EEP_HAL_BUSY Write operation in progress
 * @retval EEP_HAL_IDLE Ready for operations
 */
/*PRQA S 1503 ++*/
Eep_Hal_StatusType Eep_Hal_GetStatus(void)
{
    return Eep_Status;
}
/*PRQA S 1503 --*/

/*!
 * @brief Release the acquired resource when Eep_Hal is aborted abnormally.
 * @note Function ID: DES_EEP_API_207
 */
void Eep_Hal_AbortFreeRes(void)
{
/*PRQA S 3200 ++*/
    System_FlsDeviceFreelock(FLS_DEV);
/*PRQA S 3200--*/
}

/*!
 * @brief Get driver version information
 * @param[out] Version Pointer to version structure
 * @note Function ID: DES_EEP_API_206
 * @remark Populates:
 * - Major version
 * - Minor version
 * - Patch version
 */
void EEP_Hal_GetVersion(Eep_Hal_VersionType *Version)
{
    if (Version != NULL_PTR)
    {
        Version->Major = EEP_HAL_SW_MAJOR_VERSION;
        Version->Minor = EEP_HAL_SW_MINOR_VERSION;
        Version->Patch = EEP_HAL_SW_PATCH_VERSION;
    }
}

#endif
/* =============================================  EOF  ============================================== */
