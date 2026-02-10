/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#include <string.h>

#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Com_Struct_Ip.h"
#include "eHSM_Srv_CmdReq_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_If_Ext_Types_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define DEFAULT_RSA_E_SIZE    17

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static ehsm_uint32_t create_dh_key_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t create_dh_key_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t create_random_key_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t create_random_key_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t key_status_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t key_status_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t she_load_key_reqhdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t she_load_plain_key_reqhdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t she_ram_key_export_reqhdl(void *para, ehsm_cmd_req_st *req);

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static ehsm_uint32_t general_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_create_dh_key = {
    .service_id = EHSM_SRV_KEYMGR_KEYEXCHANGECALCSECRET,
    .reqhdl = create_dh_key_reqhdl,
    .rsphdl = create_dh_key_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t create_dh_key_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_create_dh_key_param_st *create_dh_key_para = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;
    ehsm_create_dh_sm2_ext_param_st *sm2_ext_para = NULL;
    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        create_dh_key_para = (ehsm_create_dh_key_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_CREATE_DH_KEY;

        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.create_dh_key.remote_key_handle =
            (ehsm_uint32_t)create_dh_key_para->remote_key_handle;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.create_dh_key.key_size =
            (ehsm_uint32_t)create_dh_key_para->key_size;
#ifdef CONFIG_EHSM_HW_UTC_TIME
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.create_dh_key.valid_until =
            (ehsm_uint32_t)create_dh_key_para->valid_until;
#endif
        mailbox_packet->ehsm_cmd.create_dh_key.type = create_dh_key_para->type;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.create_dh_key.key_usage, (ehsm_uint8_t *)create_dh_key_para->key_element_data);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.create_dh_key.key_usage_size =
            (ehsm_uint32_t)create_dh_key_para->key_element_size;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.create_dh_key.local_key_handle =
            create_dh_key_para->local_key_handle;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.create_dh_key.local_key_auth_value, 
            create_dh_key_para->local_key_auth_value);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.create_dh_key.local_key_auth_size =
            create_dh_key_para->local_key_auth_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.create_dh_key.remote_key_auth_value, 
            create_dh_key_para->remote_key_auth_value_or_pub_key);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.create_dh_key.remote_key_auth_size =
            create_dh_key_para->remote_key_auth_or_pub_key_size;
        mailbox_packet->ehsm_cmd.create_dh_key.algorithm =
            (ehsm_uint8_t)create_dh_key_para->target_algorithm_identifier;
        mailbox_packet->ehsm_cmd.create_dh_key.dh_mode = create_dh_key_para->dh_mode;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.create_dh_key.ss_addr, create_dh_key_para->ss_addr);
        mailbox_packet->ehsm_cmd.create_dh_key.parent_alg = create_dh_key_para->parent_alg;
        if (EHSM_ALG_SM2 == create_dh_key_para->parent_alg)
        {
            sm2_ext_para = (ehsm_create_dh_sm2_ext_param_st *)(req->rps_data);
            *(ehsm_uint32_t *)sm2_ext_para->local_tmp_key_handle = (ehsm_uint32_t)create_dh_key_para->sm2_ext_para.local_tmp_key_handle;
            ehsm_set_address_pointer(sm2_ext_para->local_tmp_key_auth_value, create_dh_key_para->sm2_ext_para.local_tmp_key_auth_value);
            *(ehsm_uint32_t *)sm2_ext_para->local_tmp_key_auth_size = (ehsm_uint32_t)create_dh_key_para->sm2_ext_para.local_tmp_key_auth_size;
            ehsm_set_address_pointer(sm2_ext_para->s1_s2_value, create_dh_key_para->sm2_ext_para.s1_s2_value);
            ehsm_set_address_pointer(sm2_ext_para->sa_sb_value, create_dh_key_para->sm2_ext_para.sa_sb_value);
            ehsm_set_address_pointer(sm2_ext_para->peer_temp_pubkey, create_dh_key_para->sm2_ext_para.peer_temp_pubkey);
            ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.create_dh_key.sm2_ext_param, (ehsm_uint8_t *)sm2_ext_para);
            sm2_ext_para->sm2_role = create_dh_key_para->sm2_ext_para.sm2_role;
        }
        else
        {;}
    }
    return ret;
}

