/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#ifdef CONFIG_EHSM_HW_COUNTER
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
ehsm_uint32_t Create_Counter(ehsm_uint32_t access_authorization_size, const ehsm_uint8_t *access_authorization_value,
                             ehsm_uint32_t *counter_identifier, ehsm_counter_value_st *counter_initial_value)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_mailbox_req_st cmd_counter;

    do
    {
        if ((NULL == counter_identifier) || (NULL == counter_initial_value))
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        else
        {;}

        /* Authentication code should not be empty */
        if ((0U == access_authorization_size) || (NULL == access_authorization_value))
        {
            ret = EVITA_AUTHORIZATION_FAILED;
            break;
        }
        else
        {;}

        ret = EVITA_OK;
    } while (0U);

    if (ret == EVITA_OK)
    {
        cmd_counter.cmd_id = EHSM_CMD_CREATE_COUNTER;
        ehsm_set_address_pointer(cmd_counter.ehsm_cmd.counter.auth_value, access_authorization_value);
        cmd_counter.ehsm_cmd.counter.auth_size = access_authorization_size;
        ehsm_set_address_pointer(cmd_counter.ehsm_cmd.counter.context_addr, (ehsm_uint8_t *)counter_initial_value);
        cmd_counter.ehsm_cmd.counter.context_size = (ehsm_addr_t)counter_identifier;
        ret = ehsm_process_sync_service(EHSM_SRV_COUNTER, &cmd_counter, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    else
    {;}
    return ret;
}

ehsm_uint32_t Read_Counter(ehsm_uint32_t counter_identifier, ehsm_counter_value_st *counter_current_value)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_mailbox_req_st cmd_counter;

    if (NULL == counter_current_value)
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        cmd_counter.cmd_id = EHSM_CMD_READ_COUNTER;
        cmd_counter.ehsm_cmd.counter.counter_id = counter_identifier;
        ehsm_set_address_pointer(cmd_counter.ehsm_cmd.counter.context_addr, (ehsm_uint8_t *)counter_current_value);
        ret = ehsm_process_sync_service(EHSM_SRV_COUNTER, &cmd_counter, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Increment_Counter(ehsm_uint32_t counter_identifier, ehsm_uint32_t access_authorization_size,
                                ehsm_uint8_t *access_authorization_value,
                                const ehsm_counter_value_st *counter_incrementation,
                                ehsm_counter_value_st *counter_new_value)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_mailbox_req_st cmd_counter;

    do
    {
        if (NULL == counter_new_value)
        {
            ret = EVITA_GENERAL_ERROR;
            break;
        }
        else
        {;}

        if (NULL == counter_incrementation)
        {
            ret = EVITA_INVALID_COUNTER_INCREMENTATION;
            break;
        }
        else
        {;}

        ret = Evita_Check_Authorization_Code(access_authorization_size, access_authorization_value);
        if (ret != EVITA_OK)
        {
            break;
        }
        else
        {;}
    } while(0U);

    if (ret == EVITA_OK)
    {
        cmd_counter.cmd_id = EHSM_CMD_INCREASE_COUNTER;
        cmd_counter.ehsm_cmd.counter.counter_id = counter_identifier;
        ehsm_set_address_pointer(cmd_counter.ehsm_cmd.counter.counter_inc, (ehsm_uint8_t *)counter_incrementation);
        cmd_counter.ehsm_cmd.counter.auth_size = access_authorization_size;
        ehsm_set_address_pointer(cmd_counter.ehsm_cmd.counter.auth_value, access_authorization_value);
        ehsm_set_address_pointer(cmd_counter.ehsm_cmd.counter.context_addr, (ehsm_uint8_t *)counter_new_value);
        ret = ehsm_process_sync_service(EHSM_SRV_COUNTER, &cmd_counter, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Delete_Counter(ehsm_uint32_t counter_identifier, ehsm_uint32_t access_authorization_size,
                             ehsm_uint8_t *access_authorization_value)
{
    ehsm_uint32_t ret = EVITA_GENERAL_ERROR;
    ehsm_mailbox_req_st cmd_counter;

    if ((0U == access_authorization_size) || (NULL == access_authorization_value))
    {
        ret = EVITA_AUTHORIZATION_FAILED;
    }
    else
    {
        ret = EVITA_OK;
    }

    if (ret == EVITA_OK)
    {
        cmd_counter.cmd_id = EHSM_CMD_DELETE_COUNTER;
        cmd_counter.ehsm_cmd.counter.counter_id = counter_identifier;
        ehsm_set_address_pointer(cmd_counter.ehsm_cmd.counter.auth_value, access_authorization_value);
        cmd_counter.ehsm_cmd.counter.auth_size = access_authorization_size;
        ret = ehsm_process_sync_service(EHSM_SRV_COUNTER, &cmd_counter, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}
#endif
