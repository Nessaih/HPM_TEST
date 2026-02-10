#ifndef EHSM_ERR_CODE_IP_H
#define EHSM_ERR_CODE_IP_H
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

/**
 * @brief Error code definitions.
 *
 * Except for values in [0x00U, 0xFFU], others values have two bytes. The first byte is used to distingwish modules.
 *   It has 16 values, where the higher 4-bit and low 4-bit are just in negation. The next byte is from [0, 255] as a
 *   specific error number.
 *
 * 0x00U is used for success.
 * [0x0001U - 0x01FFU] for crypto lib basic error
 * [0x0200U - 0x0EFFU] are reserved to be compatible with spec error code.
 * [0x0F00U - 0x0FFFU] are used for some common errors.
 * [0x1E00U - 0x1EFFU] are used for key management errors.
 * [0x2D00U - 0x2DFFU] are used for crypto primitive errors.
 *   [0x2D00U - 0x2D1FU] SKE errors
 *   [0x2D20U - 0x2D3FU] PKE errors
 *   [0x2D40U - 0x2D5FU] HASH errors
 *   [0x2D60U - 0x2D7FU] RNG errors
 *   [0x2D80U - 0x2D8FU] CERT errors
 *   [0x2D90U - 0x2D9FU] SM9 errors
 *   [0x2DA0U - 0x2DFFU] Reserved or general error.
 * [0x3C00U - 0x3CFFU] are used for secure boot, upgrade, debug auth errors.
 *   [0x3C00U - 0x3C1FU] Secure boot, upgrade errors
 *   [0x3C20U - 0x3C3FU] debug auth errors
 *   [0x3C40U - 0x3CFFU] Reserved or general error.
 * [0x4B00U - 0x4BFFU] are used for system management, self test, .etc errors.
 * [0x5A00U - 0x5AFFU] are used for counter, timer errors.
 *   [0x5A00U - 0x5A1FU] Counter errors
 *   [0x5A20U - 0x5A3FU] Timer errors
 * [0x6900U - 0xF0FFU] are reserved.
 *
 * @note 1) 0xA55AU is a specail value for mailbox success.
 *       2) The value of an error number can be changed to keep a better order, if some errors are added or deleted.
 */

/**
 * @brief This is a specail value just for mailbox success.
 */
#define EHSM_ERR_MAILBOX_SUCCESS        0xA55AU

/**
 * @brief No error.
 */
#define EHSM_ERR_SW_SUCCESS             0x0U

/**
 * @brief Error not covered by the following.
 */
#define EHSM_ERR_GENERAL_ERROR          0x01U

/**
 * @brief TRNG general error.
 */
#define EHSM_ERR_IPCORE_TRNG_BUFFER_NULL                        0x1U
#define EHSM_ERR_IPCORE_TRNG_INVALID_INPUT                      0x2U
#define EHSM_ERR_IPCORE_TRNG_INVALID_CONFIG                     0x3U
#define EHSM_ERR_IPCORE_TRNG_HT_ERROR                           0x4U
#define EHSM_ERR_IPCORE_TRNG_TIMEOUT_ERROR                      0x5U
#define EHSM_ERR_IPCORE_TRNG_ERROR                              0x6U

/**
 * @brief SKE general error.
 */
#define EHSM_ERR_IPCORE_SKE_BUFFER_NULL                         0x1U
#define EHSM_ERR_IPCORE_SKE_CONFIG_INVALID                      0x2U
#define EHSM_ERR_IPCORE_SKE_INPUT_INVALID                       0x3U
#define EHSM_ERR_IPCORE_SKE_ATTACK_ALARM                        0x4U
#define EHSM_ERR_IPCORE_SKE_PADDING_ERROR                       0x5U
#define EHSM_ERR_IPCORE_SKE_ERROR                               0x6U

/**
 * @brief HASH general error.
 */
#define EHSM_ERR_IPCORE_HASH_BUFFER_NULL                        0x1U
#define EHSM_ERR_IPCORE_HASH_CONFIG_INVALID                     0x2U
#define EHSM_ERR_IPCORE_HASH_INPUT_INVALID                      0x3U
#define EHSM_ERR_IPCORE_HASH_LEN_OVERFLOW                       0x4U
#define EHSM_ERR_IPCORE_HASH_OUTPUT_ZERO_ALL                    0x5U
#define EHSM_ERR_IPCORE_HASH_ERROR                              0x6U

