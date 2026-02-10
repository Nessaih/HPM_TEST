#ifndef EHSM_IF_EVITA_TYPES_IP_H_
#define EHSM_IF_EVITA_TYPES_IP_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Com_Struct_Ip.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define ALIGN_BYTE(x)                __attribute__((packed, aligned(x)))

#define EVITA_KEY_MAX_SIZE             (sizeof(ehsm_internal_key_st))
#define EVITA_SALT_VALUE_MAX_SIZE      (64U)
#define EVITA_MAX_RANDOM_KEY_SIZE      (1024U)
#define EVITA_OTP_SYM_KEY_SIZE         (sizeof(evita_otp_sym_key_st))
#define EVITA_OTP_ASYM_KEY_SIZE        (sizeof(evita_otp_asym_key_st))
#define EVITA_MAX_OTP_KEY_SIZE         (EVITA_OTP_ASYM_KEY_SIZE)
#define EVITA_KEY_SIGNATRUE_SIZE       (64U)
#define EVITA_OTP_PUBKEY_SIZE          (64U)
#define EVITA_OTP_PRIVKEY_SIZE         (32U)

/*error code for EVITA*/

#define EVITA_OK                                0x0U

/*General error*/
#define EVITA_GENERAL_ERROR                     0x01U

/*Given key handle is unknown or wrong (e.g.,not for this algorithm or this mode)*/
#define EVITA_WRONG_KEY_HANDLE                  0x02U

/*No resources left for an additional parallel session (or no parallel processing at all)*/
#define EVITA_ALL_SESSIONS_OCCUPIED             0x03U

/*Given algorithm or algorithm mode not available*/
#define EVITA_ALGORITHM_ERROR                   0x04U

/*Given IV does not fit the given algorithm*/
#define EVITA_WRONG_IV                          0x05U

/*Given authorization value was wrong*/
#define EVITA_AUTHORIZATION_FAILED              0x06U

/*Given session handle is unknown or wrong*/
#define EVITA_WRONG_SESSION_HANDLE              0x07U

/*Given chunk size is wrong (cf. returns on initialization)*/
#define EVITA_WRONG_CHUNK_SIZE                  0x08U

/*Given MAC length for verification is greater than MAC*/
#define EVITA_MAC_LENGTH_OVERSIZE               0x09U

/**
 * @brief Given ECR index is not existing or cannot be extended
 *
 * @note  This is not supported now.
 */
#define EVITA_WRONG_ECR_INDEX                   0x0aU

/*Requested number of random bytes exceeds PRNG limit*/
#define EVITA_PRNG_REQUEST_OVERSIZE              0x0bU

/*PRNG was unable to retrieve true random seed from TRNG*/
#define EVITA_TRNG_SEED_FAILURE                  0x0cU

/*No resources left to create an additional counter*/
#define EVITA_ALL_COUNTERS_OCCUPIED              0x0dU

/*Given counter identifier is unknown*/
#define EVITA_UNKNOWN_COUNTER_ID                 0x0eU

/*Given counter incrementation is invalid (e.g., too large)*/
#define EVITA_INVALID_COUNTER_INCREMENTATION     0x0fU

/*Requested status type is (currently) not available for this module*/
#define EVITA_STATUS_TYPE_NOT_AVAILABLE          0x10U

/*Requested test case is (currently) not available for this module*/
#define EVITA_TEST_CASE_NOT_AVAILABLE            0x11U

/*Requested test case failed*/
#define EVITA_TEST_CASE_FAILED                   0x12U

/*Given key size is invalid (e.g., too small or too large)*/
#define EVITA_INVALID_KEY_SIZE                   0x13U

/*No resources left to create an additional key*/
#define EVITA_ALL_KEY_SPACE_OCCUPIED             0x14U

/*Given key flag is invalid (e.g., wrong combination)*/
#define EVITA_INVALID_KEY_FLAG                   0x15U

/*Given remote key handle is unknown or wrong(e.g., not for this algorithm or this mode)*/
#define EVITA_WRONG_REMOTE_KEY_HANDLE            0x16U

/*Keys do not fit the algorithm (e.g., RSA key vs. ECDH)*/
#define EVITA_WRONG_KEY_COMBINATION              0x17U

/*Given authorization structure does not fit key flags (e.g., authorization definition for
  a certain flag missing)*/
