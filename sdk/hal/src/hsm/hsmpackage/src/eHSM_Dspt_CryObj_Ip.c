/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "eHSM_Dspt_CryObj_Ip.h"

#include "eHSM_Dspt_lp.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Srv_CmdReq_Ip.h"
#include "eHSM_IntCfg_Ip.h"
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
static crypto_object_st ske_object =
{
    .type = CRYPTO_OBJECT_TYPE_SKE,
    .name = (const ehsm_uint8_t *)"ske",
    .state = CRYPTO_OBJECT_STATE_FREE
};

static crypto_object_st pke_object =
{
    .type = CRYPTO_OBJECT_TYPE_PKE,
    .name = (const ehsm_uint8_t *)"pke",
    .state = CRYPTO_OBJECT_STATE_FREE
};

static crypto_object_st trng_object =
{
    .type = CRYPTO_OBJECT_TYPE_TRNG,
    .name = (const ehsm_uint8_t *)"trng",
    .state = CRYPTO_OBJECT_STATE_FREE
};

static crypto_object_st hash_object =
{
    .type = CRYPTO_OBJECT_TYPE_HASH,
    .name = (const ehsm_uint8_t *)"hash",
    .state = CRYPTO_OBJECT_STATE_FREE
};

static crypto_object_st key_object =
{
    .type = CRYPTO_OBJECT_TYPE_KEY,
    .name = (const ehsm_uint8_t *)"key",
    .state = CRYPTO_OBJECT_STATE_FREE
};

static crypto_object_st sysmgr_object =
{
    .type = CRYPTO_OBJECT_TYPE_SYSMGR,
    .name = (const ehsm_uint8_t *)"sysmgr",
    .state = CRYPTO_OBJECT_STATE_FREE
};

/* static crypto_object_type_e g_last_object; */
static crypto_object_st *object_array[] =
{
    &ske_object,
    &pke_object,
    &trng_object,
    &hash_object,
    &key_object,
    &sysmgr_object
};
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static void crypto_object_init_local(crypto_object_st *object)
{
    if (NULL != object)
    {
        object->cmd_sent_num = 0U;
        object->queue_capacity = 0U;
        INIT_DLIST_HEAD(&object->cmd_sent);
        INIT_DLIST_HEAD(&object->cmd_list);
        object->cmd_limit = 10U;
    }
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
ehsm_uint32_t ehsm_crypto_object_init(void)
{
    ehsm_uint32_t i = 0;
    ehsm_uint32_t module_num = sizeof(object_array) / sizeof(crypto_object_st *);
    for (i = 0U; i < module_num; i++)
    {
        crypto_object_init_local(object_array[i]);
    }
    return (EHSM_ERR_SW_SUCCESS);
}

ehsm_uint32_t ehsm_add_cmd_to_priority_queue(ehsm_cmd_req_st *cmd)
{
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;
    struct dlist_head *pos = NULL;
    struct dlist_head *temp = NULL;
    ehsm_bool_t inserted = FALSE;

    if (NULL == cmd)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        Exclusive_area_enter();
        crypto_object_st *crypto_object = object_array[cmd->object_type];
        if (crypto_object->queue_capacity >= crypto_object->cmd_limit)
        {
            ret = EHSM_ERR_QUEUE_FULL;
        }
        else
        {
            if (!dlist_empty(&crypto_object->cmd_list))
            {
                dlist_for_each_safe(pos, temp, &crypto_object->cmd_list)
                {
                    ehsm_cmd_req_st *cmd_node = dlist_entry(pos, ehsm_cmd_req_st, list);
                    if (cmd_node->priority < cmd->priority)
                    {
                        dlist_add(&cmd->list, pos->prev);
                        crypto_object->queue_capacity++;
                        inserted = TRUE;
                    }
                    else
                    {
                        /* No action */
                    }
                }
            }
            else
            {
                /* No action */
            }

            if (FALSE == inserted)
            {
                dlist_add_tail(&cmd->list, &crypto_object->cmd_list);
                crypto_object->queue_capacity++;
            }
            else
            {
                /* No action */
            }
        }
        Exclusive_area_exit();
    }
    return ret;
}

ehsm_uint32_t ehsm_del_cmd_from_priority_queue(ehsm_cmd_req_st *cmd)
{
    struct dlist_head *pos = NULL;
    struct dlist_head *temp = NULL;
    ehsm_uint32_t ret = EHSM_ERR_GENERAL_ERROR;

    if (cmd->object_type < CRYPTO_OBJECT_TYPE_MAX)
    {
        Exclusive_area_enter();
        crypto_object_st *crypto_object = object_array[cmd->object_type];
        if (!dlist_empty(&crypto_object->cmd_list))
        {
            dlist_for_each_safe(pos, temp, &crypto_object->cmd_list)
            {
                ehsm_cmd_req_st *cmd_node = dlist_entry(pos, ehsm_cmd_req_st, list);
                if (cmd_node == cmd)
                {
                    dlist_del(&cmd->list);
                    crypto_object->queue_capacity --;
                    ret = EHSM_ERR_SW_SUCCESS;
                }
                else
                {;}
            }
        }
        else
        {;}
        Exclusive_area_exit();
    }
    else
    {;}

    return ret;
}

