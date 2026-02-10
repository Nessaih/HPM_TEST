#ifndef EHSM_MBOX_PRTCL_IP_H_
#define EHSM_MBOX_PRTCL_IP_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <eHSM_Types_Ip.h>
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Mailbox_CmdId_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define HOST_ADDRESS_SIZE     4

#define CMD_TAG_WORD_SIZE          HOST_ADDRESS_SIZE/4
#define CMD_TAG_BYTE_SIZE          HOST_ADDRESS_SIZE
#define RESPONSE_TAG_INDEX    0x10
#define MAX_KEY_DERIVED_PWD_SIZE     (256)
#define MAX_KEY_AUTH_VALUE_SIZE      EHSM_EVITA_AUTH_VALUE_MAX_SIZE
#define MAX_IMPORT_KEY_SIZE          256
#define MAX_SM9_USER_ID_SIZE         1024
#define MAX_SM9_WARP_KEY_SIZE        512
#define MAX_SM9_CIPHER_KEY_SIZE      64

/* mailbox multichannel parameters */
//Word size for all the mailbox channels SOC to HSM
#define S2H_SRV_GENERAL_WORD_SIZE               18U
#define S2H_SRV_MGR_WORD_SIZE                   3U
#define S2H_SRV_CMD_CANCLE_WORD_SIZE            2U
#define S2H_SRV_JTAG_WORD_SIZE                  1U

//Start word for all the mailbox channels SOC to HSM
#define S2H_SRV_GENERAL_WORD_INDEX              0U
#define S2H_SRV_MGR_WORD_INDEX                  25U
#define S2H_SRV_CMD_CANCLE_WORD_INDEX           28U
#define S2H_SRV_CMD_JTAG_WORD_INDEX             31U

//Notification bit for all the mailbox channels SOC to HSM
#define S2H_SRV_GENERAL_NOTE_BIT                ((ehsm_uint32_t)0x01<<0)
#define S2H_SRV_MGR_NOTE_BIT                    ((ehsm_uint32_t)0x01<<6)
#define S2H_SRV_CMD_CANCLE_NOTE_BIT             ((ehsm_uint32_t)0x01<<9)
#define S2H_SRV_CMD_JTAG_NOTE_BIT               ((ehsm_uint32_t)0x01<<21)
#define S2H_SRV_CMD_JTAG_END_BIT                ((ehsm_uint32_t)0x01<<22)

//Word size for all the mailbox channels HSM to SOC
#define H2S_SRV_GENERAL_WORD_SIZE               5U
#define H2S_SRV_MGR_WORD_SIZE                   2U
#define H2S_SRV_CMD_CANCLE_WORD_SIZE            3U
#define H2S_SRV_JTAG_WORD_SIZE                  1U

//Start word for all the mailbox channels SOC to HSM
#define H2S_SRV_GENERAL_WORD_INDEX              0U
#define H2S_SRV_MGR_WORD_INDEX                  7U
#define H2S_SRV_CMD_CANCLE_WORD_INDEX           9U
#define H2S_SRV_JTAG_WORD_INDEX                 14U

//Notification bit for all the mailbox channels SOC to HSM
#define H2S_SRV_GENERAL_NOTE_BIT                ((ehsm_uint32_t)0x01<<0)
#define H2S_SRV_MGR_NOTE_BIT                    ((ehsm_uint32_t)0x01<<6)
#define H2S_SRV_CMD_CANCLE_NOTE_BIT             ((ehsm_uint32_t)0x01<<9)
#define H2S_SRV_CMD_JTAG_NOTE_BIT               ((ehsm_uint32_t)0x01<<21)
#define H2S_SRV_CMD_JTAG_END_BIT                ((ehsm_uint32_t)0x01<<22)

//Definition of low power mode
#define  EHSM_LOW_POWER_MODE    0x1U
#define  EHSM_NORMAL_MODE       0x2U

//Definition cancel cmd type
#define EHSM_CANCEL_SINGLE_CMD                  0x1
#define EHSM_CANCEL_CERT_TYPE_CMD               0x2

//Definition cipher key type
#define EHSM_CMD_CIPHER_KEY_TYPE_SHE        0x01
#define EHSM_CMD_CIPHER_KEY_TYPE_EVITA      0x02

#define CMD_TAG_WORD_INDEX      (S2H_SRV_GENERAL_WORD_SIZE - 1)
#define MAILBOX_CMD_MAX_SIZE    (S2H_SRV_GENERAL_WORD_SIZE * 4 + 4)

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/**
 * @brief mailbox channel
 */
#define MAILBOX_CHANNE_GENERAL_SERVICE 0U
#define MAILBOX_CHANNE_CMD_CANCLE      1U
#define MAILBOX_CHANNE_JTAG            2U
#define MAILBOX_CHANNE_MGR_SERVICE     3U
#define MAILBOX_CHANNE_MAX             4U