#define EVITA_WRONG_AUTHORIZATION                0x18U

/*Given transport key is not a capable transport key or use flag cannot be transported or
  migrated*/
#define EVITA_TRANSPORT_IMPOSSIBLE               0x19U

/*Key is not allowed to become removed*/
#define EVITA_REMOVE_IMPOSSIBLE                  0x1aU

/*Given message imprint size is invalid (e.g., too small or too large)*/
#define EVITA_INVALID_MSG_SIZE                   0x1bU

/*EVITA UTC clock is not synchronized yet*/
#define EVITA_CLOCK_NOT_SYNCHRONIZED             0x1cU

/*Given time stamp could not be interpreted correctly*/
#define EVITA_INVALID_TIME_STAMP                 0x1dU

/*Given UTC time could not be interpreted correctly*/
#define EVITA_INVALID_UTC_TIME                   0x1eU

/*Challenge was not requested or is expired already*/
#define EVITA_UTC_CHALLENGE_EXPIRED              0x1fU

/*Synchronization failed due to wrong signature or wrong challenge etc.*/
#define EVITA_UTC_SYNCHRONIZATION_FAILED         0x20U

/*Given certification key handle is unknown or wrong (e.g., not enabled for signing)*/
#define EVITA_WRONG_CERT_KEY_HANDLE                  0x21U

/* usage flag for EVITA key */
#define EVITA_KEY_USE_FLAG_SIGN                     0x1U
#define EVITA_KEY_USE_FLAG_VERIFY                   0x2U
#define EVITA_KEY_USE_FLAG_ENCRYPT                  0x4U
#define EVITA_KEY_USE_FLAG_DECRYPT                  0x8U
#define EVITA_KEY_USE_FLAG_TIMESTAMP                0x10U
#define EVITA_KEY_USE_FLAG_SECUREBOOT               0x20U
#define EVITA_KEY_USE_FLAG_SECURESTORAGE            0x40U
#define EVITA_KEY_USE_FLAG_KEYCREATION              0x80U
#define EVITA_KEY_USE_FLAG_UTCSYNC                  0x100U
#define EVITA_KEY_USE_FLAG_TRANSPORT                0x200U
#define EVITA_KEY_USE_FLAG_REMOVE                   0x400U
#define EVITA_KEY_USE_FLAG_DHKE                     0x80U

/* Authorization type */
#define EVITA_AUTH_TYPE_NONE                        0x00U
#define EVITA_AUTH_TYPE_PASSWD                      0x01U

/* KDF type */
#define EVITA_KEY_DERIVE_KDFX963                    0x00U
#define EVITA_KEY_DERIVE_PBKDF2                     0x01U

/* transport flasgs */
#define EVITA_KEY_TRNSP_INI                         0x00U
#define EVITA_KEY_TRNSP_MIG                         0x01U
#define EVITA_KEY_TRNSP_OEM                         0x02U
#define EVITA_KEY_TRNSP_EXT                         0x03U

#define EHSM_CONTEXT_SIZE 512U
#define EVITA_MAX_CHUNK_SIZE 1024U
#define SKE_CTX_BUF_SIZE    108U

/* Limitation of request size */
#define RNG_REQUEST_MAX 128U

/********************SE version kmgr key format*********************************************/
#define EHSM_KEY_AUTH_VALUE_MAX_SIZE   (32U)
#define EHSM_SYM_KEY_PAIR_MAX_SIZE     (64U)
#define EHSM_ECC_KEY_PAIR_MAX_SIZE     (198U)
#define EHSM_RSA_KEY_PAIR_MAX_SIZE     (2304U)
#define EHSM_RSA_DH_KEY_PAIR_MAX_SIZE  (2304U)
#define EHSM_SM2_SM2_KEY_MAX_SIZE      (100U)

#define EHSM_KEY_DATA_MAX_SIZE (EHSM_SYM_KEY_PAIR_MAX_SIZE)  //default key size for symmetric key

#undef EHSM_KEY_DATA_MAX_SIZE
#define EHSM_KEY_DATA_MAX_SIZE (EHSM_SM2_SM2_KEY_MAX_SIZE)

#undef EHSM_KEY_DATA_MAX_SIZE
#define EHSM_KEY_DATA_MAX_SIZE (EHSM_ECC_KEY_PAIR_MAX_SIZE) //max is SECP521R1 curve key

