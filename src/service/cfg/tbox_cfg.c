#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "Rcm_Hal.h"
#include "delay.h"
#include "fdb.h"
#include "tbox_log.h"
#include "tbox_string.h"
#include "tbox_memory.h"
#include "tbox_shell_if.h"

typedef struct
{
    UINT8 used;
    TBOX_CFG_INFO info;
}TBOX_CFG_ITEM;

#define CFG_MAGEIC_NAME  "MAGICNO"
#define CFG_MAGEIC_VALUE 0x1234ABCDU

static VOID cfg_reg_all_define_item(VOID);
static INT32 tbox_cfg_inner_init(VOID);
static VOID tbox_cfg_lock(fdb_kvdb_t db);
static VOID tbox_cfg_unlock(fdb_kvdb_t db);
static INT32 tbox_cfg_init_kvdb(VOID);
static BaseType_t tbox_cfg_shell_lscfg(CHAR *buf, UINT32 bufsz, const CHAR *cmd);
static BaseType_t tbox_cfg_shell_setdefault(CHAR *buf, UINT32 bufsz, const CHAR *cmd);
static BaseType_t tbox_cfg_shell_setcfg(CHAR *buf, UINT32 bufsz, const CHAR *cmd);
static INT32 tbox_cfg_init(UINT8 seq);
static VOID  tbox_cfg_exit(VOID);

static struct fdb_kvdb cfg_fdb;
static struct fdb_kvdb kv_fdb;
static TBOX_CFG_ITEM cfg_items[TBOX_CFG_ITEM_NUMBER];

/*模块定义*/
TBOX_MODULE_FUN(TBOXCFG, tbox_cfg_init, NULL_PTR, NULL_PTR, NULL_PTR, tbox_cfg_exit, NULL_PTR);
TBOX_MODULE(TBOXCFG, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, TRUE, FALSE);
TBOX_MESSSAGE(CFG_SET_DEFAULT, TBOX_MSG_PRIORITY_HIGH, TBOX_MSG_TYPE_TOPIC);
TBOX_MESSSAGE(CFG_VALUE_CHANGE, TBOX_MSG_PRIORITY_NORMAL, TBOX_MSG_TYPE_TOPIC);
TBOX_MODULE_LOADER(TBOXCFG)
{
    REGISTRY_TBOX_MESSAGE(CFG_SET_DEFAULT);
    REGISTRY_TBOX_MESSAGE(CFG_VALUE_CHANGE);
}

/*SHELL定义*/
TBOX_SHELL_DEFINE(lstcfg, "print cfg list info", 0U, tbox_cfg_shell_lscfg);
TBOX_SHELL_DEFINE(setdefault, "set all configurations to default value", 0U, tbox_cfg_shell_setdefault);
TBOX_SHELL_DEFINE(setcfg, "set configuration values", 2U, tbox_cfg_shell_setcfg);

