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
 * @file Hsm_Hal.c
 *
 * @brief This file provides HSM integration functions.
 *
 */
/* ===========================================  Includes  =========================================== */
#include "Hsm_Hal.h"
#include "Fls_Hal.h"
#include "OsIf.h"
#include "OsIf_Time.h"
/* ============================================  Define  ============================================ */
/**
 * @brief Key handle info structure. Size: 40 bytes.
 * @note DES ID: DES_HSM_TYP_054
 */
typedef struct
{
    uint32 KeyHandle; /**< Key handle ID by HSM */
    uint8 AuthValue[32]; /**< Authentication value */
    uint32 AuthSize; /**< Authentication size */
}HSM_KeyHandleInfoType;

/**
 * @brief Key slot info structure. Size: 8 bytes.
 * @note DES ID: DES_HSM_TYP_055
 */
typedef struct
{
    uint32 SlotValid; /**< Slot valid flag: 0xA5 * 4 means invalid, others depends on 'dataValidFlag' */
    uint32 SlotIndex; /**< Slot index */
}HSM_KeySlotInfoType;

/**
 * @brief Key index info structure. Size: 8 bytes.
 * @note DES ID: DES_HSM_TYP_056
 */
typedef struct
{
    uint32 KeyValid;  /**< Key valid flag: 0x52525252 means valid, other means invalid */
    uint32 KeyIndex;   /**< Key index by host */
}HSM_KeyIndexInfoType;

/**
 * @brief Key info structure. Size: 56 bytes.
 * @note DES ID: DES_HSM_TYP_057
 */
typedef struct
{
    HSM_KeySlotInfoType SlotInfo; /**< Slot information */
    HSM_KeyIndexInfoType IndexInfo; /**< Index information */
    HSM_KeyHandleInfoType HandleInfo; /**< Handle information */
}HSM_FlashKeyType;

/**
 * @brief Flash key page structure. Size: DFLASH_PAGE_SIZE.
 * @note DES ID: DES_HSM_TYP_058
 */
typedef struct
{
    HSM_FlashKeyType KeyInfo[HSM_FLASH_KEY_MAX_SLOT_NUM]; /**< Key information array */
    uint8 PageReverse[HSM_FLASH_PAGE_REVERSE_LEN]; /**< Page reverse area */
    uint8 PageValid[HSM_FLASH_PAGE_VALID_LEN]; /**< Page valid flag: 0xa5 means valid, other means invalid */
}HSM_FlashKeyPageType;

/**
 * @brief RAM key info structure.
 * @note DES ID: DES_HSM_TYP_059
 */
typedef struct
{
    uint8 *KeyHandleAddr; /**< Pointer to key handle */
    uint32 Used; /**< Key slot be used */
    uint32 KeyIndex; /**< Key index */
}HSM_RamKeyInfoType;

/**
 * @brief RAM key space header structure.
 * @note DES ID: DES_HSM_TYP_060
 */
typedef struct
{
    uint8 *KeyMemStart; /**< RAM key handle buffer address */
    uint32 KeyMemUsedSize; /**< RAM key space used size */
    uint32 KeyNum; /**< Already existed key number */
    HSM_RamKeyInfoType KeyInfo[HSM_RAM_KEY_MAX_NUM]; /**< RAM key handle buffer slot */
}HSM_RamKeyHeadType;

#ifndef FLASH_INFO1_BASE_OFFSET
#define FLASH_INFO1_BASE_OFFSET                (0x4800U)
#endif

#ifndef FLASH_INFO2_BASE_OFFSET
#define FLASH_INFO2_BASE_OFFSET                (0x5000U)
#endif

#define FLASH_INFO_SIZE                        (0x30U)

#define HSM_FIRMWARE_INSTALL_ADDRESS           (0x01120000U)

/**
 * @brief Maximum number of salt info, because hsm support 128 bytes random.
 * @note DES ID: DES_GPT_MACRO_037
 */
#define SALT_MAX_LEN                           (128)
/* ==========================================  Variables  =========================================== */
/*PRQA S 4342 EOF # convert unsigned to enum to ensure that there will be no problems in the current code.*/

/**
 * @brief HSM session buffer
 * @note  DES ID: DES_HSM_VAR_100
 */
static ehsm_ctx_session_st Hsm_Session[1U];
/**
 * @brief HSM key buffer
 * @note  DES ID: DES_HSM_VAR_101
 */
static uint32 Hsm_Key[1024U];

/* PRQA S 3218 ++ #Variables inside functions that occupy a large stack space are allowed to be defined as static global variables. */
/**
 * @brief RAM key info buffer
 * @note  DES ID: DES_HSM_VAR_102
 */
static uint8 HSM_RamKey[HSM_RAM_KEY_MEM_SIZE];
/**
 * @brief Total output size
 * @note  DES ID: DES_HSM_VAR_103
 */
static uint32 TotalOuputSize = 0U;
/**
 * @brief RAM key head structure
 * @note  DES ID: DES_HSM_VAR_104
 */
static HSM_RamKeyHeadType RamKeyHead;
/**
 * @brief Internal key structure
 * @note  DES ID: DES_HSM_VAR_105
 */
static ehsm_internal_key_st InterKey;

/*PRQA S 3218 -- #Variables inside functions that occupy a large of stack space are allowed to be defined as static global variables. */

/**
 * @brief Callback function pointer
 * @note  DES ID: DES_HSM_VAR_106
 */
static Hal_CallbackType HSM_IsrCallback = NULL_PTR;
/**
 * @brief Callback argument pointer
 * @note  DES ID: DES_HSM_VAR_107
 */
static void *CallbackArgs = NULL_PTR;

/* Functions declaration section. */
/* ====================================  Functions declaration  ===================================== */
ISR(HSM_IRQHandler);

/*!
 * @brief get key store type by key id.
 *
 * @note  Function ID: DES_HSM_API_154
 * @param[in] KeyId: key index will be used.
 * @return HSM_KeyStorageType.
 */
static HSM_KeyStorageType HSM_Hal_GetKeyStoreTypeByKeyId(HSM_KeyId KeyId)
{
    HSM_KeyStorageType StoreType;
    if (KeyId <= HSM_FLASH_KEY_RSA_3)
    {
        StoreType = HSM_KEY_TYPE_NVM;
    }
    else
    {
        StoreType = HSM_KEY_TYPE_RAM;
    }

    return StoreType;
}

/*!
 * @brief error code convert
 * @note  Function ID: DES_HSM_API_149
 * @param [in] Ret: error code.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: get key handle successfull.
 */
static Hal_StatusType HSM_Hal_ErrorCodeCov(uint32 Ret);

/*!
 * @brief adjust the keyindex valid.
 *
 * @note  Function ID: DES_HSM_API_120
 * @param[in] KeyId: key index will be used.
 * @param[in] KeyType: the key index storage type.
 * @param[in] AlgoType: the key use for algo type.
 * @return Hal_StatusType:
 *         STATUS_ERROR: invalid key index;
 *         STATUS_SUCCESS: valid key index.
 */
static Hal_StatusType HSM_Hal_IsKeyIndexValid(HSM_KeyId KeyId, HSM_KeyAlgoType AlgoType)
{
    Hal_StatusType Ret = STATUS_ERROR;
    
    if (HSM_Hal_GetKeyStoreTypeByKeyId(KeyId) == HSM_KEY_TYPE_NVM)
    {
        switch (AlgoType)
        {
        case HSM_ALGO_TYPE_RANDOM:
        case HSM_ALGO_TYPE_SYM:
        case HSM_ALGO_TYPE_DH:
            if (KeyId <= HSM_FLASH_KEY_SYM_9)
            {
                Ret = STATUS_SUCCESS;
            }
            break;
        case HSM_ALGO_TYPE_ECC:
            if ((KeyId >= HSM_FLASH_KEY_ECC_0) && (KeyId <= HSM_FLASH_KEY_ECC_9))
            {
                Ret = STATUS_SUCCESS;
            }
            break;
        case HSM_ALGO_TYPE_SM2:
            if ((KeyId >= HSM_FLASH_KEY_SM2_0) && (KeyId <= HSM_FLASH_KEY_SM2_9))
            {
                Ret = STATUS_SUCCESS;
            }
            break;
        case HSM_ALGO_TYPE_RSA_CRT:
        case HSM_ALGO_TYPE_RSA_COMM:
            if (KeyId >= HSM_FLASH_KEY_RSA_0)
            {
                Ret = STATUS_SUCCESS;
            }
            break;
        default:
            Ret = STATUS_ERROR;
            break;
        }
    }
    else
    {
        switch (AlgoType)
        {
        case HSM_ALGO_TYPE_RANDOM:
        case HSM_ALGO_TYPE_SYM:
        case HSM_ALGO_TYPE_DH:
            if (KeyId <= HSM_RAM_KEY_SYM_9)
            {
                Ret = STATUS_SUCCESS;
            }
            break;
        case HSM_ALGO_TYPE_ECC:
            if ((KeyId >= HSM_RAM_KEY_ECC_0) && (KeyId <= HSM_RAM_KEY_ECC_4))
             {
                 Ret = STATUS_SUCCESS;
             }
            break;
        case HSM_ALGO_TYPE_SM2:
            if ((KeyId >= HSM_RAM_KEY_SM2_0) && (KeyId <= HSM_RAM_KEY_SM2_4))
             {
                 Ret = STATUS_SUCCESS;
             }

            break;
        case HSM_ALGO_TYPE_RSA_CRT:
        case HSM_ALGO_TYPE_RSA_COMM:
            if ((KeyId >= HSM_RAM_KEY_RSA_0) && (KeyId <= HSM_RAM_KEY_RSA_3))
             {
                 Ret = STATUS_SUCCESS;
             }

            break;
        default:
            Ret = STATUS_ERROR;
            break;
        }
    }

    return Ret;
}

/*!
 * @brief adjust the flash key page valid.
 *
 * @note  Function ID: DES_HSM_API_121
 * @param[in] PageIndex: Page Index, 0 or 1.
 * @return Hal_StatusType:
 *         STATUS_ERROR: invalid page index;
 *         STATUS_SUCCESS: valid page index.
 */
static Hal_StatusType HSM_Hal_IsKeyPageValid(uint8 PageIndex)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint8 TagIndex;
    HSM_FlashKeyPageType const *TempKeyPage = (HSM_FlashKeyPageType *)((uint32)HSM_FLASH_KEY_BASE);
    HSM_FlashKeyPageType const *KeyPage;
    /*KeyPage step is DFLASH_PAGE_SIZE=0x400*/
    KeyPage = &TempKeyPage[PageIndex];
    /*pgae valid tag is 0xAA * 8*/
    for (TagIndex = 0U; TagIndex < HSM_FLASH_PAGE_VALID_LEN; TagIndex++)
    {
        if (KeyPage->PageValid[TagIndex] != HSM_PAGE_VALID_TAG)
        {
            Ret = STATUS_ERROR;
            break;
        }
    }

   return Ret;
}

/*!
 * @brief find valid flash key page.
 *
 * @note  Function ID: DES_HSM_API_122
 * @param[in] PageIndex: Page Index, 0 or 1.
 * @return Hal_StatusType:
 *         STATUS_ERROR: invalid page index;
 *         STATUS_SUCCESS: valid page index.
 */
static Hal_StatusType HSM_Hal_FindValidKeyPage(uint8 *PageIndex)
{
    Hal_StatusType Ret = STATUS_ERROR;
    uint8 Index;

    /*find Page is valid*/
    for (Index = 0U; Index < HSM_FLASH_PAGE_NUM; Index++)
    {
        /*Page is valid*/
        if (HSM_Hal_IsKeyPageValid(Index) == STATUS_SUCCESS)
        {
            /*return page index value*/
            *PageIndex = Index;
            Ret = STATUS_SUCCESS;
            break;
        }
    }

    return Ret;
}

/*!
* @brief Install Key Data into target key-page and key-slot.
*
* @note  Function ID: DES_HSM_API_123
* @param[in] PageIndex: the index of the target key-page
* @param[in] SlotIndex: the index of the target key-slot
* @param[in] KeyHandle: the key handle struct.
* @param[in] KeyIndex: the index of the target key(0-8)
* @return Hal_StatusType:
*         STATUS_ERROR: error happend in flash operation;
*         STATUS_SUCCESS: setup finished
*/
static Hal_StatusType HSM_Hal_SetupKeyData(uint32 PageIndex, uint32 SlotIndex,
            const HSM_KeyHandleInfoType *KeyHandle, uint8 KeyIndex)
{
    Hal_StatusType Ret;
    HSM_FlashKeyPageType const *KeyPage = (HSM_FlashKeyPageType *)
                               ((HSM_FLASH_KEY_OFFSET + (PageIndex * DFLASH_PAGE_SIZE)));
    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
    HSM_FlashKeyType  const  *TempKeyInfoNode = (HSM_FlashKeyType const *)KeyPage->KeyInfo;
    /*PRQA S 0310 -- #make sure there are no alignment issues.*/
    HSM_KeyIndexInfoType IndexType;

    /*set KeyInfo Node address*/
    HSM_FlashKeyType  const  *KeyInfoNode = &TempKeyInfoNode[SlotIndex];

    IndexType.KeyIndex = KeyIndex;
    IndexType.KeyValid = HSM_KEY_DATA_VALID_TAG;
    /*write Key Index Data and Key valid data to dflash*/
    Ret = DFlash_Hal_PageWrite((uint32)&KeyInfoNode->IndexInfo,
        sizeof(HSM_KeyIndexInfoType), (const uint8 *)&IndexType);
    if (STATUS_SUCCESS == Ret)
    {
        /*write Key handle data to dflash*/
        Ret = DFlash_Hal_PageWrite((uint32)&KeyInfoNode->HandleInfo,
                    sizeof(HSM_KeyHandleInfoType), (const uint8 *)KeyHandle);
    }
    return Ret;
}

/*!
 * @brief Make sure the input index of the key-slot could be deprecated.
 *
 * @note  Function ID: DES_HSM_API_124
 * @param[in] SlotIndex: the index of key-slot need to be check
 * @param[in] PageIndex: the index of key-page need to be check
 * @return Hal_StatusType:
 *         STATUS_ERROR: key slot invalid.
 *         STATUS_SUCCESS: key slot valid.
 */
static Hal_StatusType HSM_Hal_IsKeySlotValid(uint32 SlotIndex, uint32 PageIndex)
{
    HSM_FlashKeyType const *keySlot = (HSM_FlashKeyType *)(HSM_FLASH_KEY_BASE + (PageIndex * DFLASH_PAGE_SIZE));

    uint8 const *Ptr;
    uint8 i;
    Hal_StatusType Ret = STATUS_SUCCESS;

    keySlot = &keySlot[SlotIndex];

    Ptr = (const uint8 *)keySlot;
    for (i = 0U; i < sizeof(HSM_KeySlotInfoType); i++)
    {
        if (Ptr[i] != 0xFFU)
        {
            Ret = STATUS_ERROR;
            break;
        }
    }

    return Ret;
}

/*!
 * @brief Find Key Slot is invalid.
 *
 * @note  Function ID: DES_HSM_API_125
 * @param[in] PageIndex: the index of key-page need to be check.
 * @return Hal_StatusType:
 *         STATUS_ERROR: error happend;
 *         STATUS_SUCCESS: setup finished
 */
static Hal_StatusType HSM_Hal_FindKeySlotInvalid(uint32 PageIndex)
{
    uint8 i;
    Hal_StatusType Ret = STATUS_ERROR;
    for (i = 0U; i < HSM_FLASH_KEY_MAX_SLOT_NUM; i++)
    {
        /*have invalid slot but use space*/
        if (HSM_Hal_IsKeySlotValid(i, PageIndex) == STATUS_ERROR)
        {
            Ret = STATUS_SUCCESS;
            break;
        }
    }

    return Ret;
}