#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA
#undef EHSM_KEY_DATA_MAX_SIZE
#define EHSM_KEY_DATA_MAX_SIZE (EHSM_RSA_KEY_PAIR_MAX_SIZE) //max is rsa 4096bit crt key
#endif

#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA
#undef EHSM_KEY_DATA_MAX_SIZE
#define EHSM_KEY_DATA_MAX_SIZE (EHSM_RSA_DH_KEY_PAIR_MAX_SIZE) //max is rsa dh 4096bit key
#endif

#define EHSM_KEY_SIZE_INFO_MAX_LEN (16U)
#define EHSM_KEY_HEAD_SIZE (sizeof(ehsm_se_key_st))

#define EVITA_HASH_BUF_SIZE         64U
#define EVITA_SIGNATURE_BUF_SIZE    512U
#define EVITA_MAC_BUF_SIZE          16U

#define EHSM_EVITA_KEY_NUM      20U
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    ehsm_bool_t sign;
    ehsm_bool_t verify;
    ehsm_bool_t encrypt;
    ehsm_bool_t decrypt;
    ehsm_bool_t timestamp;
    ehsm_bool_t secureboot;
    ehsm_bool_t securestorage;
    ehsm_bool_t createkey;
    ehsm_bool_t utcsync;
    ehsm_bool_t transport;
    ehsm_bool_t remove;
}key_act_use_flags_t;

typedef struct ehsm_dh_param_size_info
{
    ehsm_uint16_t p_size;
    ehsm_uint16_t q_size;
    ehsm_uint16_t g_size;
}ehsm_dh_param_size_info_st;
typedef ehsm_uint8_t storage_key_type_e;

#define EVITA_STORAGE_NONE_KEY_TYPE        0U
#define EVITA_STORAGE_KDF_KEY_TYPE         1U
#define EVITA_STORAGE_RSA_KEY_TYPE         2U
#define EVITA_STORAGE_DH_KEY_TYPE          3U
#define EVITA_STORAGE_RANDOM_KEY_TYPE      4U
#define EVITA_STORAGE_SM9_MASTER_KEY_TYPE  5U
#define EVITA_STORAGE_OTP_KEY_TYPE         6U
#define EVITA_STORAGE_DH_KEY_PAIR_TYPE     7U

typedef struct
{
    ehsm_uint32_t storage_key_type;
    union {
        ehsm_uint16_t kdf_key_size;
        ehsm_uint16_t dh_key_size;
        ehsm_uint16_t rsa_e_bytes_size;
        ehsm_uint16_t random_key_size;
        ehsm_uint32_t key_handle; //otp key handle
        ehsm_dh_param_size_info_st dh_param;
    }storage_info;
}key_info_st;

typedef struct ehsm_key_attr_data_
{
    ehsm_uint32_t key_identifier;
    ehsm_uint32_t algo_id;           //key type and key size info
    ehsm_uint32_t valid_util;
    key_info_st key_info;
    ehsm_uint16_t key_usage_size;      //size of storage key usage
    ehsm_uint16_t key_signatrue_off;    //key signatrue offset, 0 means this key not exist signatrue data
}ehsm_key_attr_data_st;

typedef struct ehsm_key_signature_
{
    ehsm_uint32_t sign_id;       //signatrue identifier
    ehsm_uint32_t sign_info;     //sign information to store signatrue key algo id
    ehsm_uint32_t sign_key_id;   //signatrue key identifier
    ehsm_uint32_t target_key_id; //target key identifier
    ehsm_uint8_t signatrue[EVITA_KEY_SIGNATRUE_SIZE];  //signature;
}ehsm_key_signatrue_st;

typedef struct ehsm_ecc_pubkey
{
    ehsm_uint8_t p[132U];
}ehsm_ecc_pubkey_st;

typedef struct ehsm_rsa_pubkey
{
    ehsm_uint8_t e[512U];
    ehsm_uint8_t n[512U];
}ehsm_rsa_pubkey_st;

typedef struct ehsm_dh_param
{
    ehsm_uint8_t p[512U];
    ehsm_uint8_t q[512U];
    ehsm_uint8_t g[512U];
}ehsm_dh_param_st;

typedef struct ehsm_dh_pubkey
{
    ehsm_uint8_t pub[512U];
    ehsm_dh_param_st dh_param;
}ehsm_dh_pubkey_st;