/*配置定义*/
TBOX_CFG_DEFINE(MAGICNO, CFG_TYPE_NUMBER16, CFG_CAN_ERASE, "0x1234ABCD", 4U);
TBOX_CFG_DEFINE(DEVICEID, CFG_TYPE_STRING, CFG_DONOT_ERASE, "", TBOX_CFG_DEVICEID_LEN);
TBOX_CFG_DEFINE(TRACECODE, CFG_TYPE_STRING, CFG_DONOT_ERASE, "", TBOX_CFG_TRACECODE_LEN);
TBOX_CFG_DEFINE(SEID, CFG_TYPE_STRING, CFG_DONOT_ERASE, "", TBOX_CFG_SEID_LEN);
TBOX_CFG_DEFINE(BATTYPE, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "0", TBOX_CFG_BATTYPE_LEN);
TBOX_CFG_DEFINE(SLEEPMODE, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "0", TBOX_CFG_SLEEPMODE_LEN);
TBOX_CFG_DEFINE(SLEEPDELAY, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "30", TBOX_CFG_SLEEPDELAY_LEN);
TBOX_CFG_DEFINE(RTCENABLE, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "1", TBOX_CFG_RTCENABLE_LEN);
TBOX_CFG_DEFINE(RTCINTV, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "480", TBOX_CFG_RTCINTV_LEN);
TBOX_CFG_DEFINE(CAN1BAUD, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "250", TBOX_CFG_CANBAUD_LEN);
TBOX_CFG_DEFINE(CAN2BAUD, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "0", TBOX_CFG_CANBAUD_LEN);
TBOX_CFG_DEFINE(CAN3BAUD, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "0", TBOX_CFG_CANBAUD_LEN);
TBOX_CFG_DEFINE(CAN1AUTO, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "0", TBOX_CFG_CANBAUD_LEN);
TBOX_CFG_DEFINE(CAN2AUTO, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "0", TBOX_CFG_CANBAUD_LEN);
TBOX_CFG_DEFINE(CAN3AUTO, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "0", TBOX_CFG_CANBAUD_LEN);
TBOX_CFG_DEFINE(TIMEZONE, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "8", TBOX_CFG_TIMEZONE_LEN); //bit7:[+,-]
TBOX_CFG_DEFINE(PUBAPN, CFG_TYPE_STRING, CFG_CAN_ERASE, "cmnet", TBOX_CFG_APN_LEN);
TBOX_CFG_DEFINE(PRIAPN, CFG_TYPE_STRING, CFG_CAN_ERASE, "", TBOX_CFG_APN_LEN);
TBOX_CFG_DEFINE(OTAAPN, CFG_TYPE_STRING, CFG_CAN_ERASE, "", TBOX_CFG_APN_LEN);
TBOX_CFG_DEFINE(SMSCENTER, CFG_TYPE_STRING, CFG_CAN_ERASE, "", TBOX_CFG_SMSCENTER_LEN);
TBOX_CFG_DEFINE(GPSMODE, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "3", TBOX_CFG_GPSMODE_LEN);
TBOX_CFG_DEFINE(GPSRATE, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "1", TBOX_CFG_GPSRATE_LEN);
TBOX_CFG_DEFINE(VIN, CFG_TYPE_STRING, CFG_CAN_ERASE, "", TBOX_CFG_VIN_LEN);
TBOX_CFG_DEFINE(RESETCOUNT, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "0", TBOX_CFG_RESETCOUNT_LEN);
TBOX_CFG_DEFINE(RTCPOR, CFG_TYPE_NUMBER16, CFG_CAN_ERASE, "0", TBOX_CFG_RTCPOR_LEN);
TBOX_CFG_DEFINE(GBFURL, CFG_TYPE_STRING, CFG_CAN_ERASE, "gw-gb6.sea-level.cn", TBOX_CFG_URL_LEN);
TBOX_CFG_DEFINE(GBFPORT, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "60001", TBOX_CFG_PORT_LEN);
TBOX_CFG_DEFINE(HPMMURL, CFG_TYPE_STRING, CFG_CAN_ERASE, "", TBOX_CFG_URL_LEN);
TBOX_CFG_DEFINE(HPMMIP, CFG_TYPE_STRING, CFG_CAN_ERASE, "159.138.101.33", TBOX_CFG_IP_LEN);
TBOX_CFG_DEFINE(HPMMPORT, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "9088", TBOX_CFG_PORT_LEN);
TBOX_CFG_DEFINE(HPMSURL, CFG_TYPE_STRING, CFG_CAN_ERASE, "", TBOX_CFG_URL_LEN);
TBOX_CFG_DEFINE(HPMSIP, CFG_TYPE_STRING, CFG_CAN_ERASE, "", TBOX_CFG_IP_LEN);
TBOX_CFG_DEFINE(HPMSPORT, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "0", TBOX_CFG_PORT_LEN);
TBOX_CFG_DEFINE(HPMHTBT, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "60", TBOX_CFG_REPOT_INTV_LEN);
TBOX_CFG_DEFINE(HPMCYCON, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "20", TBOX_CFG_REPOT_INTV_LEN);
TBOX_CFG_DEFINE(HPMCYCOFF, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "60", TBOX_CFG_REPOT_INTV_LEN);
TBOX_CFG_DEFINE(HPMSLPDY, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "60", TBOX_CFG_REPOT_INTV_LEN);
TBOX_CFG_DEFINE(HPMSVRINTV, CFG_TYPE_NUMBER, CFG_CAN_ERASE, "10", TBOX_CFG_REPOT_INTV_LEN);


