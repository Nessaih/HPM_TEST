#ifndef EHSM_CMD_REQ_H
#define EHSM_CMD_REQ_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include "eHSM_Com_Struct_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Compt_List.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define MAX_RESPONSE_DATA_SIZE     (sizeof(ehsm_sm9_exchg_key_cmd_child_st) + 4U)

/*default job priority*/
#define EHSM_CMD_PRIORITY_DEFAULT  10

#define EHSM_CMD_REQ_TYPE_SYNC    0U
#define EHSM_CMD_REQ_TYPE_ASYNC   1U
#define EHSM_CMD_REQ_TYPE_NO_RSP  2U
typedef ehsm_uint8_t ehsm_cmd_req_type_e;

#define EHSM_CMD_REQ_STATE_IDLE        0U
#define EHSM_CMD_REQ_STATE_INIT        1U
#define EHSM_CMD_REQ_STATE_PROCESSING  2U
#define EHSM_CMD_REQ_STATE_CANCELED    3U
#define EHSM_CMD_REQ_STATE_DONE        4U
typedef ehsm_uint8_t ehsm_cmd_req_state_e;

#define CRYPTO_OBJECT_TYPE_SKE     0U
#define CRYPTO_OBJECT_TYPE_PKE     1U
#define CRYPTO_OBJECT_TYPE_TRNG    2U
#define CRYPTO_OBJECT_TYPE_HASH    3U
#define CRYPTO_OBJECT_TYPE_KEY     4U
#define CRYPTO_OBJECT_TYPE_SYSMGR  5U
#define CRYPTO_OBJECT_TYPE_MAX     6U
typedef ehsm_uint8_t crypto_object_type_e;

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef struct ehsm_cmd_req ehsm_cmd_req_st;
typedef ehsm_uint32_t (*cmd_req_cb)(void *para, ehsm_cmd_req_st *req);
typedef void (*cmd_release_cb)(ehsm_cmd_req_st *req);

struct ehsm_cmd_req
{
    ehsm_uint32_t cmd_id;
    /* command state */
    ehsm_cmd_req_state_e cmd_state;
    /** request type, synchronous or asynchronous*/
    ehsm_cmd_req_type_e req_type;
    /* command priority */
    ehsm_uint32_t priority;
    /* callback function used for asynchronous notification */
    cmd_req_cb req_cb;
    /* callback function used for releasing struct ehsm_cmd_req */
    cmd_release_cb release_cb;
    /* crypto object type */
    crypto_object_type_e object_type;
    /* whitch API type does the cmd come from */
    ehsm_api_type_e api_type;
    struct dlist_head list;
    /*error for command processing*/
    ehsm_uint32_t error_code;
    /* command context, store pointer of Crypto_JobType*/
    void *req_ctx;
    ehsm_uint8_t rps_data[MAX_RESPONSE_DATA_SIZE];
    /* command data size */
    ehsm_uint32_t cmd_size;
    /* mailbox channel */
    mailbox_channel_e channel;
    /* command timeout */
    ehsm_uint32_t timeout;
    /* pointer of command data */
    ehsm_uint8_t cmd_data[MAILBOX_CMD_MAX_SIZE];
};

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
#endif
