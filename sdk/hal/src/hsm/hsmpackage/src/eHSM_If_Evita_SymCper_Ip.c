/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include "eHSM_Types_Ip.h"
#include "eHSM_Debug_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Srv_Cipher_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_If_Evita_Ip.h"
#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_If_Evita_ErrCode_Ip.h"

#include "eHSM_Mgr_Ctx_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

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

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static ehsm_uint32_t Cipher_Get_Block_Size(ehsm_uint32_t alg)
{
    ehsm_uint32_t block_sz;

    switch (alg)
    {
        case EHSM_DES:
        {
            block_sz = 8U;
            break;
        }
        case EHSM_AES_128:
        case EHSM_AES_192:
        case EHSM_AES_256:
        {
            block_sz = 16U;
            break;
        }
        case EHSM_SM4:
        {
            block_sz = 16U;
            break;
        }
        default:
        {
            block_sz = 0U;
            break;
        }
    }
    return block_sz;
}

static ehsm_uint32_t Cipher_Check_Operation_Mode(operation_mode_e operation_mode)
{
    ehsm_uint32_t ret;
    switch (operation_mode)
    {
#ifdef CONFIG_EHSM_CRYPTO_ALGOMODE_ECB
        case EVITA_ECB_MODE:
        {
            ret = EVITA_OK;
            break;
        }
#endif
        case EVITA_CBC_MODE:
        {
            ret = EVITA_OK;
            break;
        }
        case EVITA_CFB_MODE:
        {
            ret = EVITA_OK;
            break;
        }
        case EVITA_OFB_MODE:
        {
            ret = EVITA_OK;
            break;
        }
        case EVITA_CTR_MODE:
        {
            ret = EVITA_OK;
            break;
        }
        default:
        {
            ret = EVITA_ALGORITHM_ERROR;
            break;
        }
    }
    return ret;
}

static ehsm_uint32_t Mac_Check_Opreation_Mode(operation_mode_e operation_mode)
{
    ehsm_uint32_t ret;
    switch (operation_mode)
    {
        case EVITA_CMAC_MODE:
        {
            ret = EVITA_OK;
            break;
        }
        case EVITA_CBC_MAC_MODE:
        {
            ret = EVITA_OK;
            break;
        }
#ifdef CONFIG_EHSM_CRYPTO_ALGOMODE_GMAC
        case EVITA_GMAC_MODE:
        {
            ret = EVITA_OK;
            break;
        }
#endif
        default:
        {
            ret = EVITA_ALGORITHM_ERROR;
            break;
        }
    }
    return ret;
}

static ehsm_uint32_t Cipher_Check_Algorithm(ehsm_uint32_t alg)
{
    ehsm_uint32_t ret;
    switch (alg)
    {
        case EHSM_DES:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_AES_128:
        case EHSM_AES_192:
        case EHSM_AES_256:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SM4:
        {
            ret = EVITA_OK;
            break;
        }
        default:
        {
            ret = EVITA_ALGORITHM_ERROR;
            break;
        }
    }
    return ret;
}

static ehsm_uint32_t Cipher_Check_Padding(padding_scheme_e padding)
{
    ehsm_uint32_t ret;
    switch (padding)
    {
        case EVITA_NOPADDING:
        case EVITA_PKCS7:
        case EVITA_ONEWITHZEROS:
        {
            ret = EVITA_OK;
            break;
        }
        default:
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
    }
    return ret;
}

