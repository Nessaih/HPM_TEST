/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#include <string.h>

#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Com_Struct_Ip.h"
#include "eHSM_Srv_CmdReq_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_Mailbox_CmdId_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_If_Ext_Ip.h"

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
static ehsm_uint32_t fw_encrypt_key_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t fw_encrypt_key_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t fw_get_random_key_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t fw_get_random_key_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t image_upgrade_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t image_upgrade_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t image_verify_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t image_verify_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t soc_image_verify_reqhdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t read_otp_data_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t read_otp_data_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t write_otp_data_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t write_otp_data_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t low_power_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t low_power_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t set_baudrate_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t set_baudrate_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t change_lifecycle_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t change_lifecycle_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t change_control_field_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t change_control_field_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t get_she_status_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t get_she_status_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t get_she_id_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t get_she_id_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t bootloader_cmd_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t bootloader_cmd_rsphdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t get_challenge_reqhdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t debug_auth_reqhdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t soc_boot_status_reqhdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t close_debug_reqhdl(void *para, ehsm_cmd_req_st *req);

static ehsm_uint32_t she_cancle_reqhdl(void *para, ehsm_cmd_req_st *req);

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static ehsm_uint32_t general_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_fw_encrypt_key = {
    .service_id = EHSM_SRV_EXTENDED_FW_ENCRYPT_KEY,
    .reqhdl = fw_encrypt_key_reqhdl,
    .rsphdl = fw_encrypt_key_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t fw_encrypt_key_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_fw_encrypt_key_st *encrypt_key = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        encrypt_key = (ehsm_fw_encrypt_key_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_FW_ENCRYPT_KEY;
        mailbox_packet->ehsm_cmd.fw_encrypt_key.key_slot = encrypt_key->key_slot;
        mailbox_packet->ehsm_cmd.fw_encrypt_key.key_type = encrypt_key->key_type;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.fw_encrypt_key.input_addr, encrypt_key->key_data);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.fw_encrypt_key.input_size = encrypt_key->key_size;
    }
    return ret;
}

static ehsm_uint32_t fw_encrypt_key_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_fw_get_random_key = {
    .service_id = EHSM_SRV_EXTENDED_FW_GET_RANDOM_KEY,
    .reqhdl = fw_get_random_key_reqhdl,
    .rsphdl = fw_get_random_key_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t fw_get_random_key_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_fw_random_key_st *random_key = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        random_key = (ehsm_fw_random_key_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_FW_GET_RANDOM_KEY;
        mailbox_packet->ehsm_cmd.fw_random_key.key_slot = random_key->key_slot;
        mailbox_packet->ehsm_cmd.fw_random_key.key_type = random_key->key_type;
    }
    return ret;
}

static ehsm_uint32_t fw_get_random_key_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_image_upgrade = {
    .service_id = EHSM_SRV_EXTENDED_IMAGE_UPGRADE,
    .reqhdl = image_upgrade_reqhdl,
    .rsphdl = image_upgrade_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t image_upgrade_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_image_upgrade_st *image_upgrade = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_PKE;
        image_upgrade = (ehsm_image_upgrade_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_IMAGE_UPGRADE;
        mailbox_packet->ehsm_cmd.image_upgrade.process_mode = image_upgrade->process_mode;

        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.image_upgrade.image_addr, image_upgrade->image);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.image_upgrade.image_size = (ehsm_uint32_t)image_upgrade->image_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.image_upgrade.storage_addr, image_upgrade->storage);
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.image_upgrade.ctx_addr, image_upgrade->ctx);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.image_upgrade.ctx_size = (ehsm_uint32_t)image_upgrade->ctx_size;
    }
    return ret;
}

