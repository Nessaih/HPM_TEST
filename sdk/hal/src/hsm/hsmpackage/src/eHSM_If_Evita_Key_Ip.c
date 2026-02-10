/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "eHSM_IntCfg_Ip.h"
#include "eHSM_Config_Ip.h"

#include <string.h>

#include "eHSM_Debug_Ip.h"
#include "eHSM_Types_Ip.h"
#include "eHSM_Com_Struct_Ip.h"
#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_Srv_Mgr_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_If_Evita_Ip.h"
#include "eHSM_If_Evita_ErrCode_Ip.h"
#include "eHSM_If_Ext_Types_Ip.h"
#include "eHSM_IntCfg_Ip.h"

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
void translate_bool_array_to_use_flags(ehsm_uint32_t *export_use_flags, key_act_use_flags_t *use_flags);

static ehsm_bool_t valid_algo_type(ehsm_uint32_t algo_id)
{
    ehsm_bool_t ret = TRUE;
    
    if ((EHSM_ALG_END <= algo_id) || (EHSM_ALG_RANDOM == algo_id))
    {
        ret = FALSE;
    }
    
    return ret;
}

#ifdef CONFIG_EHSM_HW_UTC_TIME
ehsm_uint32_t Create_Random_Key(ehsm_uint32_t target_algorithm, ehsm_uint32_t key_size,
    ehsm_uint32_t valid_until, ehsm_key_mem_type_e type,
    ehsm_uint32_t key_usages_size, ehsm_uint8_t *key_usages_data, ehsm_uint32_t *key_handle)
#else
ehsm_uint32_t Create_Random_Key(ehsm_uint32_t target_algorithm, ehsm_uint32_t key_size,
    ehsm_key_mem_type_e type,
    ehsm_uint32_t key_usages_size, ehsm_uint8_t *key_usages_data, ehsm_uint32_t *key_handle)
#endif
{
    ehsm_create_random_key_param_st key_param[1];
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (((NULL == key_usages_data) && (0U != key_usages_size)) ||
        (NULL == key_handle) || (EHSM_EVITA_KEY_TYPE_RAM < type) || (EHSM_ALG_END <= target_algorithm) ||
        ((EHSM_ALG_RANDOM == target_algorithm) && (0U == key_size)))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {

        (void)System_Memset((ehsm_uint8_t *)key_param, 0U, sizeof(ehsm_create_random_key_param_st));
        key_param->target_algorithm_identifier = target_algorithm;
#ifdef CONFIG_EHSM_HW_UTC_TIME
        key_param->valid_until = valid_until;
#endif
        key_param->key_size = key_size;
        key_param->type = type;
        key_param->key_element_size = key_usages_size;
        key_param->key_element_data = (ehsm_key_flags_element_st *)key_usages_data;

        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_KEYGENERATE, key_param, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            *key_handle = key_param->key_handle;
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Create_Random_Dh_Key_Pair(ehsm_uint32_t key_size,
    ehsm_uint32_t valid_until, ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size,
    ehsm_uint8_t *key_usages_data, ehsm_uint8_t *p, ehsm_uint32_t p_size, ehsm_uint8_t *q,
    ehsm_uint32_t q_size, ehsm_uint8_t *g, ehsm_uint32_t g_size, ehsm_uint32_t *key_handle)
{
    ehsm_create_random_key_param_st key_param[1];
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == key_usages_data) || (NULL == p) || (NULL == q) || (NULL == g) || (NULL == key_handle))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        (void)System_Memset((ehsm_uint8_t *)key_param, 0U, sizeof(ehsm_create_random_key_param_st));

        key_param->target_algorithm_identifier = EHSM_ALG_DH;
#ifdef CONFIG_EHSM_HW_UTC_TIME
        key_param->valid_until = valid_until;
#endif
        key_param->key_size = key_size;
        key_param->type = type;
        key_param->key_element_size = key_usages_size;
        key_param->key_element_data = (ehsm_key_flags_element_st *)key_usages_data;
        key_param->p = p;
        key_param->p_size = p_size;
        key_param->q = q;
        key_param->q_size = q_size;
        key_param->g = g;
        key_param->g_size = g_size;

        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_KEYGENERATE, key_param, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            *key_handle = key_param->key_handle;
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

#ifdef CONFIG_EHSM_HW_UTC_TIME
ehsm_uint32_t Create_Dh_Key(ehsm_uint32_t target_algorithm, ehsm_uint32_t key_size,
    ehsm_uint32_t valid_until, ehsm_key_mem_type_e type,
    ehsm_uint32_t key_usages_size, ehsm_uint8_t *key_usages_data, ehsm_uint32_t local_key_handle,
    ehsm_uint32_t local_key_auth_size, ehsm_uint8_t *local_key_auth_value,
    ehsm_uint32_t remote_key_handle, ehsm_uint32_t remote_key_auth_size,
    ehsm_uint8_t *remote_key_auth_value, ehsm_uint8_t *ss_addr, ehsm_uint8_t dh_mode, ehsm_uint32_t *key_handle)
#else
ehsm_uint32_t Create_Dh_Key(ehsm_uint32_t target_algorithm, ehsm_uint32_t key_size,
    ehsm_key_mem_type_e type,
    ehsm_uint32_t key_usages_size, ehsm_uint8_t *key_usages_data, ehsm_uint32_t local_key_handle,
    ehsm_uint32_t local_key_auth_size, ehsm_uint8_t *local_key_auth_value,
    ehsm_uint32_t remote_key_handle, ehsm_uint32_t remote_key_auth_size,
    ehsm_uint8_t *remote_key_auth_value, ehsm_uint32_t *key_handle)
#endif
{
    ehsm_create_dh_key_param_st create_dh_key_para[1];
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == key_usages_data) || (0U == key_usages_size) ||
        (NULL == key_handle) || (EHSM_EVITA_KEY_TYPE_RAM < type) || (0U == key_size) ||
        ((local_key_auth_size != 0U) && (NULL == local_key_auth_value)) ||
        ((remote_key_auth_size != 0U) && (NULL == remote_key_auth_value)))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else if (EHSM_ALG_END <= target_algorithm)
    {
        ret = EVITA_ALGORITHM_ERROR;
    }
    else if (/*(0U == remote_key_handle)|| */ (0U == local_key_handle))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        (void)System_Memset((ehsm_uint8_t *)create_dh_key_para, 0U, sizeof(ehsm_create_dh_key_param_st));
        create_dh_key_para->target_algorithm_identifier = target_algorithm;
        create_dh_key_para->key_size = key_size;