static ehsm_uint32_t Cipher_Check_Direction(cipher_mode_e cipher_mode)
{
    ehsm_uint32_t ret;
    switch (cipher_mode)
    {
        case EVITA_ENCRYPTION:
        case EVITA_DECRYPTION:
        {
            ret = EVITA_OK;
            break;
        }
        default:
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
    }
    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
ehsm_uint32_t Cipher_Init(ehsm_uint32_t algorithm_identifier, cipher_mode_e cipher_mode, operation_mode_e operation_mode,
                         padding_scheme_e padding, ehsm_uint32_t total_message_length, ehsm_uint32_t iv_size,
                         ehsm_uint8_t *iv, ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size,
                         ehsm_uint8_t *key_authorization_value, ehsm_ctx_session_st *session_handle)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st aes_para = {0};
    ehsm_uint32_t session_id;
    cipher_session_st *session_ctx;
    ehsm_uint32_t block_sz;

    do
    {
        ret = Cipher_Check_Algorithm(algorithm_identifier);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {
            block_sz = Cipher_Get_Block_Size(algorithm_identifier);
        }

        ret = Cipher_Check_Direction(cipher_mode);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Cipher_Check_Padding(padding);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Cipher_Check_Operation_Mode(operation_mode);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        if (total_message_length == 0)
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        else
        {
            if (padding == EVITA_NOPADDING)
            {
                if (total_message_length % block_sz != 0U)
                {
                    ret = EVITA_GENERAL_ERROR;
                    break;
                }
                else
                {;}
            }
            else
            {;}
        }

        ret = Evita_Check_Key_Handle(key_handle);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Evita_Check_Authorization_Code(key_authorization_size, key_authorization_value);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        if (operation_mode != EVITA_ECB_MODE)
        {
            if (iv_size == 0U)
            {
                ret = EVITA_WRONG_IV;
                break;
            }
            else
            {
                if (iv_size == block_sz)
                {
                    if (iv != NULL)
                    {
                        ret = EVITA_OK;
                    }
                    else
                    {
                        ret = EVITA_WRONG_IV;
                        break;
                    }
                }
                else
                {
                    ret = EVITA_WRONG_IV;
                    break;
                }
            }
        }
        else
        {
            if ((iv_size != 0U) || (iv != NULL))
            {
                /* EVITA_LOG_WARN("Given IV is valid in ECB mode!\r\n"); */
            }
            else
            {;}
        }
    } while (0U);

    /* Try to get the context session */
    if (ret == EVITA_OK)
    {
        if (session_handle != NULL)
        {
            ret = eHSM_Mgr_Cipher_Ctx_Get_Free(&session_id, &session_ctx);
            if (EVITA_OK == ret)
            {
                ret = ehsm_init_block_mgr(&session_ctx->ctx_block_mgr, Cipher_Get_Block_Size(algorithm_identifier));
            }
        }
        else
        {
            ret = EVITA_WRONG_SESSION_HANDLE;
        }
    }
    else
    {;}

    if (ret == EVITA_OK)
    {
        aes_para.cmd_id = EHSM_CMD_SYM_CIPHER;
        aes_para.u_hdr.hdr_ske.process_mode = EHSM_START;
        aes_para.u_hdr.hdr_ske.direction = (ehsm_uint8_t)cipher_mode;
        aes_para.u_hdr.hdr_ske.padding = (ehsm_uint8_t)padding;
        aes_para.u_hdr.hdr_ske.algorithm = algorithm_identifier;
        aes_para.u_hdr.hdr_ske.cipher_mode = (ehsm_uint8_t)operation_mode;
        aes_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
        aes_para.key_handle = key_handle;
        aes_para.key_addr = (ehsm_addr_t)key_authorization_value;
        aes_para.key_size = key_authorization_size;
        aes_para.input_size = total_message_length;
        if (operation_mode == EVITA_ECB_MODE)
        {
            aes_para.sec_input_addr = (ehsm_addr_t)0U;
            aes_para.sec_input_size = 0U;
        }
        else
        {
            aes_para.sec_input_addr = (ehsm_addr_t)iv;
            aes_para.sec_input_size = iv_size;
        }
        aes_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
        aes_para.context_size = sizeof(session_ctx->ctx);
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &aes_para, EHSM_API_TYPE_EVITA);
        if (ret == EHSM_ERR_SW_SUCCESS)
        {
            session_handle->max_chunk_size = EVITA_MAX_CHUNK_SIZE;
            session_handle->block_sz = block_sz;
            session_ctx->direction = aes_para.u_hdr.hdr_ske.direction;
            session_ctx->padding = aes_para.u_hdr.hdr_ske.padding;
            session_ctx->algorithm = aes_para.u_hdr.hdr_ske.algorithm;
            session_ctx->cipher_mode = aes_para.u_hdr.hdr_ske.cipher_mode;
            session_ctx->key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
            session_ctx->key_handle = key_handle;
            session_ctx->status = EVITA_INIT;
            session_handle->session_id = session_id;
        }
        else
        {
            eHSM_Mgr_Cipher_Ctx_Free(session_id);
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Cipher_Process(ehsm_ctx_session_st *session_handle, ehsm_uint32_t input_data_size, const ehsm_uint8_t *input_data,
                             ehsm_uint32_t *output_data_size, ehsm_uint8_t *output_data)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_with_rps_st aes_packet = {0};
    ehsm_cmd_cipher_st aes_para = {0};
    cipher_session_st *session_ctx;

    if ((session_handle == NULL) || (session_handle->max_chunk_size == 0U))
    {
        ret = EVITA_WRONG_SESSION_HANDLE;
    }
    else if (input_data_size > session_handle->max_chunk_size)
    {
        ret = EVITA_WRONG_CHUNK_SIZE;
    }
    else if ((output_data_size == NULL) || (output_data == NULL) || ((input_data == NULL) && (input_data_size != 0U)))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((session_ctx->status == EVITA_INIT) || (session_ctx->status == EVITA_UPDATE)))
        {
            aes_para.cmd_id = EHSM_CMD_SYM_CIPHER;
            aes_para.u_hdr.hdr_ske.process_mode = EHSM_UPDATE;
            aes_para.u_hdr.hdr_ske.direction = session_ctx->direction;
            aes_para.u_hdr.hdr_ske.padding = session_ctx->padding;
            aes_para.u_hdr.hdr_ske.algorithm = session_ctx->algorithm;
            aes_para.u_hdr.hdr_ske.cipher_mode = session_ctx->cipher_mode;
            aes_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
            aes_para.key_handle = session_ctx->key_handle;
            aes_para.input_addr = (ehsm_addr_t)input_data;
            /* input_data_size could be 0, and in that case EHSM will return EHSM_ERR_SW_SUCCESS */
            aes_para.input_size = input_data_size;
            aes_para.output_addr = (ehsm_addr_t)output_data;
            aes_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
            aes_para.context_size = sizeof(session_ctx->ctx);
            aes_packet.req_cipher = aes_para;
            /* ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &aes_packet, EHSM_API_TYPE_EVITA); */
            do
            {
                ret = ehsm_disjuction_updata(EHSM_SRV_EXTENDED_CRYPTO_SKE, &aes_packet, &session_ctx->ctx_block_mgr);
                if (EHSM_ERR_SW_SUCCESS != ret)
                {
                    break;
                }
            } while (aes_packet.req_cipher.input_size > 0);
            if (ret == EHSM_ERR_SW_SUCCESS)
            {
                session_ctx->status = EVITA_UPDATE;
                *output_data_size = aes_packet.output_size;
            }
            else
            {
                eHSM_Mgr_Cipher_Ctx_Free(session_handle->session_id);
            }
            ret = ehsm_evita_convert_ret_code(ret);
        }
        else
        {;}
    }

    return ret;
}