/*!
 * @brief Make sure the input index of the key-slot could be used.
 *
 * @note  Function ID: DES_HSM_API_126
 * @param[in] SlotIndex: the index of key-slot need to be check
 * @param[in] PageIndex: the index of key-page need to be check
 * @return Hal_StatusType:
 *         STATUS_ERROR: key slot not usable.
 *         STATUS_SUCCESS: key slot can use.
 */
static Hal_StatusType HSM_Hal_IsKeySlotUsable(uint32 SlotIndex, uint32 PageIndex)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    HSM_FlashKeyType const *KeyNode = (HSM_FlashKeyType *)(HSM_FLASH_KEY_BASE + (PageIndex * DFLASH_PAGE_SIZE));
    uint8 const *Ptr;
    uint8 i;

    KeyNode = &KeyNode[SlotIndex];

    Ptr = (const uint8 *)KeyNode;
    for (i = 0U; i < sizeof(HSM_FlashKeyType); i++)
    {
        if (Ptr[i] != 0xFFU)
        {
            Ret = STATUS_ERROR;
            break;
        }
    }

    return Ret;
}

/*!
 * @brief Find the key-slot which include the target keyindex.
 *
 * @note  Function ID: DES_HSM_API_127
 * @param[out] SlotIndex: output the usable index of key-slot
 * @param[in] PageIndex: the index of key-page need to be check
 * @param[in] KeyIndex: the index of key need to be check
 * @return Hal_StatusType:
 *         STATUS_ERROR: all key-slot are used-up;
 *         STATUS_SUCCESS: slotIndex is output successfull
 */
static Hal_StatusType HSM_Hal_FindKeySlotByIndex(uint32 *SlotIndex, uint32 PageIndex, uint8 KeyIndex)
{
    Hal_StatusType Ret = STATUS_ERROR;
    HSM_FlashKeyPageType const *KeyPage = (HSM_FlashKeyPageType *)(HSM_FLASH_KEY_BASE + (PageIndex * DFLASH_PAGE_SIZE));
    uint32 SlotId;

    for (SlotId = 0U; SlotId < HSM_FLASH_KEY_MAX_SLOT_NUM; SlotId++)
    {
        /* If current key data is A5A5, than key slot is valid , will we can use it by key info*/
        if (((KeyPage->KeyInfo[SlotId].IndexInfo.KeyValid == HSM_KEY_DATA_VALID_TAG)) &&
                (KeyPage->KeyInfo[SlotId].IndexInfo.KeyIndex == KeyIndex) &&
                (HSM_Hal_IsKeySlotValid(SlotId, PageIndex) == STATUS_SUCCESS))
        {
            *SlotIndex = SlotId;
            Ret = STATUS_SUCCESS;
            break;
        }
    }
    return Ret;
}

/*!
 * @brief get pubkey size and pubkey by algo type
 * @note  Function ID: DES_HSM_API_128
 * @param [in] Algo: key algo type
 * @param [in] DesPubKey: pubkey databuffer.
 * @param [in] SrcPubKey: source pubkey databuffer.
 * @param [in] PubKeySize: pubkey size.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: get key handle successfull.
 */
static Hal_StatusType HSM_Hal_GetPubKeyByAlgo(HSM_GenKeyAlgo Algo, uint8 *DesPubKey,
        const ehsm_export_pub_key_st *SrcPubKey, uint32 *PubKeySize)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    switch (Algo)
    {
    case HSM_ALG_SM2:
        *PubKeySize = 64u;
        (void)System_Memcpy(DesPubKey, (const uint8 *)&SrcPubKey->key.ecc, *PubKeySize);
        break;
    case HSM_ALG_SECP256R1:
        *PubKeySize = 64u;
        (void)System_Memcpy(DesPubKey, (const uint8 *)&SrcPubKey->key.ecc, *PubKeySize);
        break;
    case HSM_ALG_SECP384R1:
        *PubKeySize = 96u;
        (void)System_Memcpy(DesPubKey, (const uint8 *)&SrcPubKey->key.ecc, *PubKeySize);
        break;

    case HSM_ALG_RSA_1024:
    case HSM_ALG_RSA_1024_CRT:
        *PubKeySize = 256u;  /* n and e*/
        (void)System_Memcpy(DesPubKey, (const uint8 *)&SrcPubKey->key.rsa, *PubKeySize);
        break;
    case HSM_ALG_RSA_2048:
    case HSM_ALG_RSA_2048_CRT:
        *PubKeySize = 512u;  /* n and e*/
        (void)System_Memcpy(DesPubKey, (const uint8 *)&SrcPubKey->key.rsa, *PubKeySize);
        break;
    case HSM_ALG_DH:
        *PubKeySize = 2048u;  /* n and e*/
        (void)System_Memcpy(DesPubKey, (const uint8 *)&SrcPubKey->key.dh, *PubKeySize);
        break;
    default:
        *PubKeySize = 0U;
        Ret = STATUS_ERROR;
        break;
    }

    return Ret;
}

/*!
 * @brief Invalid the target key-slot.
 *
 * @note  Function ID: DES_HSM_API_129
 * @param[in] PageIndex: the index of the target key-page
 * @param[in] SlotIndex: the index of the target key-slot
 * @return Hal_StatusType:
 *         STATUS_ERROR: error happend in flash operation;
 *         STATUS_SUCCESS: invalid finished
 */
static Hal_StatusType HSM_Hal_InvalidKeySlot(uint32 PageIndex, uint32 SlotIndex)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    HSM_FlashKeyPageType const *KeyPage = ((HSM_FlashKeyPageType *)HSM_FLASH_KEY_OFFSET);

    KeyPage = &KeyPage[PageIndex];
    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
    HSM_FlashKeyType const *KeyPtr = (HSM_FlashKeyType const *)KeyPage;
    /*PRQA S 0310 -- #make sure there are no alignment issues.*/
    HSM_KeySlotInfoType KeySlot = {0};

    KeySlot.SlotValid = HSM_SLOT_INVALID_TAG;

    KeyPtr = &KeyPtr[SlotIndex];

    /* set slot invalid tag */
    Ret = DFlash_Hal_PageWrite((uint32)&KeyPtr->SlotInfo, sizeof(KeySlot) / sizeof(uint8), (const uint8 *)&KeySlot);
    return Ret;
}

/*!
 * @brief Find an usable key-slot to install a new key from the input key-page.
 *
 * @note  Function ID: DES_HSM_API_130
 * @param[out] SlotIndex: output the usable index of key-slot
 * @param[in] PageIndex: the index of key-page need to be check
 * @return Hal_StatusType:
 *         STATUS_ERROR: all key-slot are used-up;
 *         STATUS_SUCCESS: slotIndex is output successfull
 */
static Hal_StatusType HSM_Hal_FindUsableKeySlot(uint32 *SlotIndex, uint32 PageIndex)
{
    Hal_StatusType Ret = STATUS_ERROR;
    uint32 SlotId;

    for (SlotId = 0U; SlotId < HSM_FLASH_KEY_MAX_SLOT_NUM; SlotId++)
    {
        /* If current slot is A5A5, than key slot is invalid which means it could be used directly */
        if (HSM_Hal_IsKeySlotUsable(SlotId, PageIndex) == STATUS_SUCCESS)
        {
            *SlotIndex = SlotId;
            Ret = STATUS_SUCCESS;

            break;
        }
        else
        {
            Ret = STATUS_ERROR;
        }
    }

    return Ret;
}

/*!
 * @brief Update key-page: set the first page as invalid, and second page as active.
 *
 * @note  Function ID: DES_HSM_API_131
 * @param[out] PageIndex: output the index of the final valid key-page
 * @return Hal_StatusType:
 *         STATUS_ERROR: error happend in flash operation;
 *         STATUS_SUCCESS: pageIndex is output successfull
 */
static Hal_StatusType HSM_Hal_UpdateFlashKeyPage(uint8 *PageIndex)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    HSM_FlashKeyPageType const *mainKeyPage = (HSM_FlashKeyPageType *)(HSM_FLASH_KEY_OFFSET
            + ((uint32)(*PageIndex)*DFLASH_PAGE_SIZE));
    HSM_FlashKeyPageType const *backupKeyPage;
    HSM_FlashKeyType const *mainKeySlot;
    HSM_FlashKeyType const *backupKeySlot;
    uint8 flashWriteBuffer[HSM_FLASH_PAGE_VALID_LEN];
    HSM_FlashKeyType const *DstAddr = NULL_PTR;
    uint32 i = 0U, j = 0U;

    /*pageIndex =0, will copy to page 1*/
    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
    if (0U == *PageIndex)
    {
        /*PRQA S 0489 ++ #The integer value 1 is being added or subtracted from a pointer..*/
        backupKeyPage = mainKeyPage + 1U;
        /*PRQA S 0489 -- #The integer value 1 is being added or subtracted from a pointer..*/
    }
    else  /*pageIndex =1, will copy to page 0*/
    {
        /*PRQA S 0489 ++ #The integer value 1 is being added or subtracted from a pointer..*/
        backupKeyPage = mainKeyPage - 1U;
        /*PRQA S 0489 -- #The integer value 1 is being added or subtracted from a pointer..*/
    }
    mainKeySlot = mainKeyPage->KeyInfo;
    backupKeySlot = backupKeyPage->KeyInfo;

    DstAddr = (HSM_FlashKeyType *)((uint32)mainKeySlot + DFLASH_BASE);
    /* Copy valid key slot from main page to backup page */
    for (i = 0U; i < HSM_FLASH_KEY_MAX_SLOT_NUM; i++)
    {
        if (STATUS_SUCCESS == HSM_Hal_IsKeySlotValid(i, *PageIndex))
        {
            Ret = DFlash_Hal_PageWrite((uint32)(&backupKeySlot[j]), sizeof(HSM_FlashKeyType) / sizeof(uint8),
                        (const uint8*)(&DstAddr[i]));
            j++;
        }
    }

    if (STATUS_SUCCESS == Ret)
    {
        /* mark the backup keypage as valid state */
        /*PRQA S 1291 ++ #An integer constant of 'essentially unsigned' type is being converted to signed type on assignment.*/
        (void)System_Memset(flashWriteBuffer, HSM_PAGE_VALID_TAG, HSM_FLASH_PAGE_VALID_LEN);
        /*PRQA S 1291 -- #An integer constant of 'essentially unsigned' type is being converted to signed type on assignment.*/
        Ret = DFlash_Hal_PageWrite((uint32)backupKeyPage->PageValid, HSM_FLASH_PAGE_VALID_LEN,
            (const uint8 *)flashWriteBuffer);
        if (STATUS_SUCCESS == Ret)
        {
            /*the erase page and  pageindex*/
            /* clean the default keypage */
            Ret = DFlash_Hal_PageErase(HSM_FLASH_KEY_OFFSET + (((uint32)(*PageIndex))*DFLASH_PAGE_SIZE));
            if (STATUS_SUCCESS == Ret)
            {
                if (0U == *PageIndex)
                {
                    *PageIndex = 1U;
                }
                else
                {
                    *PageIndex = 0U;
                }
            }
        }
    }
    /*PRQA S 0310 -- #make sure there are no alignment issues.*/
    return Ret;
}

/*!
 * @brief Initial Flash Key Store Page function, default pageindex = 0.
 * @note  Function ID: DES_HSM_API_132
 * @return Hal_StatusType:
 *         STATUS_ERROR: error happend in flash operation;
 *         STATUS_SUCCESS: pageIndex is output successfull
 */
static Hal_StatusType HSM_Hal_InitFlashKeyPage(void)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    HSM_FlashKeyPageType const *KeyPage = NULL_PTR;
    uint8 i;
    /*define FLASH Key Page offset from customer*/

    uint8 PageValid[HSM_FLASH_PAGE_VALID_LEN];

    /* find valid key page exist or not */
    for (i=0U; i<HSM_FLASH_PAGE_NUM; i++)
    {
        Ret = HSM_Hal_IsKeyPageValid(i);
        if (STATUS_SUCCESS == Ret)
        {
            break;
        }
    }
    /* not find  valid key page*/
    if (2U == i)
   {
         Ret = STATUS_ERROR;
    }
    /* not find key page valid , so init key page and default key pageindex = 0 */
    if (Ret != STATUS_SUCCESS)
    {
        /* if no exist key page valid, we should init all key page */
        Ret =  DFlash_Hal_PageErase(HSM_FLASH_KEY_OFFSET);
        if (STATUS_SUCCESS == Ret)
        {
            Ret =  DFlash_Hal_PageErase(HSM_FLASH_KEY_OFFSET + DFLASH_PAGE_SIZE);
        }

        if (Ret != STATUS_SUCCESS)
        {
            Ret = STATUS_ERROR;
        }
        else /* init all key page success set first page valid at default*/
        {
            KeyPage = (HSM_FlashKeyPageType *)(HSM_FLASH_KEY_OFFSET);
            /* fill valid tag, default key page is page 0 */
            /*PRQA S 1291 ++ #An integer constant of 'essentially unsigned' type is being converted to signed type on assignment.*/
            (void)System_Memset(PageValid, HSM_PAGE_VALID_TAG, HSM_FLASH_PAGE_VALID_LEN);
            /*PRQA S 1291 -- #An integer constant of 'essentially unsigned' type is being converted to signed type on assignment.*/
            Ret = DFlash_Hal_PageWrite((uint32)KeyPage->PageValid, HSM_FLASH_PAGE_VALID_LEN / sizeof(uint8),
                                                    (const uint8 *)PageValid);
            if (Ret != STATUS_SUCCESS)
            {
                Ret = STATUS_ERROR;
            }
        }
    }

    return Ret;
}

/*!
 * @brief Initial Ram Key Store global variable.
 * @note  Function ID: DES_HSM_API_133
 * @return void
 */
static void HSM_Hal_InitRamKeyMem(void)
{
    /* System_Memset hsm ram key space */
    (void)System_Memset(HSM_RamKey, 0U, sizeof(HSM_RamKey));

    /* System_Memset hsm ram key head struct */
    (void)System_Memset((void *)&RamKeyHead, 0U, sizeof(RamKeyHead));

    /* init ram key start address */
    RamKeyHead.KeyMemStart = HSM_RamKey;

    /* init ramkey mem used size */
    RamKeyHead.KeyMemUsedSize = 0U;

    /* init ramkey num */
    RamKeyHead.KeyNum = 0U;
}

/*!
 * @brief get ram key handle function.
 * @note  Function ID: DES_HSM_API_134
 * @param [out] Handle: key handle pointer.
 * @param [in] KeyIndex: keyindex index.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: get key handle successfull.
 */
static Hal_StatusType HSM_Hal_GetRamKeyHandle(HSM_KeyHandleInfoType *Handle, uint32 KeyIndex)
{
    Hal_StatusType Ret = STATUS_ERROR;
    uint8 i;
    /*PRQA S 3397 ++ # A binary operation is the operand of a binary operator with different precedence.*/
    for (i = 0U; i < HSM_RAM_KEY_MAX_NUM; i++)
    /*PRQA S 3397 -- # A binary operation is the operand of a binary operator with different precedence.*/
    {
        /* find keyhandle by keyindex */
        if ((RamKeyHead.KeyInfo[i].KeyIndex == KeyIndex) && (1U == RamKeyHead.KeyInfo[i].Used))
        {
            (void)System_Memcpy((uint8 *)Handle, (const uint8 *)RamKeyHead.KeyInfo[i].KeyHandleAddr,
                        sizeof(HSM_KeyHandleInfoType));
            Ret = STATUS_SUCCESS;

            break;
        }
    }

    return Ret;
}

