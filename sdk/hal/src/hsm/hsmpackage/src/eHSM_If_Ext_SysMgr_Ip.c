/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_Config_Ip.h"
#include "eHSM_IntCfg_Ip.h"

#include <string.h>

#include "eHSM_If_Ext_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Mailbox_CmdId_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_If_She_ErrCode_Ip.h"
#include "eHSM_If_Evita_Ip.h"
#include "eHSM_Mailbox_Ip.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define EHSM_IMAGE_VERIFY_TYPE_FW_UPGRADE  0x01UL
#define EHSM_IMAGE_VERIFY_TYPE_SOC_BOOT    0x02UL
#define EHSM_IMAGE_VERIFY_TYPE_SOC_UPGRADE 0x04UL

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
ehsm_uint32_t g_ctx = 0x1FFEF000;
/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
ehsm_uint32_t ehsm_get_random_key(ehsm_fw_random_key_type_e key_type, ehsm_fw_random_key_slot_e key_slot)
{
    ehsm_uint32_t ret;

    ehsm_fw_random_key_st random_key_s;
    if (((key_type == EHSM_FW_RANDOM_KEY_TYPE_SYMMETRIC_KEY) &&
         (key_slot == EHSM_FW_RANDOM_KEY_SLOT_DEVICE_ROOT_KEY ||
          key_slot == EHSM_FW_RANDOM_KEY_SLOT_SOC_FW_VERIFY_KEY ||
          key_slot == EHSM_FW_RANDOM_KEY_SLOT_SOC_ENC_KEY || key_slot == EHSM_FW_RANDOM_KEY_SLOT_USER_ROOT_KEY)) ||
        ((key_type == EHSM_FW_RANDOM_KEY_TYPE_SM2_PRIVATE_KEY ||
          key_type == EHSM_FW_RANDOM_KEY_TYPE_SECP256R1_PRIVATE_KEY ||
          key_type == EHSM_FW_RANDOM_KEY_TYPE_SYMMETRIC_KEY) &&
         (key_slot == EHSM_FW_RANDOM_KEY_SLOT_SOC_PRIVATE_KEY)))
    {
        random_key_s.key_type = key_type;
        random_key_s.key_slot = key_slot;

        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_FW_GET_RANDOM_KEY, &random_key_s, EHSM_API_TYPE_EXT);
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    return ret;
}

ehsm_uint32_t ehsm_encrypt_key(ehsm_fw_encrypt_key_type_e key_type, ehsm_fw_encrypt_key_slot_e key_slot,
                               const ehsm_uint8_t *key, ehsm_uint32_t key_len)
{
    ehsm_uint32_t ret = EHSM_ERR_PARAM_ERROR;
    ehsm_fw_encrypt_key_st encrypt_key_s;

    if (((key_type == EHSM_FW_ENCRYPT_KEY_TYPE_SYMMETRIC_KEY) &&
         (key_slot == EHSM_FW_ENCRYPT_KEY_SLOT_SOC_FW_VERIFY_KEY ||
          key_slot == EHSM_FW_ENCRYPT_KEY_SLOT_SOC_UPGRADE_ENC_KEY ||
          key_slot == EHSM_FW_ENCRYPT_KEY_SLOT_SOC_UPGRADE_VERIFY_KEY ||
          key_slot == EHSM_FW_ENCRYPT_KEY_SLOT_USER_DEBUG_KEY)) ||
        ((key_type == EHSM_FW_ENCRYPT_KEY_TYPE_PUBLIC_KEY_HASH) &&
         (key_slot == EHSM_FW_ENCRYPT_KEY_SLOT_SOC_DEBUG_KEY ||
          key_slot == EHSM_FW_ENCRYPT_KEY_SLOT_SOC_FW_VERIFY_KEY ||
          key_slot == EHSM_FW_ENCRYPT_KEY_SLOT_SOC_UPGRADE_VERIFY_KEY ||
          key_slot == EHSM_FW_ENCRYPT_KEY_SLOT_USER_DEBUG_KEY)))
    {
        ret = EHSM_ERR_SW_SUCCESS;
    }
    else
    {;}

    if (ret == EHSM_ERR_SW_SUCCESS)
    {
        if ((key != NULL) && ((key_len == 16U) || (key_len == 32U)|| (key_len == 48U)))
        {
            encrypt_key_s.key_type = key_type;
            encrypt_key_s.key_slot = key_slot;

            encrypt_key_s.key_data = (ehsm_uint8_t *)key;
            encrypt_key_s.key_size = key_len;
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_FW_ENCRYPT_KEY, &encrypt_key_s, EHSM_API_TYPE_EXT);
        }
        else
        {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        }
    }
    else
    {;}

    return ret;
}