/**
 * @brief PKE general error.
 */
#define EHSM_ERR_IPCORE_PKE_NO_MODINV                           0x1U
#define EHSM_ERR_IPCORE_PKE_NOT_ON_CURVE                        0x2U
#define EHSM_ERR_IPCORE_PKE_INFINITY_POINT                      0x3U
#define EHSM_ERR_IPCORE_PKE_ZERO_ALL                            0x4U
#define EHSM_ERR_IPCORE_PKE_INTEGER_TOO_BIG                     0x5U
#define EHSM_ERR_IPCORE_PKE_INVALID_INPUT                       0x6U
#define EHSM_ERR_IPCORE_PKE_FINISHED                            0x7U
#define EHSM_ERR_IPCORE_PKE_ERROR                               0x8U

/**
 * @brief PKE specific algorithm error.
 */
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA
#define EHSM_ERR_IPCORE_RSA_BUFFER_NULL                         0x30U
#define EHSM_ERR_IPCORE_RSA_INPUT_TOO_LONG                      0x31U
#define EHSM_ERR_IPCORE_RSA_INPUT_INVALID                       0x32U
#endif

#define EHSM_ERR_IPCORE_SM2_BUFFER_NULL                         0x40U
#define EHSM_ERR_IPCORE_SM2_NOT_ON_CURVE                        0x41U
#define EHSM_ERR_IPCORE_SM2_EXCHANGE_ROLE_INVALID               0x42U
#define EHSM_ERR_IPCORE_SM2_INPUT_INVALID                       0x43U
#define EHSM_ERR_IPCORE_SM2_ZERO_ALL                            0x44U
#define EHSM_ERR_IPCORE_SM2_INTEGER_TOO_BIG                     0x45U
#define EHSM_ERR_IPCORE_SM2_VERIFY_FAILED                       0x46U
#define EHSM_ERR_IPCORE_SM2_DECRYPT_VERIFY_FAILED               0x47U

#define EHSM_ERR_IPCORE_ECDSA_POINTOR_NULL                      0x50U
#define EHSM_ERR_IPCORE_ECDSA_INVALID_INPUT                     0x51U
#define EHSM_ERR_IPCORE_ECDSA_ZERO_ALL                          0x52U
#define EHSM_ERR_IPCORE_ECDSA_INTEGER_TOO_BIG                   0x53U
#define EHSM_ERR_IPCORE_ECDSA_VERIFY_FAILED                     0x54U

#define EHSM_ERR_IPCORE_ECDH_POINTOR_NULL                       0x60U
#define EHSM_ERR_IPCORE_ECDH_INVALID_INPUT                      0x61U
#define EHSM_ERR_IPCORE_ECDH_ZERO_ALL                           0x62U
#define EHSM_ERR_IPCORE_ECDH_INTEGER_TOO_BIG                    0x63U


#define EHSM_ERR_IPCORE_DH_POINTER_NULL                         0xA0U
#define EHSM_ERR_IPCORE_DH_INVALID_INPUT                        0xA1U
#define EHSM_ERR_IPCORE_DH_ZERO_ALL                             0xA2U
#define EHSM_ERR_IPCORE_DH_VALUE_ONE                            0xA3U
#define EHSM_ERR_IPCORE_DH_INTEGER_TOO_BIG                      0xA4U

#define EHSM_ERR_IPCORE_ECIES_POINTOR_NULL                      0x100U
#define EHSM_ERR_IPCORE_ECIES_INVALID_INPUT                     0x101U
#define EHSM_ERR_IPCORE_ECIES_ZERO_ALL                          0x102U
#define EHSM_ERR_IPCORE_ECIES_INTEGER_TOO_BIG                   0x103U
#define EHSM_ERR_IPCORE_ECIES_ERROR                             0x104U

/**
 * @brief The crypto driver is not initialized.
 */
#define EHSM_ERR_NOT_NIT                0x0F00U

/**
 * @brief No free space to handle the request command.
 */
#define EHSM_ERR_OUT_OF_MEM             0x0F01U

