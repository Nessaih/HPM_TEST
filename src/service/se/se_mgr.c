#include "tbox_common.h"
#include "tbox_core.h"
#include "stimer.h"
#include "tbox_cfg_if.h"

#include "vse.h"
#include "se_if.h"
#include "se.h"
#include "se_mgr.h"
#include "vse_cfg.h"

#define SM2_MSG_MAX_SIZE 		(1024)
#define SE_MGR_CYCLE_INTV		(1*1000)

#define SE_MGR_INIT_CNT_MAX		(3)
#define SE_MGR_RINT_ITV_MAX		(3*1000/SE_MGR_CYCLE_INTV)

typedef enum
{
	SE_MGR_DEV_IDLE 	= 0,
	SE_MGR_DEV_OPEN 	= 1,
	SE_MGR_DEV_CLOSE 	= 2,
}SE_MGR_DEV_STA;

typedef struct
{
	INT32 init_cnt;
	INT32 rint_itv;
	SE_MGR_DEV_STA sta;
}SE_MGR_DEV_INFO_T;

typedef struct
{
	BOOL	flag;
	UINT8	sm2za[SM2_PREP_ZA_SIZE];
}SE_MGR_SM2_ZA;

static STIMER_ID 		se_mgr_timer;
static SE_MGR_SM2_ZA	se_mgr_sm2_za;
static UINT8         	se_mgr_uid[TBOX_CFG_SEID_LEN];

static SE_MGR_DEV_INFO_T se_mgr_dev_info;

static INT32 se_mgr_gen_za(VOID)
{
	INT32  ret;
    UINT8  pub[SM2_PUB_KEY_SIZE] = {0};
    UINT16 len                   = SM2_PUB_KEY_SIZE;
	
	ret = vse_get_sm2key(pub, &len);
	if (ret == VSE_SUCCESS)
    {
        MODULE_LOG_I(SE, "VSE get sm2 pubilc key success.\n");
		memset(&se_mgr_sm2_za, 0, sizeof(se_mgr_sm2_za));

        len = SM2_PREP_ZA_SIZE;
        ret = vse_do_sm2prep(se_mgr_uid, SM2_USER_ID_SIZE, pub, SM2_PUB_KEY_SIZE, se_mgr_sm2_za.sm2za, &len);

        if (ret == VSE_SUCCESS)
        {
            se_mgr_sm2_za.flag = TRUE;
        }
        else
        {
            MODULE_LOG_E(SE, "VSE sm2prep failed.\n");
            return -1;
        }
    }
    else
    {
        MODULE_LOG_E(SE, "VSE get sm2 pubilc key failed.\n");
        return -1;
    }
	
	
	return 0;
}


static INT32 se_mgr_vse_init(VOID)
{
	INT32 ret = 0;
	
    vse_gpio_reset();
	ret |= vse_deinit();
	tbox_log_print("[ret 1] %d\r\n", ret);
	ret |= vse_init();
	tbox_log_print("[ret 2] %d\r\n", ret);
	ret |= vse_connect();	
	tbox_log_print("[ret 3] %d\r\n", ret);
    return ret;
}

INT32 se_mgr_vse_deinit(VOID)
{
	INT32 ret = 0;
	if(SE_MGR_DEV_OPEN != se_mgr_dev_info.sta)
	{
		return ret;
	}
	
    ret |= vse_disconnect();
    ret |= vse_deinit();
	if(VSE_SUCCESS == ret)
	{
		se_mgr_dev_info.sta = SE_MGR_DEV_CLOSE;
	}
	
    return ret;
}

static VOID se_mgr_uid_init(VOID)
{
	INT32 ret = 0;
	memset(se_mgr_uid, 0, sizeof(se_mgr_uid));
	
	TBOX_CFG_ID cfg_id;
	TBOX_CFG_ID_GET(SEID, cfg_id);
	ret = tbox_cfg_read(cfg_id, se_mgr_uid);
	if(0 != ret)
	{
		MODULE_LOG_E(SE, "se mgr uid init failed, ret: %d", ret);
	}

	return;
}