ehsm_uint32_t Cipher_Finish(ehsm_ctx_session_st *session_handle, ehsm_uint32_t *output_data_size, ehsm_uint8_t *output_data)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_with_rps_st aes_packet = {0}; 
    ehsm_cmd_cipher_st aes_para = {0};
    cipher_session_st *session_ctx;

    if (session_handle == NULL)
    {
        ret = EVITA_WRONG_SESSION_HANDLE;
    }
    else if ((output_data_size == NULL) || (output_data == NULL))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((session_ctx->status == EVITA_INIT) || (session_ctx->status == EVITA_UPDATE)))
        {
            aes_para.cmd_id = EHSM_CMD_SYM_CIPHER;
            aes_para.u_hdr.hdr_ske.process_mode = EHSM_FINISH;
            aes_para.u_hdr.hdr_ske.direction = session_ctx->direction;
            aes_para.u_hdr.hdr_ske.padding = session_ctx->padding;
            aes_para.u_hdr.hdr_ske.algorithm = session_ctx->algorithm;
            aes_para.u_hdr.hdr_ske.cipher_mode = session_ctx->cipher_mode;
            aes_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
            aes_para.key_handle = session_ctx->key_handle;
            aes_para.output_addr = (ehsm_addr_t)output_data;
            aes_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
            aes_para.context_size = sizeof(session_ctx->ctx);
            ret = ehsm_disjuction_finish(&session_ctx->ctx_block_mgr, &aes_para.input_addr, &aes_para.input_size);
            aes_packet.req_cipher = aes_para;
            if (ret == EHSM_ERR_SW_SUCCESS)
            {
                ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &aes_packet, EHSM_API_TYPE_EVITA);
                if (ret == EHSM_ERR_SW_SUCCESS)
                {
                    session_ctx->status = EVITA_FINISH;
                    *output_data_size = aes_packet.output_size;
                }
            }
            else
            {;}
            ret = ehsm_evita_convert_ret_code(ret);
        }
        else
        {;}

        eHSM_Mgr_Cipher_Ctx_Free(session_handle->session_id);
    }
    return ret;
}