/*!
 * @brief install ram Key to ram key space function.
 * @note  Function ID: DES_HSM_API_135
 * @param [in] KeyHandle: key handle from hsm.
 * @param [in] AuthValue: authentication message when access key.
 * @param [in] AuthSize: authentication message size
 * @param [in] KeyIndex: key index in ram space.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: install ram key successfull.
 */
static Hal_StatusType HSM_Hal_InstallRamKey(uint32 KeyHandle, const uint8 *AuthValue, uint32 AuthSize, uint32 KeyIndex)
{
    Hal_StatusType Ret = STATUS_ERROR;
    HSM_KeyHandleInfoType Handle;
    uint8 i;

    /* if free key space not enough , return STATUS_ERROR */
    /*PRQA S 3397 ++ # A binary operation is the operand of a binary operator with different precedence.*/
    if (RamKeyHead.KeyNum < HSM_RAM_KEY_MAX_NUM)
    {
        for (i=0U; i< HSM_RAM_KEY_MAX_NUM; i++) /* PRQA S 0771 ++ # More break */
        {
            /*if key index exist, must delete it then install it */
            if ((RamKeyHead.KeyInfo[i].KeyIndex == KeyIndex) && (1U == RamKeyHead.KeyInfo[i].Used))
            {
                Ret = STATUS_ERROR;
                break;
            }
            else
            {
                /*if key slot used, continue find next slot */
                if (1U == RamKeyHead.KeyInfo[i].Used)
                {
                    continue;
                }
                else
                {
                    RamKeyHead.KeyInfo[i].Used = 1U;
                    RamKeyHead.KeyInfo[i].KeyIndex = KeyIndex;
                    /* set handle address */
                    RamKeyHead.KeyInfo[i].KeyHandleAddr = &RamKeyHead.KeyMemStart[40U * i];
                    Handle.KeyHandle = KeyHandle;
                    Handle.AuthSize = AuthSize;
                    (void)System_Memcpy(Handle.AuthValue, AuthValue, AuthSize);
                    (void)System_Memcpy(RamKeyHead.KeyInfo[i].KeyHandleAddr, (const uint8 *)&Handle,
                                    sizeof(HSM_KeyHandleInfoType));
                }
                RamKeyHead.KeyNum++;

                /* adjust key slot append tail */
                if (RamKeyHead.KeyInfo[i].KeyHandleAddr >= &RamKeyHead.KeyMemStart[RamKeyHead.KeyMemUsedSize])
                {
                    RamKeyHead.KeyMemUsedSize += 40U;
                }
                Ret = STATUS_SUCCESS;
                break;
            }
        }
    }
    /*PRQA S 3397 -- # A binary operation is the operand of a binary operator with different precedence.*/
    return Ret;
}

/*!
 * @brief adjust the key index existed in ram key slot.
 * @note  Function ID: DES_HSM_API_136
 * @param [in] KeyIndex: key index in ram space.
 * @param [in] SlotIndex: key slot usable.
 * @return Hal_StatusType
 *         STATUS_ERROR: key index not existed ;
 *         STATUS_SUCCESS: key index existed and return key slot.
 */
static Hal_StatusType HSM_Hal_IsRamKeyIndexExisted(uint8 KeyIndex, uint8 *SlotIndex)
{
    Hal_StatusType Ret = STATUS_ERROR;
    uint8 i;
    /*PRQA S 3397 ++ # A binary operation is the operand of a binary operator with different precedence.*/
    for (i = 0U; i < HSM_RAM_KEY_MAX_NUM; i++)
    /*PRQA S 3397 -- # A binary operation is the operand of a binary operator with different precedence.*/
    {
        /* If current key data is A5A5, than key slot is valid , will we can't use the keyindex */
        if ((RamKeyHead.KeyInfo[i].KeyIndex == KeyIndex) && (1U == RamKeyHead.KeyInfo[i].Used))
        {
            if (SlotIndex != NULL_PTR)
            {
                *SlotIndex = i;
            }
            Ret = STATUS_SUCCESS;
            break;
        }
    }

    return Ret;
}

/*!
 * @brief delete ram Key function.
 * @note  Function ID: DES_HSM_API_137
 * @param [in] KeyIndex: the key index will be remove from ram space and hsm.
 * @return Hal_StatusType
 *         STATUS_ERROR: remove fail;
 *         STATUS_SUCCESS: remove successfull.
 */
static Hal_StatusType HSM_Hal_UninstallRamKey(uint8 KeyIndex)
{
    Hal_StatusType Ret = STATUS_ERROR;

    uint8 SlotIndex;
    HSM_KeyHandleInfoType HandleInfo;

    Ret = HSM_Hal_IsRamKeyIndexExisted(KeyIndex, &SlotIndex);
    if (STATUS_SUCCESS == Ret)
    {
        Ret = HSM_Hal_GetRamKeyHandle(&HandleInfo, KeyIndex);
        if (STATUS_SUCCESS == Ret)
        {
            uint32 RetCode = Key_Remove(HandleInfo.KeyHandle, HandleInfo.AuthSize, HandleInfo.AuthValue);
            if (EHSM_ERR_SW_SUCCESS == RetCode)
            {
                RamKeyHead.KeyInfo[SlotIndex].Used = 0U;
                RamKeyHead.KeyInfo[SlotIndex].KeyIndex = 0xFFFFFFFFU;
                RamKeyHead.KeyNum--;
            }
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
        }
    }

    return Ret;
}

/*!
 * @brief adjust input keyid exist.
 *
 * @note  Function ID: DES_HSM_API_138
 * @param[in] KeyId: the index of key need to be check
 * @return Hal_StatusType:
 *         STATUS_ERROR: all key-slot are used-up;
 *         STATUS_SUCCESS: slotIndex is output successfull
 */
static Hal_StatusType HSM_Hal_IsKeyIdExistById(uint8 KeyId)
{
    Hal_StatusType Ret = STATUS_ERROR;
    HSM_FlashKeyPageType const *KeyPage = NULL_PTR;
    uint32 SlotId;
    uint8 PageIndex = 0U;

    Ret = HSM_Hal_FindValidKeyPage(&PageIndex);
    if (STATUS_SUCCESS == Ret)
    {
        KeyPage = (HSM_FlashKeyPageType *)(HSM_FLASH_KEY_BASE + ((uint32)PageIndex * DFLASH_PAGE_SIZE));
        for (SlotId = 0U; SlotId < HSM_FLASH_KEY_MAX_SLOT_NUM; SlotId++)
        {
            Ret = HSM_Hal_IsKeySlotValid(SlotId, PageIndex);
            if (STATUS_SUCCESS == Ret)
            {
                /* if keyId is exist and valid will return success */
                if (((HSM_KEY_DATA_VALID_TAG == KeyPage->KeyInfo[SlotId].IndexInfo.KeyValid)) &&
                        (KeyPage->KeyInfo[SlotId].IndexInfo.KeyIndex == KeyId))
                {
                    break;
                }
                else
                {
                    Ret = STATUS_ERROR;
                }
            }
        }
    }

    return Ret;
}

/*!
 * @brief Initial Flash Key Store Page function.
 * @note  Function ID: DES_HSM_API_139
 * @param[in] KeyId: Keyhandle from hsm
 * @param[in] KeyAuth: access keyhandle authentication value
 * @param[in] KeyAuthSize: authentication value size
 * @param[in] KeyIndex: customer define, reference HSM_KeyId
 * @return op status
 *         STATUS_ERROR: error happend;
 *         STATUS_SUCCESS: install flash key successfull.
 */
static Hal_StatusType HSM_Hal_InstallFlashKey(uint32 KeyId, const uint8 *KeyAuth, uint32 KeyAuthSize, uint8 KeyIndex)
{
    Hal_StatusType Ret;

    uint8 PageIndex = 0U;
    uint32 SlotIndex = 0U;
    HSM_KeyHandleInfoType KeyHandle;

    /*find page valid, pageindex  < 2*/
    Ret = HSM_Hal_FindValidKeyPage(&PageIndex);
    if (STATUS_SUCCESS == Ret)/* find page valid*/
    {
        Ret = HSM_Hal_IsKeyIdExistById(KeyIndex);
        if (Ret != STATUS_SUCCESS)
        {
            /*find KeyIndex already exist, must be delete it then to set the kid */
            /*find usable slot in valid key page*/
            Ret = HSM_Hal_FindUsableKeySlot(&SlotIndex, PageIndex);
            if (Ret != STATUS_SUCCESS)
            {
                 /*if key page not find usable slot, then adjust have invalid slot or not.*/
                 /*if no, valid key be full with all slot one page, not to install new key.if have invalid slot can install new key */
                 Ret = HSM_Hal_FindKeySlotInvalid(PageIndex);
                 if (STATUS_SUCCESS == Ret)
                 {
                     /* If no valid slot could be used, we update to backup keypage */
                     Ret = HSM_Hal_UpdateFlashKeyPage(&PageIndex);
                     if (STATUS_SUCCESS == Ret)
                     {
                         /* re-find usabe slot in new key page */
                         Ret = HSM_Hal_FindUsableKeySlot(&SlotIndex, PageIndex);
                     }
                 }
                 else
                 {
                      Ret = HSM_ALL_KEY_SPACE_OCCUPIED;
                 }
            }

            if ( STATUS_SUCCESS == Ret)
            {
                KeyHandle.KeyHandle = KeyId;
                KeyHandle.AuthSize = KeyAuthSize;
                (void)System_Memcpy(KeyHandle.AuthValue, KeyAuth, KeyAuthSize);
                Ret = HSM_Hal_SetupKeyData(PageIndex, SlotIndex, &KeyHandle, KeyIndex);
            }
            else
            {
                Ret = STATUS_ERROR;
            }
        }
        else
        {
           Ret = HSM_WRONG_KEY_HANDLE;
       }
    }
    else
    {
        Ret = STATUS_ERROR;
    }
    return Ret;
}

/*!
 * @brief uninstll Flash Key.
 * @note  Function ID: DES_HSM_API_140
 * @param[in] KeyId: customer define, reference HSM_KeyId
 * @return op status
 *         STATUS_ERROR: error happend;
 *         STATUS_SUCCESS: uninstall flash key successfull.
 */
