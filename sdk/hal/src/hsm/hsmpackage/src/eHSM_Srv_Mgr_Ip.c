/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#include <string.h>

#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Dspt_lp.h"
#include "eHSM_Compt_Bitmap.h"
#include "eHSM_Exclusive_Area.h"
#include "eHSM_Mailbox_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define COMMAND_REQ_QUANTITY      (CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_SKE_QUEUE_SIZE + CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_PKE_QUEUE_SIZE + \
                                    CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_TRNG_QUEUE_SIZE + CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_HASH_QUEUE_SIZE + \
                                    CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_K_QUEUE_SIZE + CONFIG_EHSM_ARCH_V_CRYPTO_OBJ_SYSMGR_QUEUE_SIZE + \
                                    CONFIG_EHSM_ARCH_V_CMD_QUEUE_SIZE * CRYPTO_OBJECT_TYPE_MAX)
/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct
{
    ehsm_cmd_req_st cmd_req[COMMAND_REQ_QUANTITY];
    bitmap_st *bitmap;
}ehsm_cmd_req_buffer_st;
/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static ehsm_uint32_t bitmap_buffer[sizeof(bitmap_st) + (COMMAND_REQ_QUANTITY/8U)];   /* atc modify, access align */
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/
ehsm_service_st *service_table[EHSM_SRV_END] = {NULL};

/* atc modify */
/* static ehsm_uint32_t _p_cmd_buffer = 0x1FFED000U; */
static ehsm_uint32_t _p_cmd_buffer[3025];
ehsm_cmd_req_buffer_st *g_cmd_req_buffer = NULL;
/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static void init_cmd_channel(ehsm_cmd_ext_type_e service_id, ehsm_cmd_req_st *cmd_req)
{
    switch (service_id)
    {
        case EHSM_SRV_LOW_POWER:
        case EHSM_SRV_SET_BAUDRATE:
        case EHSM_SRV_CHANGE_LIFECYCLE:
        case EHSM_SRV_CHANGE_CONTROLFIELD:
#ifdef CONFIG_EHSM_DEBUG
        case EHSM_SRV_SYS_RESET_FIRMWARE:
#endif
            cmd_req->channel = MAILBOX_CHANNE_MGR_SERVICE;
            cmd_req->cmd_size = S2H_SRV_MGR_WORD_SIZE * 4;
            break;
        case EHSM_SRV_SYS_ASR_CANCEL:
            cmd_req->channel = MAILBOX_CHANNE_CMD_CANCLE;
            cmd_req->cmd_size = S2H_SRV_CMD_CANCLE_WORD_SIZE * 4;
            break;
        default:
            cmd_req->cmd_size = S2H_SRV_GENERAL_WORD_SIZE * 4;
            cmd_req->channel = MAILBOX_CHANNE_GENERAL_SERVICE;
            break;
    }
}