static ehsm_uint32_t Aead_Iv_Tag_Check(operation_mode_e mode, ehsm_uint32_t iv_size, ehsm_uint32_t tag_size)
{
    ehsm_uint32_t ret;

    if (mode == EVITA_GCM_MODE)
    {
        if (iv_size == 12)
        {
            ret = EVITA_OK;
        }
        else
        {
            ret = EVITA_ALGORITHM_ERROR;
        }
    }
    else
    {
        if ((iv_size >= 7 && iv_size <= 13) && ((tag_size >= 4) && (tag_size <= 16) && (tag_size % 2 == 0)))
        {
            ret = EVITA_OK;
        }
        else
        {
            ret = EVITA_ALGORITHM_ERROR;
        }
    }

    return ret;
}

static ehsm_uint32_t Aead_Aad_Check(ehsm_uint32_t aad_size)
{
    ehsm_uint32_t ret;

    if (aad_size <= CONFIG_EHSM_CRYPTO_V_GCM_MAX_AAD_SIZE)
    {
        ret = EVITA_OK;
    }
    else
    {
       ret = EVITA_ALGORITHM_ERROR;
    }

    return ret;
}

static ehsm_uint32_t Aead_Check_Operation_Mode(operation_mode_e operation_mode)
{
    ehsm_uint32_t ret;
    switch (operation_mode)
    {
        case EVITA_GCM_MODE:
        {
            ret = EVITA_OK;
            break;
        }
#ifdef CONFIG_EHSM_CRYPTO_ALGOMODE_CCM
        case EVITA_CCM_MODE:
        {
            ret = EVITA_OK;
            break;
        }
#endif
        default:
        {
            ret = EVITA_ALGORITHM_ERROR;
            break;
        }
    }
    return ret;
}

static ehsm_uint32_t Aead_Check_Algorithm(ehsm_uint32_t alg)
{
    ehsm_uint32_t ret;
    switch (alg)
    {
        case EHSM_AES_128:
        case EHSM_AES_192:
        case EHSM_AES_256:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SM4:
        {
            ret = EVITA_OK;
            break;
        }
        default:
        {
            ret = EVITA_ALGORITHM_ERROR;
            break;
        }
    }
    return ret;
}

ehsm_uint32_t Aead_Init(ehsm_uint32_t algorithm_identifier, cipher_mode_e cipher_mode, operation_mode_e operation_mode,
                        ehsm_uint32_t total_message_length, ehsm_uint32_t iv_size,
                        ehsm_uint8_t *iv, ehsm_uint32_t aad_size, ehsm_uint32_t tag_size, ehsm_uint32_t key_handle,
                        ehsm_uint32_t key_authorization_size, ehsm_uint8_t *key_authorization_value,
                        ehsm_ctx_session_st *session_handle)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st cipher_para = {0};
    ehsm_uint32_t session_id;
    cipher_session_st *session_ctx;
    ehsm_uint32_t block_sz;

    do
    {
        ret = Aead_Check_Algorithm(algorithm_identifier);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        block_sz = Cipher_Get_Block_Size(algorithm_identifier);

        ret = Aead_Check_Operation_Mode(operation_mode);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        if ((cipher_mode != EVITA_ENCRYPTION) && (cipher_mode != EVITA_DECRYPTION))
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        else
        {;}

        ret = Aead_Iv_Tag_Check(operation_mode, iv_size, tag_size);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Aead_Aad_Check(aad_size);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Evita_Check_Key_Handle(key_handle);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Evita_Check_Authorization_Code(key_authorization_size, key_authorization_value);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        if (iv == NULL)
        {
            ret = EVITA_WRONG_IV;
            break;
        }
        else
        {;}
    } while (0U);

    /* Try to get the context session */
    if (ret == EVITA_OK)
    {
        if (session_handle != NULL)
        {
            ret = eHSM_Mgr_Cipher_Ctx_Get_Free(&session_id, &session_ctx);
        }
        else
        {
            ret = EVITA_WRONG_SESSION_HANDLE;
        }
    }
    else
    {;}

    if (ret == EVITA_OK)
    {
        if (operation_mode == EVITA_GCM_MODE)
        {
            cipher_para.cmd_id = EHSM_CMD_AEAD_GCM;
            /* GCM only support tag size of 16 now. */
            cipher_para.u_hdr.hdr_ske.tag_size = 16U;
        }
        else
        {
            cipher_para.cmd_id = EHSM_CMD_AEAD_CCM;
            cipher_para.u_hdr.hdr_ske.tag_size = tag_size;
        }
        cipher_para.u_hdr.hdr_ske.process_mode = EHSM_START;
        cipher_para.u_hdr.hdr_ske.direction = (ehsm_uint8_t)cipher_mode;
        cipher_para.u_hdr.hdr_ske.algorithm = algorithm_identifier;
        cipher_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
        cipher_para.key_handle = key_handle;
        cipher_para.key_addr = (ehsm_addr_t)key_authorization_value;
        cipher_para.key_size = key_authorization_size;
        /* total_message_length could be any value, including 0 */
        cipher_para.input_size = total_message_length;
        cipher_para.sec_input_addr = (ehsm_addr_t)iv;
        cipher_para.sec_input_size = iv_size;
        /* aad_size could be any value, include 0 */
        cipher_para.output_size = aad_size;
        cipher_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
        cipher_para.context_size = sizeof(session_ctx->ctx);
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &cipher_para, EHSM_API_TYPE_EVITA);
        if (ret == EHSM_ERR_SW_SUCCESS)
        {
            session_handle->max_chunk_size = EVITA_MAX_CHUNK_SIZE;
            session_handle->block_sz = block_sz;
            session_ctx->direction = cipher_para.u_hdr.hdr_ske.direction;
            session_ctx->algorithm = cipher_para.u_hdr.hdr_ske.algorithm;
            /* GCM or CCM */
            session_ctx->cipher_mode = (ehsm_uint8_t)operation_mode;
            session_ctx->key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
            session_ctx->key_handle = key_handle;
            session_ctx->status = EVITA_INIT;
            session_handle->session_id = session_id;
        }
        else
        {
            eHSM_Mgr_Cipher_Ctx_Free(session_id);
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    else
    {;}

    return ret;
}