static Hal_StatusType HSM_Hal_UnInstallFlashKey(uint8 KeyId)
{
    Hal_StatusType Ret;

    uint32 SlotIndex = 0U;
    uint8 PageIndex = 0U;
    HSM_FlashKeyPageType *KeyPagePtr;
    /*find valid page index*/
    Ret = HSM_Hal_FindValidKeyPage(&PageIndex);
    if (STATUS_SUCCESS == Ret)
    {
        KeyPagePtr = (HSM_FlashKeyPageType *)(HSM_FLASH_KEY_BASE + ((uint32)PageIndex*DFLASH_PAGE_SIZE));
        if (HSM_Hal_FindKeySlotByIndex(&SlotIndex, (uint32)PageIndex, KeyId) == STATUS_SUCCESS)
        {
            uint32 RetCode = Key_Remove(KeyPagePtr->KeyInfo[SlotIndex].HandleInfo.KeyHandle,
                            KeyPagePtr->KeyInfo[SlotIndex].HandleInfo.AuthSize,
                        KeyPagePtr->KeyInfo[SlotIndex].HandleInfo.AuthValue);
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
            if (STATUS_SUCCESS == Ret)
            {
                /*invalid key slot*/
                Ret = HSM_Hal_InvalidKeySlot(PageIndex, SlotIndex);
            }
        }
        else
        {
            Ret = HSM_WRONG_KEY_HANDLE;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

/*!
 * @brief get flash key handle function.
 * @note  Function ID: DES_HSM_API_141
 * @param [out] Handle: key handle pointer.
 * @param [in] KeyId: keyindex index.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: get key handle successfull.
 */
static Hal_StatusType HSM_Hal_GetFlashKeyHandle(HSM_KeyHandleInfoType *Handle, uint8 KeyId)
{
    Hal_StatusType Ret = STATUS_ERROR;
    uint8 PageIndex;
    uint32 SlotIndex;

    HSM_FlashKeyPageType const *KeyPage = NULL_PTR;

    Ret = HSM_Hal_FindValidKeyPage(&PageIndex);
    if (STATUS_SUCCESS == Ret)
    {
        KeyPage = (HSM_FlashKeyPageType *)(HSM_FLASH_KEY_BASE + ((uint32)PageIndex * DFLASH_PAGE_SIZE));
        Ret = HSM_Hal_FindKeySlotByIndex(&SlotIndex, PageIndex, KeyId);
        if (STATUS_SUCCESS == Ret)
        {
            Handle->KeyHandle= KeyPage->KeyInfo[SlotIndex].HandleInfo.KeyHandle;
            Handle->AuthSize= KeyPage->KeyInfo[SlotIndex].HandleInfo.AuthSize;
            (void)System_Memcpy(Handle->AuthValue, KeyPage->KeyInfo[SlotIndex].HandleInfo.AuthValue,
                KeyPage->KeyInfo[SlotIndex].HandleInfo.AuthSize);
        }
    }

    return Ret;
}

/*!
 * @brief get key handle function.
 * @note  Function ID: DES_HSM_API_142
 * @param [out] Handle: key handle pointer.
 * @param [in] KeyId: keyindex index.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: get key handle successfull.
 */
static Hal_StatusType HSM_Hal_GetKeyHandleInfo(HSM_KeyHandleInfoType *HandleInfo, HSM_KeyId KeyId)
{
    Hal_StatusType Ret = STATUS_ERROR;
    /*adjust key type by keyindex*/
    if (KeyId <= HSM_FLASH_KEY_RSA_3)  /*flash key, find flash key*/
    {
        Ret = HSM_Hal_GetFlashKeyHandle(HandleInfo, (uint8)KeyId);
    }
    else
    {
        Ret = HSM_Hal_GetRamKeyHandle(HandleInfo, (uint8)KeyId);
    }

    return Ret;
}

/*!
 * @brief get key algo type
 * @note  Function ID: DES_HSM_API_144
 * @param [in] KeyAlgo: key generate algo.
 * @return HSM_KeyAlgoType
 */
static HSM_KeyAlgoType HSM_Hal_AlgoIdToType(HSM_GenKeyAlgo KeyAlgo)
{
    HSM_KeyAlgoType AlgoType = HSM_ALGO_TYPE_NONE;
    switch (KeyAlgo)
    {
    case HSM_ALG_RANDOM:
        {
            AlgoType = HSM_ALGO_TYPE_RANDOM; /* not dhkey */
        }
        break;
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_AES
    case HSM_ALG_AES_128:
    case HSM_ALG_AES_256:
        {
            AlgoType = HSM_ALGO_TYPE_SYM; /* not dhkey */
        }
        break;
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_SM4
    case HSM_ALG_SM4:
        {
            AlgoType = HSM_ALGO_TYPE_SYM; /* not dhkey */
        }
        break;
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_SM2
    case HSM_ALG_SM2:
        {
            AlgoType = HSM_ALGO_TYPE_SM2;
        }
        break;
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_ECC
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_SECP256R1
    case HSM_ALG_SECP256R1:
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_SECP384R1
    case HSM_ALG_SECP384R1:
#endif
        {
            AlgoType = HSM_ALGO_TYPE_ECC; /* support dhkey */
        }
        break;
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_1024
    case HSM_ALG_RSA_1024:
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_2048
    case HSM_ALG_RSA_2048:
#endif
        {
            AlgoType = HSM_ALGO_TYPE_RSA_COMM; /* support dhkey */
        }
        break;
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_1024_CRT
    case HSM_ALG_RSA_1024_CRT:
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_2048_CRT
    case HSM_ALG_RSA_2048_CRT:
#endif
        {
            AlgoType = HSM_ALGO_TYPE_RSA_CRT;
        }
        break;
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_DH
    case HSM_ALG_DH:
        {
            AlgoType = HSM_ALGO_TYPE_DH; /* support dhkey */
        }
        break;
#endif
    default:
        AlgoType = HSM_ALGO_TYPE_NONE;
        break;
    }
    return AlgoType;
}

/*!
 * @brief uninstall ram ro flash key.
 * @note  Function ID: DES_HSM_API_145
 * @param [in] KeyId: key id.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: get key handle successfull.
 */
static Hal_StatusType HSM_Hal_UninstallKey(HSM_KeyId KeyId)
{
    Hal_StatusType Ret;

    /* remove ram key*/
    if ((KeyId >= HSM_RAM_KEY_SYM_0) && (KeyId <= HSM_RAM_KEY_RSA_3))
    {
        Ret = HSM_Hal_UninstallRamKey((uint8)KeyId);
    }
    else if (KeyId <= HSM_FLASH_KEY_RSA_3)
    {
        /* remove flash key*/
        Ret = HSM_Hal_UnInstallFlashKey((uint8)KeyId);
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

/*!
 * @brief copy rsa e param
 * @note  Function ID: DES_HSM_API_146
 * @param [in] Dst: dest buffer.
 * @param [in] Src: inpurt buffer.
 * @param [in] Sz: dest buffer size.
 * @return void
 */
static void HSM_Hal_RsaECopy(uint8 *Dst, const uint8 *Src, uint32 Sz)
{
    uint32 Mod;
    Mod = Sz % 4U;
    uint32 Index = 0U;
    if (0U != Mod)
    {
        Index = 4U - Mod;
       (void) System_Memcpy(&Dst[Index], Src, Sz);
    }
    else
    {
        (void)System_Memcpy(Dst, Src, Sz);
    }
}

/*!
 * @brief get hash algo output digest size
 * @note  Function ID: DES_HSM_API_147
 * @param [in] HashAlg: hash algo.
 * @return uint8 digest length.
 */
static uint8 HSM_Hal_GetHashSize(HSM_HashAlgoType HashAlg)
{
    uint8 BlockSize = 0U;

    switch(HashAlg)
    {
    case HSM_SHA1:
        BlockSize = 20U;
        break;
    case HSM_SHA224:
    case HSM_SHA512_224:
        BlockSize = 28U;
        break;
    case HSM_SM3:
    case HSM_SHA256:
    case HSM_SHA512_256:
        BlockSize = 32U;
        break;
    case HSM_SHA384:
        BlockSize = 48U;
        break;
    case HSM_SHA512:
        BlockSize = 64U;
        break;
    default:
        BlockSize = 0U;
        break;
    }

    return BlockSize;
}

static Hal_StatusType HSM_Hal_ErrorCodeCov(uint32 Ret)
{
    Hal_StatusType ErrCode = STATUS_SUCCESS;
    if ((Ret < 0x21U) && (Ret > 0x01U))
    {
        /*PRQA S 4394 ++ #Isn't out of Hal_StatusType range.*/
        ErrCode = (Hal_StatusType)(Ret + 0x90U);
        /*PRQA S 4394 -- #Isn't out of Hal_StatusType range.*/
    }
    else
    {
        if (Ret != EHSM_ERR_SW_SUCCESS)
        {
            ErrCode = STATUS_ERROR;
        }
    }

    return ErrCode;
}

/*!
 * @brief symmetric algo cipher function.
 * @note  Function ID: DES_HSM_API_150
 * @param [in] CfgPtr: symmetric Algo config
 * @param [in] InOutPtr: Input and output config.
 *             InBufLen: Message length. OnePass Mode will <= 1024 bytes.
 * @param [in] ProMode: process mode.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: get key handle successfull.
 */
/*PRQA S 4322 EOF #allow conversion between enum and enum.*/
static Hal_StatusType HSM_Hal_SymCipher(const HSM_SymCfgType *CfgPtr, const HSM_InOutType* InOutPtr,
                                                    HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret;

    uint32 OutputDataSize;

    uint32 IvSize;
    HSM_KeyHandleInfoType HandleInfo;

    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
    ehsm_ctx_session_st *session_handle = Hsm_Session;
    /*PRQA S 0310 -- #make sure there are no alignment issues.*/

    if (TRUE == CfgPtr->Async)
    {
        /* open mailbox path */
        HOST2HSM_ACCESS_PATH |= (1U << 1U);
        /* enable hsm interrupt */
        HSM->MB_HSM2SOC_SOC_INT_EN = 0x01U;
        /* close mailbox path */
        HOST2HSM_ACCESS_PATH &= ~(1U << 1U);
    }

    if (ECB_MODE != CfgPtr->CMode)
    {
        IvSize = 16U;
    }
    else
    {
        IvSize = 0U;
    }
    /*adjust key type by keyindex*/
    Ret = HSM_Hal_GetKeyHandleInfo(&HandleInfo, CfgPtr->KeyId);
    if (STATUS_SUCCESS == Ret)
    {
        uint32 RetCode;
        switch(ProMode)
        {
        case START_MODE:
            RetCode = Cipher_Init((uint32)CfgPtr->SymAlgo, (cipher_mode_e)CfgPtr->CipherDir,
                    (operation_mode_e)CfgPtr->CMode,(padding_scheme_e)CfgPtr->Padding,
                    InOutPtr->InBufLen, IvSize, CfgPtr->Iv, HandleInfo.KeyHandle,
                  HandleInfo.AuthSize, HandleInfo.AuthValue, session_handle);
            break;
        case UPDATE_MODE:
            if (0U == (InOutPtr->InBufLen % 16U))
            {
                RetCode = Cipher_Process(session_handle, InOutPtr->InBufLen, InOutPtr->InBuf,
                        &OutputDataSize, InOutPtr->OutBuf);
                *InOutPtr->OutBufLen = OutputDataSize;
            }
            else
            {
                RetCode = HSM_WRONG_CHUNK_SIZE;
            }
            break;
        case FINISH_MODE:
            if ((HSM_NOPADDING == CfgPtr->Padding) && (0u != (InOutPtr->InBufLen % 16U)))
            {
                RetCode = HSM_WRONG_CHUNK_SIZE;
            }
            else
            {
                RetCode = Cipher_Finish(session_handle, &OutputDataSize, InOutPtr->OutBuf);
                if (EHSM_ERR_SW_SUCCESS == RetCode)
                {
                    *InOutPtr->OutBufLen =  OutputDataSize;
                }
            }
            break;
        case FW_ONEPASS_MODE:
            TotalOuputSize = 0U;
            RetCode = Cipher_Init((uint32)CfgPtr->SymAlgo, (cipher_mode_e)CfgPtr->CipherDir,
                (operation_mode_e)CfgPtr->CMode,(padding_scheme_e)CfgPtr->Padding, InOutPtr->InBufLen,
                    IvSize, CfgPtr->Iv, HandleInfo.KeyHandle,
                              HandleInfo.AuthSize, HandleInfo.AuthValue, session_handle);
            if (EHSM_ERR_SW_SUCCESS == RetCode)
            {
                RetCode = Cipher_Process(session_handle, InOutPtr->InBufLen, InOutPtr->InBuf,
                            &OutputDataSize, InOutPtr->OutBuf);
                if (EHSM_ERR_SW_SUCCESS == RetCode)
                {
                    TotalOuputSize += OutputDataSize;
                    RetCode = Cipher_Finish(session_handle, &OutputDataSize, InOutPtr->OutBuf);
                    if (EHSM_ERR_SW_SUCCESS == RetCode)
                    {
                        *InOutPtr->OutBufLen = TotalOuputSize + OutputDataSize;
                    }
                }
            }
            break;
        default:
            RetCode = EHSM_ERR_GENERAL_ERROR;
            break;
        }
        Ret = HSM_Hal_ErrorCodeCov(RetCode);
    }
    return Ret;
}

/*!
 * @brief asymmetric Algo signature.
 * @note  Function ID: DES_HSM_API_151
 * @param [in] CfgPtr: asymmetric Algo config
 *              SaltLen: only for RSA PSS used.the Salt length value must <= 128 bytes and Salt length + Hash digest length must < RSA Key.n length(moudle).RSA1024 n length = 128, RSA2048 n length = 256
 * @param [in] InOutPtr: Input and output config.
 * @param [in] ProMode: process mode.
 * @return Hal_StatusType
 *         STATUS_ERROR: error happend ;
 *         STATUS_SUCCESS: get key handle successfull.
 */
static Hal_StatusType HSM_Hal_PkeSign(const HSM_AsymCfgType *CfgPtr, const HSM_InOutSignType *InOutPtr,
        HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
    ehsm_ctx_session_st *SessionHandle = Hsm_Session;
    /*PRQA S 0310 -- #make sure there are no alignment issues.*/

    signature_st v_out[1];
    uint8 SaltBuf[SALT_MAX_LEN] = {0U};
    uint8 SaltLen = 0U;
    uint32 RetCode = 0U;
    ehsm_utc_time_t utc_time;
    HSM_KeyHandleInfoType HandleInfo;
    /* PSASSA_PSS Padding, need to fill salt value */
    if ((HSM_ASYM_RSA == CfgPtr->AsymAlgo) && (HSM_PSASSA_PSS == CfgPtr->Padding)
        && ((FW_ONEPASS_MODE == ProMode) || (FINISH_MODE == ProMode)))
    {
        if (NULL_PTR == CfgPtr->SaltLen)
        {
            SaltLen = HSM_Hal_GetHashSize(CfgPtr->HAlgo);
        }
        else
        {
            SaltLen = *CfgPtr->SaltLen;
        }

        RetCode = RNG_Get_Random((uint32)EHSM_AES_CTRDRBG, (uint32)SaltLen, SaltBuf);
        Ret = HSM_Hal_ErrorCodeCov(RetCode);
    }

    if (STATUS_SUCCESS == Ret)
    {
        Ret = HSM_Hal_GetKeyHandleInfo(&HandleInfo, CfgPtr->KeyId);
        if (STATUS_SUCCESS == Ret)
        {
            switch (ProMode)
            {
            case START_MODE:
                if (HSM_SIGN == CfgPtr->SignDir)
                {
                    RetCode = Sign_Init((ehsm_asym_alg_e)CfgPtr->AsymAlgo, (uint32)CfgPtr->HAlgo,\
                                    (padding_scheme_e)CfgPtr->Padding, \
                                    InOutPtr->BasicInOut.InBufLen, 0, HandleInfo.KeyHandle, HandleInfo.AuthSize, \
                                    HandleInfo.AuthValue, SessionHandle);
                }
                else if (HSM_VERIFY == CfgPtr->SignDir)
                {
                    (void)System_Memcpy(v_out->signature, InOutPtr->SignInBuf, InOutPtr->SignInBufLen);
                    v_out->signature_size = InOutPtr->SignInBufLen;
                    RetCode = Verify_Init((ehsm_asym_alg_e)CfgPtr->AsymAlgo, (uint32)CfgPtr->HAlgo,
                                (padding_scheme_e)CfgPtr->Padding, InOutPtr->BasicInOut.InBufLen,
                        HandleInfo.KeyHandle, HandleInfo.AuthSize, HandleInfo.AuthValue, v_out, SessionHandle);
                }
                else
                {
                    RetCode = EVITA_GENERAL_ERROR;
                }
                break;
            case UPDATE_MODE:
                if (HSM_SIGN == CfgPtr->SignDir)
                {
                    RetCode = Sign_Update(SessionHandle, InOutPtr->BasicInOut.InBufLen, InOutPtr->BasicInOut.InBuf);
                }
                else if (HSM_VERIFY == CfgPtr->SignDir)
                {
                    RetCode = Verify_Update(SessionHandle, InOutPtr->BasicInOut.InBufLen, InOutPtr->BasicInOut.InBuf);
                }
                else
                {
                    RetCode = EVITA_GENERAL_ERROR;
                }
                break;
            case FINISH_MODE:
                    if (HSM_SIGN == CfgPtr->SignDir)
                    {
                        RetCode = Sign_Finish(SessionHandle, v_out, (uint32)SaltBuf, SaltLen);
                        if (EHSM_ERR_SW_SUCCESS == RetCode)
                        {
                            if (((CfgPtr->AsymAlgo == HSM_ASYM_ECDSA) && ((v_out->signature_size == 96u)
                                    || (v_out->signature_size == 64u)))
                                    || ((CfgPtr->AsymAlgo == HSM_ASYM_SM2) && (v_out->signature_size == 64u))
                                    || ((CfgPtr->AsymAlgo == HSM_ASYM_RSA) &&
                                    ((v_out->signature_size == 128u) || (v_out->signature_size == 256u))))
                            {
                                (void)System_Memcpy(InOutPtr->BasicInOut.OutBuf, v_out->signature,
                                                 v_out->signature_size);
                                *InOutPtr->BasicInOut.OutBufLen = v_out->signature_size;
                            }
                        }
                    }
                    else if (HSM_VERIFY == CfgPtr->SignDir)
                    {
                        RetCode = Verify_Finish(SessionHandle, &utc_time, (ehsm_bool_t *)InOutPtr->Vry,
                                                    (uint32)SaltBuf, SaltLen);
                    }
                    else
                    {
                        RetCode = EHSM_ERR_GENERAL_ERROR;
                    }
                break;
            case FW_ONEPASS_MODE:
                    if (HSM_SIGN == CfgPtr->SignDir)
                    {
                        RetCode = Sign_Init((ehsm_asym_alg_e)CfgPtr->AsymAlgo, (uint32)CfgPtr->HAlgo,\
                                        (padding_scheme_e)CfgPtr->Padding, \
                                        InOutPtr->BasicInOut.InBufLen, 0U, HandleInfo.KeyHandle, HandleInfo.AuthSize, \
                                        HandleInfo.AuthValue, SessionHandle);
                        if (EHSM_ERR_SW_SUCCESS == RetCode)
                        {
                            RetCode = Sign_Update(SessionHandle, InOutPtr->BasicInOut.InBufLen,
                                                    InOutPtr->BasicInOut.InBuf);
                            if (EHSM_ERR_SW_SUCCESS == RetCode)
                            {
                                RetCode = Sign_Finish(SessionHandle, v_out, (uint32)SaltBuf, SaltLen);
                                if (EHSM_ERR_SW_SUCCESS == RetCode)
                                {
                                    if (((CfgPtr->AsymAlgo == HSM_ASYM_ECDSA) && ((v_out->signature_size == 96u)
                                            || (v_out->signature_size == 64u)))
                                            || ((CfgPtr->AsymAlgo == HSM_ASYM_SM2) && (v_out->signature_size == 64u))
                                            || ((CfgPtr->AsymAlgo == HSM_ASYM_RSA) &&
                                            ((v_out->signature_size == 128u) || (v_out->signature_size == 256u))))
                                    {
                                        (void)System_Memcpy(InOutPtr->BasicInOut.OutBuf,
                                                v_out->signature, v_out->signature_size);
                                        *InOutPtr->BasicInOut.OutBufLen = v_out->signature_size;
                                    }
                                }
                            }
                        }
                    }
                    else if (HSM_VERIFY == CfgPtr->SignDir)
                    {
                        (void)System_Memcpy(v_out->signature, InOutPtr->SignInBuf, InOutPtr->SignInBufLen);
                        v_out->signature_size = InOutPtr->SignInBufLen;
                        RetCode = Verify_Init((ehsm_asym_alg_e)CfgPtr->AsymAlgo, (uint32)CfgPtr->HAlgo,
                                    (padding_scheme_e)CfgPtr->Padding, InOutPtr->BasicInOut.InBufLen,
                            HandleInfo.KeyHandle, HandleInfo.AuthSize, HandleInfo.AuthValue, v_out, SessionHandle);
                        if (EHSM_ERR_SW_SUCCESS == RetCode)
                        {
                            RetCode = Verify_Update(SessionHandle,
                                     InOutPtr->BasicInOut.InBufLen, InOutPtr->BasicInOut.InBuf);
                            if (EHSM_ERR_SW_SUCCESS == RetCode)
                            {
                                RetCode = Verify_Finish(SessionHandle, &utc_time, (ehsm_bool_t *)InOutPtr->Vry,
                                                (uint32)SaltBuf, SaltLen);
                            }
                        }
                    }
                    else
                    {
                        RetCode = EHSM_ERR_GENERAL_ERROR;
                    }
                break;
            default:
                RetCode = EHSM_ERR_GENERAL_ERROR;
                break;
            }
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
        }
        else
        {
            Ret = HSM_WRONG_KEY_HANDLE;
        }
    }

    return Ret;
}

/*!
 * @brief Rsa cipher function.
 * @note  Function ID: DES_HSM_API_152
 * @param [in] CfgPtr: asymmetric Algo config
 * @param [in] InOutPtr: Input and output config.
 * @param [in] HandleInfo: keyhandle info.
 * @param [in] CrtMode: crt mode.
 * @return op status
 */
static uint32 HSM_Hal_RsaCipherOnePass(const HSM_AsymCfgType *CfgPtr, const HSM_InOutType *InOutPtr,
        const HSM_KeyHandleInfoType *HandleInfo, uint8 CrtMode)
{
    uint32 cmdBuf[11] = {0};
    uint32 ret;

    cmdBuf[0U] = 0xFAFE0501U;
    cmdBuf[3U] = ((uint32)CrtMode) |((uint32)CfgPtr->CipherDir<< 8U);
    cmdBuf[5U] = HandleInfo->KeyHandle;
    cmdBuf[6U] = (uint32)HandleInfo->AuthValue;
    cmdBuf[7U] = HandleInfo->AuthSize;
    cmdBuf[8U] = (uint32)InOutPtr->InBuf;
    cmdBuf[9U] = InOutPtr->InBufLen;
    cmdBuf[10U] = (uint32)InOutPtr->OutBuf;

    ret = HSM_Reg_TrigCmd(cmdBuf, 11U);

    HSM_Reg_GetReturnDataCode(InOutPtr->OutBufLen, 1U);

    return ehsm_evita_convert_ret_code(ret);
}

/*!
 * @brief
 * @note  Function ID: DES_HSM_API_153
 * @param [in] CfgPtr: asymmetric Algo config
 * @param [in] InOutPtr: Input and output config.
 * @param [in] HandleInfo: keyhandle info.
 * @return op status
 */
static uint32 HSM_Hal_Sm2CipherOnePass(const HSM_AsymCfgType *CfgPtr, const HSM_InOutType *InOutPtr,
        const HSM_KeyHandleInfoType *HandleInfo)
{
    uint32 cmdBuf[11] = {0};
    uint32 Ret = 0U;
    cmdBuf[0U] = 0xFBFE0401U;
    cmdBuf[3U] = (uint32)CfgPtr->CipherDir<< 8U;
    cmdBuf[5U] = HandleInfo->KeyHandle;
    cmdBuf[6U] = (uint32)HandleInfo->AuthValue;
    cmdBuf[7U] = HandleInfo->AuthSize;
    cmdBuf[8U] = (uint32)InOutPtr->InBuf;
    cmdBuf[9U] = InOutPtr->InBufLen;
    cmdBuf[10U] = (uint32)InOutPtr->OutBuf;

    Ret = HSM_Reg_TrigCmd(cmdBuf, 11U);

    return ehsm_evita_convert_ret_code(Ret);
}

static Hal_StatusType HSM_Hal_InitHsmFw(boolean HsmIrqEn, uint8 HsmIrqPri)
{
    Hal_StatusType Ret = STATUS_TIMEOUT;

    uint32 Timeout = 50000U;
    uint32 MicrTick = OsIf_MicrosToTicks(Timeout);
    uint32 CurrentCounter = OsIf_GetCounter();
    uint32 ElapseTick = 0U;

    while(MicrTick > ElapseTick)
    {
        ElapseTick += OsIf_GetElapsed(&CurrentCounter);
        /*wait hsm fw ready*/
        if (((HSM_Reg_GetHsmStatus0()) & 0xFFU) == 0x15U)
        {
            (void)ehsm_service_init();
            (void)System_Memset(Hsm_Key, 0U, sizeof(Hsm_Key));
            (void)System_Memset(Hsm_Session, 0U, sizeof(Hsm_Session));

            /*Enable hsm interrupt*/
            if (HsmIrqEn == TRUE)
            {
                /* Clear the pending for avoiding jump to the handler immediately */
                Core_Hal_ClearPendingIrq(HSM_IRQn);
                /* Set priority. The priority is 0 to 15, 0 is the higest priority */
                Core_Hal_SetIrqPriority(HSM_IRQn, HsmIrqPri);
                /* Enable the IRQ */
                Core_Hal_EnableIrq(HSM_IRQn);
            }

            /* init ram key space */
            HSM_Hal_InitRamKeyMem();

            Ret = HSM_Hal_InitFlashKeyPage();
            break;
        }
    }

    return Ret;
}

void HSM_Hal_Lock(void)
{
    HOST2HSM_ACCESS_PATH &= ~(1U << 1U);
}

void HSM_Hal_Unlock(void)
{
    HOST2HSM_ACCESS_PATH |= (1U << 1U);
}

boolean HSM_Hal_GetLockState(void)
{
    boolean LockState = FALSE;

    if (((HOST2HSM_ACCESS_PATH >> 1U) & 0x01U) == 1U)
    {
        LockState = TRUE;
    }

    return LockState;
}

Hal_StatusType HSM_Hal_Init(boolean HsmIrqEn, uint8 HsmIrqPri)
{
    Hal_StatusType Ret = STATUS_ERROR;

    uint32 Timeout;
    uint32 MicrTick;
    uint32 CurrentCounter;
    uint32 ElapseTick = 0U;

    /*adjust hsm firmware update status*/
    if (0x01U == ((HSM_Reg_GetHsmStatus1() & HSM_STATUS1_FUK_Msk) >> HSM_STATUS1_FUK_Pos))
    {
        /*timeout value is 20s*/
        Timeout = 20000000U;
        MicrTick = OsIf_MicrosToTicks(Timeout);
        CurrentCounter = OsIf_GetCounter();
        while(MicrTick > ElapseTick)
        {
            ElapseTick += OsIf_GetElapsed(&CurrentCounter);
            /* hsm firmware update completed*/
            if (0x0U == ((HSM_Reg_GetHsmStatus1() & HSM_STATUS1_FUK_Msk) >> HSM_STATUS1_FUK_Pos))
            {
                /* hsm firmware update success*/
                if (0x0U == (((HSM_Reg_GetHsmStatus1()& HSM_STATUS1_FUD_Msk) >> HSM_STATUS1_FUD_Pos)) &&
                    (0x1U == ((HSM_Reg_GetHsmStatus1()& HSM_STATUS1_FUE_Msk) >> HSM_STATUS1_FUE_Pos)))
                {
                    /*Init Hsm Fw*/
                    Ret = HSM_Hal_InitHsmFw(HsmIrqEn, HsmIrqPri);
                }
                else
                {
                    Ret = HSM_FIRMWARE_UPDATE_FAIL;
                }
            }
        }
    }
    else
    {
        Ret = HSM_Hal_InitHsmFw(HsmIrqEn, HsmIrqPri);
    }

    return Ret;
}

void HSM_Hal_Deinit(void)
{
    /* disable the IRQ */
    Core_Hal_DisableIrq(HSM_IRQn);
    /* Clear the pending for avoiding jump to the handler immediately */
    Core_Hal_ClearPendingIrq(HSM_IRQn);
}

Hal_StatusType HSM_Hal_GetRnd(HSM_RndAlgo Algo, uint32 RndSize, uint8 *Rnd)
{
    Hal_StatusType Ret;

    uint32 RetCode = RNG_Get_Random((uint32)Algo, RndSize, Rnd);
    Ret = HSM_Hal_ErrorCodeCov(RetCode);

    return Ret;
}

Hal_StatusType HSM_Hal_AesCipher(const HSM_SymCfgType *CfgPtr, const HSM_InOutType* InOutPtr, HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret;
    if ((CfgPtr!=NULL_PTR) && (InOutPtr != NULL_PTR))
    {
        if ((CfgPtr->SymAlgo == HSM_AES_128) || (CfgPtr->SymAlgo == HSM_AES_256))
        {
            Ret = HSM_Hal_SymCipher(CfgPtr, InOutPtr, ProMode);
        }
        else
        {
            Ret = HSM_WRONG_ALGO_TYPE;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_Sm4Cipher(const HSM_SymCfgType *CfgPtr, const HSM_InOutType* InOutPtr, HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret;

    if ((CfgPtr != NULL_PTR) && (InOutPtr != NULL_PTR))
    {
        if (CfgPtr->SymAlgo == HSM_SM4)
        {
            Ret = HSM_Hal_SymCipher(CfgPtr, InOutPtr, ProMode);
        }
        else
        {
            Ret = HSM_WRONG_ALGO_TYPE;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_CipherMac(const HSM_CMacCfgType *CfgPtr, HSM_InOutMacType *InOutPtr, HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret;
    uint32 MacSize = 0U;
    HSM_KeyHandleInfoType HandleInfo;
    mac_st Mac[1];
    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
    ehsm_ctx_session_st *SessionHandle = Hsm_Session;
    /*PRQA S 0310 -- #make sure there are no alignment issues.*/
    if ((CfgPtr!=NULL_PTR) && (InOutPtr != NULL_PTR))
    {
        if (TRUE == CfgPtr->Async)
        {
            /* open mailbox path */
            HOST2HSM_ACCESS_PATH |= (1U << 1U);
            /* enable hsm interrupt */
            HSM->MB_HSM2SOC_SOC_INT_EN = 0x01U;
            /* close mailbox path */
            HOST2HSM_ACCESS_PATH &= ~(1U << 1U);
        }

        Ret = HSM_Hal_GetKeyHandleInfo(&HandleInfo, CfgPtr->KeyId);
        if (STATUS_SUCCESS == Ret)
        {
            uint32 RetCode;
            switch (ProMode)
            {
            case START_MODE:
                RetCode = MAC_Init((uint32)CfgPtr->SymAlgo, (mac_mode_e)CfgPtr->MacDir, EVITA_CMAC_MODE,
                               (padding_scheme_e)HSM_NOPADDING, InOutPtr->BasicInOut.InBufLen, 16, HandleInfo.KeyHandle,
                               HandleInfo.AuthSize, HandleInfo.AuthValue, SessionHandle);
                break;
            case UPDATE_MODE:
                RetCode = MAC_Update(SessionHandle, InOutPtr->BasicInOut.InBufLen, InOutPtr->BasicInOut.InBuf);
        
                break;
            case FINISH_MODE:
                if (CfgPtr->MacDir == HSM_MAC_VERIFY)
                {
                    (void)System_Memcpy(Mac->mac_value, InOutPtr->MacInBuf, InOutPtr->MacInBufLen);
                    InOutPtr->MacInBufLen = 16U;
                    MacSize = InOutPtr->MacInBufLen;
                    RetCode = MAC_Finish(SessionHandle, &MacSize, Mac, (ehsm_bool_t *)InOutPtr->Vry);
                }
                else
                {
                    RetCode = MAC_Finish(SessionHandle, &MacSize, Mac, NULL);
                    (void)System_Memcpy(InOutPtr->BasicInOut.OutBuf, Mac->mac_value, MacSize);
                    *InOutPtr->BasicInOut.OutBufLen = MacSize;
                }
                break;

            case FW_ONEPASS_MODE:
                RetCode = MAC_Init((uint32)CfgPtr->SymAlgo, (mac_mode_e)CfgPtr->MacDir, EVITA_CMAC_MODE,
                               (padding_scheme_e)HSM_NOPADDING, InOutPtr->BasicInOut.InBufLen, 16, HandleInfo.KeyHandle,
                               HandleInfo.AuthSize, HandleInfo.AuthValue, SessionHandle);
                if (EHSM_ERR_SW_SUCCESS == RetCode)
                {
                    RetCode = MAC_Update(SessionHandle, InOutPtr->BasicInOut.InBufLen, InOutPtr->BasicInOut.InBuf);
                    if (EHSM_ERR_SW_SUCCESS == RetCode)
                    {
                        if (CfgPtr->MacDir == HSM_MAC_VERIFY)
                        {
                            (void)System_Memcpy(Mac->mac_value, InOutPtr->MacInBuf, InOutPtr->MacInBufLen);
                            InOutPtr->MacInBufLen = 16U;
                            MacSize = InOutPtr->MacInBufLen;
                            RetCode = MAC_Finish(SessionHandle, &MacSize, Mac, (ehsm_bool_t *)InOutPtr->Vry);
                        }
                        else
                        {
                            RetCode = MAC_Finish(SessionHandle, &MacSize, Mac, NULL);
                            (void)System_Memcpy(InOutPtr->BasicInOut.OutBuf, Mac->mac_value, MacSize);
                            *InOutPtr->BasicInOut.OutBufLen = MacSize;
                        }
                    }
                }
                break;
            default:
                RetCode = EVITA_GENERAL_ERROR;
                break;
            }
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
        }
        else
        {
            Ret = HSM_WRONG_KEY_HANDLE;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_Hash(const HSM_HashAlgoType Algo, const HSM_InOutType *InOutPtr,  HSM_ProcessMode ProMode)
{
    Hal_StatusType Res;
    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
    ehsm_ctx_session_st *SessionHandle = Hsm_Session;
    /*PRQA S 0310 -- #make sure there are no alignment issues.*/

    hash_hmac_st Mac[1]= {0};

    uint32 HashLen = HSM_Hal_GetHashSize(Algo);

    if ((InOutPtr != NULL_PTR) || (ProMode == START_MODE))
    {
        uint32 RetCode;
        switch (ProMode)
        {
        case START_MODE:
            RetCode = Hash_Init((uint32)Algo, EVITA_HASH, 0U, 0U, NULL, SessionHandle);
            break;
        case UPDATE_MODE:
            RetCode = Hash_Update(SessionHandle, InOutPtr->InBufLen, InOutPtr->InBuf);
            break;
        case FINISH_MODE:
            Mac->hash_hmac_size = sizeof(Mac->hash_hmac);
            RetCode = Hash_Finish(SessionHandle, Mac, NULL);
            break;
        case FW_ONEPASS_MODE:
            RetCode = Hash_Init((uint32)Algo, EVITA_HASH, 0U, 0U, NULL, SessionHandle);
            if (EHSM_ERR_SW_SUCCESS == RetCode)
            {
                RetCode = Hash_Update(SessionHandle, InOutPtr->InBufLen, InOutPtr->InBuf);
                if (EHSM_ERR_SW_SUCCESS == RetCode)
                {
                    Mac->hash_hmac_size = sizeof(Mac->hash_hmac);
                    RetCode = Hash_Finish(SessionHandle, Mac, NULL);
                }
            }
            break;
        default:
            RetCode = EHSM_ERR_GENERAL_ERROR;
            break;
        }

        if ((EHSM_ERR_SW_SUCCESS == RetCode) && (ProMode > UPDATE_MODE))
        {
            *InOutPtr->OutBufLen = HashLen;
            (void)System_Memcpy(InOutPtr->OutBuf, Mac->hash_hmac, *InOutPtr->OutBufLen);
        }
        Res = HSM_Hal_ErrorCodeCov(RetCode);
    }
    else
    {
        Res = STATUS_ERROR;
    }

    return Res;
}

Hal_StatusType HSM_Hal_HashMac(const HSM_HMacCfgType *CfgPtr, const HSM_InOutMacType *InOutPtr, HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret;

    uint8 MacSize = 0U;
    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
    ehsm_ctx_session_st *SessionHandle = Hsm_Session;
    /*PRQA S 0310 -- #make sure there are no alignment issues.*/
    hash_hmac_st Mac[1U];

    HSM_KeyHandleInfoType HandleInfo;

    if ((CfgPtr!=NULL_PTR) && (InOutPtr != NULL_PTR))
    {
        MacSize = HSM_Hal_GetHashSize(CfgPtr->HAlgo);
        
        Ret = HSM_Hal_GetKeyHandleInfo(&HandleInfo, CfgPtr->KeyId);
        if (STATUS_SUCCESS == Ret)
        {
            uint32 RetCode;
            switch (ProMode)
            {
            case START_MODE:
                RetCode = Hash_Init((uint32)CfgPtr->HAlgo, (hash_mode_e)CfgPtr->MacDir, HandleInfo.KeyHandle,
                        HandleInfo.AuthSize, HandleInfo.AuthValue, SessionHandle);
                break;
            
            case UPDATE_MODE:
                RetCode = Hash_Update(SessionHandle, InOutPtr->BasicInOut.InBufLen, InOutPtr->BasicInOut.InBuf);
                break;
            
            case FINISH_MODE:
                if (CfgPtr->MacDir == HSM_MAC_GEN)
                {
                    RetCode = Hash_Finish(SessionHandle, Mac, NULL);
                    if (EHSM_ERR_SW_SUCCESS == RetCode)
                    {
                        (void)System_Memcpy(InOutPtr->BasicInOut.OutBuf, Mac->hash_hmac, MacSize);
                        *InOutPtr->BasicInOut.OutBufLen = MacSize;
                    }
                }
                else
                {
                    if (InOutPtr->MacInBufLen == MacSize)
                    {
                        (void)System_Memcpy(Mac->hash_hmac, InOutPtr->MacInBuf, MacSize);
                        RetCode = Hash_Finish(SessionHandle, Mac, (ehsm_bool_t *)InOutPtr->Vry);
                    }
                    else
                    {
                        RetCode = EHSM_ERR_GENERAL_ERROR;
                    }
                }
                break;
            
            case FW_ONEPASS_MODE:
                if (MacSize != 0U)
                {
                    RetCode = Hash_Init((uint32)CfgPtr->HAlgo, (hash_mode_e)CfgPtr->MacDir, HandleInfo.KeyHandle,
                                    HandleInfo.AuthSize, HandleInfo.AuthValue, SessionHandle);
                    if (EHSM_ERR_SW_SUCCESS == RetCode)
                    {
                        RetCode = Hash_Update(SessionHandle, InOutPtr->BasicInOut.InBufLen, InOutPtr->BasicInOut.InBuf);
                        if (EHSM_ERR_SW_SUCCESS == RetCode)
                        {
                            Mac->hash_hmac_size = sizeof(Mac->hash_hmac);
            
                            if (CfgPtr->MacDir == HSM_MAC_GEN)
                            {
                                RetCode = Hash_Finish(SessionHandle, Mac, NULL);
                                if (EHSM_ERR_SW_SUCCESS == RetCode)
                                {
                                    (void)System_Memcpy(InOutPtr->BasicInOut.OutBuf,Mac->hash_hmac, MacSize);
                                    *InOutPtr->BasicInOut.OutBufLen = MacSize;
                                }
                            }
                            else
                            {
                                if (InOutPtr->MacInBufLen == MacSize)
                                {
                                    (void)System_Memcpy(Mac->hash_hmac, InOutPtr->MacInBuf, MacSize);
                                    RetCode = Hash_Finish(SessionHandle, Mac, (ehsm_bool_t *)InOutPtr->Vry);
                                }
                                else
                                {
                                    RetCode = EHSM_ERR_GENERAL_ERROR;
                                }
                            }
                        }
                    }
                }
                else
                {
                    RetCode = EHSM_ERR_GENERAL_ERROR;
                }
                break;
            default:
                RetCode = EHSM_ERR_GENERAL_ERROR;
                break;
            }
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
        }
        else
        {
            Ret = HSM_WRONG_KEY_HANDLE;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_EccSign(const HSM_AsymCfgType *CfgPtr, const HSM_InOutSignType *InOutPtr,
                                        HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret;

    if ((CfgPtr!=NULL_PTR) && (InOutPtr != NULL_PTR))
    {
        Ret = HSM_Hal_PkeSign(CfgPtr, InOutPtr, ProMode);
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_RsaSign(const HSM_AsymCfgType *CfgPtr, const HSM_InOutSignType *InOutPtr,
                                HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret;

    if ((CfgPtr!=NULL_PTR) && (InOutPtr != NULL_PTR))
    {
        Ret = HSM_Hal_PkeSign(CfgPtr, InOutPtr, ProMode);
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_Sm2Sign(const HSM_AsymCfgType *CfgPtr, const HSM_InOutSignType *InOutPtr,
        HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret;
    if ((CfgPtr!=NULL_PTR) && (InOutPtr != NULL_PTR))
    {
        Ret = HSM_Hal_PkeSign(CfgPtr, InOutPtr, ProMode);
    }
    else
    {
        Ret = STATUS_ERROR;
    }
    return Ret;
}

Hal_StatusType HSM_Hal_RsaCipher(const HSM_AsymCfgType *CfgPtr, const HSM_InOutType *InOutPtr,
        HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    HSM_KeyHandleInfoType HandleInfo;

    (void)ProMode;
    if ((CfgPtr!=NULL_PTR) && (InOutPtr != NULL_PTR))
    {
        if (CfgPtr->Padding == HSM_NOPADDING)
        {
            /*NOPADDING: HSM Firmware will add zero at the beginning of the buffer in HSM*/
            if (((CfgPtr->AsymAlgo == HSM_ASYM_RSA_CIPHER_1024) && (InOutPtr->InBufLen > 128U))
                || ((CfgPtr->AsymAlgo == HSM_ASYM_RSA_CIPHER_2048) && (InOutPtr->InBufLen > 256U)))
            {
                Ret = HSM_WRONG_CHUNK_SIZE;
            }
        }
        else if (CfgPtr->Padding == HSM_PKCS1_V15)
        {
            if (((CfgPtr->AsymAlgo == HSM_ASYM_RSA_CIPHER_1024) && (InOutPtr->InBufLen > 117U))
                || ((CfgPtr->AsymAlgo == HSM_ASYM_RSA_CIPHER_2048) && (InOutPtr->InBufLen > 245U)))
            {
                Ret = HSM_WRONG_CHUNK_SIZE;
            }
            /*HSM_PKCS1_V15: software to padding*/
            //to do
        }
        else
        {
            Ret = STATUS_ERROR;
        }

        if (STATUS_SUCCESS == Ret)
        {
            Ret = HSM_Hal_GetKeyHandleInfo(&HandleInfo, CfgPtr->KeyId);
            if (STATUS_SUCCESS == Ret)
            {
                uint32 RetCode = HSM_Hal_RsaCipherOnePass(CfgPtr, InOutPtr, &HandleInfo, 0U);
                Ret = HSM_Hal_ErrorCodeCov(RetCode);
            }
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

/*!
 * @brief Sm2 Decrypt/Encrypt function.
 * @note  Function ID:
 * @param [in] CfgPtr: sm2 Algo config.
 * @param [in] InOutPtr: Input and output config.
 * @return op status
 */
Hal_StatusType HSM_Hal_Sm2Cipher(const HSM_AsymCfgType *CfgPtr,const HSM_InOutType *InOutPtr, HSM_ProcessMode ProMode)
{
    Hal_StatusType Ret;
    HSM_KeyHandleInfoType HandleInfo;

   (void)ProMode;
    if ((CfgPtr!=NULL_PTR) && (InOutPtr != NULL_PTR))
    {
        Ret = HSM_Hal_GetKeyHandleInfo(&HandleInfo, CfgPtr->KeyId);
        if (STATUS_SUCCESS == Ret)
        {
            uint32 RetCode = HSM_Hal_Sm2CipherOnePass(CfgPtr, InOutPtr, &HandleInfo);
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
            if (STATUS_SUCCESS == Ret)
            {
                if (CfgPtr->CipherDir == HSM_ENCRYPTION)
                {
                    *InOutPtr->OutBufLen = 97U + InOutPtr->InBufLen;
                }
                else
                {
                    if (InOutPtr->InBufLen > 97U)
                    {
                        *InOutPtr->OutBufLen = InOutPtr->InBufLen - 97U;
                    }
                    else
                    {
                        Ret = HSM_WRONG_CHUNK_SIZE;
                    }
                }
            }
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_RemoveKey(HSM_KeyId KeyId)
{
    Hal_StatusType Ret = STATUS_ERROR;

    Ret = HSM_Hal_UninstallKey(KeyId);

    return Ret;
}

void HSM_Hal_RemoveAllNvmKeyByHsm(void)
{
    uint8 DefaultAuthValue[32U] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
                                     0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (uint32 id = 65546U; id<65556U; id++)
    {
        (void)Key_Remove(id, 32U, DefaultAuthValue);
    }
}

Hal_StatusType HSM_Hal_SetPlainKey(const HSM_PlainKeyCfgType *CfgPtr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    HSM_KeyAlgoType AlgoType;
    HSM_KeyStorageType StoreType;
    uint32 KeyHandle;
    uint32 TempLen;
    HSM_RsaPubKeyType const *RsaPub = NULL;
    HSM_RsaPrvCrtType const *RsaPrvCrt = NULL;
    (void)System_Memset(&InterKey, 0x00U, sizeof(ehsm_internal_key_st));
    uint8 *imp_key_addr = (uint8 *)Hsm_Key;
    InterKey.attr.valid_util = CfgPtr->ValidUtil;
    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
    /*PRQA S 3305 ++ #make sure there are no alignment issues.*/
    InterKey.key_usage = (*(ehsm_key_usages_st *)CfgPtr->KeyUsages);
    /*PRQA S 3305 -- #make sure there are no alignment issues.*/
    /*PRQA S 0310 -- #make sure there are no alignment issues.*/
    InterKey.attr.key_usage_size = (uint16)(sizeof(ehsm_key_flags_element_st) * CfgPtr->KeyUsagesCnt);
    InterKey.attr.algo_id = (uint32)CfgPtr->KeyAlgo;

    AlgoType = HSM_Hal_AlgoIdToType(CfgPtr->KeyAlgo);

    if ((AlgoType == HSM_ALGO_TYPE_RANDOM) || (AlgoType == HSM_ALGO_TYPE_SYM))
    {
        /* Use EHSM_ALG_RANDOM for AES-XTS */
        if (CfgPtr->KeyAlgo == HSM_ALG_RANDOM)
        {
            InterKey.attr.key_info.storage_key_type = (uint32)EVITA_STORAGE_RANDOM_KEY_TYPE;
            InterKey.attr.key_info.storage_info.random_key_size = CfgPtr->RandomKeySize;
        }
        else
        {
            /*do nothing*/
        }
        (void)System_Memcpy(InterKey.prikey.sym_k, CfgPtr->PrivKey, CfgPtr->PrivKeyLen);
    }
    else
    {
        if ((AlgoType == HSM_ALGO_TYPE_RSA_CRT) || (AlgoType == HSM_ALGO_TYPE_RSA_COMM))
        {
            /* Use ext_param for passing the e_size */
            if (CfgPtr->ExtParam != 0u)
            {
                InterKey.attr.key_info.storage_key_type = (uint32)EVITA_STORAGE_RANDOM_KEY_TYPE;
                if ((CfgPtr->ExtParam % 4u) != 0u)
                {
                    InterKey.attr.key_info.storage_info.rsa_e_bytes_size =
                        CfgPtr->ExtParam + 4u - (CfgPtr->ExtParam % 4u);
                }
                else
                {
                    InterKey.attr.key_info.storage_info.rsa_e_bytes_size = CfgPtr->ExtParam;
                }
            }
            else
            {
                /*do nothing*/
            }
        }
        else
        {
            /*do nothing*/
        }
        if (NULL != CfgPtr->PrivKey)
        {

            /* this key raw data to interkey struct */
            switch (AlgoType)
            {
            case HSM_ALGO_TYPE_SM2:
            case HSM_ALGO_TYPE_ECC:
                (void)System_Memcpy(InterKey.prikey.ecc_k, CfgPtr->PrivKey, CfgPtr->PrivKeyLen);
                break;
            case HSM_ALGO_TYPE_DH:
                break;
            case HSM_ALGO_TYPE_RSA_CRT:
                TempLen = CfgPtr->PrivKeyLen / 5U;
                if ((CfgPtr->PrivKeyLen % 5U) == 0U)
                {
                    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
                    RsaPrvCrt = (HSM_RsaPrvCrtType *)CfgPtr->PrivKey;
                    /*PRQA S 0310 -- #make sure there are no alignment issues.*/
                    (void)System_Memcpy(InterKey.prikey.rsa_crt.p, RsaPrvCrt->p, TempLen);
                    (void)System_Memcpy(InterKey.prikey.rsa_crt.q, RsaPrvCrt->q, TempLen);
                    (void)System_Memcpy(InterKey.prikey.rsa_crt.dp, RsaPrvCrt->dp, TempLen);
                    (void)System_Memcpy(InterKey.prikey.rsa_crt.dq, RsaPrvCrt->dq, TempLen);
                    (void)System_Memcpy(InterKey.prikey.rsa_crt.u, RsaPrvCrt->u, TempLen);
                }
                else
                {
                    Ret = STATUS_ERROR;
                }
                break;
            case HSM_ALGO_TYPE_RSA_COMM:
                (void)System_Memcpy(InterKey.prikey.rsa_d, CfgPtr->PrivKey, CfgPtr->PrivKeyLen);
                break;
            default:
                    Ret = STATUS_ERROR;
                break;
            }
        }
        else
        {
            /*do nothing*/
        }
        if (NULL != CfgPtr->PubKey)
        {
            switch (AlgoType)
            {
            case HSM_ALGO_TYPE_SM2:
                /* pubkey include uncompress flag*/
                if (((CfgPtr->PubKeyLen%32U) != 0U) && (CfgPtr->PubKey[0U] == 0x04U))
                {
                    /*remove 0x04 for ECC Algo Key*/
                    (void)System_Memcpy(InterKey.pubkey.ecc.p, CfgPtr->PubKey, CfgPtr->PubKeyLen);
                }
                else
                {
                    Ret = HSM_INVALID_KEY_SIZE;
                }
                break;
            case HSM_ALGO_TYPE_ECC:
                /* pubkey include uncompress flag*/
                if (((CfgPtr->PubKeyLen%32U) != 0U) && (CfgPtr->PubKey[0U] == 0x04U))
                {
                    /*remove 0x04 for ECC Algo Key*/
                    (void)System_Memcpy(InterKey.pubkey.ecc.p, &CfgPtr->PubKey[1U], CfgPtr->PubKeyLen - 1U);
                }
                else
                {
                    Ret = HSM_INVALID_KEY_SIZE;
                }
                break;
            case HSM_ALGO_TYPE_DH:
                break;
            case HSM_ALGO_TYPE_RSA_CRT:
            case HSM_ALGO_TYPE_RSA_COMM:
                /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
                RsaPub = (HSM_RsaPubKeyType *)CfgPtr->PubKey;
               /*PRQA S 0310 -- #make sure there are no alignment issues.*/
                TempLen =  (CfgPtr->PubKeyLen / 128U) * 128U;

                (void)System_Memcpy(InterKey.pubkey.rsa.n, RsaPub->n, TempLen);

                HSM_Hal_RsaECopy(InterKey.pubkey.rsa.e, RsaPub->e, CfgPtr->PubKeyLen - TempLen);
                break;

            default:
                Ret = STATUS_ERROR;
                break;
            }
        }
        else
        {
            /*do nothing*/
        }
    }

    if (STATUS_SUCCESS == Ret)
    {
        (void)System_Memcpy(imp_key_addr, (uint8 *)&InterKey, sizeof(ehsm_internal_key_st));

        if (HSM_Hal_IsKeyIndexValid(CfgPtr->KeyId, AlgoType) == STATUS_SUCCESS) /* adjust keyindex valid*/
        {
            StoreType = HSM_Hal_GetKeyStoreTypeByKeyId(CfgPtr->KeyId);

            /* Ram Key*/
            if (StoreType == EHSM_EVITA_KEY_TYPE_RAM)
            {
                /* ram key exist already, delete it*/
                if (HSM_Hal_IsRamKeyIndexExisted((uint8)CfgPtr->KeyId, NULL_PTR) == STATUS_SUCCESS)
                {
                    Ret = HSM_Hal_UninstallRamKey((uint8)CfgPtr->KeyId);
                }
            }
            else
            {
                /* flash key exist already, not install again, need user to delete then install it */
                if (HSM_Hal_IsKeyIdExistById((uint8)CfgPtr->KeyId) == STATUS_SUCCESS)
                {
                    Ret = HSM_WRONG_KEY_HANDLE;
                }
                else
                {
                    Ret = STATUS_SUCCESS;
                }
            }
            if (Ret == STATUS_SUCCESS)
            {
                /* should adjust keyid exist or not when import key */
                uint32 RetCode = Key_Import(0U, 0U, NULL, 0U, 0U, NULL, (ehsm_key_mem_type_e)StoreType,
                                 sizeof(ehsm_internal_key_st),(uint8 *)imp_key_addr,
                                 CfgPtr->AuthValueSize, CfgPtr->AuthValue, &KeyHandle);
                Ret = HSM_Hal_ErrorCodeCov(RetCode);
                if (STATUS_SUCCESS == Ret)
                {
                    if (HSM_KEY_TYPE_NVM == StoreType)
                    {
                        /* install key handle info to dflash */
                        Ret = HSM_Hal_InstallFlashKey(KeyHandle, CfgPtr->AuthValue,
                            CfgPtr->AuthValueSize, (uint8)CfgPtr->KeyId);
                        if (STATUS_SUCCESS  != Ret)
                        {
                            (void)Key_Remove(KeyHandle, CfgPtr->AuthValueSize, CfgPtr->AuthValue);
                        }
                    }
                    else
                    {
                        /* install key handle info to sram */
                        Ret = HSM_Hal_InstallRamKey(KeyHandle, CfgPtr->AuthValue,
                                CfgPtr->AuthValueSize, (uint32)CfgPtr->KeyId);
                    }
                }
            }
            else
            {
                Ret = STATUS_ERROR;
            }
        }
    }

    return Ret;
}

Hal_StatusType HSM_Hal_GenerateKey(const HSM_GenKeyCfgType *CfgPtr)
{
    Hal_StatusType Ret;
    uint32 KeyHandle;
    HSM_KeyAlgoType AlgoType;
    HSM_KeyStorageType StoreType;
    AlgoType = HSM_Hal_AlgoIdToType(CfgPtr->KeyAlgo);

    if ((HSM_Hal_IsKeyIndexValid(CfgPtr->KeyId, AlgoType) == STATUS_SUCCESS)
            && ((HSM_Hal_IsKeyIdExistById((uint8)CfgPtr->KeyId) != STATUS_SUCCESS))) /* adjust keyindex valid*/
    {
        StoreType = HSM_Hal_GetKeyStoreTypeByKeyId(CfgPtr->KeyId);
        /* should adjust keyid exist or not when import key */
        uint32 RetCode = Create_Random_Key((uint32)CfgPtr->KeyAlgo, CfgPtr->KeySize, CfgPtr->ValidUntil,
                        (ehsm_key_mem_type_e)StoreType, CfgPtr->KeyUsageSize,
                    (uint8 *)CfgPtr->KeyUsage, &KeyHandle);
        Ret = HSM_Hal_ErrorCodeCov(RetCode);
        if (STATUS_SUCCESS == Ret)
        {
            if (HSM_KEY_TYPE_NVM == StoreType)
            {
                /* instll key handle info to dflash */
                Ret = HSM_Hal_InstallFlashKey(KeyHandle, CfgPtr->KeyUsage[0U].auth_value,
                        CfgPtr->KeyUsage[0U].auth_size, (uint8)CfgPtr->KeyId);
            }
            else
            {
                /* instll key handle info to sram */
                Ret = HSM_Hal_InstallRamKey(KeyHandle, CfgPtr->KeyUsage[0U].auth_value,
                        CfgPtr->KeyUsage[0U].auth_size, (uint32)CfgPtr->KeyId);
            }
            if (Ret != STATUS_SUCCESS)
            {
                Ret = HSM_Hal_UninstallKey(KeyHandle);
            }
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_DeriveKey(HSM_KeyId ParentKeyId, HSM_GenKeyAlgo ParentKeyAlgo,
        const HSM_DeriveKeyCfgType *TargetKeyPtr)
{
    Hal_StatusType Res = STATUS_ERROR;
    HSM_KeyHandleInfoType HandleInfo;
    uint32 KeyHandle;
    HSM_KeyAlgoType AlgoType;
    HSM_KeyStorageType StoreType;
    AlgoType = HSM_Hal_AlgoIdToType(ParentKeyAlgo);

    Res = HSM_Hal_GetKeyHandleInfo(&HandleInfo, ParentKeyId);
    if (Res == STATUS_SUCCESS)
    {
        if ((HSM_Hal_IsKeyIndexValid(TargetKeyPtr->KeyId, AlgoType) == STATUS_SUCCESS)
            && ((HSM_Hal_IsKeyIdExistById((uint8)TargetKeyPtr->KeyId) != STATUS_SUCCESS))) /* adjust keyindex valid*/
        {
            StoreType = HSM_Hal_GetKeyStoreTypeByKeyId(TargetKeyPtr->KeyId);
            uint32 RetCode;
            RetCode = Create_Derived_Key((uint32)TargetKeyPtr->Kdf, TargetKeyPtr->KeySize, TargetKeyPtr->ValidUntil,
                                     (ehsm_key_mem_type_e)StoreType, TargetKeyPtr->KeyUsageSize,
                                     (uint8 *)TargetKeyPtr->KeyUsage,
                                     HandleInfo.KeyHandle, HandleInfo.AuthSize,
                                     HandleInfo.AuthValue, TargetKeyPtr->SaltDataSize,
                                     TargetKeyPtr->SaltData, &KeyHandle);
            Res = HSM_Hal_ErrorCodeCov(RetCode);
            if (STATUS_SUCCESS == Res)
            {
                if (HSM_KEY_TYPE_NVM == StoreType)
                {
                    Res = HSM_Hal_InstallFlashKey(KeyHandle, TargetKeyPtr->KeyUsage[0U].auth_value,
                            TargetKeyPtr->KeyUsage[0U].auth_size, (uint8)TargetKeyPtr->KeyId);
                }
                else
                {
                    Res = HSM_Hal_InstallRamKey(KeyHandle, TargetKeyPtr->KeyUsage[0U].auth_value,
                            TargetKeyPtr->KeyUsage[0U].auth_size, (uint32)TargetKeyPtr->KeyId);
                }
            }

            if (Res != STATUS_SUCCESS)
            {
                Res = HSM_Hal_UninstallKey(KeyHandle);
            }
        }
        else
        {
            Res = HSM_WRONG_KEY_HANDLE;
        }
    }

    return Res;
}

Hal_StatusType HSM_Hal_GetSecretkey(HSM_KeyId TpKeyId, HSM_KeyId AuthKeyId, HSM_KeyId TargetKeyId,
            const HSM_SecretKeyCfgType *ExportKeyCfg)
{
    Hal_StatusType Ret = STATUS_ERROR;
    HSM_KeyHandleInfoType TpHandle;
    HSM_KeyHandleInfoType AuthHandle;
    HSM_KeyHandleInfoType TargetHandle;
    if (ExportKeyCfg != NULL_PTR)
    {
        Ret =  HSM_Hal_GetKeyHandleInfo(&TpHandle, TpKeyId);
        if (STATUS_SUCCESS == Ret)
        {
            Ret = HSM_Hal_GetKeyHandleInfo(&AuthHandle, AuthKeyId);
            if (STATUS_SUCCESS == Ret)
            {
                Ret = HSM_Hal_GetKeyHandleInfo(&TargetHandle, TargetKeyId);
                if (STATUS_SUCCESS == Ret)
                {
                    /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
                    uint32 RetCode = Key_Export(TargetHandle.KeyHandle,
                         (key_act_use_flags_t *)ExportKeyCfg->SetKeyUseFlag,
                         TpHandle.KeyHandle, TpHandle.AuthSize, TpHandle.AuthValue,
                         AuthHandle.KeyHandle, AuthHandle.AuthSize, AuthHandle.AuthValue,
                         ExportKeyCfg->SetKeyBlobSize, ExportKeyCfg->SecretKeyBlob,
                         ExportKeyCfg->AuthVAlueSize, ExportKeyCfg->AuthValue);
                    /*PRQA S 0310 -- #make sure there are no alignment issues.*/
                    Ret = HSM_Hal_ErrorCodeCov(RetCode);
                }
                else
                {
                    Ret = HSM_WRONG_KEY_HANDLE;
                }
            }
            else
            {
                Ret = HSM_WRONG_KEY_HANDLE;
            }
        }
        else
        {
            Ret = HSM_WRONG_KEY_HANDLE;
        }
    }

    return Ret;
}

Hal_StatusType HSM_Hal_SetSecretKey(HSM_KeyId TpKeyId, HSM_KeyId AuthKeyId,
            const HSM_SecretKeyCfgType *ImportKeyPtr, HSM_KeyId KeyId)
{
    Hal_StatusType Ret;
    uint32 KeyHandle;
    uint8 Auth[32U] = {0};
    uint32 AuthSize = 0U;
    uint8 i;

    HSM_KeyHandleInfoType TpHandle;
    HSM_KeyHandleInfoType AuthHandle;

    HSM_KeyAlgoType AlgoType;
    HSM_KeyStorageType StoreType;

    Ret =  HSM_Hal_GetKeyHandleInfo(&TpHandle, TpKeyId);
    if (STATUS_SUCCESS == Ret)
    {
        Ret = HSM_Hal_GetKeyHandleInfo(&AuthHandle, AuthKeyId);
    }
    if ((STATUS_SUCCESS == Ret) && (ImportKeyPtr != NULL_PTR))
    {
        /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
        /*PRQA S 3305 ++ #make sure there are no alignment issues.*/
        ehsm_internal_key_st const *TempInterKey = (ehsm_internal_key_st *)ImportKeyPtr->SecretKeyBlob;
        HSM_KeyFlagsElementType const *KeyUsages = (HSM_KeyFlagsElementType  const *)&TempInterKey->key_usage.sign;
        /*PRQA S 0310 -- #make sure there are no alignment issues.*/
        /*PRQA S 3305 -- #make sure there are no alignment issues.*/
        for (i = 0U; i < (sizeof(HSM_KeyUsagesType) / sizeof(HSM_KeyFlagsElementType)); i++)
        {
            if (KeyUsages->use_flags !=0U)
            {
                (void)System_Memcpy(Auth, KeyUsages->auth_value, KeyUsages->auth_size);
                AuthSize = KeyUsages->auth_size;
                break;
            }
            KeyUsages ++;
        }

        AlgoType = HSM_Hal_AlgoIdToType(ImportKeyPtr->KeyAlgo);

        if ((HSM_Hal_IsKeyIndexValid(KeyId, AlgoType) == STATUS_SUCCESS)
                && ((HSM_Hal_IsKeyIdExistById((uint8)KeyId) != STATUS_SUCCESS))) /* adjust keyindex valid*/
        {
            StoreType = HSM_Hal_GetKeyStoreTypeByKeyId(KeyId);
            /* should adjust keyid exist or not when import key */
            uint32 RetCode = Key_Import(TpHandle.KeyHandle, TpHandle.AuthSize, TpHandle.AuthValue,
                             AuthHandle.KeyHandle, AuthHandle.AuthSize, AuthHandle.AuthValue,
                             (ehsm_key_mem_type_e)StoreType, *ImportKeyPtr->SetKeyBlobSize,
                             ImportKeyPtr->SecretKeyBlob, *ImportKeyPtr->AuthVAlueSize,
                                ImportKeyPtr->AuthValue, &KeyHandle);
        
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
            if (STATUS_SUCCESS == Ret)
            {
                if (HSM_KEY_TYPE_NVM == StoreType)
                {
                    /* instll key handle info to dflash */
                    Ret = HSM_Hal_InstallFlashKey(KeyHandle, Auth, AuthSize, (uint8)KeyId);
                }
                else
                {
                    /* instll key handle info to sram */
                    Ret = HSM_Hal_InstallRamKey(KeyHandle, Auth, AuthSize, (uint32)KeyId);
                }
                if (Ret != STATUS_SUCCESS)
                {
                    Ret = HSM_Hal_UninstallKey(KeyHandle);
                }
            }
        }
        else
        {
            Ret = STATUS_ERROR;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_GetPubKeyFromPrvKey(HSM_KeyId KeyId, uint8 *PubKey, uint32 *PubKeySize, HSM_GenKeyAlgo Algo)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    HSM_KeyHandleInfoType Handle = {0};

    ehsm_export_pub_key_st InterPubKey = {0};
    uint32 InterPubKeySize= sizeof(ehsm_export_pub_key_st);

    Ret =  HSM_Hal_GetKeyHandleInfo(&Handle, KeyId);
    if (STATUS_SUCCESS == Ret)
    {
        uint32 RetCode = ehsm_get_pub_from_priv(Handle.KeyHandle, Handle.AuthSize, Handle.AuthValue,
                (uint8 *)&InterPubKey, &InterPubKeySize, (uint32)Algo);
        Ret = HSM_Hal_ErrorCodeCov(RetCode);
        if (STATUS_SUCCESS == Ret)
        {
            Ret = HSM_Hal_GetPubKeyByAlgo(Algo, PubKey, &InterPubKey, PubKeySize);
        }
    }

    return Ret;
}

Hal_StatusType HSM_Hal_GenerateDHKey(HSM_KeyId LocalKeyId, const uint8 *RemotePubKey, uint32 RemotePubKeySize,
        HSM_GenKeyAlgo ParKeyAlgo ,const HSM_GenKeyCfgType *TargetKeyPtr)
{
    Hal_StatusType Ret;
    uint32 KeyIndex;
    HSM_KeyHandleInfoType LocalHandle;
    HSM_KeyAlgoType AlgoType;
    HSM_KeyStorageType StoreType;
    ehsm_export_pub_key_st PubKeyType;

    Ret =  HSM_Hal_GetKeyHandleInfo(&LocalHandle, LocalKeyId);
    if (STATUS_SUCCESS == Ret)
    {
        AlgoType = HSM_Hal_AlgoIdToType(TargetKeyPtr->KeyAlgo);

        PubKeyType.algo_id = (uint32)ParKeyAlgo;

        if (RemotePubKey != NULL_PTR)
        {
            (void)System_Memcpy(PubKeyType.key.ecc.p, RemotePubKey, RemotePubKeySize);
        }
        if ((HSM_Hal_IsKeyIndexValid(TargetKeyPtr->KeyId, AlgoType) == STATUS_SUCCESS)
        && ((HSM_Hal_IsKeyIdExistById((uint8)TargetKeyPtr->KeyId) != STATUS_SUCCESS))) /* adjust keyindex valid*/
        {
            StoreType = HSM_Hal_GetKeyStoreTypeByKeyId(TargetKeyPtr->KeyId);
            /* noly support dh_mode = 1 use pubkey data */
            uint32 RetCode = Create_Dh_Key((uint32)TargetKeyPtr->KeyAlgo, TargetKeyPtr->KeySize,
                                TargetKeyPtr->ValidUntil, (ehsm_key_mem_type_e)StoreType, TargetKeyPtr->KeyUsageSize,
                                (ehsm_uint8_t *)TargetKeyPtr->KeyUsage, LocalHandle.KeyHandle, LocalHandle.AuthSize,
                                LocalHandle.AuthValue, 0U, sizeof(ehsm_export_pub_key_st),
                                (uint8 *)&PubKeyType, NULL_PTR, 1U, &KeyIndex);
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
            if (STATUS_SUCCESS == Ret)
            {
                if (HSM_KEY_TYPE_NVM == StoreType)
                {
                    Ret = HSM_Hal_InstallFlashKey(KeyIndex, TargetKeyPtr->KeyUsage[0U].auth_value,
                        TargetKeyPtr->KeyUsage[0U].auth_size, (uint8)TargetKeyPtr->KeyId);
                }
                else
                {
                    Ret = HSM_Hal_InstallRamKey(KeyIndex, TargetKeyPtr->KeyUsage[0U].auth_value,
                        TargetKeyPtr->KeyUsage[0U].auth_size, (uint32)TargetKeyPtr->KeyId);
                }
            }
        }
        else
        {
            Ret = STATUS_ERROR;
        }
    }

    return Ret;
}

Hal_StatusType HSM_Hal_GetKeyStatus(const HSM_KeyStatusType *Status)
{
    Hal_StatusType Ret;
    HSM_KeyHandleInfoType TargetKeyHandle;
    HSM_KeyHandleInfoType CertKeyHandle;

    if (Status != NULL_PTR)
    {
        if ((HSM_Hal_GetKeyHandleInfo(&TargetKeyHandle, Status->TargetKeyId) == STATUS_SUCCESS) &&
            (HSM_Hal_GetKeyHandleInfo(&CertKeyHandle, Status->CertificationKeyId) == STATUS_SUCCESS))
        {
            uint32 RetCode = Key_Status((uint32)TargetKeyHandle.KeyHandle, (uint32)CertKeyHandle.KeyHandle,
                                 CertKeyHandle.AuthSize, CertKeyHandle.AuthValue,
                                 Status->KeyStatusSize, (ehsm_uint8_t *)Status->KeyStatus);
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
        }
        else
        {
            Ret = HSM_WRONG_KEY_HANDLE;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

HSM_LifeCycleType HSM_Hal_GetLifeCycleByReg(void)
{
    uint32 Lc;
    uint8 i;
    HSM_LifeCycleType LcMode = HSM_LIFE_CYCLE_UNNORMAL_MODE;

    Lc = HSM_Reg_GetHsmStatus0();
    for (i = 0U; i < 6u; i++)
    {
        if (((Lc >> (8U + i)) & 0x01u) == 1u)
        {
            LcMode = (HSM_LifeCycleType)(i + 1u);
            break;
        }
    }

    return LcMode;
}

HSM_LifeCycleType HSM_Hal_GetLifeCycle(void)
{
    uint32 LcValue = 0;
    uint32 Ret = 0;
    HSM_LifeCycleType LcMode = HSM_LIFE_CYCLE_UNNORMAL_MODE;

    Ret = HSM_Hal_OtpRead((uint32)OTP_LIFE_CYCLE_ADDR, 4U, (uint8 *)&LcValue);
    if (STATUS_SUCCESS == Ret)
    {
        switch (LcValue)
        {
        case HSM_TEST_MODE:
            LcMode = HSM_LIFE_CYCLE_TEST_MODE;
            break;
        case HSM_DEVELOP_MODE:
            LcMode = HSM_LIFE_CYCLE_DEV_MODE;
            break;
        case HSM_MANU_MODE:
            LcMode = HSM_LIFE_CYCLE_MANU_MODE;
            break;
        case HSM_USER_MODE:
            LcMode = HSM_LIFE_CYCLE_USER_MODE;
            break;
        case HSM_DEBUG_MODE:
            LcMode = HSM_LIFE_CYCLE_DEBUG_MODE;
            break;
        case HSM_DESTROY_MODE:
            LcMode = HSM_LIFE_CYCLE_DESTORY_MODE;
            break;
        default:
            LcMode = HSM_LIFE_CYCLE_UNNORMAL_MODE;
            break;
        }
    }

    return LcMode;
}

Hal_StatusType HSM_Hal_SetLifeCycle(HSM_LifeCycleType LcIndex)
{
    Hal_StatusType Ret;
    uint32 Lc;
    uint32 const LcValue[7u] = {HSM_UNNORMAL_MODE, HSM_TEST_MODE,
                          HSM_DEVELOP_MODE, HSM_MANU_MODE, HSM_USER_MODE, HSM_DEBUG_MODE, HSM_DESTROY_MODE};
    Lc = LcValue[LcIndex];
    Ret = HSM_Hal_OtpWrite(OTP_LIFE_CYCLE_ADDR, 4u, (uint8 *)&Lc);
    return Ret;
}

Hal_StatusType HSM_Hal_DebugAuth(HSM_DebugAuthConfigType *CfgPtr)
{
    Hal_StatusType Ret;
    if (CfgPtr != NULL_PTR)
    {
        Ret = STATUS_SUCCESS;
        if ((CfgPtr->Alg == HSM_DEBUG_AUTH_ALG_SM2_WITH_SM3) ||
            (CfgPtr->Alg == HSM_DEBUG_AUTH_ALG_ECCSECP256R1_WITH_SHA256))
        {
            if ((CfgPtr->PubKey[0U] == 0x04U) && (CfgPtr->PubKeySize == 65U))
            {
                CfgPtr->PubKey = &CfgPtr->PubKey[1U];
                CfgPtr->PubKeySize -= 1U;
            }
            else
            {
                Ret = HSM_INVALID_KEY_SIZE;
            }
        }

        if (STATUS_SUCCESS == Ret)
        {
            /*PRQA S 0310 ++ #make sure there are no alignment issues.*/
            uint32 RetCode = ehsm_debug_auth((void *)CfgPtr);
            Ret = HSM_Hal_ErrorCodeCov(RetCode);
            /*PRQA S 0310 -- #make sure there are no alignment issues.*/
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }
    return Ret;
}

Hal_StatusType HSM_Hal_GetChallenge(HSM_ChallengeType Type, uint8 *ChBuf, uint32 ChSize)
{
    Hal_StatusType Ret;
    ehsm_get_challenge_st Req;
    Req.type = (ehsm_challenge_type_e)Type;
    Req.buf = ChBuf;
    Req.size = ChSize;
    uint32 RetCode = ehsm_get_challenge(&Req);
    Ret = HSM_Hal_ErrorCodeCov(RetCode);
    return Ret;
}

Hal_StatusType HSM_Hal_OtpRead(uint32 OtpAddr, uint32 BytesLen, uint8 *Data)
{
    uint32 Error;
    Hal_StatusType Ret;

    Error = HSM_Reg_OtpRead(OtpAddr, BytesLen, Data);  /* command not used when HSM ready */
    if (Error != HSM_SUCCESS)
    {
        Ret = HSM_WRONG_OTP_READ_ERROR;
    }
    else
    {
        Ret = STATUS_SUCCESS;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_OtpWrite(uint32 OtpAddr, uint32 BytesLen, uint8 *Data)
{
    uint32 Error;
    Hal_StatusType Ret;

    Error = HSM_Reg_OtpWrite(OtpAddr, BytesLen, Data);
    if (Error != HSM_SUCCESS)
    {
        Ret = HSM_WRONG_OTP_WRITE_ERROR;
    }
    else
    {
        Ret = STATUS_SUCCESS;
    }

    return Ret;
}

Hal_StatusType HSM_Hal_EnableSecureBoot(void)
{
    Hal_StatusType Ret;
    uint32 Temp = HSM_SECURE_BOOT_ENABLE;
    Ret = HSM_Hal_OtpWrite((uint32)OTP_SECURE_BOOT_ADDR, 4U, (uint8 *)&Temp);
    return Ret;
}

Hal_StatusType HSM_Hal_DisableSecureBoot(void)
{
    Hal_StatusType Ret;
    uint32 Temp = HSM_SECURE_BOOT_DISABLE;
    Ret = HSM_Hal_OtpWrite((uint32)OTP_SECURE_BOOT_ADDR, 4U, (uint8 *)&Temp);
    return Ret;
}

Hal_StatusType HSM_Hal_SetSecureBootCfg(const HSM_BootCfgType *CfgPtr)
{
    Hal_StatusType Ret;

    if (((FLASH->SWAPSTAT >> 25U) & 0x01U) == 1U)
    {
        Ret =  Flash_Hal_InfoWrite( FLASH_INFO1_BASE_OFFSET, FLASH_INFO_SIZE, (const uint8 *)CfgPtr);
    }
    else
    {
        Ret =  Flash_Hal_InfoWrite( FLASH_INFO2_BASE_OFFSET, FLASH_INFO_SIZE, (const uint8 *)CfgPtr);
    }

    return Ret;
}

void HSM_Hal_InstallCallback(const Hal_CallbackType Callback, void *Args)
{
    HSM_IsrCallback = Callback;
    CallbackArgs = Args;
}

/*!
 * @brief Host image upgrade.
 *
 * @param[in] HostUpgradePara: Pointer to host image upgrade parameter
 * @return operate status
 */
Hal_StatusType HSM_Hal_HostImageSecureUpgrade(const HSM_SecureUpgradeType *HostUpgradePara)
{
    Hal_StatusType Ret;

    if (HostUpgradePara != NULL_PTR)
    {
        uint32 RetCode = HSM_Reg_HostImageUpgrade((const void *)HostUpgradePara);
        if (HSM_SUCCESS == RetCode)
        {
            Ret = STATUS_SUCCESS;
        }
        else
        {
            Ret = HSM_FIRMWARE_UPDATE_FAIL;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

/*!
 * @brief Host image uggrade verify.
 *
 * @param[in] HostVerifyPara: Pointer to host image verify parameter
 * @return operate status
 */
Hal_StatusType HSM_Hal_HostImageSecureVerify(const HSM_ImageVerifyType *HostVerifyPara)
{
    Hal_StatusType Ret;

    if (HostVerifyPara != NULL_PTR)
    {
        uint32 RetCode = HSM_Reg_HostImageVerify((const void *)HostVerifyPara);
        if (HSM_SUCCESS == RetCode)
        {
            Ret = STATUS_SUCCESS;
        }
        else
        {
            Ret = HSM_FIRMWARE_UPDATE_FAIL;
        }
    }
    else
    {
        Ret = STATUS_ERROR;
    }

    return Ret;
}

/*!
 * @brief HSM image install command.
 *
 * @note  Function ID: DES_HSM_API_206
 * @param[in] ProMode: 1: START
                    2: UPDATE
                    4: FINISH
                    8: ONEPASS
 * @param[in] ImageAddr:
                   For installation, it is the host address of the total image for ONEPASS.
                   For upgrading, it is the header of image for START, or the encrypted part
                   of image for UPDATE, or the last encrypted part (may be null) of image for FINISH
 * @param[in] ImageSize: The size of upgrade image in bytes
 * @param[in] CtxAddr: Host address of the context for install.size should be 1.6Kbyte.
 * @param[in] CtxSize: The size of context in bytes.
 * @return HSM mailbox command operate status
 */
Hal_StatusType HSM_Hal_HsmImageSecureInstall(const HSM_HsmImageInstallType *InstallPtr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    if (NULL_PTR == InstallPtr)
    {
        Ret = STATUS_ERROR;
    }

    if (STATUS_SUCCESS == Ret)
    {
        uint32 RetCode = HSM_Reg_HsmImageInstall(InstallPtr->ProMode, InstallPtr->ImageAddr, InstallPtr->ImageSize,
            HSM_FIRMWARE_INSTALL_ADDRESS, InstallPtr->CtxAddr, InstallPtr->CtxSize);
        if (HSM_SUCCESS == RetCode)
        {
            Ret = STATUS_SUCCESS;
        }
        else
        {
            Ret = STATUS_ERROR;
        }
    }

    return Ret;
}

/*!
 * @brief HSM image verify command.
 *
 * @note  Function ID: DES_HSM_API_207
 * @param[in] ProMode: 1: START
                    2: UPDATE
                    4: FINISH
                    8: ONEPASS
 * @param[in] VerifyType: 0x00: For image verification.
                          0x03: For secure boot.
 * @param[in] ImageAddr:
                   For installation, it is the host address of the total image for ONEPASS.
                   For upgrading, it is the header of image for START, or the encrypted part
                   of image for UPDATE, or the last encrypted part (may be null) of image for FINISH
 * @param[in] ImageSize: The size of upgrade image in bytes
 * @return HSM mailbox command operate status
 */
Hal_StatusType HSM_Hal_HsmImageSecureVerify(const HSM_HsmImageVerifyType *VerifyPtr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    if (NULL_PTR == VerifyPtr)
    {
        Ret = STATUS_ERROR;
    }

    if (STATUS_SUCCESS == Ret)
    {
        uint32 RetCode = HSM_Reg_HsmImageVerify(VerifyPtr->ProMode, VerifyPtr->VerifyType,
            VerifyPtr->ImageAddr, VerifyPtr->ImageSize, VerifyPtr->CtxAddr, VerifyPtr->CtxSize);
        if (HSM_SUCCESS == RetCode)
        {
            Ret = STATUS_SUCCESS;
        }
        else
        {
            Ret = STATUS_ERROR;
        }
    }

    return Ret;
}

/**
 * @brief Get HSM firmware version.
 * @note  Function ID: DES_HSM_API_047
 * @param[out] Version HSM firmware version
 * @return Firmware version value
 */
uint32 HSM_Hal_GetHsmFwVersion(const uint32 *Version)
{
    uint32 Ret;

    if (Version != NULL_PTR)
    {
        Ret = HSM_Reg_GetHsmFwVersion(Version);
    }
    else
    {
        Ret = (uint32)STATUS_ERROR;
    }

    return Ret;
}

/**
 * @brief reset HSM.
 * @note  Function ID: DES_HSM_API_048
 * @return none
 */
void HSM_Hal_ResetHsm(void)
{
    MODIFY_REG32(CKGEN->PERI_SFT_RST2, CKGEN_PERI_SFT_RST2_SRST_HSM_Msk, CKGEN_PERI_SFT_RST2_SRST_HSM_Pos, 1U);
}

ISR(HSM_IRQHandler)
{
    /* for crypto handle*/
    if (HSM_IsrCallback != NULL_PTR)
    {
        HSM_IsrCallback(CallbackArgs);
    }

    /* clear interrupt */
    WRITE_REG32(HSM->MB_HSM2SOC_SOC_INT, 0x01U);
    /* disable interrupt */
    WRITE_REG32(HSM->MB_HSM2SOC_SOC_INT_EN, 0x0U);
}

/* =============================================  EOF  ============================================== */
