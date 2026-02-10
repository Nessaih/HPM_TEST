#ifndef EHSM_IF_EXYT_IP_H
#define EHSM_IF_EXYT_IP_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_If_Ext_Types_Ip.h"
#include "eHSM_Com_Struct_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

typedef struct
{
    ehsm_uint8_t       rev1[1];
    ehsm_uint8_t       type;
    ehsm_uint8_t       storage_alg;
    ehsm_uint8_t       need_encryption;
    const ehsm_uint8_t *pubkey_addr;
    ehsm_uint32_t      pubkey_size;
    const ehsm_uint8_t *image_addr;
    ehsm_uint32_t      image_size;
    const ehsm_uint8_t *header_addr;
    ehsm_uint32_t      header_size;
    const ehsm_uint8_t *encrypt_iv_addr;
    ehsm_uint32_t      encrypt_iv_size;
    const ehsm_uint8_t *sign_addr;
    ehsm_uint32_t      sign_size;
} ehsm_secure_boot_st;

typedef struct
{
    /* [in] Type of key, can be SHE or EVITA. */
    ehsm_uint32_t key_type;
    /* [in] Key handle */
    ehsm_uint32_t key_handle;
    /* [in] Key authentication value and size, can be NULL. */
    ehsm_uint32_t key_auth_size;
    ehsm_uint8_t *key_auth_value;
    /* [in] The input message and its size. */
    ehsm_uint32_t size;
    ehsm_uint8_t *in;
    /** Mac direction.0: Mac generation 1: Mac verification */
    ehsm_uint8_t direction;
    /*Specifies the mac algorithm.1: AES_128 2: S*/
    ehsm_uint8_t algorithm;
    /* [in] The output size of buffer in bytes. Should not be smaller than 16. */
    /* [in/out] The output buffer of cmac. */
    ehsm_uint8_t *mac;
} ehsm_fast_cmac_st;

typedef enum
{
    EHSM_POWER_MODE_NORMAL = 0x1,
    EHSM_POWER_MODE_LOW_POWER,
} ehsm_power_mode_e;

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
 * @brief This funciton is used to install random SOC keys.
 *
 * @param[in] key_type Please see ehsm_fw_random_key_type_e.
 * @param[in] key_slot Please see ehsm_fw_random_key_slot_e.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 *
 * @note This function can only be called in TEST_MODE or MANUFACTOR_MODE.
 */
ehsm_uint32_t ehsm_get_random_key(ehsm_fw_random_key_type_e key_type, ehsm_fw_random_key_slot_e key_slot);

/**
 * @brief This funciton is used to install SOC keys such as upgrade encrypt key.
 *
 * @param[in] key_type Please see ehsm_fw_encrypt_key_type_e.
 * @param[in] key_slot Please see ehsm_fw_encrypt_key_slot_e.
 * @param[in] key      The raw key or hash of public key to be encrypted.
 * @param[in] key_len  The size of key in bytes.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 *
 * @note This function can only be called in TEST_MODE or MANUFACTOR_MODE.
 */
ehsm_uint32_t ehsm_encrypt_key(ehsm_fw_encrypt_key_type_e key_type, ehsm_fw_encrypt_key_slot_e key_slot,
                               const ehsm_uint8_t *key, ehsm_uint32_t key_len);

/**
 * @brief This funciton is used to get challenge for debug authentication.
 *
 * @param[in] req The challenge type and buf to stored the challenge.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_EHSM_LIFECYCLE_LIMIT  function is limited by lifecycle
 * @retval EHSM_ERR_WRONG_CHALLENGE_TYPE  Wrong challenge type for getting challenge.
 * @retval EHSM_ERR_TRNG_WORK_ERROR       TRNG IP work err by crypto software
 *
 */
ehsm_uint32_t ehsm_get_challenge(ehsm_get_challenge_st *req);

/**
 * @brief This funciton is used for debug authentication.
 *
 * @param[in] req Info for debug authentication.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 * @retval EHSM_ERR_WRONG_CHALLENGE_TYPE  Wrong challenge type for getting challenge.
 * @retval EHSM_ERR_WRONG_ALGORITHM       Given algorithm or algorithm mode not available.
 * @retval EHSM_ERR_EHSM_LIFECYCLE_LIMIT  function is limited by lifecycle
 * @retval EHSM_ERR_WRONG_KEY_TYPE        Wrong key type.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_HASH_WORK_ERROR       HASH IP work err by crypto software
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 *
 * @note This function should be called after ehsm_get_challenge
 */
