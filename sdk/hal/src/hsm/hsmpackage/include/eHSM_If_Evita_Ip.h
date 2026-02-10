#ifndef EHSM_EVITA_IF_IP_H
#define EHSM_EVITA_IF_IP_H
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_Mgr_Ctx_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
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
/**
 * @brief Create a random key according to the specified algorithm
 *   
 * @param [in] target_algorithm reference to target algorithm for key generation/usage based on RNG outputs, 
 *                              otherwise = 0 (cf. hardware interface data structures)
 * @param [in] key_size         if algorithm_identifier = 0 (just random string) then define key length in bits,
 *                              otherwise ignored
 * @param [in] valid_until      define key life limitation as UTC time
 * @param [in] type             memory_target non-volatile or RAM
 * @param [in] key_usages_size  key usages size
 * @param [in] key_usages_data  key usages data, refers to ehsm_key_usages_st
 * @param [out] key_handle      return key handle for later use in other functions
 *   
 * @return ehsm_uint32_t 
 * @retval EVITA_OK                        Request successful
 * @retval EVITA_ALGORITHM_ERROR           Given algorithm or algorithm mode not available
 * @retval EVITA_INVALID_KEY_SIZE          Given key size is invalid (e.g., too small or too large)
 * @retval EVITA_ALL_KEY_SPACE_OCCUPIED    No resources left to create an additional key
 * @retval EVITA_INVALID_KEY_FLAG          Given key flag is invalid (e.g., wrong combination)
 * @retval EVITA_WRONG_AUTHORIZATION       Given authorization structure does not fit  key flags (e.g., authorization
 *                                         definition for a certain flag missing)
 * @note       
 */
#ifdef CONFIG_EHSM_HW_UTC_TIME
ehsm_uint32_t Create_Random_Key(ehsm_uint32_t target_algorithm, ehsm_uint32_t key_size,
                                ehsm_uint32_t valid_until,
                                ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size,
                                ehsm_uint8_t *key_usages_data, ehsm_uint32_t *key_handle);
#else
ehsm_uint32_t Create_Random_Key(ehsm_uint32_t target_algorithm, ehsm_uint32_t key_size,
                                ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size,
                                ehsm_uint8_t *key_usages_data, ehsm_uint32_t *key_handle);
#endif
/**
 * @brief Create a Diffie-Hellman key according to the specified algorithm
 *   
 * @param [in] target_algorithm      reference to target algorithm for key generation/usage based on DH outputs,
 *                                   otherwise = 0
 * @param [in] key_size              if algorithm_identifier = 0 then define key length in bits, otherwise ignored; must
 *                                   not be greater than DH parent key lengths otherwise error
 * @param [in] valid_until           define key life limitation as UTC time
 * @param [in] type                  memory_target non-volatile or RAM
 * @param [in] key_usages_size       key usages size
 * @param [in] key_usages_data       key usages data, refers to ehsm_key_usages_st
 * @param [in] local_key_handle      refers to internal private key that will be used by DH
 * @param [in] local_key_auth_size   size of local key usage authorization value (0 for none)
 * @param [in] local_key_auth_value  local key usage authorization (i.e.,password)
 * @param [in] remote_key_handle     refers to public remote key that will be used by DH
 * @param [in] remote_key_auth_size  size of remote key usage authorization value (0 for none)
 * @param [in] remote_key_auth_value remote key usage authorization (i.e.,password)
 * @param [out] key_handle           return key handle of DH-calculated shared secret for later use in other functions
 *   
 * @return ehsm_uint32_t 
 * @retval EVITA_OK                      Request successful
 * @retval EVITA_ALGORITHM_ERROR         Given algorithm or algorithm mode not available
 * @retval EVITA_INVALID_KEY_SIZE        Given key size is invalid (e.g., too small or too large)
 * @retval EVITA_ALL_KEY_SPACE_OCCUPIED  No resources left to create an additional key
 * @retval EVITA_INVALID_KEY_FLAG        Given key flag is invalid (e.g., wrong combination)
 * @retval EVITA_WRONG_AUTHORIZATION     Given authorization structure does not fit  key flags (e.g., authorization
 *                                       definition for a certain flag missing)
 * @retval EVITA_WRONG_KEY_HANDLE        Given key handle is unknown or wrong (e.g., not for this algorithm or mode)
 * @retval EVITA_WRONG_REMOTE_KEY_HANDLE Given remote key handle is unknown or wrong (e.g., not for this algorithm or
 *                                       this mode)
 * @retval EVITA_WRONG_KEY_COMBINATION   Keys do not fit the algorithm (e.g., RSA key vs. ECDH)
 * @retval EVITA_AUTHORIZATION_FAILED    Given authorization value was wrong
 * @note       
 */
 #ifdef CONFIG_EHSM_HW_UTC_TIME
ehsm_uint32_t Create_Dh_Key(ehsm_uint32_t target_algorithm, ehsm_uint32_t key_size,
                            ehsm_uint32_t valid_until,
                            ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size, ehsm_uint8_t *key_usages_data,
                            ehsm_uint32_t local_key_handle, ehsm_uint32_t local_key_auth_size,
                            ehsm_uint8_t *local_key_auth_value, ehsm_uint32_t remote_key_handle,
                            ehsm_uint32_t remote_key_auth_size, ehsm_uint8_t *remote_key_auth_value, ehsm_uint8_t *ss_addr,ehsm_uint8_t dh_mode,
                            ehsm_uint32_t *key_handle);
#else
ehsm_uint32_t Create_Dh_Key(ehsm_uint32_t target_algorithm, ehsm_uint32_t key_size,
                            ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size, ehsm_uint8_t *key_usages_data,
                            ehsm_uint32_t local_key_handle, ehsm_uint32_t local_key_auth_size,
                            ehsm_uint8_t *local_key_auth_value, ehsm_uint32_t remote_key_handle,
                            ehsm_uint32_t remote_key_auth_size, ehsm_uint8_t *remote_key_auth_value,
                            ehsm_uint32_t *key_handle);
