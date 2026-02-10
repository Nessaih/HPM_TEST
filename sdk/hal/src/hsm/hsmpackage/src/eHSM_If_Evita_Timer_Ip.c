/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/

#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#ifdef CONFIG_EHSM_HW_UTC_TIME
#include "eHSM_Types_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Com_Struct_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_If_Evita_ErrCode_Ip.h"
#include "eHSM_Mailbox_CmdId_Ip.h"
#include "eHSM_If_Evita_Ip.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

#define EVITA_MAX_MSG_INPUT_SIZE    1024

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
ehsm_uint32_t Create_Time_Stamp(ehsm_uint32_t msg_imprint_size, const ehsm_uint8_t *msg_imprint,
                                ehsm_uint32_t signature_key_handle, ehsm_uint32_t signature_key_authorization_size,
                                ehsm_uint8_t *signature_key_authorization_value, ehsm_uint32_t *signature_size,
                                signature_st *signature)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_mailbox_req_st cmd_timer = {0};

    do
    {
        /* msg_imprint_size can be 0, which means the whole input data only contain the time stamp */
        if (msg_imprint_size > EVITA_MAX_MSG_INPUT_SIZE)
        {
            ret = EVITA_INVALID_MSG_SIZE;
            break;
        }
        else if((NULL == msg_imprint) || (NULL == signature_size) || (NULL == signature))
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        else
        {;}

        ret = Evita_Check_Key_Handle(signature_key_handle);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Evita_Check_Authorization_Code(signature_key_authorization_size, signature_key_authorization_value);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

    } while(0U);

    if (ret == EVITA_OK)
    {
        cmd_timer.cmd_id = EHSM_CMD_CREATE_TIMER;
        cmd_timer.ehsm_cmd.timer.key_handle = signature_key_handle;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.key_auth_addr, (ehsm_uint8_t *)signature_key_authorization_value);
        cmd_timer.ehsm_cmd.timer.key_auth_size = signature_key_authorization_size;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.input_addr, (ehsm_uint8_t *)msg_imprint);
        cmd_timer.ehsm_cmd.timer.input_size = msg_imprint_size;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.output_addr, (ehsm_uint8_t *)signature);
        ret = ehsm_process_sync_service(EHSM_SRV_TIMER, &cmd_timer, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Check_Time_Stamp(ehsm_uint32_t msg_imprint_size, const ehsm_uint8_t *msg_imprint,
                               ehsm_uint32_t verification_key_handle, ehsm_uint32_t verification_key_authorization_size,
                               ehsm_uint8_t *verification_key_authorization_value, const signature_st *time_stamp,
                               ehsm_bool_t *time_stamp_vry, ehsm_uint32_t *delta)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_mailbox_req_st cmd_timer = {0};

    do
    {
        if ((NULL == msg_imprint) || (NULL == time_stamp) || (NULL == time_stamp_vry) || (NULL == delta))
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        else
        {;}

        if (msg_imprint_size > EVITA_MAX_MSG_INPUT_SIZE)
        {
            ret = EVITA_INVALID_MSG_SIZE;
            break;
        }
        else
        {;}

        ret = Evita_Check_Key_Handle(verification_key_handle);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Evita_Check_Authorization_Code(verification_key_authorization_size, verification_key_authorization_value);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

    } while (0U);

    if (EVITA_OK == ret)
    {
        cmd_timer.cmd_id = EHSM_CMD_CHECK_TIMER;
        cmd_timer.ehsm_cmd.timer.key_handle = verification_key_handle;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.key_auth_addr, (ehsm_uint8_t *)verification_key_authorization_value);
        cmd_timer.ehsm_cmd.timer.key_auth_size = verification_key_authorization_size;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.input_addr, (ehsm_uint8_t *)msg_imprint);
        cmd_timer.ehsm_cmd.timer.input_size = msg_imprint_size;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.output_addr, (ehsm_uint8_t *)time_stamp);
        cmd_timer.ehsm_cmd.timer.output_size = 48;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.context_addr, (ehsm_uint8_t *)delta);
        ret = ehsm_process_sync_service(EHSM_SRV_TIMER, &cmd_timer, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            *time_stamp_vry = TRUE;
        }
        else
        {
            *time_stamp_vry = FALSE;
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Get_Time_Sync_Challenge(ehsm_uint32_t *time_sync_challenge_size, ehsm_uint8_t time_sync_challenge[])
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_uint8_t i;
    ehsm_mailbox_req_st cmd_timer;

    if ((time_sync_challenge_size != NULL) && (time_sync_challenge != NULL))
    {
        cmd_timer.cmd_id = EHSM_CMD_GET_CHALLENGE;
        cmd_timer.ehsm_cmd.get_challenge.type = (ehsm_uint8_t)EHSM_CHALLENGE_TYPE_TIME_SYNC;
        for(i = 0; i < HOST_ADDRESS_SIZE; i++)
        {
            cmd_timer.ehsm_cmd.get_challenge.output_addr[i] = time_sync_challenge[i];
        }
        ret = ehsm_process_sync_service(EHSM_SRV_TIMER, &cmd_timer, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            /* The size is fixed to 32 bytes */
            *time_sync_challenge_size = 32U;
        }
        else
        {;}
        ret = ehsm_evita_convert_ret_code(ret);
    }
    else
    {
        ret = EVITA_GENERAL_ERROR;
    }
    return ret;
}

ehsm_uint32_t Set_UTC_Time(ehsm_utc_time_t utc_time, ehsm_uint32_t signature_size, const ehsm_uint8_t *signature,
                           ehsm_uint32_t verification_key_handle, ehsm_uint32_t verification_key_authorization_size,
                           ehsm_uint8_t *verification_key_authorization_value)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_mailbox_req_st cmd_timer = {0};

    do
    {
        if ((0U == signature_size) || (NULL == signature))
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        ret = Evita_Check_Key_Handle(verification_key_handle);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

        ret = Evita_Check_Authorization_Code(verification_key_authorization_size, verification_key_authorization_value);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}

    } while(0U);

    if (ret == EVITA_OK)
    {
        cmd_timer.cmd_id = EHSM_CMD_SET_UTC_TIMER;
        cmd_timer.ehsm_cmd.timer.utc_time = utc_time;
        cmd_timer.ehsm_cmd.timer.key_handle = verification_key_handle;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.key_auth_addr, (ehsm_uint8_t *)verification_key_authorization_value);
        cmd_timer.ehsm_cmd.timer.key_auth_size = verification_key_authorization_size;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.output_addr, (ehsm_uint8_t *)signature);
        cmd_timer.ehsm_cmd.timer.output_size = signature_size;
        ret = ehsm_process_sync_service(EHSM_SRV_TIMER, &cmd_timer, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Get_UTC_Time(ehsm_utc_time_t *utc_time)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_mailbox_req_st cmd_timer = {0};

    if (utc_time == NULL)
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        cmd_timer.cmd_id = EHSM_CMD_GET_UTC_TIMER;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.context_addr, (ehsm_uint8_t *)utc_time);
        ret = ehsm_process_sync_service(EHSM_SRV_TIMER, &cmd_timer, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Get_Tick_Count(ehsm_tick_value_st *tick_value)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_mailbox_req_st cmd_timer = {0};

    if (tick_value == NULL)
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        cmd_timer.cmd_id = EHSM_CMD_GET_TICK_COUNT;
        ehsm_set_address_pointer(cmd_timer.ehsm_cmd.timer.output_addr, (ehsm_uint8_t *)tick_value);
        ret = ehsm_process_sync_service(EHSM_SRV_TIMER, &cmd_timer, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}
#endif
