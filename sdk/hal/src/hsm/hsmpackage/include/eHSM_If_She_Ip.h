#ifndef EHSM_IF_SHE_IP
#define EHSM_IF_SHE_IP

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
 #include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Types_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/* Defination of RNG algorithm */
#define SM4_CTRDRBG 0
#define AES_CTRDRBG 1

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
/** @brief SHE encryption or decryption in ECB mode. No padding is supported.
 *
 * @param [in] key_id: SHE key index, only [4, 54] are valid.
 * @param [in] in: Plaintext for encryption (or ciphertext for decryption).
 * @param [in/out] out: Ciphertext for encryption (or plaintext for decryption).
 * @param [in] size: Size of input data in bytes, should be a multiple of 16.
 * @param [in] direction: Encryption or decryption.
 *
 * @return 0 for success, a negative value for error.
 */
ehsm_uint32_t she_crypto_ecb_extend(ehsm_uint32_t key_id, const ehsm_uint8_t *in, ehsm_uint8_t *out, ehsm_uint32_t size, ehsm_uint32_t direction);

/** @brief SHE encryption or decryption in ECB mode. No padding is supported, only support 128 bit input text.
 *
 * @param [in] key_id: SHE key index, only [4, 54] are valid.
 * @param [in] in: Plaintext for encryption (or ciphertext for decryption).
 * @param [in/out] out: Ciphertext for encryption (or plaintext for decryption).
 * @param [in] direction: Encryption or decryption.
 *
 * @return 0 for success, a negative value for error.
 */
ehsm_uint32_t she_crypto_ecb(ehsm_uint32_t key_id, const ehsm_uint8_t *in, ehsm_uint8_t *out, ehsm_uint32_t direction);

/** @brief SHE encryption or decryption in CBC mode. No padding is supported.
 *
 * @param [in] key_id: SHE key index, only [4, 54] are valid.
 * @param [in] iv: Iv for CBC mode, size should be 16.
 * @param [in] in: Plaintext for encryption (or ciphertext for decryption).
 * @param [in/out] out: Ciphertext for encryption (or plaintext for decryption).
 * @param [in] size: Size of input data in bytes, should be a multiple of 16.
 * @param [in] direction: Encryption or decryption.
 *
 * @return 0 for success, a negative value for error.
 */
ehsm_uint32_t she_crypto_cbc(ehsm_uint32_t key_id, const ehsm_uint8_t *iv, const ehsm_uint8_t *in, ehsm_uint8_t *out,
                            ehsm_uint32_t size, ehsm_uint32_t direction);

/** @brief SHE mac generation. No padding is supported.
 *
 * @param [in] key_id: SHE key index, only [4, 54] are valid.
 * @param [in] msg: Message.
 * @param [in] size: Size of message in bytes, should be a multiple of 16.
 * @param [in/out] mac: The generated mac, size should not be smaller than 16.
 *
 * @return 0 for success, a negative value for error.
 */
ehsm_uint32_t she_generate_mac(ehsm_uint32_t key_id, const ehsm_uint8_t *msg, ehsm_uint32_t size, ehsm_uint8_t *mac);

/** @brief SHE mac verification. No padding is supported.
 *
 * @param [in] key_id: SHE key index, only [4, 54] are valid.
 * @param [in] msg: Message.
 * @param [in] size: Size of message in bytes, should be a multiple of 16.
 * @param [in] mac: The mac to be verified.
 * @param [in] mac_size: The mac size in byte to be verified. Belongs to [1, 16].
 * @param [in/out] vrf_status: 1 for mac matching, 0 for not matching.
 *
 * @return 0 for success, the result is written to vrf_status. A negative value for error.
 */
ehsm_uint32_t she_verify_mac(ehsm_uint32_t key_id, const ehsm_uint8_t *msg, ehsm_uint32_t size, const ehsm_uint8_t *mac,
                            ehsm_uint32_t mac_size, ehsm_uint32_t *vrf_status);

