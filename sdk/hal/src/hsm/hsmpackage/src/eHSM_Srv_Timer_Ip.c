/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#ifdef CONFIG_EHSM_HW_UTC_TIME
#include <string.h>

#include "eHSM_If_Evita_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Srv_CmdReq_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Mailbox_CmdId_Ip.h"

static ehsm_uint32_t srv_timer_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_int32_t ret;
    ehsm_mailbox_req_st *timer_packet;
    ehsm_mailbox_req_st *timer_para;
    ehsm_uint8_t i;
    if ((NULL != para) && (NULL != req))
    {
        timer_para = (ehsm_mailbox_req_st *)para;
        timer_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        timer_packet->cmd_id = timer_para->cmd_id;
        if (timer_para->cmd_id != EHSM_CMD_GET_CHALLENGE)
        {
            timer_packet->ehsm_cmd.timer.utc_time = timer_para->ehsm_cmd.timer.utc_time;
            timer_packet->ehsm_cmd.timer.key_handle = timer_para->ehsm_cmd.timer.key_handle;
            (void)System_Memcpy(timer_packet->ehsm_cmd.timer.key_auth_addr, timer_para->ehsm_cmd.timer.key_auth_addr, HOST_ADDRESS_SIZE);
            timer_packet->ehsm_cmd.timer.key_auth_size = timer_para->ehsm_cmd.timer.key_auth_size;
            (void)System_Memcpy(timer_packet->ehsm_cmd.timer.input_addr, timer_para->ehsm_cmd.timer.input_addr, HOST_ADDRESS_SIZE);
            timer_packet->ehsm_cmd.timer.input_size = timer_para->ehsm_cmd.timer.input_size;
            (void)System_Memcpy(timer_packet->ehsm_cmd.timer.output_addr, timer_para->ehsm_cmd.timer.output_addr, HOST_ADDRESS_SIZE);
            timer_packet->ehsm_cmd.timer.output_size = timer_para->ehsm_cmd.timer.output_size;
            (void)System_Memcpy(timer_packet->ehsm_cmd.timer.context_addr, timer_para->ehsm_cmd.timer.context_addr, HOST_ADDRESS_SIZE);
            timer_packet->ehsm_cmd.timer.context_size = timer_para->ehsm_cmd.timer.context_size;
        }
        else
        {
            for(i = 0; i < 4; i++)
            {
                timer_packet->ehsm_cmd.get_challenge.output_addr[i] = timer_para->ehsm_cmd.get_challenge.output_addr[i];
            }
            timer_packet->ehsm_cmd.get_challenge.type = timer_para->ehsm_cmd.get_challenge.type;
        }
        ret = EHSM_ERR_SW_SUCCESS;
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

static ehsm_uint32_t srv_timer_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_int32_t ret;
    ehsm_uint32_t *p_data;
    ehsm_uint32_t *p_tmp;
    ehsm_addr_t p_addr;
    ehsm_mailbox_req_st *timer_para = (ehsm_mailbox_req_st *)para;
    if ((NULL != para) && (NULL != req))
    {
        if ((timer_para->cmd_id == EHSM_CMD_CHECK_TIMER) || (timer_para->cmd_id == EHSM_CMD_GET_UTC_TIMER))
        {
            p_addr = *(ehsm_addr_t *)timer_para->ehsm_cmd.timer.context_addr;
            p_data = (ehsm_uint32_t *)p_addr;
            p_tmp = (ehsm_uint32_t *)req->rps_data;
            *p_data = *p_tmp;
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

ehsm_service_st srv_timer = {
    .service_id = EHSM_SRV_TIMER,
    .reqhdl = srv_timer_reqhdl,
    .rsphdl = srv_timer_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};
#endif

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
