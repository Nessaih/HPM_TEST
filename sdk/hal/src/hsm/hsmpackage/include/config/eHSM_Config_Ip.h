#ifndef EHSM_CONFIG_IP_H
#define EHSM_CONFIG_IP_H

#ifdef __cplusplus
extern "C"{
#endif

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/**
 * @brief configuration for AUTOSAR support
 */
#define CONFIG_EHSM_AUTOSAR

/**
 * @brief configuration for EVITA support
 */
#define CONFIG_EHSM_EVITA

/**
 * @brief configuration for SHE support
 */
#define CONFIG_EHSM_SHE

/**
 * @brief configuration for SE support
 */
//#define CONFIG_EHSM_SE

/***********************************************************************************************************************
 *  Configuration for software architecture
 *  named as CONFIG_EHSM_ARCH_xxx if it's a feature configuration
 *  named as CONFIG_EHSM_ARCH_V_xxx if it's a value configuration
 **********************************************************************************************************************/
/**
 * @brief configuration for crypto object queue size
 */
#define CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_PKE_QUEUE_SIZE            10U
#define CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_TRNG_QUEUE_SIZE           10U
#define CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_HASH_QUEUE_SIZE           10U
#define CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_K_QUEUE_SIZE              10U
#define CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_SYSMGR_QUEUE_SIZE         10U
#define CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_SKE_QUEUE_SIZE            10U

/**
 * @brief command queue size in eHSM for each crypto object
 */
#define CONFIG_EHSM_ARCH_V_CMD_QUEUE_SIZE                       2U

/**
 * @brief time out for Synchronous command, millisecond.
 */
#define CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT                  0x1FFCFFFFU

/**
 * @brief time out for RSA key generation.
 */
#define CONFIG_EHSM_ARCH_V_RSA_K_CMD_TIMEOUT                    0x1FFFFFFFU

/**
 * @brief time out for Synchronous command, millisecond.
 */
#define CONFIG_EHSM_ARCH_V_MAILBOX_TIMEOUT                      100U

/**
 * @brief time out for jtag channel of mailbox, millisecond.
 */
#define CONFIG_EHSM_ARCH_V_JTAG_TIMEOUT                         1000U

/**
 * @brief configuration host driver using polling mechanism to read mailbox data, if not defined using interrupt to read
 * mailbox data
 */
#define CONFIG_EHSM_ARCH_HOST_MAILBOX_POLLING

/***********************************************************************************************************************
 *  Configuration for key management
 *  named as CONFIG_EHSM_KMGR_xxx if it's a feature configuration
 *  named as CONFIG_EHSM_KMGR_V_xxx if it's a value configuration
 **********************************************************************************************************************/
/**
 * @brief configuration for key element CRYPTO_KE_KEY_MATERIAL
 */
#define CONFIG_EHSM_KMGR_V_ASR_K_MATERIAL_PARTIAL_ACCESS                (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_K_MATERIAL_READ_ACCESS                   (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_K_MATERIAL_WRITE_ACCESS                  (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_K_MATERIAL_PERSIST                       (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_EXT_SHE_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_K_EXT_SHE_KEY_PARTIAL_ACCESS             (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_K_EXT_SHE_KEY_READ_ACCESS                (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_K_EXT_SHE_KEY_WRITE_ACCESS               (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_K_EXT_SHE_KEY_PERSIST                    (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_MAC_PROOF
 */
#define CONFIG_EHSM_KMGR_V_ASR_MAC_PROOF_PARTIAL_ACCESSRTIAL_ACCESS     (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_MAC_PROOF_READ_ACCESS                    (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_MAC_PROOF_WRITE_ACCESS                   (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_MAC_PROOF_PERSIST                        (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_PROOF
 */
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_PROOF_PARTIAL_ACCESS              (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_PROOF_READ_ACCESS                 (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_PROOF_WRITE_ACCESS                (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_PROOF_PERSIST                      (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_IV
 */
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_IV_PARTIAL_ACCESS                 (TRUE)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_IV_READ_ACCESS                    (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_IV_WRITE_ACCESS                   (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_IV_PERSIST                        (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_MAC_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_MAC_K_PARTIAL_ACCESS                     (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_MAC_K_READ_ACCESS                        (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_MAC_K_WRITE_ACCESS                       (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_MAC_K_PERSIST                            (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_K_PARTIAL_ACCESS                  (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_K_READ_ACCESS                     (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_K_WRITE_ACCESS                    (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_K_PERSIST                         (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_CURVE_ID
 */
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_CURVE_ID_PARTIAL_ACCESS           (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_CURVE_ID_READ_ACCESS              (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_CURVE_ID_WRITE_ACCESS             (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_CURVE_ID_PERSIST                  (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_CIPHER_ALG
 */
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_CIPHER_ALG_PARTIAL_ACCESS         (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_CIPHER_ALG_READ_ACCESS            (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_CIPHER_ALG_WRITE_ACCESS           (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_CIPHER_ALG_PERSIST                (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_KDF_ALG
 */
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_KDF_ALG_PARTIAL_ACCESS            (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_KDF_ALG_READ_ACCESS               (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_KDF_ALG_WRITE_ACCESS              (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_KDF_ALG_PERSIST                   (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_MAC_ALG
 */
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_MAC_ALG_PARTIAL_ACCESS            (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_MAC_ALG_READ_ACCESS               (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_MAC_ALG_WRITE_ACCESS              (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_MAC_ALG_PERSIST                   (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_MAC_ALG
 */
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_MAC_SIZE_PARTIAL_ACCESS           (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_MAC_SIZE_READ_ACCESS              (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_MAC_SIZE_WRITE_ACCESS             (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_MAC_SIZE_PERSIST                  (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_MAC_ALG
 */
#define CONFIG_EHSM_KMGR_V_ASR_AEAD_TAG_SIZE_PARTIAL_ACCESS             (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_AEAD_TAG_SIZE_READ_ACCESS                (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_AEAD_TAG_SIZE_WRITE_ACCESS               (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_AEAD_TAG_SIZE_PERSIST                    (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CIPHER_2NDKEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_2NDKEY_PARTIAL_ACCESS             (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_2NDKEY_READ_ACCESS                (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_2NDKEY_WRITE_ACCESS               (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CIPHER_2NDKEY_PERSIST                    (FALSE)

/**
 * @brief configuration for key element CYRPTO_KE_KEYEXCHANGE_SHAREDVALUE
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SHAREDVALUE_PARTIAL_ACCESS     (TRUE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SHAREDVALUE_READ_ACCESS        (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SHAREDVALUE_WRITE_ACCESS       (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SHAREDVALUE_PERSIST            (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_BASE
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_BASE_PARTIAL_ACCESS            (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_BASE_READ_ACCESS               (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_BASE_WRITE_ACCESS              (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_BASE_PERSIST                   (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_PRIVKEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PRIVKEY_PARTIAL_ACCESS         (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PRIVKEY_READ_ACCESS            (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PRIVKEY_WRITE_ACCESS           (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PRIVKEY_PERSIST                (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_OWNPUBKEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_OWNPUBKEY_PARTIAL_ACCESS       (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_OWNPUBKEY_READ_ACCESS          (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_OWNPUBKEY_WRITE_ACCESS         (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_OWNPUBKEY_PERSIST              (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_ALGORITHM
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_ALGORITHM_PARTIAL_ACCESS       (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_ALGORITHM_READ_ACCESS          (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_ALGORITHM_WRITE_ACCESS         (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_ALGORITHM_PERSIST              (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_PEERPUBKEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PEERPUBKEY_PARTIAL_ACCESS      (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PEERPUBKEY_READ_ACCESS         (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PEERPUBKEY_WRITE_ACCESS        (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PEERPUBKEY_PERSIST             (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_KEYINFO
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_KEYINFO_PARTIAL_ACCESS         (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_KEYINFO_READ_ACCESS            (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_KEYINFO_WRITE_ACCESS           (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_KEYINFO_PERSIST                (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_PUBTYPE
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PUBKTYPE_PARTIAL_ACCESS        (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PUBKTYPE_READ_ACCESS           (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PUBKTYPE_WRITE_ACCESS          (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_PUBKTYPE_PERSIST               (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_SM2_LOCALTMPKINFO
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_LOCALTMPKINFO_PARTIAL_ACCESS   (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_LOCALTMPKINFO_READ_ACCESS  (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_LOCALTMPKINFO_WRITE_ACCESS (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_LOCALTMPKINFO_PERSIST      (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_SM2_PEERTMPPUBK
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_PEERTMPPUBK_PARTIAL_ACCESS (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_PEERTMPPUBK_READ_ACCESS    (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_PEERTMPPUBK_WRITE_ACCESS   (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_PEERTMPPUBK_PERSIST        (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_SM2_S1_S2_VALUE
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_S1_S2_VALUE_PARTIAL_ACCESS (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_S1_S2_VALUE_READ_ACCESS    (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_S1_S2_VALUE_WRITE_ACCESS   (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_S1_S2_VALUE_PERSIST        (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_SM2_SA_SB_VALUE
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_SA_SB_VALUE_PARTIAL_ACCESS (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_SA_SB_VALUE_READ_ACCESS    (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_SA_SB_VALUE_WRITE_ACCESS   (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_SA_SB_VALUE_PERSIST        (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYEXCHANGE_SM2_ROLE
 */
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_ROLE_PARTIAL_ACCESS        (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_ROLE_READ_ACCESS           (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_ROLE_WRITE_ACCESS          (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KEXCHANGE_SM2_ROLE_PERSIST               (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_SIGNATURE_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_K_PARTIAL_ACCESS               (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_K_READ_ACCESS                  (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_K_WRITE_ACCESS                 (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_K_PERSIST                      (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_SIGNATURE_TIMESTAMPED
 */
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_TIMESTAMPED_PARTIAL_ACCESS     (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_TIMESTAMPED_READ_ACCESS        (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_TIMESTAMPED_WRITE_ACCESS       (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_TIMESTAMPED_PERSIST            (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_SIGNATURE_RSA_CRT_MODE
 */
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_RSA_CRT_MODE_PARTIAL_ACCESS    (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_RSA_CRT_MODE_READ_ACCESS       (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_RSA_CRT_MODE_WRITE_ACCESS      (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_SIGNATURE_RSA_CRT_MODE_PERSIST           (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYDERIVATION_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_K_PARTIAL_ACCESS             (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_K_READ_ACCESS                (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_K_WRITE_ACCESS               (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_K_PERSIST                    (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYDERIVATION_PASSWD
 */
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_PASSWD_PARTIAL_ACCESS        (TRUE)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_PASSWD_READ_ACCESS           (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_PASSWD_WRITE_ACCESS          (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_PASSWD_PERSIST               (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYDERIVATION_SALT
 */
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_SALT_PARTIAL_ACCESS          (TRUE)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_SALT_READ_ACCESS             (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_SALT_WRITE_ACCESS            (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_SALT_PERSIST                 (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYDERIVATION_ITERATIONS
 */
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_ITERATIONS_PARTIAL_ACCESS    (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_ITERATIONS_READ_ACCESS       (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_ITERATIONS_WRITE_ACCESS      (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_ITERATIONS_PERSIST           (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYDERIVATION_ALGORITHM
 */
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_ALGORITHM_PARTIAL_ACCESS     (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_ALGORITHM_READ_ACCESS        (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_ALGORITHM_WRITE_ACCESS       (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_ALGORITHM_PERSIST            (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYDERIVATION_KEYHANDLE
 */
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_KHANDLE_PARTIAL_ACCESS       (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_KHANDLE_READ_ACCESS          (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_KHANDLE_WRITE_ACCESS         (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_KHANDLE_PERSIST              (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYDERIVATION_TYPE
 */
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_TYPE_PARTIAL_ACCESS          (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_TYPE_READ_ACCESS             (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_TYPE_WRITE_ACCESS            (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KDERIVATION_TYPE_PERSIST                 (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYGENERATE_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_K_PARTIAL_ACCESS               (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_K_READ_ACCESS                  (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_K_WRITE_ACCESS                 (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_K_PERSIST                      (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYGENERATE_KEYINFO
 */
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_KINFO_PARTIAL_ACCESS           (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_KINFO_READ_ACCESS              (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_KINFO_WRITE_ACCESS             (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_KINFO_PERSIST                  (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYGENERATE_SEED
 */
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_SEED_PARTIAL_ACCESS            (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_SEED_READ_ACCESS               (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_SEED_WRITE_ACCESS              (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_SEED_PERSIST                   (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYGENERATE_ALGORITHM
 */
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_ALGORITHM_PARTIAL_ACCESS       (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_ALGORITHM_READ_ACCESS          (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_ALGORITHM_WRITE_ACCESS         (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_ALGORITHM_PERSIST              (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEYGENERATE_DH_KEY_INFO
 */
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_DH_K_INFO_PARTIAL_ACCESS       (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_DH_K_INFO_READ_ACCESS          (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_DH_K_INFO_WRITE_ACCESS         (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_KGENERATE_DH_K_INFO_PERSIST              (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_IMPORT_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_IMPORT_K_PARTIAL_ACCESS                  (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_IMPORT_K_READ_ACCESS                     (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_IMPORT_K_WRITE_ACCESS                    (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_IMPORT_K_PERSIST                         (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_IMPORTED_KEY_KEYHANDLE
 */
#define CONFIG_EHSM_KMGR_V_ASR_IMPORTED_K_KHANDLE_PARTIAL_ACCESS        (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_IMPORTED_K_KHANDLE_READ_ACCESS           (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_IMPORTED_K_KHANDLE_WRITE_ACCESS          (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_IMPORTED_K_KHANDLE_PERSIST               (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_EXPORT_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_EXPORT_K_PARTIAL_ACCESS                  (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_EXPORT_K_READ_ACCESS                     (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_EXPORT_K_WRITE_ACCESS                    (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_EXPORT_K_PERSIST                         (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_EXPORT_KEY_BLOB
 */
#define CONFIG_EHSM_KMGR_V_ASR_EXPORT_K_BLOB_PARTIAL_ACCESS             (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_EXPORT_K_BLOB_READ_ACCESS                (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_EXPORT_K_BLOB_WRITE_ACCESS               (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_EXPORT_K_BLOB_PERSIST                    (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_KEY_STATUS
 */
#define CONFIG_EHSM_KMGR_V_ASR_K_STATUS_PARTIAL_ACCESS                  (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_K_STATUS_READ_ACCESS                     (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_K_STATUS_WRITE_ACCESS                    (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_K_STATUS_PERSIST                         (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_REMOVE_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_REMOVE_K_PARTIAL_ACCESS                  (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_REMOVE_K_READ_ACCESS                     (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_REMOVE_K_WRITE_ACCESS                    (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_REMOVE_K_PERSIST                         (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_COPY_KEY_PARENT_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_COPY_K_PARENT_K_PARTIAL_ACCESS           (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_COPY_K_PARENT_K_READ_ACCESS              (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_COPY_K_PARENT_K_WRITE_ACCESS             (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_COPY_K_PARENT_K_PERSIST                  (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_COPY_KEY_TARGET_KEY_HANDLE
 */
#define CONFIG_EHSM_KMGR_V_ASR_COPY_K_TARGET_K_HANDLE_PARTIAL_ACCESS    (FALSE)
#define CONFIG_EHSM_KMGR_V_ASR_COPY_K_TARGET_K_HANDLE_READ_ACCESS       (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_COPY_K_TARGET_K_HANDLE_WRITE_ACCESS      (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_COPY_K_TARGET_K_HANDLE_PERSIST           (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CERTIFICATE_DATA
 */
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_DATA_PARTIAL_ACCESS          (TRUE)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_DATA_READ_ACCESS             (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_DATA_WRITE_ACCESS            (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_DATA_PERSIST                 (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CERTIFICATE_SUBJECT_PUBLIC_KEY
 */
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SUBJECT_PUBLIC_K_PARTIAL_ACCESS      (TRUE)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SUBJECT_PUBLIC_K_READ_ACCESS (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SUBJECT_PUBLIC_K_WRITE_ACCESS    (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SUBJECT_PUBLIC_K_PERSIST     (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CERTIFICATE_SIGNATURE
 */
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SIGNATURE_PARTIAL_ACCESS     (TRUE)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SIGNATURE_READ_ACCESS        (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SIGNATURE_WRITE_ACCESS       (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SIGNATURE_PERSIST            (FALSE)

/**
 * @brief configuration for key element CRYPTO_KE_CERTIFICATE_SIGNEDDATA
 */
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SIGNEDDATA_PARTIAL_ACCESS    (TRUE)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SIGNEDDATA_READ_ACCESS       (CRYPTO_RA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SIGNEDDATA_WRITE_ACCESS      (CRYPTO_WA_ALLOWED)
#define CONFIG_EHSM_KMGR_V_ASR_CERTIFICATE_SIGNEDDATA_PERSIST           (FALSE)

/***********************************************************************************************************************
 *  Configuration for other feature
 *  named as CONFIG_EHSM_xxx if it's a feature configuration
 *  named as CONFIG_EHSM_V_ if it's a value configuration
 **********************************************************************************************************************/
/**
 * @brief configuration for secure boot of SOC bl1.
 */
//#define CONFIG_EHSM_HOST_SECUREBOOT
#ifdef CONFIG_EHSM_HOST_SECUREBOOT
#define CONFIG_EHSM_HOST_V_IMAGE_ADDR                           0x1FFD0000UL
#endif

/**
 * @brief configuration for eHSM debug, enable by default. Use //#define CONFIG_EHSM_DEBUG if not needed.
 */
//#define CONFIG_EHSM_DEBUG

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

#endif /* EHSM_CONFIG_IP_H */