/**
 *   @brief she load key operation
 *
 *   @param [in] m1: m1 in she spec. should be 16 bytes.
 *   @param [in] m2: m2 in she spec. should be 32 bytes.
 *   @param [in] m3: m3 in she spec. should be 16 bytes.
 *   @param [out] m4: m4 in she spec. should be 32 bytes.
 *   @param [out] m5: m5 in she spec. should be 16 bytes.
 *
 *   @return ehsm_int32_t            ERC_NO_ERROR for success, other values for error.
 *   @retval ERC_NO_ERROR            No error has occurred and the command will be executed.
 *   @retval ERC_KEY_NOT_AVAILABLE   This error code is returned if a key is locked due to failed boot measurement or an active debugger.
 *   @retval ERC_KEY_INVALID         This error code is returned by SHE whenever a function is called to perform an operation with a key that is not allowed for the given operation.
 *   @retval ERC_KEY_WRITE_PROTECTED This error is returned when a key update is attempted on a memory slot that has been write protected or when an attempt to active the debugger is started when a key is write-protected.
 *   @retval ERC_KEY_UPDATE_ERROR    This error is returned when a key update did not succeed due to errors in verification of the messages.
 *   @retval ERC_MEMORY_FAILURE      This error code can be returned if the underlying memory technology is able to detect physical errors.
 *   @retval ERC_KEY_EMPTY           This error code is returned by SHE if the application attempts to use a key that has not been initialized yet.
 *   @retval ERC_GENERAL_ERROR       This error code is returned if an error not covered by the error codes above is detected inside SHE.
 *
 *   @note
 */
ehsm_uint32_t she_load_key(const ehsm_uint8_t *m1, const ehsm_uint8_t *m2, const ehsm_uint8_t *m3, ehsm_uint8_t *m4, ehsm_uint8_t *m5);

/**
 *   @brief she load key operation
 *
 *   @param [in] m1: m1 in she spec. should be 16 bytes.
 *   @param [in] m2: m2 in she spec. should be 32 bytes.
 *   @param [in] m3: m3 in she spec. should be 16 bytes.
 *   @param [out] m4: m4 in she spec. should be 32 bytes.
 *   @param [out] m5: m5 in she spec. should be 16 bytes.
 *
 *   @return ehsm_uint32_t            ERC_NO_ERROR for success, other values for error.
 *   @retval ERC_NO_ERROR            No error has occurred and the command will be executed.
 *   @retval ERC_KEY_NOT_AVAILABLE   This error code is returned if a key is locked due to failed boot measurement or an active debugger.
 *   @retval ERC_KEY_INVALID         This error code is returned by SHE whenever a function is called to perform an operation with a key that is not allowed for the given operation.
 *   @retval ERC_KEY_WRITE_PROTECTED This error is returned when a key update is attempted on a memory slot that has been write protected or when an attempt to active the debugger is started when a key is write-protected.
 *   @retval ERC_KEY_UPDATE_ERROR    This error is returned when a key update did not succeed due to errors in verification of the messages.
 *   @retval ERC_MEMORY_FAILURE      This error code can be returned if the underlying memory technology is able to detect physical errors.
 *   @retval ERC_KEY_EMPTY           This error code is returned by SHE if the application attempts to use a key that has not been initialized yet.
 *   @retval ERC_GENERAL_ERROR       This error code is returned if an error not covered by the error codes above is detected inside SHE.
 *
 *   @note
 */
ehsm_uint32_t she_load_key_extend(const ehsm_uint8_t *m1, const ehsm_uint8_t *m2, const ehsm_uint8_t *m3, ehsm_uint8_t *m4, ehsm_uint8_t *m5);

/**
 *   @brief SHE load plain key operation
 *
 *   @param [in] key: 16 bytes key in raw data.
 *
 *   @return ehsm_uint32_t      ERC_NO_ERROR for success, other values for error.
 *   @retval ERC_NO_ERROR      No error has occurred and the command will be executed.
 *   @retval ERC_GENERAL_ERROR This error code is returned if an error not covered by the error codes above is detected inside SHE.
 *
 *   @note
 */
ehsm_uint32_t she_load_plain_key(const ehsm_uint8_t *key);