/**
 * @brief The service request failed because the service is still busy.
 *        Value is 0x02U
 */
#define EHSM_ERR_EHSM_BUSY              0x0F02U

/**
 * @brief The service request failed because the queue is full.
 *        Value is 0x05U
 */
#define EHSM_ERR_QUEUE_FULL             0x0F03U

/**
 * @brief The service request failed because the Job has been canceled.
 *        Value is 0x0BU
 */
#define EHSM_ERR_JOB_CANCELED           0x0F04U

/**
 * @brief The service request failed because the provided buffer is too small to store the result.
 *        Value is 0x03U
 */
#define EHSM_ERR_SMALL_BUFFER           0x0F05U

/**
 * @brief General parameter wrong.
 */
#define EHSM_ERR_PARAM_ERROR            0x0F06U

/**
 * @brief The alg, mode, padding or required feature is not supported
 */
#define EHSM_ERR_NOT_SUPPORT            0x0F07U

/**
 * @brief Wrong module type for module_status_cmd.
 */
#define EHSM_ERR_WRONG_MODULE_TYPE      0x0F08U

/**
 * @brief Wrong job id for cancel_cmd.
 */
#define EHSM_ERR_WRONG_JOB_ID           0x0F09U

/**
 * @brief The service request failed because the synchronous Job has been canceled.
 */
#define EHSM_ERR_CMD_CANCELED           0x0F0AU

/**
 * @brief move code into iram error
 */
#define EHSM_ERR_CODE_MOVE_ERROR        0x0F0BU

/**
 * @brief Wrong key usage.
 */
#define EHSM_ERR_WRONG_KEY_USAGE        0x1E00U
#define EHSM_ERR_WRONG_KEY_HANDLE       0x1E01U  /*Given key handle is wrong.*/
#define EHSM_ERR_AUTH_FAILED            0x1E02U  /*Given authorization value was wrong.*/
#define EHSM_ERR_WRONG_KEY_LIFE_LIMIT   0x1E03U  /*Wrong key life limitation.*/
#define EHSM_ERR_WRONG_KEY_TYPE         0x1E04U  /*Wrong key type.*/
#define EHSM_ERR_WRONG_KEY_SIZE         0x1E05U  /*Wrong key size.*/
#define EHSM_ERR_WRONG_KEY_DERIVE_FUNC  0x1E06U  /*Wrong key derivation function.*/
#define EHSM_ERR_WRONG_SALT_SIZE        0x1E07U  /*Wrong salt size for key derivation.*/
#define EHSM_ERR_KEY_BUFF_SMALLER       0x1E08U  /*Buffer for key storage is too small.*/
#define EHSM_ERR_KEY_STORE_FULL         0x1E09U  /*Key storage in eHSM is full.*/
#define EHSM_ERR_WRONG_KEY_LEVEL        0x1E0AU  /*Wrong key level for bootrom.*/
#define EHSM_ERR_WRONG_PUB_KEY          0x1E0BU  /*Wrong public key for debug authentication.*/
#define EHSM_ERR_TRANSPORT_IMPOSSIBLE   0x1E0CU  /*Give tansport key is not a capable transport key.*/
#define EHSM_ERR_ALL_KEY_SPACE_OCCUPIED 0x1E0DU  /*Evita error: no resources left to create/import an additional key*/
#define EHSM_ERR_REMOVE_IMPOSSIBLE      0x1E0EU  /*Evita error: key is not allowed to become removed*/
#define EHSM_ERR_WRONG_CERT_KEY_HANDLE  0x1E0FU  /*Evita error: given certification key handle is unkown wrong*/
#define EHSM_ERR_ALGORITHM_ERROR        0x1E10U  /*Evita error: Algorithm or mode not available*/
#define EHSM_ERR_WRONG_AUTHORIZATION    0x1E11U  /*Evita error: given athorization structure does not fit key flags*/
#define EHSM_ERR_INVALID_KEY_FLAG       0x1E12U  /*Evita error: given key flag is vinvalid (e.g., wrong combination)*/
#define EHSM_ERR_WRONG_REMOTE_KEY_HANDLE 0x1E13U /*Evita error: Given remote key handle is unknow or wrong*/
#define EHSM_ERR_WRONG_KEY_COMBINATION  0x1E14U  /*Keys do not fit the algorithm (e.g., RSA key vs. ECDH)*/
#define EHSM_ERR_KEY_EMPTY_ERROR        0x1E18U  /*key data empty */
#define EHSM_ERR_WRITE_PROTECTED        0x1E15U  /*She key has been write protect.*/
#define EHSM_ERR_KEY_UPDATE_ERROR       0x1E16U  /*She key update error*/
#define EHSM_ERR_KEY_INVALID_ERROR      0x1E17U  /*She key invalid*/
#define EHSM_ERR_KEY_NOT_AVAILABLE_ERROR 0x1E19U /*She key not available */
#define EHSM_ERR_MEMORY_FAILURE         0x1E1AU  /*She key memory failure */

