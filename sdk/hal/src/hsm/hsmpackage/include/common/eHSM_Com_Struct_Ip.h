#ifndef EHSM_COM_STRUCT_H_
#define EHSM_COM_STRUCT_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define OTP_CONTROL_FILED_BYTE_SIZE   (8U)

#define EHSM_SHE_M1_MAX_SIZE      (32U)
#define EHSM_SHE_M2_MAX_SIZE      (32U)
#define EHSM_SHE_M3_MAX_SIZE      (16U)
#define EHSM_SHE_M4_MAX_SIZE      (48U)
#define EHSM_SHE_M5_MAX_SIZE      (16U)

#define EHSM_EVITA_AUTH_VALUE_MAX_SIZE     (32U)
#define DEFAULT_RSAKEY_E_SIZE               (17U)

/* For CMD_GET_STATUS in SHE */
#define EHSM_GET_STATUS_SHE    (0x0U)
/* For Module_Status with type SBB in EVITA */
#define EHSM_GET_STATUS_SBB    (0x1U)
/* For Module_Status with type MEM in EVITA */
#define EHSM_GET_STATUS_MEM    (0x2U)
/* For Module_Status with type ERRORS in EVITA */
#define EHSM_GET_STATUS_ERRORS (0x4U)


/* For symmetric DES algorithm testing */
#define EHSM_SELF_TEST_SKE_DES      (0x1U << 0U)
/* For symmetric TDES algorithm testing */
#define EHSM_SELF_TEST_SKE_TDES     (0x1U << 1U)
/* For symmetric AES algorithm testing */
#define EHSM_SELF_TEST_SKE_AES      (0x1U << 2U)
/* For symmetric SM4 algorithm testing */
#define EHSM_SELF_TEST_SKE_SM4      (0x1U << 3U)

/* For asymmetric RSA algorithm testing */
#define EHSM_SELF_TEST_PKE_RSA      (0x1U << 8U)
/* For asymmetric ECC algorithms testing */
#define EHSM_SELF_TEST_PKE_ECC      (0x1U << 9U)
/* For asymmetric SM2 algorithm testing */
#define EHSM_SELF_TEST_PKE_SM2      (0x1U << 10U)
/* For asymmetric SM9 algorithm testing */
#define EHSM_SELF_TEST_PKE_SM9      (0x1U << 11U)

/* For hash algorithm testing */
#define EHSM_SELF_TEST_HASH_MD5     (0x1U << 16U)
#define EHSM_SELF_TEST_HASH_SHA1    (0x1U << 17U)
#define EHSM_SELF_TEST_HASH_SHA2    (0x1U << 18U)
#define EHSM_SELF_TEST_HASH_SHA3    (0x1U << 19U)
#define EHSM_SELF_TEST_HASH_SM3     (0x1U << 20U)
#define EHSM_SELF_TEST_HASH_SHA256  (0x1U << 21U)

/* For trng testing */
#define EHSM_SELF_TEST_TRNG         (0x1U << 24U)

/* For symmetric algorithms testing */
#define EHSM_SELF_TEST_SKE          (EHSM_SELF_TEST_SKE_DES | EHSM_SELF_TEST_SKE_TDES | \
                                    EHSM_SELF_TEST_SKE_AES | EHSM_SELF_TEST_SKE_SM4)

/* For asymmetric algorithms testing */
#define EHSM_SELF_TEST_PKE          (EHSM_SELF_TEST_PKE_RSA | EHSM_SELF_TEST_PKE_ECC | \
                                     EHSM_SELF_TEST_PKE_SM2 | EHSM_SELF_TEST_PKE_SM9)

/* For hash algorithms testing */
#define EHSM_SELF_TEST_HASH         (EHSM_SELF_TEST_HASH_MD5 | EHSM_SELF_TEST_HASH_SHA1 | EHSM_SELF_TEST_HASH_SHA2 | \
                                     EHSM_SELF_TEST_HASH_SHA3 | EHSM_SELF_TEST_HASH_SM3 | EHSM_SELF_TEST_HASH_SHA256)

/* For all testing */
#define EHSM_SELF_TEST_ALL          (EHSM_SELF_TEST_TRNG | \
                                    EHSM_SELF_TEST_SKE | \
                                    EHSM_SELF_TEST_PKE | \
                                    EHSM_SELF_TEST_HASH)

/*algorithm for fast cmac*/
#define EHSM_FAST_CMAC_AES128       1
#define EHSM_FAST_CMAC_SM4          2

/*fast cmac direction*/
#define EHSM_FAST_CMAC_GEN          0
#define EHSM_FAST_CMAC_VERFY        1

/*key type for fast cmac*/
#define EHSM_FAST_CMAC_SHE_KEY      EHSM_CMD_CIPHER_KEY_TYPE_SHE
#define EHSM_FAST_CMAC_EVITA_KEY    EHSM_CMD_CIPHER_KEY_TYPE_EVITA

#define SECURE_BOOT_TYPE_IMAGE_VERIFY (0x01U)
#define SECURE_BOOT_TYPE_SECURE_BOOT  (0x02U)

#define IMAGE_SIGNATURE_MAX_LENGTH    (256U)
#define IMAGE_PUBLIC_KEY_MAX_LENGTH   (64U+256U)

#define EHSM_CODE_VERIFY_FALG         (0x01)
#define SOC_CODE_VERIFY_FALG          (0x02)

#define CODE_VALID_FLAG               (0x8E97645DUL)
#define UPGRADE_VALID_FLAG            (0x71689BA2UL)

