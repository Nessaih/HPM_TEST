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
 * AutoChips Inc. (C) 2021. All rights reserved.
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
 * @file AC784xx_Hsm_Reg.h
 *
 * @brief This file provides HSM hardware integration functions.
 *
 */

#ifndef AC784XX_HSM_REG_H
#define AC784XX_HSM_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Device_Register.h"
#include "eHSM_Mailbox_Reg_Ip.h"
/* ============================================  Define  ============================================ */

#define OTP_BASE_ADDR                           (0x01540000UL)
#define OTP_LIFE_CYCLE_ADDR                     (OTP_BASE_ADDR + 0x00UL)
#define OTP_UID_ADDR                            (OTP_BASE_ADDR + 0x04UL)
#define OTP_HW_CTRL_FIELD_ADDR                  (OTP_BASE_ADDR + 0x18UL)
#define OTP_FW_CTRL_FIELD_ADDR_L                (OTP_BASE_ADDR + 0x20UL)
#define OTP_FW_CTRL_FIELD_ADDR_H                (OTP_BASE_ADDR + 0x24UL)
#define OTP_HOST_CTRL_FIELD_ADDR_L              (OTP_BASE_ADDR + 0x28UL)
#define OTP_HOST_CTRL_FIELD_ADDR_H              (OTP_BASE_ADDR + 0x2CUL)
#define OTP_ERR_RSP_CTRL_ADDR                   (OTP_BASE_ADDR + 0x30UL)
#define OTP_HSM_VERSION_ADDR                    (OTP_BASE_ADDR + 0x50UL)
#define OTP_SOC_VERSION_ADDR                    (OTP_BASE_ADDR + 0x60UL)
#define OTP_KEY_ADDR(a)                         (OTP_BASE_ADDR + 0x70UL + ((a)*10U*4UL))
#define OTP_KEY_CRC(a)                          (OTP_BASE_ADDR + 0x94UL +((a)*10U*4UL))
#define OTP_HSM_ENABLE_ADDR                     (OTP_BASE_ADDR + 0x570UL)
#define OTP_SECURE_BOOT_ADDR                    (OTP_BASE_ADDR + 0x574UL)

#define OTP_KEY_ATTR_ENCODE_EACH_LENGTH         (CONFIG_OTP_KEY_ATTR_LENGTH)
#define OTP_KEY_ATTR_BYTE_LENGTH                (4UL)

#define OTP_VERSION_ENCODE_LENGTH               (CONFIG_OTP_VERSION_LENGTH)
#define OTP_VERSION_LENGTH                      (16UL)

#define OTP_KEY_CRC_SIZE                        (4UL)
#define OTP_KEY_SIZE                            (32UL + OTP_KEY_CRC_SIZE)

#define OTP_KEY_ATTR_LC                         (OTP_BASE_ADDR + 0x600UL)
#define OTP_SIZE                                (0xC00UL)

#define SOC_CMD_IMAGE_UPGRADE_UPGRADE               (0x00DFFF20U)
#define SOC_CMD_IMAGE_VERIFY                        (0x00DDFF22U)
#define SOC_CMD_GET_HSM_FW_VERSION                  (0xAC784301U)

/*!
 * @brief bootloader cmd.
 * @return none
 */
#define HSM_CMD_GET_RANDOM_KEY              (0x00F7FF08U)
#define HSM_CMD_ENCRYPT_KEY                 (0x00F6FF09U)
#define HSM_CMD_BOOTROM_SET_BAUDRATE        (0x00F5FF0AU)
#define HSM_CMD_OTP_WRITE                   (0x00FEFF01U)
#define HSM_CMD_OTP_READ                    (0x00FDFF02U)
#define HSM_CMD_BOOTROM_IMAGE_INSTALL       (0x00FAFF05U)
#define HSM_CMD_BOOTROM_IMAGE_VERIFY        (0x00F9FF06U)
/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */

/*!
 * @brief get hsm status 0 register.
 * @note  Function ID: DES_HSM_API_200
 * @return none
 */
