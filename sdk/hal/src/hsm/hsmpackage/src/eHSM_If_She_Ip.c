/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#include <string.h>

#include "eHSM_If_She_Ip.h"
#include "eHSM_If_She_Types_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_If_She_Types_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Mailbox_CmdId_Ip.h"
#include "eHSM_If_Ext_Ip.h"
#include "eHSM_Com_Struct_Ip.h"
#include "eHSM_Srv_Cipher_Ip.h"
#include "eHSM_If_She_ErrCode_Ip.h"
#include "eHSM_Dspt_lp.h"
#include "eHSM_If_She_Ip.h"
#include "eHSM_Exclusive_Area.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

#define EHSM_SOC_BOOT_STATUS_OK             (0x01U)
#define EHSM_SOC_BOOT_STATUS_FAIL           (0x02U)
#define CONFIG_EHSM_KMGR_V_SHE_K_MIN        (0x04U)
#define CONFIG_EHSM_KMGR_V_SHE_BOOT_MAC_K   (0x02U)

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
static ehsm_bool_t excuting_she_cmd = FALSE;

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static ehsm_bool_t can_exec_she_cmd(void)
{
    ehsm_bool_t ret = FALSE;

    Exclusive_area_enter();
    if (excuting_she_cmd == TRUE)
    {
    }
    else
    {
        excuting_she_cmd = TRUE;
        ret = TRUE;
    }
    Exclusive_area_exit();
    return ret;
}

static void clear_exec_she_cmd_flag(void)
{
    Exclusive_area_enter();
    excuting_she_cmd = FALSE;
    Exclusive_area_exit();
}