static ehsm_uint32_t create_dh_key_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_create_dh_key_param_st *create_dh_key_para = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            create_dh_key_para = (ehsm_create_dh_key_param_st *)para;
            create_dh_key_para->key_handle = *(ehsm_uint32_t *)req->rps_data;
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;

}

ehsm_service_st srv_create_random_key = {
    .service_id = EHSM_SRV_KEYMGR_KEYGENERATE,
    .reqhdl = create_random_key_reqhdl,
    .rsphdl = create_random_key_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_RSA_K_CMD_TIMEOUT
};

static void init_rsa_param(ehsm_mailbox_req_st *mailbox_packet, ehsm_int32_t n_size, ehsm_rsa_key_type_e key_type)
{
    *(ehsm_uint16_t *)mailbox_packet->ehsm_cmd.ehsm_gen_key.e_size = DEFAULT_RSA_E_SIZE;
    *(ehsm_uint16_t *)mailbox_packet->ehsm_cmd.ehsm_gen_key.n_size = n_size;
    mailbox_packet->ehsm_cmd.ehsm_gen_key.alg_or_crt = key_type;
    mailbox_packet->cmd_id = EHSM_CMD_RSA_GEN_KEY;
}

static ehsm_uint32_t create_random_key_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_create_random_key_param_st *random_key_para = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        random_key_para = (ehsm_create_random_key_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
#ifdef CONFIG_EHSM_HW_UTC_TIME
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.ehsm_gen_key.valid_until = random_key_para->valid_until;
#endif
        mailbox_packet->ehsm_cmd.ehsm_gen_key.type = random_key_para->type;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.ehsm_gen_key.key_usage_size = random_key_para->key_element_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.ehsm_gen_key.key_usage, (ehsm_uint8_t *)random_key_para->key_element_data);

        switch (random_key_para->target_algorithm_identifier)
        {
            case EHSM_ALG_DES:
            case EHSM_ALG_TDES_128:
            case EHSM_ALG_TDES_192:
            case EHSM_ALG_AES_128:
            case EHSM_ALG_AES_192:
            case EHSM_ALG_AES_256:
            case EHSM_ALG_SM4:
                mailbox_packet->ehsm_cmd.ehsm_gen_key.alg_or_crt = random_key_para->target_algorithm_identifier;
                mailbox_packet->cmd_id = EHSM_CMD_SYM_GEN_KEY;
                break;
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_1024
            case EHSM_ALG_RSA_1024:
                init_rsa_param(mailbox_packet, 1024, EHSM_RSA_KEY_TYPE_COMMON);
                break;
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_2048
            case EHSM_ALG_RSA_2048:
                init_rsa_param(mailbox_packet, 2048, EHSM_RSA_KEY_TYPE_COMMON);
                break;
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_1024_CRT
            case EHSM_ALG_RSA_1024_CRT:
                init_rsa_param(mailbox_packet, 1024, EHSM_RSA_KEY_TYPE_CRT);
                break;
#endif
#ifdef CONFIG_EHSM_CRYPTO_ALGOFAM_RSA_2048_CRT
            case EHSM_ALG_RSA_2048_CRT:
                init_rsa_param(mailbox_packet, 2048, EHSM_RSA_KEY_TYPE_CRT);
                break;
#endif

            case EHSM_ALG_BRAINPOOLP160R1:
            case EHSM_ALG_BRAINPOOLP192R1:
            case EHSM_ALG_BRAINPOOLP224R1:
            case EHSM_ALG_BRAINPOOLP256R1:
            case EHSM_ALG_BRAINPOOLP320R1:
            case EHSM_ALG_BRAINPOOLP384R1:
            case EHSM_ALG_BRAINPOOLP512R1:
            case EHSM_ALG_SECP192R1:
            case EHSM_ALG_SECP224R1:
            case EHSM_ALG_SECP256R1:
            case EHSM_ALG_SECP384R1:
            case EHSM_ALG_SECP521R1:
                mailbox_packet->ehsm_cmd.ehsm_gen_key.alg_or_crt = random_key_para->target_algorithm_identifier;
                mailbox_packet->cmd_id = EHSM_CMD_ECCP_GEN_KEY;
                break;

            case EHSM_ALG_ED25519:
                mailbox_packet->ehsm_cmd.ehsm_gen_key.alg_or_crt = random_key_para->target_algorithm_identifier;
                mailbox_packet->cmd_id = EHSM_CMD_ECCP_GEN_KEY;
                break;

            case EHSM_ALG_SM2:
                mailbox_packet->cmd_id = EHSM_CMD_SM2_GEN_KEY;
                break;
            case EHSM_ALG_DH:
                *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.ehsm_gen_key.p_size = random_key_para->p_size;
                ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.ehsm_gen_key.p, (ehsm_uint8_t *)random_key_para->p);
                *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.ehsm_gen_key.q_size = random_key_para->q_size;
                ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.ehsm_gen_key.q, (ehsm_uint8_t *)random_key_para->q);
                *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.ehsm_gen_key.g_size = random_key_para->g_size;
                ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.ehsm_gen_key.g, (ehsm_uint8_t *)random_key_para->g);
                mailbox_packet->cmd_id = EHSM_CMD_GEN_DH_KEY_PAIR;
                mailbox_packet->ehsm_cmd.ehsm_gen_key.alg_or_crt = random_key_para->target_algorithm_identifier;
                break;
            default:
                ret = EHSM_ERR_PARAM_ERROR;
                break;
        }
    }
    return ret;
}

