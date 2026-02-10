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
 * @file Mpu_Hal.c
 *
 * @brief This file provides Hal Mpu api.
 *
 */

/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Mpu_Reg.h"
#include "Mpu_Hal.h"
/*===============================================FILE VERION========================================*/

/*==============================================LOCAL MACROS========================================*/
#define  MPU_MASTER_PID_ENABLE_MASK  0xF0000000UL
#define  MPU_PERMISSION_MASK         0xFFFFFFFUL
/*=============================================LOCAL TYPEDEFS=======================================*/

/*=============================================LOCAL CONSTANTS======================================*/

/*=============================================LOCAL VARIABLES======================================*/

/*============================================GLOBAL CONSTANTS======================================*/

/*============================================GLOBAL VARIABLES======================================*/

/*=======================================LOCAL FUNCTION PROTOTYPES==================================*/

/*============================================LOCAL FUNCTIONS=======================================*/

/*============================================GLOBAL FUNCTIONS======================================*/
/**
 * @brief Mpu module init.
 * @note Function ID: DES_MCL_API_401
 * @param[in] ConfigPtr: mpu configuration
 * @return void
 */
void Mpu_Hal_Init(const Mpu_ConfigType *ConfigPtr)
{
    Mpu_RegionIdType RegionId;
    uint8 MpuIdx;
    uint8 RegionIdx;
    uint8 MasterIdx;
    const Mpu_ModuleConfigType *ModuleConfig; /*!< mpu module configure pointer */
    const Mpu_RegionConfigType *RegionConfig;
    const Mpu_RegionParamsType *RegionParams; /*!< region attributes pointer */
    Mpu_IdType MpuId; /*!< Mpu module id */

    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    /* PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    DEVICE_ASSERT(ConfigPtr->ModuleConfigCnt <= (uint8)MPU_ID_MAX);
    /* PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/

    /* init all mpu */
    for (MpuIdx = 0U; MpuIdx < ConfigPtr->ModuleConfigCnt; MpuIdx++)
    {
        ModuleConfig = &ConfigPtr->ModuleConfig[MpuIdx];
        if (ModuleConfig != NULL_PTR)
        {
            /* PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
            MpuId = ModuleConfig->MpuId ;
            /* PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
            /*mpu disable*/
            Mpu_Reg_Disable((uint8)MpuId);
            /*modify rgd0 word2 to disable region 0,for using mpu*/
            Mpu_Reg_SetRegionACCAttrs((uint8)MpuId, (uint8)MPU_REGION_ID_0, 0U);
#if MPU_HAS_PROCESS_IDENTIFIER
            for (MasterIdx = 0U; MasterIdx < (uint8)MPU_MASTER_MAX; MasterIdx++)
            {
                /* only dma mstetr not set pid */
                if ((uint8)MPU_MASTER_DMA != MasterIdx)
                {
                    Mpu_Reg_SetMasterPid((uint8)MpuId, MasterIdx, ModuleConfig->MasterPid[MasterIdx]);
                }
            }
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
            /* init all region */
            for (RegionIdx = 0U; RegionIdx < ModuleConfig->RegionConfigCnt; RegionIdx++)
            {
                RegionConfig = &ModuleConfig->RegionConfig[RegionIdx];
                RegionId = RegionConfig->RegionId;
                RegionParams = &RegionConfig->RegionParams;
                Mpu_Reg_SetRegionStartAddr((uint8)MpuId, (uint8)RegionId, RegionParams->Addr.StartAddr);
                Mpu_Reg_SetRegionEndAddr((uint8)MpuId, (uint8)RegionId, RegionParams->Addr.EndAddr);
#if MPU_HAS_PROCESS_IDENTIFIER
                Mpu_Reg_SetRegionPid((uint8)MpuId, (uint8)RegionId, (uint8)RegionParams->Attrs.Pid, \
                                     (uint8)RegionParams->Attrs.PidMask);
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
                Mpu_Reg_SetRegionACCAttrs((uint8)MpuId, (uint8)RegionId, \
                                          RegionParams->Attrs.Permission | RegionParams->Attrs.MasterPidEnable);
                Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, TRUE);
            }
            /*mpu enable*/
            Mpu_Reg_Enable((uint8)MpuId);
        }
        else
        {
            /* nothing */
        }
    }

}