INT32 tbox_cfg_reset(VOID)
{
    CHAR *end_str;
    INT32 ret;
    struct fdb_blob blob;
    
    UINT8 *def_buff = mempool_alloc(TBOX_CFG_VALUE_MAX_LEN);
    if(NULL_PTR == def_buff)
    {
        MODULE_LOG_E(TBOXCFG, "failed to alloc memory");
        return (INT32)TBOX_E_FAILED_ALLOC;
    }

    /*设置默认值*/
    for (UINT8 i = 0U;  i < TBOX_CFG_ITEM_NUMBER; i++) 
    {
        if(FALSE == cfg_items[i].used || 
           CFG_CAN_ERASE != cfg_items[i].info.earse_flag)
        {
            continue;
        }

        if(CFG_TYPE_NUMBER == cfg_items[i].info.type)
        {
            UINT32 value = 0U;
            if(NULL_PTR != cfg_items[i].info.def_value &&
               strlen(cfg_items[i].info.def_value) > 0U)
            {
                value = (UINT32)strtol(cfg_items[i].info.def_value, &end_str, 10U);
                if(end_str == cfg_items[i].info.def_value)
                {
                    MODULE_LOG_E(TBOXCFG, "invalid number:%s", cfg_items[i].info.def_value);
                    mempool_free(def_buff);
                    return (INT32)TBOX_E_INVALID_DATA;
                }
            }
            fdb_blob_make(&blob, &value, cfg_items[i].info.len);
        }
        else if(CFG_TYPE_NUMBER16 == cfg_items[i].info.type)
        {
            UINT32 value = 0U;
            if(NULL_PTR != cfg_items[i].info.def_value &&
               strlen(cfg_items[i].info.def_value) > 0U)
            {
                value = (UINT32)strtol(cfg_items[i].info.def_value, &end_str, 16U);
                if(end_str == cfg_items[i].info.def_value)
                {
                    MODULE_LOG_E(TBOXCFG, "invalid number:%s", cfg_items[i].info.def_value);
                    mempool_free(def_buff);
                    return (INT32)TBOX_E_INVALID_DATA;
                }
            }
            fdb_blob_make(&blob, &value, cfg_items[i].info.len);
        }
        else if(CFG_TYPE_STRING == cfg_items[i].info.type)
        {
            memset(def_buff, 0U, TBOX_CFG_VALUE_MAX_LEN);
            if(NULL_PTR != cfg_items[i].info.def_value &&
               strlen(cfg_items[i].info.def_value) > 0U)
            {
                strncpy((CHAR*)def_buff, cfg_items[i].info.def_value, cfg_items[i].info.len);
            }
            fdb_blob_make(&blob, def_buff, cfg_items[i].info.len);
        }
        else if(CFG_TYPE_BYTE == cfg_items[i].info.type)
        {
            memset(def_buff, 0U, TBOX_CFG_VALUE_MAX_LEN);
            if(NULL_PTR != cfg_items[i].info.def_value &&
               strlen(cfg_items[i].info.def_value) > 0U)
            {
                tbox_string_to_bytes(cfg_items[i].info.def_value, def_buff, cfg_items[i].info.len);
            }
            fdb_blob_make(&blob, def_buff, cfg_items[i].info.len);
        }
        else
        {
            continue;
        }
        
        ret = (INT32)fdb_kv_set_blob(&cfg_fdb, cfg_items[i].info.name, &blob);
        if ((INT32)FDB_NO_ERR != ret) 
        {
            MODULE_LOG_E(TBOXCFG, "set error:%d,%d", ret, i);
            mempool_free(def_buff);
            return (INT32)TBOX_E_FAILED;
        }
    }

    mempool_free(def_buff);
    
    tbox_message_publish(TBOX_CFG_EVENT_SET_DEFAULT,  NULL_PTR);

    return (INT32)TBOX_E_OK;
}

INT32 tbox_cfg_reg(TBOX_CFG_INFO *item)
{
    UINT8 index;

    if(NULL_PTR == item)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    for(index = 0U; index < TBOX_CFG_ITEM_NUMBER; index++)
    {
        if(TRUE == cfg_items[index].used)
        {
            continue;
        }
        memcpy(&cfg_items[index].info, item, sizeof(TBOX_CFG_INFO));
        cfg_items[index].used = TRUE;
        break;
    }
    if(index >= TBOX_CFG_ITEM_NUMBER)
    {
        return (INT32)TBOX_E_NORESOURCES;
    }

    return (INT32)index;
}

INT32 tbox_cfg_write(TBOX_CFG_ID id, VOID *buf)
{
    if(id >= TBOX_CFG_ITEM_NUMBER || 
       NULL_PTR == buf)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    struct fdb_blob blob;
    TBOX_CFG_INFO *cfg_info = &cfg_items[id].info;
    if(FALSE == cfg_items[id].used)
    {
        MODULE_LOG_E(TBOXCFG, "cfg id:%d not register", id);
        return (INT32)TBOX_E_NOEXISTS;
    }
    fdb_blob_make(&blob, buf, cfg_info->len);
    if(FDB_NO_ERR != fdb_kv_set_blob(&cfg_fdb, cfg_info->name, &blob))
    {
        MODULE_LOG_E(TBOXCFG, "write cfg[%s] failed", cfg_items[id].info.name);
        return (INT32)TBOX_E_FAILED;
    }

    TBOX_CFG_CHANGE_INFO change_info;
    change_info.id = id;
    change_info.name = cfg_info->name;
    TBOX_MSG_DATA data = {.data = (UINT8*)&change_info, .size = sizeof(change_info)};
    tbox_message_publish(TBOX_CFG_EVENT_VALUE_CHANGE, &data);
    return (INT32)TBOX_E_OK;
}

INT32 tbox_cfg_read(TBOX_CFG_ID id, VOID *buf)
{
    if(id >= TBOX_CFG_ITEM_NUMBER || 
       NULL_PTR == buf)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    struct fdb_blob blob;
    TBOX_CFG_INFO *cfg_info = &cfg_items[id].info;
    if(FALSE == cfg_items[id].used)
    {
        MODULE_LOG_E(TBOXCFG, "cfg id:%d not register", id);
        return (INT32)TBOX_E_NOEXISTS;
    }

    fdb_blob_make(&blob, buf, cfg_info->len);
    if(0U == fdb_kv_get_blob(&cfg_fdb, cfg_info->name, &blob))
    {
        MODULE_LOG_E(TBOXCFG, "read cfg[%s] failed", cfg_info->name);
        return (INT32)TBOX_E_FAILED;
    }

    return (INT32)TBOX_E_OK;
}

