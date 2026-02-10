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
 * @file Cmu_Hal.c
 *
 * @brief cmu hal source file.
 */
/*==============================================INCLUDE FILES=======================================*/

#include "Cmu_Hal.h"
#include "AC784xx_Cmu_Reg.h"
#include "OsIf_Critical.h"
#include "OsIf_Irq.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/* cmu module count value */
#define CMU_MODULE_COUNT          (3U)

/* cmu window length minimum value */
#define CMU_WINLEN_MIN_VALUE          (7U)

/* cmu threshold maximum value */
#define CMU_THRESHOLD_MAX_VALUE       (0xFFFFFFU)

/* timeout value to wait cmu work status stable */
#define CMU_WAIT_TIMEOUT             (100000U)

/*===================================================ENUMS==========================================*/
/*!
 * @brief cmu state machine
 */
typedef enum
{
    CMU_INITED = 0x0U,
    CMU_UNINITED,
    CMU_RUNNING,
    CMU_ABNORMAL
} Cmu_StateType;
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
static Cmu_StateType CmuStatus[CMU_MODULE_COUNT] = {CMU_UNINITED, CMU_UNINITED, CMU_UNINITED};
/*============================================FUNCTION PROTOTYPES===================================*/

/**
 * @brief Disable cmu module by wait timeout
 * @note Function ID: DES_MCU_API_406
 * @param[in] CmuId: CMU module id
 * @return Hal_StatusType: Disable cmu module status ,STATUS_TIMEOUT or STATUS_SUCCESS
 */
static Hal_StatusType Cmu_Hal_DisabeWithSync(Cmu_ModuleIdType CmuId)
{
    volatile uint8 CmuStatus0;/* the value of cmu status register */
    volatile uint8 CmuStatus1;/* the value of cmu status register */
    uint32 Timeout;/* timeout value to wait cmu is disabled status */
    Hal_StatusType RetValue = STATUS_SUCCESS;/* function return value */

    OSIF_ENTER_CRITICAL(CMU_HAL_ID1);
    /* wait cmu work status can be closed */
    for (Timeout = CMU_WAIT_TIMEOUT; 0U != Timeout; Timeout--)
    {
        CmuStatus0 = (uint8)((Cmu_Reg_GetStatus((uint8)CmuId) & CMU_SR_WORK_Msk) >> CMU_SR_WORK_Pos);
        CmuStatus1 = (uint8)((Cmu_Reg_GetStatus((uint8)CmuId) & CMU_SR_WORK_Msk) >> CMU_SR_WORK_Pos);
        /*PRQA S 3415 ++ # the right operand of the && and || operations has no side effect.*/
        if ((0U == CmuStatus0) && (1U == CmuStatus1))/* 0 means stop counting, 1 means counting is workoing */
            /*PRQA S 3415 -- */
        {
            Cmu_Reg_Enable((uint8)CmuId, FALSE);
            break;/*  can be closed only during the counting phase */
        }
    }
    OSIF_EXIT_CRITICAL(CMU_HAL_ID1);
    if (0U == Timeout)/* wait cmu work status equal to IsEnable timeout */
    {
        RetValue = STATUS_TIMEOUT;
    }

    return RetValue;
}

/**
 * @brief Initialize CMU module
 * @note Function ID: DES_MCU_API_401
 * @param[in] CmuId: CMU module id
 * @param[in] CtrlParam: the pointer to the Cmu_CtrlParamType structure
 * @return Hal_StatusType: Initialize success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Cmu_Hal_Init(Cmu_ModuleIdType CmuId, const Cmu_CtrlParamType *CtrlParam)
{
    Hal_StatusType RetValue = STATUS_ERROR;/* function return value */

    DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
    DEVICE_ASSERT(NULL_PTR != CtrlParam);
    if (NULL_PTR != CtrlParam)
    {
        DEVICE_ASSERT(CtrlParam->HighThreshold <= CMU_THRESHOLD_MAX_VALUE);
        DEVICE_ASSERT(CtrlParam->LowThreshold <= CMU_THRESHOLD_MAX_VALUE);
        DEVICE_ASSERT(CMU_WINLEN_MIN_VALUE <= CtrlParam->WinLen);

        if ((CMU_UNINITED == CmuStatus[(uint8)CmuId]) || (CMU_INITED == CmuStatus[(uint8)CmuId]))
        {
            Cmu_Reg_ClearStatus((uint8)CmuId);
            Cmu_Reg_SetWindowLen((uint8)CmuId, CtrlParam->WinLen);
            Cmu_Reg_SetHighThreshold((uint8)CmuId, CtrlParam->HighThreshold);
            Cmu_Reg_SetLowThreshold((uint8)CmuId, CtrlParam->LowThreshold);
            CmuStatus[(uint8)CmuId] = CMU_INITED;
            RetValue = STATUS_SUCCESS;
        }
        else
        {
            RetValue = STATUS_ERROR;
        }
    }

    return RetValue;
}

/**
 * @brief Deinitialize CMU module
 * @note Function ID: DES_MCU_API_402
 * @param[in] CmuId: CMU module id
 * @return void
 */
