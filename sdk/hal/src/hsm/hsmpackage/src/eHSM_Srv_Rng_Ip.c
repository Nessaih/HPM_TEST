/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Com_Struct_Ip.h"
#include "eHSM_Srv_CmdReq_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Srv_Cipher_Ip.h"

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
 ehsm_service_st srv_crypto_randomgenerate = {
    .service_id = EHSM_SRV_CRYPTO_RANDOMGENERATE,
    .reqhdl = srv_crypto_cipher_reqhdl,
    .rsphdl = srv_crypto_cipher_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

 /***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static ehsm_uint32_t rng_init_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t rng_init_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_rng_init = {
    .service_id = EHSM_SRV_RNG_INIT,
    .reqhdl = rng_init_reqhdl,
    .rsphdl = rng_init_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t rng_init_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    return EHSM_ERR_SW_SUCCESS;
}

static ehsm_uint32_t rng_init_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    return EHSM_ERR_SW_SUCCESS;
}
static ehsm_uint32_t rng_extend_seed_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t rng_extend_seed_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_rng_extend_seed = {
    .service_id = EHSM_SRV_RNG_EXTEND_SEED,
    .reqhdl = rng_extend_seed_reqhdl,
    .rsphdl = rng_extend_seed_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t rng_extend_seed_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    return EHSM_ERR_SW_SUCCESS;
}

static ehsm_uint32_t rng_extend_seed_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    return EHSM_ERR_SW_SUCCESS;
}

static ehsm_uint32_t srv_crypto_randomseed_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t srv_crypto_randomseed_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_crypto_randomseed = {
    .service_id = EHSM_SRV_CRYPTO_RANDOMSEED,
    .reqhdl = srv_crypto_randomseed_reqhdl,
    .rsphdl = srv_crypto_randomseed_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t srv_crypto_randomseed_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    return EHSM_ERR_SW_SUCCESS;
}

static ehsm_uint32_t srv_crypto_randomseed_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    return EHSM_ERR_SW_SUCCESS;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
