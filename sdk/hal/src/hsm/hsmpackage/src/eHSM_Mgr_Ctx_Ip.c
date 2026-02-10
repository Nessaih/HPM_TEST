/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#include <string.h>

#include "eHSM_Mgr_Ctx_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"

#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_If_Evita_ErrCode_Ip.h"
#include "eHSM_Exclusive_Area.h"
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
//static cipher_session_st *g_cipher_session = (cipher_session_st *)0x1FFE5800;
//atc modify
static ehsm_uint32_t cipher_session_st_buf[sizeof(cipher_session_st)/sizeof(ehsm_uint32_t)];
static cipher_session_st *g_cipher_session = (cipher_session_st *)cipher_session_st_buf;

static ehsm_uint32_t g_cipher_session_status[CONFIG_EHSM_ARCH_V_REQ_SKE_MAX_SIZE] = {0};

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
ehsm_uint32_t eHSM_Mgr_Cipher_Ctx_Get_Free(ehsm_uint32_t *session_id, cipher_session_st **ctx)
{
    ehsm_uint32_t ret;
    ehsm_uint32_t index;

    ret = EVITA_GENERAL_ERROR;

    Exclusive_area_enter();
    if (session_id != NULL && ctx != NULL)
    {
        for (index = 0; index < CONFIG_EHSM_ARCH_V_REQ_SKE_MAX_SIZE; ++index)
        {
            if (g_cipher_session_status[index] == 0)
            {
                g_cipher_session_status[index] = 1;
                *session_id = index;
                *ctx = &g_cipher_session[index];
                ret = EVITA_OK;
                break;
            }
            else
            {;}
        }

        if (index >= CONFIG_EHSM_ARCH_V_REQ_SKE_MAX_SIZE)
        {
            ret = EVITA_ALL_SESSIONS_OCCUPIED;
        }
    }
    else
    {;}
    Exclusive_area_exit();

    return ret;
}

cipher_session_st *eHSM_Mgr_Cipher_Ctx_Find(ehsm_uint32_t index)
{
    cipher_session_st *ctx = NULL;

    Exclusive_area_enter();
    if ((index < CONFIG_EHSM_ARCH_V_REQ_SKE_MAX_SIZE) && (g_cipher_session_status[index] == 1))
    {
        ctx = &g_cipher_session[index];
    }
    else
    {;}
    Exclusive_area_exit();

    return ctx;
}

void eHSM_Mgr_Cipher_Ctx_Free(ehsm_uint32_t index)
{
    Exclusive_area_enter();
    if (index < CONFIG_EHSM_ARCH_V_REQ_SKE_MAX_SIZE)
    {
        (void)System_Memset(&g_cipher_session[index], 0, sizeof(cipher_session_st));
        g_cipher_session_status[index] = 0;
    }
    else
    {;}
    Exclusive_area_exit();
}