#define IMAGE_DECRYPT_CODE            (0x5A)
#define IMAGE_ENCRYPT_CODE            (0x5C)
#define IMAGE_ANALYSIS_CODE           (0x55)

#ifdef CONFIG_EHSM_SOC_UPGRADE_AND_VERIFY
#define SOC_CTX_INIT_DONE            (0x569AA965)
#endif

#define SOC_BOOT_TYPE_SEQUENTIAL  0x0U
#define SOC_BOOT_TYPE_PARALLEL    0x1U
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef struct
{
    /* Attributes of use flags, only the least significant 10 bits are valid */
    ehsm_uint16_t use_flags;
    /* Attributes of trnsp flags, only the least significant 2 bits are valid */
    ehsm_uint8_t trnsp_flags;
    /* Attributes of auth flags */
    ehsm_uint32_t auth_flag;
    /* size of auth value */
    ehsm_uint8_t auth_size;
    /*if value is 0 means element has auth_value data else indicates auth_value data exist in other element, NOTE now this flag only use on OTP key*/
    ehsm_uint16_t auth_value_exist_flags;
    /* Auth data, only valid when auth_flags isn't 0 */
    ehsm_uint8_t auth_value[EHSM_EVITA_AUTH_VALUE_MAX_SIZE];
} ehsm_key_flags_element_st;

typedef struct
{
    ehsm_key_flags_element_st sign;
    ehsm_key_flags_element_st verify;
    ehsm_key_flags_element_st encrypt;
    ehsm_key_flags_element_st decrypt;
    ehsm_key_flags_element_st timestamp;
    ehsm_key_flags_element_st secureboot;
    ehsm_key_flags_element_st securestorage;
    ehsm_key_flags_element_st dhkey;
    ehsm_key_flags_element_st utcsync;
    ehsm_key_flags_element_st transport;
    ehsm_key_flags_element_st remove;
} ehsm_key_usages_st;

#define EHSM_EVITA_KEY_TYPE_NVM 0U
#define EHSM_EVITA_KEY_TYPE_RAM 1U
typedef ehsm_uint8_t ehsm_key_mem_type_e;

/*use key handle to calculate DH key.*/
#define EHSM_DH_MODE_KEY_HANDLE  0U
/*use remote public key to calculate DH key*/
#define EHSM_DH_MODE_KEY_PUB_KEY 1U
/*use public key directly, not in evita mode */
#define EHSM_DH_MODE_RAW_PUB_KEY 2U
typedef ehsm_uint8_t ehsm_dh_mode_e;

/*common RSA key.*/
#define EHSM_RSA_KEY_TYPE_COMMON 0U
/* CRT key */
#define EHSM_RSA_KEY_TYPE_CRT    1U
typedef ehsm_uint8_t ehsm_rsa_key_type_e;

#define SM2_KEY_EXCHANGE_ROLE_SPONSOR  0U
#define SM2_KE_EXCHANGE_ROLE_RESPONSOR 1U
typedef ehsm_uint8_t sm2_key_exchange_role_e;

#define EHSM_UART_BAUDRATE_9600    1U
#define EHSM_UART_BAUDRATE_19200   2U
#define EHSM_UART_BAUDRATE_38400   3U
#define EHSM_UART_BAUDRATE_57600   4U
#define EHSM_UART_BAUDRATE_115200  5U
#define EHSM_UART_BAUDRATE_INVALID 6U
typedef ehsm_uint8_t ehsm_uart_baudrate_e;

typedef struct ehsm_create_random_key_param
{
    /** reference to target algorithm for key
    * generation/usage based on RNG outputs, otherwise = 0 (cf. hardware
    interface
    * data structures) */
    ehsm_uint32_t target_algorithm_identifier;
    /* key size */
    ehsm_uint32_t key_size;
#ifdef CONFIG_EHSM_HW_UTC_TIME
    /* key life limitation as UTC */
    ehsm_uint32_t valid_until;
#endif
    /* EVITA key type non-volatile or RAM */
    ehsm_key_mem_type_e type;
    /* key usages size */
    ehsm_uint32_t key_element_size;
    /* key usages data */
    ehsm_key_flags_element_st *key_element_data;
    /*p, q, g is used for DH key pair generation*/
    ehsm_uint8_t *p;
    ehsm_uint32_t p_size;
    ehsm_uint8_t *q;
    ehsm_uint32_t q_size;
    ehsm_uint8_t *g;
    ehsm_uint32_t g_size;
    /* key handle of the created key */
    ehsm_uint32_t key_handle;
} ehsm_create_random_key_param_st;

#define CRYPTO_KEY_DERIVE_USER_PASSWD 1U
#define CRYPTO_KEY_DERIVE_USER_KEYHANDLE 2U
typedef ehsm_uint8_t crypto_key_derive_type_e;
typedef struct ehsm_key_derived_param
{
    /* function identifier for key derivation */
    ehsm_uint8_t key_deriv_func;
    /* key size */
    ehsm_uint32_t key_size;
#ifdef CONFIG_EHSM_HW_UTC_TIME
    /* key life limitation as UTC time */
    ehsm_uint32_t valid_until;
#endif
    /* EVITA key type non-volatile or RAM */
    ehsm_key_mem_type_e type;
    /* key usages size */
    ehsm_uint32_t key_element_size;
    /* key usages data */
    ehsm_key_flags_element_st *key_element_data;
    crypto_key_derive_type_e derive_type;
    ehsm_uint8_t *passwd;
    ehsm_uint32_t passwd_size;
    /*iteration times, used for PBKDF2*/
    ehsm_uint32_t itera_times;
    /* refers to the parent key for key derivation */
    ehsm_uint32_t parent_key_handle;
    /* size of parent key usage authorization value */
    ehsm_uint32_t parent_key_author_size;
    /* parent key usage authorization value */
    ehsm_uint8_t *parent_key_author_value;
    /* size of cryptographic salt */
    ehsm_uint32_t salt_size;
    /* random data for cryptographic salt */
    ehsm_uint8_t *salt_data;
    /* return key handle of derived key */
    ehsm_uint32_t key_handle;
} ehsm_key_derived_param_st;

