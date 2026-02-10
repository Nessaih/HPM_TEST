/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include <string.h>
#include "eHSM_If_Evita_Ip.h"

#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Srv_CmdReq_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Mailbox_CmdId_Ip.h"

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
#ifdef CONFIG_EHSM_HW_COUNTER
static ehsm_uint32_t srv_counter_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_int32_t ret;
    ehsm_mailbox_req_st *counter_packet;
    ehsm_mailbox_req_st *counter_para;
    if ((NULL != para) && (NULL != req))
    {
        counter_para = (ehsm_mailbox_req_st *)para;
        counter_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        counter_packet->cmd_id = counter_para->cmd_id;
        counter_packet->ehsm_cmd.counter.counter_id = counter_para->ehsm_cmd.counter.counter_id;
        (void)System_Memcpy(counter_packet->ehsm_cmd.counter.counter_inc, counter_para->ehsm_cmd.counter.counter_inc, HOST_ADDRESS_SIZE);
        counter_packet->ehsm_cmd.counter.auth_size = counter_para->ehsm_cmd.counter.auth_size;
        (void)System_Memcpy(counter_packet->ehsm_cmd.counter.auth_value, counter_para->ehsm_cmd.counter.auth_value, HOST_ADDRESS_SIZE);
        (void)System_Memcpy(counter_packet->ehsm_cmd.counter.context_addr, counter_para->ehsm_cmd.counter.context_addr, HOST_ADDRESS_SIZE);
        counter_packet->ehsm_cmd.counter.context_size = counter_para->ehsm_cmd.counter.context_size;
        ret = EHSM_ERR_SW_SUCCESS;
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

static ehsm_uint32_t srv_counter_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_int32_t ret;
    ehsm_uint32_t *p_id;
    ehsm_counter_value_st *p_counrter;
    ehsm_addr_t tmp_addr;

    ehsm_uint32_t *p_tmp;
    ehsm_mailbox_req_st *counter_para = (ehsm_mailbox_req_st *)para;
    if ((NULL != para) && (NULL != req))
    {
        if (counter_para->cmd_id == EHSM_CMD_CREATE_COUNTER)
        {
            p_id = (ehsm_uint32_t *)counter_para->ehsm_cmd.counter.context_size;
            p_tmp = (ehsm_uint32_t *)req->rps_data;
            p_tmp += 2; /* 12-15 byte for counter_id */
            *p_id = *p_tmp;
        }
        else
        {;}

        if (counter_para->cmd_id != EHSM_CMD_DELETE_COUNTER)
        {
            tmp_addr = *(ehsm_addr_t *)counter_para->ehsm_cmd.counter.context_addr;
            p_counrter = (ehsm_counter_value_st *)tmp_addr;
            p_tmp = (ehsm_uint32_t *)req->rps_data;
            p_counrter->low_word = *p_tmp; /* 4-7 byte for low_word */
            p_tmp += 1; /* 8-11 byte for high_word */
            p_counrter->high_word = *p_tmp;
        }
        else
        {;}
        ret = req->error_code;
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

ehsm_service_st srv_counter = {
    .service_id = EHSM_SRV_COUNTER,
    .reqhdl = srv_counter_reqhdl,
    .rsphdl = srv_counter_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

#endif