#endif
/**
 * @brief This function is used for importing keys into eHSM that was were generated and exported byanother EVITA module
 *        or another external trusted party
 *   
 * @param [in] transport_key_handle                reference to the internal key used for transport protection
 * @param [in] transport_key_authorization_size    size of transport key usage authorization
 * @param [in] transport_key_authorization         transport key usage authorization (i.e.,password)
 * @param [in] authenticity_key_handle             reference to the key used for authenticity code verification
 *                                                 (use_flag = verify)
 * @param [in] authenticity_key_authorization_size size of authenticity key usage authorization
 * @param [in] authenticity_key_authorization      authenticity key usage authorization (i.e., password)
 * @param [in] type memory_target                  {nv|ram}
 * @param [in] encrypted_key_size                  given encrypted key blob size
 * @param [in] encrypted_key given                 encrypted key blob
 * @param [in] key_authenticity_code_size          given key authenticity code (signature or MAC)
 * @param [in] key_authenticity_code               given key authenticity code (signature or MAC)
 * @param [out] key_handle                         reference to the (now) internal key that was imported
 *   
 * @return ehsm_uint32_t 
 * @retval EVITA_AUTHORIZATION_FAILED   Given authorization value was wrong
 * @retval EVITA_WRONG_KEY_HANDLE       Given key handle is unknown or wrong (e.g., not for this algorithm or mode)
 * @retval EVITA_TRANSPORT_IMPOSSIBLE   Given transport key is not a capable transport key or use flag cannot be
 *                                      transported or migrated
 * @retval EVITA_ALL_KEY_SPACE_OCCUPIED No resources left to create an additional key
 * @note       
 */
ehsm_uint32_t Key_Import(ehsm_uint32_t transport_key_handle, ehsm_uint32_t transport_key_authorization_size,
                         ehsm_uint8_t *transport_key_authorization, ehsm_uint32_t authenticity_key_handle,
                         ehsm_uint32_t authenticity_key_authorization_size,
                         ehsm_uint8_t *authenticity_key_authorization, ehsm_key_mem_type_e type,
                         ehsm_uint32_t encrypted_key_size, ehsm_uint8_t *encrypted_key,
                         ehsm_uint32_t key_authenticity_code_size, ehsm_uint8_t *key_authenticity_code,
                         ehsm_uint32_t *key_handle);

/**
 * @brief This function is used for exporting keys from eHSM
 *   
 * @param [in] key_handle                          reference to the internal key that becomes exported
 * @param [in] use_flags                           define set of key usages to become exported
 * @param [in] transport_key_handle                reference to the key used for transport protection
 * @param [in] transport_key_authorization_size    size of transport key usage authorization
 * @param [in] transport_key_authorization         transport key usage authorization (i.e., password)
 * @param [in] authenticity_key_handle             reference to the key used for authenticity code verification
 *                                                 (use_flag = verify)
 * @param [in] authenticity_key_authorization_size size of authenticity key usage authorization
 * @param [in] authenticity_key_authorization      authenticity key usage authorization (i.e.,password)
 * @param [out] encrypted_key_size                 returned encrypted key blob size
 * @param [out] encrypted_key                      returned encrypted key blob
 * @param [out] key_authenticity_code_size         size of key authenticity code (signature or MAC) created by transport
 *                                                 key
 * @param [out] key_authenticity_code              key authenticity code (signature or MAC) to enforce and proof module
 *                                                 internal protection
 *   
 * @return ehsm_uint32_t 
 * @retval EVITA_AUTHORIZATION_FAILED Given authorization value was wrong
 * @retval EVITA_WRONG_KEY_HANDLE     Given key handle is unknown or wrong (e.g., not for this algorithm or mode)
 * @retval EVITA_TRANSPORT_IMPOSSIBLE Given transport key is not a capable transport key or use flag cannot be
 *                                    transported or migrated  
 * @note       
 */
ehsm_uint32_t Key_Export(ehsm_uint32_t key_handle, key_act_use_flags_t *use_flags, ehsm_uint32_t transport_key_handle,
                         ehsm_uint32_t transport_key_authorization_size, ehsm_uint8_t *transport_key_authorization,
                         ehsm_uint32_t authenticity_key_handle, ehsm_uint32_t authenticity_key_authorization_size,
                         ehsm_uint8_t *authenticity_key_authorization, ehsm_uint32_t *encrypted_key_size,
                         ehsm_uint8_t *encrypted_key, ehsm_uint32_t *key_authenticity_code_size,
                         ehsm_uint8_t *key_authenticity_code);

/**
 * @brief This function creates a key in a similar way as an RNG-based key generation, but with a symmetric parent key
 *        as a base
 *   
 * @param [in] key_derivation_function_identifier reference to underlying key derivation function (KDF)
 * @param [in] key_size                           Must not be 0 if a key shorter than KDF output is allowed, otherwise
 *                                                error.
 * @param [in] valid_until                        define key life limitation as UTC time
 * @param [in] type                               memory_target non-volatile or RAM
 * @param [in] key_usages_size                    key usages size
 * @param [in] key_usages_data                    key usages data to define set of allowed key usages, cf. data
 *                                                structures section
 * @param [in] parent_key_handle                  refers to internal secret key that will be used as parent
 * @param [in] parent_key_authorization_size      size of local key usage authorization value (0 for none)
 * @param [in] parent_key_authorization_value     local key usage authorization (i.e., password)
 * @param [in] salt_size                          size of cryptographic salt
 * @param [in] salt_data                          random data for cryptographic salt
 * @param [out] key_handle                        return key handle of derived key for later use in other functions
 *   
 * @return ehsm_uint32_t 
 * @retval EVITA_ALGORITHM_ERROR           Given algorithm or algorithm mode not available
 * @retval EVITA_INVALID_KEY_SIZE          Given key size is invalid (e.g., too small or too large)
 * @retval EVITA_ALL_KEY_SPACE_OCCUPIED    No resources left to create an additional key
 * @retval EVITA_INVALID_KEY_FLAG          Given key flag is invalid (e.g., wrong combination)
 * @retval EVITA_WRONG_AUTHORIZATION       Given authorization structure does not fit  key flags
 * @retval EVITA_WRONG_KEY_HANDLE          Given key handle is unknown or wrong (e.g., not for this algorithm or mode)
 * @retval EVITA_AUTHORIZATION_FAILED      Given authorization value was wrong 
 * @note       
 */