typedef struct
{
    /* Reference to the key used for transport protection */
    ehsm_uint32_t transport_key_handle;
    /* Size of transport key usage authorization */
    ehsm_uint32_t transport_key_author_size;
    /* Transport key usage authorization (i.e., password) */
    ehsm_uint8_t *transport_key_author_value;
    /* Reference to the key used for authenticity code verification(use_flag =
    verify) */
    ehsm_uint32_t authenticity_key_handle;
    /* Size of authenticity key usage authorization */
    ehsm_uint32_t authenticity_key_author_size;
    /* Authenticity key usage authorization (i.e., password) */
    ehsm_uint8_t *authenticity_key_author_value;
    /* key type non-volatile or RAM */
    ehsm_key_mem_type_e type;
    /* Given encrypted key blob size */
    ehsm_uint32_t encrypted_key_size;
    /* Given encrypted key blob */
    ehsm_uint8_t *encrypted_key;
    /* Size of key authenticity code (signature or MAC) created by transport key
    */
    ehsm_uint32_t key_auth_code_size;
    /* Key authenticity code */
    ehsm_uint8_t *key_auth_code;
    /* return key handle of created key */
    ehsm_uint32_t key_handle;
} ehsm_evita_key_import_st;

typedef struct ehsm_evita_key_export
{
    /* Refers to the key to be export */
    ehsm_uint32_t key_handle;
    /* Use flag bit */
    ehsm_uint32_t use_flags;
    /* Reference to the key used for transport protection */
    ehsm_uint32_t transport_key_handle;
    /* Size of transport key usage authorization */
    ehsm_uint32_t transport_key_author_size;
    /* Transport key usage authorization (i.e., password) */
    ehsm_uint8_t *transport_key_author_value;
    /* Reference to the key used for authenticity code verification(use_flag =
    verify) */
    ehsm_uint32_t authenticity_key_handle;
    /* Size of authenticity key usage authorization */
    ehsm_uint32_t authenticity_key_author_size;
    /* Authenticity key usage authorization (i.e., password) */
    ehsm_uint8_t *authenticity_key_author_value;
    /* [in/out] Returned encrypted key blob size */
    ehsm_uint32_t encrypted_key_size;
    /* [in/out] Returned encrypted key blob */
    ehsm_uint8_t *encrypted_key;
    /* [in/out] Size of key authenticity code (signature or MAC) created by
    transport key */
    ehsm_uint32_t key_auth_code_size;
    /* [in/out] Key authenticity code */
    ehsm_uint8_t *key_auth_code;
} ehsm_evita_key_export_st;

/* extra parameters of SM2 for key exchange*/
typedef struct sm2_ext_param
{
    ehsm_uint8_t sm2_role;
    ehsm_uint8_t *peer_pubkey;
    ehsm_uint8_t *peer_temp_pubkey;
    ehsm_uint8_t *s1_s2_value;
    ehsm_uint8_t *sa_sb_value;
    ehsm_uint32_t local_tmp_key_handle;
    ehsm_uint32_t local_tmp_key_auth_size;
    ehsm_uint8_t *local_tmp_key_auth_value;
}sm2_ext_param_st;

typedef struct ehsm_create_dh_key_param
{
    /* reference to target algorithm for
    keygeneration/usage based on DH outputs, otherwise = 0 */
    ehsm_uint32_t target_algorithm_identifier;
    /* Mode for creating DH key, refers to ehsm_dh_mode_e */
    ehsm_uint8_t dh_mode;
    /* algorithm of parent key */
    ehsm_uint8_t parent_alg;
    /* if algorithm_identifier = 0 then define key length */
    ehsm_uint32_t key_size;
#ifdef CONFIG_EHSM_HW_UTC_TIME
    /* define key life limitation as UTC time */
    ehsm_uint32_t valid_until;
#endif
    /* EVITA key type */
    ehsm_key_mem_type_e type;
    /* key usages size */
    ehsm_uint32_t key_element_size;
    /* key usages data */
    ehsm_key_flags_element_st *key_element_data;
    /* refers to internal private key that will be used by DH */
    ehsm_uint32_t local_key_handle;
    /* size of local key usage authorization value (0 for none) */
    ehsm_uint32_t local_key_auth_size;
    /* local key usage authorization (i.e.,password) */
    ehsm_uint8_t *local_key_auth_value;
    /* refers to public remote key that willbe used by DH */
    ehsm_uint32_t remote_key_handle;
    /* size of remote key usage authorization value (0 for none) */
    ehsm_uint32_t remote_key_auth_or_pub_key_size;
    /* remote key usage authorization (i.e.,password) */
    ehsm_uint8_t *remote_key_auth_value_or_pub_key;
    /* return key handle of DH-calculated shared secret for later use in other
    functions */
    ehsm_uint32_t key_handle;
    ehsm_uint8_t *ss_addr;
    sm2_ext_param_st sm2_ext_para;
} ehsm_create_dh_key_param_st;

