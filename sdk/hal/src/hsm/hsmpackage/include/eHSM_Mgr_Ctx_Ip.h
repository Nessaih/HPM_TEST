#ifndef EHSM_MGR_CTX_IP_H
#define EHSM_MGR_CTX_IP_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_If_Evita_ErrCode_Ip.h"
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"
#include "Std_Types.h"
#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define EHSM_ERR_CTX_MGR_BUFFER_DATA_VALID      (0xdeadbee)
#define EHSM_ERR_CTX_MGR_DATA_NOT_READY         (0xbadcafe)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef struct ehsm_ctx_block_mgr
{
    ehsm_uint32_t block_sz;
    ehsm_uint32_t remain_data_sz;
    ehsm_uint8_t block_buf[144 + 4];
} ehsm_ctx_block_mgr_st;

typedef struct ehsm_aead_data_ptr
{
    ehsm_cmd_aead_ptr_st input_data;
    ehsm_cmd_aead_ptr_st output_data;
} ehsm_aead_data_ptr_st;
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/
typedef struct
{
    ehsm_uint32_t max_chunk_size;
    ehsm_uint32_t block_sz;
    ehsm_uint32_t session_id;
} ehsm_ctx_session_st;

typedef struct
{
    ehsm_uint32_t cmd_id;   //Used in PKE
    ehsm_uint8_t direction;
    ehsm_uint8_t padding;
    ehsm_uint8_t algorithm;
    ehsm_uint8_t cipher_mode;
    ehsm_uint8_t time_stamp;
    ehsm_uint8_t is_hmac;
    ehsm_uint8_t mac_bytes;
    ehsm_uint8_t rsa_crt_mode;
    ehsm_uint8_t key_type;
    ehsm_uint32_t key_handle;
    ehsm_uint32_t key_auth_addr;
    ehsm_uint32_t key_auth_size;
    ehsm_uint32_t signature_addr; //Used in EVITA verification finish for getting the address of signature
    ehsm_uint32_t signature_size;
    ehsm_uint32_t job_id;
    ehsm_aead_data_ptr_st aead_data;
    ehsm_utc_time_t utc_time;
    session_status_e status;
    ehsm_uint8_t ctx[EHSM_CONTEXT_SIZE];
    ehsm_ctx_block_mgr_st ctx_block_mgr;
} cipher_session_st;

typedef ehsm_uint8_t ehsm_asym_alg_e;
#define EVITA_ASYM_SM2    0U
#define EVITA_ASYM_ECDSA  1U
#define EVITA_ASYM_RSA    2U

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

ehsm_uint32_t eHSM_Mgr_Cipher_Ctx_Get_Free(ehsm_uint32_t *session_id, cipher_session_st **ctx);

cipher_session_st *eHSM_Mgr_Cipher_Ctx_Find(ehsm_uint32_t index);

void eHSM_Mgr_Cipher_Ctx_Free(ehsm_uint32_t index);

ehsm_uint32_t ehsm_cipher_ctx_data_check(ehsm_ctx_block_mgr_st *block_mgr, ehsm_uint8_t **input_data,
                                         ehsm_uint32_t *input_sz, ehsm_bool_t keep_last_block);

ehsm_uint32_t ehsm_disjuction_updata(ehsm_cmd_ext_type_e service_id, void *param, ehsm_ctx_block_mgr_st *block_mgr);

ehsm_uint32_t ehsm_init_block_mgr(ehsm_ctx_block_mgr_st *block_mgr, ehsm_uint32_t block_sz);

ehsm_uint32_t ehsm_disjuction_finish(ehsm_ctx_block_mgr_st *block_mgr, ehsm_addr_t *input_addr, ehsm_uint32_t *input_sz);
#endif