/**
 * @brief ehsm kmgr module read fail
 */
#define EHSM_ERR_KMGR_READ_ERROR        0x1E1BU  

/**
 * @brief Wrong algorithm.
 */
#define EHSM_ERR_SKE_WRONG_ALG          0x2D00U

/**
 * @brief Wrong mode for the algorithm.
 */
#define EHSM_ERR_SKE_WRONG_MODE         0x2D01U

/**
 * @brief Wrong padding for the algorithm.
 */
#define EHSM_ERR_SKE_WRONG_PADIDNG      0x2D02U

/**
 * @brief Wrong key size for the algorithm.
 */
#define EHSM_ERR_SKE_WRONG_K_SIZE       0x2D03U

/**
 * @brief Wrong iv size for the algorithm.
 */
#define EHSM_ERR_SKE_WRONG_IV_SIZE      0x2D04U

/**
 * @brief Iv should not be null, but it's null.
 */
#define EHSM_ERR_SKE_IV_SHOULD_NOT_NULL 0x2D05U

/**
 * @brief Wrong tag size.
 */
#define EHSM_ERR_SKE_WRONG_TAG_SIZE     0x2D06U

/**
 * @brief Wrong L size (only for CCM).
 */
#define EHSM_ERR_SKE_WRONG_L_SIZE       0x2D07U

/**
 * @brief Wrong aad size (only fro GCM and CCM)
 */
#define EHSM_ERR_SKE_WRONG_AAD_SIZE     0x2D08U

/**
 * @brief MAC verify failed.
 */
#define EHSM_ERR_SKE_MAC_VRY_FAILED     0x2D09U

/**
 * @brief Wrong curve_id for ECIES or ECCP.
 *        Reserved some values for ske.
 */
#define EHSM_ERR_PKE_WRONG_CURVE_ID     0x2D20U

/**
 * @brief Wrong CRT mode for RSA.
 */
#define EHSM_ERR_PKE_WRONG_RSA_CRT_MODE 0x2D21U

/**
 * @brief Wrong e value size for RSA key generation.
 */
#define EHSM_ERR_PKE_WRONG_E_SIZE       0x2D22U

/**
 * @brief Wrong n value size for RSA key generation.
 */
#define EHSM_ERR_PKE_WRONG_N_SIZE       0x2D23U

/**
 * @brief PKE sign failed.
 */
#define EHSM_ERR_PKE_SIGN_FAILED        0x2D24U

/**
 * @brief PKE verify failed.
 */
#define EHSM_ERR_PKE_VERIFY_FAILED      0x2D25U

/**
 * @brief Message size is overflow for ed25519.
 */
#define EHSM_ERR_PKE_ED25519_MSG_FLOW   0x2D26U

/**
 * @brief Wrong KDF algorithm for ECIES.
 */
#define EHSM_ERR_PKE_WRONG_KDF_ALG      0x2D27U

/**
 * @brief HMAC verify failed.
 */
#define EHSM_ERR_HMAC_VERIFY_FAILED     0x2D40U

/**
 * @brief HASH buffer is NULL
 */
#define EHSM_ERR_HASH_BUFFER_NULL       0x2D41U

/**
 * @brief HASH configuration invalid
 */
#define EHSM_ERR_HASH_CONFIG_INVALID    0x2D42U

/**
 * @brief HASH input invalid
 */
#define EHSM_ERR_HASH_INPUT_INVALID     0x2D43U