#ifdef CONFIG_EHSM_HW_UTC_TIME
ehsm_uint32_t Create_Derived_Key(ehsm_uint32_t key_derivation_function_identifier, ehsm_uint32_t key_size,
                                 ehsm_uint32_t valid_until,
                                 ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size,
                                 ehsm_uint8_t *key_usages_data, ehsm_uint32_t parent_key_handle,
                                 ehsm_uint32_t parent_key_authorization_size,
                                 ehsm_uint8_t *parent_key_authorization_value, ehsm_uint32_t salt_size,
                                 ehsm_uint8_t *salt_data, ehsm_uint32_t *key_handle);
#else
ehsm_uint32_t Create_Derived_Key(ehsm_uint32_t key_derivation_function_identifier, ehsm_uint32_t key_size,
                                 ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size,
                                 ehsm_uint8_t *key_usages_data, ehsm_uint32_t parent_key_handle,
                                 ehsm_uint32_t parent_key_authorization_size,
                                 ehsm_uint8_t *parent_key_authorization_value, ehsm_uint32_t salt_size,
                                 ehsm_uint8_t *salt_data, ehsm_uint32_t *key_handle);
#endif
/**
 * @brief This function is used for removing loaded keys from the eHSM 
 *   
 * @param [in] key_handle             reference to key that should be removed
 * @param [in] key_authorization_size size of key remove authorization
 * @param [in] key_authorization      key remove authorization (i.e. password)
 *   
 * @return ehsm_uint32_t 
 * @retval EVITA_WRONG_KEY_HANDLE     Given key handle is unknown or wrong (e.g., not for this algorithm or mode)
 * @retval EVITA_AUTHORIZATION_FAILED Given authorization value was wrong
 * @retval EVITA_REMOVE_IMPOSSIBLE    Key is not allowed to become removed 
 * @note       
 */
ehsm_uint32_t Key_Remove(ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size,
                         ehsm_uint8_t *key_authorization);

/**
 * @brief This function is used for obtaining all public properties about a key loaded by the eHSM
 *   
 * @param [in] key_handle                           reference to key whose status is to be returned
 * @param [in] certification_key_handle             reference to key that should sign the returned key status
 *                                                  (NULL = w/o signature)
 * @param [in] certification_key_authorization_size size of certification key usage authorization
 * @param [in] certification_key_authorization      certification key usage authorization (i.e. password)
 * @param [out] key_status_size                     size of (certified) key status
 * @param [out] key_status                          (certified) key status = { public info about key || signature
 *                                                  (optional) }
 *   
 * @return ehsm_uint32_t 
 * @retval EVITA_WRONG_KEY_HANDLE      Given key handle is unknown or wrong (e.g., not for this algorithm or mode)
 * @retval EVITA_WRONG_CERT_KEY_HANDLE Given certification key handle is unknown or wrong (e.g., not enabled for
 *                                     signing)
 * @retval EVITA_AUTHORIZATION_FAILED  Given authorization value was wrong
 * @note       
 */
ehsm_uint32_t Key_Status(ehsm_uint32_t key_handle, ehsm_uint32_t certification_key_handle,
    ehsm_uint32_t certification_key_authorization_size, ehsm_uint8_t *certification_key_authorization,
    ehsm_uint32_t *key_status_size, ehsm_uint8_t *key_status);

/**
 * @brief This function is used for creating a DH key pair
 *
 * @param [in] key_size                           Must not be 0 if a key shorter than KDF output is allowed, otherwise
 *                                                error.
 * @param [in] valid_until                        define key life limitation as UTC time
 * @param [in] type                               memory_target non-volatile or RAM
 * @param [in] key_usages_size                    key usages size
 * @param [in] key_usages_data                    key usages data to define set of allowed key usages, cf. data
 *                                                structures section
 * @param [in] p                                  a prime defining the GF(p)
 * @param [in] p_size                             size of q
 * @param [in] q                                  a prime factor of p-1, aka order of g
 * @param [in] q_size                             size of q
 * @param [in] g                                  a generator of the q-order subgroup of GF(p)*
 * @param [in] g_size                             size of g
 * @param [out] key_handle                        return key handle of created key pair
 *
 * @return ehsm_uint32_t
 * @retval EVITA_ALGORITHM_ERROR           Given algorithm or algorithm mode not available
 * @retval EVITA_INVALID_KEY_SIZE          Given key size is invalid (e.g., too small or too large)
 * @retval EVITA_ALL_KEY_SPACE_OCCUPIED    No resources left to create an additional key
 * @retval EVITA_INVALID_KEY_FLAG          Given key flag is invalid (e.g., wrong combination)
 * @retval EVITA_WRONG_AUTHORIZATION       Given authorization structure does not fit  key flags
 * @note
 */
ehsm_uint32_t Create_Random_Dh_Key_Pair(ehsm_uint32_t key_size,
    ehsm_uint32_t valid_until, ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size,
    ehsm_uint8_t *key_usages_data, ehsm_uint8_t *p, ehsm_uint32_t p_size, ehsm_uint8_t *q,
    ehsm_uint32_t q_size, ehsm_uint8_t *g, ehsm_uint32_t g_size,ehsm_uint32_t *key_handle);

/**
 * @brief Generate the random number
 *
 * @param [in] algorithm_identifier     reference to associated (pseudo) random algorithm (e.g., CTRDRBG, TRNG)
 * @param [in] random_byte_request_size number of random bytes to be returned
 * @param [in] random_bytes             returned random bytes
 *
 * @return ehsm_uint32_t
 * @retval EVITA_ALGORITHM_ERROR       Given algorithm or algorithm mode not available
 * @retval EVITA_PRNG_REQUEST_OVERSIZE Requested number of random bytes exceeds PRNG limit
 * @return EVITA_TRNG_SEED_FAILURE     PRNG was unable to retrieve TRUE random seed from TRNG
 *
 * @note
 */
ehsm_uint32_t RNG_Get_Random(ehsm_uint32_t algorithm_identifier, ehsm_uint32_t random_byte_request_size,
                             ehsm_uint8_t *random_bytes);

