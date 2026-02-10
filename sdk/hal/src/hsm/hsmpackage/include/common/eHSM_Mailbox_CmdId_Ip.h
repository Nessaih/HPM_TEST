#ifndef EHSM_MBOX_CMDID_IP_H_
#define EHSM_MBOX_CMDID_IP_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/*
 * EHSM Mailbox command ID list
 */

#define EHSM_CMD_WRITE_OTP_DATA                     0x00FEFF01U
#define EHSM_CMD_READ_OTP_DATA                      0x00FDFF02U

#define EHSM_CMD_GET_CHALLENGE                      0x00FCFF03U
#define EHSM_CMD_DEBUG_AUTHENCATION                 0x00FBFF04U

#define EHSM_CMD_IMAGE_UPGRADE                      0x00FAFF05U
#define EHSM_CMD_IMAGE_VERIFY                       0x00F9FF06U

/* command ID for secure boot services */
#define EHSM_CMD_SOC_BOOT_STATUS                    0x00F8FF07U

/* command ID for device key installlation services */
#define EHSM_CMD_FW_GET_RANDOM_KEY                  0x00F7FF08U
#define EHSM_CMD_FW_ENCRYPT_KEY                     0x00F6FF09U


#define EHSM_CMD_CLOSE_DEBUG                        0x00F1FF0EU

#ifdef CONFIG_EHSM_HW_UTC_TIME
/* command ID for secure clock services */
#define EHSM_CMD_CREATE_TIMER                       0xFFFE0001U
#define EHSM_CMD_CHECK_TIMER                        0xFFFD0002U
#define EHSM_CMD_SET_UTC_TIMER                      0xFFFC0003U
#define EHSM_CMD_GET_UTC_TIMER                      0xFFFB0004U
#define EHSM_CMD_GET_TICK_COUNT                     0xFFFA0005U
#endif

/* command ID for counter services */
#define EHSM_CMD_CREATE_COUNTER                     0xFFEF0010U
#define EHSM_CMD_READ_COUNTER                       0xFFEE0011U
#define EHSM_CMD_INCREASE_COUNTER                   0xFFED0012U
#define EHSM_CMD_DELETE_COUNTER                     0xFFEC0013U

/* command ID for system managenment services */
#define EHSM_CMD_SELF_TEST                          0xFFDF0020U
#define EHSM_CMD_GET_SHE_STATUS                     0xFFBF0040U
#define EHSM_CMD_MODULE_STATUS                      0xFFAF0050U
#define EHSM_CMD_GET_SHE_ID                         0xFF9F0060U


#define EHSM_CMD_RESET_FIRMWARE                     0xFFEF00EEU

/* command ID for SKE services */
#define EHSM_CMD_SYM_CIPHER                         0xFEFE0101U
#define EHSM_CMD_AEAD_GCM                           0xFEFD0102U
#define EHSM_CMD_AEAD_CCM                           0xFEFC0103U
#define EHSM_CMD_SYM_GEN_KEY                        0xFEFB0104U

/* command ID for MAC services */
#define EHSM_CMD_MAC                                0xFDFE0201U

/* command ID for HASH services */
#define EHSM_CMD_HASH                               0xFCFE0301U

/* command ID for PKE services */
#define EHSM_CMD_SM2_CIPHER                         0xFBFE0401U
#define EHSM_CMD_SM2_SIGN                           0xFBFD0402U
#define EHSM_CMD_SM2_GEN_KEY                        0xFBFC0403U
#define EHSM_CMD_RSA_CIPHER                         0xFAFE0501U
#define EHSM_CMD_RSA_SIGN                           0xFAFD0502U
#define EHSM_CMD_RSA_GEN_KEY                        0xFAFC0503U
#define EHSM_CMD_ECDSA                              0xF9FE0601U
#define EHSM_CMD_ECIES                              0xF9FD0602U
#define EHSM_CMD_ECCP_GEN_KEY                       0xF9FC0603U


/* command ID for key managment services */
#define EHSM_CMD_IMPORT_KEY                         0xF7FE0801U
#define EHSM_CMD_EXPORT_KEY                         0xF7FD0802U
#define EHSM_CMD_DERIVE_KEY                         0xF7FC0803U
#define EHSM_CMD_CREATE_DH_KEY                      0xF7FB0804U
#define EHSM_CMD_GET_PUB_FROM_PRIV                  0xF7FA0805U
#define EHSM_CMD_KEY_REMOVE                         0xF7F90806U
#define EHSM_CMD_KEY_STATUS                         0xF7F80807U
#define EHSM_CMD_SHE_LOAD_KEY                       0xF7F70808U
#define EHSM_CMD_SHE_LOAD_PLAIN_KEY                 0xF7F60809U
#define EHSM_CMD_SHE_RAM_KEY_EXPORT                 0xF7F5080AU
#define EHSM_CMD_COPY_EVITA_KEY                     0xF7F4080BU
#define EHSM_CMD_GEN_DH_KEY_PAIR                    0xF7F3080CU