#ifdef CONFIG_EHSM_HW_UTC_TIME
        create_dh_key_para->valid_until = valid_until;
#endif
        create_dh_key_para->dh_mode = dh_mode;
        create_dh_key_para->type = type;
        create_dh_key_para->key_element_size = key_usages_size;
        create_dh_key_para->key_element_data = (ehsm_key_flags_element_st *)key_usages_data;
        create_dh_key_para->local_key_handle = local_key_handle;
        create_dh_key_para->local_key_auth_size = local_key_auth_size;
        create_dh_key_para->local_key_auth_value = local_key_auth_value;
        create_dh_key_para->remote_key_handle = remote_key_handle;
        create_dh_key_para->remote_key_auth_or_pub_key_size = remote_key_auth_size;
        create_dh_key_para->remote_key_auth_value_or_pub_key = remote_key_auth_value;
        create_dh_key_para->ss_addr = ss_addr;

        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_KEYEXCHANGECALCSECRET, create_dh_key_para, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            *key_handle = create_dh_key_para->key_handle;
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Key_Import(ehsm_uint32_t transport_key_handle, ehsm_uint32_t transport_key_authorization_size,
    ehsm_uint8_t *transport_key_authorization, ehsm_uint32_t authenticity_key_handle,
    ehsm_uint32_t authenticity_key_authorization_size, ehsm_uint8_t *authenticity_key_authorization,
    ehsm_key_mem_type_e type, ehsm_uint32_t encrypted_key_size, ehsm_uint8_t *encrypted_key,
    ehsm_uint32_t key_authenticity_code_size, ehsm_uint8_t *key_authenticity_code, ehsm_uint32_t *key_handle)
{
    ehsm_evita_key_import_st import_key_para[1];
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == encrypted_key) || (NULL == key_handle) ||
            ((transport_key_authorization_size != 0U) && (NULL == transport_key_authorization)) ||
            ((authenticity_key_authorization_size != 0U) && (NULL == authenticity_key_authorization)))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        (void)System_Memset((ehsm_uint8_t *)import_key_para, 0U, sizeof(ehsm_evita_key_import_st));
        import_key_para->transport_key_handle = transport_key_handle;
        import_key_para->transport_key_author_size = transport_key_authorization_size;
        import_key_para->transport_key_author_value = transport_key_authorization;
        import_key_para->authenticity_key_handle = authenticity_key_handle;
        import_key_para->authenticity_key_author_size = authenticity_key_authorization_size;
        import_key_para->authenticity_key_author_value = authenticity_key_authorization;
        import_key_para->type = type;
        import_key_para->encrypted_key_size = encrypted_key_size;
        import_key_para->encrypted_key = encrypted_key;
        import_key_para->key_auth_code_size = key_authenticity_code_size;
        import_key_para->key_auth_code = key_authenticity_code;

        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_KEY_IMPORT, import_key_para, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            *key_handle = import_key_para->key_handle;
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Key_Export(ehsm_uint32_t key_handle, key_act_use_flags_t *use_flags, ehsm_uint32_t transport_key_handle,
    ehsm_uint32_t transport_key_authorization_size, ehsm_uint8_t *transport_key_authorization,
    ehsm_uint32_t authenticity_key_handle, ehsm_uint32_t authenticity_key_authorization_size,
    ehsm_uint8_t *authenticity_key_authorization, ehsm_uint32_t *encrypted_key_size, ehsm_uint8_t *encrypted_key,
    ehsm_uint32_t *key_authenticity_code_size, ehsm_uint8_t *key_authenticity_code)
{
    ehsm_evita_key_export_st export_key_para[1];
    ehsm_uint32_t export_use_flags = 0U;
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == use_flags) || (NULL == encrypted_key_size) || (NULL == encrypted_key) ||
         ((NULL == key_authenticity_code_size) && (0U != authenticity_key_handle)) ||
         ((NULL == key_authenticity_code) && (0U != authenticity_key_handle)) ||
        ((transport_key_authorization_size != 0U) && (NULL == transport_key_authorization)) ||
        ((authenticity_key_authorization_size != 0U) && (NULL == authenticity_key_authorization)))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else if ((0U == transport_key_handle) || (0U == authenticity_key_handle) || (0U == key_handle))
    {
        ret = EVITA_WRONG_KEY_HANDLE;
    }
    else
    {
        (void)System_Memset((ehsm_uint8_t *)export_key_para, 0U, sizeof(ehsm_evita_key_export_st));
        export_key_para->key_handle = key_handle;
        export_key_para->transport_key_handle = transport_key_handle;
        export_key_para->transport_key_author_size = transport_key_authorization_size;
        export_key_para->transport_key_author_value = transport_key_authorization;
        export_key_para->authenticity_key_handle = authenticity_key_handle;
        export_key_para->authenticity_key_author_size = authenticity_key_authorization_size;
        export_key_para->authenticity_key_author_value = authenticity_key_authorization;
        export_key_para->encrypted_key = encrypted_key;
        export_key_para->key_auth_code = key_authenticity_code;
        export_key_para->key_auth_code_size = *key_authenticity_code_size;
        translate_bool_array_to_use_flags(&export_use_flags, use_flags);
        export_key_para->use_flags = export_use_flags;

        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_KEY_EXPORT, export_key_para, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            *encrypted_key_size = export_key_para->encrypted_key_size;
            *key_authenticity_code_size = export_key_para->key_auth_code_size;
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

#ifdef CONFIG_EHSM_HW_UTC_TIME
ehsm_uint32_t Create_Derived_Key(ehsm_uint32_t key_derivation_function_identifier, ehsm_uint32_t key_size,
    ehsm_uint32_t valid_until, ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size, ehsm_uint8_t *key_usages_data,
    ehsm_uint32_t parent_key_handle, ehsm_uint32_t parent_key_authorization_size, ehsm_uint8_t *parent_key_authorization_value,
    ehsm_uint32_t salt_size, ehsm_uint8_t *salt_data, ehsm_uint32_t *key_handle)
#else
ehsm_uint32_t Create_Derived_Key(ehsm_uint32_t key_derivation_function_identifier, ehsm_uint32_t key_size,
    ehsm_key_mem_type_e type, ehsm_uint32_t key_usages_size, ehsm_uint8_t *key_usages_data,
    ehsm_uint32_t parent_key_handle, ehsm_uint32_t parent_key_authorization_size, ehsm_uint8_t *parent_key_authorization_value,
    ehsm_uint32_t salt_size, ehsm_uint8_t *salt_data, ehsm_uint32_t *key_handle)
#endif
{
    ehsm_key_derived_param_st derive_key_para[1];
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == key_usages_data) || (0U == key_usages_size) || (EHSM_EVITA_KEY_TYPE_RAM < type) ||
        (0U == salt_size) || (NULL == salt_data) || (NULL == key_handle) ||
        ((parent_key_authorization_size != 0U) && (NULL == parent_key_authorization_value)))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else if (EVITA_KEY_DERIVE_PBKDF2 < key_derivation_function_identifier)
    {
        ret = EVITA_ALGORITHM_ERROR;
    }
    else if (0U == key_size)
    {
        ret = EVITA_INVALID_KEY_SIZE;
    }
    else if (0U == parent_key_handle)
    {
        ret = EVITA_WRONG_KEY_HANDLE;
    }
    else
    {
        (void)System_Memset((ehsm_uint8_t *)derive_key_para, 0U, sizeof(ehsm_key_derived_param_st));
        derive_key_para->key_deriv_func = key_derivation_function_identifier;
        derive_key_para->key_size = key_size;
#ifdef CONFIG_EHSM_HW_UTC_TIME
        derive_key_para->valid_until = valid_until;
#endif
        derive_key_para->type = type;
        derive_key_para->key_element_size = key_usages_size;
        derive_key_para->key_element_data = (ehsm_key_flags_element_st *)key_usages_data;
        derive_key_para->parent_key_handle = parent_key_handle;
        derive_key_para->parent_key_author_size = parent_key_authorization_size;
        derive_key_para->parent_key_author_value = parent_key_authorization_value;
        derive_key_para->salt_size = salt_size;
        derive_key_para->salt_data = salt_data;
        derive_key_para->derive_type = CRYPTO_KEY_DERIVE_USER_KEYHANDLE;

        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_CREATE_DERIVED_KEY, derive_key_para, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            *key_handle = derive_key_para->key_handle;
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Key_Remove(ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size,
    ehsm_uint8_t *key_authorization)

{
    ehsm_key_remove_param_st key_remove[1];
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((key_authorization_size != 0U) && (NULL == key_authorization))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else if (0U == key_handle)
    {
        ret = EVITA_WRONG_KEY_HANDLE;
    }
    else
    {
        (void)System_Memset((ehsm_uint8_t *)key_remove, 0U, sizeof(ehsm_key_remove_param_st));
        key_remove->key_handle = key_handle;
        key_remove->key_auth_size = key_authorization_size;
        key_remove->key_auth_value = key_authorization;

        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_KEY_REMOVE, key_remove, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t Key_Status(ehsm_uint32_t key_handle, ehsm_uint32_t certification_key_handle,
    ehsm_uint32_t certification_key_authorization_size, ehsm_uint8_t *certification_key_authorization,
    ehsm_uint32_t *key_status_size, ehsm_uint8_t *key_status)
{
    ehsm_key_status_param_st key_status_param[1];
    ehsm_uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (((certification_key_authorization_size != 0U) && (NULL == certification_key_authorization)) ||
        (NULL == key_status_size) || (NULL == key_status))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else if ((0U == key_handle) || (0U == certification_key_handle))
    {
        ret = EVITA_WRONG_KEY_HANDLE;
    }
    else
    {
        (void)System_Memset((ehsm_uint8_t *)key_status_param, 0U, sizeof(ehsm_key_status_param_st));
        key_status_param->key_handle = key_handle;
        key_status_param->certification_key_handle = certification_key_handle;
        key_status_param->certification_key_auth_size = certification_key_authorization_size;
        key_status_param->certification_key_auth_value = certification_key_authorization;
        key_status_param->key_status_buffer_size = *key_status_size;
        key_status_param->key_status = key_status;

        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_KEY_STATUS, key_status_param, EHSM_API_TYPE_EVITA);
        if (EHSM_ERR_SW_SUCCESS == ret)
        {
            *key_status_size = key_status_param->key_status_size;
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

void translate_bool_array_to_use_flags(ehsm_uint32_t *export_use_flags, key_act_use_flags_t *use_flags)
{
    *export_use_flags = 0U;
    if (TRUE == use_flags->sign)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_SIGN;
    }
    if (TRUE == use_flags->verify)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_VERIFY;
    }
    if (TRUE == use_flags->encrypt)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_ENCRYPT;
    }
    if (TRUE == use_flags->decrypt)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_DECRYPT;
    }
    if (TRUE == use_flags->timestamp)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_TIMESTAMP;
    }
    if (TRUE == use_flags->secureboot)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_SECUREBOOT;
    }
    if (TRUE == use_flags->securestorage)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_SECURESTORAGE;
    }
    if (TRUE == use_flags->createkey)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_KEYCREATION;
    }
    if (TRUE == use_flags->utcsync)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_UTCSYNC;
    }
    if (TRUE == use_flags->transport)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_TRANSPORT;
    }
    if (TRUE == use_flags->remove)
    {
        *export_use_flags |= EVITA_KEY_USE_FLAG_REMOVE;
    }
}