typedef union ehsm_pubkey_data_
{
    ehsm_rsa_pubkey_st rsa;
    ehsm_ecc_pubkey_st ecc;
    ehsm_dh_pubkey_st dh;
}ehsm_pubkey_data_st;

typedef struct ehsm_dh_prikey
{
    ehsm_uint8_t priv[512U];
}ehsm_dh_prikey_st;

typedef struct ehsm_rsa_crt_param_
{
    ehsm_uint8_t p[256U];
    ehsm_uint8_t q[256U];
    ehsm_uint8_t dp[256U];
    ehsm_uint8_t dq[256U];
    ehsm_uint8_t u[256U];
}ehsm_rsa_ctr_st;

typedef union ehsm_prikey_data_
{
    ehsm_uint8_t sym_k[1024U];  //remove?
    ehsm_uint8_t ecc_k[66U];
    ehsm_uint8_t rsa_d[512U];
    ehsm_uint8_t kdf_k[512U];
    ehsm_uint8_t dh_k[512U];
    ehsm_dh_prikey_st dh;
    ehsm_rsa_ctr_st rsa_crt;
}ehsm_prikey_data_st;

typedef struct ehsm_key_status_
{
    ehsm_uint8_t keyId[4];
    ehsm_uint32_t keyIdSize;
    ehsm_uint32_t algo_id;
    ehsm_uint32_t valid_util;
    key_act_use_flags_t activeUseFlag;
    ehsm_uint32_t mem_location;
    ehsm_key_signatrue_st key_sign_data;
    ehsm_pubkey_data_st pubkey;
    ehsm_uint32_t cert_size;
    ehsm_uint8_t cert_data[512U];
}ALIGN_BYTE(4) ehsm_key_status_st;

typedef struct ehsm_internal_key_
{
    ehsm_key_attr_data_st attr;
    ehsm_key_usages_st key_usage;
    ehsm_pubkey_data_st pubkey;
    ehsm_uint32_t prikey_enc_size;
    ehsm_prikey_data_st prikey;   //max rsa4096_CRT key
    ehsm_key_signatrue_st key_signatrue;
} ehsm_internal_key_st;  //ALIGN_BYTE(4)

typedef struct ehsm_external_key_
{
    ehsm_internal_key_st evita_internal_key;
    ehsm_key_signatrue_st auth_sign_data;
}ALIGN_BYTE(4) ehsm_external_key_st;

typedef struct ehsm_export_pub_key_
{
    ehsm_uint32_t algo_id;           //key type and key size info
    union {
        ehsm_uint16_t rsa_e_bytes_size;
        ehsm_uint16_t dh_pubkey_bytes_size;
    }size_info;
    ehsm_pubkey_data_st key;
}ALIGN_BYTE(4) ehsm_export_pub_key_st;

typedef struct ehsm_sym_key_size_
{
    ehsm_uint16_t key_size;
    ehsm_uint16_t reserved[2];
}ehsm_sym_key_size_st;

typedef struct ehsm_ecc_key_size_
{
    ehsm_uint16_t pub_key_size;
    ehsm_uint16_t priv_key_size;
}ehsm_ecc_key_size_st;

typedef struct ehsm_rsa_key_size_
{
    ehsm_uint16_t rsa_n_size;
    ehsm_uint16_t rsa_e_size;
    ehsm_uint16_t rsa_d_size;
    ehsm_uint16_t reserved[2];
}ehsm_rsa_key_size_st;

typedef struct ehsm_rsa_dh_key_size_
{
    ehsm_uint16_t rsa_dh_p_size;
    ehsm_uint16_t rsa_dh_q_size;
    ehsm_uint16_t rsa_dh_g_size;
    ehsm_uint8_t reserved[2];
}ehsm_rsa_dh_key_size_st;

typedef struct ehsm_se_key_
{
    ehsm_uint32_t key_handle;
    ehsm_uint32_t algo_id;
    ehsm_uint8_t auth_size;
    ehsm_uint8_t reserved[3];
    ehsm_uint8_t auth_value[EHSM_KEY_AUTH_VALUE_MAX_SIZE];
    ehsm_uint8_t key_size_info[EHSM_KEY_SIZE_INFO_MAX_LEN];
    ehsm_uint8_t key_data[0];
}ALIGN_BYTE(4) ehsm_se_key_st;