/**
 * @brief Mpu module deinit.
 * @note  Function ID:DES_MCL_API_402
 * @return void
 */
void Mpu_Hal_Deinit(void)
{
    uint8 MpuIdx;
    uint8 RegionIdx;

    /* deinit all mpu */
    /* PRQA S 2877 ++ # the loop will be executed multiple times in other devices.*/
    for (MpuIdx = 0U; MpuIdx < MPU_INSTANCE_MAX; MpuIdx++)
    {
        Mpu_Reg_Disable(MpuIdx);
        /* deinit all region */
        for (RegionIdx = 1U; RegionIdx < (uint8)MPU_REGION_ID_MAX; RegionIdx++)
        {
            Mpu_Reg_EnableRegion(MpuIdx, RegionIdx, FALSE);
        }
    }
    /* PRQA S 2877 -- # the loop will be executed multiple times in other devices.*/
}

/**
 * @brief set region address configuration.
 * @note Function ID: DES_MCL_API_403
 * @param[in] MpuId: mpu id
 * @param[in] RegionId: region id
 * @param[in] AddrConfigPtr: region address configuration pointer
 * @return void
 */
void Mpu_Hal_SetRegionAddr(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, const Mpu_RegionAddrConfigType *AddrConfigPtr)
{
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(RegionId < MPU_REGION_ID_MAX);
    DEVICE_ASSERT(AddrConfigPtr != NULL_PTR);
    Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, FALSE);
    /* PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    Mpu_Reg_SetRegionStartAddr((uint8)MpuId, (uint8)RegionId, AddrConfigPtr->StartAddr);
    /* PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
    Mpu_Reg_SetRegionEndAddr((uint8)MpuId, (uint8)RegionId, AddrConfigPtr->EndAddr);
    Mpu_Reg_EnableRegion((uint8)MpuId, (uint8) RegionId, TRUE);
}

/**
 * @brief get region info.
 * @note Function ID:DES_MCL_API_404
 * @param[in] MpuId: mpu id
 * @param[in] RegionId: region id
 * @param[out] RegionParamsPtr: region params pointer
 * @return void
 */
void Mpu_Hal_GetRegionInfo(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, Mpu_RegionParamsType *RegionParamsPtr)
{
#if MPU_HAS_PROCESS_IDENTIFIER
    uint32 RegionPid;
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
    uint32 RegionAttrs;
    DEVICE_ASSERT(RegionParamsPtr != NULL_PTR);
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(RegionId < MPU_REGION_ID_MAX);

    /* PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    RegionParamsPtr->Addr.StartAddr = Mpu_Reg_GetRegionStartAddr((uint8)MpuId, (uint8)RegionId);
    /* PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
    RegionParamsPtr->Addr.EndAddr = Mpu_Reg_GetRegionEndAddr((uint8)MpuId, (uint8)RegionId);
    RegionAttrs = Mpu_Reg_GetRegionAttrs((uint8)MpuId, (uint8)RegionId);
#if MPU_HAS_PROCESS_IDENTIFIER
    RegionPid = Mpu_Reg_GetRegionPid((uint8)MpuId, (uint8)RegionId);
    RegionParamsPtr->Attrs.PidMask = (uint8)((RegionPid & MPU_RGD0_WORD3_PIDMASK_Msk) >> MPU_RGD0_WORD3_PIDMASK_Pos);
    RegionParamsPtr->Attrs.Pid = (uint8)((RegionPid & MPU_RGD0_WORD3_PID_Msk) >> MPU_RGD0_WORD3_PID_Pos);
    RegionParamsPtr->Attrs.MasterPidEnable = RegionAttrs & MPU_MASTER_PID_ENABLE_MASK;
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
    RegionParamsPtr->Attrs.Permission = RegionAttrs & MPU_PERMISSION_MASK;
}

/**
 * @brief enable/disbale region.
 * @note Function ID: DES_MCL_API_405
 * @param[in] MpuId: mpu id
 * @param[in] RegionId: region id
 * @param[in] Enable: enable/disable region configuration
 * @return void
 */
void Mpu_Hal_EnableRegion(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, boolean Enable)
{
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(RegionId < MPU_REGION_ID_MAX);

    Mpu_Reg_Disable((uint8)MpuId);
    Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, Enable);
    Mpu_Reg_Enable((uint8)MpuId);
}