INT32 tbox_cfg_set_default(TBOX_CFG_ID id)
{
    if(id >= TBOX_CFG_ITEM_NUMBER )
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(FALSE == cfg_items[id].used)
    {
        MODULE_LOG_E(TBOXCFG, "cfg id:%d not register", id);
        return (INT32)TBOX_E_NOEXISTS;
    }

    UINT8 *def_buff = mempool_alloc(TBOX_CFG_VALUE_MAX_LEN);
    if(NULL_PTR == def_buff)
    {
        MODULE_LOG_E(TBOXCFG, "failed to alloc memory");
        return (INT32)TBOX_E_FAILED_ALLOC;
    }

    CHAR *end_str;
    struct fdb_blob blob;
    TBOX_CFG_INFO *cfg_info = &cfg_items[id].info;
    if(CFG_TYPE_NUMBER == cfg_info->type)
    {
        UINT32 value = 0U;
        if(NULL_PTR != cfg_info->def_value && 
           strlen(cfg_info->def_value) > 0U)
        {
            value = (UINT32)strtol(cfg_info->def_value, &end_str, 10U);
            if(end_str == cfg_info->def_value)
            {
                MODULE_LOG_E(TBOXCFG, "invalid number:%s", cfg_info->def_value);
                mempool_free(def_buff); 
                return (INT32)TBOX_E_FAILED;
            }
        }
        fdb_blob_make(&blob, &value, cfg_info->len);
    }
    else if(CFG_TYPE_NUMBER16 == cfg_info->type)
    {
        UINT32 value = 0U;
        if(NULL_PTR != cfg_info->def_value && 
           strlen(cfg_info->def_value) > 0U)
        {
            value = (UINT32)strtol(cfg_info->def_value, &end_str, 16U);
            if(end_str == cfg_info->def_value)
            {
                MODULE_LOG_E(TBOXCFG, "invalid number:%s", cfg_info->def_value);
                mempool_free(def_buff); 
                return (INT32)TBOX_E_FAILED;
            }
        }
        fdb_blob_make(&blob, &value, cfg_info->len);
    }
    else if(CFG_TYPE_STRING == cfg_info->type)
    {
        memset(def_buff, 0U, TBOX_CFG_VALUE_MAX_LEN);
        if(NULL_PTR != cfg_info->def_value && 
           strlen(cfg_info->def_value) > 0U)
        {
            strncpy((CHAR*)def_buff, cfg_info->def_value, cfg_info->len);
        }        
        strncpy((CHAR*)def_buff, cfg_info->def_value, cfg_info->len);
        fdb_blob_make(&blob, def_buff, cfg_info->len);
    }
    else if(CFG_TYPE_BYTE == cfg_info->type)
    {
        memset(def_buff, 0U, TBOX_CFG_VALUE_MAX_LEN);
        if(NULL_PTR != cfg_info->def_value && 
           strlen(cfg_info->def_value) > 0U)
        {
            tbox_string_to_bytes(cfg_info->def_value, def_buff, cfg_info->len);
        }
        fdb_blob_make(&blob, def_buff, cfg_info->len);
    }
    else
    {
        mempool_free(def_buff);
        return (INT32)TBOX_E_FAILED;
    }
    
    if ((INT32)FDB_NO_ERR != fdb_kv_set_blob(&cfg_fdb, cfg_info->name, &blob)) 
    {
        MODULE_LOG_E(TBOXCFG, "failed to set kvdb value");
        mempool_free(def_buff); 
        return (INT32)TBOX_E_FAILED;
    }

    mempool_free(def_buff); 

    TBOX_CFG_CHANGE_INFO change_info;
    change_info.id = id;
    change_info.name = cfg_info->name;
    TBOX_MSG_DATA data = {.data = (UINT8 *)&change_info, .size = sizeof(change_info)};
    tbox_message_publish(TBOX_CFG_EVENT_VALUE_CHANGE, &data);

    return (INT32)TBOX_E_OK;     
}

INT32 tbox_cfg_setkv(CHAR *key, VOID *value, UINT16 value_len)
{
    INT32         ret;
    struct fdb_blob blob;

    if (NULL == key || NULL == value || 0U == value_len) 
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    ret = (INT32)fdb_kv_set_blob(&kv_fdb, key, fdb_blob_make(&blob, value, value_len));
    if (0 != ret) 
    {
        MODULE_LOG_E(TBOXCFG, "failed to set kvdb value");
        return (INT32) TBOX_E_FAILED;
    }

    /*TBOX_CFG_CHANGE_INFO change_info;
    change_info.id = CFG_ID_INVALID;
    change_info.name = key;
    TBOX_MSG_DATA data = {.data = (UINT8 *)&change_info, .size = sizeof(change_info)};
    tbox_message_publish(TBOX_CFG_EVENT_VALUE_CHANGE, &data);*/    
    return (INT32)TBOX_E_OK;     
}