ehsm_uint32_t ehsm_debug_auth(void *req);

/**
 * @brief Secure upgrade service init. Only supported by eHSM firmware.
 *
 * @param [in] upg_info: Upgrade info header.
 * @param [in] upg_info_size: Size of upgrade info header in byte.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 * @retval EHSM_ERR_WRONG_VERSION_COUNTER Wrong version counter
 * @retval EHSM_ERR_WRONG_ALGORITHM       Given algorithm or algorithm mode not available
 * @retval EHSM_ERR_WRONG_CONTEXT         Given context was wrong
 * @retval EHSM_ERR_DATA_CHECK_ERROR      Two parts of data is not the same
 */
ehsm_uint32_t ehsm_hsm_fw_upgrade_init(const char *upg_info, ehsm_uint32_t upg_info_size);

/**
 * @brief Secure upgrade service update. Only supported by eHSM firmware.
 *
 * @param [in] upg_encrypted_img: Part of the encrypted code image.
 * @param [in] upg_img_size: Input size in bytes, should be a multiple of the hash or symmetric algorithm block size.
 * @param [in/out] storage_img: Output storage encrypted code image.
 * @param [in/out] storage_img_size: Output code image size.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 * @retval EHSM_ERR_WRONG_VERSION_COUNTER Wrong version counter
 * @retval EHSM_ERR_WRONG_ALGORITHM       Given algorithm or algorithm mode not available
 * @retval EHSM_ERR_WRONG_CONTEXT         Given context was wrong
 * @retval EHSM_ERR_DATA_CHECK_ERROR      Two parts of data is not the same
 */
ehsm_uint32_t ehsm_hsm_fw_upgrade_update(const ehsm_uint8_t *upg_encrypted_img, ehsm_uint32_t upg_img_size, ehsm_uint8_t *storage_img,
                                         ehsm_uint32_t *storage_img_size);

/**
 * @brief Secure upgrade service finish. Only supported by eHSM firmware.
 *
 * @param [in] upg_encrypted_img: Remaining encrypted code image, may be NULL.
 * @param [in] upg_img_size: Remaining code image size in bytes, may be 0.
 * @param [in/out] storage_img: Output storage encrypted code image, may be NULL.
 * @param [in/out] storage_img_size: Output code image size, may be 0.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 * @retval EHSM_ERR_WRONG_VERSION_COUNTER Wrong version counter
 * @retval EHSM_ERR_WRONG_ALGORITHM       Given algorithm or algorithm mode not available
 * @retval EHSM_ERR_WRONG_CONTEXT         Given context was wrong
 * @retval EHSM_ERR_DATA_CHECK_ERROR      Two parts of data is not the same
 */
ehsm_uint32_t ehsm_hsm_fw_upgrade_finish(const ehsm_uint8_t *upg_encrypted_img, ehsm_uint32_t upg_img_size, ehsm_uint8_t *storage_img,
                                         ehsm_uint32_t *storage_img_size, char *mac, ehsm_uint32_t *mac_size);

/**
 * @brief Secure upgrade verify service init. Only supported by eHSM firmware.
 *
 * @param [in] code_info: Code info header.
 * @param [in] code_info_size: Size of code info header in byte.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 * @retval EHSM_ERR_WRONG_VERSION_COUNTER Wrong version counter
 * @retval EHSM_ERR_WRONG_ALGORITHM       Given algorithm or algorithm mode not available
 * @retval EHSM_ERR_WRONG_CONTEXT         Given context was wrong
 * @retval EHSM_ERR_DATA_CHECK_ERROR      Two parts of data is not the same
 */
ehsm_uint32_t ehsm_hsm_fw_upgrade_verify_init(const ehsm_uint8_t *code_info, ehsm_uint32_t code_info_size);