typedef struct ehsm_pub_key_
{
    ehsm_uint32_t algo_id;
    ehsm_uint8_t key_size_info[EHSM_KEY_SIZE_INFO_MAX_LEN];  //NOTE that: for ECC key set rsa_d_size value to zero. for rsa_dh key set both value rsa_dh_q_size and rsa_dh_g_size to zero
    ehsm_uint8_t key_pub_data[0];
}ALIGN_BYTE(4) ehsm_pub_key_st;
/********************SE version kmgr key format*********************************************/

//EVITA D3.2 4.3.7.1
//seconds from 01.01.1970 without milliseconds
typedef ehsm_uint32_t ehsm_utc_time_t;
//EVITA D3.2 4.3.7.2.2
typedef struct counter_value_64_t {
    ehsm_uint32_t high_word;
    ehsm_uint32_t low_word;
} ehsm_counter_value_st;

//EVITA D3.2 4.3.7.2.3
typedef struct hash_hmac_t {
    ehsm_uint8_t hash_hmac[EVITA_HASH_BUF_SIZE];
    ehsm_uint32_t hash_hmac_size;
    ehsm_utc_time_t utc_time;
} hash_hmac_st;

//EVITA D3.2 4.3.7.2.4
typedef struct signature_t {
    ehsm_uint8_t signature[EVITA_SIGNATURE_BUF_SIZE];
    ehsm_uint32_t signature_size;
    ehsm_utc_time_t utc_time;
} signature_st;

//EVITA D3.2 4.3.7.2.5
typedef signature_st time_stamp_st;

//EVITA D3.2 4.3.7.2.6
typedef struct mac_t {
    ehsm_uint8_t mac_value[EVITA_MAC_BUF_SIZE];
    ehsm_uint32_t mac_size;
    ehsm_utc_time_t utc_time;
} mac_st;

//EVITA D3.2 4.3.2.6
typedef struct ehsm_tick_value {
    ehsm_uint32_t current_ticks;
    ehsm_uint32_t tick_length;
    ehsm_uint32_t tick_accuracy;
} ehsm_tick_value_st;

/* Eumu of EVITA SKE */
#define EVITA_ENCRYPTION 0U
#define EVITA_DECRYPTION 1U
typedef ehsm_uint8_t cipher_mode_e;

#define EVITA_ECB_MODE         1U
#define EVITA_XTS_MODE         2U
#define EVITA_CBC_MODE         3U
#define EVITA_CFB_MODE         4U
#define EVITA_OFB_MODE         5U
#define EVITA_CTR_MODE         6U
#define EVITA_CMAC_MODE        7U
#define EVITA_CBC_MAC_MODE     8U
#define EVITA_GMAC_MODE        9U
#define EVITA_GCM_MODE         10U
#define EVITA_CCM_MODE         11U
typedef ehsm_uint8_t operation_mode_e;

#define EVITA_NOPADDING        0U
#define EVITA_PSASSA_PSS       1U
#define EVITA_PKCS7            2U
#define EVITA_ONEWITHZEROS     3U
typedef ehsm_uint8_t padding_scheme_e;

#define EVITA_INVALID          0U
#define EVITA_INIT             1U
#define EVITA_UPDATE           3U
#define EVITA_FINISH           7U
typedef ehsm_uint8_t session_status_e;

#define EVITA_MAC_SIGN              0U
#define EVITA_MAC_VERIFY            1U
#define EVITA_MAC_TIMESTAMPED_SIGN  2U
typedef ehsm_uint8_t mac_mode_e;

#define EVITA_HMAC_SIGN             0U
#define EVITA_HMAC_VERIFY           1U
#define EVITA_HMAC_TIMESTAMP_SIGN   2U
#define EVITA_HASH                  3U
typedef ehsm_uint8_t hash_mode_e;

#define EHSM_KEY_EVITA_MVK   1U
#define EHSM_KEY_EVITA_IDK   2U
#define EHSM_KEY_EVITA_SRK   3U
#define EHSM_KEY_EVITA_CSK   4U
#define EHSM_KEY_EVITA_OVK   5U
#define EHSM_KEY_EVITA_END   (EHSM_KEY_EVITA_OVK + EHSM_EVITA_KEY_NUM)
typedef ehsm_uint8_t ehsm_evita_key_handle_e;

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
