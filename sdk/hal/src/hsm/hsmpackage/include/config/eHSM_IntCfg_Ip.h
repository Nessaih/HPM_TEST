#ifndef EHSM_INTCFG_IP_H
#define EHSM_INTCFG_IP_H

#ifdef __cplusplus
extern "C"{
#endif

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/**
 * @brief configuration for multiple channels support
 */
#define CONFIG_EHSM_ARCH_MULTI_CHANNEL

/**
 * @brief configuration for operation system support
 */
#define CONFIG_EHSM_ARCH_OS_NONE

/**
 * @brief configuration for RT-Thread support
 */
//#define CONFIG_EHSM_ARCH_OS_RTT

/**
 * @brief The size of ske requirements in cache.
 */
#define CONFIG_EHSM_ARCH_V_REQ_SKE_MAX_SIZE                     16U

/**
 * @brief The size of hash requirements in cache.
 */
#define CONFIG_EHSM_ARCH_V_REQ_HASH_MAX_SIZE                    16U

/**
 * @brief configuration for secure counter increased automatically support
 */
#define CONFIG_EHSM_COUNTER_AUTO_INCREATE

/**
 * @brief configuration for cryptographic primitives support
 */
#define CONFIG_EHSM_CRYPTO_AEAD
//#define CONFIG_EHSM_CRYPTO_ALGOFAM_3DES
#define CONFIG_EHSM_CRYPTO_ALGOFAM_AES
#define CONFIG_EHSM_CRYPTO_ALGOFAM_CTRDRBG
//#define CONFIG_EHSM_CRYPTO_ALGOFAM_DES
#define CONFIG_EHSM_CRYPTO_ALGOFAM_DH
#define CONFIG_EHSM_CRYPTO_ALGOFAM_ECC
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_BRAINPOOLP160R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_BRAINPOOLP192R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_BRAINPOOLP224R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_BRAINPOOLP256R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_BRAINPOOLP320R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_BRAINPOOLP384R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_BRAINPOOLP512R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_SECP160R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_SECP192R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_SECP224R1
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_SECP256R1
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_SECP384R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_SECP521R1
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_ECIES
#define CONFIG_EHSM_CRYPTO_ALGOFAM_ED25519
#define CONFIG_EHSM_CRYPTO_ALGOFAM_MD5
#define CONFIG_EHSM_CRYPTO_ALGOFAM_PBKDF2
#define CONFIG_EHSM_CRYPTO_ALGOFAM_RSA
    #ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_1024
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_1024_CRT
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_2048
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_2048_CRT
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_3072
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_3072_CRT
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_4096
    //#define CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_4096_CRT
    #define CONFIG_EHSM_CRYPTO_ALGOMODE_RSASSA_PSS
    #endif
#define CONFIG_EHSM_CRYPTO_ALGOFAM_SHA1
#define CONFIG_EHSM_CRYPTO_ALGOFAM_SHA2
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_SHA224
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_SHA256
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_SHA384
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_SHA512
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_SHA512_224
    #define CONFIG_EHSM_CRYPTO_ALGOFAM_SHA512_256
//#define CONFIG_EHSM_CRYPTO_ALGOFAM_SHA3
#define CONFIG_EHSM_CRYPTO_ALGOFAM_SM2
#define CONFIG_EHSM_CRYPTO_ALGOFAM_SM3
#define CONFIG_EHSM_CRYPTO_ALGOFAM_SM4
//#define CONFIG_EHSM_CRYPTO_ALGOFAM_SM9
//#define CONFIG_EHSM_CRYPTO_ALGOFAM_X25519
#define CONFIG_EHSM_CRYPTO_ALGOFAM_X963
#define CONFIG_EHSM_CRYPTO_ALGOMODE_CBC
#define CONFIG_EHSM_CRYPTO_ALGOMODE_CBC_MAC
//#define CONFIG_EHSM_CRYPTO_ALGOMODE_CCM
#define CONFIG_EHSM_CRYPTO_ALGOMODE_CFB
#define CONFIG_EHSM_CRYPTO_ALGOMODE_CMAC
#define CONFIG_EHSM_CRYPTO_ALGOMODE_CTR
#define CONFIG_EHSM_CRYPTO_ALGOMODE_ECB
//#define CONFIG_EHSM_CRYPTO_ALGOMODE_GCM
#define CONFIG_EHSM_CRYPTO_ALGOMODE_GMAC
#define CONFIG_EHSM_CRYPTO_ALGOMODE_OFB
//#define CONFIG_EHSM_CRYPTO_ALGOMODE_XTS

/**
 * @brief configuration for certificate support
 */
//#define CONFIG_EHSM_CRYPTO_CERTIFICATE

/**
 * @brief configuration for fast cmac support
 */
//#define CONFIG_EHSM_CRYPTO_FAST_CMAC

/**
 * @brief configuration for RSA CRT mode support
 */
#define CONFIG_EHSM_CRYPTO_RSA_CRT_MODE

/**
 * @brief The max public key size of x509 certificate
 */
#define CONFIG_EHSM_CRYPTO_V_CERT_MAX_PUB_K_SIZE                256U

/**
 * @brief The max size of x509 certiticate data
 */
#define CONFIG_EHSM_CRYPTO_V_CERT_MAX_SIZE                      1024U

/**
 * @brief The max size of associated data in bytes
 */
#define CONFIG_EHSM_CRYPTO_V_GCM_MAX_AAD_SIZE                   128U

/**
 * @brief The max key size in bytes of HMAC
 */
#define CONFIG_EHSM_CRYPTO_V_HMAC_MAX_KSIZE                     256U

/**
 * @brief configuration for EVB board support
 */
//#define CONFIG_EHSM_CUSTOM_EVB_BOARD

/**
 * @brief configuration for log support
 */
//#define CONFIG_EHSM_DEBUG_ENABLE
    #define CONFIG_EHSM_V_LOG_ERR   0
    #define CONFIG_EHSM_V_LOG_WARN  1
    #define CONFIG_EHSM_V_LOG_DEBUG 2
    #define CONFIG_EHSM_V_LOG_INFO  3
    #define CONFIG_EHSM_V_LOG_LEVEL CONFIG_EHSM_V_LOG_DEBUG

/**
 * @brief configuration of debug load key for test
 */
//#define CONFIG_EHSM_DEBUG_LOAD_K

/**
 * @brief configuration for firmware upgrade support
 */
#define CONFIG_EHSM_FIRMWARE_UPGRADE

/**
 * @brief configuration for AHB byte operation support
 */
#define CONFIG_EHSM_HW_AHB_BYTE

/**
 * @brief configuration for version 1
 */
//#define CONFIG_EHSM_HW_BRANCH_1_0_0

/**
 * @brief configuration for version 2
 */
#define CONFIG_EHSM_HW_BRANCH_2_0_0

/**
 * @brief configuration for version 3
 */
//#define CONFIG_EHSM_HW_BRANCH_3_0_0

/**
 * @brief configuration custom HW
 */
//#define CONFIG_EHSM_HW_CUSTOM

/**
 * @brief configuration for hardware counter support
 */
#define CONFIG_EHSM_HW_COUNTER

/**
 * @brief configuration for flash type
 */
#define CONFIG_EHSM_HW_FLASH
    #define CONFIG_EHSM_HW_V_FLASH_NONE          0
    #define CONFIG_EHSM_HW_V_FLASH_CUSTOMER      1
    #define CONFIG_EHSM_HW_V_FLASH_SIMULATE      2
    #define CONFIG_EHSM_HW_V_FLASH_TYPE          CONFIG_EHSM_HW_V_FLASH_CUSTOMER

#if (CONFIG_EHSM_HW_V_FLASH_TYPE == CONFIG_EHSM_HW_V_FLASH_CUSTOMER)
    #define CONFIG_EHSM_HW_V_FLASH_BASE_ADDR         (0x01120000)
#else
    #define CONFIG_EHSM_HW_V_FLASH_BASE_ADDR         (0x71120000)
#endif

    #define CONFIG_EHSM_HW_V_FLASH_SIZE              (0x20000)
    #define CONFIG_EHSM_HW_V_CODE_MAX_SIZE           (0x1E000)
    #define CONFIG_EHSM_HW_V_FLASH_PAGE_SIZE         (0x800)

    #define CONFIG_EHSM_HW_V_FLASH_DATA_ADDR         (CONFIG_EHSM_HW_V_FLASH_BASE_ADDR + CONFIG_EHSM_HW_V_CODE_MAX_SIZE)
    #define CONFIG_EHSM_HW_V_FLASH_DATA_SIZE         (CONFIG_EHSM_HW_V_FLASH_SIZE - CONFIG_EHSM_HW_V_CODE_MAX_SIZE)

    #define CONFIG_EHSM_HW_V_FLASH_KEY_ADDR          (CONFIG_EHSM_HW_V_FLASH_DATA_ADDR)
    #define CONFIG_EHSM_HW_V_FLASH_KEY_SIZE          (0x1000)
    #define CONFIG_EHSM_HW_V_FLASH_LOG_ADDR          (CONFIG_EHSM_HW_V_FLASH_KEY_ADDR + CONFIG_EHSM_HW_V_FLASH_KEY_SIZE)
    #define CONFIG_EHSM_HW_V_FLASH_LOG_SIZE          (CONFIG_EHSM_HW_V_FLASH_PAGE_SIZE)
    #define CONFIG_EHSM_HW_V_FLASH_SYS_ADDR          (CONFIG_EHSM_HW_V_FLASH_LOG_ADDR + CONFIG_EHSM_HW_V_FLASH_LOG_SIZE)
    #define CONFIG_EHSM_HW_V_FLASH_SYS_SIZE          (CONFIG_EHSM_HW_V_FLASH_PAGE_SIZE)

    #define CONFIG_EHSM_HW_V_FLASH_FREE_ECC_IDX1     (0xFBFFBFFFFBFFBFFF)
    #define CONFIG_EHSM_HW_V_FLASH_FREE_ECC_IDX2     (0xF3BFBBEFF3BFBBEF)
    #define CONFIG_EHSM_HW_V_FLASH_WRITE_MIN_BYTES   (8)
    #define CONFIG_EHSM_HW_V_FLASH_ERASE_CELL_VALUE  (0xFF)

/**
 * @brief configuration for flash ECC support
 */
#define CONFIG_EHSM_HW_FLASH_ECC

/**
 * @brief configuration for GuoMi level1
 */
#define CONFIG_EHSM_HW_GUOMI_LEVEL1

/**
 * @brief configuration for GuoMi level2
 */
//#define CONFIG_EHSM_HW_GUOMI_LEVEL2

/**
 * @brief configuration for HASH support
 */
#define CONFIG_EHSM_HW_HASH

/**
 * @brief configuration for HASH DMA mode support
 */
#define CONFIG_EHSM_HW_HASH_DMA

/**
 * @brief configuration for HASH HP support
 */
//#define CONFIG_EHSM_HW_HASH_HP

/**
 * @brief configuration for HASH LP support
 */
#define CONFIG_EHSM_HW_HASH_LP

/**
 * @brief configuration for HMAC secure port support
 */
//#define CONFIG_EHSM_HW_HMAC_SECURE_PORT

/**
 * @brief configuration for protection in encrypt key install
 */
#define CONFIG_EHSM_HW_INSTALL_K_KEK

/**
 * @brief configuration for hardware KMU support
 */
//#define CONFIG_EHSM_HW_KMU

/**
 * @brief configuration for low power support
 */
#define CONFIG_EHSM_HW_LOW_POWER

/**
 * @brief configuration life cycle exist valid field
 */
//#define CONFIG_EHSM_HW_OTP_WITH_LIFE_CYCLE_VALID

/**
 * @brief configuration life cycle mode value
 */
#define CONFIG_EHSM_HW_LIFE_CYCLE_TEST_MODE            0xFFFFFFFF
#define CONFIG_EHSM_HW_LIFE_CYCLE_DEVELOP_MODE         0xBD7E7BEB
#define CONFIG_EHSM_HW_LIFE_CYCLE_MANUFACTURE_MODE     0xB93E5BE9
#define CONFIG_EHSM_HW_LIFE_CYCLE_USER_MODE            0xA83E1369
#define CONFIG_EHSM_HW_LIFE_CYCLE_DEBUG_MODE           0x283A0321
#define CONFIG_EHSM_HW_LIFE_CYCLE_DESTROY_MODE         0x00000000

/**
 * @brief configuration for REG read write support
 */
//#define CONFIG_EHSM_HW_REG

/**
 * @brief configuration for OTP support
 */
#define CONFIG_EHSM_HW_OTP

/**
 * @brief configuration for OTP ECC support
 */
//#define CONFIG_EHSM_HW_OTP_ECC

/**
 * @brief configuration for OTP version2 support
 */
#define CONFIG_EHSM_HW_OTP_MAP

#define CONFIG_EHSM_HW_V_NONE_OTP            0
#define CONFIG_EHSM_HW_V_CUSTOMER_OTP        1
#define CONFIG_EHSM_HW_V_SIMULATE_OTP        2
#define CONFIG_EHSM_HW_V_OTP_TYPE            CONFIG_EHSM_HW_V_CUSTOMER_OTP

#if (CONFIG_EHSM_HW_V_OTP_TYPE == CONFIG_EHSM_HW_V_CUSTOMER_OTP)
#define CONFIG_EHSM_HW_V_OTP_BASE_ADDR         (0x01540000UL)
#else
#define CONFIG_EHSM_HW_V_OTP_BASE_ADDR         (0x71540000UL)
#endif
#define CONFIG_EHSM_HW_V_OTP_SIZE              (0x800)
#define CONFIG_EHSM_HW_V_OTP_PAGE_SIZE         (0x800)
#define CONFIG_EHSM_HW_V_OTP_KEY_NUM           (32)
#define CONFIG_EHSM_HW_V_OTP_WRITE_MIN_BYTES   (4)

//#define CONFIG_EHSM_HW_OTP_KEY_ATTR_4BYTE_TO_3BYTE


//#define CONFIG_EHSM_HW_OTP_K_ATTR_4BYTE_TO_3BYTE

#ifdef CONFIG_EHSM_HW_OTP_ECC_32BIT_TO_4BIT_AND_2BIT_NO1
#define CONFIG_EHSM_HW_OTP_ECC_BIT_1111        (0x333D93E5U)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_1110        (0x323913E1U)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_1100        (0x122911E0U)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_1000        (0x020901A0U)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_0000        (0x00000000U)

#define CONFIG_EHSM_HW_OTP_ECC_BIT_11           (0x323913E1U)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_10           (0x122911E0U)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_01           (0x020901A0U)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_00           (0x00000000U)

#elif defined CONFIG_EHSM_HW_OTP_ECC_64BIT_TO_1BIT_NO2
#define CONFIG_EHSM_HW_OTP_ECC_BIT_1_DWORD1     (0xFFFFFFFFU)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_1_DWORD2     (0xFFFFFFFFU)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_0_DWORD1     (0x00000000U)
#define CONFIG_EHSM_HW_OTP_ECC_BIT_0_DWORD2     (0x00000000U)
#else
#endif

#define CONFIG_EHSM_HW_V_OTP_K_ATTR_LENGTH      (64U) //64U //192U
#define CONFIG_EHSM_HW_V_OTP_VERSION_LENGTH     (0U) //128U //1024U

/**
 * @brief configuration for PKE support
 */
#define CONFIG_EHSM_HW_PKE

/**
 * @brief configuration for PKE HP support
 */
//#define CONFIG_EHSM_HW_PKE_HP

/**
 * @brief configuration for PKE LP support
 */
#define CONFIG_EHSM_HW_PKE_LP

/**
 * @brief configuration for PKE LP secure support
 */
//#define CONFIG_EHSM_HW_PKE_LP_SECURE

/**
 * @brief configuration for PKE secure support
 */
//#define CONFIG_EHSM_HW_PKE_SECURE

/**
 * @brief configuration for PKE UHP support
 */
//#define CONFIG_EHSM_HW_PKE_UHP

/**
 * @brief configuration for PKE UHP ECC support
 */
//#define CONFIG_EHSM_HW_PKE_UHP_ECC

/**
 * @brief configuration for hardware initialize ram support
 */
//#define CONFIG_EHSM_HW_RAM_INIT

/**
 * @brief configuration for SKE DMA mode support
 */
#define CONFIG_EHSM_HW_SKE_DMA

/**
 * @brief configuration for SKE HP support
 */
//#define CONFIG_EHSM_HW_SKE_HP

/**
 * @brief configuration for SKE LP support
 */
#define CONFIG_EHSM_HW_SKE_LP

/**
 * @brief configuration for SKE LP secure support
 */
//#define CONFIG_EHSM_HW_SKE_LP_SECURE

/**
 * @brief configuration for SKE secure port support
 */
#define CONFIG_EHSM_HW_SKE_SECURE_PORT

/**
 * @brief configuration for TRNG support
 */
#define CONFIG_EHSM_HW_TRNG

/**
 * @brief configuration for UTC TIME support
 */
#define CONFIG_EHSM_HW_UTC_TIME

/**
 * @brief configuration for HOST address size 64 bits
 */
//#define CONFIG_EHSM_HOST_ADDR_64BITS

/**
 * @brief configuration work frequency of eHSM
 */
#define CONFIG_EHSM_HW_V_WORK_FREQ                              10000000U

/**
 * @brief configuration for jtag debug authentication support
 */
#define CONFIG_EHSM_JTAG_DEBUG_AUTH

/**
 * @brief configuration for flash key backup support
 */
//#define CONFIG_EHSM_KMGR_BACKUP

/**
 * @brief configuration for batch_write_otp_key_attr support
 */
//#define CONFIG_EHSM_KMGR_BATCH_WRITE_OTP_K_ATTR

/**
 * @brief configuration for checking OTP key attributes
 */
#define CONFIG_EHSM_KMGR_CHECK_OTP_K_ATTR

/**
 * @brief configuration for key signature support
 */
//#define CONFIG_EHSM_KMGR_K_SIGNATURE

/**
 * @brief configuration for import/export plain key support
 */
#define CONFIG_EHSM_KMGR_PLAIN_K_IMPORT

/**
 * @brief Max authentication code size of EVITA key
 */
#define CONFIG_EHSM_KMGR_V_MAX_AUTH_CODE_SIZE                   (32U)

#define CONFIG_EHSM_KMGR_V_OTP_EXT_K_OFF                        (0x400)
#define CONFIG_EHSM_KMGR_V_OTP_EXT_K_SIZE                       (0x400)

/**
 * @brief configuration for flash key storage address
 */
#define CONFIG_EHSM_KMGR_V_FLASH_K_START_OFFSET                 (0x00)
#define CONFIG_EHSM_KMGR_V_FLASH_OTP_K_START_OFFSET             (CONFIG_EHSM_KMGR_V_OTP_EXT_K_OFF)

/**
 * @brief configuration for flash key number and size
 */
#define CONFIG_EHSM_KMGR_V_SHE_K_NUM                            (40)
#define CONFIG_EHSM_KMGR_V_SHE_K_SLOT_SIZE                      (48)
#define CONFIG_EHSM_KMGR_V_SHE_K_AREA_SIZE                      (0x800)
#define CONFIG_EHSM_KMGR_V_SHE_K_START_ADDR                     (CONFIG_EHSM_KMGR_V_FLASH_K_START_OFFSET)
#define CONFIG_EHSM_KMGR_V_SHE_K_END_ADDR                       (CONFIG_EHSM_KMGR_V_SHE_K_START_ADDR + \
                                                                 CONFIG_EHSM_KMGR_V_SHE_K_AREA_SIZE)