ehsm_uint32_t ehsm_get_challenge(ehsm_get_challenge_st *req)
{
    ehsm_uint32_t ret = 0U;

    if (NULL == req)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_GET_CHALLENGE, req, EHSM_API_TYPE_EXT);
    }

    return ret;
}

ehsm_uint32_t ehsm_debug_auth(void *req)
{
    ehsm_uint32_t ret = 0U;

    if (NULL == req)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_DEBUG_AUTHENTICATION, req, EHSM_API_TYPE_EXT);
    }
    return ret;
}

ehsm_uint32_t ehsm_hsm_fw_upgrade_init(const char *upg_info, ehsm_uint32_t upg_info_size)
{
    ehsm_uint32_t ret = 0U;
    ehsm_image_upgrade_st image_upgrade_s;

    if ((NULL == upg_info) || (0U == upg_info_size))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else if(upg_info_size%16U == 0U)
    {
        image_upgrade_s.process_mode = EHSM_IMAGE_PROCESS_MODE_INIT;
        image_upgrade_s.image = (ehsm_uint8_t *)upg_info;
        image_upgrade_s.image_size = upg_info_size;
        image_upgrade_s.ctx = (ehsm_uint8_t *)g_ctx;
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_IMAGE_UPGRADE, &image_upgrade_s, EHSM_API_TYPE_EXT);
    }
    else
    {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    }

    return ret;
}

ehsm_uint32_t ehsm_hsm_fw_upgrade_update(const ehsm_uint8_t *upg_encrypted_img, ehsm_uint32_t upg_img_size, ehsm_uint8_t *storage_img,
                                         ehsm_uint32_t *storage_img_size)
{
    ehsm_uint32_t ret = 0U;
    ehsm_image_upgrade_st image_upgrade_s;

    if ((NULL == upg_encrypted_img) || (0U == upg_img_size))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else if(upg_img_size%16U == 0U)
    {
        image_upgrade_s.process_mode = EHSM_IMAGE_PROCESS_MODE_UPDATE;
        image_upgrade_s.image = (ehsm_uint8_t *)upg_encrypted_img;
        image_upgrade_s.image_size = upg_img_size;
        image_upgrade_s.ctx = (ehsm_uint8_t *)g_ctx;
        image_upgrade_s.storage = storage_img;
        *storage_img_size = 0U;
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_IMAGE_UPGRADE, &image_upgrade_s, EHSM_API_TYPE_EXT);    
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            *storage_img_size = upg_img_size;
        }
    }
    else
    {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    }

    return ret;
}

ehsm_uint32_t ehsm_hsm_fw_upgrade_finish(const ehsm_uint8_t *upg_encrypted_img, ehsm_uint32_t upg_img_size, ehsm_uint8_t *storage_img,
                                         ehsm_uint32_t *storage_img_size, char *mac, ehsm_uint32_t *mac_size)
{
    ehsm_uint32_t ret = 0U;
    ehsm_image_upgrade_st image_upgrade_s;

    if ((NULL == upg_encrypted_img) || (0U == upg_img_size) || (NULL == mac) || (NULL == mac_size))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else if(upg_img_size%16U == 0U)
    {
        image_upgrade_s.process_mode = EHSM_IMAGE_PROCESS_MODE_FINISH;
        image_upgrade_s.image = (ehsm_uint8_t *)upg_encrypted_img;
        image_upgrade_s.image_size = upg_img_size;
        image_upgrade_s.ctx = (ehsm_uint8_t *)g_ctx;
        image_upgrade_s.storage = storage_img;
        *storage_img_size = 0U;
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_IMAGE_UPGRADE, &image_upgrade_s, EHSM_API_TYPE_EXT);
        if(EHSM_ERR_SW_SUCCESS == ret)
        {
            *storage_img_size = upg_img_size;
            (void)System_Memcpy(mac, (ehsm_uint8_t *)g_ctx, 16U);
            *mac_size = 16U;
        }
    }
    else
    {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    }
    return ret;
}