INT32 tbox_cfg_getkv(CHAR *key, VOID *value, UINT16 value_len)
{
    INT32         ret;
    struct fdb_blob blob;

    if (NULL == key || NULL == value || 0U == value_len) 
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    ret = (int32_t)fdb_kv_get_blob(&kv_fdb, key, fdb_blob_make(&blob, value, value_len));
    if (ret <= 0 || ret > (int32_t)value_len) 
    {
        MODULE_LOG_E(TBOXCFG, "failed to get kvdb value");
        return (INT32) TBOX_E_FAILED;
    }
    return (INT32)TBOX_E_OK;     
}

static INT32 tbox_cfg_init(UINT8 seq)
{
    INT32 ret = (INT32)TBOX_E_OK;
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            ret = tbox_cfg_inner_init();
            if((INT32)TBOX_E_OK != ret)
            {
                ret = (INT32)TBOX_E_FAILED_INIT;
            }
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            TBOX_SHELL_REGISTER(lstcfg);
            TBOX_SHELL_REGISTER(setdefault);
            TBOX_SHELL_REGISTER(setcfg);
            break;
            
        default:
            break;
    }

    MODULE_LOG_D(TBOXCFG, "init seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID  tbox_cfg_exit(VOID)
{
    fdb_kvdb_deinit(&cfg_fdb);
    fdb_kvdb_deinit(&kv_fdb);
    MODULE_LOG_D(TBOXCFG, "cfg exit");
}

static VOID tbox_cfg_lock(fdb_kvdb_t db)
{
    DISABLE_INTERRUPT;
}

static VOID tbox_cfg_unlock(fdb_kvdb_t db)
{
    ENABLE_INTERRUPT;
}

static INT32 tbox_cfg_init_kvdb(VOID)
{
    UINT32 sec_size = 2048U;
    UINT32 fdb_size = sec_size * 16U;
    INT32  ret;

    fdb_kvdb_control(&cfg_fdb, FDB_KVDB_CTRL_SET_MAX_SIZE, (VOID *)(&fdb_size));
    fdb_kvdb_control(&cfg_fdb, FDB_KVDB_CTRL_SET_LOCK, (VOID *)(&tbox_cfg_lock));
    fdb_kvdb_control(&cfg_fdb, FDB_KVDB_CTRL_SET_UNLOCK, (VOID *)(&tbox_cfg_unlock));
    ret = (INT32)fdb_kvdb_init(&cfg_fdb, "cfg", "cfg");
    if (0 != ret)
    {
        return (INT32)TBOX_E_FAILED_INIT;
    }

    ret = (INT32)fdb_kvdb_check(&cfg_fdb);
    if (0 != ret) 
    {
        return (INT32)TBOX_E_FAILED;
    }

    fdb_kvdb_control(&kv_fdb, FDB_KVDB_CTRL_SET_MAX_SIZE, (VOID *)(&fdb_size));
    fdb_kvdb_control(&kv_fdb, FDB_KVDB_CTRL_SET_LOCK, (VOID *)(&tbox_cfg_lock));
    fdb_kvdb_control(&kv_fdb, FDB_KVDB_CTRL_SET_UNLOCK, (VOID *)(&tbox_cfg_unlock));
    ret = (INT32)fdb_kvdb_init(&kv_fdb, "kv", "kv");
    if (0 != ret)
    {
        return (INT32)TBOX_E_FAILED_INIT;
    }
    ret = (INT32)fdb_kvdb_check(&kv_fdb);
    if (0 != ret) 
    {
        return (INT32)TBOX_E_FAILED;
    }

    return (INT32)TBOX_E_OK;
}

static BaseType_t tbox_cfg_shell_lscfg(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    static INT32 cfg_index = 1;
    static UINT8 *cfg_buf = NULL_PTR;
    CHAR         *cfg_name;
    CHAR  priv_str[5] = " ";

    if(NULL_PTR == cfg_buf)
    {
        cfg_buf = mempool_alloc(TBOX_CFG_VALUE_MAX_LEN);
        if(NULL_PTR == cfg_buf)
        {
            snprintf(buf, bufsz, " %-16s : failed to alloc memory\r\n", "error");
            return pdFALSE;
        }
        strncpy(priv_str, "\r\n ", 4);
    }

    UINT8 index;
    for(index = 0U; index < TBOX_CFG_ITEM_NUMBER; index++)
    {
        if(FALSE == cfg_items[cfg_index].used)
        {
            cfg_index++;
            continue;
        }
        break;
    }
    if (cfg_index >= TBOX_CFG_ITEM_NUMBER) 
    {
        snprintf(buf, bufsz, "\r\n");
        cfg_index = 1;
        if(NULL_PTR != cfg_buf)
        {
          mempool_free(cfg_buf);
          cfg_buf = NULL_PTR;      
        }
        return pdFALSE;
    }

    cfg_name = cfg_items[cfg_index].info.name;
    if((INT32)TBOX_E_OK != tbox_cfg_read(cfg_index, cfg_buf))
    {
        snprintf(buf, bufsz, "%s%-16s : read failed\r\n", priv_str, cfg_name);
    }
    else
    {
        switch(cfg_items[cfg_index].info.type)
        {
            case CFG_TYPE_NUMBER:
                {
                    UINT32 value = 0U;
                    memcpy(&value, cfg_buf, cfg_items[cfg_index].info.len);
                    snprintf(buf, bufsz, "%s%-16s : %u\r\n", priv_str, cfg_name, value);
                }
                break;

            case CFG_TYPE_NUMBER16:
                {
                    UINT32 value = 0U;
                    memcpy(&value, cfg_buf, cfg_items[cfg_index].info.len);         
                    snprintf(buf, bufsz, "%s%-16s : 0x%X\r\n", priv_str, cfg_name, value);
                }
                break;

            case CFG_TYPE_STRING:
                {
                    snprintf(buf, bufsz, "%s%-16s : %s\r\n", priv_str, cfg_name, (CHAR *)cfg_buf);
                }
                break;

            case CFG_TYPE_BYTE:
                {
                    INT16  cfg_len = cfg_items[cfg_index].info.len;
                    snprintf(buf, bufsz, "%s%-16s : ", priv_str, cfg_name);
                    for(UINT8 i = 0U; i < cfg_len; i++)
                    {
                        snprintf(buf+strlen(buf), bufsz-strlen(buf), "%02X ", cfg_buf[i]);
                    }
                    snprintf(buf+strlen(buf), bufsz-strlen(buf), "\r\n");
                }
                break;

            default:
                snprintf(buf, bufsz, "%s%-16s : type invalid\r\n", priv_str, cfg_name);
                break;
        }
    }

    if (cfg_index++ >= TBOX_CFG_ITEM_NUMBER) 
    {
        snprintf(buf, bufsz, "\r\n");
        cfg_index = 1;
        if(NULL_PTR != cfg_buf)
        {
          mempool_free(cfg_buf);
          cfg_buf = NULL_PTR;      
        }
        return pdFALSE;
    }

    return pdTRUE;
}

static BaseType_t tbox_cfg_shell_setdefault(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    UNUSED(cmd);
    if((INT32)TBOX_E_OK == tbox_cfg_reset())
    {
        snprintf(buf, bufsz, "tbox config set default success\r\n");
    }
    else
    {
        snprintf(buf, bufsz, "tbox config set default failed\r\n");
    }
    return pdFALSE;
}

static BaseType_t tbox_cfg_shell_setcfg(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    CHAR *temp_buff = NULL_PTR;
    TBOX_CFG_INFO *cfg_info = NULL_PTR;
    CHAR *end_str;          
    const CHAR   *param_ptr;
    BaseType_t    param_len = 0;
    struct fdb_blob blob;
    UINT8 i;

    /*1、获取配置名字并查找对应配置项目信息*/
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL == param_ptr || param_len == 0) 
    {
        snprintf(buf, bufsz, "parse module name failed.\r\n");
        return pdFALSE;
    }
    temp_buff = mempool_alloc(TBOX_CFG_VALUE_MAX_LEN);
    if(NULL_PTR == temp_buff)
    {
        snprintf(buf, bufsz, "failed to alloc memory.\r\n");
        return pdFALSE;
    }
    strncpy(temp_buff, param_ptr, param_len);
    temp_buff[param_len] = '\0'; // 确保字符串终止
    tbox_string_toupper(temp_buff);
    for(i = 0U; i < TBOX_CFG_ITEM_NUMBER; i++)
    {
        if(FALSE == cfg_items[i].used)
        {
            continue;
        }
        if(0 == strncmp(temp_buff, cfg_items[i].info.name, param_len))
        {
            cfg_info = &cfg_items[i].info;
            break;
        }
    }
    if(NULL_PTR == cfg_info)
    {
        snprintf(buf, bufsz, "the module %s not found.\r\n", temp_buff);
        mempool_free(temp_buff);
        return pdFALSE;
    }

    /*获取配置值，并且根据配置类型转换对应的值*/
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param_len);
    if (NULL == param_ptr || param_len == 0) 
    {
        snprintf(buf, bufsz, "parse config value failed.\r\n");
        mempool_free(temp_buff);
        return pdFALSE;
    }

    if(CFG_TYPE_NUMBER == cfg_info->type)
    {
        UINT32 value = 0U;
        value = (UINT32)strtol(param_ptr, &end_str, 10U);
        if(end_str == param_ptr)
        {
            snprintf(buf, bufsz, "invalid number:%s.\r\n", param_ptr);
            mempool_free(temp_buff);
            return pdFALSE;
        }
        fdb_blob_make(&blob, &value, cfg_info->len);
    }
    else if(CFG_TYPE_NUMBER16 == cfg_info->type)
    {
        UINT32 value = 0U;
        value = (UINT32)strtol(param_ptr, &end_str, 16U);
        if(end_str == param_ptr)
        {
            snprintf(buf, bufsz, "invalid number:%s.\r\n", param_ptr);
            mempool_free(temp_buff);
            return pdFALSE;
        }
        fdb_blob_make(&blob, &value, cfg_info->len);
    }
    else if(CFG_TYPE_STRING == cfg_info->type)
    {
        if(param_len >= cfg_info->len)
        {
            snprintf(buf, bufsz, "the string:%s is too long.\r\n", param_ptr);
            mempool_free(temp_buff);
            return pdFALSE;
        }
        memset(temp_buff, 0U, TBOX_CFG_VALUE_MAX_LEN);
        strncpy(temp_buff, param_ptr, param_len);
        fdb_blob_make(&blob, temp_buff, cfg_info->len);
    }
    else if(CFG_TYPE_BYTE == cfg_info->type)
    {
        memset(temp_buff, 0U, TBOX_CFG_VALUE_MAX_LEN);
        if(cfg_info->len != tbox_string_to_bytes(param_ptr, (UINT8 *)temp_buff, TBOX_CFG_VALUE_MAX_LEN))
        {
            snprintf(buf, bufsz, "the data:%s lenght is invalid.\r\n", param_ptr);
            mempool_free(temp_buff);
            return pdFALSE;
        }
        fdb_blob_make(&blob, temp_buff, cfg_info->len);
    }
    else
    {
        snprintf(buf, bufsz, "the config type is invaliad.\r\n");
        mempool_free(temp_buff);
        return pdFALSE;
    }
    
    fdb_kv_set_blob(&cfg_fdb, cfg_info->name, &blob); 

    mempool_free(temp_buff);

    snprintf(buf, bufsz, "set config:%s to value:%s success.\r\n", cfg_info->name, param_ptr);

    /*发送通知消息*/
    TBOX_CFG_CHANGE_INFO change_info;
    change_info.id = i;
    change_info.name = cfg_info->name;
    TBOX_MSG_DATA data = {.data = (UINT8 *)&change_info, .size = sizeof(change_info)};
    tbox_message_publish(TBOX_CFG_EVENT_VALUE_CHANGE, &data);      
    return pdFALSE;    
}