static ehsm_uint32_t image_upgrade_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_image_verify = {
    .service_id = EHSM_SRV_EXTENDED_IMAGE_VERIFY,
    .reqhdl = image_verify_reqhdl,
    .rsphdl = image_verify_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t image_verify_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_image_verify_st *image_verify = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_PKE;
        image_verify = (ehsm_image_verify_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_IMAGE_VERIFY;
        mailbox_packet->ehsm_cmd.image_verify.process_mode = image_verify->process_mode;
        mailbox_packet->ehsm_cmd.image_verify.type = image_verify->type;

        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.image_verify.image_addr, image_verify->image);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.image_verify.image_size = (ehsm_uint32_t)image_verify->image_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.image_verify.ctx_addr, image_verify->ctx);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.image_verify.ctx_size = (ehsm_uint32_t)image_verify->ctx_size;
    }
    return ret;
}

ehsm_service_st g_srv_soc_image_verify = {
    .service_id = EHSM_SRV_EXTENDED_SOC_IMAGE_VERIFY,
    .reqhdl = soc_image_verify_reqhdl,
    .rsphdl = image_verify_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t soc_image_verify_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    /* ehsm_secure_boot_st *image_verify = NULL; */
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_PKE;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_SOC_IMAGE_VERIFY;
        ehsm_set_address_pointer((ehsm_uint8_t *)&mailbox_packet->ehsm_cmd, (ehsm_uint8_t *)para);        
    }

    return ret;
}

static ehsm_uint32_t image_verify_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}

#ifdef CONFIG_EHSM_SOC_UPGRADE_AND_VERIFY
static ehsm_uint32_t soc_image_upgrade_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t soc_image_upgrade_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_soc_image_upgrade = {
    .service_id = EHSM_SRV_EXTENDED_SOC_IMAGE_UPGRADE,
    .reqhdl = soc_image_upgrade_reqhdl,
    .rsphdl = soc_image_upgrade_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t soc_image_upgrade_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_PKE;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_SOC_IAMGE_UPGRADE_INIT;    
        ehsm_set_address_pointer((ehsm_uint8_t *)&mailbox_packet->ehsm_cmd, (ehsm_uint8_t *)para);        

    }
    return ret;
}

static ehsm_uint32_t soc_image_upgrade_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}
#endif


ehsm_service_st srv_read_otp_data = {
    .service_id = EHSM_SRV_EXTENDED_OTP_READ,
    .reqhdl = read_otp_data_reqhdl,
    .rsphdl = read_otp_data_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t read_otp_data_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_otp_read_param_st *otp_read_param = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        otp_read_param = (ehsm_otp_read_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_READ_OTP_DATA;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.otp_read.ehsm_src_addr = otp_read_param->flash_read_addr;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.otp_read.host_dst_addr, otp_read_param->otp_data_addr);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.otp_read.size = otp_read_param->read_data_size;
    }
    return ret;
}

static ehsm_uint32_t read_otp_data_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_write_otp_data = {
    .service_id = EHSM_SRV_EXTENDED_OTP_WRITE,
    .reqhdl = write_otp_data_reqhdl,
    .rsphdl = write_otp_data_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t write_otp_data_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_otp_write_param_st *otp_write_param = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        otp_write_param = (ehsm_otp_write_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_WRITE_OTP_DATA;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.otp_write.ehsm_dst_addr = otp_write_param->flash_write_addr;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.otp_write.host_src_addr, otp_write_param->otp_data_addr);
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.otp_write.size = otp_write_param->write_data_size;
    }
    return ret;
}

static ehsm_uint32_t write_otp_data_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}

static ehsm_uint32_t self_test_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t self_test_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_self_test = {
    .service_id = EHSM_SRV_SYS_SELF_TEST,
    .reqhdl = self_test_reqhdl,
    .rsphdl = self_test_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t self_test_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mbox_mgr_channel_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mbox_mgr_channel_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_SELF_TEST;
        mailbox_packet->ehsm_cmd.self_test_cmd.test_type = *(ehsm_uint32_t *)para;
    }
    return ret;
}

static ehsm_uint32_t self_test_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}