static ehsm_uint32_t create_random_key_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_create_random_key_param_st *random_key_para = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            random_key_para = (ehsm_create_random_key_param_st *)para;
            (void)System_Memcpy(&random_key_para->key_handle, req->rps_data, sizeof(ehsm_int32_t));
            /* random_key_para->key_handle = *(ehsm_uint32_t *)req->rps_data; */
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;
}


static ehsm_uint32_t derive_key_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t derive_key_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_derive_key = {
    .service_id = EHSM_SRV_KEYMGR_CREATE_DERIVED_KEY,
    .reqhdl = derive_key_reqhdl,
    .rsphdl = derive_key_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t derive_key_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_key_derived_param_st *derive_key_para = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        derive_key_para = (ehsm_key_derived_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_DERIVE_KEY;
        mailbox_packet->ehsm_cmd.derive_key.key_deriv_func =
            (ehsm_uint8_t)derive_key_para->key_deriv_func;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.derive_key.key_size = derive_key_para->key_size;
#ifdef CONFIG_EHSM_HW_UTC_TIME
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.derive_key.valid_until = derive_key_para->valid_until;
#endif
        mailbox_packet->ehsm_cmd.derive_key.type = derive_key_para->type;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.derive_key.key_usage_size = derive_key_para->key_element_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.derive_key.key_usage, (ehsm_uint8_t *)derive_key_para->key_element_data);

        mailbox_packet->ehsm_cmd.derive_key.derive_type = derive_key_para->derive_type;
        if (mailbox_packet->ehsm_cmd.derive_key.derive_type == CRYPTO_KEY_DERIVE_USER_PASSWD)
        {
            *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.derive_key.pw_size = derive_key_para->passwd_size;
            ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.derive_key.pw_data, derive_key_para->passwd);
        }
        else if (mailbox_packet->ehsm_cmd.derive_key.derive_type == CRYPTO_KEY_DERIVE_USER_KEYHANDLE)
        {
            ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.derive_key.pw_data, NULL);
            ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.derive_key.pw_size, NULL);
            *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.derive_key.parent_key_handle =
                derive_key_para->parent_key_handle;
            ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.derive_key.parent_key_auth_value,
                derive_key_para->parent_key_author_value);
            *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.derive_key.parent_key_auth_size =
                derive_key_para->parent_key_author_size;
        }
        else
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.derive_key.salt_size =
            derive_key_para->salt_size;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.derive_key.itera_times = derive_key_para->itera_times;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.derive_key.salt_data, derive_key_para->salt_data);
    }
    return ret;
}