static ehsm_uint32_t she_load_key_local(const ehsm_uint8_t *m1, const ehsm_uint8_t *m2, const ehsm_uint8_t *m3,
        ehsm_uint8_t *m4, ehsm_uint8_t *m5, ehsm_bool_t extend_she_key)
{
    ehsm_she_key_host_param_st key_param[1];

    ehsm_uint32_t ret = 0U;

    if ((m1 == NULL) || (m2 == NULL) || (m3 == NULL) || (m4 == NULL) || (m5 == NULL))
    {
        ret = ERC_GENERAL_ERROR;
    }
    else
    {
        key_param->m1 = (ehsm_uint8_t*)m1;
        key_param->m2 = (ehsm_uint8_t*)m2;
        key_param->m3 = (ehsm_uint8_t*)m3;
        key_param->m4 = m4;
        key_param->m5 = m5;
        key_param->she_ext_flag = extend_she_key;
        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_LOAD_KEY, key_param, EHSM_API_TYPE_SHE);
        ret = ehsm_she_convert_ret_code(ret);
    }

    return ret;
}
static ehsm_uint32_t do_set_she_boot_status(ehsm_uint8_t status)
{
    ehsm_int32_t ret;

    ret = ehsm_process_sync_service(EHSM_SRV_SOC_BOOT_STATUS, &status, EHSM_API_TYPE_SHE);
    ret = ehsm_she_convert_ret_code(ret);

    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
ehsm_uint32_t she_crypto_ecb_extend(ehsm_uint32_t key_id, const ehsm_uint8_t *in, ehsm_uint8_t *out,
    ehsm_uint32_t size, ehsm_uint32_t direction)
{
    ehsm_uint32_t ret = EHSM_ERR_GENERAL_ERROR;
    ehsm_cmd_cipher_st aes_para = {0};
    ehsm_cmd_cipher_with_rps_st aes_packet;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        if ((key_id > CONFIG_EHSM_KMGR_V_SHE_K_NUM) || (key_id < CONFIG_EHSM_KMGR_V_SHE_K_MIN))
        {
            ret = ERC_KEY_INVALID;
        }
        else if ((NULL != in) && (NULL != out) && (0 == size % 16) && (size > 0) && (direction <= EHSM_DECRYPTION))
        {
            aes_para.cmd_id = EHSM_CMD_SYM_CIPHER;
            aes_para.u_hdr.hdr_ske.process_mode = EHSM_ONEPASS;
            aes_para.u_hdr.hdr_ske.direction = direction;
            aes_para.u_hdr.hdr_ske.padding = EHSM_NOPADDING;
            aes_para.u_hdr.hdr_ske.algorithm = EHSM_AES_128;
            aes_para.u_hdr.hdr_ske.cipher_mode = EHSM_ECB_MODE;
            aes_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_SHE;
            aes_para.key_handle = key_id;
            aes_para.input_addr = (ehsm_addr_t)in;
            aes_para.input_size = size; /* Default 128 bits 16 Byte in SHE */
            aes_para.output_addr = (ehsm_addr_t)out;
            aes_packet.req_cipher = aes_para;
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &aes_packet, EHSM_API_TYPE_SHE);
            ret = ehsm_she_convert_ret_code(ret);
        }
        else
        {;} /* Do nothing, because SHE does not contain an error code for INVALID_PARAMETER */
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t she_crypto_ecb(ehsm_uint32_t key_id, const ehsm_uint8_t *in, ehsm_uint8_t *out, ehsm_uint32_t direction)
{
    ehsm_uint32_t ret = EHSM_ERR_GENERAL_ERROR;
    ehsm_uint32_t size = 16U;

    ret = she_crypto_ecb_extend(key_id, in, out, size, direction);
    return ret;
}

ehsm_uint32_t she_crypto_cbc(ehsm_uint32_t key_id, const ehsm_uint8_t *iv, const ehsm_uint8_t *in, ehsm_uint8_t *out,
        ehsm_uint32_t size, ehsm_uint32_t direction)
{
    ehsm_uint32_t ret = EHSM_ERR_GENERAL_ERROR;
    ehsm_cmd_cipher_st aes_para = {0};
    ehsm_cmd_cipher_with_rps_st aes_packet;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        if ((key_id > CONFIG_EHSM_KMGR_V_SHE_K_NUM) || (key_id < CONFIG_EHSM_KMGR_V_SHE_K_MIN))
        {
            ret = ERC_KEY_INVALID;
        }
        else if ((NULL != iv) && (NULL != in) && (NULL != out) && (0 == size % 16) && (size > 0) &&
                (direction <= EHSM_DECRYPTION))
        {
            aes_para.cmd_id = EHSM_CMD_SYM_CIPHER;
            aes_para.u_hdr.hdr_ske.process_mode = EHSM_ONEPASS;
            aes_para.u_hdr.hdr_ske.direction = direction;
            aes_para.u_hdr.hdr_ske.padding = EHSM_NOPADDING;
            aes_para.u_hdr.hdr_ske.algorithm = EHSM_AES_128;
            aes_para.u_hdr.hdr_ske.cipher_mode = EHSM_CBC_MODE;
            aes_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_SHE;
            aes_para.key_handle = key_id;
            aes_para.input_addr = (ehsm_addr_t)in;
            aes_para.input_size = size;
            aes_para.output_addr = (ehsm_addr_t)out;
            aes_para.sec_input_addr = (ehsm_addr_t)iv;
            aes_para.sec_input_size = 16U;
            aes_packet.req_cipher = aes_para;
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &aes_packet, EHSM_API_TYPE_SHE);
            ret = ehsm_she_convert_ret_code(ret);

        }
        else
        {;} /* Do nothing, because SHE does not contain an error code for INVALID_PARAMETER */
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t she_generate_mac(ehsm_uint32_t key_id, const ehsm_uint8_t *msg, ehsm_uint32_t size, ehsm_uint8_t *mac)
{
    ehsm_uint32_t ret = ERC_GENERAL_ERROR;
    ehsm_cmd_cipher_st mac_para = {0};

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        if ((key_id > CONFIG_EHSM_KMGR_V_SHE_K_NUM) || (key_id < CONFIG_EHSM_KMGR_V_SHE_K_MIN))
        {
            ret = ERC_KEY_INVALID;
        }
        else if ((NULL != msg) && (NULL != mac))
        {
            mac_para.cmd_id = EHSM_CMD_MAC;
            mac_para.u_hdr.hdr_ske.process_mode = EHSM_ONEPASS;
            mac_para.u_hdr.hdr_ske.direction = EHSM_MAC_GENERATION;
            mac_para.u_hdr.hdr_ske.padding = EHSM_NOPADDING;
            mac_para.u_hdr.hdr_ske.algorithm = EHSM_AES_128;
            mac_para.u_hdr.hdr_ske.cipher_mode = EHSM_CMAC_MODE;
            mac_para.u_hdr.hdr_ske.time_stamp = EHSM_NO_TIME_STAMP;
            mac_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_SHE;
            mac_para.key_handle = key_id;
            mac_para.input_addr = (ehsm_addr_t)msg;
            /* size could be any value, including 0 */
            mac_para.input_size = size;
            mac_para.output_addr = (ehsm_addr_t)mac;
            mac_para.output_size = 16U; /* Default 128 bits in SHE */
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &mac_para, EHSM_API_TYPE_SHE);

            ret = ehsm_she_convert_ret_code(ret);
        }
        else
        {;} /* return ERC_GENERAL_ERROR */
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t she_verify_mac(ehsm_uint32_t key_id, const ehsm_uint8_t *msg, ehsm_uint32_t size, const ehsm_uint8_t *mac,
        ehsm_uint32_t mac_size, ehsm_uint32_t *vrf_status)
{
    ehsm_uint32_t ret = ERC_GENERAL_ERROR;
    ehsm_cmd_cipher_st mac_para = {0};

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        /* Boot_MAC_KEY could be used in CMD_VERIFY_MAC */
        if ((key_id != CONFIG_EHSM_KMGR_V_SHE_BOOT_MAC_K) && ((key_id > CONFIG_EHSM_KMGR_V_SHE_K_NUM) || (key_id < CONFIG_EHSM_KMGR_V_SHE_K_MIN)))
        {
            ret = ERC_KEY_INVALID;
        }
        else if ((NULL != msg) && (NULL != mac) && (mac_size <= 16U) && (mac_size > 0))
        {
            mac_para.cmd_id = EHSM_CMD_MAC;
            mac_para.u_hdr.hdr_ske.process_mode = EHSM_ONEPASS;
            mac_para.u_hdr.hdr_ske.direction = EHSM_MAC_VERIFICATION;
            mac_para.u_hdr.hdr_ske.padding = EHSM_NOPADDING;
            mac_para.u_hdr.hdr_ske.algorithm = EHSM_AES_128;
            mac_para.u_hdr.hdr_ske.cipher_mode = EHSM_CMAC_MODE;
            mac_para.u_hdr.hdr_ske.time_stamp = EHSM_NO_TIME_STAMP;
            mac_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_SHE;
            mac_para.key_handle = key_id;
            mac_para.input_addr = (ehsm_addr_t)msg;
            mac_para.input_size = size;
            mac_para.output_addr = (ehsm_addr_t)mac;
            mac_para.output_size = mac_size;
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &mac_para, EHSM_API_TYPE_SHE);
            *vrf_status = (EHSM_ERR_SW_SUCCESS == ret); /* crypto_lib return 0 for match */

            ret = ehsm_she_convert_ret_code(ret);
        }
        else
        {;} /* return ERC_GENERAL_ERROR */
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t she_load_key(const ehsm_uint8_t *m1, const ehsm_uint8_t *m2, const ehsm_uint8_t *m3,
        ehsm_uint8_t *m4, ehsm_uint8_t *m5)
{
    ehsm_uint32_t ret = ERC_GENERAL_ERROR;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        ret = she_load_key_local(m1, m2, m3, m4, m5, FALSE);
        clear_exec_she_cmd_flag();
    }
    return ret;
}

