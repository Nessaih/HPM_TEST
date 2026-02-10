/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include "eHSM_Types_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Debug_Ip.h"
#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_If_Evita_ErrCode_Ip.h"
#include "eHSM_If_Evita_Ip.h"

#include "eHSM_Mgr_Ctx_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
    /* HASH_type     msg_block_sz   digest_sz */
#define HASH_SIZE                        \
    X(EHSM_SM3,         64U,        32U) \
    X(EHSM_MD5,         64U,        16U) \
    X(EHSM_SHA1,        64U,        20U) \
    X(EHSM_SHA256,      64U,        32U) \
    X(EHSM_SHA224,      64U,        28U) \
    X(EHSM_SHA384,      128U,       48U) \
    X(EHSM_SHA512,      128U,       64U) \
    X(EHSM_SHA512_224,  128U,       28U) \
    X(EHSM_SHA512_256,  128U,       32U) \
    X(EHSM_SHA3_224,    144U,       28U) \
    X(EHSM_SHA3_256,    136U,       32U) \
    X(EHSM_SHA3_384,    104U,       48U) \
    X(EHSM_SHA3_512,    72U,        64U) \
    X(EHSM_INVALID_ALG, 0U,         0U)

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
static ehsm_uint32_t Hash_Get_Block_Size(ehsm_uint8_t alg)
{
    ehsm_uint32_t msg_block_sz;

    switch(alg)
    {
        #define X(hash_type, block_sz, sz)  \
            case hash_type:                 \
            {                               \
                msg_block_sz = block_sz;    \
                break;                      \
            }
        HASH_SIZE
        #undef X
        default:
        {
            msg_block_sz = 0U;
            break;
        }
    }
    return msg_block_sz;
}

static ehsm_uint32_t Hash_Check_Algorithm(ehsm_uint32_t alg)
{
    ehsm_uint32_t ret;
    switch(alg)
    {
        #define X(hash_type, block_sz, sz)  \
            case hash_type:                 \
            {                               \
                ret = EVITA_OK;             \
                break;                      \
            }
        HASH_SIZE
        #undef X
        default:
        {
            ret = EVITA_ALGORITHM_ERROR;
            break;
        }
    }
    return ret;
}

static ehsm_uint8_t Hmac_Get_Direction(hash_mode_e hash_mode)
{
    ehsm_uint8_t dir;
    switch (hash_mode)
    {
        case EVITA_HMAC_SIGN:
        case EVITA_HMAC_TIMESTAMP_SIGN:
        {
            dir = EHSM_MAC_GENERATION;
            break;
        }
        case EVITA_HMAC_VERIFY:
        {
            dir = EHSM_MAC_VERIFICATION;
            break;
        }
        default:
        {
            dir = EHSM_INVALID_DIR;
            break;
        }
    }
    return dir;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
ehsm_uint32_t Hash_Init(ehsm_uint32_t algorithm_identifier, hash_mode_e hash_mode, ehsm_uint32_t key_handle,
                        ehsm_uint32_t key_authorization_size, ehsm_uint8_t *key_authorization_value,
                        ehsm_ctx_session_st *session_handle)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st hash_para = {0};
    ehsm_uint32_t session_id;
    cipher_session_st *session_ctx;

    /* Check the parameter */
    do
    {
        ret = Hash_Check_Algorithm(algorithm_identifier);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        if (hash_mode != EVITA_HASH)
        {
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
        }
        else
        {
             /* For Hash, key_handle should be 0. */
            if (key_handle != 0)
            {
            }
            else
            {;}
        }
    } while(0U);

    /* Try to get the context session */
    if (ret == EVITA_OK)
    {
        if (session_handle != NULL)
        {
            ret = eHSM_Mgr_Cipher_Ctx_Get_Free(&session_id, &session_ctx);
            if (EVITA_OK == ret)
            {
                ret = ehsm_init_block_mgr(&session_ctx->ctx_block_mgr, Hash_Get_Block_Size(algorithm_identifier));
            }
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
        hash_para.cmd_id = EHSM_CMD_HASH;
        hash_para.u_hdr.hdr_ske.process_mode = EHSM_START;
        if (key_handle != 0)
        {
            /* In hash mode, direction can be invalid */
            hash_para.u_hdr.hdr_ske.direction = Hmac_Get_Direction(hash_mode);
            hash_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
            hash_para.key_handle = key_handle;
            hash_para.key_addr = (ehsm_addr_t)key_authorization_value;
            hash_para.key_size = key_authorization_size;
            hash_para.u_hdr.hdr_ske.time_stamp = (hash_mode == EVITA_HMAC_TIMESTAMP_SIGN ? EHSM_USE_TIME_STAMP : EHSM_NO_TIME_STAMP);
        }
        else
        {;}

        hash_para.u_hdr.hdr_ske.algorithm = algorithm_identifier;
        hash_para.context_addr = (ehsm_addr_t)session_ctx->ctx;
        hash_para.context_size = sizeof(session_ctx->ctx);
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_HASH, &hash_para, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            session_ctx->is_hmac = (hash_mode == EVITA_HASH) ? 0 : 1;
            if (session_ctx->is_hmac == 1)
            {
                session_ctx->direction = hash_para.u_hdr.hdr_ske.direction;
                session_ctx->time_stamp = hash_para.u_hdr.hdr_ske.time_stamp;
            }
            else
            {;}
            session_ctx->algorithm = algorithm_identifier;
            session_ctx->key_handle = key_handle;
            session_ctx->status = EVITA_INIT;
            session_handle->block_sz = Hash_Get_Block_Size(algorithm_identifier);
            session_handle->max_chunk_size = EVITA_MAX_CHUNK_SIZE;
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
ehsm_uint32_t Hash_Update(ehsm_ctx_session_st *session_handle, ehsm_uint32_t chunk_size, const ehsm_uint8_t *chunk_data)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st hash_para = {0};
    cipher_session_st *session_ctx = NULL;

    if ((NULL == session_handle) || (0U == session_handle->max_chunk_size))
    {
        ret = EVITA_WRONG_SESSION_HANDLE;
    }
    else if (chunk_size > session_handle->max_chunk_size)
    {
        ret = EVITA_WRONG_CHUNK_SIZE;
    }
    else if ((NULL == chunk_data) && (0U != chunk_size))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((EVITA_INIT == session_ctx->status) || (EVITA_UPDATE == session_ctx->status)))
        {
            hash_para.cmd_id = EHSM_CMD_HASH;
            hash_para.u_hdr.hdr_ske.process_mode = EHSM_UPDATE;
            hash_para.u_hdr.hdr_ske.algorithm = session_ctx->algorithm;
            if (session_ctx->is_hmac == 1)
            {
                hash_para.u_hdr.hdr_ske.direction = session_ctx->direction;
                hash_para.u_hdr.hdr_ske.time_stamp = session_ctx->time_stamp;
                hash_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
                hash_para.key_handle = session_ctx->key_handle;
            }

            hash_para.input_addr = (ehsm_addr_t)chunk_data;
            hash_para.input_size = chunk_size;
            hash_para.context_addr = (ehsm_addr_t)(session_ctx->ctx);
            hash_para.context_size = sizeof(session_ctx->ctx);
            do
            {
                ret = ehsm_disjuction_updata(EHSM_SRV_EXTENDED_CRYPTO_HASH, &hash_para, &session_ctx->ctx_block_mgr);
                if (EHSM_ERR_SW_SUCCESS != ret)
                {
                    break;
                }
            } while (hash_para.input_size > 0);
            if (ret == EHSM_ERR_SW_SUCCESS)
            {
                session_ctx->status = EVITA_UPDATE;
            }
            else
            {;}
            ret = ehsm_evita_convert_ret_code(ret);
        }
        else
        {;}
    }

    return ret;
}