static ehsm_uint32_t derive_key_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_key_derived_param_st *derive_key_para = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            derive_key_para = (ehsm_key_derived_param_st *)para;
            derive_key_para->key_handle = *(ehsm_uint32_t *)req->rps_data;
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;
}

static ehsm_uint32_t export_key_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t export_key_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_export_key = {
    .service_id = EHSM_SRV_KEYMGR_KEY_EXPORT,
    .reqhdl = export_key_reqhdl,
    .rsphdl = export_key_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t export_key_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_evita_key_export_st *export_key_para = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        export_key_para = (ehsm_evita_key_export_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_EXPORT_KEY;

        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.export_key.key_handle =
            (ehsm_uint32_t)export_key_para->key_handle;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.export_key.use_flags =
            (ehsm_uint32_t)export_key_para->use_flags;

        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.export_key.transport_key_handle =
            export_key_para->transport_key_handle;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.export_key.transport_key_auth_value, 
            export_key_para->transport_key_author_value);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.export_key.transport_key_auth_size =
            export_key_para->transport_key_author_size;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.export_key.authenticity_key_handle =
            export_key_para->authenticity_key_handle;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.export_key.authenticity_key_auth_value, 
            export_key_para->authenticity_key_author_value);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.export_key.authenticity_key_auth_size =
            export_key_para->authenticity_key_author_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.export_key.encrypted_key, export_key_para->encrypted_key);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.export_key.encrypted_key_size = export_key_para->encrypted_key_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.export_key.key_auth_value, export_key_para->key_auth_code);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.export_key.key_auth_size = export_key_para->key_auth_code_size;
    }
    return ret;
}

static ehsm_uint32_t export_key_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_evita_key_export_st *export_key_para = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            export_key_para = (ehsm_evita_key_export_st *)para;
            export_key_para->encrypted_key_size = *(ehsm_uint32_t *)&req->rps_data[0];
            export_key_para->key_auth_code_size = *(ehsm_uint32_t *)&req->rps_data[4];
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;
}

static ehsm_uint32_t get_pub_from_priv_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t get_pub_from_priv_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_get_pub_from_priv = {
    .service_id = EHSM_SRV_KEYMGR_KEYEXCHANGECALCPUBVAL,
    .reqhdl = get_pub_from_priv_reqhdl,
    .rsphdl = get_pub_from_priv_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t get_pub_from_priv_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_get_pub_from_priv_param_st *get_pub_from_priv = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        get_pub_from_priv = (ehsm_get_pub_from_priv_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_GET_PUB_FROM_PRIV;

        mailbox_packet->ehsm_cmd.get_pub_from_priv.key_alg_id =
            get_pub_from_priv->key_alg_id;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.get_pub_from_priv.key_handle =
            (ehsm_uint32_t)get_pub_from_priv->key_handle;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.get_pub_from_priv.key_auth_value, 
            get_pub_from_priv->key_auth_value);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.get_pub_from_priv.key_auth_size =
            get_pub_from_priv->key_auth_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.get_pub_from_priv.public_key_addr, 
            get_pub_from_priv->public_key_addr);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.get_pub_from_priv.public_key_buffer_size =
            get_pub_from_priv->public_key_buffer_size;
    }
    return ret;
}