typedef struct ehsm_get_pub_from_priv_param
{
    /* The key for calculating public key */
    ehsm_uint32_t key_handle;
    /* size of local key usage authorization value (0 for none) */
    ehsm_uint32_t key_auth_size;
    /* local key usage authorization (i.e.,password) */
    ehsm_uint8_t *key_auth_value;
    /* local key algorithm id (only be used in OTP key and only support sm2/ecc) */
    ehsm_uint32_t key_alg_id;
    /*Address of the returned public key.*/
    ehsm_uint8_t *public_key_addr;
    /*Size of public_key_addr Pointer buffer.*/
    ehsm_uint32_t public_key_buffer_size;
    /*Size of the returned public key.*/
    ehsm_uint32_t public_key_size;
} ehsm_get_pub_from_priv_param_st;

typedef struct ehsm_key_remove_param
{
    ehsm_uint32_t key_handle;
    ehsm_uint8_t *key_auth_value;
    ehsm_uint32_t key_auth_size;
} ehsm_key_remove_param_st;

typedef struct ehsm_key_status_param
{
    /* reference to key whose status is to be returned */
    ehsm_uint32_t key_handle;
    /* reference to key that should sign the returned key status (NULL = w/o
    signature) */
    ehsm_uint32_t certification_key_handle;
    /* size of certification key usage authorization */
    ehsm_uint32_t certification_key_auth_size;
    /* certification key usage authorization (i.e. password) */
    ehsm_uint8_t *certification_key_auth_value;
    /* [out] size of (certified) key status */
    ehsm_uint32_t key_status_size;
    /* [out] (certified) key status = { public info about key || signature
    (optional) } */
    ehsm_uint8_t *key_status;
    /* [in] size of key status buffer*/
    ehsm_uint32_t key_status_buffer_size;
} ehsm_key_status_param_st;

typedef struct ehsm_key_copy_param
{
    ehsm_uint32_t parent_key_handle;
    ehsm_uint8_t *key_auth_value;
    ehsm_uint32_t key_auth_size;
    ehsm_uint32_t target_key_handle;
    /* key usages size */
    ehsm_uint32_t key_element_size;
    /* key usages data */
    ehsm_key_flags_element_st *key_element_data;
} ehsm_key_copy_param_st;

typedef struct ehsm_she_key_param
{
    ehsm_uint8_t m1[EHSM_SHE_M1_MAX_SIZE];
    ehsm_uint8_t m2[EHSM_SHE_M2_MAX_SIZE];
    ehsm_uint8_t m3[EHSM_SHE_M3_MAX_SIZE];
    ehsm_uint8_t m4[EHSM_SHE_M4_MAX_SIZE];
    ehsm_uint8_t m5[EHSM_SHE_M5_MAX_SIZE];
    /* [in] is she+ or she */
    ehsm_bool_t she_ext_flag;
} ehsm_she_key_param_st;

/* used by host driver */
typedef struct ehsm_she_key_host_param
{
    ehsm_uint8_t *m1;
    ehsm_uint8_t *m2;
    ehsm_uint8_t *m3;
    ehsm_uint8_t *m4;
    ehsm_uint8_t *m5;
    /* [in] is she+ or she */
    ehsm_bool_t she_ext_flag;
} ehsm_she_key_host_param_st;

typedef struct ehsm_she_plain_key_param
{
    ehsm_uint8_t key_data[16];
} ehsm_she_plain_key_param_st;

typedef struct ehsm_she_plain_key_host_param
{
    ehsm_uint8_t *key_data;
} ehsm_she_plain_key_host_param_st;

typedef struct ehsm_keyexchange_key_info
{
    /* key handle that will be used by DH */
    ehsm_uint32_t key_handle;
    /* size of key usage authorization value (0 for none) */
    ehsm_uint32_t key_auth_size;
    /* key usage authorization (i.e.,password) */
    ehsm_uint8_t key_auth_value[32];
} ehsm_keyexchange_key_info_st;

typedef struct ehsm_crypto_randomgenerate_param{
    ehsm_uint8_t algorithm;
    ehsm_uint32_t random_data_addr;
    ehsm_uint32_t request_size;
}ehsm_crypto_randomgenerate_param_st;


#define EHSM_CHALLENGE_TYPE_INVALID    0U
#define EHSM_CHALLENGE_TYPE_TIME_SYNC  1U
#define EHSM_CHALLENGE_TYPE_EHSM_DEBUG 2U
#define EHSM_CHALLENGE_TYPE_SHE_DEBUG  3U
#define EHSM_CHALLENGE_TYPE_SOC_DEBUG  4U
#define EHSM_CHALLENGE_TYPE_USER_AUTH  5U
#define EHSM_CHALLENGE_TYPE_MAX        6U
typedef ehsm_uint8_t ehsm_challenge_type_e;

#define EHSM_DEBUG_AUTH_ALG_SM2_WITH_SM3 1U
#define EHSM_DEBUG_AUTH_ALG_ECCSECP256R1_WITH_SHA256 2U
#define EHSM_DEBUG_AUTH_ALG_SM4_CMAC 3U
#define EHSM_DEBUG_AUTH_ALG_AES128_CMAC 4U
typedef ehsm_uint8_t ehsm_debug_auth_alg_e;

typedef struct
{
    ehsm_challenge_type_e type;
    ehsm_uint32_t size;
    ehsm_uint8_t *buf;
}ehsm_get_challenge_st;