typedef ehsm_uint8_t mailbox_channel_e;

typedef struct ehsm_otp_read_cmd{
    ehsm_uint8_t ehsm_src_addr[4];
    ehsm_uint8_t size[4];
    ehsm_uint8_t host_dst_addr[HOST_ADDRESS_SIZE];
} ehsm_otp_read_cmd_st;

typedef struct ehsm_otp_write_cmd{
    ehsm_uint8_t ehsm_dst_addr[4];
    ehsm_uint8_t size[4];
    ehsm_uint8_t host_src_addr[HOST_ADDRESS_SIZE];
} ehsm_otp_write_cmd_st;

typedef struct ehsm_get_challenge_cmd{
    /* Challenge type for debug. */
    ehsm_uint8_t type;
    /* Please don't change the reserved size */
    ehsm_uint8_t reserved1[35];
    /* Address for challenge output. */
    ehsm_uint8_t output_addr[HOST_ADDRESS_SIZE];
} ehsm_get_challenge_cmd_st;

typedef struct ehsm_debug_authentication_cmd{
    /* Challenge type for debug. */
    ehsm_uint8_t type;
    /* Authentication algprithm for debug. */
    ehsm_uint8_t algprithm;
    ehsm_uint8_t rev1[6];
    ehsm_uint8_t rev2[4];
    /* Host address of the signature. */
    ehsm_uint8_t sign_addr[HOST_ADDRESS_SIZE];
    /* The size of signature in bytes. */
    ehsm_uint8_t sign_size[4];
    /* Host address of the public key of the signer. */
    ehsm_uint8_t pub_addr[HOST_ADDRESS_SIZE];
    /* The size of the public key of the signer in bytes. */
    ehsm_uint8_t pub_size[4];
} ehsm_debug_authentication_cmd_st;


typedef struct ehsm_close_debug_cmd{
    /* Challenge type for debug. */
    ehsm_uint8_t type;
} ehsm_close_debug_cmd_st;