/**
 * @brief Secure upgrade verify service update. Only supported by eHSM firmware.
 *
 * @param [in] encrypted_img: Part of the encrypted code image.
 * @param [in] img_size: Input size in bytes, should be a multiple of both storage encrypt algorithm (aes or sm4) and
 *                       secure boot algorithm (symmetric or hash).
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 * @retval EHSM_ERR_WRONG_VERSION_COUNTER Wrong version counter
 * @retval EHSM_ERR_WRONG_ALGORITHM       Given algorithm or algorithm mode not available
 * @retval EHSM_ERR_WRONG_CONTEXT         Given context was wrong
 * @retval EHSM_ERR_DATA_CHECK_ERROR      Two parts of data is not the same
 */
ehsm_uint32_t ehsm_hsm_fw_upgrade_verify_update(const ehsm_uint8_t *encrypted_img, ehsm_uint32_t img_size);

/**
 * @brief Secure upgrade verify service finish. Only supported by eHSM firmware.
 *
 * @param [in] encrypted_img: Remaining encrypted code image, may be NULL.
 * @param [in] img_size: Remaining code image size in bytes, may be 0.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 * @retval EHSM_ERR_WRONG_VERSION_COUNTER Wrong version counter
 * @retval EHSM_ERR_WRONG_ALGORITHM       Given algorithm or algorithm mode not available
 * @retval EHSM_ERR_WRONG_CONTEXT         Given context was wrong
 * @retval EHSM_ERR_DATA_CHECK_ERROR      Two parts of data is not the same
 */
ehsm_uint32_t ehsm_hsm_fw_upgrade_verify_finish(const ehsm_uint8_t *encrypted_img, ehsm_uint32_t img_size);

#ifdef CONFIG_EHSM_SOC_UPGRADE_AND_VERIFY
ehsm_uint32_t ehsm_soc_image_upgrade(ehsm_soc_image_upgrade_info_st *upgrade_info);
//ehsm_uint32_t ehsm_soc_image_upgrade_init(ehsm_uint8_t storage_algorithm, ehsm_uint8_t upgrade_algorithm, ehsm_uint8_t storage_need_encryption,
//								ehsm_uint8_t is_upgrade_encrytion, const ehsm_uint8_t *upgrade_iv, ehsm_uint32_t upgrade_iv_size, 
//								const ehsm_uint8_t *storage_iv, ehsm_uint32_t storage_iv_size, const ehsm_uint8_t *head, ehsm_uint32_t head_size);
ehsm_uint32_t ehsm_soc_image_upgrade_init(ehsm_uint8_t storage_algorithm, ehsm_uint8_t upgrade_algorithm, ehsm_uint8_t storage_need_encryption,
									ehsm_uint8_t is_upgrade_encrytion, const ehsm_uint8_t *upgrade_iv, ehsm_uint32_t upgrade_iv_size, 
									const ehsm_uint8_t *storage_iv, ehsm_uint32_t storage_iv_size, const ehsm_uint8_t *head, 
									ehsm_uint32_t head_size, const ehsm_uint8_t *upgrade_pubkey, ehsm_uint32_t upgrade_pubkey_size,
									const ehsm_uint8_t *upgrade_sign, ehsm_uint32_t upgrade_sign_size);								

ehsm_uint32_t ehsm_soc_image_upgrade_finup(ehsm_uint8_t processs_mode, const ehsm_uint8_t *upgrade_image,
								ehsm_uint32_t upgrade_image_size, ehsm_uint8_t *storage_image, ehsm_uint8_t storage_image_size, 
								const ehsm_uint8_t *upgrade_sign, ehsm_uint32_t upgrade_sign_size, ehsm_uint8_t *mac_sign, ehsm_uint32_t mac_sign_size);

//ehsm_uint32_t ehsm_soc_image_verify(ehsm_uint32_t verify_alg, const ehsm_uint8_t *sign_data, ehsm_uint32_t sign_data_len, const ehsm_uint8_t *pubkey,
//								ehsm_uint32_t pubkey_size, ehsm_uint8_t is_encrypted, const ehsm_uint8_t *image_iv, ehsm_uint32_t iv_size,
//								const ehsm_uint8_t *head, ehsm_uint32_t head_size, const ehsm_uint8_t *image, ehsm_uint32_t image_size);
ehsm_uint32_t ehsm_soc_image_verify(ehsm_soc_image_verify_info_st *verify_info);
#endif