/**
 * @brief EVITA cipher initialization, aimed to prepare all the data that used in the Cipher_Process() later
 *
 * @param [in] algorithm_identifier            Reference to associated symmetric algorithm.
 * @param [in] cipher_mode                     Indicate decryption pr encryption mode.
 * @param [in] operation_mode                  Indicate cipher mode of operation.
 * @param [in] padding                         Indicate padding scheme.
 * @param [in] total_message_length            Give total message length(can be req. by padding scheme).
 * @param [in] iv_size                         Size of given initialization vector(can be 0).
 * @param [in] iv                              Set initialization vector(it's public).
 * @param [in] key_handle                      Refer to internal key that will be used.
 * @param [in] key_authorization_size          Size of key usage authorization value(0 for none).
 * @param [in] key_authorization_value         Key usage authorization(i.e., password).
 * @param [out] session_handle->session_id     Enables interruption & parallel processing and/or session authentication.
 * @param [out] session_handle->max_chunk_size Maxium size of a chunk on Cipher_Process().
 * @param [out] session_handle->block_sz       chunk has to be a multiple of this block size(1 to max).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)           For success.
 * @retval EVITA_WRONG_KEY_HANDLE      Given key handle is unknown or wrong (e.g., not for this algorithm or this mode)
 * @retval EVITA_ALL_SESSIONS_OCCUPIED No resources left for an additional parallel session (or no parallel processing
 *                                     at all)
 * @retval EVITA_ALGORITHM_ERROR       Given algorithm or algorithm mode not available
 * @retval EVITA_WRONG_IV              Given IV does not fit the given algorithm
 * @retval EVITA_AUTHORIZATION_FAILE   Given authorization value was wron
 */
ehsm_uint32_t Cipher_Init(ehsm_uint32_t algorithm_identifier, cipher_mode_e cipher_mode, operation_mode_e operation_mode,
                          padding_scheme_e padding, ehsm_uint32_t total_message_length, ehsm_uint32_t iv_size,
                          ehsm_uint8_t *iv, ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size,
                          ehsm_uint8_t *key_authorization_value, ehsm_ctx_session_st *session_handle);

/**
 * @brief EVITA cipher process, aimed to process one or more block within the encryption and decryption process of
 *        longer messages.
 *
 * @param [in] session_handle    Session reference from cipher_Init().
 * @param [in] input_data_size   Size of input data.
 * @param [in] input_data        Input data to be encrypted or decrypted.
 * @param [out] output_data_size Size of output data(can be different to input size due to padding/de-padding).
 * @param [out] output_data      Encrypted or decrypted output data.
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given session handle is unknown or wrong
 * @retval EVITA_WRONG_CHUNK_SIZE     Given chunk size is wrong (cf. returns on initialization)
 */
ehsm_uint32_t Cipher_Process(ehsm_ctx_session_st *session_handle, ehsm_uint32_t input_data_size,
                             const ehsm_uint8_t *input_data, ehsm_uint32_t *output_data_size,
                             ehsm_uint8_t *output_data);

/**
 * @brief EVITA cipher finish, aimed to terminate the encryption or decryption process after the last input block to
 *         become encrypted or decrypted has been processed by Cipher_Process().
 *
 * @param [in] session_handle    Session handle to be released.
 * @param [out] output_data_size Size of last output data(can be 0).
 * @param [out] output_data      Last encrypted or decrypted output data(e.g., due to padding scheme).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given session handle is unknown or wrong
 */
ehsm_uint32_t Cipher_Finish(ehsm_ctx_session_st *session_handle, ehsm_uint32_t *output_data_size,
                            ehsm_uint8_t *output_data);

/**
 * @brief EVITA aead initialization, aimed to prepare all the data that used in the Aead_Process() later
 *
 * @param [in] algorithm_identifier            Reference to associated symmetric algorithm.
 * @param [in] cipher_mode                     Indicate decryption pr encryption mode.
 * @param [in] operation_mode                  Indicate cipher mode of operation.
 * @param [in] total_message_length            Give total message length(can be req. by padding scheme).
 * @param [in] iv_size                         Size of given initialization vector(can be 0).
 * @param [in] iv                              Set initialization vector(it's public).
 * @param [in] key_handle                      Refer to internal key that will be used.
 * @param [in] key_authorization_size          Size of key usage authorization value(0 for none).
 * @param [in] key_authorization_value         Key usage authorization(i.e., password).
 * @param [out] session_handle->session_id     Enables interruption & parallel processing and/or session authentication.
 * @param [out] session_handle->max_chunk_size Maxium size of a chunk on Cipher_Process().
 * @param [out] session_handle->block_sz       chunk has to be a multiple of this block size(1 to max).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)           For success.
 * @retval EVITA_WRONG_KEY_HANDLE      Given key handle is unknown or wrong (e.g., not for this algorithm or this mode)
 * @retval EVITA_ALL_SESSIONS_OCCUPIED No resources left for an additional parallel session (or no parallel processing
 *                                     at all)
 * @retval EVITA_ALGORITHM_ERROR       Given algorithm or algorithm mode not available
 * @retval EVITA_WRONG_IV              Given IV does not fit the given algorithm
 * @retval EVITA_AUTHORIZATION_FAILE   Given authorization value was wrong
 *
 * @note   iv_size should be 12 (96 bits)
 */
ehsm_uint32_t Aead_Init(ehsm_uint32_t algorithm_identifier, cipher_mode_e cipher_mode, operation_mode_e operation_mode,
                        ehsm_uint32_t total_message_length, ehsm_uint32_t iv_size,
                        ehsm_uint8_t *iv, ehsm_uint32_t aad_size, ehsm_uint32_t tag_size, ehsm_uint32_t key_handle,
                        ehsm_uint32_t key_authorization_size, ehsm_uint8_t *key_authorization_value,
                        ehsm_ctx_session_st *session_handle);

/**
 * @brief EVITA aead process, aimed to process one or more block within the encryption and decryption process of
 *        longer messages.
 *
 * @param [in] session_handle    Session reference from cipher_Init().
 * @param [in] input_data_size   Size of input data.
 * @param [in] input_data        Input data to be encrypted or decrypted.
 * @param [in] aad_size          Input aad size. Can be 0.
 * @param [in] aad               Input aad. Can be null.
 * @param [out] output_data_size Size of output data(can be different to input size due to padding/de-padding).
 * @param [out] output_data      Encrypted or decrypted output data.
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given session handle is unknown or wrong
 * @retval EVITA_WRONG_CHUNK_SIZE     Given chunk size is wrong (cf. returns on initialization)
 */
ehsm_uint32_t Aead_Process(ehsm_ctx_session_st *session_handle, ehsm_uint32_t input_data_size,
                           const ehsm_uint8_t *input_data, ehsm_uint32_t aad_size, const ehsm_uint8_t *aad,
                           ehsm_uint32_t *output_data_size, ehsm_uint8_t *output_data);

