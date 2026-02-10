/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"
#include "eHSM_Dspt_lp.h"

#include "eHSM_Srv_CmdReq_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Com_Struct_Ip.h"
#include "eHSM_Debug_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Compt_List.h"
#include "eHSM_Mailbox_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Dspt_CryObj_Ip.h"
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
extern ehsm_uint32_t ptest_time_counting_get_state(ehsm_uint32_t time_id);
extern void ptest_time_counting_end(ehsm_uint32_t time_id);

ehsm_uint32_t ehsm_submit_cmd_req(ehsm_cmd_req_st *cmd)
{
    ehsm_uint32_t ret = 0;

    if ((NULL == cmd) || ((cmd->object_type >= CRYPTO_OBJECT_TYPE_MAX) || 
        (cmd->channel >= MAILBOX_CHANNE_MAX)))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else if (cmd->req_type == EHSM_CMD_REQ_TYPE_ASYNC)
    {
        /* only cmd of general service support ASYNC mode*/
        if (cmd->channel == MAILBOX_CHANNE_GENERAL_SERVICE)
        {
            /*[SWS_Crypto_00031] [ If Crypto_ProcessJob() is called, when the queue is empty and the Crypto Driver Object
            is not busy the Job shall switch to the state ‘active’ and execute the crypto primitive.]()*/
            if (TRUE == ehsm_crypto_object_is_free(cmd->object_type))
            {
                ret = ehsm_add_cmd_to_sent_queue(cmd);
                if (ret == EHSM_ERR_SW_SUCCESS)
                {
                    ret = ehsm_mbox_send_cmd(cmd);
                }
                else
                {
                    ret = ehsm_add_cmd_to_priority_queue(cmd);
                }
            }
            else
            {
                ret = ehsm_add_cmd_to_priority_queue(cmd);
            }
        }
        else
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }
    else if (cmd->req_type == EHSM_CMD_REQ_TYPE_SYNC)
    {
        ret = ehsm_add_cmd_to_sent_queue(cmd);
        if (ret == EHSM_ERR_SW_SUCCESS)
        {
            ret = ehsm_mbox_send_cmd(cmd);
            ehsm_del_cmd_from_sent_queue(cmd);
        }
        else
        {;}
    }
    else if (cmd->req_type == EHSM_CMD_REQ_TYPE_NO_RSP)
    {
        ehsm_mbox_send_cmd(cmd);
        ret = EHSM_ERR_SW_SUCCESS;
        cmd->error_code = EHSM_ERR_SW_SUCCESS;
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

ehsm_uint32_t ehsm_remove_cmd_from_queue(ehsm_cmd_req_st *cmd)
{
    ehsm_uint32_t ret = EHSM_ERR_GENERAL_ERROR;

    if (NULL == cmd)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        /*Remove the cmd from the queue and cancel the cmd*/
        ret = ehsm_del_cmd_from_priority_queue(cmd);
        if (ret == EHSM_ERR_SW_SUCCESS)
        {
            cmd->cmd_state = EHSM_CMD_REQ_STATE_DONE;
            cmd->error_code = EHSM_ERR_CMD_CANCELED;
            if (NULL != cmd->req_cb)
            {
                cmd->req_cb(cmd->req_ctx, cmd);
            }
            else
            {;}
            if (NULL != cmd->release_cb)
            {
                cmd->release_cb(cmd);
            }
            else
            {;}
        }
        else
        {;}
    }
    return ret;
}

ehsm_uint32_t ehsm_dispacher_init(void)
{
    ehsm_uint32_t ret = EHSM_ERR_GENERAL_ERROR;

    ret = ehsm_mbox_init();
    if (ret == EHSM_ERR_SW_SUCCESS)
    {
        ret = ehsm_crypto_object_init();
    }
    return ret;
}
void Exclusive_area_enter()
{

}

void Exclusive_area_exit()
{

}