ehsm_uint32_t Module_Status(ehsm_uint32_t type, ehsm_uint32_t algo_id, ehsm_uint32_t key_handle, ehsm_uint32_t key_auth_size, const ehsm_uint8_t *key_auth_value,
                                    ehsm_uint32_t *status_size, ehsm_uint8_t *status, ehsm_uint32_t *sign_size, ehsm_uint8_t *sign)
{
    ehsm_uint32_t ret = 0U;

    ehsm_module_status_st get_module_status_param;

    if ((0U == key_handle) || ((0U == key_auth_size) || (NULL == key_auth_value) ||
       ((NULL == sign_size) || (0U == *sign_size) || (NULL == sign) || (FALSE == valid_algo_type(algo_id)))))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else if ((NULL == status_size) || (NULL == status))
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else if (0U == *status_size)
    {
        ret = EVITA_GENERAL_ERROR;
    }
    else
    {
        get_module_status_param.type = type;
        get_module_status_param.algo_id = algo_id;
        get_module_status_param.key_handle = key_handle;
        get_module_status_param.key_auth_size = key_auth_size;
        get_module_status_param.key_auth_value = key_auth_value;
        get_module_status_param.status_size = status_size;
        get_module_status_param.status = status;
        get_module_status_param.sign_size = sign_size;
        get_module_status_param.sign = sign;

        ret = ehsm_process_sync_service(EHSM_SRV_MODULE_STATUS, (void *)&get_module_status_param, EHSM_API_TYPE_EVITA);
        ret = ehsm_evita_convert_ret_code(ret);
    }
    return ret;
}