ehsm_uint32_t ehsm_hsm_fw_upgrade_verify_init(const ehsm_uint8_t *code_info, ehsm_uint32_t code_info_size)
{
    ehsm_uint32_t ret = 0U;
    ehsm_image_verify_st image_verify_s;

    if ((NULL == code_info) || (0U == code_info_size))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else if(code_info_size%16U == 0U)
    {
        image_verify_s.process_mode = EHSM_IMAGE_PROCESS_MODE_INIT;
        image_verify_s.image = (ehsm_uint8_t *)code_info;
        image_verify_s.image_size = code_info_size;
        image_verify_s.ctx = (ehsm_uint8_t *)g_ctx;
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_IMAGE_VERIFY, &image_verify_s, EHSM_API_TYPE_EXT);
    }
    else
    {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    }

    return ret;
}

ehsm_uint32_t ehsm_hsm_fw_upgrade_verify_update(const ehsm_uint8_t *encrypted_img, ehsm_uint32_t img_size)
{
    ehsm_uint32_t ret = 0U;
    ehsm_image_verify_st image_verify_s;

    if ((NULL == encrypted_img) || (0U == img_size))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else if(img_size % 16U == 0U)
    {
        image_verify_s.process_mode = EHSM_IMAGE_PROCESS_MODE_UPDATE;
        image_verify_s.image = (ehsm_uint8_t *)encrypted_img;
        image_verify_s.image_size = img_size;
        image_verify_s.ctx = (ehsm_uint8_t *)g_ctx;
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_IMAGE_VERIFY, &image_verify_s, EHSM_API_TYPE_EXT);
    }
    else
    {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    }

    return ret;
}

ehsm_uint32_t ehsm_hsm_fw_upgrade_verify_finish(const ehsm_uint8_t *encrypted_img, ehsm_uint32_t img_size)
{
    ehsm_uint32_t ret = 0U;
    ehsm_image_verify_st image_verify_s;

    if ((NULL == encrypted_img) || (0U == img_size))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else if(img_size%16U == 0U)
    {
        image_verify_s.process_mode = EHSM_IMAGE_PROCESS_MODE_FINISH;
        image_verify_s.image = (ehsm_uint8_t *)encrypted_img;
        image_verify_s.image_size = img_size;
        image_verify_s.ctx = (ehsm_uint8_t *)g_ctx;
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_IMAGE_VERIFY, &image_verify_s, EHSM_API_TYPE_EXT);
    }
    else
    {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    }

    return ret;
}