#if MPU_HAS_PROCESS_IDENTIFIER
/**
 * @brief set Master Pid.
 * @note Function ID:DES_MCL_API_407
 * @param[in] MpuId: mpu id
 * @param[in] Master: master id
 * @param[in] Pid: region pid and master pid match,will hit
 * @return void
 */
void Mpu_Hal_SetMasterPid(Mpu_IdType MpuId, Mpu_MasterType Master, uint8 Pid)
{
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(Master < MPU_MASTER_MAX);

    Mpu_Reg_SetMasterPid((uint8)MpuId, (uint8)Master, Pid);
}
#endif /* MPU_HAS_PROCESS_IDENTIFIER */

/**
 * @brief set region attribution.
 * @note Function ID: DES_MCL_API_406
 * @param[in] MpuId: mpu id
 * @param[in] RegionId: region id
 * @param[in] AttrsPtr: region attribute configuration pointer
 * @return void
 */
void Mpu_Hal_SetRegionAttrs(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, const Mpu_RegionAttrsType *AttrsPtr)
{
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(RegionId < MPU_REGION_ID_MAX);
    DEVICE_ASSERT(AttrsPtr != NULL_PTR);
    if (AttrsPtr != NULL_PTR)
    {
        Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, FALSE);
        Mpu_Reg_SetRegionAttrs((uint8)MpuId, (uint8)RegionId, (AttrsPtr->Permission | AttrsPtr->MasterPidEnable));
#if MPU_HAS_PROCESS_IDENTIFIER
        Mpu_Reg_SetRegionPid((uint8)MpuId, (uint8)RegionId, (uint8)AttrsPtr->Pid, (uint8)AttrsPtr->PidMask);
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
        Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, TRUE);
    }
}

/**
 * @brief get slave register idx.
 * @note Function ID: DES_MCL_API_412
 * @param[in] Slave : slave id
 * @return Mpu_IdType: mpu id
 */
LOCAL_INLINE Mpu_IdType Mpu_Hal_SlaveToMpu(Mpu_SlaveType Slave)
{
    Mpu_IdType MpuId = MPU_ID_0;
#if defined (AC7843X)
    /* calculate MPU 1 slave idx */
    if (Slave > MPU_SLAVE_SRAM_U)
    {
        MpuId = MPU_ID_1;
    }
#elif defined (AC7840X) || defined (AC7842X)
    (void)Slave;
#endif /* AC7843X */

    return MpuId;
}

/**
 * @brief get slave register idx.
 * @note Function ID: DES_MCL_API_411
 * @param[in] Slave : slave id
 * @return uint32: slave register idx
 */
LOCAL_INLINE uint8 Mpu_Hal_SlaveToRegIdx(Mpu_SlaveType Slave)
{
    uint8 SlaveRegIdx = (uint8)Slave;

#if defined (AC7843X)
    /* calculate MPU 1 slave idx */
    if (Slave > MPU_SLAVE_SRAM_U)
    {
        SlaveRegIdx = (uint8)Slave - 2U;
    }
#endif /* AC7843X */

    return SlaveRegIdx;
}

/**
 * @brief get error slave id.
 * @note Function ID: DES_MCL_API_410
 * @param[out] Slave : slave id
 * @return Hal_StatusType: Is there an error, STATUS_SUCCESS stands for no error; STATUS_ERROR is a error
 */