/**
 * @brief Secure boot verification. Only supported by eHSM firmware.
 *
 * @param [in] req The image info (header and code) to be verified.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 * @retval EHSM_ERR_WRONG_VERSION_COUNTER Wrong version counter
 * @retval EHSM_ERR_WRONG_ALGORITHM       Given algorithm or algorithm mode not available
 * @retval EHSM_ERR_WRONG_CONTEXT         Given context was wrong
 * @retval EHSM_ERR_DATA_CHECK_ERROR      Two parts of data is not the same
 */
ehsm_uint32_t ehsm_secure_boot(ehsm_soc_image_verify_input_st *req);

/**
 * @brief Request to let eHSM enter low power mode.
 *
 * @param [in] power_mode: power mode
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_DEBUG_AUTH_FAILED     No authentication is generated before, or authentication is failed.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 */
ehsm_uint32_t ehsm_low_power(ehsm_power_mode_e power_mode);

/**
 * @brief Set uart baudrate.
 *
 * @param [in] baudrate: baudrate to be set, refers to ehsm_uart_baudrate_e.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 */
ehsm_uint32_t ehsm_set_uart_baudrate(ehsm_uint32_t baudrate);

/**
 * @brief Write data to otp area.
 *
 * @param [in] otp_addr: The address of OTP, where data being write from.
 * @param [in] buf: The start address of OTP data buffer.
 * @param [in] write_size: The size of buffer, The maximum length is 1024 bytes;
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_EHSM_LIFECYCLE_LIMIT  function is limited by lifecycle
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 */
ehsm_uint32_t ehsm_write_otp_data(ehsm_uint32_t otp_addr, ehsm_uint8_t *buf, ehsm_uint32_t write_size);

/** @brief Read data from otp area.
 *
 * @param [in] otp_addr: The address of OTP, where data being read from.
 * @param [in] buf: The start address of OTP data buffer.
 * @param [in] read_size: The size of buffer, The maximum length is 1024 bytes;
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error. 
 */
ehsm_uint32_t ehsm_read_otp_data(ehsm_uint32_t otp_addr, ehsm_uint8_t *buf, ehsm_uint32_t read_size);


/**
 * @brief Request to change lifecycle.
 *
 * @param [in] aim: The lifecycle to be changed to.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_DEBUG_AUTH_FAILED     No authentication is generated before, or authentication is failed.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 *
 * @note Partial lifecycle switching (user -> debug or user -> destroy) requires authorization with user auth key. 
 *       So the relate authorization command please invoke firstly.
 */
ehsm_uint32_t ehsm_change_lifecycle(ehsm_lifecycle_e aim);

/**
 * @brief Request to change control field.
 *
 * @param [in] type: The control field type.
 * @param [in] value: The control field value.
 * @param [in] size: The control field value size.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_DEBUG_AUTH_FAILED     No authentication is generated before, or authentication is failed.
 */
ehsm_uint32_t ehsm_change_controlfield(ehsm_control_field_type_e type, ehsm_uint8_t *value, ehsm_uint16_t size);

/**
 * @brief Request to get emu status.
 *
 * @param [in] emu: Host address to stored the EMU status.
 * @param [in/out] size: Input with the buffer size in bytes. It will be updated with the output data size.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_NOT_SUPPORT           The info type is not supported.
 * @retval EHSM_ERR_WRONG_KEY_HANDLE      Given key handle is wrong
 */
ehsm_uint32_t ehsm_get_emu_status(ehsm_uint8_t *emu, ehsm_uint32_t *size);

#ifdef CONFIG_EHSM_DEBUG
/**
 * @brief Reset the ehsm firmware.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 *
 * @note This is just for debug.
 */
ehsm_uint32_t ehsm_reset_firmware(void);
#endif



/**
 * @brief Generate KGC's master key pair
 *
 * @param [in] key_type: Master key type of SM9, sign master key, encryption master key or exchange master key
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 */
ehsm_uint32_t ehsm_sm9_generate_master_key(ehsm_sm9_master_key_type_e key_type);

/**
 * @brief Generate user's private key
 *
 * @param [in] key_type: User private key type, sign user private key, encryption user private key, exchange user
 *                       private key or exchange user temp key
 * @param [in] user_id: User ID
 * @param [in] id_size: Size of user ID
 * @param [in] type: Memory_target non-volatile or RAM
 * @param [in] hid: User private key generation function identity, published by KGC, default value is 0x01 for sign user
 *                  private key, 0x02 for exchange user private key, 0x03 for encryption user private key, 0x02 for
 *                  exchange user temp key.
 * @param [out] key_handle: Returned key handle of user private key.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 */