/**
 * @brief EVITA aead finish, aimed to terminate the encryption or decryption process after the last input block to
 *        become encrypted or decrypted has been processed by Aead_Process().
 *
 * @param [in] session_handle    Session handle to be released.
 * @param [out] output_data_size Size of last output data(can be 0).
 * @param [out] output_data      Last encrypted or decrypted output data(e.g., due to padding scheme).
 * @param [in/out] match         Only valid for aead verification. If verification success, match is 1. Otherwise,
 *                               this value is invalid.
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success. And the @param match will be updated with the result.
 * @retval EVITA_WRONG_SESSION_HANDLE Given session handle is unknown or wrong
 */
ehsm_uint32_t Aead_Finish(ehsm_ctx_session_st *session_handle, ehsm_uint32_t *output_data_size,
                          ehsm_uint8_t *output_data, ehsm_uint32_t *match);

/**
 * @brief EVITA MAC initialization, aimed to prepare all the data that used in the MAC_Update() later
 *
 * @param [in] algorithm_identifier: Reference to associated MAC algorithm.
 * @param [in] mac_mode{sign|timestamped_sign|verify}: Indicate MAC creation or verification mode.
 * @param [in] operation_mode: Indicate MAC type.
 * @param [in] padding_scheme: Indicate padding scheme.
 * @param [in] total_message_length: Give total message length(can be req. by padding scheme).
 * @param [in] mac_length: Length of MAC(eg., if MAC < AES block size)
 * @param [in] key_handle: Refer to internal key that will be used.
 * @param [in] key_authorization_size: Size of key usage authorization value(0 for none).
 * @param [in] key_authorization_value: Key usage authorization(i.e., password).
 * @param [out] session_handle: Enables interruption & parallel processing and/or session authentication.
 * @param [out] session_handle->max_chunk_size: Maxium size of a chunk on update().
 * @param [out] session_handle->chunk_block_size: Chunk has to be a multiple of this block size(1 to max).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)           For success.
 * @retval EVITA_WRONG_KEY_HANDLE      Given key handle is unknown or wrong (e.g., not for this algorithm or this mode)
 * @retval EVITA_MAC_LENGTH_OVERSIZE   Given MAC length for verification is greater than MAC
 * @retval EVITA_ALL_SESSIONS_OCCUPIED No resources left for an additional parallel session (or no parallel processing
 *                                     at all)
 * @retval EVITA_ALGORITHM_ERROR       Given algorithm or algorithm mode not available
 * @retval EVITA_AUTHORIZATION_FAILE   Given authorization value was wrong
 */
ehsm_uint32_t MAC_Init(ehsm_uint32_t algorithm_identifier, mac_mode_e mac_mode, operation_mode_e operation_mode,
                       padding_scheme_e padding_scheme, ehsm_uint32_t total_message_length, ehsm_uint32_t mac_length,
                       ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size,
                       ehsm_uint8_t *key_authorization_value, ehsm_ctx_session_st *session_handle);

/**
 * @brief EVITA MAC update, aimed to process one or more block within a MAC generation oe verification process of
 *         longer messages.
 *
 * @param [in] session_handle Session reference from MAC_Init().
 * @param [in] chunk_size     Size of data chunk used for MAC update.
 * @param [in] chunk_data     Data chunk byte array used for MAC update(data to be verified or protected with a MAC).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given key handle is unknown or wrong
 * @retval EVITA_WRONG_CHUNK_SIZE     Given chunk size is wrong (cf. returns on initialization)
 */
ehsm_uint32_t MAC_Update(ehsm_ctx_session_st *session_handle, ehsm_uint32_t chunk_size, const ehsm_uint8_t *chunk_data);

/** @brief EVITA MAC finish, aimed to terminate the MAC generation or verification process after the last block has been
 *         processed by MAC_Update.
 *
 * @param [in]      session_handle  Session handle to be released.
 * @param [in/out]   mac_size        Size of Data structure, as an input in verify mode and as an output in sign mode.
 * @param [in/out]   mac             Data structure include mac_size and mac value(optional with UTC_time_stamp in verify mode).
 * @param [out]     mac_match       True if calculated MAC mateches the given reference MAC(only for verify mode).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given session handle is unknown or wrong
 */
ehsm_uint32_t MAC_Finish(ehsm_ctx_session_st *session_handle, ehsm_uint32_t *mac_size, mac_st *mac, ehsm_bool_t *mac_match);

/**
 * @brief EVITA Hash initialization, aimed to make all necessary preparations for the calculation of hash values and
 *        HMACS or verification of HAMCs belonging to longer messages that cannot be processed within one function call.
 *
 * @param [in] algorithm_identifier              Reference to associated hash algorithm.
 * @param [in] hash_mode                         Indicate hash or HMAC creation or verification mode.
 * @param [in] key_handle                        Refer to internal key that will be used(only for HMAC, set to 0
 *                                               otherwise).
 * @param [in] key_authorization_size            Size of HAMC key usage authorization value(0 for none).
 * @param [in] key_authorization_value           HMAC key usage authorization(i.e., password).
 * @param [out] session_handle                   Enables interruption & parallel processing and/or session
                                                 authentication.
 * @param [out] session_handle->max_chunk_size   Maxium chunk size possible on update() (note that hash function have
 *                                               inherent padding scheme)
 * @param [out] session_handle->chunk_block_size Chunk has to be a multiple of this block size(1 to max).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)           For success.
 * @retval EVITA_WRONG_KEY_HANDLE      Given key handle is unknown or wrong (e.g., not for this algorithm or this mode)
 * @retval EVITA_ALL_SESSIONS_OCCUPIED No resources left for an additional session
 * @retval EVITA_ALGORITHM_ERROR       Given algorithm or algorithm mode not available
 * @retval EVITA_AUTHORIZATION_FAILE   Given authorization value was wrong
 */
ehsm_uint32_t Hash_Init(ehsm_uint32_t algorithm_identifier, hash_mode_e hash_mode, ehsm_uint32_t key_handle,
                        ehsm_uint32_t key_authorization_size, ehsm_uint8_t *key_authorization_value,
                        ehsm_ctx_session_st *session_handle);