/**
 * @brief configuration for EVITA key number and size
 */
#define CONFIG_EHSM_KMGR_V_SYM_K_NUM                            (10)
#define CONFIG_EHSM_KMGR_V_SYM_K_SLOT_SIZE                      (200)
#define CONFIG_EHSM_KMGR_V_SYM_K_AREA_SIZE                      (0x800)
#define CONFIG_EHSM_KMGR_V_SYM_K_START_ADDR                     (CONFIG_EHSM_KMGR_V_SHE_K_END_ADDR)
#define CONFIG_EHSM_KMGR_V_SYM_K_END_ADDR                       (CONFIG_EHSM_KMGR_V_SYM_K_START_ADDR + \
                                                                 CONFIG_EHSM_KMGR_V_SYM_K_AREA_SIZE)

#define CONFIG_EHSM_KMGR_V_ECC_K_NUM                            (7)
#define CONFIG_EHSM_KMGR_V_ECC_K_SLOT_SIZE                      (256)
#define CONFIG_EHSM_KMGR_V_ECC_K_AREA_SIZE                      (0x800)
#define CONFIG_EHSM_KMGR_V_ECC_K_START_ADDR                     (CONFIG_EHSM_KMGR_V_SYM_K_START_ADDR)
#define CONFIG_EHSM_KMGR_V_ECC_K_END_ADDR                       (CONFIG_EHSM_KMGR_V_SYM_K_END_ADDR)