VOID se_mgr_timeout(VOID)
{
	INT32 ret = 0;
	MODULE_LOG_I(SE, "se mgr, init:%d, itv: %d, sta: %d\r\n", se_mgr_dev_info.init_cnt, se_mgr_dev_info.rint_itv, se_mgr_dev_info.sta);

	if(SE_MGR_DEV_OPEN == se_mgr_dev_info.sta)
	{
		return;
	}
	
	if(se_mgr_dev_info.init_cnt > 0)
	{
		if(se_mgr_dev_info.rint_itv > 0)
		{
			se_mgr_dev_info.rint_itv--;
		}
		else
		{			
			ret = se_mgr_vse_init();
			if(0 == ret)
			{
				se_mgr_gen_za();
				se_mgr_dev_info.sta = SE_MGR_DEV_OPEN;
				se_mgr_dev_info.init_cnt = 0;
				se_mgr_dev_info.rint_itv = 0;
				return;
			}
			
			se_mgr_dev_info.init_cnt--;
			se_mgr_dev_info.rint_itv = SE_MGR_RINT_ITV_MAX;
		}
	}

	return;
}


static VOID tbox_mgr_timer_callback(VOID)
{
	TBOX_ID module_id;
    GET_TBOX_MODULE_ID(SE, module_id);

    tbox_message_send(SE_TBOX_TIMER_EVENT, module_id, module_id, NULL_PTR); 
}


INT32 se_mgr_init(UINT8 seq)
{
	INT32 ret = 0;
	switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
			memset(&se_mgr_dev_info, 0, sizeof(se_mgr_dev_info));
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
			se_mgr_uid_init();
			ret = se_mgr_vse_init();
			if(0 == ret)
			{
				se_mgr_gen_za();
				se_mgr_dev_info.sta = SE_MGR_DEV_OPEN;
			}
			else
			{
				se_mgr_dev_info.init_cnt = SE_MGR_INIT_CNT_MAX;
				se_mgr_dev_info.rint_itv = SE_MGR_RINT_ITV_MAX;
			}
            se_mgr_timer = stimer_create(STIMER_TYPE_PERIOD, tbox_mgr_timer_callback);
    		stimer_start(se_mgr_timer, SE_MGR_CYCLE_INTV);
            break;
            
        default:
            break;
    }

	return 0;
}

VOID se_mgr_wake(VOID)
{
	INT32 ret = 0;
	memset(&se_mgr_dev_info, 0, sizeof(se_mgr_dev_info));
	
	stimer_start(se_mgr_timer, SE_MGR_CYCLE_INTV);
	ret = se_mgr_vse_init();
	if(0 == ret)
	{
		se_mgr_gen_za();
		se_mgr_dev_info.sta = SE_MGR_DEV_OPEN;
	}
	else
	{
		se_mgr_dev_info.init_cnt = SE_MGR_INIT_CNT_MAX;
		se_mgr_dev_info.rint_itv = SE_MGR_RINT_ITV_MAX;
	}
}

VOID se_mgr_sleep(VOID)
{
	se_mgr_vse_deinit();
	stimer_stop(se_mgr_timer);	
	se_mgr_dev_info.sta = SE_MGR_DEV_CLOSE;
}


/****************** SE SERVER API ******************/

INT32 se_get_uid(UINT8 *data, UINT16 len)
{
	if(len > TBOX_CFG_SEID_LEN || NULL == data)
	{
		MODULE_LOG_E(SE, "se get uid len err");
		return -1;
	}
	
    memcpy(data, se_mgr_uid, len);
	return 0;
}