#ifdef CONFIG_EHSM_SOC_UPGRADE_AND_VERIFY
ehsm_uint32_t ehsm_soc_image_upgrade(ehsm_soc_image_upgrade_info_st *upgrade_info)
{
    ehsm_uint32_t ret = 0U;
    ehsm_soc_image_upgrade_input_st *image_input_s = (ehsm_soc_image_upgrade_input_st *)upgrade_info->cmd_input;

    if((NULL == upgrade_info)||(NULL == upgrade_info->upgrade_sign))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        image_input_s->storage_alg = upgrade_info->storage_alg;
        image_input_s->upgrade_alg = upgrade_info->upgrade_alg;
        image_input_s->storage_encryption_flag = upgrade_info->storage_encryption_flag;
        image_input_s->upgrade_decryption_flag = upgrade_info->upgrade_decryption_flag;
        image_input_s->process_mode = upgrade_info->process_mode;
        image_input_s->check_version_flag = upgrade_info->check_version_flag;
        if(upgrade_info->upgrade_version != NULL)
        {
            ehsm_set_address_pointer(image_input_s->upgrade_version_addr, (ehsm_uint8_t *)upgrade_info->upgrade_version);
            *(ehsm_uint32_t *)image_input_s->upgrade_version_size = upgrade_info->upgrade_version_size;
        }
        if(upgrade_info->upgrade_iv != NULL)
        {
            ehsm_set_address_pointer(image_input_s->upgrade_iv_addr, (ehsm_uint8_t *)upgrade_info->upgrade_iv);
            *(ehsm_uint32_t *)image_input_s->upgrade_iv_size = upgrade_info->upgrade_iv_size;
        }
        if(upgrade_info->storage_iv != NULL)
        {
            ehsm_set_address_pointer(image_input_s->storage_iv_addr, (ehsm_uint8_t *)upgrade_info->storage_iv);
            *(ehsm_uint32_t *)image_input_s->storage_iv_size = upgrade_info->storage_iv_size;
        }
        if(upgrade_info->header != NULL)
        {
            ehsm_set_address_pointer(image_input_s->header_addr, (ehsm_uint8_t *)upgrade_info->header);
            *(ehsm_uint32_t *)image_input_s->header_size = upgrade_info->header_size;
        }
        if(upgrade_info->upgrade_pubkey != NULL)
        {
            ehsm_set_address_pointer(image_input_s->upgrade_pubkey_addr, (ehsm_uint8_t *)upgrade_info->upgrade_pubkey);
            *(ehsm_uint32_t *)image_input_s->upgrade_pubkey_size = upgrade_info->upgrade_pubkey_size;
        }
        if(upgrade_info->storage_image != NULL)
        {
            ehsm_set_address_pointer(image_input_s->storage_image_addr, (ehsm_uint8_t *)upgrade_info->storage_image);
            *(ehsm_uint32_t *)image_input_s->storage_image_size = upgrade_info->storage_image_size;
        }
        if(upgrade_info->mac_sign != NULL)
        {
            ehsm_set_address_pointer(image_input_s->mac_sign_addr, (ehsm_uint8_t *)upgrade_info->mac_sign);
            *(ehsm_uint32_t *)image_input_s->mac_sign_size = upgrade_info->mac_sign_size;
        }
        ehsm_set_address_pointer(image_input_s->upgrade_image_addr, (ehsm_uint8_t *)upgrade_info->upgrade_image);
        *(ehsm_uint32_t *)image_input_s->upgrade_image_size = upgrade_info->upgrade_image_size;
        ehsm_set_address_pointer(image_input_s->upgrade_sign_addr, (ehsm_uint8_t *)upgrade_info->upgrade_sign);
        *(ehsm_uint32_t *)image_input_s->upgrade_sign_size = upgrade_info->upgrade_sign_size;
        ehsm_set_address_pointer(image_input_s->ctx_addr, (ehsm_uint8_t *)g_ctx);
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_SOC_IMAGE_UPGRADE, image_input_s, EHSM_API_TYPE_EXT);
    }
    return ret;
}

ehsm_uint32_t ehsm_soc_image_verify(ehsm_soc_image_verify_info_st *verify_info)
{
    ehsm_uint32_t ret = 0U;
    ehsm_soc_image_verify_input_st *image_input_s = (ehsm_soc_image_verify_input_st *)verify_info->cmd_input;

    if((NULL == verify_info)||(NULL == verify_info->version)||(NULL == verify_info->storage_image)||(NULL == verify_info->storage_sign))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        image_input_s->update_version_flag = verify_info->update_version_flag;
        image_input_s->storage_alg = verify_info->storage_alg;
        image_input_s->type = SECURE_BOOT_TYPE_IMAGE_VERIFY;
        image_input_s->storage_encryption_flag = verify_info->storage_encryption_flag;

        if(verify_info->storage_iv != NULL)
        {
            ehsm_set_address_pointer(image_input_s->storage_iv_addr, (ehsm_uint8_t *)verify_info->storage_iv);
            *(ehsm_uint32_t *)image_input_s->storage_iv_size = verify_info->storage_iv_size;
        }

        if(verify_info->header != NULL)
        {
            ehsm_set_address_pointer(image_input_s->header_addr, (ehsm_uint8_t *)verify_info->header);
            *(ehsm_uint32_t *)image_input_s->header_size = verify_info->header_size;
        }

        if(verify_info->pubkey != NULL)
        {
            ehsm_set_address_pointer(image_input_s->pubkey_addr, (ehsm_uint8_t *)verify_info->pubkey);
            *(ehsm_uint32_t *)image_input_s->pubkey_size = verify_info->pubkey_size;
        }

        ehsm_set_address_pointer(image_input_s->version_addr, (ehsm_uint8_t *)verify_info->version);
        *(ehsm_uint32_t *)image_input_s->version_size = verify_info->version_size;
        ehsm_set_address_pointer(image_input_s->storage_image_addr, (ehsm_uint8_t *)verify_info->storage_image);
        *(ehsm_uint32_t *)image_input_s->storage_image_size = verify_info->storage_image_size;
        ehsm_set_address_pointer(image_input_s->storage_sign_addr, (ehsm_uint8_t *)verify_info->storage_sign);
        *(ehsm_uint32_t *)image_input_s->storage_sign_size = verify_info->storage_sign_size;
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_SOC_IMAGE_VERIFY, image_input_s, EHSM_API_TYPE_EXT);
    }

    return ret;
}
#endif