#define CONFIG_EHSM_KMGR_V_RSA_K_NUM                            (3)
#define CONFIG_EHSM_KMGR_V_RSA_K_SLOT_SIZE                      (600)
#define CONFIG_EHSM_KMGR_V_RSA_K_AREA_SIZE                      (0x800)
#define CONFIG_EHSM_KMGR_V_RSA_K_START_ADDR                     (CONFIG_EHSM_KMGR_V_SYM_K_START_ADDR)
#define CONFIG_EHSM_KMGR_V_RSA_K_END_ADDR                       (CONFIG_EHSM_KMGR_V_SYM_K_END_ADDR)

#define CONFIG_EHSM_KMGR_V_SM9_K_NUM                            (0)
#define CONFIG_EHSM_KMGR_V_SM9_K_SLOT_SIZE                      (200)
#define CONFIG_EHSM_KMGR_V_SM9_K_AREA_SIZE                      (0)
#define CONFIG_EHSM_KMGR_V_SM9_K_START_ADDR                     (CONFIG_EHSM_KMGR_V_SYM_K_START_ADDR)
#define CONFIG_EHSM_KMGR_V_SM9_K_END_ADDR                       (CONFIG_EHSM_KMGR_V_SYM_K_END_ADDR)

/**
 * @brief configuration for EVITA ram key number and size
 */