void Cmu_Hal_Deinit(Cmu_ModuleIdType CmuId)
{
    Hal_StatusType RetValue;/* function return value */

    DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
    if (CMU_RUNNING == CmuStatus[(uint8)CmuId])
    {
        RetValue = Cmu_Hal_DisabeWithSync(CmuId);
        if (STATUS_SUCCESS == RetValue)
        {
            CmuStatus[(uint8)CmuId] = CMU_INITED;
        }
        else
        {
            CmuStatus[(uint8)CmuId] = CMU_ABNORMAL;
        }
    }
    if (CMU_INITED == CmuStatus[(uint8)CmuId])
    {
        Cmu_Reg_ClearStatus((uint8)CmuId);
        CmuStatus[(uint8)CmuId] = CMU_UNINITED;
    }
}

/**
 * @brief Enable Cmu
 * @note Function ID: DES_MCU_API_403
 * @param[in] CmuId: CMU module id
 * @param[in] IsEnable: true enable cmu module, false disable cmu module
 * @return Hal_StatusType: Enable success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Cmu_Hal_Enable(Cmu_ModuleIdType CmuId, boolean IsEnable)
{
    Hal_StatusType RetValue = STATUS_SUCCESS;/* function return value */

    DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
    if (TRUE == IsEnable)
    {
        if (CMU_INITED == CmuStatus[(uint8)CmuId])
        {
            Cmu_Reg_Enable((uint8)CmuId, TRUE);
            CmuStatus[(uint8)CmuId] = CMU_RUNNING;
        }
        else if (CMU_RUNNING == CmuStatus[(uint8)CmuId])
        {
            RetValue = STATUS_SUCCESS;
        }
        else
        {
            RetValue = STATUS_ERROR;
        }
    }
    else
    {
        if ((CMU_RUNNING == CmuStatus[(uint8)CmuId]) || (CMU_ABNORMAL == CmuStatus[(uint8)CmuId]))
        {
            RetValue = Cmu_Hal_DisabeWithSync(CmuId);
            if (STATUS_SUCCESS == RetValue)
            {
                CmuStatus[(uint8)CmuId] = CMU_INITED;
            }
            else
            {
                CmuStatus[(uint8)CmuId] = CMU_ABNORMAL;
            }
        }
        else if (CMU_INITED == CmuStatus[(uint8)CmuId])
        {
            RetValue = STATUS_SUCCESS;
        }
        else
        {
            RetValue = STATUS_ERROR;
        }
    }

    return RetValue;
}

/**
 * @brief Clear Cmu error flag
 * @note Function ID: DES_MCU_API_404
 * @param[in] CmuId: CMU module id
 * @return void
 */
void Cmu_Hal_ClearErrStatus(Cmu_ModuleIdType CmuId)/*cstat !MISRAC2004-8.10*/ /*cstat !MISRAC2012-Rule-8.7*/
{
    DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
    if (CMU_UNINITED != CmuStatus[(uint8)CmuId])
    {
        Cmu_Reg_ClearStatus((uint8)CmuId);
    }
}

/**
 * @brief Get the error information of CMU
 * @note Function ID: DES_MCU_API_405
 * @param[in] CmuId: CMU module id
 * @param[out] ErrInfo: the pointer to the Cmu_ErrInfoType structure, save error message
 * @return Hal_StatusType: Is there an error, STATUS_SUCCESS stands for no error; STATUS_ERROR is a error
 */
Hal_StatusType Cmu_Hal_GetErrInfo(Cmu_ModuleIdType CmuId, Cmu_ErrInfoType *ErrInfo)
{
    uint8 StatusRegVal;/* the value of cmu status register */
    Hal_StatusType RetValue = STATUS_ERROR;/* function return value */

    DEVICE_ASSERT(ErrInfo != NULL_PTR);
    if (ErrInfo != NULL_PTR)
    {
        DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
        if (CMU_UNINITED != CmuStatus[(uint8)CmuId])
        {
            StatusRegVal = Cmu_Reg_GetStatus((uint8)CmuId);
            /* cmu status contains fll event */
            if (0U != (StatusRegVal & CMU_SR_FLL_Msk))
            {
                ErrInfo->IsFLL = TRUE;
            }
            else
            {
                ErrInfo->IsFLL = FALSE;
            }
            /* cmu status contains fhh event */
            if (0U != (StatusRegVal & CMU_SR_FHH_Msk))
            {
                ErrInfo->IsFHH = TRUE;
            }
            else
            {
                ErrInfo->IsFHH = FALSE;
            }
            /* cmu status contains ClkLoss event */
            if (0U != (StatusRegVal & CMU_SR_CLKLOSS_Msk))
            {
                ErrInfo->IsClkLoss = TRUE;
            }
            else
            {
                ErrInfo->IsClkLoss = FALSE;
            }
            /* error event occures, the result return STATUS_ERROR */
            if ((TRUE == ErrInfo->IsFLL) || (TRUE == ErrInfo->IsFHH) || (TRUE == ErrInfo->IsClkLoss))
            {
                RetValue = STATUS_ERROR;
            }
            else
            {
                RetValue = STATUS_SUCCESS;
            }
        }
    }

    return RetValue;
}

/* =============================================  EOF  ============================================== */
