#ifndef CRYPTO_OBJECT_H
#define CRYPTO_OBJECT_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include "stdbool.h"
#include "eHSM_Srv_CmdReq_Ip.h"
#include "eHSM_Compt_List.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef enum  
{
    CRYPTO_OBJECT_STATE_FREE = 0,
    CRYPTO_OBJECT_STATE_BUSY
} crypto_object_state_e;

typedef struct crypto_object 
{
    /** crypto object type */
    crypto_object_type_e type;
    /**dispatcher name such as "SKE" "PKE" "TRNG" "KEY MANAGER" or "HASH"*/
    const ehsm_uint8_t *name;
    /* object state */
    crypto_object_state_e state;
    /* limit of  number that command sent each time */
    ehsm_uint32_t cmd_limit;
    /* command sent number, if it is more than cmd_limit then can't send 
        any more unless recived response of eHSM */
    ehsm_uint32_t cmd_sent_num;
    /*capacity of priority queue*/
    ehsm_uint32_t queue_capacity;
    /* priority queue for command */
    struct dlist_head cmd_list;
    /* list of command that has been sent */
    struct dlist_head cmd_sent;
} crypto_object_st;
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
/**
* @description crypto object initialization
*
* @return 0 for success, negtive values for error.
*/
ehsm_uint32_t ehsm_crypto_object_init(void);

/**
* @description submit a command to crypto object
*
* @return 0 for success, negtive values for error.
*/
ehsm_uint32_t ehsm_crypto_object_submit_cmd(ehsm_cmd_req_st *cmd);

/**
* @description fetch a command from priority queue of crypto object
*/
ehsm_uint32_t ehsm_fetch_cmd_from_crypto_object(ehsm_cmd_req_st **cmd);

/**
* @description check whether crypto object is free
*/
ehsm_bool_t ehsm_crypto_object_is_free(crypto_object_type_e object_type);

/**
* @description add cmd to sent queue of crypto object
*/
ehsm_uint32_t ehsm_add_cmd_to_sent_queue(ehsm_cmd_req_st *cmd);

/**
* @description add cmd to priority queue of crypto object
*/
ehsm_uint32_t ehsm_add_cmd_to_priority_queue(ehsm_cmd_req_st *cmd);

/**
* @description delete cmd from priority queue of crypto object
*/
ehsm_uint32_t ehsm_del_cmd_from_priority_queue(ehsm_cmd_req_st *cmd);

/**
* @description get cmd from crypto object which has been done
*/
ehsm_uint32_t ehsm_crypto_object_get_cmd_done(crypto_object_type_e object_type, ehsm_cmd_req_st **cmd);

ehsm_uint32_t ehsm_del_cmd_from_sent_queue(ehsm_cmd_req_st *cmd);
#endif
