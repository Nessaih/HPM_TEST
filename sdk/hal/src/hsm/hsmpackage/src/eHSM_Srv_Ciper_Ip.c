/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#include "eHSM_Srv_Cipher_Ip.h"

#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Srv_CmdReq_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"

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

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

/**
 * @brief Request handler for crypto cipher service.
 *
 * @param para Pointer to cipher parameters.
 * @param req Pointer to command request structure.
 * @return Error code defined by EHSM_ERR_SW_SUCCESS or EHSM_ERR_GENERAL_ERROR.
 */
ehsm_uint32_t srv_crypto_cipher_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = EHSM_ERR_GENERAL_ERROR;
    ehsm_uint32_t cmd_type;
    ehsm_cmd_cipher_st *cipher_packet = NULL; /* Here should be ehsm_key_st in some key related function */
    ehsm_cmd_cipher_st *cipher_para = NULL;
    if ((NULL != para) && (NULL != req))
    {
        cipher_para = (ehsm_cmd_cipher_st *)para;
        cmd_type = (cipher_para->cmd_id >> 8) & 0xF;

        switch (cmd_type)
        {
            case 1:
            case 2:
            {
                req->object_type = CRYPTO_OBJECT_TYPE_SKE;
                break;
            }
            case 3:
            {
                req->object_type = CRYPTO_OBJECT_TYPE_HASH;
                break;
            }
            case 4:
            case 5:
            case 6:
            case 7:
            {
                req->object_type = CRYPTO_OBJECT_TYPE_PKE;
                break;
            }
            case 8:
            {
                req->object_type = CRYPTO_OBJECT_TYPE_KEY;
                break;
            }
            case 9:
            {
                /* TODO: Add the object type of certification */
                break;
            }
            case 10:
            {
                req->object_type = CRYPTO_OBJECT_TYPE_TRNG;
                break;
            }
            default:
                break;
        }
        cipher_packet = (ehsm_cmd_cipher_st *)req->cmd_data;
        cipher_packet->cmd_id = cipher_para->cmd_id;
        cipher_packet->u_hdr = cipher_para->u_hdr;
        cipher_packet->key_handle = cipher_para->key_handle;
        cipher_packet->key_addr = cipher_para->key_addr;
        cipher_packet->key_size = cipher_para->key_size;
        cipher_packet->input_addr = cipher_para->input_addr;
        cipher_packet->input_size = cipher_para->input_size;
        cipher_packet->output_addr = cipher_para->output_addr;
        cipher_packet->output_size = cipher_para->output_size;
        cipher_packet->sec_input_addr = cipher_para->sec_input_addr;
        cipher_packet->sec_input_size = cipher_para->sec_input_size;
        cipher_packet->context_addr = cipher_para->context_addr;
        cipher_packet->context_size = cipher_para->context_size;
        ret = EHSM_ERR_SW_SUCCESS;
    }
    else
    {;}
    return ret;
}

/**
 * @brief Handles response data for cipher operations.
 *
 * @param para Pointer to cipher packet with response.
 * @param req Pointer to command request structure.
 */
static void srv_rps_data_handler(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint8_t process_mode;
    ehsm_uint8_t dir;
    ehsm_bool_t get_data_flag;
    ehsm_cmd_cipher_with_rps_st *cipher_packet = para;
    ehsm_uint32_t cmd_id = cipher_packet->req_cipher.cmd_id;

    switch(cmd_id)
    {
        case EHSM_CMD_SYM_CIPHER:
        {
            process_mode = cipher_packet->req_cipher.u_hdr.hdr_ske.process_mode;
            if (process_mode != EHSM_START)
            {
                get_data_flag = TRUE;
            }
            else
            {
                get_data_flag = FALSE;
            }
            break;
        }
        case EHSM_CMD_RSA_SIGN:
        case EHSM_CMD_ECDSA:
        {
            dir = cipher_packet->req_cipher.u_hdr.hdr_pke.direction;
            process_mode = cipher_packet->req_cipher.u_hdr.hdr_ske.process_mode;
            if (((EHSM_FINISH == process_mode) || (EHSM_ONEPASS == process_mode)) && (EHSM_SIGN_GENERATION == dir))
            {
                get_data_flag = TRUE;
            }
            else
            {
                get_data_flag = FALSE;
            }
            break;
        }

        case EHSM_CMD_RSA_CIPHER:
        case EHSM_CMD_ECIES:
        {
            get_data_flag = TRUE;
            break;
        }
        default:
        {
            get_data_flag = FALSE;
            break;
        }
    }

    if (TRUE == get_data_flag)
        {
        cipher_packet->output_size = *(ehsm_uint32_t *)req->rps_data;
        }
        else
        {;}
}

/* srv_crypto_cipher_rsphdl should only run in sync mode */
/**
 * @brief Response handler for crypto cipher service.
 *
 * @param para Pointer to cipher parameters.
 * @param req Pointer to command request structure.
 * @return Error code from request or EHSM_ERR_PARAM_ERROR.
 */
ehsm_uint32_t srv_crypto_cipher_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0;
    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        srv_rps_data_handler(para, req);
        ret = req->error_code;
    }
    return ret;
}
/**
 * @brief Service structure for SKE cipher operations.
 */
ehsm_service_st srv_crypto_ske = {
    .service_id = EHSM_SRV_EXTENDED_CRYPTO_SKE,
    .reqhdl = srv_crypto_cipher_reqhdl,
    .rsphdl = srv_crypto_cipher_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};