ehsm_uint32_t she_load_key_extend(const ehsm_uint8_t *m1, const ehsm_uint8_t *m2, const ehsm_uint8_t *m3,
        ehsm_uint8_t *m4, ehsm_uint8_t *m5)
{
    ehsm_uint32_t ret = ERC_GENERAL_ERROR;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        ret = she_load_key_local(m1, m2, m3, m4, m5, TRUE);
        clear_exec_she_cmd_flag();
    }
    return ret;
}

ehsm_uint32_t she_load_plain_key(const ehsm_uint8_t *key)
{
    ehsm_uint32_t ret = 0;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        if (key == NULL)
        {
            ret = ERC_GENERAL_ERROR;
        }
        else
        {
            ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_LOAD_PLAIN_KEY, (void *)key, EHSM_API_TYPE_SHE);
            ret = ehsm_she_convert_ret_code(ret);
        }
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t she_export_ram_key(ehsm_uint8_t *m1, ehsm_uint8_t *m2, ehsm_uint8_t *m3,
        ehsm_uint8_t *m4, ehsm_uint8_t *m5)
{

    ehsm_she_key_host_param_st key_param[1];

    ehsm_uint32_t ret = 0;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        if ((m1 == NULL) || (m2 == NULL) || (m3 == NULL) || (m4 == NULL) || (m5 == NULL))
        {
            ret = ERC_GENERAL_ERROR;
        }
        else
        {
            key_param->m1 = m1;
            key_param->m2 = m2;
            key_param->m3 = m3;
            key_param->m4 = m4;
            key_param->m5 = m5;
            key_param->she_ext_flag = FALSE;
            ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_EXPORT_RAM_KEY, key_param, EHSM_API_TYPE_SHE);
            ret = ehsm_she_convert_ret_code(ret);
        }
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t she_init_rng(void)
{
    return ERC_NO_ERROR;
}