typedef struct
{
    ehsm_challenge_type_e type;
    ehsm_uint32_t signature_size;
    ehsm_uint8_t *signature;
    ehsm_debug_auth_alg_e alg;
    ehsm_uint32_t public_key_size;
    ehsm_uint8_t *public_key;
}ehsm_debug_auth_st;

#define EHSM_FW_RANDOM_KEY_TYPE_SYMMETRIC_KEY         0x01U
#define EHSM_FW_RANDOM_KEY_TYPE_SM2_PRIVATE_KEY       0x02U
#define EHSM_FW_RANDOM_KEY_TYPE_SECP256R1_PRIVATE_KEY 0x04U
typedef ehsm_uint8_t ehsm_fw_random_key_type_e;

#define EHSM_FW_RANDOM_KEY_SLOT_DEVICE_ROOT_KEY   0x01U
#define EHSM_FW_RANDOM_KEY_SLOT_SOC_FW_VERIFY_KEY 0x02U
#define EHSM_FW_RANDOM_KEY_SLOT_SOC_ENC_KEY       0x04U
#define EHSM_FW_RANDOM_KEY_SLOT_SOC_PRIVATE_KEY   0x08U
#define EHSM_FW_RANDOM_KEY_SLOT_USER_ROOT_KEY     0x10U
typedef ehsm_uint8_t ehsm_fw_random_key_slot_e;

#define EHSM_FW_ENCRYPT_KEY_TYPE_SYMMETRIC_KEY   0x01U
#define EHSM_FW_ENCRYPT_KEY_TYPE_PUBLIC_KEY_HASH 0x02U
typedef ehsm_uint8_t ehsm_fw_encrypt_key_type_e;

#define EHSM_FW_ENCRYPT_KEY_SLOT_SOC_DEBUG_KEY          0x01U
#define EHSM_FW_ENCRYPT_KEY_SLOT_SOC_FW_VERIFY_KEY      0x02U
#define EHSM_FW_ENCRYPT_KEY_SLOT_SOC_UPGRADE_ENC_KEY    0x04U
#define EHSM_FW_ENCRYPT_KEY_SLOT_SOC_UPGRADE_VERIFY_KEY 0x08U
#define EHSM_FW_ENCRYPT_KEY_SLOT_USER_DEBUG_KEY         0x10U
typedef ehsm_uint8_t ehsm_fw_encrypt_key_slot_e;

typedef struct ehsm_fw_random_key
{
    ehsm_fw_random_key_type_e key_type;
    ehsm_fw_random_key_slot_e key_slot;
}ehsm_fw_random_key_st;

typedef struct ehsm_fw_encrypt_key
{
    ehsm_fw_encrypt_key_type_e key_type;
    ehsm_fw_encrypt_key_slot_e key_slot;
    ehsm_uint8_t *key_data;
    ehsm_uint32_t key_size;
}ehsm_fw_encrypt_key_st;

#define EHSM_IMAGE_PROCESS_MODE_INIT    0x01U
#define EHSM_IMAGE_PROCESS_MODE_UPDATE  0x02U
#define EHSM_IMAGE_PROCESS_MODE_FINISH  0x04U
#define EHSM_IMAGE_PROCESS_MODE_ONEPASS 0x08U
typedef ehsm_uint8_t ehsm_image_process_mode_e;

typedef struct ehsm_image
{
    ehsm_image_process_mode_e process_mode;
    ehsm_uint8_t *image;
    ehsm_uint32_t image_size;
    ehsm_uint8_t *storage;
    ehsm_uint32_t storage_size;
    ehsm_uint8_t *ctx;
    ehsm_uint32_t ctx_size;
} ehsm_image_upgrade_st;

typedef struct
{
    ehsm_image_process_mode_e process_mode;
    ehsm_uint8_t type;
    ehsm_uint8_t *image;
    ehsm_uint32_t image_size;
    ehsm_uint8_t *storage;
    ehsm_uint32_t storage_size;
    ehsm_uint8_t *ctx;
    ehsm_uint32_t ctx_size;
} ehsm_image_verify_st;

typedef struct soc_image_upgrade_input
{
    ehsm_uint8_t storage_alg;
    ehsm_uint8_t upgrade_alg;
    ehsm_uint8_t storage_encryption_flag;
    ehsm_uint8_t upgrade_decryption_flag;
    ehsm_uint8_t process_mode;
    ehsm_uint8_t check_version_flag;
    ehsm_uint8_t rev[2];
    ehsm_uint8_t upgrade_sign_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t upgrade_sign_size[4];
    ehsm_uint8_t upgrade_version_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t upgrade_version_size[4];
    ehsm_uint8_t upgrade_image_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t upgrade_image_size[4];
    ehsm_uint8_t storage_image_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t storage_image_size[4];
    ehsm_uint8_t mac_sign_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t mac_sign_size[4];
    ehsm_uint8_t upgrade_pubkey_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t upgrade_pubkey_size[4];
    ehsm_uint8_t upgrade_iv_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t upgrade_iv_size[4];
    ehsm_uint8_t storage_iv_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t storage_iv_size[4];
    ehsm_uint8_t header_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t header_size[4];
    ehsm_uint8_t ctx_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t ctx_size[4];
}ehsm_soc_image_upgrade_input_st;