static ehsm_uint32_t get_pub_from_priv_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_get_pub_from_priv_param_st *get_pub_from_priv = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            get_pub_from_priv = (ehsm_get_pub_from_priv_param_st *)para;
            get_pub_from_priv->public_key_size = *(ehsm_uint32_t *)req->rps_data;
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;
}

static ehsm_uint32_t import_key_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t import_key_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_import_key = {
    .service_id = EHSM_SRV_KEYMGR_KEY_IMPORT,
    .reqhdl = import_key_reqhdl,
    .rsphdl = import_key_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t import_key_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_evita_key_import_st *import_key_para = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        import_key_para = (ehsm_evita_key_import_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_IMPORT_KEY;
        mailbox_packet->ehsm_cmd.import_key.key_type = 
            (ehsm_uint8_t)import_key_para->type;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.import_key.transport_key_handle =
            import_key_para->transport_key_handle;

        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.import_key.transport_key_auth_value, 
            import_key_para->transport_key_author_value);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.import_key.transport_key_auth_size =
            import_key_para->transport_key_author_size;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.import_key.authenticity_key_handle =
            import_key_para->authenticity_key_handle;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.import_key.authenticity_key_auth_value, 
            import_key_para->authenticity_key_author_value);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.import_key.authenticity_key_auth_size =
            import_key_para->authenticity_key_author_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.import_key.encrypted_key, import_key_para->encrypted_key);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.import_key.encrypted_key_size =
            import_key_para->encrypted_key_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.import_key.key_auth_value, import_key_para->key_auth_code);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.import_key.key_auth_size = import_key_para->key_auth_code_size;

    }
    return ret;
}

static ehsm_uint32_t import_key_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_evita_key_import_st *import_key_para = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            import_key_para = (ehsm_evita_key_import_st *)para;
            import_key_para->key_handle = *(ehsm_uint32_t *)req->rps_data;
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;
}

static ehsm_uint32_t key_remove_reqhdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_key_remove = {
    .service_id = EHSM_SRV_KEYMGR_KEY_REMOVE,
    .reqhdl = key_remove_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t key_remove_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_key_remove_param_st *key_remove = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        key_remove = (ehsm_key_remove_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_KEY_REMOVE;

        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.key_remove.key_handle = (ehsm_uint32_t)key_remove->key_handle;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.key_remove.key_auth_value, key_remove->key_auth_value);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.key_remove.key_auth_size = key_remove->key_auth_size;
    }
    return ret;
}

static ehsm_uint32_t key_copy_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t key_copy_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_key_copy = {
    .service_id = EHSM_SRV_KEYMGR_COPY_KEY,
    .reqhdl = key_copy_reqhdl,
    .rsphdl = key_copy_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t key_copy_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_key_copy_param_st *key_copy_param = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        key_copy_param = (ehsm_key_copy_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_COPY_EVITA_KEY;

        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.copy_key.key_handle = (ehsm_uint32_t)key_copy_param->parent_key_handle;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.copy_key.key_auth_value, key_copy_param->key_auth_value);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.copy_key.key_auth_size = key_copy_param->key_auth_size;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.copy_key.key_usage_size = key_copy_param->key_element_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.copy_key.key_usage, (ehsm_uint8_t *)key_copy_param->key_element_data);
    }
    return ret;
}

static ehsm_uint32_t key_copy_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_key_copy_param_st *key_copy_param = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        key_copy_param = (ehsm_key_copy_param_st *)para;
        key_copy_param->target_key_handle = *(ehsm_uint32_t *)req->rps_data;
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_key_status = {
    .service_id = EHSM_SRV_KEYMGR_KEY_STATUS,
    .reqhdl = key_status_reqhdl,
    .rsphdl = key_status_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t key_status_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_key_status_param_st *key_status = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        key_status = (ehsm_key_status_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_KEY_STATUS;

        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.key_status.key_handle =
            (ehsm_uint32_t)key_status->key_handle;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.key_status.cert_key_handle =
            (ehsm_uint32_t)key_status->certification_key_handle;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.key_status.cert_key_auth_value, 
            key_status->certification_key_auth_value);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.key_status.cert_key_auth_size =
            key_status->certification_key_auth_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.key_status.key_status, key_status->key_status);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.key_status.key_status_size = key_status->key_status_buffer_size;
    }
    return ret;
}