ehsm_uint32_t she_extend_seed(void)
{
    return ERC_NO_ERROR;
}

ehsm_uint32_t she_rnd(ehsm_uint8_t *random_data_addr)
{
    ehsm_cmd_cipher_st rng_param;
    ehsm_uint32_t ret;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        if (random_data_addr == NULL)
        {
            ret = ERC_GENERAL_ERROR;
        }
        else
        {
            rng_param.cmd_id = EHSM_CMD_RNG_GENERATE;
            rng_param.u_hdr.hdr_rng.algorithm = EHSM_SM4_CTRDRBG;
            rng_param.output_addr = (ehsm_addr_t)random_data_addr;
            rng_param.output_size = 16U; /* The requset byte size is defined in spec 4.7.12 */
            ret = ehsm_process_sync_service(EHSM_SRV_CRYPTO_RANDOMGENERATE, &rng_param, EHSM_API_TYPE_SHE);
            ret = ehsm_she_convert_ret_code(ret);
        }
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t she_secure_boot(ehsm_uint32_t size, const ehsm_uint8_t *data)
{
    ehsm_uint32_t ret;
    ehsm_soc_image_verify_input_st *req = (ehsm_soc_image_verify_input_st *)0x1FFC0000;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        if (data == NULL || size <= 1024 || size > 255 * 1024)
        {
            ret = ERC_GENERAL_ERROR;
        }
        else
        {
            req->update_version_flag = 1;
            req->type = (ehsm_uint8_t)SECURE_BOOT_TYPE_SECURE_BOOT;
            *(ehsm_uint32_t *)req->storage_sign_addr = (ehsm_uint32_t)data;
            *(ehsm_uint32_t *)req->storage_sign_size = 16U;
            *(ehsm_uint32_t *)req->pubkey_addr = 0;
            *(ehsm_uint32_t *)req->pubkey_size = 0;
            *(ehsm_uint32_t *)req->storage_image_addr = (ehsm_uint32_t)(data + 1024);
            *(ehsm_uint32_t *)req->storage_image_size = *(ehsm_uint32_t *)(data + 256 + 320 + 16 + 12);
            *(ehsm_uint32_t *)req->storage_iv_addr = 0;
            *(ehsm_uint32_t *)req->storage_iv_size = 0;
            *(ehsm_uint32_t *)req->header_addr = (ehsm_uint32_t)(data + 256 + 320 + 16);
            *(ehsm_uint32_t *)req->header_size = 1024U - 256U - 320U - 16U;
            *(ehsm_uint32_t *)req->version_addr = (ehsm_uint32_t)(data + 256 + 320 + 16 + 16);
            *(ehsm_uint32_t *)req->version_size = 16;
            req->storage_alg = CODE_VERIFY_ALG_AES128_CMAC;
            req->storage_encryption_flag = 0;
            ret = ehsm_secure_boot(req);
        }
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t she_get_status(ehsm_uint8_t *sreg)
{
    ehsm_uint32_t ret = 0;

    if (sreg == NULL)
    {
        ret = ERC_GENERAL_ERROR;
    }
    else
    {
        ret = ehsm_process_sync_service(EHSM_SRV_GET_SHE_STATUS, (void *)sreg, EHSM_API_TYPE_SHE);
        ret = ehsm_she_convert_ret_code(ret);
    }

    return ret;
}

ehsm_uint32_t she_get_id(const ehsm_uint8_t *challenge, ehsm_uint8_t *id, ehsm_uint8_t *sreg, ehsm_uint8_t *mac)
{
    ehsm_uint32_t ret = 0;
    ehsm_she_get_id_param_st she_get_id_param;

    ehsm_uint8_t *status = (ehsm_uint8_t *)0x1FFC0000U;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        if ((challenge == NULL) || (id == NULL) || (sreg == NULL) || (mac == NULL))
        {
            ret = ERC_GENERAL_ERROR;
        }
        else
        {
            she_get_id_param.challenge = (ehsm_uint8_t *)challenge;
            she_get_id_param.challenge_size = 16;
            she_get_id_param.status = status;
            she_get_id_param.status_size = 16;
            she_get_id_param.signatrue = mac;
            she_get_id_param.signatrue_size = 16;

            ret = ehsm_process_sync_service(EHSM_SRV_GET_SHE_ID, (void *)&she_get_id_param, EHSM_API_TYPE_SHE);
            if (EHSM_ERR_SW_SUCCESS == ret)
            {
                (void)System_Memcpy(id, status, 15);
                (void)System_Memcpy(sreg, status + 15, 1);
            }

            ret = ehsm_she_convert_ret_code(ret);
        }
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t ehsm_she_cancel(void)
{
    ehsm_uint32_t ret = ehsm_process_sync_service(EHSM_SRV_SYS_SHE_CANCEL, NULL, EHSM_API_TYPE_SHE);
    return ehsm_she_convert_ret_code(ret);
}

ehsm_uint32_t she_debug(ehsm_uint8_t *challenge, const ehsm_uint8_t *auth)
{
    ehsm_debug_auth_st debug_auth;
    ehsm_uint32_t ret = 0;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        if (challenge == NULL || auth == NULL)
        {
            ret = ERC_GENERAL_ERROR;
        }
        else
        {
            debug_auth.alg = EHSM_DEBUG_AUTH_ALG_AES128_CMAC;
            debug_auth.type = EHSM_CHALLENGE_TYPE_SHE_DEBUG;
            debug_auth.signature = (ehsm_uint8_t *)auth;
            debug_auth.signature_size = 16;

            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_DEBUG_AUTHENTICATION, &debug_auth, EHSM_API_TYPE_SHE);

            ret = ehsm_she_convert_ret_code(ret);
        }
        clear_exec_she_cmd_flag();
    }

    return ret;
}

ehsm_uint32_t she_boot_ok(void)
{
    ehsm_uint32_t ret = ERC_GENERAL_ERROR;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        ret = do_set_she_boot_status((ehsm_uint8_t)EHSM_SOC_BOOT_STATUS_OK);
        clear_exec_she_cmd_flag();
    }
    return ret;
}

ehsm_uint32_t she_boot_failure(void)
{
    ehsm_uint32_t ret = ERC_GENERAL_ERROR;

    if (FALSE == can_exec_she_cmd())
    {
        ret = ERC_BUSY;
    }
    else
    {
        ret = do_set_she_boot_status((ehsm_uint8_t)EHSM_SOC_BOOT_STATUS_FAIL);
        clear_exec_she_cmd_flag();
    }
    return ret;
}