#define CONFIG_EHSM_KMGR_V_RAM_K_MEM_SIZE                       (2560) //2.5k
#define CONFIG_EHSM_KMGR_V_SYM_RAM_K_NUM                        (10)
#define CONFIG_EHSM_KMGR_V_ECC_RAM_K_NUM                        (5)
#define CONFIG_EHSM_KMGR_V_RSA_RAM_K_NUM                        (5)

/**
 * @brief configuration for eHSM log support
 */
#define CONFIG_EHSM_LOG

/**
 * @brief configuration for OTP key crc byte reverse support
 */
//#define CONFIG_EHSM_REVERSE_OTP_KEY_CRC_BYTE

/**
 * @brief configuration for soc upgrade and verify support
 */
#define CONFIG_EHSM_SOC_UPGRADE_AND_VERIFY

/**
 * @brief configuration for system data backup support
 */
//#define CONFIG_EHSM_SYS_DATA_BACKUP

/**
 * @brief configuration for debug with symmetric algorithms support
 */
//#define CONFIG_EHSM_SYS_DEBGU_AUTH_SYMMETRIC

/**
 * @brief PATCH for EHSM, iram space equal to rom space
 */
//#define CONFIG_EHSM_PATCH_IRAM_EQUAL_PROPORTION

/**
 * @brief configuration for getting EMU status support
 */
