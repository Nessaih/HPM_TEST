/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include "eHSM_If_Evita_Ip.h"

#include "eHSM_Types_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Srv_Cipher_Ip.h"
#include "eHSM_Err_Code_Ip.h"
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
/**
 * @brief Check if the given hash algorithm is valid for RSA.
 * @param hash_alg Hash algorithm identifier.
 * @return EVITA_OK if valid, EVITA_ALGORITHM_ERROR otherwise.
 */
static ehsm_uint32_t RSA_Check_Hash_Algorithm(ehsm_uint32_t hash_alg)
{
    ehsm_uint32_t ret;
    switch (hash_alg)
    {
        case EHSM_SHA1:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA224:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA256:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA384:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA512:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA512_224:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA512_256:
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

/**
 * @brief Check if the given hash algorithm is valid for ECDSA.
 * @param hash_alg Hash algorithm identifier.
 * @return EVITA_OK if valid, EVITA_ALGORITHM_ERROR otherwise.
 */
static ehsm_uint32_t ECDSA_Check_Hash_Algorithm(ehsm_uint32_t hash_alg)
{
    ehsm_uint32_t ret;
    switch(hash_alg)
    {
        case EHSM_SM3:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_MD5:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA1:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA256:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA224:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA384:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA512:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA512_224:
        {
            ret = EVITA_OK;
            break;
        }
        case EHSM_SHA512_256:
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

/**
 * @brief Check if the asymmetric algorithm and hash algorithm combination is valid.
 * @param algorithm_identifier Asymmetric algorithm identifier.
 * @param hash_algorithm_identifier Hash algorithm identifier.
 * @return EVITA_OK if valid, EVITA_ALGORITHM_ERROR otherwise.
 */
static ehsm_uint32_t Evita_Check_Asymmetric_Algorithm(ehsm_asym_alg_e algorithm_identifier, ehsm_uint32_t hash_algorithm_identifier)
{
    ehsm_uint32_t ret;
    switch (algorithm_identifier)
    {
        case EVITA_ASYM_SM2:
        {
            ret = EVITA_OK;
            break;
        }
        case EVITA_ASYM_ECDSA:
        {
            ret = ECDSA_Check_Hash_Algorithm(hash_algorithm_identifier);
            break;
        }
        case EVITA_ASYM_RSA:
        {
            ret = RSA_Check_Hash_Algorithm(hash_algorithm_identifier);
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

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
/**
 * @brief Initialize the signature operation for asymmetric algorithms.
 * @param algorithm_identifier Asymmetric algorithm identifier.
 * @param hash_algorithm_identifier Hash algorithm identifier.
 * @param padding Padding scheme.
 * @param total_message_length Total message length.
 * @param time_stamp_signature Enable time stamp signature.
 * @param key_handle Key handle.
 * @param key_authorization_size Size of key authorization value.
 * @param key_authorization_value Pointer to key authorization value.
 * @param session_handle Pointer to session handle.
 * @return Result code.
 */
ehsm_uint32_t Sign_Init(ehsm_asym_alg_e algorithm_identifier, ehsm_uint32_t hash_algorithm_identifier, padding_scheme_e padding,
                        ehsm_uint32_t total_message_length, ehsm_bool_t time_stamp_signature, ehsm_uint32_t key_handle,
                        ehsm_uint32_t key_authorization_size, ehsm_uint8_t *key_authorization_value, ehsm_ctx_session_st *session_handle)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st pke_para = {0};
    ehsm_uint32_t session_id;
    cipher_session_st *session_ctx;

    /* Check the parameter */
    do
    {
        ret = Evita_Check_Asymmetric_Algorithm(algorithm_identifier, hash_algorithm_identifier);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        if ((padding != EVITA_NOPADDING) && (padding != EVITA_PSASSA_PSS))
        {
            ret = EVITA_GENERAL_ERROR;
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
    } while(0U);

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

    /* Prepare the data for the mailbox command */
    if (EVITA_OK == ret)
    {
        if (EVITA_ASYM_SM2 == algorithm_identifier)
        {
            pke_para.cmd_id = EHSM_CMD_SM2_SIGN;
        }
        else if (EVITA_ASYM_ECDSA == algorithm_identifier)
        {
            pke_para.cmd_id = EHSM_CMD_ECDSA;
            pke_para.u_hdr.hdr_pke.algorithm = hash_algorithm_identifier;
            pke_para.u_hdr.hdr_pke.time_stamp = (ehsm_uint8_t)time_stamp_signature;
        }
        else
        {
            pke_para.cmd_id = EHSM_CMD_RSA_SIGN;
            pke_para.u_hdr.hdr_pke.padding = (ehsm_uint8_t)padding;
            pke_para.u_hdr.hdr_pke.algorithm = hash_algorithm_identifier;
            pke_para.u_hdr.hdr_pke.time_stamp = (ehsm_uint8_t)time_stamp_signature;
        }
        pke_para.u_hdr.hdr_pke.process_mode = EHSM_START;
        pke_para.u_hdr.hdr_pke.direction = EHSM_SIGN_GENERATION;
        pke_para.key_handle = key_handle;
        pke_para.key_addr = (ehsm_addr_t)key_authorization_value;
        pke_para.key_size = key_authorization_size;
        pke_para.input_size = total_message_length;
        pke_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
        pke_para.context_size = sizeof(session_ctx->ctx);
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_PKE, &pke_para, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            session_handle->max_chunk_size = EVITA_MAX_CHUNK_SIZE;
            session_ctx->cmd_id = pke_para.cmd_id;
            session_ctx->direction = pke_para.u_hdr.hdr_pke.direction;
            if (EVITA_ASYM_SM2 != algorithm_identifier)
            {
                session_ctx->algorithm = pke_para.u_hdr.hdr_pke.algorithm;
                session_ctx->time_stamp = pke_para.u_hdr.hdr_pke.time_stamp;
                if (EVITA_ASYM_RSA == algorithm_identifier)
                {
                    session_ctx->padding = pke_para.u_hdr.hdr_pke.padding;
                    session_ctx->rsa_crt_mode = pke_para.u_hdr.hdr_pke.rsa_crt_mode;
                }
                else
                {;}
            }
            else
            {;}
            session_ctx->key_handle = pke_para.key_handle;
            session_ctx->key_auth_addr = pke_para.key_addr;
            session_ctx->key_auth_size = pke_para.key_size;
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

/**
 * @brief Update the signature operation with a message chunk.
 * @param session_handle Pointer to session handle.
 * @param chunk_size Size of the message chunk.
 * @param chunk_data Pointer to the message chunk data.
 * @return Result code.
 */
ehsm_uint32_t Sign_Update(ehsm_ctx_session_st *session_handle, ehsm_uint32_t chunk_size, const ehsm_uint8_t *chunk_data)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st pke_para = {0};
    cipher_session_st *session_ctx;

    if (NULL == session_handle)
    {
        ret =  EVITA_WRONG_SESSION_HANDLE;
    }
    else if ((session_handle->max_chunk_size < chunk_size) || (session_handle->max_chunk_size == 0U))
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
        if (session_ctx != NULL && ((EVITA_INIT == session_ctx->status) || (EVITA_UPDATE == session_ctx->status)))
        {
            pke_para.cmd_id = session_ctx->cmd_id;
            pke_para.u_hdr.hdr_pke.process_mode = EHSM_UPDATE;
            pke_para.u_hdr.hdr_pke.direction = session_ctx->direction;
            pke_para.u_hdr.hdr_pke.padding = session_ctx->padding;
            pke_para.u_hdr.hdr_pke.algorithm = session_ctx->algorithm;
            pke_para.u_hdr.hdr_pke.rsa_crt_mode = session_ctx->rsa_crt_mode;
            pke_para.u_hdr.hdr_pke.time_stamp = session_ctx->time_stamp;
            pke_para.key_handle = session_ctx->key_handle;
            pke_para.key_addr = session_ctx->key_auth_addr;
            pke_para.key_size = session_ctx->key_auth_size;
            pke_para.input_addr = (ehsm_addr_t)chunk_data;
            pke_para.input_size = chunk_size;
            pke_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
            pke_para.context_size = sizeof(session_ctx->ctx);
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_PKE, &pke_para, EHSM_API_TYPE_EVITA);
            if (EHSM_ERR_SW_SUCCESS == ret)
            {
                session_ctx->status = EVITA_UPDATE;
            }
            else
            {
                eHSM_Mgr_Cipher_Ctx_Free(session_handle->session_id);
            }
            ret = ehsm_evita_convert_ret_code(ret);
        }
    }

    return ret;
}

/**
 * @brief Finish the signature operation and output the signature.
 * @param session_handle Pointer to session handle.
 * @param signature Pointer to signature output structure.
 * @param salt variable stores the address value of salt buffer.
 * @param salt_len variable stores salt buffer length.
 * @return Result code.
 */
ehsm_uint32_t Sign_Finish(ehsm_ctx_session_st *session_handle, signature_st *signature, ehsm_uint32_t salt, ehsm_uint32_t salt_len)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_with_rps_st pke_packet;
    ehsm_cmd_cipher_st pke_para = {0};
    cipher_session_st *session_ctx;

    if (NULL == session_handle)
    {
        ret =  EVITA_WRONG_SESSION_HANDLE;
    }
    else if (signature == NULL)
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((EVITA_INIT == session_ctx->status) || (EVITA_UPDATE == session_ctx->status)))
        {
            pke_para.cmd_id = session_ctx->cmd_id;
            pke_para.u_hdr.hdr_pke.process_mode = EHSM_FINISH;
            pke_para.u_hdr.hdr_pke.direction = session_ctx->direction;
            pke_para.u_hdr.hdr_pke.padding = session_ctx->padding;
            pke_para.u_hdr.hdr_pke.algorithm = session_ctx->algorithm;
            pke_para.u_hdr.hdr_pke.rsa_crt_mode = session_ctx->rsa_crt_mode;
            if (EHSM_USE_TIME_STAMP == session_ctx->time_stamp)
            {
#ifdef CONFIG_EHSM_HW_UTC_TIME
                ret = Get_UTC_Time(&signature->utc_time);
                if (EHSM_ERR_SW_SUCCESS == ret)
                {
                    pke_para.input_addr = (ehsm_addr_t)&signature->utc_time;
                    pke_para.input_size = sizeof(ehsm_utc_time_t);
                    pke_para.u_hdr.hdr_ske.time_stamp = EHSM_USE_TIME_STAMP;
                }
                else
#endif
                {
                    /* If it is failed to get the utc time, the signature will be also continue calculated without timestamp */
                    /* As the result the signature->utc_time will be 0 */
                    pke_para.input_addr = (ehsm_addr_t)0;
                    pke_para.input_size = 0;
                    pke_para.u_hdr.hdr_ske.time_stamp = EHSM_NO_TIME_STAMP;
                    signature->utc_time = 0;
                }
            }
            else
            {
                pke_para.u_hdr.hdr_ske.time_stamp = EHSM_NO_TIME_STAMP;
            }
            pke_para.key_handle = session_ctx->key_handle;
            pke_para.key_addr = session_ctx->key_auth_addr;
            pke_para.key_size = session_ctx->key_auth_size;
            pke_para.sec_input_addr = salt;
            pke_para.sec_input_size = salt_len;
            pke_para.output_addr = (ehsm_addr_t)signature->signature;
            pke_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
            pke_para.context_size = sizeof(session_ctx->ctx);
            pke_packet.req_cipher = pke_para;
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_PKE, &pke_packet, EHSM_API_TYPE_EVITA);
            if (EHSM_ERR_SW_SUCCESS == ret)
            {
                session_ctx->status = EVITA_FINISH;
                if (EHSM_CMD_SM2_SIGN == pke_para.cmd_id)
                {
                    signature->signature_size = 64U;
                }
                else
                {
                    signature->signature_size = pke_packet.output_size;
                }
            }
            else
            {;}
            ret = ehsm_evita_convert_ret_code(ret);
            eHSM_Mgr_Cipher_Ctx_Free(session_handle->session_id);
        }
        else
        {;}
    }
    return ret;
}

/**
 * @brief Initialize the verification operation for asymmetric algorithms.
 * @param algorithm_identifier Asymmetric algorithm identifier.
 * @param hash_algorithm_identifier Hash algorithm identifier.
 * @param padding Padding scheme.
 * @param total_message_length Total message length.
 * @param key_handle Key handle.
 * @param key_authorization_size Size of key authorization value.
 * @param key_authorization_value Pointer to key authorization value.
 * @param signture Pointer to signature structure.
 * @param session_handle Pointer to session handle.
 * @return Result code.
 */
ehsm_uint32_t Verify_Init(ehsm_asym_alg_e algorithm_identifier, ehsm_uint32_t hash_algorithm_identifier, padding_scheme_e padding,
                            ehsm_uint32_t total_message_length, ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size,
                            ehsm_uint8_t *key_authorization_value, signature_st *signture, ehsm_ctx_session_st *session_handle)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st pke_para = {0};
    /* total_message_length will not be used */
    ehsm_uint32_t session_id;
    cipher_session_st *session_ctx;

    /* Check the parameter */
    do
    {
        ret = Evita_Check_Asymmetric_Algorithm(algorithm_identifier, hash_algorithm_identifier);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        if ((padding != EVITA_NOPADDING) && (padding != EVITA_PSASSA_PSS))
        {
            ret = EVITA_GENERAL_ERROR;
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

        if (signture == NULL)
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        else
        {;}
    } while(0U);

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

    /* Prepare the data for the mailbox command */
    if (ret == EVITA_OK)
    {
        if (EVITA_ASYM_SM2 == algorithm_identifier)
        {
            pke_para.cmd_id = EHSM_CMD_SM2_SIGN;
        }
        else if (EVITA_ASYM_ECDSA == algorithm_identifier)
        {
            pke_para.cmd_id = EHSM_CMD_ECDSA;
            pke_para.u_hdr.hdr_pke.algorithm = hash_algorithm_identifier;
        }
        else
        {
            pke_para.cmd_id = EHSM_CMD_RSA_SIGN;
            pke_para.u_hdr.hdr_pke.padding = (ehsm_uint8_t)padding;
            pke_para.u_hdr.hdr_pke.algorithm = hash_algorithm_identifier;
        }
        pke_para.u_hdr.hdr_pke.process_mode = EHSM_START;
        pke_para.u_hdr.hdr_pke.direction = EHSM_SIGN_VERIFICATION;
        pke_para.key_handle = key_handle;
        pke_para.key_addr = (ehsm_addr_t)key_authorization_value;
        pke_para.key_size = key_authorization_size;
        pke_para.output_addr = (ehsm_addr_t)signture->signature;
        pke_para.output_size = signture->signature_size;
        pke_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
        pke_para.context_size = sizeof(session_ctx->ctx);
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_PKE, &pke_para, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            session_handle->max_chunk_size = EVITA_MAX_CHUNK_SIZE;
            session_ctx->cmd_id = pke_para.cmd_id;
            session_ctx->direction = pke_para.u_hdr.hdr_pke.direction;
            if (EVITA_ASYM_SM2 != algorithm_identifier)
            {
                session_ctx->algorithm = pke_para.u_hdr.hdr_pke.algorithm;
                if (EVITA_ASYM_RSA == algorithm_identifier)
                {
                    session_ctx->padding = pke_para.u_hdr.hdr_pke.padding;
                    session_ctx->rsa_crt_mode = pke_para.u_hdr.hdr_pke.rsa_crt_mode;
                }
                else
                {;}
            }
            else
            {;}
            session_ctx->key_handle = pke_para.key_handle;
            session_ctx->key_auth_addr = pke_para.key_addr;
            session_ctx->key_auth_size = pke_para.key_size;
            session_ctx->signature_addr = pke_para.output_addr;
            session_ctx->signature_size = pke_para.output_size;
            session_ctx->utc_time = signture->utc_time;
            session_ctx->status = EVITA_INIT;
            session_handle->session_id = session_id;
        }
        else
        {;}
        ret = ehsm_evita_convert_ret_code(ret);;
    }
    return ret;
}

/**
 * @brief Update the verification operation with a message chunk.
 * @param session_handle Pointer to session handle.
 * @param chunk_size Size of the message chunk.
 * @param chunk_data Pointer to the message chunk data.
 * @return Result code.
 */
ehsm_uint32_t Verify_Update(ehsm_ctx_session_st *session_handle, ehsm_uint32_t chunk_size, const ehsm_uint8_t *chunk_data)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st pke_para = {0};
    cipher_session_st *session_ctx;

    if (NULL == session_handle)
    {
        ret =  EVITA_WRONG_SESSION_HANDLE;
    }
    else if ((session_handle->max_chunk_size < chunk_size) || (session_handle->max_chunk_size == 0U))
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
        if (session_ctx != NULL && ((EVITA_INIT == session_ctx->status) || (EVITA_UPDATE == session_ctx->status)))
        {
            pke_para.cmd_id = session_ctx->cmd_id;
            pke_para.u_hdr.hdr_pke.process_mode = EHSM_UPDATE;
            pke_para.u_hdr.hdr_pke.direction = session_ctx->direction;
            pke_para.u_hdr.hdr_pke.algorithm = session_ctx->algorithm;
            pke_para.u_hdr.hdr_pke.padding = session_ctx->padding;
            pke_para.u_hdr.hdr_pke.rsa_crt_mode = session_ctx->rsa_crt_mode;
            pke_para.key_handle = session_ctx->key_handle;
            pke_para.key_addr = session_ctx->key_auth_addr;
            pke_para.key_size = session_ctx->key_auth_size;
            pke_para.input_addr = (ehsm_addr_t)chunk_data;
            pke_para.input_size = chunk_size;
            pke_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
            pke_para.context_size = sizeof(session_ctx->ctx);
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_PKE, &pke_para, EHSM_API_TYPE_EVITA);
            if (EHSM_ERR_SW_SUCCESS == ret)
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

/**
 * @brief Finish the verification operation and check the signature.
 * @param session_handle Pointer to session handle.
 * @param utc_time_stamp Pointer to UTC time stamp output.
 * @param sign_match Pointer to signature match result.
 * @param salt variable stores the address value of salt buffer.
 * @param salt_len variable stores salt buffer length.
 * @return Result code.
 */
ehsm_uint32_t Verify_Finish(ehsm_ctx_session_st *session_handle, ehsm_utc_time_t *utc_time_stamp, ehsm_bool_t *sign_match,
                                ehsm_uint32_t salt, ehsm_uint32_t salt_len)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st pke_para = {0};
    cipher_session_st *session_ctx;

    if (NULL == session_handle)
    {
        ret =  EVITA_WRONG_SESSION_HANDLE;
    }
    else if (NULL == sign_match)
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((EVITA_INIT == session_ctx->status) || (EVITA_UPDATE == session_ctx->status)))
        {
            pke_para.cmd_id = session_ctx->cmd_id;
            pke_para.u_hdr.hdr_pke.process_mode = EHSM_FINISH;
            pke_para.u_hdr.hdr_pke.direction = session_ctx->direction;
            pke_para.u_hdr.hdr_pke.algorithm = session_ctx->algorithm;
            pke_para.u_hdr.hdr_pke.padding = session_ctx->padding;
            pke_para.u_hdr.hdr_pke.rsa_crt_mode = session_ctx->rsa_crt_mode;
            pke_para.key_handle = session_ctx->key_handle;
            pke_para.key_addr = session_ctx->key_auth_addr;
            pke_para.key_size = session_ctx->key_auth_size;
            pke_para.output_addr = session_ctx->signature_addr;
            pke_para.output_size = session_ctx->signature_size;
            pke_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
            pke_para.context_size = sizeof(session_ctx->ctx);
            pke_para.sec_input_addr = salt;
            pke_para.sec_input_size = salt_len;
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_PKE, &pke_para, EHSM_API_TYPE_EVITA);
            if (EHSM_ERR_SW_SUCCESS == ret)
            {
                session_ctx->status = EVITA_FINISH;
                *sign_match = TRUE;
                if ((session_ctx->utc_time != 0U) && (utc_time_stamp != NULL))
                {
                    *utc_time_stamp = session_ctx->utc_time;
                }
                else
                {;}
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