ehsm_service_st srv_low_power = {
    .service_id = EHSM_SRV_LOW_POWER,
    .reqhdl = low_power_reqhdl,
    .rsphdl = low_power_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t low_power_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mbox_mgr_channel_req_st *mailbox_packet = NULL;
    ehsm_uint8_t low_power_mode = 0;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mbox_mgr_channel_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_LOW_POWER;
        low_power_mode = *(ehsm_uint8_t *)para;
        if (low_power_mode == EHSM_POWER_MODE_LOW_POWER)
        {
            mailbox_packet->ehsm_cmd.low_power_cmd.power_mode = EHSM_LOW_POWER_MODE;
        }
        else if (low_power_mode == EHSM_POWER_MODE_NORMAL)
        {
            mailbox_packet->ehsm_cmd.low_power_cmd.power_mode = EHSM_NORMAL_MODE;
        }
        else
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }
    return ret;
}

static ehsm_uint32_t low_power_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_set_baudrate = {
    .service_id = EHSM_SRV_SET_BAUDRATE,
    .reqhdl = set_baudrate_reqhdl,
    .rsphdl = set_baudrate_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t set_baudrate_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mbox_mgr_channel_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mbox_mgr_channel_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_SET_BAUDRATE;
        mailbox_packet->ehsm_cmd.set_baudrate_cmd.baud_div = *(ehsm_uint32_t *)para;
    }
    return ret;
}

static ehsm_uint32_t set_baudrate_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_change_lifecycle = {
    .service_id = EHSM_SRV_CHANGE_LIFECYCLE,
    .reqhdl = change_lifecycle_reqhdl,
    .rsphdl = change_lifecycle_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t change_lifecycle_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mbox_mgr_channel_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mbox_mgr_channel_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_CHANGE_LIFECYCLE;
        mailbox_packet->ehsm_cmd.change_lifecycle_cmd.type = *(ehsm_uint32_t *)para;
    }
    return ret;
}

static ehsm_uint32_t change_lifecycle_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_change_control_field = {
    .service_id = EHSM_SRV_CHANGE_CONTROLFIELD,
    .reqhdl = change_control_field_reqhdl,
    .rsphdl = change_control_field_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t change_control_field_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_change_control_field_st *chg_ctrl_field = NULL;
    ehsm_mbox_mgr_channel_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        chg_ctrl_field = (ehsm_change_control_field_st *)para;
        mailbox_packet = (ehsm_mbox_mgr_channel_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_CHANGE_CONTROL_FIELD;
        mailbox_packet->ehsm_cmd.change_control_field_cmd.type = chg_ctrl_field->type;
        mailbox_packet->ehsm_cmd.change_control_field_cmd.size = chg_ctrl_field->size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.change_control_field_cmd.value_addr, chg_ctrl_field->value);
    }
    return ret;
}

static ehsm_uint32_t change_control_field_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_get_she_status = {
    .service_id = EHSM_SRV_GET_SHE_STATUS,
    .reqhdl = get_she_status_reqhdl,
    .rsphdl = get_she_status_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t get_she_status_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_GET_SHE_STATUS;
    }
    return ret;
}

static ehsm_uint32_t get_she_status_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_uint32_t status_value = 0;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            System_Memcpy(&status_value, req->rps_data, sizeof(ehsm_uint32_t));
            *((ehsm_uint8_t *)para) = (ehsm_uint8_t)status_value;
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_get_she_id = {
    .service_id = EHSM_SRV_GET_SHE_ID,
    .reqhdl = get_she_id_reqhdl,
    .rsphdl = get_she_id_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t get_she_id_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_she_get_id_param_st *she_get_id_param = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        she_get_id_param = (ehsm_she_get_id_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_GET_SHE_ID;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.get_she_id.challenge_addr, she_get_id_param->challenge);
        mailbox_packet->ehsm_cmd.get_she_id.challenge_size = she_get_id_param->challenge_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.get_she_id.status_addr, she_get_id_param->status);
        mailbox_packet->ehsm_cmd.get_she_id.status_size = she_get_id_param->status_size;
        ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.get_she_id.signatrue_addr, she_get_id_param->signatrue);
        mailbox_packet->ehsm_cmd.get_she_id.signatrue_size = she_get_id_param->signatrue_size;
    }
    return ret;
}