/**
 *   @brief she export ram key operation
 *
 *   @param [out] m1: m1 in she spec. should be 16 bytes.
 *   @param [out] m2: m2 in she spec. should be 32 bytes.
 *   @param [out] m3: m3 in she spec. should be 16 bytes.
 *   @param [out] m4: m4 in she spec. should be 32 bytes.
 *   @param [out] m5: m5 in she spec. should be 16 bytes.
 *
 *   @return ehsm_uint32_t       ERC_NO_ERROR for success, other values for error.
 *   @retval ERC_NO_ERROR       No error has occurred and the command will be executed.
 *   @retval ERC_KEY_INVALID    This error code is returned by SHE whenever a function is called to perform an operation with a key that is not allowed for the given operation.
 *   @retval ERC_MEMORY_FAILURE This error code can be returned if the underlying memory technology is able to detect physical errors.
 *   @retval ERC_KEY_EMPTY      This error code is returned by SHE if the application attempts to use a key that has not been initialized yet.
 *   @retval ERC_GENERAL_ERROR  This error code is returned if an error not covered by the error codes above is detected inside SHE.
 *
 *   @note
 */
ehsm_uint32_t she_export_ram_key(ehsm_uint8_t *m1, ehsm_uint8_t *m2, ehsm_uint8_t *m3, ehsm_uint8_t *m4, ehsm_uint8_t *m5);

/**
 *   @brief      Generate 128 bit random number
 *
 *   @param [in] random_data_addr
 *
 *   @return     ehsm_uint32_t
 *   @retval     ERC_NO_ERROR: No error has occurred adn the command will be executed
 *   @retval     ERC_SEQUENCE_ERROR: The sequence of commands or subcommands is out of sequence
 *   @retval     ERC_RNG_SEED: The seed has not been initialized before
 *   @retval     ERC_GENERAL_ERROR: Other error code that is not include in SHE spec
 *
 *   @note
 */
ehsm_uint32_t she_rnd(ehsm_uint8_t *random_data_addr);

/**
 *   @brief      Initializes the random number generator
 *
 *
 *   @return     ehsm_uint32_t
 *   @retval     ERC_NO_ERROR: Successfully execute
 *
 *   @note       Since the rng will be initialized by hardware, this function will do nothing
 */
ehsm_uint32_t she_init_rng(void);

/**
 *   @brief      Reseed the random number generator
 *
 *
 *   @return     ehsm_uint32_t
 *   @retval     ERC_NO_ERROR: Successfully execute
 *
 *   @note       Since the rng will be reseeded by hardware, this function will do nothing
 */
ehsm_uint32_t she_extend_seed(void);

/**
 * @brief This funciton is called for the SOC secure boot in parallel.
 *
 * @param[in] size The SOC image size for secure boot. Should be (1024, 255 * 1024] bytes.
 * @param[in] data The SOC image data.
 *
 * @return ehsm_uint32_t      Secure boot result.
 * @retval ERC_NO_ERROR       Secure boot is successful.
 * @retval ERC_GENERAL_ERROR  Secure boot failed since input is illegal.
 * @retval ERC_NO_SECURE_BOOT Secure boot verification failed or secure boot has been done.
 * @retval ERC_BUSY           The request can not be handled.
 *
 * @note This function can be called only once every power on. And can be called only parallel boot is enabled.
 */
ehsm_uint32_t she_secure_boot(ehsm_uint32_t size, const ehsm_uint8_t *data);

/**
 * @brief This funciton is called to enable boot success status.
 *
 * @return ehsm_uint32_t      Result of enable boot success status.
 * @retval ERC_NO_ERROR       Operation is successful.
 * @retval ERC_NO_SECURE_BOOT 1) If sequential boot is enabled,
 *                            2) or parallel boot is enabled, but she_secure_boot is not called,
 *                            3) or she_secure_boot is called but failed,
 *                            4) or she_secure_boot is successful, but she_boot_ok has been called succussfully before,
 *                            5) or she_boot_ok is not succssuful, but she_boot_failure has been called succussfully
 *                               before.
 * @retval ERC_BUSY           The request can not be handled.
 *
 * @note This function must be called after she_secure_boot.
 */