typedef struct soc_image_upgrade_info
{
    ehsm_uint8_t *cmd_input;
    ehsm_uint8_t storage_alg;
    ehsm_uint8_t upgrade_alg;
    ehsm_uint8_t storage_encryption_flag;
    ehsm_uint8_t upgrade_decryption_flag;
    ehsm_uint8_t process_mode;
    ehsm_uint8_t check_version_flag;
    ehsm_uint8_t *upgrade_sign;
    ehsm_uint32_t upgrade_sign_size;
    ehsm_uint8_t *upgrade_version;
    ehsm_uint32_t upgrade_version_size;
    ehsm_uint8_t *upgrade_image;
    ehsm_uint32_t upgrade_image_size;
    ehsm_uint8_t *storage_image;
    ehsm_uint32_t storage_image_size;
    ehsm_uint8_t *mac_sign;
    ehsm_uint32_t mac_sign_size;
    ehsm_uint8_t *upgrade_pubkey;
    ehsm_uint32_t upgrade_pubkey_size;
    ehsm_uint8_t *upgrade_iv;
    ehsm_uint32_t upgrade_iv_size;
    ehsm_uint8_t *storage_iv;
    ehsm_uint32_t storage_iv_size;
    ehsm_uint8_t *header;
    ehsm_uint32_t header_size;
}ehsm_soc_image_upgrade_info_st;

typedef struct
{
    ehsm_uint8_t  rev1[1];
    ehsm_uint8_t  type;
    ehsm_uint8_t  storage_alg;
    ehsm_uint8_t  need_encryption;
    ehsm_uint8_t *pubkey_addr;
    ehsm_uint32_t pubkey_size;
    ehsm_uint8_t  *image_addr;
    ehsm_uint32_t image_size;
    ehsm_uint8_t  *header_addr;
    ehsm_uint32_t header_size;
    ehsm_uint8_t  *encrypt_iv_addr;
    ehsm_uint32_t encrypt_iv_size;
    ehsm_uint8_t  *sign_addr;
    ehsm_uint32_t sign_size;
} ehsm_soc_image_verify_st;

typedef struct soc_image_verify_input
{
    ehsm_uint8_t update_version_flag;
    ehsm_uint8_t type;
    ehsm_uint8_t storage_alg;
    ehsm_uint8_t storage_encryption_flag;
    ehsm_uint8_t version_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t version_size[4];
    ehsm_uint8_t pubkey_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t pubkey_size[4];
    ehsm_uint8_t storage_image_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t storage_image_size[4];
    ehsm_uint8_t header_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t header_size[4];
    ehsm_uint8_t storage_iv_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t storage_iv_size[4];
    ehsm_uint8_t storage_sign_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t storage_sign_size[4];
}ehsm_soc_image_verify_input_st;

typedef struct soc_image_verify_info
{
    ehsm_uint8_t *cmd_input;
    ehsm_uint8_t update_version_flag;
    ehsm_uint8_t type;
    ehsm_uint8_t storage_alg;
    ehsm_uint8_t storage_encryption_flag;
    ehsm_uint8_t *version;
    ehsm_uint32_t version_size;
    ehsm_uint8_t *pubkey;
    ehsm_uint32_t pubkey_size;
    ehsm_uint8_t *storage_image;
    ehsm_uint32_t storage_image_size;
    ehsm_uint8_t *header;
    ehsm_uint32_t header_size;
    ehsm_uint8_t *storage_iv;
    ehsm_uint32_t storage_iv_size;
    ehsm_uint8_t *storage_sign;
    ehsm_uint32_t storage_sign_size;
}ehsm_soc_image_verify_info_st;

#define CODE_VERIFY_ALG_RSA         0U
#define CODE_VERIFY_ALG_SM2         1U
#define CODE_VERIFY_ALG_AES128_CMAC 2U
#define CODE_VERIFY_ALG_SM4_CMAC    3U
#define CODE_VERIFY_ALG_INVALID     255U
typedef ehsm_uint8_t ehsm_code_verify_alg_e;

#define CODE_UPGRADE_ALG_RSA         0U
#define CODE_UPGRADE_ALG_SM2         1U
#define CODE_UPGRADE_ALG_AES128_GCM  2U
#define CODE_UPGRADE_ALG_SM4_GCM     3U
#define CODE_UPGRADE_ALG_AES128_CMAC 4U
#define CODE_UPGRADE_ALG_SM4_CMAC    5U
#define CODE_UPGRADE_ALG_INVALID     255U
typedef ehsm_uint8_t ehsm_code_upgrade_alg_e;
typedef struct {
    ehsm_uint8_t *p;
    ehsm_uint32_t p_size;
    ehsm_uint8_t *q;
    ehsm_uint32_t q_size;
    ehsm_uint8_t *g;
    ehsm_uint32_t g_size;
}ehsm_gen_dh_key_param_st;

typedef struct {
    ehsm_uint32_t algo_id;
    union{
        /* rsa e bit length*/
        ehsm_uint16_t rsa_e_bit_size;
        /* dh key param*/
        ehsm_gen_dh_key_param_st dh_key_param;
    } param;
}ehsm_gen_key_param_st;

typedef struct ehsm_create_evita_key_param
{
    /* key param*/
    ehsm_gen_key_param_st gen_key_param;
    /* key size */
    ehsm_uint16_t key_size;
#ifdef CONFIG_EHSM_HW_UTC_TIME
    /* key life limitation as UTC */
    ehsm_uint32_t valid_until;
#endif
    /* EVITA key type non-volatile or RAM */
    ehsm_key_mem_type_e type;
    /* key usages size */
    ehsm_uint32_t key_element_size;
    /* key usages data */
    ehsm_key_flags_element_st *key_element_data;
    /* key handle of the created key */
    ehsm_uint32_t key_handle;
}ehsm_create_evita_key_param_st;