/*
 * @brief Aead update with aad.
 *        If aad is not null, it should be put for the first time.
 */
ehsm_uint32_t Aead_Process(ehsm_ctx_session_st *session_handle, ehsm_uint32_t input_data_size,
                           const ehsm_uint8_t *input_data, ehsm_uint32_t aad_size, const ehsm_uint8_t *aad,
                           ehsm_uint32_t *output_data_size, ehsm_uint8_t *output_data)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st cipher_para = {0};
    cipher_session_st *session_ctx;
    ehsm_aead_data_ptr_st *aead_data;

    if ((NULL == session_handle) || (session_handle->max_chunk_size == 0U))
    {
        ret = EVITA_WRONG_SESSION_HANDLE;
    }
    else if (input_data_size > session_handle->max_chunk_size)
    {
        ret = EVITA_WRONG_CHUNK_SIZE;
    }
    else if ((output_data_size == NULL) || (output_data == NULL) || ((input_data == NULL) && (input_data_size != 0U)))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((EVITA_INIT == session_ctx->status) || (EVITA_UPDATE == session_ctx->status)))
        {
            if (session_ctx->cipher_mode == EVITA_GCM_MODE)
            {
                cipher_para.cmd_id = EHSM_CMD_AEAD_GCM;
            }
            else
            {
                cipher_para.cmd_id = EHSM_CMD_AEAD_CCM;
            }
            cipher_para.u_hdr.hdr_ske.process_mode = EHSM_UPDATE;
            cipher_para.u_hdr.hdr_ske.direction = session_ctx->direction;
            cipher_para.u_hdr.hdr_ske.algorithm = session_ctx->algorithm;
            cipher_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
            cipher_para.key_handle = session_ctx->key_handle;
            aead_data = &session_ctx->aead_data;
            aead_data->input_data.data_ptr = (ehsm_addr_t)input_data;
            aead_data->input_data.aad_ptr = (ehsm_addr_t)aad;
            aead_data->output_data.data_ptr = (ehsm_addr_t)output_data;
            /* input_data_size could be 0, and in that case EHSM will return EHSM_ERR_SW_SUCCESS */
            cipher_para.input_size = input_data_size;
            cipher_para.input_addr = (ehsm_addr_t)&aead_data->input_data;
            cipher_para.output_addr = (ehsm_addr_t)&aead_data->output_data;
            cipher_para.output_size = aad_size;
            cipher_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
            cipher_para.context_size = sizeof(session_ctx->ctx);
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &cipher_para, EHSM_API_TYPE_EVITA);
            if (ret == EHSM_ERR_SW_SUCCESS)
            {
                session_ctx->status = EVITA_UPDATE;
                /* TODO: for padding, use the output size from ehsm. */
                *output_data_size = input_data_size;
            }
            else
            {
                eHSM_Mgr_Cipher_Ctx_Free(session_handle->session_id);
            }
            ret = ehsm_evita_convert_ret_code(ret);
        }
        else
        {;}
    }

    return ret;
}