/** @brief EVITA Hash update, aimed to process one or several blocks(up to the maximum allowed chunk size delivered by
 *         Hash_Init)within a hash or HAMC calculation or HMAC verification for longer messages.
 *
 * @param [in] session_handle Session reference from init() to enable parallel hash sessions.
 * @param [in] chunk_size     Size of chunk used for hash update.
 * @param [in] chunk_data     Data chunk byte array used for hash update.
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given key handle is unknown or wrong
 * @retval EVITA_WRONG_CHUNK_SIZE     Given chunk size is wrong (cf. returns on initialization)
 */
ehsm_uint32_t Hash_Update(ehsm_ctx_session_st *session_handle, ehsm_uint32_t chunk_size, const ehsm_uint8_t *chunk_data);

/**
 * @brief EVITA Hash finish, aimed to terminate the hash or HMAC calculation or HMAC verification after processing the
 *        last message block.
 *
 * @param [in] session_handle Session handle to be released.
 * @param [in/out] hash_hmac  Data structure include HASH/HMAC data and size(optional with UTC_time_stamp in HMAC
 *                            verify mode).
 * @param [out] hmac_match    True if calcalated HMAC matches the given Reference HMAC(only for HMAC verify).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given key handle is unknown or wrong
 */
ehsm_uint32_t Hash_Finish(ehsm_ctx_session_st *session_handle, hash_hmac_st *hash_hmac, ehsm_bool_t *hmac_match);

/**
 * @brief EVITA asymmetric cipher initialization, aimed to prepare all the data that used in the Sign_Update() later
 *
 * @param [in] algorithm_identifier: Reference to associated asymmetric algorithm.
 * @param [in] hash_algorithm_identifier: Indicate underlying hash algorithm.
 * @param [in] padding: Indicate padding scheme.
 * @param [in] total_message_length: Give total message length(can be req. by padding scheme).
 * @param [in] key_handle: Refer to internal key that will be used.
 * @param [in] key_authorization_size: Size of key usage authorization value(0 for none).
 * @param [in] key_authorization_value: Key usage authorization(i.e., password).
 * @param [out] session_handle: Enables interruption & parallel processing and/or session authentication.
 * @param [out] session_handle->max_chunk_size: Maxium size of a chunk on AsymCipher_Process().
 * @param [out] session_handle->chunk_block_size: chunk has to be a multiple of this block size(1 to max).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)           For success.
 * @retval EVITA_WRONG_KEY_HANDLE      Given key handle is unknown or wrong (e.g., not for this algorithm or this mode)
 * @retval EVITA_ALL_SESSIONS_OCCUPIED No resources left for an additional session
 * @retval EVITA_ALGORITHM_ERROR       Given algorithm or algorithm mode not available
 * @retval EVITA_AUTHORIZATION_FAILE   Given authorization value was wrong
 */
ehsm_uint32_t Sign_Init(ehsm_asym_alg_e algorithm_identifier, ehsm_uint32_t hash_algorithm_identifier,
                        padding_scheme_e padding, ehsm_uint32_t total_message_length, ehsm_bool_t time_stamp_signature,
                        ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size,
                        ehsm_uint8_t *key_authorization_value, ehsm_ctx_session_st *session_handle);

/**
 * @brief EVITA asymmetric cipher update, aimed to process the data in chunks
 *
 * @param [in] session_handle Session reference from Sign_Init()
 * @param [in] chunk_size     Size of chunk data.
 * @param [in] chunk_data     Pointer to chunk data.
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given key handle is unknown or wrong
 * @retval EVITA_WRONG_CHUNK_SIZE     Given chunk size is wrong (cf. returns on initialization)
 */
ehsm_uint32_t Sign_Update(ehsm_ctx_session_st *session_handle, ehsm_uint32_t chunk_size, const ehsm_uint8_t *chunk_data);

/**
 * @brief EVITA asymmetric cipher finalization, aimed to finalize the asymmetric cipher process
 *
 * @param [in] session_handle Session reference from Sign_Init()
 * @param [out] signature     Pointer to signature.
 * @param salt variable stores the address value of salt buffer.
 * @param salt_len variable stores salt buffer length.
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given key handle is unknown or wrong
 */
ehsm_uint32_t Sign_Finish(ehsm_ctx_session_st *session_handle, signature_st *signature, ehsm_uint32_t salt, ehsm_uint32_t salt_len);

/**
 * @brief EVITA asymmetric cipher initialization, aimed to prepare all the data that used in the Verify_Update() later
 *
 * @param [in] algorithm_identifier: Reference to associated asymmetric algorithm.
 * @param [in] hash_algorithm_identifier: Indicate underlying hash algorithm.
 * @param [in] padding: Indicate padding scheme.
 * @param [in] total_message_length: Give total message length(can be req. by padding scheme).
 * @param [in] key_handle: Refer to internal key that will be used.
 * @param [in] key_authorization_size: Size of key usage authorization value(0 for none).
 * @param [in] key_authorization_value: Key usage authorization(i.e., password).
 * @param [in] signature: Pointer to signature.
 * @param [out] session_handle: Enables interruption & parallel processing and/or session authentication.
 * @param [out] session_handle->max_chunk_size: Maxium size of a chunk on AsymCipher_Process().
 * @param [out] session_handle->chunk_block_size: chunk has to be a multiple of this block size(1 to max).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)           For success.
 * @retval EVITA_WRONG_KEY_HANDLE      Given key handle is unknown or wrong (e.g., not for this algorithm or this mode)
 * @retval EVITA_ALL_SESSIONS_OCCUPIED No resources left for an additional session
 * @retval EVITA_ALGORITHM_ERROR       Given algorithm or algorithm mode not available
 * @retval EVITA_AUTHORIZATION_FAILE   Given authorization value was wrong
 */
ehsm_uint32_t Verify_Init(ehsm_asym_alg_e algorithm_identifier, ehsm_uint32_t hash_algorithm_identifier,
                          padding_scheme_e padding, ehsm_uint32_t total_message_length, ehsm_uint32_t key_handle,
                          ehsm_uint32_t key_authorization_size, ehsm_uint8_t *key_authorization_value,
                          signature_st *signture, ehsm_ctx_session_st *session_handle);

