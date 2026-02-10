/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_If_Evita_ErrCode_Ip.h"
#include "eHSM_Debug_Ip.h"
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

ehsm_uint32_t ehsm_evita_convert_ret_code(ehsm_uint32_t ret)
{
    ehsm_uint32_t ret_convert = 0;

    switch (ret)
    {
        case EHSM_ERR_MAILBOX_SUCCESS:
        case EHSM_ERR_SW_SUCCESS:
            ret_convert = EVITA_OK;
            break;
        case EHSM_ERR_WRONG_KEY_HANDLE:
            ret_convert = EVITA_WRONG_KEY_HANDLE;
            break;
        case EHSM_ERR_ALGORITHM_ERROR:
            ret_convert = EVITA_ALGORITHM_ERROR;
            break;
        case EHSM_ERR_WRONG_KEY_SIZE:
            ret_convert = EVITA_INVALID_KEY_SIZE;
            break;
        case EHSM_ERR_ALL_KEY_SPACE_OCCUPIED:
            ret_convert = EVITA_ALL_KEY_SPACE_OCCUPIED;
            break;
        case EHSM_ERR_INVALID_KEY_FLAG:
            ret_convert = EVITA_INVALID_KEY_FLAG;
            break;
        case EHSM_ERR_WRONG_AUTHORIZATION:
            ret_convert = EVITA_WRONG_AUTHORIZATION;
            break;
        case EHSM_ERR_WRONG_REMOTE_KEY_HANDLE:
            ret_convert = EVITA_WRONG_REMOTE_KEY_HANDLE;
            break;
        case EHSM_ERR_WRONG_KEY_COMBINATION:
            ret_convert = EVITA_WRONG_KEY_COMBINATION;
            break;
        case EHSM_ERR_AUTH_FAILED:
        case EHSM_ERR_COUNTER_AUTH_FAILED:
            ret_convert = EVITA_AUTHORIZATION_FAILED;
            break;
        case EHSM_ERR_TRANSPORT_IMPOSSIBLE:
            ret_convert = EVITA_TRANSPORT_IMPOSSIBLE;
            break;
        case EHSM_ERR_REMOVE_IMPOSSIBLE:
            ret_convert = EVITA_REMOVE_IMPOSSIBLE;
            break;
        case EHSM_ERR_WRONG_CERT_KEY_HANDLE:
            ret_convert = EVITA_WRONG_CERT_KEY_HANDLE;
            break;
        case EHSM_ERR_EHSM_BUSY:
            ret_convert = EVITA_ALL_SESSIONS_OCCUPIED;
            break;
        /* SKE relative error code */
        case EHSM_ERR_SKE_IV_SHOULD_NOT_NULL:
        case EHSM_ERR_SKE_WRONG_IV_SIZE:
            ret_convert = EVITA_WRONG_IV;
            break;
        /* RNG relative error code */
        case EHSM_ERR_DRBG_RESEED_FAILED:
            ret_convert = EVITA_TRNG_SEED_FAILURE;
            break;
        /* Counter relative error code */
        case EHSM_ERR_ALL_COUNTER_BUSY:
            ret_convert = EVITA_ALL_COUNTERS_OCCUPIED;
            break;
        case EHSM_ERR_COUNTER_WRONG_ID:
            ret_convert = EVITA_UNKNOWN_COUNTER_ID;
            break;
        case EHSM_ERR_INVALID_COUNTER_INCREMENTATION:
            ret_convert = EVITA_INVALID_COUNTER_INCREMENTATION;
            break;
        /* Timer relative error code */
        case EHSM_ERR_UTC_TIMER_NOT_SYNC:
            ret_convert = EVITA_CLOCK_NOT_SYNCHRONIZED;
            break;
        case EHSM_ERR_TIME_CHALLENGE_EXPIRED:
            ret_convert = EVITA_UTC_CHALLENGE_EXPIRED;
            break;
        case EHSM_ERR_WRONG_UTC_TIME:
            ret_convert = EVITA_INVALID_UTC_TIME;
            break;
        case EHSM_ERR_UTC_SYNCHRONIZATION_FAILED:
            ret_convert = EVITA_UTC_SYNCHRONIZATION_FAILED;
            break;
        default:
            ret_convert = EVITA_GENERAL_ERROR;
            break;
    }
#ifdef CONFIG_HOST_DEBUG_ENABLE
    if (ret_convert != EVITA_OK)
    {
        /* EVITA_LOG_ERROR("ret = 0x%x\n", ret); */
    }
#endif
    return ret_convert;
}

ehsm_uint32_t Evita_Check_Key_Handle(ehsm_uint32_t key_handle)
{
    ehsm_uint32_t ret;
    if (0U != key_handle)
    {
        ret = EVITA_OK;
    }
    else
    {
        ret = EVITA_WRONG_KEY_HANDLE;
    }
    return ret;
}

ehsm_uint32_t Evita_Check_Authorization_Code(ehsm_uint32_t authorization_size, ehsm_uint8_t *authorization_value)
{
    ehsm_uint32_t ret;
    if (0U != authorization_size)
    {
        if (NULL == authorization_value)
        {
            ret = EVITA_AUTHORIZATION_FAILED;
        }
        else
        {
            ret = EVITA_OK;
        }
    }
    else /* No need to check the authorization value */
    {
        ret = EVITA_OK;
    }
    return ret;
}