ehsm_uint32_t Hash_Finish(ehsm_ctx_session_st *session_handle, hash_hmac_st *hash_hmac, ehsm_bool_t *hmac_match)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_cmd_cipher_st hash_para = {0};
    cipher_session_st *session_ctx = NULL;

    if (NULL == session_handle)
    {
        ret = EVITA_WRONG_SESSION_HANDLE;
    }
    else if (hash_hmac == NULL)
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        session_ctx = eHSM_Mgr_Cipher_Ctx_Find(session_handle->session_id);
        if (session_ctx != NULL && ((EVITA_INIT == session_ctx->status) || (EVITA_UPDATE == session_ctx->status)))
        {
            hash_para.cmd_id = EHSM_CMD_HASH;
            hash_para.u_hdr.hdr_ske.process_mode = EHSM_FINISH;
            hash_para.u_hdr.hdr_ske.algorithm = session_ctx->algorithm;
            ret = ehsm_disjuction_finish(&session_ctx->ctx_block_mgr, &hash_para.input_addr, &hash_para.input_size);
            if (session_ctx->is_hmac == 1)
            {
                hash_para.u_hdr.hdr_ske.direction = session_ctx->direction;
                hash_para.u_hdr.hdr_ske.key_type = EHSM_CMD_CIPHER_KEY_TYPE_EVITA;
                hash_para.key_handle = session_ctx->key_handle;
#ifdef CONFIG_EHSM_HW_UTC_TIME
                if (EHSM_USE_TIME_STAMP == session_ctx->time_stamp)
                {
                    ret = Get_UTC_Time(&hash_hmac->utc_time);
                    if (EHSM_ERR_SW_SUCCESS == ret)
                    {
                        (void)System_Memcpy((ehsm_uint8_t *)hash_para.input_addr + hash_para.input_size, &hash_hmac->utc_time,
                               sizeof(ehsm_uint32_t));
                        hash_para.input_size += sizeof(ehsm_uint32_t);
                        hash_para.u_hdr.hdr_ske.time_stamp = EHSM_USE_TIME_STAMP;
                    }
                    else
                    {
                        /* If it is failed to get the utc time, the hash will be also continue calculated without timestamp */
                        /* As the result the hash_hmac->utc_time will be 0 */
                        hash_para.u_hdr.hdr_ske.time_stamp = EHSM_NO_TIME_STAMP;
                        hash_hmac->utc_time = 0;
                    }
                }
                else
#endif
                {
                    hash_para.u_hdr.hdr_ske.time_stamp = EHSM_NO_TIME_STAMP;
                }
            }

            /* Input has been initialized to 0 */

            hash_para.output_addr = (ehsm_addr_t)hash_hmac->hash_hmac;
            hash_para.output_size = hash_hmac->hash_hmac_size;
            hash_para.context_addr = (ehsm_addr_t)(session_ctx->ctx);
            hash_para.context_size = sizeof(session_ctx->ctx);
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_CRYPTO_HASH, &hash_para, EHSM_API_TYPE_EVITA);
            if ((hash_para.key_handle != 0) && (EHSM_MAC_VERIFICATION == hash_para.u_hdr.hdr_ske.direction))
            {
                if (hmac_match != NULL)
                {
                    *hmac_match = (EHSM_ERR_SW_SUCCESS == ret) ? TRUE : FALSE;
                }
                else
                {
                    ret = EVITA_GENERAL_ERROR;
                }
            }
            else
            {;}
            ret = ehsm_evita_convert_ret_code(ret);
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