/**
 * @brief HASH input overflow
 */
#define EHSM_ERR_HASH_LEN_OVER_FLOW     0x2D44U

/**
 * @brief HASH output data all zerp
 */
#define EHSM_ERR_HASH_OUTPUT_ZERO_ALL   0x2D45U

/**
 *@brief TRNG buffer is NULL. 
 */
#define EHSM_ERR_TRNG_BUFFER_NULL       0x2D60U

/**
 *@brief TRNG input is invalied.
 */
#define EHSM_ERR_TRNG_INVALID_INPUT     0x2D61U

/**
 *@brief TRNG with invalid configuration.
 */
#define EHSM_ERR_TRNG_INVALID_CONFIG    0x2D62U

/**
 *@brief TRNG HT error.
 */
#define EHSM_ERR_TRNG_HT_ERROR          0x2D63U

/**
 *@brief TRNG timeout while working.
 */
#define EHSM_ERR_TRNG_TIMEOUT_ERROR     0x2D64U

/**
 *@brief CTR_DRBG reseed fail.
 */
#define EHSM_ERR_DRBG_RESEED_FAILED     0x2D65U

/**
 *@brief CTR_DRBG buffer is NULL.     
 */
#define EHSM_ERR_DRBG_BUFFER_NULL       0x2D66U

/**
 *@brief CTR_DRBG input length is invalid.     
 */
#define EHSM_ERR_DRBG_LENGTH_INVALID    0x2D67U

/**
 *@brief CTR_DRBG input length is not multiple to 8.   
 */
#define EHSM_ERR_DRBG_LENGTH_NOT_MUL_8  0x2D68U

/**
 *@brief CTR_DRBG DF overflow.    
 */
#define EHSM_ERR_DRBG_DF_OVERFLOW       0x2D69U

/**
 * @brief Certificate parse fail.
 */
#define EHSM_ERR_CRYPTO_CERT_PARSE_FAILED         0x2D80U

/**
 * @brief Certificate verify failed
 */
#define EHSM_ERR_CRYPTO_CERT_VERIFY_FAILED        0x2D81U

/**
 * @brief Wrong id size for SM9.
 */
#define EHSM_ERR_CRYPTO_SM9_WRONG_ID_SIZE         0x2D90U

/**
 * @brief Wrong key2 size for SM9.
 */
#define EHSM_ERR_CRYPTO_SM9_WRONG_K2_SIZE         0x2D91U

/**
 * @brief TRNG IP work general error by crypto software
 *
 * @note Not recommend this error code since we don't know what happens.
 */
#define EHSM_ERR_TRNG_WORK_ERROR        0x2DFCU

/**
 * @brief SKE IP work err by crypto software
 *
 * @note Not recommend this error code since we don't know what happens.
 */
#define EHSM_ERR_SKE_WORK_ERROR         0x2DFDU

/**
 * @brief PKE IP work err by crypto software
 *
 * @note Not recommend this error code since we don't know what happens.
 */
#define EHSM_ERR_PKE_WORK_ERROR         0x2DFEU

/**
 * @brief HASH IP work err by crypto software
 *
 * @note Not recommend this error code since we don't know what happens.
 */
#define EHSM_ERR_HASH_WORK_ERROR        0x2DFFU

/**
 * @brief Given process mode is wrong
 */
#define EHSM_ERR_WRONG_PROC_MODE        0x3C00U

/**
 * @brief Given context was wrong
 */
#define EHSM_ERR_WRONG_CONTEXT          0x3C01U

/**
 * @brief Given algorithm or algorithm mode not available
 */
#define EHSM_ERR_WRONG_ALGORITHM        0x3C02U

/**
 * @brief Direction is wrong for cipher/hash/signature/verification
 */
#define EHSM_ERR_WRONG_DIRECT           0x3C03U

/**
 * @brief Wrong padding type for cipher/mac/signature/verification
 */
#define EHSM_ERR_WRONG_PADDING_TYPE     0x3C04U

/**
 * @brief Wrong eHSM address for otp writing or reading
 */
#define EHSM_ERR_WRONG_EHSM_ADDR        0x3C05U

/**
 * @brief Wrong data length
 */