static VOID cfg_reg_all_define_item(VOID)
{
    TBOX_CFG_REG(MAGICNO);
    TBOX_CFG_REG(DEVICEID);
    TBOX_CFG_REG(TRACECODE);
    TBOX_CFG_REG(SEID);
    TBOX_CFG_REG(BATTYPE);
    TBOX_CFG_REG(SLEEPMODE);
    TBOX_CFG_REG(SLEEPDELAY);
    TBOX_CFG_REG(RTCENABLE);
    TBOX_CFG_REG(RTCINTV);
    TBOX_CFG_REG(CAN1BAUD);
    TBOX_CFG_REG(CAN2BAUD);
    TBOX_CFG_REG(CAN3BAUD);
    TBOX_CFG_REG(CAN1AUTO);
    TBOX_CFG_REG(CAN2AUTO);
    TBOX_CFG_REG(CAN3AUTO);
    TBOX_CFG_REG(TIMEZONE);
    TBOX_CFG_REG(PUBAPN);
    TBOX_CFG_REG(PRIAPN);
    TBOX_CFG_REG(OTAAPN);
    TBOX_CFG_REG(SMSCENTER);
    TBOX_CFG_REG(GPSMODE);
    TBOX_CFG_REG(VIN);
    TBOX_CFG_REG(RESETCOUNT);
    TBOX_CFG_REG(RTCPOR);

    TBOX_CFG_REG(GBFURL);
    TBOX_CFG_REG(GBFPORT);
    TBOX_CFG_REG(HPMMURL);
    TBOX_CFG_REG(HPMMIP);
    TBOX_CFG_REG(HPMMPORT);
    TBOX_CFG_REG(HPMSURL);
    TBOX_CFG_REG(HPMSIP);
    TBOX_CFG_REG(HPMSPORT);
    TBOX_CFG_REG(HPMHTBT);
    TBOX_CFG_REG(HPMCYCON);
    TBOX_CFG_REG(HPMCYCOFF);
    TBOX_CFG_REG(HPMSLPDY);
    TBOX_CFG_REG(HPMSVRINTV);
}