//#define CONFIG_EHSM_SYS_EMU_STATUS

/**
 * @brief configuration for handling EMU error
 */
//#define CONFIG_EHSM_SYS_EMU_HANDLER

/**
 * @brief configuration for self test support
 */
//#define CONFIG_EHSM_SYS_SELF_TEST

/**
 * @brief configuration for user auth key exist on KMU
 */
#define CONFIG_EHSM_USER_AUTH_KYE_IN_KMU

/**
 * @brief configuration for reverse otp data, the otp default bit is 0, it can be change to bit 1
 */
//#define CONFIG_EHSM_REVERSE_OTP_DATA

/**
 * @brief configuration for checking write protection of SHE keys when doing SHE debug authentication
 */
//#define CONFIG_EHSM_SHE_DBG_AUTH_CHECK_WT_PRT

/**
 * @brief configuration for she soc boot
 */
#define CONFIG_EHSM_SHE_SOC_BOOT

/**
 * @brief configuration for multicore and core number.
 */
//#define CONFIG_EHSM_MULTI_CORE
#define CONFIG_EHSM_SKE_CORE_NUM            1
#define CONFIG_EHSM_PKE_CORE_NUM            1
#define CONFIG_EHSM_HASH_CORE_NUM           1
#define CONFIG_EHSM_TRNG_CORE_NUM           1

