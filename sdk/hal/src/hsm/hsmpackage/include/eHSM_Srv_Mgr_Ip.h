#ifndef _EHSM_SERVICE_MANAGER_
#define _EHSM_SERVICE_MANAGER_

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/

#include "eHSM_Srv_CmdReq_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

#define EHSM_SRV_START                0U
#define EHSM_SRV_CRYPTO_RANDOMGENERATE 1U
#define EHSM_SRV_CRYPTO_RANDOMSEED     2U
#define EHSM_SRV_KEYMGR_KEYGENERATE    3U
#define EHSM_SRV_KEYMGR_CREATE_DERIVED_KEY 4U
#define EHSM_SRV_KEYMGR_KEYEXCHANGECALCPUBVAL 5U
#define EHSM_SRV_KEYMGR_KEYEXCHANGECALCSECRET 6U
#define EHSM_SRV_KEYMGR_CERRITIFATEPARSE 7U
#define EHSM_SRV_KEYMGR_CERRITIFATEVERIFY 8U
#define EHSM_SRV_KEYMGR_KEYSETVALID    9U
#define EHSM_SRV_KEYMGR_KEY_IMPORT     10U
#define EHSM_SRV_KEYMGR_KEY_EXPORT     11U
#define EHSM_SRV_KEYMGR_COPY_KEY       12U
#define EHSM_SRV_KEYMGR_KEYELEMENTIDSGET 13U
#define EHSM_SRV_KEYMGR_KEYSETINVALID  14U
#define EHSM_SRV_KEYMGR_KEY_REMOVE     15U
#define EHSM_SRV_KEYMGR_KEY_STATUS     16U
#define EHSM_SRV_KEYMGR_LOAD_KEY       17U
#define EHSM_SRV_KEYMGR_LOAD_PLAIN_KEY 18U
#define EHSM_SRV_KEYMGR_EXPORT_RAM_KEY 19U
#define EHSM_SRV_KEYMGR_GENERATER_SM9_KEY 20U
#define EHSM_SRV_KEYMGR_EXCHG_SM9_KEY  21U
#define EHSM_SRV_KEYMGR_SM9_GEN_TMP_KEY 22U
#define EHSM_SRV_KEYMGR_SM9_WRAP_KEY   23U
#define EHSM_SRV_KEYMGR_SM9_UNWRAP_KEY 24U
#define EHSM_SRV_KEYMGR_SM9_EXPORT_KEY 25U
#define EHSM_SRV_KEYMGR_SM9_IMPORT_KEY 26U
#define EHSM_SRV_KEYMGR_SM9_GEN_MAST_PUBKEY 27U
#define EHSM_SRV_KEYMGR_SM9_GEN_TMP_PUBKEY 28U
#define EHSM_SRV_KEYMGR_SM9_REMOVE_KEY 29U
#define EHSM_SRV_RNG_INIT              30U
#define EHSM_SRV_RNG_EXTEND_SEED       31U
#define EHSM_SRV_ADMIN_SECURE_BOOT     32U
#define EHSM_SRV_ADMIN_BOOT_STATUS     33U
#define EHSM_SRV_ADMIN_UPGRADE         34U
#define EHSM_SRV_ADMIN_VERIFY_PROGRAM  35U
#define EHSM_SRV_DEBUG_GET_CHALLENGE   36U
#define EHSM_SRV_DEBUG_AUTH            37U
#define EHSM_SRV_DEBUG_SHE_AUTH        38U
#define EHSM_SRV_TIMER                 39U
#define EHSM_SRV_COUNTER               40U
#define EHSM_SRV_SYS_SELF_TEST         41U
#define EHSM_SRV_SYS_ASR_CANCEL        42U
#define EHSM_SRV_SYS_SHE_CANCEL        43U
#define EHSM_SRV_SYS_GET_STATUS        44U
#define EHSM_SRV_SYS_GET_ID            45U
#ifdef CONFIG_EHSM_DEBUG
#define EHSM_SRV_SYS_RESET_FIRMWARE    46U
#define EHSM_SRV_SYS_GET_FW_LIFECYCLE  47U
#define EHSM_SRV_EXTENDED_OTP_ERASE    48U
#define EHSM_SRV_EXTENDED_FLASH_ERASE  49U
#define EHSM_SEV_GET_FLASH_UTC_TIME    50U
#endif
#define EHSM_SRV_SYS_HASH_FINISH_EXTEND 51U
#define EHSM_SRV_EXTENDED_FW_GET_RANDOM_KEY 52U
#define EHSM_SRV_EXTENDED_FW_ENCRYPT_KEY 53U
#define EHSM_SRV_EXTENDED_GET_CHALLENGE 54U
#define EHSM_SRV_EXTENDED_DEBUG_AUTHENTICATION 55U
#define EHSM_SRV_EXTENDED_IMAGE_UPGRADE 56U
#define EHSM_SRV_EXTENDED_IMAGE_VERIFY 57U
#ifdef CONFIG_EHSM_SOC_UPGRADE_AND_VERIFY    
#define EHSM_SRV_EXTENDED_SOC_IMAGE_UPGRADE 58U
#define EHSM_SRV_EXTENDED_SOC_IMAGE_INIT 59U
#define EHSM_SRV_EXTENDED_SOC_IMAGE_FINUP 60U
#endif    
#define EHSM_SRV_EXTENDED_SOC_IMAGE_VERIFY 61U
#define EHSM_SRV_EXTENDED_CRYPTO_SKE   62U
#define EHSM_SRV_EXTENDED_CRYPTO_PKE   63U
#define EHSM_SRV_EXTENDED_CRYPTO_HASH  64U
#define EHSM_SRV_EXTENDED_OTP_READ     65U
#define EHSM_SRV_EXTENDED_OTP_WRITE    66U
#define EHSM_SRV_DEBUG_LOAD_KEY        67U
#define EHSM_SRV_LOW_POWER             68U
#define EHSM_SRV_SET_BAUDRATE          69U
#define EHSM_SRV_CHANGE_LIFECYCLE      70U
#define EHSM_SRV_CHANGE_CONTROLFIELD   71U
#define EHSM_SRV_GET_SHE_STATUS        72U
#define EHSM_SRV_GET_SHE_ID            73U
#define EHSM_SRV_GET_EMU_STATUS        74U
#define EHSM_SRV_MODULE_STATUS         75U
#define EHSM_SRV_BOOTLOADER_CMD        76U
#define EHSM_SRV_SOC_BOOT_STATUS       77U
#define EHSM_SRV_SYS_CLOSE_DEBUG       78U
#define EHSM_SRV_SENSOR_RESP_INIT      79U
#define EHSM_SRV_END                   80U