static inline uint32 HSM_Reg_GetHsmStatus0(void)
{
    return READ_REG32(HSM_BOOTROM->HSM_STATUS0);
}

/*!
 * @brief get hsm status 1 register.
 * @note  Function ID: DES_HSM_API_201
 * @return none
 */
static inline uint32 HSM_Reg_GetHsmStatus1(void)
{
    return READ_REG32(HSM_BOOTROM->HSM_STATUS1);
}

/*!
 * @brief get hsm boot stat register.
 * @note  Function ID: DES_HSM_API_202
 * @return none
 */
static inline uint32 HSM_Reg_GetHsmBootStatus(void)
{
    return ((READ_REG32(HSM_BOOTROM->HSM_BOOT_STAT) & HSM_BOOT_STAT_BOOT_STAT_Msk) >> HSM_BOOT_STAT_BOOT_STAT_Pos);
}

/*!
 * @brief set hsm boot stat register.
 * @note  Function ID: DES_HSM_API_203
 * @return none
 */
static inline uint32 HSM_Reg_SetHsmBootStatus(uint32 Value)
{
    return MODIFY_REG32(HSM_BOOTROM->HSM_BOOT_STAT, HSM_BOOT_STAT_BOOT_STAT_Msk, HSM_BOOT_STAT_BOOT_STAT_Pos, Value);
}

/*!
 * @brief get secureboot enable status.
 * @note  Function ID: DES_HSM_API_204
 * @return none
 */
static inline uint32 HSM_Reg_GetSecureBootEnable(void)
{
    return ((READ_REG32(HSM_BOOTROM->HSM_STAT) & HSM_STAT_SECURE_BOOT_ENABLE_Msk) >> HSM_STAT_SECURE_BOOT_ENABLE_Pos);
}

/*!
 * @brief get hsm enable status.
 * @note  Function ID: DES_HSM_API_205
 * @return none
 */
static inline uint32 HSM_Reg_GetHsmEnable(void)
{
    return ((READ_REG32(HSM_BOOTROM->HSM_STAT) & HSM_STAT_HSM_ENABLE_Msk) >> HSM_STAT_HSM_ENABLE_Pos);
}

/**
 * @brief Get HSM OTP hardware control value for Hsm.
 * @note  Function ID: DES_HSM_API_206
 * @return OTP hardware control value
 */
static inline uint32 HSM_Reg_GetHsmOtpCtrl(void)
{
    return READ_REG32(HSM_BOOTROM->HSM_FW_CTRL_HSM_H);
}

/**
 * @brief Get Host OTP hardware control value for SoC.
 * @note  Function ID: DES_HSM_API_207
 * @return OTP hardware control value
 */
static inline uint32 HSM_Reg_GetHostOtpCtrl(void)
{
    return READ_REG32(HSM_BOOTROM->HSM_FW_CTRL_SOC_L);
}

/**
 * @brief Get OTP hardware control value for Otp key.
 * @note  Function ID: DES_HSM_API_208
 * @return OTP hardware control value
 */
static inline  uint32 HSM_Reg_GetHwOtpCtrl(void)
{
    return READ_REG32(HSM_BOOTROM->HSM_HW_CTRL_L);
}