Hal_StatusType Mpu_Hal_GetErrorSlaveId(Mpu_SlaveType *Slave)
{
    uint32 Status;
    uint8 MpuIdx;
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */
    *Slave = MPU_SLAVE_MAX;

#if defined (AC7840X) || defined (AC7842X)
    MpuIdx = (uint8)MPU_ID_0;
    Status = Mpu_Reg_GetStatus(MpuIdx);
    /* MPU_SLAVE_FLASH occured error */
    if ((Status & MPU_CESR_SPERR0_Msk) != 0U)
    {
        *Slave = MPU_SLAVE_FLASH;
    }
    /* MPU_SLAVE_SRAM_L occured error */
    else if ((Status & MPU_CESR_SPERR1_Msk) != 0U)
    {
        *Slave = MPU_SLAVE_SRAM_L;
    }
    /* MPU_SLAVE_SRAM_U occured error */
    else if ((Status & MPU_CESR_SPERR2_Msk) != 0U)
    {
        *Slave = MPU_SLAVE_SRAM_U;
    }
    else
    {
        /*  nothing */
    }
#elif defined (AC7843X) /* AC7840X AC7842X */
    for (MpuIdx = (uint8)MPU_ID_0; MpuIdx < (uint8)MPU_ID_MAX; MpuIdx++)
    {
        Status = Mpu_Reg_GetStatus((uint8)MpuIdx);
        /* MPU slave 0 occured error */
        if ((Status & MPU_CESR_SPERR1_Msk) != 0U)
        {
            /* MPU 0 slave idx */
            if ((uint8)MPU_ID_0 == MpuIdx)
            {
                *Slave = MPU_SLAVE_SRAM_L;
            }
            else/* MPU 1 slave idx */
            {
                *Slave = MPU_SLAVE_PFLASH;
            }
        }
        /* MPU slave 1 occured error */
        else if ((Status & MPU_CESR_SPERR2_Msk) != 0U)
        {
            /* MPU 0 slave idx */
            if ((uint8)MPU_ID_0 == MpuIdx)
            {
                *Slave = MPU_SLAVE_SRAM_U;
            }
            else/* MPU 1 slave idx */
            {
                *Slave = MPU_SLAVE_DFLASH;
            }
        }
        else
        {
            /*  nothing */
        }
        if (MPU_SLAVE_MAX != *Slave)
        {
            break;
        }
    }
#endif /* AC7843X */
    /* mpu no error */
    if (MPU_SLAVE_MAX != *Slave)
    {
        ReturnValue = STATUS_ERROR;
    }

    return ReturnValue;
}

/**
 * @brief get slave error info.
 * @note Function ID: DES_MCL_API_408
 * @param[in] Slave: slave id
 * @param[in] ErrInfoPtr: Error info struction of Slave
 * @return Hal_StatusType: Is there an error, STATUS_SUCCESS stands for no error; STATUS_ERROR is a error
 */
Hal_StatusType Mpu_Hal_GetSlaveErrorInfo(Mpu_SlaveType Slave, Mpu_ErrorInfoType *ErrInfoPtr)
{
    Mpu_IdType MpuId;
    uint32 ErrorInfoal;
    uint8 SlaveRegIdx;
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */

    DEVICE_ASSERT(ErrInfoPtr != NULL_PTR);
    DEVICE_ASSERT(Slave < MPU_SLAVE_MAX);
    MpuId = Mpu_Hal_SlaveToMpu(Slave);
    SlaveRegIdx = Mpu_Hal_SlaveToRegIdx(Slave);
    ErrorInfoal = Mpu_Reg_GetErrorInfo((uint8)MpuId, SlaveRegIdx);
    /* mpu occured error */
    if ((ErrorInfoal & MPU_EDR2_EFLG_Msk) != 0U)
    {
        /* PRQA S 2812 ++ # the upper layer call guarantees that a null pointer will never appear.*/
        ErrInfoPtr->Addr = Mpu_Reg_GetErrorAddr((uint8)MpuId, SlaveRegIdx);
        /* PRQA S 2812 -- # the upper layer call guarantees that a null pointer will never appear.*/
        ErrInfoPtr->ErrorInfoal = ErrorInfoal;
        ReturnValue = STATUS_ERROR;
    }

    return ReturnValue;
}

/**
 * @brief Clear Error status of Port .
 * @note Function ID: DES_MCL_API_409
 * @param[in] Slave: slave id
 * @return void
 */
void Mpu_Hal_ClearSlaveError(Mpu_SlaveType Slave)
{
    uint32 Status;
    Mpu_IdType MpuId;

    DEVICE_ASSERT(Slave < MPU_SLAVE_MAX);
    MpuId = Mpu_Hal_SlaveToMpu(Slave);
    Mpu_Reg_ClearErrorInfo((uint8)MpuId, (uint8)Slave);

    /*clear interrupt status*/
    Status = Mpu_Reg_GetStatus((uint8)MpuId);
    Mpu_Reg_ClearStatus((uint8)MpuId, Status);
}