ehsm_uint32_t ehsm_secure_boot(ehsm_soc_image_verify_input_st *req)
{
    ehsm_uint32_t ret = 0U;

    if ((NULL == req) ||
        (0U == *(ehsm_uint32_t *)req->storage_image_size) || (1 == ehsm_is_cmd_addr_null(req->storage_image_addr)) ||
        (0U == *(ehsm_uint32_t *)req->storage_sign_size) || (1 == ehsm_is_cmd_addr_null(req->storage_sign_addr)))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        if(*(ehsm_uint32_t *)req->header_size % 16U == 0U)
        {
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_SOC_IMAGE_VERIFY, req, EHSM_API_TYPE_EXT);
        }
        else
        {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        }
    }

    ret = ehsm_she_convert_ret_code(ret);
    return ret;
}


ehsm_uint32_t ehsm_read_otp_data(ehsm_uint32_t otp_addr, ehsm_uint8_t *buf, ehsm_uint32_t read_size)
{
    ehsm_uint32_t ret = 0U;

    ehsm_otp_read_param_st otp_read_param;

    if ((NULL == buf) || (0U == read_size))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        otp_read_param.otp_data_addr = buf;
        otp_read_param.flash_read_addr = otp_addr;
        otp_read_param.read_data_size = read_size;

        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_OTP_READ, &otp_read_param, EHSM_API_TYPE_EXT);
    }
    return ret;
}

ehsm_uint32_t ehsm_write_otp_data(ehsm_uint32_t otp_addr, ehsm_uint8_t *buf, ehsm_uint32_t write_size)
{
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;
    ehsm_uint32_t w_otp_max_size = 0x320;

    ehsm_otp_write_param_st otp_write_param;
    ehsm_uint32_t idx = 0U;
    ehsm_uint32_t remain = write_size % w_otp_max_size;
    ehsm_uint32_t times = write_size / w_otp_max_size;

    if ((NULL == buf) || (0U == write_size))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        for (idx=0U; idx < times; idx++)
        {
            otp_write_param.otp_data_addr = buf + idx * w_otp_max_size;
            otp_write_param.flash_write_addr = otp_addr + idx * w_otp_max_size;
            otp_write_param.write_data_size = w_otp_max_size;
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_OTP_WRITE, &otp_write_param, EHSM_API_TYPE_EXT);
            if (ret != EHSM_ERR_SW_SUCCESS)
            {
                break;
            }
        }

        if ((ret == EHSM_ERR_SW_SUCCESS) && (remain != 0U))
        {
            otp_write_param.otp_data_addr = buf + idx * w_otp_max_size;
            otp_write_param.flash_write_addr = otp_addr + idx * w_otp_max_size;
            otp_write_param.write_data_size = remain;
            ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_OTP_WRITE, &otp_write_param, EHSM_API_TYPE_EXT);
        }
    }
    return ret;
}


ehsm_uint32_t ehsm_self_test(ehsm_uint32_t flag)
{
    ehsm_uint32_t ret = 0U;

    if (0U != (flag & (~EHSM_SELF_TEST_ALL)))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = ehsm_process_sync_service(EHSM_SRV_SYS_SELF_TEST, (void *)&flag, EHSM_API_TYPE_EXT);
    }
    return ret;
}

ehsm_uint32_t ehsm_low_power(ehsm_power_mode_e power_mode)
{
    ehsm_uint32_t ret = 0U;

    if ((power_mode != EHSM_POWER_MODE_NORMAL) && (power_mode != EHSM_POWER_MODE_LOW_POWER))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = ehsm_process_sync_service(EHSM_SRV_LOW_POWER, (void *)&power_mode, EHSM_API_TYPE_EXT);
    }
    return ret;
}
ehsm_uint32_t ehsm_set_uart_baudrate(ehsm_uint32_t baudrate)
{
    ehsm_uint32_t ret = 0U;

    if ((baudrate >= EHSM_UART_BAUDRATE_INVALID) || (baudrate < EHSM_UART_BAUDRATE_9600))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = ehsm_process_sync_service(EHSM_SRV_SET_BAUDRATE, (void *)&baudrate, EHSM_API_TYPE_EXT);
    }
    return ret;
}