/*!
 * @brief Host trigger command.
 * @note  Function ID: DES_HSM_API_209
 * @param [in] Cmd: mailbox pointer
 * @param [in] WordLen: mailbox length (the unit is word)
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_TrigCmd(const uint32 *Cmd, uint32 WordLen)
{
    uint32 RegValue;
    /* open mailbox path */
    HOST2HSM_ACCESS_PATH |= (1U << 1U);
    /* 1:hsm working ;soc to hsm cmd not handle done  0:hsm write 1 free */
    while (0x0U != ((HSM->MB_SOC2HSM_NOTE & HSM_MB_SOC2HSM_NOTE_Msk) >> HSM_MB_SOC2HSM_NOTE_Pos))
    {
    }

    /* write cmd to Mailbox reg, will send to hsm */
    for (uint32 i = 0U; i < WordLen; i++)
    {
        /*PRQA S 1840 ++ # ensure right value correct.*/
        HSM->MB_SOC2HSM_INFO_D[i] = Cmd[i];
        /*PRQA S 1840 -- # ensure right value correct.*/
    }

    /* soc set, hsm clear */
    /* soc write 1 to notify hsm cmd ready, please read and handle */
    HSM->MB_SOC2HSM_NOTE = 0x01U;
    /* hsm will 1 to clear and tigger soc s2h irq reg */
    /* 0:hsm doing req, please soc wait.*/
    /* 1:req handle done, hsm write 1 set ok, notify soc read handle result and trigger h2s irq reg */
    while (0x0U == ((HSM->MB_HSM2SOC_NOTE & HSM_MB_HSM2SOC_NOTE_Msk) >> HSM_MB_HSM2SOC_NOTE_Pos))
    {
    }
    /* soc clear, hsm set */
    /* soc write 1 to clear this req */
    HSM->MB_HSM2SOC_NOTE = 0x01U;

    RegValue = HSM->MB_HSM2SOC_INFO_D[0];

    /* close mailbox path */
    HOST2HSM_ACCESS_PATH &= ~(1U << 1U);
    return RegValue;

}

/*!
 * @brief hsm get return data code.
 *
 * @note  Function ID: DES_HSM_API_210
 * @param[in] none
 * @return none
 */
static inline void HSM_Reg_GetReturnDataCode(uint32 *data, uint32 size)
{
    /* open mailbox path */
    HOST2HSM_ACCESS_PATH |= (1U << 1U);

    for (uint32 i = 0U; i < size; i+=4U)
    {
        /*PRQA S 0458 ++ # allow directon operator .*/
        data[i] = *(volatile uint32 *)((uint32)&(HSM->MB_HSM2SOC_INFO_D[1]) + i);
        /*PRQA S 0458 -- # allow directon operator */
    }

    /* close mailbox path */
    HOST2HSM_ACCESS_PATH &= ~(1U << 1U);
}