ehsm_uint32_t she_boot_ok(void);

/**
 * @brief This funciton is called to enable boot failure status.
 *
 * @return ehsm_uint32_t      Result of enable boot failure status.
 * @retval ERC_NO_ERROR       Operation is successful.
 * @retval ERC_NO_SECURE_BOOT 1) If sequential boot is enabled,
 *                            2) or parallel boot is enabled, but she_secure_boot is not called,
 *                            3) or she_secure_boot is called but failed,
 *                            4) or she_secure_boot is successful, but she_boot_ok has been called succussfully before,
 *                            5) or she_boot_ok is not succssuful, but she_boot_failure has been called succussfully
 *                               before.
 * @retval ERC_BUSY           The request can not be handled.
 *
 * @note This function must be called after she_secure_boot.
 */
ehsm_uint32_t she_boot_failure(void);

/**
 *   @brief This function returns the eHSM status.
 *
 *   @param [in] sreg: Buffer to store the status infomation, should be 1 byte.
 *
 *   @return ehsm_uint32_t      ERC_NO_ERROR for success, other values for error.
 *   @retval ERC_NO_ERROR      No error has occurred and the command will be executed.
 *   @retval ERC_GENERAL_ERROR This error code is returned if an error not covered by the error codes above is detected inside SHE.
 *
 *   @note
 */
ehsm_uint32_t she_get_status(ehsm_uint8_t *sreg);

/**
 *   @brief The function returns the identity (UID) and the value of the status register protected by a MAC over a challenge and the data.
 *
 *   @param [in] challenge: The 128 bits challenge.
 *   @param [in] id: The output buffer to store UID, should be 120 bits.
 *   @param [in] sreg: The output buffer to store status info, should be 8 bits.
 *   @param [in] mac: The output buffer to store MAC, should be 128 bits.
 *
 *   @return ehsm_uint32_t          ERC_NO_ERROR for success, other values for error.
 *   @retval ERC_NO_ERROR          No error has occurred and the command will be executed.
 *   @retval ERC_KEY_NOT_AVAILABLE This error code is returned if a key is locked due to failed boot measurement or an active debugger.
 *   @retval ERC_MEMORY_FAILURE    This error code can be returned if the underlying memory technology is able to detect physical errors.
 *   @retval ERC_GENERAL_ERROR     This error code is returned if an error not covered by the error codes above is detected inside SHE.
 *
 *   @note
 */
ehsm_uint32_t she_get_id(const ehsm_uint8_t *challenge, ehsm_uint8_t *id, ehsm_uint8_t *sreg, ehsm_uint8_t *mac);

/**
 *   @brief Job cancel.
 *
 *   @return ehsm_uint32_t      ERC_NO_ERROR for success, other values for error.
 *   @retval ERC_NO_ERROR      No error has occurred and the command will be executed.
 *   @retval ERC_GENERAL_ERROR This error code is returned if an error not covered by the error codes above is detected inside SHE.
 *
 *   @note
 */
ehsm_uint32_t ehsm_she_cancel(void);

/**
 *   @brief Debug authentication.
 *
 *   @param [in] challenge: The 128 bits challenge.
 *   @param [in] auth: The 128 bits authentication.
 *
 *   @return ehsm_uint32_t           ERC_NO_ERROR for successful, other values for failure.
 *   @retval ERC_NO_ERROR            No error has occurred and the command will be executed.
 *   @retval ERC_KEY_WRITE_PROTECTED This error is returned when a key update is attempted on a memory slot that has been write protected or when an attempt to active the debugger is started when a key is write-protected.
 *   @retval ERC_NO_DEBUGGING        The error code is returned if internal debugging is not possible because the authentication with the challenge response protocol did not succeed.
 *   @retval ERC_MEMORY_FAILURE      This error code can be returned if the underlying memory technology is able to detect physical errors.
 *   @retval ERC_GENERAL_ERROR       This error code is returned if an error not covered by the error codes above is detected inside SHE.
 *
 *   @note
 */
ehsm_uint32_t she_debug(ehsm_uint8_t *challenge, const ehsm_uint8_t *auth);

#endif