ehsm_uint32_t ehsm_add_cmd_to_sent_queue(ehsm_cmd_req_st *cmd)
{
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == cmd) || (cmd->object_type >= CRYPTO_OBJECT_TYPE_MAX))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        Exclusive_area_enter();
        if (object_array[cmd->object_type]->state == CRYPTO_OBJECT_STATE_BUSY)
        {
            ret = EHSM_ERR_EHSM_BUSY;
        }
        else
        {
            crypto_object_st *crypto_object = object_array[cmd->object_type];
            if (crypto_object->cmd_sent_num >= CONFIG_EHSM_ARCH_V_CMD_QUEUE_SIZE)
            {
                crypto_object->state = CRYPTO_OBJECT_STATE_BUSY;
                ret = EHSM_ERR_EHSM_BUSY;
            }
            else
            {
                crypto_object->cmd_sent_num ++;
                dlist_add(&cmd->list, &crypto_object->cmd_sent);
                if (crypto_object->cmd_sent_num >= CONFIG_EHSM_ARCH_V_CMD_QUEUE_SIZE)
                {
                    crypto_object->state = CRYPTO_OBJECT_STATE_BUSY;
                }
                else
                {;}
            }
        }
        Exclusive_area_exit();
    }
    return ret;
}

ehsm_uint32_t ehsm_fetch_cmd_from_crypto_object(ehsm_cmd_req_st **cmd)
{
    ehsm_uint32_t i = 0U;
    struct dlist_head *pos = NULL;
    ehsm_uint32_t ret = EHSM_ERR_GENERAL_ERROR;

    Exclusive_area_enter();
    for (i = 0U; i < (ehsm_uint32_t)CRYPTO_OBJECT_TYPE_MAX; i++)
    {
        if ((object_array[i]->state == CRYPTO_OBJECT_STATE_FREE) &&
            (object_array[i]->queue_capacity > 0U))
        {
            if (!dlist_empty(&(object_array[i]->cmd_list)))
            {
                pos = object_array[i]->cmd_list.next;
                ehsm_cmd_req_st *cmd_node = dlist_entry(pos, ehsm_cmd_req_st, list);
                dlist_del(&cmd_node->list);
                *cmd = cmd_node;
                object_array[i]->cmd_sent_num++;
                if (object_array[i]->cmd_sent_num >= CONFIG_EHSM_ARCH_V_CMD_QUEUE_SIZE)
                {
                    object_array[i]->state = CRYPTO_OBJECT_STATE_BUSY;
                }
                else
                {
                    /* No action */
                }
                dlist_add(&cmd_node->list, &object_array[i]->cmd_sent);
                object_array[i]->queue_capacity--;
                ret = EHSM_ERR_SW_SUCCESS;
            }
            else
            {
                /* No action */
            }
        }
        else
        {
            /* No action */
        }
    }
    Exclusive_area_exit();
    return ret;
}

ehsm_uint32_t ehsm_crypto_object_get_cmd_done(crypto_object_type_e object_type, ehsm_cmd_req_st **cmd)
{
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;
    struct dlist_head *pos = NULL;
    struct dlist_head *temp = NULL;
    crypto_object_st *crypto_object = NULL;

    if (object_type >= CRYPTO_OBJECT_TYPE_MAX)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        Exclusive_area_enter();
        crypto_object = object_array[object_type];
        ret = EHSM_ERR_GENERAL_ERROR;
        if (!dlist_empty(&crypto_object->cmd_sent))
        {
            dlist_for_each_safe(pos, temp, &crypto_object->cmd_sent)
            {
                ehsm_cmd_req_st *cmd_node = (ehsm_cmd_req_st *)dlist_entry(pos, ehsm_cmd_req_st, list);
                if ((((cmd_node->cmd_state == EHSM_CMD_REQ_STATE_DONE) ||
                    (cmd_node->cmd_state == EHSM_CMD_REQ_STATE_CANCELED)) &&
                    (cmd_node->req_type == EHSM_CMD_REQ_TYPE_ASYNC)))
                {
                    dlist_del(&cmd_node->list);
                    crypto_object->cmd_sent_num--;
                    crypto_object->state = CRYPTO_OBJECT_STATE_FREE;
                    *cmd = cmd_node;
                    ret = EHSM_ERR_SW_SUCCESS;
                }
                else
                {
                    /* No action */
                }
            }
        }
        else
        {
            /* No action */
        }
        Exclusive_area_exit();
    }
    return ret;
}

ehsm_bool_t ehsm_crypto_object_is_free(crypto_object_type_e object_type)
{
    ehsm_bool_t ret = FALSE;

    if (object_type >= CRYPTO_OBJECT_TYPE_MAX)
    {
        ret = FALSE;
    }
    else if (object_array[object_type]->state == CRYPTO_OBJECT_STATE_FREE)
    {
        ret = TRUE;
    }
    else
    {
        /* No action */
    }
    return ret;
}

ehsm_uint32_t ehsm_del_cmd_from_sent_queue(ehsm_cmd_req_st *cmd)
{
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;
    crypto_object_st *crypto_object = NULL;

    if ((NULL == cmd) || (cmd->object_type >= CRYPTO_OBJECT_TYPE_MAX))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        Exclusive_area_enter();
        crypto_object = object_array[cmd->object_type];
        dlist_del(&cmd->list);
        crypto_object->cmd_sent_num --;
        crypto_object->state = CRYPTO_OBJECT_STATE_FREE;
        Exclusive_area_exit();
    }
    return ret;
}