ehsm_uint32_t Aead_Finish(ehsm_ctx_session_st *session_handle, ehsm_uint32_t *output_data_size, ehsm_uint8_t *output_data,
                          ehsm_uint32_t *match)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st cipher_para = {0};
    cipher_session_st *session_ctx;
    ehsm_aead_data_ptr_st *aead_data;

    if (NULL == session_handle)
    {
        ret = EVITA_WRONG_SESSION_HANDLE;
    }
    else if ((output_data_size == NULL) || (output_data == NULL))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((EVITA_INIT == session_ctx->status) || (EVITA_UPDATE == session_ctx->status)))
        {
            if (session_ctx->direction == EVITA_DECRYPTION && match == NULL)
            {
                ret = EVITA_GENERAL_ERROR;
            }
            else
            {
                if (session_ctx->cipher_mode == EVITA_GCM_MODE)
                {
                    cipher_para.cmd_id = EHSM_CMD_AEAD_GCM;
                }
                else
                {
                    cipher_para.cmd_id = EHSM_CMD_AEAD_CCM;
                }
                cipher_para.u_hdr.hdr_ske.process_mode = EHSM_FINISH;
                cipher_para.u_hdr.hdr_ske.direction = session_ctx->direction;
                cipher_para.u_hdr.hdr_ske.algorithm = session_ctx->algorithm;
                cipher_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
                cipher_para.key_handle = session_ctx->key_handle;
                aead_data = &session_ctx->aead_data;
                /* The ehsm_slave should write the last block data to output_data */
                if (session_ctx->direction == EVITA_DECRYPTION)
                {
                    aead_data->input_data.tag_ptr = (ehsm_addr_t)output_data;
                }
                else
                {
                    aead_data->output_data.tag_ptr = (ehsm_addr_t)output_data;
                }
                /* No input data to be handled */
                cipher_para.input_size = 0;
                cipher_para.input_addr = (ehsm_addr_t)&aead_data->input_data;
                cipher_para.output_addr = (ehsm_addr_t)&aead_data->output_data;
                cipher_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
                cipher_para.context_size = sizeof(session_ctx->ctx);
                ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &cipher_para, EHSM_API_TYPE_EVITA);
                if (ret == EHSM_ERR_SW_SUCCESS)
                {
                    session_ctx->status = EVITA_FINISH;
                    if (session_ctx->direction == EVITA_DECRYPTION)
                    {
                        *match = 1;
                    }
                    else
                    {
                        /* TODO: The output size should be updated with the real data handled by HSM. */
                    }
                }
                else
                {
                    if (match != NULL)
                    {
                        *match = 0;
                    }
                }
                ret = ehsm_evita_convert_ret_code(ret);
            }
        }
        else
        {;}
    }

    if (session_handle != NULL)
    {
        eHSM_Mgr_Cipher_Ctx_Free(session_handle->session_id);
    }
    else
    {;}

    return ret;
}