#ifdef CONFIG_EHSM_HW_UTC_TIME
typedef struct ehsm_timer_cmd {
    ehsm_uint32_t utc_time;
    ehsm_uint8_t rev1[4];
    ehsm_uint32_t key_handle;
    ehsm_uint8_t key_auth_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t key_auth_size;
    ehsm_uint8_t input_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t input_size;
    ehsm_uint8_t output_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t output_size;
    ehsm_uint8_t rev2[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev3[4];
    ehsm_uint8_t context_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t context_size;
} ehsm_timer_cmd_st;
#endif

#ifdef CONFIG_EHSM_HW_COUNTER
typedef struct ehsm_counter_cmd {
    ehsm_uint32_t counter_id;
    ehsm_uint8_t rev1[4];
    ehsm_uint8_t rev2[4];
    ehsm_uint8_t counter_inc[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev3[8];
    ehsm_uint8_t auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint32_t auth_size;
    ehsm_uint8_t rev4[16];
    ehsm_uint8_t context_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t context_size;
} ehsm_counter_cmd_st;
#endif

typedef struct ehsm_fw_get_random_key_cmd
{
    ehsm_uint8_t reserved1;
    ehsm_uint8_t key_type;
    ehsm_uint8_t key_slot;
} ehsm_fw_get_random_key_cmd_st;

typedef struct ehsm_fw_encrypt_key_cmd
{
    ehsm_uint8_t rev1;
    ehsm_uint8_t key_type;
    ehsm_uint8_t key_slot;
    ehsm_uint8_t rev2[5];
    /* Please don't change the reserved size */
    ehsm_uint8_t rev3[16];
    ehsm_uint8_t input_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t input_size[4];
} ehsm_fw_encrypt_key_cmd_st;

typedef struct ehsm_image_upgrade_cmd
{
    ehsm_uint8_t process_mode;
    ehsm_uint8_t rev1[7];
    ehsm_uint8_t rev_key_handle[4];
    ehsm_uint8_t rev_key_auth[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev_key_auth_size[4];
    ehsm_uint8_t image_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t image_size[4];
    ehsm_uint8_t storage_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev2[4];
    ehsm_uint8_t rev3[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev4[4];
    ehsm_uint8_t ctx_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t ctx_size[4];    
} ehsm_image_upgrade_cmd_st;

typedef struct ehsm_image_verfiy_cmd
{
    ehsm_uint8_t process_mode;
    ehsm_uint8_t type;
    ehsm_uint8_t rev1[6];
    ehsm_uint8_t rev_key_handle[4];
    ehsm_uint8_t rev_key_auth[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev_key_auth_size[4];
    ehsm_uint8_t image_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t image_size[4];
    ehsm_uint8_t rev2[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev3[4];
    ehsm_uint8_t rev4[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev5[4];
    ehsm_uint8_t ctx_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t ctx_size[4];    
} ehsm_image_verify_cmd_st;

typedef struct ehsm_soc_image_verify_cmd
{
    ehsm_uint8_t resered1[1];
    ehsm_uint8_t type;
    ehsm_uint8_t storage_alg;
    ehsm_uint8_t storage_encryption_flag;
    ehsm_uint8_t resered2[4];
    ehsm_uint8_t resered3[4];
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
}ehsm_soc_image_verify_cmd_st;

typedef struct ehsm_derive_key_cmd {
    ehsm_uint8_t key_deriv_func;
    ehsm_uint8_t reserved1[3];
    ehsm_uint8_t key_size[4];
    ehsm_uint8_t valid_until[4];
    ehsm_uint8_t type;
    ehsm_uint8_t derive_type;
    ehsm_uint8_t reserved2[2];
    ehsm_uint8_t key_usage[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_usage_size[4];
    ehsm_uint8_t parent_key_handle[4];
    ehsm_uint8_t parent_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t parent_key_auth_size[4];
    ehsm_uint8_t salt_size[4];
    ehsm_uint8_t salt_data[HOST_ADDRESS_SIZE];
    ehsm_uint8_t pw_size[4];
    ehsm_uint8_t pw_data[HOST_ADDRESS_SIZE];
    ehsm_uint8_t itera_times[4];
} ehsm_derive_key_cmd_st;

typedef struct ehsm_rng_generate_cmd {
    ehsm_uint8_t rev1[3];
    ehsm_uint8_t algorithm;
    ehsm_uint8_t rev2[4];
    ehsm_uint32_t rev_key_handle;
    ehsm_uint8_t rev_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint32_t rev_key_auth_size;
    ehsm_uint8_t rev3[HOST_ADDRESS_SIZE];
    ehsm_uint32_t rev4;
    ehsm_uint8_t random_data_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t request_size;
}ehsm_rng_generate_cmd_st;

typedef struct ehsm_import_key_cmd {
    ehsm_uint8_t key_type;
    ehsm_uint8_t rev1[7];
    ehsm_uint8_t transport_key_handle[4];
    ehsm_uint8_t transport_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t transport_key_auth_size[4];
    ehsm_uint8_t authenticity_key_handle[4];
    ehsm_uint8_t authenticity_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t authenticity_key_auth_size[4];
    ehsm_uint8_t encrypted_key[HOST_ADDRESS_SIZE];
    ehsm_uint8_t encrypted_key_size[4];
    ehsm_uint8_t key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_auth_size[4];
} ehsm_import_key_cmd_st;

typedef struct ehsm_export_key_cmd {
    ehsm_uint8_t key_handle[4];
    ehsm_uint8_t use_flags[4];
    ehsm_uint8_t transport_key_handle[4];
    ehsm_uint8_t transport_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t transport_key_auth_size[4];
    ehsm_uint8_t authenticity_key_handle[4];
    ehsm_uint8_t authenticity_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t authenticity_key_auth_size[4];
    ehsm_uint8_t encrypted_key[HOST_ADDRESS_SIZE];
    ehsm_uint8_t encrypted_key_size[4];
    ehsm_uint8_t key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_auth_size[4];
} ehsm_export_key_cmd_st;

typedef struct ehsm_create_dh_sm2_ext_param {
    ehsm_uint8_t sm2_role;
    ehsm_uint8_t reserved2[3];
    ehsm_uint8_t local_tmp_key_handle[4];
    ehsm_uint8_t local_tmp_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t local_tmp_key_auth_size[4];
    ehsm_uint8_t s1_s2_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t sa_sb_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t peer_temp_pubkey[HOST_ADDRESS_SIZE];
} ehsm_create_dh_sm2_ext_param_st;

typedef struct ehsm_create_dh_key_cmd {
    ehsm_uint8_t remote_key_handle[4];
    ehsm_uint8_t key_size[4];
    ehsm_uint8_t valid_until[4];
    ehsm_uint8_t type;
    ehsm_uint8_t parent_alg;
    ehsm_uint8_t reserved2[2];
    ehsm_uint8_t key_usage[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_usage_size[4];
    ehsm_uint8_t local_key_handle[4];
    ehsm_uint8_t local_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t local_key_auth_size[4];
    ehsm_uint8_t remote_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t remote_key_auth_size[4];
    ehsm_uint8_t algorithm;
    ehsm_uint8_t dh_mode;
    ehsm_uint8_t reserved3[2];
    ehsm_uint8_t ss_addr[HOST_ADDRESS_SIZE];
    /* Address of ehsm_create_dh_cmd_sm2_ext_para_st */
    ehsm_uint8_t sm2_ext_param[HOST_ADDRESS_SIZE];
} ehsm_create_dh_key_cmd_st;

typedef struct ehsm_get_pub_from_priv_cmd
{
    ehsm_uint8_t reserved1[8];
    ehsm_uint8_t key_handle[4];
    ehsm_uint8_t key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_auth_size[4];
    ehsm_uint32_t key_alg_id;
    ehsm_uint8_t reserved2[4];
    ehsm_uint8_t public_key_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t public_key_buffer_size[4];
} ehsm_get_pub_from_priv_cmd_st;

typedef struct ehsm_key_remove_cmd
{
    ehsm_uint8_t reserved1[8];
    ehsm_uint8_t key_handle[4];
    ehsm_uint8_t key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_auth_size[4];
} ehsm_key_remove_cmd_st;

typedef struct ehsm_key_status_cmd
{
    ehsm_uint8_t key_handle[4];
    ehsm_uint8_t reserved1[4];
    ehsm_uint8_t cert_key_handle[4];
    ehsm_uint8_t cert_key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t cert_key_auth_size[4];
    ehsm_uint8_t reserved2[8];
    ehsm_uint8_t key_status[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_status_size[4];
} ehsm_key_status_cmd_st;

typedef struct ehsm_copy_key_cmd
{
    ehsm_uint8_t reserved1[16];
    ehsm_uint8_t key_usage[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_usage_size[4];
    ehsm_uint8_t key_handle[4];
    ehsm_uint8_t key_auth_value[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_auth_size[4];
} ehsm_copy_key_cmd_st;

typedef struct ehsm_she_load_export_key_cmd
{
    ehsm_uint8_t m1[HOST_ADDRESS_SIZE];
    ehsm_uint8_t m2[HOST_ADDRESS_SIZE];
    ehsm_uint8_t m3[HOST_ADDRESS_SIZE];
    ehsm_uint8_t m4[HOST_ADDRESS_SIZE];
    ehsm_uint8_t m5[HOST_ADDRESS_SIZE];
    ehsm_bool_t she_ext_flag;
} ehsm_she_load_export_key_cmd_st;

typedef struct ehsm_she_load_plain_key_cmd
{
    ehsm_uint8_t key_data[HOST_ADDRESS_SIZE];
} ehsm_she_load_plain_key_cmd_st;

/* struct for key generation, including ehsm_sym_gen_key_cmd, 
 ehsm_sm2_gen_key_cmd, ehsm_rsa_gen_key_cmd, ehsm_eccp_gen_key_cmd */
typedef struct ehsm_gen_key_cmd
{
    /* algorithm id for ehsm_sym_gen_key_cmd, curve_id for ehsm_eccp_gen_key_cmd 
    CRT mode for ehsm_rsa_gen_key_cmd*/
    ehsm_uint8_t alg_or_crt;
    ehsm_uint8_t type;
    ehsm_uint8_t reserved1[2];
    /* e_size for ehsm_rsa_gen_key_cmd*/
    ehsm_uint8_t e_size[2];
    /* n_size for ehsm_rsa_gen_key_cmd*/
    ehsm_uint8_t n_size[2];
    ehsm_uint8_t reserved2[4];
    ehsm_uint8_t key_usage[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_usage_size[4];
    ehsm_uint8_t reserved3[4];
    ehsm_uint8_t valid_until[4];
    ehsm_uint8_t p_size[4];
    ehsm_uint8_t p[HOST_ADDRESS_SIZE];
    ehsm_uint8_t q_size[4];
    ehsm_uint8_t q[HOST_ADDRESS_SIZE];
    ehsm_uint8_t g_size[4];
    ehsm_uint8_t g[HOST_ADDRESS_SIZE];
} ehsm_gen_key_cmd_st;

typedef struct ehsm_sm9_exchg_key_cmd_child
{
    ehsm_uint8_t peer_tmp_pub[HOST_ADDRESS_SIZE];
    ehsm_uint8_t kgc_pub_key[HOST_ADDRESS_SIZE];
    ehsm_uint8_t fp12g[HOST_ADDRESS_SIZE];
    ehsm_uint8_t self_id[HOST_ADDRESS_SIZE];
    ehsm_uint8_t peer_id[HOST_ADDRESS_SIZE];
    ehsm_uint8_t self_id_size[4];
    ehsm_uint8_t peer_id_size[4];
    ehsm_uint8_t s1_s2[HOST_ADDRESS_SIZE];
    ehsm_uint8_t sa_sb[HOST_ADDRESS_SIZE];
} ehsm_sm9_exchg_key_cmd_child_st;

/* struct for SM9 key exchange */
typedef struct ehsm_sm9_exchg_key_cmd
{
    ehsm_uint8_t type;
    ehsm_uint8_t rev1[6];
    ehsm_uint8_t role;
    ehsm_uint8_t user_priv_key_handle[4];
    ehsm_uint8_t rev2[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev3[4];
    ehsm_uint8_t info_stuct[HOST_ADDRESS_SIZE];
    ehsm_uint8_t user_tmp_key_handle[4];
    ehsm_uint8_t rev4[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_size[4];
} ehsm_sm9_exchg_key_cmd_st;

/**
 * @brief Struct for command of getting challenge,
 * the command is from uart or JTAG channel of mailbox
 */
typedef struct ehsm_uart_cmd{
    void *uart_cmd_buffer;
}ehsm_uart_cmd_st;

/* struct for sm9 user private key generation, including ehsm_sm9_gen_sign_userpriv_key_cmd,
 ehsm_sm9_gen_enc_userpriv_key_cmd, ehsm_sm9_gen_exchg_userpriv_key_cmd */
typedef struct ehsm_gen_sm9_userpriv_key_cmd
{
    /* key storage type*/
    ehsm_uint8_t type;
    ehsm_uint8_t rev1[7];
    ehsm_uint8_t rev_key_handle[4];
    ehsm_uint8_t rev_key_auth[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev_key_auth_size[4];
    /* address of user ID*/
    ehsm_uint8_t id_addr[HOST_ADDRESS_SIZE];
    /* size of user ID*/
    ehsm_uint8_t id_size[4];
} ehsm_gen_sm9_userpriv_key_cmd_st;

/* struct for SM9 key wrap */
typedef struct ehsm_sm9_wrap_key_cmd
{
    ehsm_uint8_t rev1[6];
    ehsm_uint8_t hid;
    ehsm_uint8_t rev2[1];
    ehsm_uint8_t rev_key_handle[4];
    ehsm_uint8_t pub_key[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev_key_auth_size[4];
    ehsm_uint8_t id_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t id_size[4];
    ehsm_uint8_t key_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_size[4];
    ehsm_uint8_t rev3[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev4[4];
    ehsm_uint8_t fp12g[HOST_ADDRESS_SIZE];
} ehsm_sm9_wrap_key_cmd_st;

/* struct for SM9 key unwrap */
typedef struct ehsm_sm9_unwrap_key_cmd
{
    ehsm_uint8_t rev1[8];
    ehsm_uint8_t user_priv_key_handle[4];
    ehsm_uint8_t rev_key_auth[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev_key_auth_size[4];
    ehsm_uint8_t cipher_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t cipher_size[4];
    ehsm_uint8_t key_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t key_size[4];
    ehsm_uint8_t id_addr[HOST_ADDRESS_SIZE];
    ehsm_uint8_t id_size[4];
} ehsm_sm9_unwrap_key_cmd_st;

typedef struct ehsm_sm9_export_key_cmd
{
    ehsm_uint8_t rev1[8];
    ehsm_uint8_t key_handle[4];
    ehsm_uint8_t rev_key_auth[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev_key_auth_size[4];
    ehsm_uint8_t encrypted_key[HOST_ADDRESS_SIZE];
    ehsm_uint8_t encrypted_key_size[4];
    ehsm_uint8_t authenticated_key[HOST_ADDRESS_SIZE];
    ehsm_uint8_t authenticated_key_size[4];
} ehsm_sm9_export_key_cmd_st;

typedef struct ehsm_sm9_import_key_cmd
{
    ehsm_uint8_t type;
    ehsm_uint8_t key_is_plain;
    ehsm_uint8_t rev1[6];
    ehsm_uint8_t rev_key_handle[4];
    ehsm_uint8_t rev_key_auth[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev_key_auth_size[4];
    ehsm_uint8_t encrypted_key[HOST_ADDRESS_SIZE];
    ehsm_uint8_t encrypted_key_size[4];
    ehsm_uint8_t authenticated_key[HOST_ADDRESS_SIZE];
    ehsm_uint8_t authenticated_key_size[4];
} ehsm_sm9_import_key_cmd_st;

typedef struct ehsm_sm9_get_mast_pubkey_cmd
{
    ehsm_uint8_t master_key_type;
    ehsm_uint8_t reserved2[35];
    ehsm_uint8_t public_key_addr[HOST_ADDRESS_SIZE];
} ehsm_sm9_get_mast_pubkey_cmd_st;

typedef struct ehsm_sm9_get_tmp_pubkey_cmd
{
    ehsm_uint8_t reserved1[8];
    ehsm_uint8_t key_handle[4];
    ehsm_uint8_t reserved2[12];
    ehsm_uint8_t user_id[HOST_ADDRESS_SIZE];
    ehsm_uint8_t user_id_size[4];
    ehsm_uint8_t public_key_addr[HOST_ADDRESS_SIZE];
} ehsm_sm9_get_tmp_pubkey_cmd_st;

typedef struct ehsm_sm9_exchg_gen_usertmp_cmd
{
    /* key storage type*/
    ehsm_uint8_t type;
    ehsm_uint8_t rev1[7];
    ehsm_uint8_t rev_key_handle[4];
    /*The address of KGC’s master public key*/
    ehsm_uint8_t kgc_public_key[HOST_ADDRESS_SIZE];
    ehsm_uint8_t rev_key_auth_size[4];
    /* address of peer user ID*/
    ehsm_uint8_t peer_id[HOST_ADDRESS_SIZE];
    /* size of peer user ID*/
    ehsm_uint8_t peer_id_size[4];
} ehsm_sm9_exchg_gen_usertmp_cmd_st;

typedef struct ehsm_sm9_remove_key_cmd
{
    ehsm_uint8_t reserved1[8];
    ehsm_uint8_t key_handle[4];
} ehsm_sm9_remove_key_cmd_st;

#ifdef CONFIG_EHSM_DEBUG
typedef struct ehsm_erase_area_cmd{
    ehsm_uint8_t ehsm_dst_addr[4];
    ehsm_uint8_t size[4];
}ehsm_erase_area_cmd_st;
#endif

typedef struct ehsm_low_power_cmd{
    ehsm_uint8_t reserved[4];
    ehsm_uint8_t power_mode;
} ehsm_low_power_cmd_st;

typedef struct ehsm_change_lifecycle_cmd{
    ehsm_uint32_t type;
} ehsm_change_lifecycle_cmd_st;

typedef struct ehsm_change_control_field_cmd{
    ehsm_uint8_t type;
    ehsm_uint8_t reserved[1];
    ehsm_uint16_t size;
    ehsm_uint8_t value_addr[HOST_ADDRESS_SIZE];
} ehsm_change_control_field_cmd_st;

typedef struct ehsm_sensor_resp_init_cmd{
    ehsm_uint32_t data_size;
    ehsm_uint8_t data_addr[HOST_ADDRESS_SIZE];
} ehsm_sensor_resp_init_cmd_st;

typedef struct ehsm_set_baudrate_cmd{
    ehsm_uint32_t baud_div;
} ehsm_set_baudrate_cmd_st;

typedef struct ehsm_get_she_id_cmd{
    ehsm_uint8_t reserved[12];
    ehsm_uint8_t challenge_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t challenge_size;
    ehsm_uint8_t status_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t status_size;
    ehsm_uint8_t signatrue_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t signatrue_size;
}ehsm_get_she_id_cmd_st;

typedef struct ehsm_get_emu_cmd{
    ehsm_uint8_t rev1[8];
    ehsm_uint8_t rev_key_handle[4];
    ehsm_uint8_t rev_key_auth_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t rev_key_auth_size;
    ehsm_uint8_t rev2[HOST_ADDRESS_SIZE];
    ehsm_uint32_t rev3;
    ehsm_uint8_t emu_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t emu_size;
}ehsm_get_emu_cmd_st;

typedef struct ehsm_module_status_cmd{
    ehsm_uint8_t type;
    ehsm_uint8_t algo_id;
    ehsm_uint8_t reserved[6];
    ehsm_uint8_t key_handle[4];
    ehsm_uint8_t key_auth_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t key_auth_size;
    ehsm_uint8_t status_addr[HOST_ADDRESS_SIZE];
    ehsm_uint32_t status_size;
    ehsm_uint8_t signatrue[HOST_ADDRESS_SIZE];
    ehsm_uint32_t signatrue_size;
}ehsm_module_status_cmd_st;


typedef struct
{
    ehsm_uint8_t status;
} ehsm_soc_secure_boot_status_st;

typedef struct ehsm_mailbox_req {
    /** @brief General service id */
    int cmd_id;
    ehsm_uint8_t api_type;
    /** @brief Reserved */
    ehsm_uint8_t rev[7];

    union {
        ehsm_derive_key_cmd_st derive_key;
        ehsm_import_key_cmd_st import_key;
        ehsm_export_key_cmd_st export_key;
        ehsm_get_challenge_cmd_st get_challenge;
        ehsm_debug_authentication_cmd_st debug_authentication;
#ifdef CONFIG_EHSM_HW_UTC_TIME
        ehsm_timer_cmd_st timer;
#endif
#ifdef CONFIG_EHSM_HW_COUNTER
        ehsm_counter_cmd_st counter;
#endif
        ehsm_fw_get_random_key_cmd_st fw_random_key;
        ehsm_fw_encrypt_key_cmd_st fw_encrypt_key;
        ehsm_image_upgrade_cmd_st image_upgrade;
        ehsm_image_verify_cmd_st image_verify;
        ehsm_soc_image_verify_cmd_st soc_image_verify;
        ehsm_create_dh_key_cmd_st create_dh_key;
        ehsm_get_pub_from_priv_cmd_st get_pub_from_priv;
        ehsm_key_remove_cmd_st key_remove;
        ehsm_key_status_cmd_st key_status;
        ehsm_copy_key_cmd_st copy_key;
        ehsm_she_load_export_key_cmd_st she_load_export_key;
        ehsm_she_load_plain_key_cmd_st she_load_plain_key;
        ehsm_gen_key_cmd_st ehsm_gen_key;
        ehsm_gen_sm9_userpriv_key_cmd_st ehsm_gen_sm9_userpriv_key;
        ehsm_sm9_exchg_gen_usertmp_cmd_st gen_usertmp;
        ehsm_sm9_exchg_key_cmd_st sm9_exchg_key;
        ehsm_sm9_wrap_key_cmd_st sm9_wrap_key;
        ehsm_sm9_unwrap_key_cmd_st sm9_unwrap_key;
        ehsm_sm9_export_key_cmd_st sm9_export_key;
        ehsm_sm9_import_key_cmd_st sm9_import_key;
        ehsm_sm9_get_mast_pubkey_cmd_st get_mast_pubkey;
        ehsm_sm9_get_tmp_pubkey_cmd_st get_tmp_pubkey;
        ehsm_sm9_remove_key_cmd_st sm9_remove_key;

        ehsm_otp_read_cmd_st otp_read;
        ehsm_otp_write_cmd_st otp_write;
        ehsm_uart_cmd_st uart_cmd;
        ehsm_get_she_id_cmd_st get_she_id;
        ehsm_get_emu_cmd_st get_emu;
        ehsm_module_status_cmd_st module_status;
        ehsm_soc_secure_boot_status_st soc_boot;
        ehsm_close_debug_cmd_st close_debug;
#ifdef CONFIG_EHSM_DEBUG
        ehsm_erase_area_cmd_st erase_area;
#endif
    } ehsm_cmd;

} ehsm_mailbox_req_st;

typedef struct ehsm_self_test_cmd{
    ehsm_uint32_t test_type;
} ehsm_self_test_cmd_st;

/* Data struct for mailbox independent channel  */
typedef struct ehsm_mbox_mgr_channel_req {
    /** @brief General service id */
    int cmd_id;
    union {
        ehsm_low_power_cmd_st low_power_cmd;
        ehsm_self_test_cmd_st self_test_cmd;
        ehsm_set_baudrate_cmd_st set_baudrate_cmd;
        ehsm_change_lifecycle_cmd_st change_lifecycle_cmd;
        ehsm_change_control_field_cmd_st change_control_field_cmd;
        ehsm_sensor_resp_init_cmd_st sensor_resp_init_cmd;
    } ehsm_cmd;
} ehsm_mbox_mgr_channel_req_st;


/* Data struct for cancel cmd channel  */
typedef struct ehsm_mbox_cancel_channel_req {
    /* cancel type, 1: cancel s single cmd, 2: cancel a certain 
    type of cmds. such as cmds come from she apis*/
    ehsm_uint8_t cancel_type;
    /*type of apis, 1: SHE, 2: Evita, 3: Autosar*/
    ehsm_uint8_t api_type;
    ehsm_uint8_t rev[2];
    /*cmd tag ,refers to the cmd to be cancled, ignored when cancel_type is 2*/
    ehsm_uint8_t cmd_tag[HOST_ADDRESS_SIZE];
} ehsm_mbox_cancel_channel_req_st;

typedef struct ehsm_mbox_cancel_channel_rps {
    ehsm_uint8_t ret_code[4];
    /* cancel type, 1: cancel s single cmd, 2: cancel a certain 
    type of cmds. such as cmds come from she apis*/
    ehsm_uint8_t cancel_type;
    /*type of apis, 1: SHE, 2: Evita, 3: Autosar*/
    ehsm_uint8_t api_type;
    ehsm_uint8_t rev[2];
    /*cmd tag ,refers to the cmd to be cancled, ignored when cancel_type is 2*/
    ehsm_uint8_t cmd_tag[HOST_ADDRESS_SIZE];
} ehsm_mbox_cancel_channel_rps_st;

/**
 * @brief Header for Ske/Aead/Mac/Hash
 */
typedef struct
{
    // Stream mode
    ehsm_uint8_t process_mode;
    // Encryption or Decryption, MAC/HMAC generation or MAC verification
    ehsm_uint8_t direction;
    // Padding for symmetric
    ehsm_uint8_t padding;
    // Symmetric, hash, RNG algorithm
    ehsm_uint8_t algorithm;
    // Symmetric mode, etc.
    ehsm_uint8_t cipher_mode;
    // Whether time stamp is used. Only for MAC/HMAC/Sign/Verify
    ehsm_uint8_t time_stamp;
    // Tag size for AEAD.
    ehsm_uint8_t tag_size;
    // Key type for SHE or EVITA.
    ehsm_uint8_t key_type;
} ehsm_cmd_hdr_ske_st;

/**
 * @brief Header for sm2/rsa/ecdsa
 */
typedef struct
{
    // Stream mode
    ehsm_uint8_t process_mode;
    // Encryption or Decryption, Signature or Verification
    ehsm_uint8_t direction;
    // Padding for RSA
    ehsm_uint8_t padding;
    // Hash algorithm for message digest
    ehsm_uint8_t algorithm;
    ehsm_uint8_t rsa_crt_mode;
    ehsm_uint8_t time_stamp;
    ehsm_uint8_t hdr_rev1[2];
} ehsm_cmd_hdr_pke_st;

/**
 * @brief Header for ecies
 */
typedef struct
{
    ehsm_uint8_t hdr_rev1[1];
    ehsm_uint8_t direction;
    ehsm_uint8_t hdr_rev2[1];
    ehsm_uint8_t mac_k_byte;
    ehsm_uint8_t curve_id;
    ehsm_uint8_t kdf_alg;
    ehsm_uint8_t cipher_alg;
    ehsm_uint8_t mac_alg;
} ehsm_cmd_hdr_ecise_st;

/**
 * @brief Header for sm2 and eccp key generation
 */
typedef struct
{
    ehsm_uint8_t curve_id;
    ehsm_uint8_t type;
    ehsm_uint8_t hdr_rev1[6];
} ehsm_cmd_hdr_eccp_keygen_st;

/**
 * @brief Header for rsa key generation
 */
typedef struct
{
    // Generate RSA key in CRT mode, only for RSA
    ehsm_uint8_t is_crt;
    ehsm_uint8_t type;
    ehsm_uint8_t hdr_rev1[2];
    ehsm_uint8_t e_bit_size[2];
    ehsm_uint8_t n_bit_size[2];
} ehsm_cmd_hdr_rsa_keygen_st;

/**
 * @brief Header for SM9 ciphert and signature
 */
typedef struct
{
    ehsm_uint8_t enc_type;
    ehsm_uint8_t direction;
    ehsm_uint8_t padding;
    ehsm_uint8_t key2_size;
    ehsm_uint8_t hdr_rev1[2];
    ehsm_uint8_t hid;
    ehsm_uint8_t hdr_rev2[1];
} ehsm_cmd_hdr_sm9_st;

/**
 * @brief SM9 signature and verification structure. It's the content of ehsm_cmd_cipher_st.output_addr.
 */
typedef struct
{
    ehsm_uint8_t h[32];
    ehsm_uint8_t Sig[65];
} ehsm_cmd_sm9_sig_vry_output_ptr_st;

/**
 * @brief GCM/CCM data structure. It's the content of ehsm_cmd_cipher_st.input_addr/output_addr
 */
typedef struct
{
    ehsm_addr_t aad_ptr;
    ehsm_addr_t data_ptr;
    ehsm_addr_t tag_ptr;
} ehsm_cmd_aead_ptr_st;

/**
 * @brief Header for RNG
 */
typedef struct
{
    ehsm_uint8_t hdr_rev1[3];
    ehsm_uint8_t algorithm;
    ehsm_uint8_t hdr_rev2[4];
} ehsm_cmd_hdr_rng_st;

#pragma pack (4)
typedef struct
{
    /* Command id */

    ehsm_uint32_t cmd_id;

    /* Reserved */

    ehsm_uint32_t rev1[2];

    /* Header */

    union osr_cmd_hdr_u
    {
        ehsm_cmd_hdr_ske_st hdr_ske;
        ehsm_cmd_hdr_pke_st hdr_pke;
        ehsm_cmd_hdr_ecise_st hdr_ecise;
        ehsm_cmd_hdr_sm9_st hdr_sm9;
        ehsm_cmd_hdr_eccp_keygen_st hdr_eccp_keygen;
        ehsm_cmd_hdr_rsa_keygen_st hdr_rsa_keygen;
        ehsm_cmd_hdr_rng_st hdr_rng;
    } u_hdr;

    /* Key info*/

    ehsm_uint32_t key_handle;
    ehsm_addr_t key_addr;
    ehsm_uint32_t key_size;

    /* Input address and input size
     *
     * For Init in stream mode, the input_addr is always null,
     *   input_size can be used for total_message_length.
     */

    ehsm_addr_t input_addr;
    ehsm_uint32_t input_size;

    /* Output
     *
     * For key generation in eHSM, output_size can be used for valid_until
     */

    ehsm_addr_t output_addr;
    ehsm_uint32_t output_size;

    /* Second Input
     * For symmetric operations, it's the iv.
     * For key exchange, it can be the remote public key.
     */

    ehsm_addr_t sec_input_addr;
    ehsm_uint32_t sec_input_size;

    /* Input and output for context */

    ehsm_addr_t context_addr;
    ehsm_uint32_t context_size;
} ehsm_cmd_cipher_st;
#pragma pack ()

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