/**
 * @brief EVITA asymmetric cipher update, aimed to process the data in chunks
 *
 * @param [in] session_handle Session reference from Verify_Init()
 * @param [in] chunk_size     Size of chunk data.
 * @param [in] chunk_data     Pointer to chunk data.
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given key handle is unknown or wrong
 * @retval EVITA_WRONG_CHUNK_SIZE     Given chunk size is wrong (cf. returns on initialization)
 */
ehsm_uint32_t Verify_Update(ehsm_ctx_session_st *session_handle, ehsm_uint32_t chunk_size, const ehsm_uint8_t *chunk_data);

/**
 * @brief EVITA asymmetric cipher finalization, aimed to finalize the asymmetric cipher process
 *
 * @param [in] session_handle  Session reference from Verify_Init()
 * @param [out] utc_time_stamp Vefified UTC time stamp(if available)
 * @param [out] sign_match     Indicate whether the signature match or not.
 * @param salt variable stores the address value of salt buffer.
 * @param salt_len variable stores salt buffer length.
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_WRONG_SESSION_HANDLE Given key handle is unknown or wrong
 */
ehsm_uint32_t Verify_Finish(ehsm_ctx_session_st *session_handle, ehsm_utc_time_t *utc_time_stamp, ehsm_bool_t *sign_match, ehsm_uint32_t salt, ehsm_uint32_t salt_len);

/**
 * @brief This function is used for the initial creation of a counter
 *
 * @param [in] access_authorization_size  size of counter access authorization value
 * @param [in] access_authorization_value counter access authorization value (i.e., password hash)
 * @param [out] counter_identifier        counter id for later reference
 * @param [out] counter_initial_value      initial counter value
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)           For success.
 * @retval EVITA_ALL_COUNTERS_OCCUPIED No resources left to create an additional counter
 */
ehsm_uint32_t Create_Counter(ehsm_uint32_t access_authorization_size, const ehsm_uint8_t *access_authorization_value,
                             ehsm_uint32_t *counter_identifier, ehsm_counter_value_st *counter_initial_value);

/**
 * @brief This function is used to read the value of a counter.
 *
 * @param [in] counter_identifier     counter id of counter to be read
 * @param [out] counter_current_value current counter value
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)        For success.
 * @retval EVITA_UNKNOWN_COUNTER_ID Given counter identifier is unknown
 */
ehsm_uint32_t Read_Counter(ehsm_uint32_t counter_identifier, ehsm_counter_value_st *counter_current_value);

/**
 * @brief This function is used to increment an existing counter
 *
 * @param [in] counter_identifier         id of counter to be incremented
 * @param [in] access_authorization_size  size of counter access authorization value
 * @param [in] access_authorization_value counter access authorization value (i.e., password hash)
 * @param [in] counter_incrementation     counter incrementation value
 * @param [out] counter_new_value         new counter value after incrementation
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)                    For success.
 * @retval EVITA_UNKNOWN_COUNTER_ID             Given counter identifier is unknown
 * @retval EVITA_AUTHORIZATION_FAILED           Given authorization value was wrong
 * @retval EVITA_INVALID_COUNTER_INCREMENTATION Given counter incrementation is invalid (e.g., too large)
 */
ehsm_uint32_t Increment_Counter(ehsm_uint32_t counter_identifier, ehsm_uint32_t access_authorization_size,
                                ehsm_uint8_t *access_authorization_value,
                                const ehsm_counter_value_st *counter_incrementation,
                                ehsm_counter_value_st *counter_new_value);

/**
 * @brief With this function, a previously created counter is deleted again
 *
 * @param [in] counter_identifier         id of counter to be deleted
 * @param [in] access_authorization_size  size of counter access authorization value
 * @param [in] access_authorization_value counter access authorization value (i.e., password hash)
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)          For success.
 * @retval EVITA_UNKNOWN_COUNTER_ID   Given counter identifier is unknown
 * @retval EVITA_AUTHORIZATION_FAILED Given authorization value was wrong
 */
ehsm_uint32_t Delete_Counter(ehsm_uint32_t counter_identifier, ehsm_uint32_t access_authorization_size,
                             ehsm_uint8_t *access_authorization_value);

/**
 * @brief This function allows to sign arbitrary data and to include a time stamp (in form of UTC) into the signature
 *
 * @param [in] msg_imprint_size                  size of message imprint (e.g. hash) to become time stamped
 * @param [in] msg_imprint                       message imprint (e.g. hash) to become time stamped
 * @param [in] signature_key_handle              reference key that should sign the time stamp
 * @param [in] signature_key_authorization_size  size of signature key authorization (if set)
 * @param [in] signature_key_authorization_value signature key authorization (if set)
 * @param [out] signature_size                   size of signed time stamp
 * @param [out] signature                        signed time stamp (MAC or ECDSA depending on key_handle
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)            For success.
 * @retval EVITA_INVALID_MSG_SIZE       Given message imprint size is invalid (e.g., too small or too large)
 * @retval EVITA_WRONG_KEY_HANDLE       Given key handle is unknown or wrong (e.g., not for this algorithm or this mode)
 * @retval EVITA_CLOCK_NOT_SYNCHRONIZED EVITA UTC clock is not synchronized yet
 * @retval EVITA_AUTHORIZATION_FAILED   Given authorization value was wrong
 */
ehsm_uint32_t Create_Time_Stamp(ehsm_uint32_t msg_imprint_size, const ehsm_uint8_t *msg_imprint,
                                ehsm_uint32_t signature_key_handle, ehsm_uint32_t signature_key_authorization_size,
                                ehsm_uint8_t *signature_key_authorization_value, ehsm_uint32_t *signature_size,
                                signature_st *signature);

/**
 * @brief If a time stamp has been created by the Create_Time_Stamp function described before, the Check_Time_Stamp
 *        function may be used to check if the time stamp is valid
 *
 * @param [in] msg_imprint_size                     size of message imprint (e.g. hash) to become time stamped
 * @param [in] msg_imprint                          message imprint (e.g. hash) to become time stamped
 * @param [in] verification_key_handle              reference key that should verify the time stamp
 * @param [in] verification_key_authorization_size  size of verification key authorization (if set)
 * @param [in] verification_key_authorization_value verification key authorization (if set)
 * @param [in/out] time_stamp                       size of signed time stamp
 * @param [out] time_stamp_vry                      TRUE if verification succeeded, otherwise FALSE
 * @param [out] delta                               delta between actual UTC time and time_stamp time
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)            For success.
 * @retval EVITA_INVALID_TIME_STAMP     Given time stamp could not be interpreted correctly
 * @retval EVITA_CLOCK_NOT_SYNCHRONIZED EVITA UTC clock is not synchronized yet
 */