ehsm_uint32_t ehsm_get_pub_from_priv(ehsm_uint32_t key_handle, ehsm_uint32_t key_authorization_size, ehsm_uint8_t *key_authorization,
                                     ehsm_uint8_t *partner_pub_value, ehsm_uint32_t *partner_pub_size, ehsm_uint32_t priv_key_algo)
{
    ehsm_uint32_t ret = 0U;

    ehsm_get_pub_from_priv_param_st pub_key_param;

    if (E_OK == ret)
    {
        pub_key_param.key_handle = key_handle;
        pub_key_param.key_auth_size = key_authorization_size;
        pub_key_param.key_auth_value = key_authorization;
        pub_key_param.public_key_addr = partner_pub_value;
        pub_key_param.public_key_buffer_size = *partner_pub_size;
        pub_key_param.key_alg_id = priv_key_algo;
        ret = ehsm_process_sync_service(EHSM_SRV_KEYMGR_KEYEXCHANGECALCPUBVAL, &pub_key_param, EHSM_API_TYPE_EVITA);
        if ((EHSM_ERR_MAILBOX_SUCCESS == ret) || (EHSM_ERR_SW_SUCCESS == ret))
        {
            *partner_pub_size = pub_key_param.public_key_size;
        }
        else
        {
            ;
        }
        ret = ehsm_evita_convert_ret_code(ret);
    }
    else
    {
        ;
    }
    return ret;
}