ehsm_uint32_t ehsm_sm9_generate_priv_key(ehsm_sm9_user_privkey_type_e key_type, ehsm_uint8_t *user_id,
                                         ehsm_uint32_t id_size, ehsm_key_mem_type_e type, ehsm_uint8_t hid,
                                         ehsm_uint32_t *key_handle);

/**
 * @brief SM9 Key exchange
 *
 * @param [in] role: Local user's role(SM9_Role_Sponsor or SM9_Role_Responsor)
 * @param [in] key_size: The output key bytes
 * @param [in] user_tmp_key_handle: Key handle of local user's temporary key.
 * @param [in] user_priv_key_handle: Key handle of local user's private key.
 * @param [in] peer_tmp_pub: Peer user's temporary public key.
 * @param [in] peer_id: Peer user's ID.
 * @param [in] ida_size: Size of peer user's ID.
 * @param [in] self_id: Self user ID.
 * @param [in] idb_size: Size of self user ID.
 * @param [in] s1_s2: Sponsor's S1, or responsor's S2
 * @param [in] s1_s2_size: Size of s1_s2
 * @param [in] sa_sb: Sponsor's SA, or responsor's SB
 * @param [in] sa_sb_size: Size of sa_sb
 * @param [in] fp12g: The value of e(P1, pub_key), if set to NULL, it will be calculated in eHSM.
 * @param [in] pub_key: KGC's encryption master public key, pub_key size is 64 byte.
 * @param [out] key_handle: Returned key handle
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 */
ehsm_uint32_t ehsm_sm9_exchg_key(ehsm_uint8_t role, ehsm_key_mem_type_e storage_type, ehsm_uint32_t shared_key_size,
                                 ehsm_uint32_t user_tmp_key_handle, ehsm_uint32_t user_priv_key_handle,
                                 const ehsm_uint8_t *peer_tmp_pub,
                                 const ehsm_uint8_t *peer_id, ehsm_uint32_t peer_id_size,
                                 const ehsm_uint8_t *self_id, ehsm_uint32_t self_id_size,
                                 const ehsm_uint8_t *fp12g, ehsm_uint8_t kgc_pub_key[64],
                                 ehsm_uint8_t *s1_s2, ehsm_uint32_t *s1_s2_size, ehsm_uint8_t *sa_sb,
                                 ehsm_uint32_t *sa_sb_size, ehsm_uint32_t *key_handle);

/**
 * @brief SM9 key encapsulation
 *
 * @param [in] user_id: User ID
 * @param [in] id_size: Size of user ID
 * @param [in] key_addr: Address of the generated key and its cipher. The buffer should be key_size + 64
 * @param [in/out] key_size: Size of generated key.
 * @param [in] fp12g: The value of e(P1, pub_key), if set to NULL, it will be calculated in eHSM.
 * @param [in] pub_key: KGC's encryption master public key, pub_key size is 64 bytes.
 * @param [in] hid: User private key generation function identity, published by KGC, default value is 0x03, one byte,
 *                  ingnroed in signature mode.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 */
ehsm_uint32_t ehsm_sm9_wrap_key(const ehsm_uint8_t *user_id, ehsm_uint32_t id_size, ehsm_uint8_t *key_addr,
                                ehsm_uint32_t *key_size, const ehsm_uint8_t *fp12g, const ehsm_uint8_t pub_key[64],
                                ehsm_uint8_t hid);

/**
 * @brief SM9 key decapsulation
 *
 * @param [in] user_priv_key_handle: Key handle of user private key
 * @param [in] cipher_addr: Address of the wrapped key.
 * @param [in] cipher_size: Size of the wrapped key
 * @param [in] user_id: User ID
 * @param [in] id_size: Size of User ID
 * @param [in] key_addr: Address of the unwrapped key.
 * @param [in] key_size: Size of the unwrapped key.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 */