ehsm_uint32_t ehsm_change_lifecycle(ehsm_lifecycle_e aim)
{
    ehsm_uint32_t ret = 0U;

    if ((aim != EHSM_LIFE_CYCLE_TEST_MODE) &&
        (aim != EHSM_LIFE_CYCLE_DEV_MODE) &&
        (aim != EHSM_LIFE_CYCLE_MANU_MODE) &&
        (aim != EHSM_LIFE_CYCLE_DEBUG_MODE) &&
        (aim != EHSM_LIFE_CYCLE_USER_MODE) &&
        (aim != EHSM_LIFE_CYCLE_DESTORY_MODE))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = ehsm_process_sync_service(EHSM_SRV_CHANGE_LIFECYCLE, (void *)&aim, EHSM_API_TYPE_EXT);
    }
    return ret;
}

ehsm_uint32_t ehsm_change_controlfield(ehsm_control_field_type_e type, ehsm_uint8_t *value, ehsm_uint16_t size)
{
    ehsm_uint32_t ret = 0U;

    ehsm_change_control_field_st chg_ctrl_field;

    if ((type != EHSM_CONTROL_FIELD_TYPE_HW) &&
        (type != EHSM_CONTROL_FIELD_TYPE_EHSM) &&
        (type != EHSM_CONTROL_FIELD_TYPE_SOC))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else if ((value == NULL) || (size == 0U))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        chg_ctrl_field.type = type;
        chg_ctrl_field.size = size;
        chg_ctrl_field.value = value;

        ret = ehsm_process_sync_service(EHSM_SRV_CHANGE_CONTROLFIELD, (void *)&chg_ctrl_field, EHSM_API_TYPE_EXT);
    }
    return ret;
}

ehsm_uint32_t ehsm_get_emu_status(ehsm_uint8_t *emu, ehsm_uint32_t *size)
{
    ehsm_uint32_t ret = 0U;

    ehsm_get_emu_status_param_st get_emu_status_param;

    if ((NULL == emu) || (NULL == size))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else if (0U == *size)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        get_emu_status_param.emu_addr = emu;
        get_emu_status_param.emu_size = size;

        ret = ehsm_process_sync_service(EHSM_SRV_GET_EMU_STATUS, (void *)&get_emu_status_param, EHSM_API_TYPE_EXT);
    }
    return ret;
}


ehsm_uint32_t ehsm_close_debug(ehsm_challenge_type_e type)
{
    ehsm_uint32_t ret = 0U;

    ret = ehsm_process_sync_service(EHSM_SRV_SYS_CLOSE_DEBUG, (void *)&type, EHSM_API_TYPE_EXT);
    return ret;
}


#ifdef CONFIG_EHSM_DEBUG
ehsm_uint32_t ehsm_reset_firmware()
{
    ehsm_uint32_t ret = 0U;
    ehsm_uint32_t param;

    ret = ehsm_process_norps_service(EHSM_SRV_SYS_RESET_FIRMWARE, (void *)&param, EHSM_API_TYPE_EXT);
    return ret;
}

ehsm_uint32_t ehsm_get_fw_lifecycle(ehsm_lifecycle_e *lf)
{
    ehsm_uint32_t ret = 0U;

    if (NULL == lf)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = ehsm_process_sync_service(EHSM_SRV_SYS_GET_FW_LIFECYCLE, (void *)lf, EHSM_API_TYPE_EXT);
    }
    return ret;
}

ehsm_uint32_t ehsm_erase_otp_data(ehsm_uint32_t otp_addr, ehsm_uint32_t otp_size)
{
    ehsm_uint32_t ret = 0U;

    ehsm_storage_area_param_st otp_area_param;

    if (0U == otp_size)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        otp_area_param.addr = otp_addr;
        otp_area_param.size = otp_size;
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_OTP_ERASE, &otp_area_param, EHSM_API_TYPE_EXT);
    }
    return ret;
}

ehsm_uint32_t ehsm_erase_flash_data(ehsm_uint32_t flash_addr, ehsm_uint32_t flash_size)
{
    ehsm_uint32_t ret = 0U;
    ehsm_storage_area_param_st flash_area_param;

    if (0U == flash_size)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        flash_area_param.addr = flash_addr;
        flash_area_param.size = flash_size;
        ret = ehsm_process_sync_service(EHSM_SRV_EXTENDED_FLASH_ERASE, &flash_area_param, EHSM_API_TYPE_EXT);
    }
    return ret;
}
#endif