#define EHSM_SM9_EXCHG_KEY_ROLE_SELF 1U
#define EHSM_SM9_EXCHG_KEY_ROLE_PEER 2U
typedef ehsm_uint8_t ehsm_SM9_exchg_key_role_e;

#define EHSM_GEN_SM9_SIGN_MASTER_KEY    0U
#define EHSM_GEN_SM9_ENC_MASTER_KEY     1U
#define EHSM_GEN_SM9_EXCHG_MASTER_KEY   2U
#define EHSM_GEN_SM9_INVALID_MASTER_KEY 3U
typedef ehsm_uint8_t ehsm_sm9_master_key_type_e;

#define EHSM_GEN_SM9_SIGN_USERPRIV_KEY    0U
#define EHSM_GEN_SM9_ENC_USERPRIV_KEY     1U
#define EHSM_GEN_SM9_EXCHG_USERPRIV_KEY   2U
#define EHSM_GEN_SM9_EXCHG_USERTMP_KEY    3U
#define EHSM_GEN_SM9_INVALID_USERPRIV_KEY 4U
typedef ehsm_uint8_t ehsm_sm9_user_privkey_type_e;

#define EHSM_GEN_SM9_MASTER_KEY  0U
#define EHSM_GEN_SM9_PRIV_KEY    1U
#define EHSM_GEN_SM9_INVALID_KEY 2U
typedef ehsm_uint8_t ehsm_gen_sm9_key_type_e;

typedef struct ehsm_gen_sm9_master_key_param
{
    ehsm_sm9_master_key_type_e master_key_type;
} ehsm_gen_sm9_master_key_param_st;

typedef struct ehsm_gen_sm9_userpriv_key_param
{
    ehsm_sm9_user_privkey_type_e priv_key_type;
    /* address of user ID*/
    ehsm_uint8_t *user_id_value;
    /* size of user ID*/
    ehsm_uint32_t user_id_size;
    /*key storage location */
    ehsm_key_mem_type_e type;
    ehsm_uint8_t with_pubkey;
    /*public key of KGC */
    ehsm_uint8_t kgc_pubkey[128];
} ehsm_gen_sm9_userpriv_key_param_st;

typedef struct ehsm_gen_sm9_key_param
{
    ehsm_gen_sm9_key_type_e sm9_key_type;
    ehsm_uint8_t hid;
    ehsm_uint8_t rev[3];
    union {
        ehsm_gen_sm9_master_key_param_st master_key;
        ehsm_gen_sm9_userpriv_key_param_st priv_key;
    } key_param;
    ehsm_uint32_t key_handle;
} ehsm_gen_sm9_key_param_st;

//parameters for SM9 exchange key
typedef struct ehsm_exchange_sm9_key_param
{
    ehsm_key_mem_type_e type;
    ehsm_SM9_exchg_key_role_e role;
    ehsm_uint32_t key_size;
    ehsm_uint32_t user_tmp_key_handle;
    ehsm_uint32_t user_priv_key_handle;
    const ehsm_uint8_t *peer_tmp_pub;
    const ehsm_uint8_t *kgc_pub_key;
    const ehsm_uint8_t *fp12g;
    const ehsm_uint8_t *self_id;
    const ehsm_uint8_t *peer_id;
    ehsm_uint32_t self_id_size;
    ehsm_uint32_t peer_id_size;
    ehsm_uint8_t *s1_s2;
    ehsm_uint8_t *sa_sb;
    ehsm_uint32_t key_handle;
} ehsm_exchange_sm9_key_param_st;

typedef struct ehsm_sm9_wrap_key_param
{
    const ehsm_uint8_t *user_id;
    ehsm_uint32_t id_size;
    const ehsm_uint8_t *fp12g;
    const ehsm_uint8_t *pub_key;
    /*Address of the generated key and its cipher. The buffer
    should be key_size + 64*/
    ehsm_uint8_t *key_addr;
    /*Size in bytes of the generated key.*/
    ehsm_uint32_t key_size;
    ehsm_uint8_t hid;
} ehsm_sm9_wrap_key_param_st;

typedef struct ehsm_sm9_unwrap_key_param
{
    /*User private key handle.*/
    ehsm_uint32_t user_priv_key_handle;
    /* Address of the wrapped key. */
    const ehsm_uint8_t *cipher_addr;
    /*Size of the wrapped key.*/
    ehsm_uint32_t cipher_size;
    /*Address of user ID.*/
    const ehsm_uint8_t *id_addr;
    /*Size of user ID.*/
    ehsm_uint32_t id_size;
    /*Address of the unwrapped key.*/
    ehsm_uint8_t *key_addr;
    /*Size of the unwrapped key.*/
    ehsm_uint32_t key_size;
} ehsm_sm9_unwrap_key_param_st;

typedef struct ehsm_sm9_exckey_gen_tmpkey_param
{
    ehsm_sm9_user_privkey_type_e priv_key_type;
    const ehsm_uint8_t *peer_id;
    ehsm_uint32_t peer_id_size;
    ehsm_key_mem_type_e type;
    const ehsm_uint8_t *kgc_pub_key;
    ehsm_uint32_t key_handle;
} ehsm_sm9_exckey_gen_tmpkey_st;

//parameters for export SM9 key
typedef struct ehsm_sm9_inexport_key_param
{
    ehsm_uint32_t key_handle;
    ehsm_uint8_t *key_blob;
    ehsm_uint32_t key_blob_size;
    ehsm_uint8_t *key_auth_value;
    ehsm_uint32_t key_auth_size;
    ehsm_uint8_t key_is_plain;    //Note only import key can be set plain
    ehsm_key_mem_type_e type;
} ehsm_sm9_inexport_key_param_st;