static ehsm_uint32_t get_she_id_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = req->error_code;
    }
    return ret;
}


static ehsm_uint32_t module_status_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t module_status_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_get_module_status = {
    .service_id = EHSM_SRV_MODULE_STATUS,
    .reqhdl = module_status_reqhdl,
    .rsphdl = module_status_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t module_status_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mailbox_req_st *mailbox_packet = NULL;
    ehsm_module_status_st *module_status_param = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_KEY;
        module_status_param = (ehsm_module_status_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_MODULE_STATUS;
        mailbox_packet->ehsm_cmd.module_status.type = module_status_param->type;
        mailbox_packet->ehsm_cmd.module_status.algo_id = module_status_param->algo_id;
        *((ehsm_uint32_t *)mailbox_packet->ehsm_cmd.module_status.key_handle) = module_status_param->key_handle;
        mailbox_packet->ehsm_cmd.module_status.key_auth_size = module_status_param->key_auth_size; 
        ehsm_set_address_pointer((ehsm_uint8_t *)mailbox_packet->ehsm_cmd.module_status.key_auth_addr, (ehsm_uint8_t *)module_status_param->key_auth_value);
        mailbox_packet->ehsm_cmd.module_status.status_size = *module_status_param->status_size;
        ehsm_set_address_pointer((ehsm_uint8_t *)mailbox_packet->ehsm_cmd.module_status.status_addr, (ehsm_uint8_t *)module_status_param->status);
        mailbox_packet->ehsm_cmd.module_status.signatrue_size = *module_status_param->sign_size;
        ehsm_set_address_pointer((ehsm_uint8_t *)mailbox_packet->ehsm_cmd.module_status.signatrue, (ehsm_uint8_t *)module_status_param->sign);
    }
    return ret;
}

static ehsm_uint32_t module_status_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_module_status_st *module_status_param = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            module_status_param = (ehsm_module_status_st *)para;
            *module_status_param->status_size = *((ehsm_uint32_t *)req->rps_data);
            *module_status_param->sign_size = *((ehsm_uint32_t *)(req->rps_data + sizeof(ehsm_uint32_t)));
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;
}

ehsm_service_st srv_bootloader_cmd = {
    .service_id = EHSM_SRV_BOOTLOADER_CMD,
    .reqhdl = bootloader_cmd_reqhdl,
    .rsphdl = bootloader_cmd_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t bootloader_cmd_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        System_Memcpy(req->cmd_data, para, 32);
    }
    return ret;
}

static ehsm_uint32_t bootloader_cmd_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = req->error_code;
    }
    return ret;
}


ehsm_service_st srv_get_challenge = {
    .service_id = EHSM_SRV_EXTENDED_GET_CHALLENGE,
    .reqhdl = get_challenge_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t get_challenge_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mailbox_req_st *mailbox_packet = NULL;
    ehsm_get_challenge_st *challenge = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        challenge = (ehsm_get_challenge_st *)para;
        if (NULL == challenge->buf)
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        else if ((challenge->type >= EHSM_CHALLENGE_TYPE_MAX) && (challenge->type <= EHSM_CHALLENGE_TYPE_INVALID))
        {
            ret = EHSM_ERR_WRONG_CHALLENGE_TYPE;
        }
        else if ((challenge->type == EHSM_CHALLENGE_TYPE_SHE_DEBUG) && (challenge->size < 16))
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        else if ((challenge->type == EHSM_CHALLENGE_TYPE_TIME_SYNC) && (challenge->size < 32))
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        else if (((challenge->type == EHSM_CHALLENGE_TYPE_EHSM_DEBUG) || (challenge->type == EHSM_CHALLENGE_TYPE_SOC_DEBUG)
            || (challenge->type == EHSM_CHALLENGE_TYPE_USER_AUTH)) && (challenge->size < 48))
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        else
        {
            req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
            mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
            mailbox_packet->cmd_id = EHSM_CMD_GET_CHALLENGE;
            mailbox_packet->ehsm_cmd.get_challenge.type = challenge->type;
            ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.get_challenge.output_addr, challenge->buf);
        }
    }
    return ret;
}