static INT32 tbox_cfg_inner_init(VOID)
{
    for(UINT32 i = 0U; i < TBOX_CFG_ITEM_NUMBER; i++)
    {
        cfg_items[i].used = FALSE;
        cfg_items[i].info.name = NULL;
        cfg_items[i].info.def_value = NULL;
        cfg_items[i].info.len = 0U;
        cfg_items[i].info.type = CFG_TYPE_INVALID;
    }

    /*注册默认配置项*/
    cfg_reg_all_define_item();

    /*初始化KVDB*/
    INT32 ret = tbox_cfg_init_kvdb();
    if ((INT32)TBOX_E_OK != ret) 
    {
        MODULE_LOG_E(TBOXCFG, "init failed");
        return ret;
    }

    UINT32  magic = 0U;
    CHAR *end_str;
    struct fdb_blob blob;
    fdb_blob_make(&blob, &magic, sizeof(magic));
    ret = (INT32)fdb_kv_get_blob(&cfg_fdb, CFG_MAGEIC_NAME, &blob);
    if (((INT32)sizeof(magic) != ret) || (CFG_MAGEIC_VALUE != magic)) 
    {
        /*设置DB默认值*/
        ret = (INT32)fdb_kv_set_default(&cfg_fdb);
        if ((INT32)FDB_NO_ERR != (INT32)ret) 
        {
            MODULE_LOG_E(TBOXCFG, "set cfg lvdb default failed");
            return (INT32)TBOX_E_FAILED;
        }

        UINT8 *def_buff = mempool_alloc(TBOX_CFG_VALUE_MAX_LEN);
        if(NULL_PTR == def_buff)
        {
            MODULE_LOG_E(TBOXCFG, "failed to alloc memory");
            return (INT32)TBOX_E_FAILED_ALLOC;
        }

        /*设置默认值*/
        for (UINT8 i = 0U;  i < TBOX_CFG_ITEM_NUMBER; i++) 
        {
            if(FALSE == cfg_items[i].used)
            {
                continue;
            }

            if(CFG_TYPE_NUMBER == cfg_items[i].info.type)
            {
                UINT32 value = 0U;
                if(NULL_PTR != cfg_items[i].info.def_value &&
                  strlen(cfg_items[i].info.def_value) > 0U)
                {
                    value = (UINT32)strtol(cfg_items[i].info.def_value, &end_str, 10U);
                    if(end_str == cfg_items[i].info.def_value)
                    {
                        MODULE_LOG_E(TBOXCFG, "invalid number:%s", cfg_items[i].info.def_value);
                        continue;
                    }
                }
                fdb_blob_make(&blob, &value, cfg_items[i].info.len);
            }
            else if(CFG_TYPE_NUMBER16 == cfg_items[i].info.type)
            {
                UINT32 value = 0U;
                if(NULL_PTR != cfg_items[i].info.def_value &&
                  strlen(cfg_items[i].info.def_value) > 0U)
                {
                    value = (UINT32)strtol(cfg_items[i].info.def_value, &end_str, 16U);
                    if(end_str == cfg_items[i].info.def_value)
                    {
                        MODULE_LOG_E(TBOXCFG, "invalid number:%s", cfg_items[i].info.def_value);
                        continue;
                    }
                }
                fdb_blob_make(&blob, &value, cfg_items[i].info.len);
            }
            else if(CFG_TYPE_STRING == cfg_items[i].info.type)
            {
                memset(def_buff, 0U, TBOX_CFG_VALUE_MAX_LEN);
                if(NULL_PTR != cfg_items[i].info.def_value &&
                  strlen(cfg_items[i].info.def_value) > 0U)
                {
                    strncpy((CHAR*)def_buff, cfg_items[i].info.def_value, cfg_items[i].info.len);
                }
                fdb_blob_make(&blob, def_buff, cfg_items[i].info.len);
            }
            else if(CFG_TYPE_BYTE == cfg_items[i].info.type)
            {
                memset(def_buff, 0U, TBOX_CFG_VALUE_MAX_LEN);
                if(NULL_PTR != cfg_items[i].info.def_value &&
                  strlen(cfg_items[i].info.def_value) > 0U)
                {
                    tbox_string_to_bytes(cfg_items[i].info.def_value, def_buff, cfg_items[i].info.len);
                }
                fdb_blob_make(&blob, def_buff, cfg_items[i].info.len);
            }
            else
            {
                continue;
            }
            
            ret = (INT32)fdb_kv_set_blob(&cfg_fdb, cfg_items[i].info.name, &blob);
            if ((INT32)FDB_NO_ERR != ret) 
            {
                MODULE_LOG_E(TBOXCFG, "set error:%d,%d", ret, i);
                continue;
            }
        }

        mempool_free(def_buff);
    }

    /*更新重启次数*/
    UINT32 reset;
    fdb_blob_make(&blob, &reset, sizeof(reset));
    ret = (INT32)fdb_kv_get_blob(&cfg_fdb, "RESETCOUNT", &blob);
    if(sizeof(reset) != (UINT64)ret) 
    {
        reset = 0U;
    } 
    else 
    {
        reset += 1U;
    }
    fdb_blob_make(&blob, &reset, sizeof(reset));
    ret = (INT32)fdb_kv_set_blob(&cfg_fdb, "RESETCOUNT", &blob);
    return ret;
}