typedef struct ehsm_sm9_gen_mast_pubkey
{
    ehsm_sm9_master_key_type_e key_type;
    ehsm_uint8_t *pub_key;
} ehsm_sm9_gen_mast_pubkey_st;

typedef struct ehsm_sm9_gen_tmp_pubkey_param
{
    ehsm_uint32_t key_handle;
    const ehsm_uint8_t *user_id;
    ehsm_uint32_t id_size;
    ehsm_uint8_t *pub_key;
} ehsm_sm9_gen_tmp_pubkey_st;

typedef struct {
    ehsm_uint32_t flash_read_addr;
    ehsm_uint32_t read_data_size;
    ehsm_uint8_t *otp_data_addr;
}ehsm_otp_read_param_st;

typedef struct {
    ehsm_uint32_t flash_write_addr;
    ehsm_uint32_t write_data_size;
    ehsm_uint8_t *otp_data_addr;
}ehsm_otp_write_param_st;


typedef struct {
    ehsm_uint32_t addr;
    ehsm_uint32_t size;
}ehsm_storage_area_param_st;

#define EHSM_LIFE_CYCLE_UNNORMAL_MODE 0U
#define EHSM_LIFE_CYCLE_TEST_MODE     1U
#define EHSM_LIFE_CYCLE_DEV_MODE      2U
#define EHSM_LIFE_CYCLE_MANU_MODE     3U
#define EHSM_LIFE_CYCLE_USER_MODE     4U
#define EHSM_LIFE_CYCLE_DEBUG_MODE    5U
#define EHSM_LIFE_CYCLE_DESTORY_MODE  6U
typedef ehsm_uint8_t ehsm_lifecycle_e;

#define EHSM_CONTROL_FIELD_TYPE_HW   0U
#define EHSM_CONTROL_FIELD_TYPE_EHSM 1U
#define EHSM_CONTROL_FIELD_TYPE_SOC  2U
typedef ehsm_uint8_t ehsm_control_field_type_e;

typedef struct {
    ehsm_uint8_t type;
        ehsm_uint8_t rev[1];
        ehsm_uint16_t size;
    ehsm_uint8_t *value;
}ehsm_change_control_field_st;

typedef struct {
    ehsm_uint8_t *challenge;
    ehsm_uint32_t challenge_size;
    ehsm_uint8_t *status;
    ehsm_uint32_t status_size;
    ehsm_uint8_t *signatrue;
    ehsm_uint32_t signatrue_size;
} ehsm_she_get_id_param_st;

typedef struct {
        /*The output buffer for EMU status.*/
        ehsm_uint8_t *emu_addr;
        /*The buffer size of emu_addr. */
    ehsm_uint32_t *emu_size;
} ehsm_get_emu_status_param_st;

/*define the ehsm EMU status*/
typedef struct ehsm_emu_status_st_{
    /*ehsm status */
    ehsm_uint32_t o_hsm_status[2];
    /*sensore detected errors */
    ehsm_uint32_t o_hsm_err_sensor;
    /*hardware detected errors */
    ehsm_uint32_t o_hsm_err_hw[2];
    /*firmware detected errors */
    ehsm_uint32_t o_hsm_err_fw[2];
} ehsm_emu_status_st;

typedef struct ehsm_evita_memory_info_st_{
    ehsm_uint32_t nvm_total_size;
    ehsm_uint32_t nvm_free_size;
    ehsm_uint32_t ram_total_size;
    ehsm_uint32_t ram_free_size;
}ehsm_evita_memory_info_st;

/** @brief The structure to store the information of ehsm status.
* The parameter alg, key_handle, key_auth_size,
* key_auth_value, sign_size and sign are only valid when type is not
* EHSM_GET_STATUS_SHE.
*/
typedef struct
{
    /* The type to get status, belongs to [0, 7].
    When the value is 0, CMD_GET_STATUS will be done.
    Others values, the Module_Status will be done. */
    ehsm_uint32_t type;
    /* Optional, algorithm to sign(mac or signature) the status. */
    ehsm_uint32_t algo_id;
    /* Optional, reference of the key. */
    ehsm_uint32_t key_handle;
    /* Optional, size of the key usage authorization value. */
    ehsm_uint32_t key_auth_size;
    /* Optional, key usage authorization. */
    const ehsm_uint8_t *key_auth_value;
    /* [in/out] Size of module status data blob. Inited with the size. */
    ehsm_uint32_t *status_size;
    /* [in/out] The returned status data blob. */
    ehsm_uint8_t *status;
    /* [in/out] Optional, size of signature size. Inited with the size. */
    ehsm_uint32_t *sign_size;
    /* [in/out] Optional, the signature of status. */
    ehsm_uint8_t *sign;
} ehsm_module_status_st;


typedef struct
{
    ehsm_uint8_t *certificate_info;
    ehsm_uint32_t *verify;
} ehsm_certificate_verify_st;

#define EHSM_API_TYPE_SHE     1U
#define EHSM_API_TYPE_EVITA   2U
#define EHSM_API_TYPE_AUTOSAR 3U
#define EHSM_API_TYPE_EXT     4U
#define EHSM_API_TYPE_INVALID 5U
typedef ehsm_uint8_t ehsm_api_type_e;

typedef struct {
    ehsm_uint32_t size;
    ehsm_uint8_t *data;
}ehsm_sensor_init_param_st;
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
#endif