ehsm_service_st srv_debug_auth = {
    .service_id = EHSM_SRV_EXTENDED_DEBUG_AUTHENTICATION,
    .reqhdl = debug_auth_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t debug_auth_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mailbox_req_st *mailbox_packet = NULL;
    ehsm_debug_auth_st *debug_auth = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        debug_auth = (ehsm_debug_auth_st *)para;
        if ((debug_auth->alg > EHSM_DEBUG_AUTH_ALG_AES128_CMAC) || (debug_auth->alg < EHSM_DEBUG_AUTH_ALG_SM2_WITH_SM3))
        {
            ret = EHSM_ERR_WRONG_ALGORITHM;
        }
        else if ((debug_auth->type != EHSM_CHALLENGE_TYPE_EHSM_DEBUG) && (debug_auth->type != EHSM_CHALLENGE_TYPE_SOC_DEBUG) &&
            (debug_auth->type != EHSM_CHALLENGE_TYPE_USER_AUTH) && (debug_auth->type != EHSM_CHALLENGE_TYPE_SHE_DEBUG))
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        else if ((NULL == debug_auth->signature) || (0 == debug_auth->signature_size))
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        else if (((debug_auth->alg == EHSM_DEBUG_AUTH_ALG_SM2_WITH_SM3) || (debug_auth->alg == EHSM_DEBUG_AUTH_ALG_ECCSECP256R1_WITH_SHA256)) &&
                ((NULL == debug_auth->public_key) || (0 == debug_auth->public_key_size)))
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        else
        {
            req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
            mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
            mailbox_packet->cmd_id = EHSM_CMD_DEBUG_AUTHENCATION;

            mailbox_packet->ehsm_cmd.debug_authentication.algprithm = debug_auth->alg;
            mailbox_packet->ehsm_cmd.debug_authentication.type = debug_auth->type;
            ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.debug_authentication.pub_addr, debug_auth->public_key);
            *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.debug_authentication.pub_size = debug_auth->public_key_size;
            ehsm_set_address_pointer(mailbox_packet->ehsm_cmd.debug_authentication.sign_addr, debug_auth->signature);
            *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.debug_authentication.sign_size = debug_auth->signature_size;
        }
    }
    return ret;
}

ehsm_service_st srv_soc_boot_status = {
    .service_id = EHSM_SRV_SOC_BOOT_STATUS,
    .reqhdl = soc_boot_status_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t soc_boot_status_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret;
    ehsm_uint8_t soc_boot_status;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        soc_boot_status = *(ehsm_uint8_t *)para;
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_SOC_BOOT_STATUS;
        mailbox_packet->ehsm_cmd.soc_boot.status = soc_boot_status;
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

ehsm_service_st srv_close_debug = {
    .service_id = EHSM_SRV_SYS_CLOSE_DEBUG,
    .reqhdl = close_debug_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t close_debug_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_CLOSE_DEBUG;
        mailbox_packet->ehsm_cmd.close_debug.type = *(ehsm_uint32_t *)para;
    }
    return ret;
}

#ifdef CONFIG_EHSM_DEBUG
static ehsm_uint32_t reset_ehsm_firmware_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t reset_ehsm_firmware_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_reset_firmware = {
    .service_id = EHSM_SRV_SYS_RESET_FIRMWARE,
    .reqhdl = reset_ehsm_firmware_reqhdl,
    .rsphdl = reset_ehsm_firmware_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t reset_ehsm_firmware_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mbox_mgr_channel_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mbox_mgr_channel_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_RESET_FIRMWARE;
    }
    return ret;
}