typedef ehsm_uint8_t ehsm_cmd_ext_type_e;

typedef ehsm_uint32_t (*service_reqhdl)(void *para, ehsm_cmd_req_st *req);
typedef ehsm_uint32_t (*service_rsphdl)(void *para, ehsm_cmd_req_st *req);

typedef struct ehsm_service {
    ehsm_uint32_t service_id;
    service_reqhdl reqhdl;
    service_rsphdl rsphdl;
    /*timeout for synchronous command*/
    ehsm_uint32_t timeout;
} ehsm_service_st;

typedef struct ehsm_service_info {
    ehsm_cmd_req_type_e req_type;
    cmd_req_cb cb;
    ehsm_uint32_t priority;
    ehsm_api_type_e api_type;
    /* Store pointer of Crypto_JobType */
    void *service_ctx;
} ehsm_service_info_st;
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

ehsm_uint32_t ehsm_process_sync_service(ehsm_cmd_ext_type_e service_id, void *param, ehsm_api_type_e api_type);

ehsm_uint32_t ehsm_register_service(ehsm_service_st *service);

ehsm_uint32_t ehsm_service_init(void);

void ehsm_set_address_pointer(ehsm_uint8_t *addr_array, const ehsm_uint8_t *addr);

#ifdef CONFIG_EHSM_DEBUG
ehsm_uint32_t ehsm_process_norps_service(ehsm_uint32_t service_id, void *param, ehsm_api_type_e api_type);
#endif
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/
extern ehsm_service_st srv_crypto_randomgenerate;
extern ehsm_service_st srv_crypto_ske;
extern ehsm_service_st srv_crypto_hash;
extern ehsm_service_st srv_crypto_pke;
extern ehsm_service_st srv_she_load_key;
extern ehsm_service_st srv_she_load_plain_key;
extern ehsm_service_st srv_she_ram_key_export;
extern ehsm_service_st srv_get_she_status;
extern ehsm_service_st srv_get_she_id;
extern ehsm_service_st srv_she_cancle_cmd;
extern ehsm_service_st srv_key_copy;
extern ehsm_service_st srv_asr_cancle_cmd;
extern ehsm_service_st srv_certificate_parse;
extern ehsm_service_st srv_certificate_verify;
extern ehsm_service_st srv_create_random_key;
extern ehsm_service_st srv_derive_key;
extern ehsm_service_st srv_create_dh_key;
extern ehsm_service_st srv_derive_key;
extern ehsm_service_st srv_export_key;
extern ehsm_service_st srv_get_pub_from_priv;
extern ehsm_service_st srv_import_key;
extern ehsm_service_st srv_key_remove;
extern ehsm_service_st srv_key_status;
extern ehsm_service_st srv_get_module_status;
#ifdef CONFIG_EHSM_HW_UTC_TIME
extern ehsm_service_st srv_timer;
#endif
#ifdef CONFIG_EHSM_HW_COUNTER
extern ehsm_service_st srv_counter;
#endif

extern ehsm_service_st srv_get_challenge;
extern ehsm_service_st srv_debug_auth;
extern ehsm_service_st srv_close_debug;
extern ehsm_service_st srv_image_upgrade;
extern ehsm_service_st srv_image_verify;
#ifdef CONFIG_EHSM_SOC_UPGRADE_AND_VERIFY
extern ehsm_service_st srv_soc_image_upgrade;
#endif
extern ehsm_service_st g_srv_soc_image_verify;
extern ehsm_service_st srv_low_power;
#ifdef CONFIG_EHSM_SYS_SELF_TEST
extern ehsm_service_st srv_self_test;
#endif
extern ehsm_service_st srv_set_baudrate;
extern ehsm_service_st srv_soc_boot_status;
extern ehsm_service_st srv_read_otp_data;
extern ehsm_service_st srv_write_otp_data;
extern ehsm_service_st srv_fw_get_random_key;
extern ehsm_service_st srv_fw_encrypt_key;
extern ehsm_service_st srv_change_lifecycle;
extern ehsm_service_st srv_change_control_field;
extern ehsm_service_st srv_bootloader_cmd;

#ifdef CONFIG_EHSM_DEBUG
extern ehsm_service_st srv_get_fw_lifecycle;
extern ehsm_service_st srv_reset_firmware;
extern ehsm_service_st srv_erase_otp_data;
extern ehsm_service_st srv_erase_flash_data;
#ifdef CONFIG_EHSM_HW_UTC_TIME
extern ehsm_service_st srv_debug_get_flash_utc_time;
#endif
#endif

#endif