/**
 * @brief configuration for key install used new mailbox/API format
 */
//#define CONFIG_EHSM_K_INSTALL_WITH_ATTR

/**
 * @brief configuration for TCM.
 */
//#define CONFIG_EHSM_TCM
//#define CONFIG_EHSM_TCM_FUNC_ADDR           (0x10028000)

/**
 * @brief configuration ehsm soft reset enable
 */
//#define CONFIG_EHSM_RESET_ENABLE

/**
 * @brief configuration for feature of main hook
 */
#define CONFIG_EHSM_ARCH_MAIN_HOOK

/**
 * @brief configuration for share memory whitch used for flash page buffer
 */
#define CONFIG_EHSM_ARCH_SHARE_MEM
/***********************************************************************************************************************
 *  Configuration for unitest
 *  named as CONFIG_EHSM_UNIT_TEST_xxx if it's a feature configuration
 *  named as CONFIG_UNIT_V_xx if it's a value configuration
 **********************************************************************************************************************/
/**
 * @brief config for unitest
 */
#define CONFIG_EHSM_UNIT_TEST

/***********************************************************************************************************************
 *  Configuration for other feature
 *  named as CONFIG_EHSM_xxx if it's a feature configuration
 *  named as CONFIG_EHSM_V_ if it's a value configuration
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* EHSM_INTCFG_IP_H */