static ehsm_uint32_t reset_ehsm_firmware_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = req->error_code;
    }
    return ret;
}

static ehsm_uint32_t get_fw_lifecycle_reqhdl(void *para, ehsm_cmd_req_st *req);
static ehsm_uint32_t get_fw_lifecycle_rsphdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_get_fw_lifecycle = {
    .service_id = EHSM_SRV_SYS_GET_FW_LIFECYCLE,
    .reqhdl = get_fw_lifecycle_reqhdl,
    .rsphdl = get_fw_lifecycle_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t get_fw_lifecycle_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mbox_mgr_channel_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mbox_mgr_channel_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_GET_FW_LIFECYCLE;
    }
    return ret;
}

static ehsm_uint32_t get_fw_lifecycle_rsphdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        if (req->error_code == EHSM_ERR_SW_SUCCESS)
        {
            *((ehsm_uint32_t*)para) = *((ehsm_uint32_t *)req->rps_data);
        }
        else
        {;}
        ret = req->error_code;
    }
    return ret;
}

static ehsm_uint32_t erase_otp_data_reqhdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_erase_otp_data = {
    .service_id = EHSM_SRV_EXTENDED_OTP_ERASE,
    .reqhdl = erase_otp_data_reqhdl,
    .rsphdl = general_rsphdl,
};

static ehsm_uint32_t erase_otp_data_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_storage_area_param_st *otp_erase_area_param = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        otp_erase_area_param = (ehsm_storage_area_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_ERASE_OTP_DATA;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.erase_area.size = otp_erase_area_param->size;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.erase_area.ehsm_dst_addr = otp_erase_area_param->addr;
    }
    return ret;
}

static ehsm_uint32_t erase_flash_data_reqhdl(void *para, ehsm_cmd_req_st *req);

ehsm_service_st srv_erase_flash_data = {
    .service_id = EHSM_SRV_EXTENDED_FLASH_ERASE,
    .reqhdl = erase_flash_data_reqhdl,
    .rsphdl = general_rsphdl,
};

static ehsm_uint32_t erase_flash_data_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_storage_area_param_st *flash_erase_area_param = NULL;
    ehsm_mailbox_req_st *mailbox_packet = NULL;

    if ((req == NULL) || (NULL == para))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        flash_erase_area_param = (ehsm_storage_area_param_st *)para;
        mailbox_packet = (ehsm_mailbox_req_st *)req->cmd_data;
        mailbox_packet->cmd_id = EHSM_CMD_ERASE_FLASH_DATA;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.erase_area.size = flash_erase_area_param->size;
        *(ehsm_uint32_t *)mailbox_packet->ehsm_cmd.erase_area.ehsm_dst_addr = flash_erase_area_param->addr;
    }
    return ret;
}
#endif

ehsm_service_st srv_she_cancle_cmd = {
    .service_id = EHSM_SRV_SYS_SHE_CANCEL,
    .reqhdl = she_cancle_reqhdl,
    .rsphdl = general_rsphdl,
    .timeout = CONFIG_EHSM_ARCH_V_DEFAULT_CMD_TIMEOUT
};

static ehsm_uint32_t she_cancle_reqhdl(void *para, ehsm_cmd_req_st *req)
{
    ehsm_uint32_t ret = 0U;
    ehsm_mbox_cancel_channel_req_st *mailbox_packet = NULL;

    if (req == NULL)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else 
    {
        req->object_type = CRYPTO_OBJECT_TYPE_SYSMGR;
        mailbox_packet = (ehsm_mbox_cancel_channel_req_st *)req->cmd_data;
        mailbox_packet->cancel_type = EHSM_CANCEL_CERT_TYPE_CMD;
        mailbox_packet->api_type = EHSM_API_TYPE_SHE;
    }
    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