ehsm_uint32_t ehsm_cipher_ctx_data_check(ehsm_ctx_block_mgr_st *block_mgr, ehsm_uint8_t **input_data,
                                         ehsm_uint32_t *input_sz, ehsm_bool_t keep_last_block)
{
    ehsm_uint32_t ret;
    ehsm_uint32_t sz;
    if ((NULL != block_mgr) && (0U != block_mgr->block_sz))
    {
        if (0U == block_mgr->remain_data_sz)
        {
            sz = *input_sz % block_mgr->block_sz;
            if (0U == sz)
            {
                ret = EHSM_ERR_SW_SUCCESS;
            }
            else
            {
                (void)System_Memcpy(block_mgr->block_buf, *input_data + *input_sz - sz, sz);
                block_mgr->remain_data_sz += sz;
                *input_sz -= sz;
                if (0U != *input_sz)
                {
                    ret = EHSM_ERR_SW_SUCCESS;
                }
                else
                {
                    ret = EHSM_ERR_CTX_MGR_DATA_NOT_READY;
                }
            }
        }
        else
        {
            sz = block_mgr->block_sz - block_mgr->remain_data_sz;
            if (*input_sz >= sz)
            {
                (void)System_Memcpy(block_mgr->block_buf + block_mgr->remain_data_sz, *input_data, sz);
                block_mgr->remain_data_sz += sz;
                *input_sz -= sz;
                *input_data += sz;
                ret = EHSM_ERR_CTX_MGR_BUFFER_DATA_VALID;
            }
            else
            {
                (void)System_Memcpy(block_mgr->block_buf + block_mgr->remain_data_sz, *input_data, *input_sz);
                block_mgr->remain_data_sz += *input_sz;
                *input_sz = 0U;
                ret = EHSM_ERR_CTX_MGR_DATA_NOT_READY;
            }
        }

        //It should keep a block to deal with in final stage
        if (keep_last_block)
        {
            if (0U == block_mgr->remain_data_sz)
            {
                if (*input_sz > block_mgr->block_sz)
                {
                    (void)System_Memcpy(block_mgr->block_buf, *input_data + *input_sz - block_mgr->block_sz, block_mgr->block_sz);
                    block_mgr->remain_data_sz = block_mgr->block_sz;
                    *input_sz -= block_mgr->block_sz;
                }
                else if (*input_sz > 0)
                {
                    (void)System_Memcpy(block_mgr->block_buf, *input_data, *input_sz);
                    block_mgr->remain_data_sz = *input_sz;
                    *input_sz = 0U;
                    ret = EHSM_ERR_CTX_MGR_DATA_NOT_READY;
                }
            }
            else
            {;}
        }
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

ehsm_uint32_t ehsm_init_block_mgr(ehsm_ctx_block_mgr_st *block_mgr, ehsm_uint32_t block_sz)
{
    ehsm_uint32_t ret;
    if ((NULL != block_mgr) && (0U != block_sz))
    {
        block_mgr->block_sz = block_sz;
        block_mgr->remain_data_sz = 0U;
        ret = EHSM_ERR_SW_SUCCESS;
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

ehsm_uint32_t ehsm_disjuction_updata(ehsm_cmd_ext_type_e service_id, void *param, ehsm_ctx_block_mgr_st *block_mgr)
{
    ehsm_uint32_t ret;
    ehsm_cmd_cipher_st *cmd = param;
    ehsm_uint32_t total_sz = cmd->input_size;
    ehsm_addr_t tmp_addr;
    ehsm_bool_t keep_last_block = FALSE;
    if (((EHSM_CMD_MAC == cmd->cmd_id) && (EHSM_CBC_MAC_MODE == cmd->u_hdr.hdr_ske.cipher_mode))
        || ((EHSM_CMD_SYM_CIPHER == cmd->cmd_id) && (EHSM_NOPADDING != cmd->u_hdr.hdr_ske.padding)))
    {
        keep_last_block = TRUE;
    }

    if (0U != total_sz)
    {
        ret = ehsm_cipher_ctx_data_check(block_mgr, (ehsm_uint8_t **)&cmd->input_addr, &cmd->input_size, keep_last_block);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            ret = ehsm_process_sync_service(service_id, param, EHSM_API_TYPE_EVITA);
            if (EHSM_ERR_SW_SUCCESS == ret)
            {
                cmd->input_size = 0U;
            }
        }
        else if (EHSM_ERR_CTX_MGR_BUFFER_DATA_VALID == ret)
        {
            tmp_addr = cmd->input_addr;
            total_sz = cmd->input_size;
            cmd->input_addr = (ehsm_addr_t)block_mgr->block_buf;
            cmd->input_size = block_mgr->block_sz;
            ret = ehsm_process_sync_service(service_id, param, EHSM_API_TYPE_EVITA);
            if (EHSM_ERR_SW_SUCCESS == ret)
            {
                //TODO: Add the AEAD
                if (EHSM_CMD_SYM_CIPHER == cmd->cmd_id)
                {
                    cmd->output_addr += cmd->input_size;
                    cmd->output_size -= cmd->input_size;
                }
                cmd->input_addr = tmp_addr;
                cmd->input_size = total_sz;
                block_mgr->remain_data_sz = 0U;
                (void)System_Memset(block_mgr->block_buf, 0, block_mgr->block_sz);
            }
        }
        else if (EHSM_ERR_CTX_MGR_DATA_NOT_READY == ret)
        {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }
    else
    {
        ret = EHSM_ERR_SW_SUCCESS;
    }
    return ret;
}

ehsm_uint32_t ehsm_disjuction_finish(ehsm_ctx_block_mgr_st *block_mgr, ehsm_addr_t *input_addr, ehsm_uint32_t *input_sz)
{
    ehsm_uint32_t ret;
    if ((NULL != block_mgr) && (NULL != input_addr) && (NULL != input_sz))
    {
        *input_addr = (ehsm_addr_t)block_mgr->block_buf;
        if (block_mgr->remain_data_sz != 0U)
        {
            *input_sz = block_mgr->remain_data_sz;
        }
        else
        {
            *input_sz = 0U;
        }
        ret = EHSM_ERR_SW_SUCCESS;
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}