/* command ID for RNG services */
#define EHSM_CMD_RNG_GENERATE                       0xF5FE0A01U

#define EHSM_CMD_CHANGE_LIFECYCLE                   0xF0FE0F01U
#define EHSM_CMD_CHANGE_CONTROL_FIELD               0xF0FD0F02U
#define EHSM_CMD_LOW_POWER                          0xF0F90F03U
#define EHSM_CMD_SET_BAUDRATE                       0xF0FB0F04U
#define EHSM_CMD_SENSOR_RESP_INIT                   0xF0FA0F05U

#ifdef CONFIG_EHSM_DEBUG
#define EHSM_CMD_GET_FW_LIFECYCLE                   0xFFFF0001U
#define EHSM_CMD_GET_FLASH_UTC_TIME                 0xFFFF0002U
#define EHSM_CMD_ERASE_OTP_DATA                     0xFFF3FF0CU
#define EHSM_CMD_ERASE_FLASH_DATA                   0xFFF2FF0DU
#endif

/* command ID for independent mailbox channnel */
#define EHSM_CMD_UART_COMMAND                       0xFF000001U

/* command ID for independent mailbox channnel */
#define EHSM_CMD_UART_COMMAND                       0xFF000001U


#ifdef CONFIG_EHSM_SOC_UPGRADE_AND_VERIFY
#define EHSM_CMD_SOC_IAMGE_UPGRADE_INIT                0x00DFFF20U
#endif

#define EHSM_CMD_SOC_IMAGE_VERIFY                   0x00DDFF22U

/* command ID for independent mailbox channnel */

/*
 * Macro definition of command parameter
 */
/* 12th byte */
#define EHSM_START                  (1U)
#define EHSM_UPDATE                 (2U)
#define EHSM_STREAMSTART            (3U)
#define EHSM_FINISH                 (4U)
#define EHSM_ONEPASS                (7U)

/* 13th byte */
#define EHSM_ENCRYPTION             (0U)
#define EHSM_DECRYPTION             (1U)
#define EHSM_MAC_GENERATION         (0U)
#define EHSM_MAC_VERIFICATION       (1U)
#define EHSM_SIGN_GENERATION        (1U)
#define EHSM_SIGN_VERIFICATION      (0U)
#define EHSM_INVALID_DIR            (0xFFU)

/* 14th byte */
#define EHSM_NOPADDING              (0U)
#define EHSM_RSASSA_PPS             (1U)
#define EHSM_PKCS7                  (2U)
#define EHSM_ONEWITHZEROS           (3U)

/* 15th byte */
#define EHSM_SM4_CTRDRBG            (0U)
#define EHSM_AES_CTRDRBG            (1U)

#define EHSM_DES                    (0U)
#define EHSM_TDES_128               (1U)
#define EHSM_TDES_192               (2U)
#define EHSM_AES_128                (5U)
#define EHSM_AES_192                (6U)
#define EHSM_AES_256                (7U)
#define EHSM_SM4                    (8U)

#define EHSM_SM3                    (0U)
#define EHSM_MD5                    (1U)
#define EHSM_SHA256                 (2U)
#define EHSM_SHA384                 (3U)
#define EHSM_SHA512                 (4U)
#define EHSM_SHA1                   (5U)
#define EHSM_SHA224                 (6U)
#define EHSM_SHA512_224             (7U)
#define EHSM_SHA512_256             (8U)
#define EHSM_SHA3_224               (9U)
#define EHSM_SHA3_256               (10U)
#define EHSM_SHA3_384               (11U)
#define EHSM_SHA3_512               (12U)
#define EHSM_INVALID_ALG            (0xFFU)

/* 16th byte */
#define EHSM_ECB_MODE               (1U)
#define EHSM_XTS_MODE               (2U)
#define EHSM_CBC_MODE               (3U)
#define EHSM_CFB_MODE               (4U)
#define EHSM_OFB_MODE               (5U)
#define EHSM_CTR_MODE               (6U)

#define EHSM_NONE_CRT               (0U)
#define EHSM_CRT_MODE               (1U)

#define EHSM_CMAC_MODE              (7U)
#define EHSM_CBC_MAC_MODE           (8U)
#define EHSM_GMAC_MODE              (9U)

/* 17th byte */
#define EHSM_NO_TIME_STAMP          (0U)
#define EHSM_USE_TIME_STAMP         (1U)
//Used in ECIES
//#define EHSM_X963_KDF             (0U)

/* 18th byte */
//Usde in ECIES
#define EHSM_XOR                    (0U)

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/*
 * Enum definition of command type
*/
typedef enum
{
    CMD_TYPE_SKE  =     1U,
    CMD_TYPE_MAC  =     2U,
    CMD_TYPE_HASH =     3U,
    CMD_TYPE_SM2  =     4U,
    CMD_TYPE_RSA  =     5U,
    CMD_TYPE_ECC  =     6U,
    CMD_TYPE_SM9  =     7U,
    CMD_TYPE_RNG  =     10U
} cmd_type_e;

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

#endif /* EHSM_MBOX_CMDID_IP_H_ */