INT32 se_set_uid(UINT8 *data, UINT16 len)
{
	INT32 		ret = 0;
	TBOX_CFG_ID cfg_id;
	
	if(len > TBOX_CFG_SEID_LEN || NULL == data)
	{
		MODULE_LOG_E(SE, "se set uid len err");
		return -1;
	}
	
	TBOX_CFG_ID_GET(SEID, cfg_id);
	ret = tbox_cfg_write(cfg_id, data);
	if(0 != ret)
	{
		MODULE_LOG_E(SE, "se set uid failed, ret: %d", ret);
		return -1;
	}

	memset(se_mgr_uid, 0, sizeof(se_mgr_uid));
	memcpy(se_mgr_uid, data, len);
	se_mgr_gen_za();

	return ret;
}

INT32 se_get_version(UINT8 *ver, UINT16 *data_len)
{
	INT32  ret = 0;
	UINT8  se_ver[VSE_VERSION_SIZE] = {0};
	UINT16 len = VSE_VERSION_SIZE;

	if(NULL == ver)
	{
		MODULE_LOG_E(SE, "se get version: null pointer");
		return -1;
	}

	if(SE_MGR_DEV_OPEN != se_mgr_dev_info.sta)
	{
		return -1;
	}
	
	ret = vse_get_ver(VERSION_TYPE_SDK, se_ver, &len);
	if(0 != ret)
	{
		MODULE_LOG_E(SE, "se get ver failed, ret: %d", ret);
	}

	memcpy(ver, se_ver, len);
	*data_len = len;
	
	return 0;
}

INT32 se_get_pubkey(UINT8 *data, UINT16 *len)
{
    INT32  ret = -1;
	UINT16 key_len;

	if (data == NULL || len == NULL) {
		MODULE_LOG_E(SE, "se get pubkey: null pointer");
		return -1;
	}

	if(SE_MGR_DEV_OPEN != se_mgr_dev_info.sta)
	{
		return -1;
	}

	key_len = *len;
    ret = vse_get_sm2key(data, &key_len);
    if (0 != ret)
    {
        MODULE_LOG_E(SE, "se get pubkey error: %d\r\n", ret);
		return -1;
    }

	*len = key_len;
    return ret;
}
INT32 se_get_real_chip_id(UINT8 *data, UINT16 *len)
{
    INT32 ret = -1;
	UINT16 id_len;

	if(SE_MGR_DEV_OPEN != se_mgr_dev_info.sta)
	{
		return -1;
	}
	
	id_len = *len;
    ret = vse_get_did(data, len);
    if (0 != ret)
    {
        MODULE_LOG_E(SE, "se get chip id  error: %d\r\n", ret);
		return -1;
    }

	*len = id_len;
    return ret;
}

INT32 se_get_signature(UINT8 *idata, INT32 ilen, UINT8 *odata, UINT16 *olen)
{
    INT32 ret = -1;
    UINT16       sign_len;
    static UINT8 sm2_msg[SM2_MSG_MAX_SIZE] = {0};

	if(SE_MGR_DEV_OPEN != se_mgr_dev_info.sta)
	{
		return -1;
	}

	if(FALSE == se_mgr_sm2_za.flag)
	{
		ret = se_mgr_gen_za();
		if(0 != ret)
		{
			return ret;
		}
	}

	memcpy(sm2_msg, se_mgr_sm2_za.sm2za, SM2_PREP_ZA_SIZE);
    memcpy(sm2_msg + SM2_PREP_ZA_SIZE, idata, ilen);
    sign_len = *olen;
	
    ret = vse_do_sm2sign(sm2_msg, SM2_PREP_ZA_SIZE + ilen, odata, &sign_len);
    if (0 != ret)
    {
        MODULE_LOG_E(SE, " se signature error: %d\r\n", ret);
    }

    return ret;
}

INT32 se_verify_signature(UINT8 *idata, INT32 ilen, UINT8 *sdata)
{
	UNUSED(idata);
	UNUSED(ilen);
	UNUSED(sdata);
	
    return 0;
}