#define EHSM_ERR_WRONG_DATA_LENGTH        0x3C06U

/**
 * @brief function is limited by lifecycle
 */
#define EHSM_ERR_EHSM_LIFECYCLE_LIMIT     0x3C07U

/**
 * @brief Wrong version counter
 */
#define EHSM_ERR_WRONG_VERSION_COUNTER     0x3C08U

/**
 * @brief Wrong version counter
 */
#define EHSM_ERR_EQUAL_VERSION_COUNTER     0x3C09U

/**
 * @brief two parts of data is not the same
 */
#define EHSM_ERR_DATA_CHECK_ERROR          0x3C0AU

/**
 * @brief code flag is invalid
 */
#define EHSM_ERR_INVALID_CODE_FLAG         0x3C0BU

/**
 * @brief The twice operation result does not match.
 */
#define EHSM_ERR_REG_TWICE_NOT_MATCH    0x3C0CU

/**
 * @brief No secure boot is done.
 */
#define EHSM_ERR_ERC_NO_SECURE_BOOT     0x3C0DU

/**
 * @brief Image verify failed for secure boot.
 */
#define EHSM_ERR_IMAGE_VERIFY_FAILED    0x3C0EU

/**
 * @brief This is special value for success in secure boot, image upgrade module.
 */
#define EHSM_ERR_MIDDLE_SW                0x3C0FU

/**
 * @brief Wrong challenge type for getting challenge
 */
#define EHSM_ERR_WRONG_CHALLENGE_TYPE   0x3C20U

/**
 * @brief Failed for challenge
 */
#define EHSM_ERR_CHALLENGE_FAILED       0x3C21U

/**
 * @brief The challenge has been expired.
 */
#define EHSM_ERR_CHALLENGE_EXPIRED      0x3C22U

/**
 * @brief No challenge available for the authentication.
 */
#define EHSM_ERR_NO_CHALLENGE_AVAILABLE 0x3C23U

/**
 * @brief Debug authentication was failed.
 */
#define EHSM_ERR_DEBUG_AUTH_FAILED      0x3C24U

/**
 * @brief Wrong counter access authorization value.
 */
#define EHSM_ERR_COUNTER_AUTH_FAILED    0x5A00U

/**
 * @brief Counter not initialization.
 */
#define EHSM_ERR_COUNTER_NOT_INIT       0x5A01U

/**
 * @brief Wrong id index.
 */
#define EHSM_ERR_COUNTER_WRONG_ID       0x5A02U

/**
 * @brief All counters are busy.
 */
#define EHSM_ERR_ALL_COUNTER_BUSY       0x5A03U

/**
 * @brief Invalid incrementaion of counter.
 */
#define EHSM_ERR_INVALID_COUNTER_INCREMENTATION 0x5A04U

/**
 * @brief Failed for timestamp checking.
 */
#define EHSM_ERR_CHECK_TIME_FAILED      0x5A20U

/**
 * @brief Wrong UTC time.
 */
#define EHSM_ERR_WRONG_UTC_TIME         0x5A21U

/**
 * @brief UTC timer is not synchronized.
 */
#define EHSM_ERR_UTC_TIMER_NOT_SYNC     0x5A22U

/**
 * @brief UTC timer index is invalid.
 */
#define EHSM_ERR_UTC_TIMER_INVALID_INDEX          0x5A23U

/**
 * @brief Time stamp verify failed.
 */
#define EHSM_ERR_TIME_STAMP_VERIFY_FAILED         0x5A24U

/**
 * @brief Time stamp is expired.
 */
#define EHSM_ERR_TIME_STAMP_EXPIRED     0x5A25U

/**
 * @brief Verify time stamp failed.
 */
#define EHSM_ERR_CHECK_TIME_STAMP_ERROR 0x5A26U

/**
 *@brief UTC time synchronization failed.
 */
#define EHSM_ERR_UTC_SYNCHRONIZATION_FAILED 0x5A27U

/**
 *@brief Time challenge expired.
 */
#define EHSM_ERR_TIME_CHALLENGE_EXPIRED     0x5A28U

/**
 *@brief Item data is empty.
 */
#define EHSM_ERR_DATA_EMPTY                 0x5A29U


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
#endif