ehsm_uint32_t MAC_Init(ehsm_uint32_t algorithm_identifier, mac_mode_e mac_mode, operation_mode_e operation_mode,
                       padding_scheme_e padding_scheme, ehsm_uint32_t total_message_length, ehsm_uint32_t mac_length,
                       ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size, ehsm_uint8_t *key_authorization_value,
                       ehsm_ctx_session_st *session_handle)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st mac_para = {0};
    ehsm_uint32_t session_id;
    cipher_session_st *session_ctx;
    ehsm_uint32_t block_sz;

    /* Check the parameter */
    do
    {
        ret = Cipher_Check_Algorithm(algorithm_identifier);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {
            block_sz = Cipher_Get_Block_Size(algorithm_identifier);
        }

        if ((mac_mode != EVITA_MAC_SIGN) && (mac_mode != EVITA_MAC_VERIFY) && (mac_mode != EVITA_MAC_TIMESTAMPED_SIGN))
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        else
        {;}

        ret = Mac_Check_Opreation_Mode(operation_mode);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Cipher_Check_Padding(padding_scheme);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Evita_Check_Key_Handle(key_handle);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Evita_Check_Authorization_Code(key_authorization_size, key_authorization_value);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        if (mac_length == 0U)
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        else if (mac_length > block_sz)
        {
            ret = EVITA_MAC_LENGTH_OVERSIZE;
            break;
        }
        else
        {
            ret = EVITA_OK;
        }
    } while (0U);

    /* Try to get the context session */
    if (EVITA_OK == ret)
    {
        if (NULL != session_handle)
        {
            ret = eHSM_Mgr_Cipher_Ctx_Get_Free(&session_id, &session_ctx);
        }
        else
        {
            ret = EVITA_WRONG_SESSION_HANDLE;
        }
    }
    else
    {;}

    /* Prepare the data for the mailbox command */
    if (ret == EVITA_OK)
    {
        mac_para.cmd_id = EHSM_CMD_MAC;
        mac_para.u_hdr.hdr_ske.process_mode = EHSM_START;
        mac_para.u_hdr.hdr_ske.direction = (mac_mode == EVITA_MAC_VERIFY) ? EHSM_MAC_VERIFICATION :EHSM_MAC_GENERATION;
        mac_para.u_hdr.hdr_ske.padding = (ehsm_uint8_t)padding_scheme;
        mac_para.u_hdr.hdr_ske.algorithm = algorithm_identifier;
        mac_para.u_hdr.hdr_ske.cipher_mode = (ehsm_uint8_t)operation_mode;
        mac_para.u_hdr.hdr_ske.time_stamp = (mac_mode == EVITA_MAC_TIMESTAMPED_SIGN) ? EHSM_USE_TIME_STAMP : EHSM_NO_TIME_STAMP;
        mac_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
        mac_para.key_handle = key_handle;
        mac_para.key_size = key_authorization_size;
        mac_para.key_addr = (ehsm_addr_t)key_authorization_value;
        mac_para.input_size = total_message_length;
        mac_para.output_size = mac_length;
        mac_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
        mac_para.context_size = sizeof(session_ctx->ctx);
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &mac_para, EHSM_API_TYPE_EVITA);
        if (ret == EHSM_ERR_SW_SUCCESS)
        {
            session_ctx->direction = mac_para.u_hdr.hdr_ske.direction;
            session_ctx->padding = mac_para.u_hdr.hdr_ske.padding;
            session_ctx->algorithm = mac_para.u_hdr.hdr_ske.algorithm;
            session_ctx->cipher_mode = mac_para.u_hdr.hdr_ske.cipher_mode;
            session_ctx->time_stamp = mac_para.u_hdr.hdr_ske.time_stamp;
            session_ctx->mac_bytes = mac_length;
            session_ctx->key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
            session_ctx->key_handle = key_handle;
            session_ctx->status = EVITA_INIT;

            /* ehsm_uint32_t total_message_length; */
            session_handle->max_chunk_size = EVITA_MAX_CHUNK_SIZE;
            session_handle->block_sz = block_sz;
            session_handle->session_id = session_id;
        }
        else
        {
            eHSM_Mgr_Cipher_Ctx_Free(session_id);
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    else
    {;}

    return ret;
}

ehsm_uint32_t MAC_Update(ehsm_ctx_session_st *session_handle, ehsm_uint32_t chunk_size, const ehsm_uint8_t *chunk_data)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st mac_para = {0};
    cipher_session_st *session_ctx;

    if ((session_handle == NULL) || (session_handle->max_chunk_size == 0U))
    {
        ret = EVITA_WRONG_SESSION_HANDLE;
    }
    else if (chunk_size > session_handle->max_chunk_size)
    {
        ret = EVITA_WRONG_CHUNK_SIZE;
    }
    else if ((chunk_data == NULL) && (chunk_size != 0U))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((session_ctx->status == EVITA_INIT) || (session_ctx->status == EVITA_UPDATE)))
        {
            mac_para.cmd_id = EHSM_CMD_MAC;
            mac_para.u_hdr.hdr_ske.process_mode = EHSM_UPDATE;
            mac_para.u_hdr.hdr_ske.direction = session_ctx->direction;
            mac_para.u_hdr.hdr_ske.padding = session_ctx->padding;
            mac_para.u_hdr.hdr_ske.algorithm = session_ctx->algorithm;
            mac_para.u_hdr.hdr_ske.cipher_mode = session_ctx->cipher_mode;
            mac_para.u_hdr.hdr_ske.time_stamp = session_ctx->time_stamp;
            mac_para.u_hdr.hdr_ske.key_type = session_ctx->key_type;
            mac_para.key_handle = session_ctx->key_handle;
            /* chunk_data could be 0, and in that case EHSM will return EHSM_ERR_SW_SUCCESS */
            mac_para.input_addr = (ehsm_addr_t)chunk_data;
            mac_para.input_size = chunk_size;
            /* The output data should be saved in to session_handle and as the input data for next Update() function-call */
            mac_para.context_addr = (ehsm_addr_t)(session_ctx->ctx);
            mac_para.context_size = sizeof(session_ctx->ctx);
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &mac_para, EHSM_API_TYPE_EVITA);
            if (ret == EHSM_ERR_SW_SUCCESS)
            {
                session_ctx->status = EVITA_UPDATE;
            }
            else
            {
                eHSM_Mgr_Cipher_Ctx_Free(session_handle->session_id);
            }
            ret = ehsm_evita_convert_ret_code(ret);
        }
        else
        {;}
    }

    return ret;
}