ehsm_uint32_t Check_Time_Stamp(ehsm_uint32_t msg_imprint_size, const ehsm_uint8_t *msg_imprint,
                               ehsm_uint32_t verification_key_handle, ehsm_uint32_t verification_key_authorization_size,
                               ehsm_uint8_t *verification_key_authorization_value, const signature_st *time_stamp,
                               ehsm_bool_t *time_stamp_vry, ehsm_uint32_t *delta);

/**
 * @brief This function is used to obtain a challenge for checking the freshness of external time synchronization
 *
 * @param [out] time_sync_challenge_size size of challenge value
 * @param [out] time_sync_challenge      obtained challenge value
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)            For success.
 */
ehsm_uint32_t Get_Time_Sync_Challenge(ehsm_uint32_t *time_sync_challenge_size, ehsm_uint8_t time_sync_challenge[]);

/**
 * @brief This function is used for setting the UTC time of the EVITA module
 *
 * @param [in] utc_time                             UNIX UTC time (seconds since 1.1.1970)
 * @param [in] signature_size                       size of signature over UTC time and challenge
 * @param [in] signature                            signature over UTC time and challenge
 * @param [in] verification_key_handle              reference key that should verify the UTC reference
 * @param [in] verification_key_authorization_size  size of verification key authorization (if set)
 * @param [in] verification_key_authorization_value verification key authorization (if set)
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)                For success.
 * @retval EVITA_INVALID_UTC_TIME           Given UTC time could not be interpreted correctly
 * @retval EVITA_WRONG_KEY_HANDLE           Given key handle is unknown or wrong (e.g., not for this algorithm or this
 *                                          mode)
 * @retval EVITA_UTC_CHALLENGE_EXPIRED      Challenge was not requested or is expired already
 * @retval EVITA_UTC_SYNCHRONIZATION_FAILED Synchronization failed due to wrong signature or wrong challenge etc.
 * @retval EVITA_AUTHORIZATION_FAILED       Given authorization value was wrong
 */
ehsm_uint32_t Set_UTC_Time(ehsm_utc_time_t utc_time, ehsm_uint32_t signature_size, const ehsm_uint8_t *signature,
                           ehsm_uint32_t verification_key_handle, ehsm_uint32_t verification_key_authorization_size,
                           ehsm_uint8_t *verification_key_authorization_value);

/**
 * @brief With this function, the UTC time value is obtained from the EVITA module
 *
 * @param [out] utc_time UNIX UTC time seconds since 1.1.1970
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)            For success.
 * @retval EVITA_CLOCK_NOT_SYNCHRONIZED EVITA UTC clock is not synchronized yet
 */
ehsm_uint32_t Get_UTC_Time(ehsm_utc_time_t *utc_time);

/**
 * @brief This function is used for obtaining the actual value of the EVITA tick counter.
 *
 * @param [out] tick_value actual value of tick counter.
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)            For success.
 */
ehsm_uint32_t Get_Tick_Count(ehsm_tick_value_st *tick_value);

/**
 * @brief Get the status of eHSM.
 *
 * @param [in] flag: Indicate type of module status info to be returned.
 * @param [in] alg: Reference to signing algorithm (could also be a MAC scheme).
 * @param [in] key_handle: Reference key that signs returned status (NULL = w/osignature).
 * @param [in] key_auth_size: Size of key usage authorization value (0 fornone).
 * @param [in] key_auth_value: Key usage authorization (i.e., password).
 * @param [in/out] status_size: Size of module status structure.
 * @param [in/out] status: Module status structure data.
 * @param [in/out] sign_size: Status signature size (if requested, otherwise 0).
 * @param [in/out] sign: Status signature (if requested, otherwise NULL).
 *
 * @return ehsm_uint32_t
 * @retval EVITA_OK(value 0)               For success.
 * @retval EVITA_STATUS_TYPE_NOT_AVAILABLE Requested status type is (currently) not available for this module
 * @retval EVITA_WRONG_KEY_HANDLE          Given key handle is unknown or wrong
 * @retval EVITA_AUTHORIZATION_FAILED      Given authorization value was wrong
 */
ehsm_uint32_t Module_Status(ehsm_uint32_t flag, ehsm_uint32_t alg, ehsm_uint32_t key_handle,
                            ehsm_uint32_t key_auth_size, const ehsm_uint8_t *key_auth_value, ehsm_uint32_t *status_size,
                            ehsm_uint8_t *status, ehsm_uint32_t *sign_size, ehsm_uint8_t *sign);

/**
 * @brief The self test service.
 *
 * @param [in] flag: The flag of algorithms to be tested.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS        No error
 * @retval EHSM_ERR_OUT_OF_MEM        No memory to handle this request.
 * @retval EHSM_ERR_DEBUG_AUTH_FAILED This function is called with no authentication.
 * @retval EHSM_ERR_GENERAL_ERROR     Some algorithm self test failed.
 */
ehsm_uint32_t ehsm_self_test(ehsm_uint32_t flag);

/**
 * @brief The function use to get key public party that calculation from private party
 *
 * @param [in] key_handle                reference to the internal key used for get public key
 * @param [in] key_authorization_size    size of key usage authorization
 * @param [in] key_authorization         key usage authorization (i.e.,password)
 * @param [out] pub_value                the output public key
 * @param [in/out] pub_size              the size of public key
 * @param [in] priv_key_algo             the private key algorithm(only support sm2/ecc)
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS        No error
 * @retval EHSM_ERR_OUT_OF_MEM        No memory to handle this request.
 * @retval EHSM_ERR_DEBUG_AUTH_FAILED This function is called with no authentication.
 * @retval EHSM_ERR_GENERAL_ERROR     Some algorithm self test failed.
 */
ehsm_uint32_t ehsm_get_pub_from_priv(ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size, ehsm_uint8_t *key_authorization,
                                     ehsm_uint8_t *pub_value, ehsm_uint32_t *pub_size, ehsm_uint32_t priv_key_algo);
#endif /* EHSM_EVITA_IF_IP_H */