static void cmd_req_buffer_init(ehsm_cmd_req_buffer_st *cmd_req_buffer)
{
    (void)System_Memset((ehsm_uint8_t *)cmd_req_buffer, 0U, sizeof(ehsm_cmd_req_buffer_st));
    cmd_req_buffer->bitmap = ehsm_bitmap_init((void *)bitmap_buffer, COMMAND_REQ_QUANTITY);
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void ehsm_set_address_pointer(ehsm_uint8_t *addr_array, const ehsm_uint8_t *data_pointer)
{
    ehsm_addr_t pointer_addr = (ehsm_addr_t)data_pointer;
    *(ehsm_uint32_t *)addr_array = (ehsm_uint32_t)pointer_addr;
}

ehsm_uint32_t ehsm_register_service(ehsm_service_st *service)
{
    ehsm_uint32_t ret = 0U;
    if (NULL != service)
    {
        if ((service->service_id <= EHSM_SRV_START) || (service->service_id >= EHSM_SRV_END))
        {
            ret =  EHSM_ERR_PARAM_ERROR;
        }
        service_table[service->service_id] = service;
    }

    return ret;
}

ehsm_uint32_t ehsm_service_init(void)
{
    ehsm_uint32_t i = 0U;
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    ret = ehsm_mbox_init();
    g_cmd_req_buffer = (ehsm_cmd_req_buffer_st *)_p_cmd_buffer;
    ret = ehsm_dispacher_init();
    if (ret == EHSM_ERR_SW_SUCCESS)
    {
        cmd_req_buffer_init(g_cmd_req_buffer);
        for (i = 0U; i < (ehsm_uint32_t)EHSM_SRV_END; i ++)
        {
            service_table[i] = NULL;
        }

        (void)ehsm_register_service(&srv_crypto_randomgenerate);
        (void)ehsm_register_service(&srv_crypto_ske);
        (void)ehsm_register_service(&srv_crypto_hash);
        (void)ehsm_register_service(&srv_crypto_pke);
        (void)ehsm_register_service(&srv_she_load_key);
        (void)ehsm_register_service(&srv_she_load_plain_key);
        (void)ehsm_register_service(&srv_she_ram_key_export);
        (void)ehsm_register_service(&srv_get_she_status);
        (void)ehsm_register_service(&srv_get_she_id);
        (void)ehsm_register_service(&srv_she_cancle_cmd);
        (void)ehsm_register_service(&srv_key_copy);
        (void)ehsm_register_service(&srv_certificate_parse);
        (void)ehsm_register_service(&srv_certificate_verify);
        (void)ehsm_register_service(&srv_create_random_key);
        (void)ehsm_register_service(&srv_derive_key);
        (void)ehsm_register_service(&srv_create_dh_key);
        (void)ehsm_register_service(&srv_derive_key);
        (void)ehsm_register_service(&srv_export_key);
        (void)ehsm_register_service(&srv_get_pub_from_priv);
        (void)ehsm_register_service(&srv_import_key);
        (void)ehsm_register_service(&srv_key_remove);
        (void)ehsm_register_service(&srv_get_module_status);
        (void)ehsm_register_service(&srv_key_status);
#ifdef CONFIG_EHSM_HW_COUNTER
        (void)ehsm_register_service(&srv_counter);
#endif
#ifdef CONFIG_EHSM_HW_UTC_TIME
        (void)ehsm_register_service(&srv_timer);
#endif
        (void)ehsm_register_service(&srv_get_challenge);
        (void)ehsm_register_service(&srv_debug_auth);
        (void)ehsm_register_service(&srv_close_debug);
        (void)ehsm_register_service(&srv_image_upgrade);
        (void)ehsm_register_service(&srv_image_verify);
        (void)ehsm_register_service(&g_srv_soc_image_verify);
#ifdef CONFIG_EHSM_SOC_UPGRADE_AND_VERIFY
        (void)ehsm_register_service(&srv_soc_image_upgrade);
#endif
        (void)ehsm_register_service(&srv_low_power);
#ifdef CONFIG_EHSM_SYS_SELF_TEST
        (void)ehsm_register_service(&srv_self_test);
#endif
        (void)ehsm_register_service(&srv_set_baudrate);
        (void)ehsm_register_service(&srv_soc_boot_status);
        (void)ehsm_register_service(&srv_read_otp_data);
        (void)ehsm_register_service(&srv_write_otp_data);

        (void)ehsm_register_service(&srv_fw_get_random_key);
        (void)ehsm_register_service(&srv_fw_encrypt_key);
        (void)ehsm_register_service(&srv_change_lifecycle);
        (void)ehsm_register_service(&srv_change_control_field);
        (void)ehsm_register_service(&srv_bootloader_cmd);
#ifdef CONFIG_EHSM_DEBUG
        (void)ehsm_register_service(&srv_get_fw_lifecycle);
        (void)ehsm_register_service(&srv_reset_firmware);
        (void)ehsm_register_service(&srv_erase_otp_data);
        (void)ehsm_register_service(&srv_erase_flash_data);
#ifdef CONFIG_EHSM_HW_UTC_TIME
        (void)ehsm_register_service(&srv_debug_get_flash_utc_time);
#endif
#endif

    }
    return ret;
}

ehsm_uint32_t ehsm_process_sync_service(ehsm_cmd_ext_type_e service_id, void *param, ehsm_api_type_e api_type)
{
    ehsm_cmd_req_st cmd_req[1];
    const ehsm_service_st *service;
    ehsm_uint32_t ret = 0U;

    if ((NULL == param) || (service_id > EHSM_SRV_END) || (NULL == service_table[service_id]))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        cmd_req->req_type = EHSM_CMD_REQ_TYPE_SYNC;
        cmd_req->api_type = api_type;
        init_cmd_channel(service_id, cmd_req);
        service = service_table[service_id];
        ret = service->reqhdl(param, cmd_req);
        if (ret == 0U)
        {
            ret = ehsm_mbox_send_cmd(cmd_req);
            if (ret == EHSM_ERR_SW_SUCCESS)
            {
                ret = service->rsphdl(param, cmd_req);
            }
            else
            {;}
        }
        else
        {;}
    }
    return ret;
}
#ifdef CONFIG_EHSM_DEBUG
ehsm_uint32_t ehsm_process_norps_service(ehsm_uint32_t service_id, void *param, ehsm_api_type_e api_type)
{
    ehsm_cmd_req_st cmd_req[1];
    ehsm_service_st *service;
    ehsm_uint32_t ret = 0U;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((NULL == param) || (service_id > EHSM_SRV_END) || (NULL == service_table[service_id]))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        cmd_req->req_type = EHSM_CMD_REQ_TYPE_NO_RSP;
        cmd_req->api_type = api_type;
        init_cmd_channel(service_id, cmd_req);
        service = service_table[service_id];
        ret = service->reqhdl(param, cmd_req);
        if (ret == 0U)
        {
            ret = ehsm_mbox_send_cmd(cmd_req);
            if (ret == EHSM_ERR_SW_SUCCESS)
            {
                ret = service->rsphdl(param, cmd_req);
            }
            else
            {;}
        }
        else
        {;}
    }
    return ret;
}
#endif