ehsm_uint32_t ehsm_sm9_unwrap_key(ehsm_uint32_t user_priv_key_handle, const ehsm_uint8_t *cipher_addr,
                                  ehsm_uint32_t cipher_size, const ehsm_uint8_t *user_id, ehsm_uint32_t id_size,
                                  ehsm_uint8_t *key_addr, ehsm_uint32_t *key_size);

/** @brief generate user exchange temporary key pair.
 *
 * @param [in] user_id: User ID
 * @param [in] id_size: Size of user ID
 * @param [in] type: Memory_target non-volatile or RAM
 * @param [in] hid: User private key generation function identity, published by KGC, default value is 0x01 for sign
 *                  user private key, 0x02 for exchange user private key, 0x03 for encryption user private key, 0x02 for
 *                  exchange user temp key.
 * @param [in] pub_key: KGC's encryption master public key, pub_key size is 64 byte.
 * @param [out] key_handle: Returned key handle of user private key.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 */
ehsm_uint32_t ehsm_sm9_exckey_gen_tmpkey(const ehsm_uint8_t *peer_id, ehsm_uint32_t peer_id_size,
                                         ehsm_key_mem_type_e type, ehsm_uint8_t hid,
                                         const ehsm_uint8_t kgc_pub_key[64], ehsm_uint32_t *key_handle);

/**
 * @brief SM9 key export
 *
 * @param [in] key_handle: Key handle of the key to be exported
 * @param [in] key_blob: Address of the exported key blob
 * @param [in/out] key_blob_size: In for Size of buffer size, out for the real key blob size
 * @param [in] key_auth_value: Address of the authorization code for the exported key.
 * @param [in/out] key_auth_size: In for Size of buffer size, out for the real authorization code size
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 */
ehsm_uint32_t ehsm_sm9_export_key(ehsm_uint32_t key_handle, ehsm_uint8_t *key_blob, ehsm_uint32_t *key_blob_size,
                                  ehsm_uint8_t *key_auth_value, ehsm_uint32_t *key_auth_size);

/**
 * @brief SM9 key import
 *
 * @param [in] key_blob: Address of the key blob to be imported
 * @param [in] key_blob_size: Size of key_blob
 * @param [in] key_handle: Retured key handle
 * @param [in] type: type of imported key
 * @param [in] key_auth_value: Address of the authorization code
 * @param [in] key_auth_size: Aize of the authorization code
 * @param [in] key_is_plain: Import key is plain
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 */
ehsm_uint32_t ehsm_sm9_import_key(ehsm_uint32_t *key_handle, const ehsm_uint8_t *key_blob, ehsm_uint32_t key_blob_size,
                                  ehsm_key_mem_type_e type, ehsm_uint8_t *key_auth_value, ehsm_uint32_t key_auth_size,
                                  ehsm_uint8_t key_is_plain);

/**
 * @brief Generate KGC's master public key from master private key
 *
 * @param [in] key_type: Masker key type
 * @param [out] pub_key: Buffer for returned public key
 * @param [out] pub_key_size: Size of returned public key
 *
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software 
 */
ehsm_uint32_t ehsm_sm9_gen_mastpubkey_from_mastprivkey(ehsm_sm9_master_key_type_e key_type,
                                                       ehsm_uint8_t pub_key[128], ehsm_uint32_t *pub_key_size);

/**
 * @brief Generate user's temporary public key from private key for SM9 key-exchange system.
 *
 * @param [in] key_handle: Key handle of local's temporary private key
 * @param [in] user_id: User ID
 * @param [in] id_size: Size of User ID
 * @param [in] hid: user private key generation function identity, published by KGC, default value is 0x02, one byte.
 * @param [in] pub_key: Buffer for returned public key, pub_key size is 64 byte, so buffer must be larger than 64 bytes.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 */
ehsm_uint32_t ehsm_sm9_gen_tmppubkey_from_tmpprivkey(ehsm_uint32_t key_handle, const ehsm_uint8_t *user_id,
                                                     ehsm_uint32_t id_size, ehsm_uint8_t hid, ehsm_uint8_t pub_key[64]);

/**
 * @brief Remove a SM9 key
 *
 * @param [in] key_handle: Key handle to be removed.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 */
ehsm_int32_t ehsm_sm9_remove_key(ehsm_uint32_t key_handle);