static ehsm_uint32_t key_status_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_key_status_param_st *key_status = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            key_status = (ehsm_key_status_param_st *)para;
            key_status->key_status_size = *(ehsm_uint32_t *)req->rps_data;
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_she_load_key = {
    .service_id = EHSM_SRV_KEYMGR_LOAD_KEY,
    .reqhdl = she_load_key_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t she_load_key_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_she_key_host_param_st *she_key = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        she_key = (ehsm_she_key_host_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_SHE_LOAD_KEY;

        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m1, she_key->m1);
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m2, she_key->m2);
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m3, she_key->m3);
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m4, she_key->m4);
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m5, she_key->m5);
        mailbox_packet->ehsm_cmd.she_load_export_key.she_ext_flag = she_key->she_ext_flag;
    }
    return ret;
}

ehsm_service_st srv_she_load_plain_key = {
    .service_id = EHSM_SRV_KEYMGR_LOAD_PLAIN_KEY,
    .reqhdl = she_load_plain_key_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t she_load_plain_key_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_SHE_LOAD_PLAIN_KEY;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_plain_key.key_data, (ehsm_uint8_t *)para);
    }
    return ret;
}

ehsm_service_st srv_she_ram_key_export = {
    .service_id = EHSM_SRV_KEYMGR_EXPORT_RAM_KEY,
    .reqhdl = she_ram_key_export_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t she_ram_key_export_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    ehsm_she_key_host_param_st *she_key = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        she_key = (ehsm_she_key_host_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_SHE_RAM_KEY_EXPORT;

        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m1, she_key->m1);
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m2, she_key->m2);
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m3, she_key->m3);
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m4, she_key->m4);
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.she_load_export_key.m5, she_key->m5);
        mailbox_packet->ehsm_cmd.she_load_export_key.she_ext_flag = she_key->she_ext_flag;
    }
    return ret;
}


static ehsm_uint32_t certificate_parse_reqhdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_certificate_parse = {
    .service_id = EHSM_SRV_KEYMGR_CERRITIFATEPARSE,
    .reqhdl = certificate_parse_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t certificate_parse_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    const ehsm_cmd_cipher_st *cmd = NULL;
    ehsm_cmd_cipher_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        cmd = (const ehsm_cmd_cipher_st *)para;
        mailbox_packet = (ehsm_cmd_cipher_st *)req->cmd_data;
        mailbox_packet->cmd_id = cmd->cmd_id;
        mailbox_packet->input_addr = cmd->input_addr;
        mailbox_packet->input_size = cmd->input_size;
        mailbox_packet->output_addr = cmd->output_addr;
        mailbox_packet->output_size = cmd->output_size;
    }

    return ret;
}

static ehsm_uint32_t certificate_verify_reqhdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_certificate_verify = {
    .service_id = EHSM_SRV_KEYMGR_CERRITIFATEVERIFY,
    .reqhdl = certificate_verify_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t certificate_verify_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret;
    ehsm_cmd_cipher_st *cmd = NULL;
    ehsm_cmd_cipher_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        cmd = (ehsm_cmd_cipher_st *)para;
        mailbox_packet = (ehsm_cmd_cipher_st *)req->cmd_data;
        mailbox_packet->cmd_id = cmd->cmd_id;
        mailbox_packet->input_addr = cmd->input_addr;
        mailbox_packet->input_size = cmd->input_size;
        mailbox_packet->output_addr = cmd->output_addr;
        mailbox_packet->output_size = cmd->output_size;
        mailbox_packet->sec_input_addr = cmd->sec_input_addr;
        mailbox_packet->sec_input_size = cmd->sec_input_size;
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