/*!
 * @brief host image upgrade.
 *
 * @note  Function ID: DES_HSM_API_211
 * @param[in] HostUpgradePara: Pointer to host image upgrade parameter
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_HostImageUpgrade(const void *HostUpgradePara)
{
    uint32 cmdBuf[4] = {0};

    cmdBuf[0] = SOC_CMD_IMAGE_UPGRADE_UPGRADE;
    cmdBuf[3] = (uint32)HostUpgradePara;

    return HSM_Reg_TrigCmd(cmdBuf, 4U);
}

/*!
 * @brief Host image upgrade verify.
 *
 * @note  Function ID: DES_HSM_API_212
 * @param[in] HostVerifyPara: Pointer to host image verify parameter
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_HostImageVerify(const void *HostVerifyPara)
{
    uint32 cmdBuf[4] = {0};

    cmdBuf[0] = SOC_CMD_IMAGE_VERIFY;
    cmdBuf[3] = (uint32)HostVerifyPara;

    return HSM_Reg_TrigCmd(cmdBuf, 4U);
}

/*!
 * @brief HSM image install command.
 *
 * @note  Function ID: DES_HSM_API_213
 * @param[in] ProMode: 1: START
                    2: UPDATE
                    4: FINISH
                    8: ONEPASS
 * @param[in] ImageAddr:
                   For installation, it is the host address of the total image for ONEPASS.
                   For upgrading, it is the header of image for START, or the encrypted part
                   of image for UPDATE, or the last encrypted part (may be null) of image for FINISH
 * @param[in] ImageSize: The size of upgrade image in bytes
 * @param[in] StoreAddr: destination address for installing
 * @param[in] CtxAddr: Host address of the context for streaming approach
 * @param[in] CtxSize: The size of context in bytes

 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_HsmImageInstall(uint8 ProMode, uint32 ImageAddr, uint32 ImageSize,
                          uint32 StoreAddr, uint32 CtxAddr, uint32 CtxSize)
{
    uint32 CmdBuf[16U] = {0U};

    CmdBuf[0U] = HSM_CMD_BOOTROM_IMAGE_INSTALL;
    CmdBuf[3U] = ProMode;
    CmdBuf[8U] = ImageAddr;
    CmdBuf[9U] = ImageSize;
    CmdBuf[10U] = StoreAddr;
    CmdBuf[14U] = CtxAddr;
    CmdBuf[15U] = CtxSize;

    return HSM_Reg_TrigCmd(CmdBuf, 16U);
}

/*!
 * @brief HSM image verify command.
 *
 * @note  Function ID: DES_HSM_API_214
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
 * @param[in] CtxAddr: Host address of the context for streaming approach
 * @param[in] CtxSize: The size of context in bytes

 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_HsmImageVerify(uint8 ProMode, uint8 VerifyType, uint32 ImageAddr,
                             uint32 ImageSize, uint32 CtxAddr, uint32 CtxSize)
{
    uint32 CmdBuf[16u] = {0u};

    CmdBuf[0U] = HSM_CMD_BOOTROM_IMAGE_VERIFY;
    CmdBuf[3u] = ProMode | ((uint32)VerifyType << 8U);
    CmdBuf[8u] = ImageAddr;
    CmdBuf[9u] = ImageSize;
    CmdBuf[14u] = CtxAddr;
    CmdBuf[15u] = CtxSize;

    return HSM_Reg_TrigCmd(CmdBuf, 16U);
}

/*!
 * @brief get hsm fw version.
 *
 * @note  Function ID: DES_HSM_API_215
 * @param[in] Version: Version address
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_GetHsmFwVersion(const uint32 *Version)
{
    uint32 CmdBuf[4] = {0};

    CmdBuf[0] = SOC_CMD_GET_HSM_FW_VERSION;  //0xac784301U
    CmdBuf[3] = (uint32)Version;

    return HSM_Reg_TrigCmd(CmdBuf, 4U);
}

/*!
 * @brief HSM get rand key by bootloader.
 *
 * @note  Function ID: DES_HSM_API_216
 * @param[in] KeyLevel
              l. The random key is encrypted with CHIP_ROOT_KEY
              2: The random key is encrypted with DEVICE_ROOT_KEY
 * @param[in] KeyType
              l. for symmetric key
              2: for SM2 private key
              4: for SECP256R1 private key
 * @param[out] KeyOut: Host address of output encrypted random key and 4
                       bytes CRC32 value for encrypted key
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_GetRandKey(uint8 KeyLevel, uint32 KeyType, const uint8 *KeyOut)
{
    uint32 CmdBuf[11] = {0};

    CmdBuf[0]   = HSM_CMD_GET_RANDOM_KEY;
    CmdBuf[3]   = KeyLevel | (KeyType << 8U);
    CmdBuf[10]  = (uint32)KeyOut;

    return HSM_Reg_TrigCmd(CmdBuf, 11U);
}

/*!
 * @brief HSM get rand key by hsm firmware.
 *
 * @note  Function ID: DES_HSM_API_217
 * @param[in] KeySlot
             0x01: Generate random DEVICE_ROOT_KEY(Only support symmetric key.)
             0x02: Generate random SOC_FW_VERIFY_KEY(It is needed only for secure boot with symmetric key.)
             0x04: Generate random SOC_ENC_KEY(Only support symmetric key.)
             0x08: Generate random SOC_PRIVATE_KEY(Only support SM2 or SECP256R1.)
             0x10: Generate random USER_ROOT_KEY(Only support symmetric key.)
             Other values are reserved.
 * @param[in] KeyType
              l. for symmetric key
              2: for SM2 private key
              4: for SECP256R1 private key
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_GetRandKeyByHsmFw(uint32 KeySlot, uint32 KeyType)
{
    uint32 CmdBuf[4] = {0};

    CmdBuf[0]   = HSM_CMD_GET_RANDOM_KEY;
    CmdBuf[3]   = (KeySlot << 16U) | (KeyType << 8U);

    return HSM_Reg_TrigCmd(CmdBuf, 4U);
}

/*!
 * @brief encrypt key by bootloader
 *
 * @note  Function ID: DES_HSM_API_218
 * @param[in] KeyLevel
              l. The random key is encrypted with CHIP_ROOT_KEY
              2: The random key is encrypted with DEVICE_ROOT_KEY
 * @param[in] KeyIn: Host address of encrypted input key or hash
 * @param[in] keySize: Size of Encrypted input key or hash, can only be 48 bytes
 * @param[out] KeyOut: Host address to stored encrypted key or hash and 4 bytes
                       CRC32 value for encrypted key which size is 32
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_EncryptKey(uint8 KeyLevel, const uint8 *KeyIn, uint32 KeySize, const uint8 *KeyOut)
{

    uint32 cmdBuf[11U] = {0};

    cmdBuf[0]   = HSM_CMD_ENCRYPT_KEY;
    cmdBuf[3]   = KeyLevel;
    cmdBuf[8]   = (uint32)KeyIn;
    cmdBuf[9]   = KeySize;
    cmdBuf[10]  = (uint32)KeyOut;

    return HSM_Reg_TrigCmd(cmdBuf, 11U);
}

/*!
 * @brief encrypt key by hsm firmware.
 *
 * @note  Function ID: DES_HSM_API_219
 * @param[in] KeyIn: Host address of encrypted input key or hash.
 * @param[in] KeySize: Size of Encrypted input key or hash, can only be 48 bytes.
 * @param[in] KeyType:
                0x01: Symmetric key
                0x02: for hash or public key
                0x04: for extend key attributes(reserved)
 * @param[in] KeySlot:
                0x01: HOST_DEBUG_KEY
                0x02: HOST_FW_VERIFY_KEY
                0x04: HOST_UPGRADE_ENCRYPT_KEY
                0x08: HOST_UPGRADE_VERIFY_KEY
                0x10: USER_DEBUG_KEY
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_EncryptKeyByHsmFw(const uint8 *KeyIn, uint32 KeySize, uint32 KeySlot, uint32 KeyType)
{

    uint32 CmdBuf[11] = {0};

    CmdBuf[0] = HSM_CMD_ENCRYPT_KEY;
    CmdBuf[3] = (KeySlot << 16U) | (KeyType << 8U);
    CmdBuf[9] = (uint32)KeyIn;
    CmdBuf[10] = KeySize;

    return HSM_Reg_TrigCmd(CmdBuf, 11U);
}

/*!
 * @brief OTP write.
 *
 * @note  Function ID: DES_HSM_API_220
 * @param[in] Addr: destination address for writing
 * @param[in] ByteLen: mailbox length (the unit is word)
 * @param[in] SrcData: The data pointer for writing
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_OtpWrite(uint32 Addr, uint32 ByteLen, uint8* SrcData)
{
    uint32 CmdBuf[6u] = {0U};

    CmdBuf[0u] = HSM_CMD_OTP_WRITE;
    CmdBuf[3u] = Addr;
    CmdBuf[4u] = ByteLen;
    CmdBuf[5u] = (uint32)SrcData;

    return HSM_Reg_TrigCmd(CmdBuf, 6U);
}

/*!
 * @brief OTP read.
 *
 * @note  Function ID: DES_HSM_API_221
 * @param[in] Addr: source address for reading
 * @param[in] ByteLen: mailbox length (the unit is word)
 * @param[in] SrcData: The pointer for reading data
 * @return HSM mailbox command operate status
 */
static inline uint32 HSM_Reg_OtpRead(uint32 Addr, uint32 ByteLen, uint8* SrcData)
{
    uint32 CmdBuf[6] = {0};

    CmdBuf[0] = HSM_CMD_OTP_READ;
    CmdBuf[3] = Addr;
    CmdBuf[4] = ByteLen;
    CmdBuf[5] = (uint32)SrcData;

    return HSM_Reg_TrigCmd(CmdBuf, 6U);
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* AC784XX_HSM_REG_H */

/* =============================================  EOF  ============================================== */