/**
 * @brief SM9 encryption and decryption
 *
 * @param [in] enc_type: type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 * @param [in] dir: encryption or decryption
 * @param [in] padding: type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 * @param [in] input: input address of data
 * @param [in] input_size: the size of input data
 * @param [out] output: cipher text of plain text
 * @param [out] output_size: the size of cipher, only valid when in encryption mode
 * @param [in] id_addr: the address of identity of user B, user B is the cipher receiver
 * @param [in] id_size: the size of identity of user B
 * @param [in] fp12g: the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 * @param [in] kgc_pubkey: KGC's system encryption master public key
 * @param [in] key2_size: bytes length of the key K2 in MAC function
 * @param [in] hid: user private generation function identity, published by KGC, default value is 0x03, one byte.
 * @param [in] key_handle: key handle in sm9
 * 
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 */
ehsm_uint32_t ehsm_sm9_cipher(ehsm_uint8_t enc_type, ehsm_uint8_t dir, ehsm_uint8_t padding, ehsm_uint32_t key2_size,
                              ehsm_uint8_t hid, ehsm_uint32_t key_handle,
                              const ehsm_uint8_t *id_addr, ehsm_uint32_t id_size,
                              const ehsm_uint8_t *fp12g, ehsm_uint8_t const *kgc_pubkey,
                              const ehsm_uint8_t *input, ehsm_uint32_t input_size,
                              ehsm_uint8_t *output, ehsm_uint32_t *output_size);

/**
 * @brief SM9 signifacation and verification
 *
 * @param [in] dir: encryption or decryption
 * @param [in] key_handle: key id
 * @param [in] input: input address of msg
 * @param [in] input_size: the size of input msg
 * @param [inout] signature: address of signature to be generated or verified
 * @param [in] id_addr: the address of identity of user B, user B is the cipher receiver
 * @param [in] id_size: the size of identity of user B
 * @param [in] fp12g: the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 * @param [in] kgc_pubkey: KGC's system encryption master public key
 * @param [in] hid: user private generation function identity, published by KGC, default value is 0x03, one byte.
 * @param [in] auth_code: authriozation code
 * @param [in] auth_code_sz: size of authriozation code
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_OUT_OF_MEM            No memery to handle this request.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @retval EHSM_ERR_PKE_WORK_ERROR        PKE IP work err by crypto software
 * @retval EHSM_ERR_WRONG_DATA_LENGTH     Wrong data length
 */
ehsm_uint32_t ehsm_sm9_sign(ehsm_uint8_t dir, ehsm_uint8_t hid, ehsm_uint32_t key_handle,
                            const ehsm_uint8_t *id_addr, ehsm_uint32_t id_size,
                            const ehsm_uint8_t *fp12g, const ehsm_uint8_t *kgc_pubkey,
                            const ehsm_uint8_t *input, ehsm_uint32_t input_size, ehsm_uint8_t *signature);

/**
 * @brief This function is used to close authentication result(the previous authentication results will be cleared).
 *
 * @param [in] type: which debug type to be closed
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            The function has been successfully executed
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @note
 */
ehsm_uint32_t ehsm_close_debug(ehsm_challenge_type_e type);


#ifdef CONFIG_EHSM_DEBUG
/**
 * @brief Request to get lifecycle.
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            The function has been successfully executed
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 * @note
 */
ehsm_uint32_t ehsm_get_fw_lifecycle(ehsm_lifecycle_e *lf);

/**
 * @brief erase otp area data.
 *
 * @param [in] otp_addr: The address of OTP erase area.
 * @param [in] otp_size: The size of OTP erase area;
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 */
ehsm_uint32_t ehsm_erase_otp_data(ehsm_uint32_t otp_addr, ehsm_uint32_t otp_size);

/**
 * @brief erase flash area data.
 *
 * @param [in] flash_addr: The address of flash erase area.
 * @param [in] flash_size: The size of flash erase area;
 *
 * @return ehsm_uint32_t
 * @retval EHSM_ERR_SW_SUCCESS            No error.
 * @retval EHSM_ERR_PARAM_ERROR           Parameter error.
 * @retval EHSM_ERR_GENERAL_ERROR         Common error.
 */
ehsm_uint32_t ehsm_erase_flash_data(ehsm_uint32_t flash_addr, ehsm_uint32_t flash_size);
#endif

#endif