ehsm_uint32_t MAC_Finish(ehsm_ctx_session_st *session_handle, ehsm_uint32_t *mac_size, mac_st *mac, ehsm_bool_t *mac_match)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st mac_para = {0};
    cipher_session_st *session_ctx;

    if (session_handle == NULL)
    {
        ret = EVITA_WRONG_SESSION_HANDLE;
    }
    else if ((mac_size == NULL) || (mac == NULL))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((session_ctx->status == EVITA_INIT) || (session_ctx->status == EVITA_UPDATE)))
        {
#ifdef CONFIG_EHSM_HW_UTC_TIME
            if (session_ctx->time_stamp == EHSM_USE_TIME_STAMP)
            {
                ret = Get_UTC_Time(&mac->utc_time);
                if (ret == EVITA_OK)
                {
                    mac_para.input_addr = (ehsm_addr_t)&mac->utc_time;
                    mac_para.input_size = sizeof(ehsm_utc_time_t);
                    mac_para.u_hdr.hdr_ske.time_stamp = EHSM_USE_TIME_STAMP;
                }
                else
                {
                    /* If it is failed to get the utc time, the mac will be also continue calculated without timestamp */
                    /* As the result the mac->utc_time will be 0 */
                    mac_para.input_addr = (ehsm_addr_t)0;
                    mac_para.input_size = 0;
                    mac_para.u_hdr.hdr_ske.time_stamp = EHSM_NO_TIME_STAMP;
                    mac->utc_time = 0;
                }
            }
            else
#endif
            {
                mac_para.u_hdr.hdr_ske.time_stamp = EHSM_NO_TIME_STAMP;
            }
            mac_para.cmd_id = EHSM_CMD_MAC;
            mac_para.u_hdr.hdr_ske.process_mode = EHSM_FINISH;
            mac_para.u_hdr.hdr_ske.direction = session_ctx->direction;
            mac_para.u_hdr.hdr_ske.padding = session_ctx->padding;
            mac_para.u_hdr.hdr_ske.algorithm = session_ctx->algorithm;
            mac_para.u_hdr.hdr_ske.cipher_mode = session_ctx->cipher_mode;
            mac_para.u_hdr.hdr_ske.key_type = session_ctx->key_type;
            mac_para.key_handle = session_ctx->key_handle;
            mac_para.output_addr = (ehsm_addr_t)mac->mac_value;
            /* Mac size has been set in the initialization step. */
            if (mac_para.u_hdr.hdr_ske.direction == EHSM_MAC_VERIFICATION)
            {
                mac_para.output_size = *mac_size;
            }
            else
            {
                mac_para.output_size = 0;
            }
            mac_para.context_addr = (ehsm_addr_t)(session_ctx->ctx);
            mac_para.context_size = sizeof(session_ctx->ctx);
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_SKE, &mac_para, EHSM_API_TYPE_EVITA);
            session_ctx->status = EVITA_FINISH;
            if (mac_para.u_hdr.hdr_ske.direction == EHSM_MAC_VERIFICATION)
            {
                if (mac_match != NULL)
                {
                    *mac_match = (ret == EHSM_ERR_SW_SUCCESS) ? TRUE : FALSE;
                }
                else
                {
                    ret = EVITA_GENERAL_ERROR;
                }
            }
            else
            {
                *mac_size = Cipher_Get_Block_Size(mac_para.u_hdr.hdr_ske.algorithm);
            }
            ret = ehsm_evita_convert_ret_code(ret);
        }
        else
        {;}

        eHSM_Mgr_Cipher_Ctx_Free(session_handle->session_id);
    }
    return ret;
}
