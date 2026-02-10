#include "tbox_common.h"
#include "tbox_cfg_if.h"
#include "tbox_log.h"
#include "analog_if.h"
#include "macros.h"
#include "version.h"
#include "gnss_if.h"
#include "4g_if.h"

#include "fct_cmd.h"

typedef INT8 (*FCT_CMD_CMD_PROC)(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len);

typedef struct
{
    CHAR         	*name;
    FCT_CMD_CMD_PROC proc;
} FCT_CMD_CMD;

static FCT_PM_ACTION fct_pm_action = FCT_PM_ACTION_INVALID;

static INT8  fct_cmd_startup_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static INT8  fct_cmd_4gsignal_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UINT8 ret = 0;
	UNUSED(msg);
	
	ret = if_4g_get_signal_signalstrength();
	if(ret >= 99)
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+signal:%d\r\n", ret);
    return 0;
}

static INT8  fct_cmd_iccid_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UINT8 iccid[IF_4G_MAX_ICCID_LEN] = {0};
	UINT8 len = sizeof(iccid);
	UNUSED(msg);

	if(FALSE == if_4g_get_iccid(iccid, &len))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)iccid))
	{
		return -1;
	}
	
	*out_len = snprintf(res, res_len, "+iccid:%s\r\n", iccid);

    return 0;
}

static INT8  fct_cmd_imei_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UINT8 imei[IF_4G_MAX_IMEI_LEN] = {0};
	UINT8 len = IF_4G_MAX_IMEI_LEN;
	CHAR *temp_ptr = res;
	UINT8 index;

	*out_len = 0;
	
	if(FALSE == if_4g_get_imei(imei, &len))
	{
		return -1;
	}

	strcpy(temp_ptr, "+imei:");
	*out_len = strlen("+imei:");
	temp_ptr += *out_len;

	for(index = 0; index < len; index++)
	{
		*temp_ptr = imei[index] + '0';
		temp_ptr += 1;
		*out_len += 1;
	}

	strcpy(temp_ptr, "\r\n");
	*out_len += strlen("\r\n");

	return 0;
}

static INT8  fct_cmd_set_telno_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UINT8  index = 0;
	UINT8  telno[IF_4G_MAX_PHONE_NUM_LEN+1] = {0};
	UINT16 len = strlen("fctsettelno:telno=\r\n")+IF_4G_MAX_PHONE_NUM_LEN;

	if(strlen(msg) > len)
	{
		return -1;
	}

	if(sscanf(msg, "fctsettelno:telno=%s\r\n", (char *)telno))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)telno))
	{
		return -1;
	}

	for(index = 0; index < strlen((CHAR *)telno); index++)
	{
		if(!isdigit(telno[index]))
		{
			return -1;
		}
	}
	
	if(FALSE == if_4g_set_phone_num(telno))
	{
		return -1;
	}

	if_4g_query_phone_num();
	
    return 0;
}

static INT8  fct_cmd_get_telno_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UINT8 telno[IF_4G_MAX_PHONE_NUM_LEN+1] = {0};
	UINT8 len = sizeof(telno);
	UNUSED(msg);

	if(FALSE == if_4g_get_phone_num(telno, &len))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)telno))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+telno:%s\r\n", telno);
	
    return 0;
}


static INT8  fct_cmd_4gnet_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UNUSED(msg);

	if(IF_4G_STATE_CONNECTED != if_4g_get_call_state(IF_4G_PUBLIC_APN))
	{
		return -1;
	}

	return 0;
}

static INT8  fct_cmd_4gant_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UNUSED(msg);
	
	if (ANT_NORMAL != analog_lte_ant_status())
    {
        return -1;
    }
	
    return 0;
}

static INT8  fct_cmd_gnssant_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    UNUSED(msg);
	
	if (ANT_NORMAL != analog_gps_ant_status())
    {
        return -1;
    }
	
    return 0;
}

static INT8  fct_cmd_gnssfix_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UNUSED(msg);
	
	if(GNSS_POS_STATE_FIX != gnss_get_fix_state())
	{
		return -1;
	}
	
    return 0;
}

static INT8  fct_cmd_rtc_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	//TODO: get rtc time
    return -1;
}

static INT8  fct_cmd_setcan_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static INT8  fct_cmd_can_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static INT8  fct_cmd_can0_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static INT8  fct_cmd_can1_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static INT8  fct_cmd_can2_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static INT8  fct_cmd_mainpm_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UNUSED(msg);
	*out_len = snprintf(res, res_len, "+mpower:%d\r\n", analog_pwr_vtg());
    return 0;
}

static INT8  fct_cmd_batpm_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UNUSED(msg);
	*out_len = snprintf(res, res_len, "+batvol:%d\r\n", analog_bat_vtg());
    return 0;
}

static INT8  fct_cmd_battmp_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	UNUSED(msg);
	*out_len = snprintf(res, res_len, "+battmp:%d\r\n", analog_bat_tmp());
	return 0;
}

static INT8  fct_cmd_sleep_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    fct_pm_action = FCT_PM_ACTION_SLEEP;
    return 0;
}

static INT8  fct_cmd_setsn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	TBOX_CFG_ID cfg_id;
    UINT8 sn[TBOX_CFG_DEVICEID_LEN] = {0};
	UINT16 len = strlen("fctsetsn:sn=\r\n")+TBOX_CFG_DEVICEID_LEN;

	if(strlen(msg) > len)
	{
		return -1;
	}

	if(-1 == sscanf(msg, "fctsetsn:sn=%s\r\n", (CHAR *)sn))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(DEVICEID, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, sn))
	{
		return -1;
	}
	
    return 0;
}

static INT8  fct_cmd_getsn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	TBOX_CFG_ID cfg_id;
    UINT8 sn[TBOX_CFG_DEVICEID_LEN] = {0};

	
	TBOX_CFG_ID_GET(DEVICEID, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, sn))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+sn:%s\r\n", sn);
    return 0;
}

static INT8  fct_cmd_io_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	//TODO
    return -1;
}

static INT8  fct_cmd_set_trace_code_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	TBOX_CFG_ID cfg_id;
    UINT8 trace_code[TBOX_CFG_TRACECODE_LEN] = {0};
	UINT16 len = strlen("fctsettcode:code=\r\n")+TBOX_CFG_TRACECODE_LEN;
	if(strlen(msg) > len)
	{
		return -1;
	}

	if(-1 == sscanf(msg, "fctsettcode:code=%s\r\n", (CHAR *)trace_code))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(TRACECODE, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, trace_code))
	{
		return -1;
	}
		
    return 0;
}

static INT8  fct_cmd_get_trace_code_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8 trace_code[TBOX_CFG_TRACECODE_LEN] = {0};

	
	TBOX_CFG_ID_GET(TRACECODE, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, trace_code))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+code:%s\r\n", trace_code);
    return 0;
}

static INT8  fct_cmd_setdefault_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	if(0 != tbox_cfg_reset())
	{
		return -1;
	}
	
    return 0;
}


static INT8  fct_cmd_flash_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	//TODO: 服务层暂不支持
	return -1;
}

static INT8  fct_cmd_eeprom_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	//TODO: 不支持
	
    return -1;
}

static INT8  fct_cmd_efs_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
#if 0
    UINT8 ret = 0;
	UINT8 test_data[10] = {0xAB, 0xFA, 0xAF, 0xAC, 0x71, 0xC7, 0x89, 0xDA, 0x65, 0x46};
    UINT8 rcv_data[10]  = {0};

	ret = drv_flash_nor_write(EXFLASH_ADDR_FCT_TEST, test_data, sizeof(test_data));
	if(0 != ret)
	{
		return -1;
	}

	ret = drv_flash_nor_read(EXFLASH_ADDR_FCT_TEST, rcv_data, sizeof(rcv_data));
	if(0 != ret)
	{
		return -1;
	}

	if(0 != memcmp(test_data, rcv_data, sizeof(rcv_data)))
	{
		return -1;
	}
	
    return 0;
#else
	return -1;
#endif
}

static FCT_CMD_CMD fct_cmd_table[] = {
    {"fctstartup",     fct_cmd_startup_proc       },
    {"fct4gsignal",    fct_cmd_4gsignal_proc      },
    {"fcticcid",       fct_cmd_iccid_proc         },
    {"fctimei",        fct_cmd_imei_proc          },
    {"fctsettelno",    fct_cmd_set_telno_proc      },
    {"fctgettelno",    fct_cmd_get_telno_proc      },
    {"fct4gant",       fct_cmd_4gant_proc         },    
    {"fct4gnet",       fct_cmd_4gnet_proc         },
    {"fctgnssant",     fct_cmd_gnssant_proc       },
    {"fctgnssfix",     fct_cmd_gnssfix_proc       },
    {"fctrtc",         fct_cmd_rtc_proc           },
    {"fctsetcan",      fct_cmd_setcan_proc        },
    {"fctcanloop",     fct_cmd_can_proc           },
    {"fctcan0",        fct_cmd_can0_proc          },
    {"fctcan1",        fct_cmd_can1_proc          },
    {"fctcan2",        fct_cmd_can2_proc          },
    {"fctrmainpm",     fct_cmd_mainpm_proc        },
    {"fctbatpm",	   fct_cmd_batpm_proc         },
    {"fctbattmp",      fct_cmd_battmp_proc        },
    {"fctsleep",       fct_cmd_sleep_proc         },
    {"fctsetsn",       fct_cmd_setsn_proc         },
    {"fctgetsn",       fct_cmd_getsn_proc         },
    {"fctsettcode",    fct_cmd_set_trace_code_proc},
    {"fctgettcode",    fct_cmd_get_trace_code_proc},
    {"fctio",          fct_cmd_io_proc            },
    {"fctflash",       fct_cmd_flash_proc         },
    {"fctefs",         fct_cmd_efs_proc           }, 
    {"fcteeprom",      fct_cmd_eeprom_proc        },
    
    {"fctsetdefault",  fct_cmd_setdefault_proc    },
};

static INT8 fct_eol_btlver_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	const CHAR *ver = NULL;
	UNUSED(msg);
	
	ver = version_get(VERSION_TYPE_BOOT);

	if(0 == strlen(ver))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+mcubtlver:%.*s\r\n", (int)strlen(ver), ver);
    return 0;
}

static INT8 fct_eol_appver_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    const CHAR *ver = NULL;
	UNUSED(msg);
	
	ver = version_get(VERSION_TYPE_APP);

	if(0 == strlen(ver))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+mcuappver:%.*s\r\n", (int)strlen(ver), ver);
    return 0;
}

static INT8 fct_eol_get_pubapn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8 apn[TBOX_CFG_APN_LEN] = {0};

	TBOX_CFG_ID_GET(PUBAPN, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, apn))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+pubapn:%s\r\n", apn);
    return 0;
}

static INT8 fct_eol_set_pubapn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8  apn[TBOX_CFG_APN_LEN] = {0};
	UINT16 len = strlen("eolsetpubapn:apn=\r\n")+TBOX_CFG_APN_LEN;

	if(strlen(msg) > len)
	{
		return -1;
	}

	if(-1 == sscanf(msg, "eolsetpubapn:apn=%63s\r\n", (CHAR *)apn))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(PUBAPN, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, apn))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_priapn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8 apn[TBOX_CFG_APN_LEN] = {0};

	TBOX_CFG_ID_GET(PRIAPN, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, apn))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+priapn:%s\r\n", apn);
    return 0;
}

static INT8 fct_eol_set_priapn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8  apn[TBOX_CFG_APN_LEN] = {0};
	UINT16 len = strlen("eolsetpriapn:apn=\r\n")+TBOX_CFG_APN_LEN;

	if(strlen(msg) > len)
	{
		return -1;
	}

	if(-1 == sscanf(msg, "eolsetpriapn:apn=%63s\r\n", (CHAR *)apn))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(PRIAPN, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, apn))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_otaapn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8 apn[TBOX_CFG_APN_LEN] = {0};

	TBOX_CFG_ID_GET(OTAAPN, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, apn))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+otaapn:%s\r\n", apn);
    return 0;
}

static INT8 fct_eol_set_otaapn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8  apn[TBOX_CFG_APN_LEN] = {0};
	UINT16 len = strlen("eolsetotaapn:apn=\r\n")+TBOX_CFG_APN_LEN;

	if(strlen(msg) > len)
	{
		return -1;
	}

	if(-1 == sscanf(msg, "eolsetotaapn:apn=%63s\r\n", (CHAR *)apn))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(OTAAPN, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, apn))
	{
		return -1;
	}
	
    return 0;
}


static INT8 fct_eol_get_can1baud_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT32 baud = 0;

	TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, &baud))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+can1baud:%d\r\n", baud);
	return 0;
}

static INT8 fct_eol_get_can2baud_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT32 baud = 0;

	TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, &baud))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+can2baud:%d\r\n", baud);
	return 0;
}

static INT8 fct_eol_get_can3baud_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT32 baud = 0;

	TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, &baud))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+can3baud:%d\r\n", baud);
	return 0;
}

static INT8 fct_eol_set_can1baud_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	TBOX_CFG_ID cfg_id;
	UINT32 baud = 0;

	if(-1 == sscanf((char *)msg, "eolsetcan1baud:baud=%d\r\n", &baud))
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &baud))
	{
		//TODO: 重新初始化波特率
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_set_can2baud_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	TBOX_CFG_ID cfg_id;
	UINT32 baud = 0;

	if(-1 == sscanf((char *)msg, "eolsetcan2baud:baud=%d\r\n", &baud))
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &baud))
	{
		//TODO: 重新初始化波特率
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_set_can3baud_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	TBOX_CFG_ID cfg_id;
	UINT32 baud = 0;

	if(-1 == sscanf((char *)msg, "eolsetcan3baud:baud=%d\r\n", &baud))
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &baud))
	{
		//TODO: 重新初始化波特率
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_setsn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8 sn[TBOX_CFG_DEVICEID_LEN] = {0};
	UINT16 len = strlen("eolsetsn:sn=\r\n")+TBOX_CFG_DEVICEID_LEN;

	if(strlen(msg) > len)
	{
		return -1;
	}

	if(-1 == sscanf(msg, "eolsetsn:sn=%s\r\n", (CHAR *)sn))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)sn))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(DEVICEID, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, sn))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_getsn_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8 sn[TBOX_CFG_DEVICEID_LEN] = {0};

	
	TBOX_CFG_ID_GET(DEVICEID, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, sn))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)sn))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+sn:%s\r\n", sn);
    return 0;
}

static INT8 fct_eol_set_trace_code_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8 trace_code[TBOX_CFG_TRACECODE_LEN] = {0};
	UINT16 len = strlen("eolsettcode:code=\r\n")+TBOX_CFG_TRACECODE_LEN;
	if(strlen(msg) > len)
	{
		return -1;
	}

	if(-1 == sscanf(msg, "eolsettcode:code=%s\r\n", (CHAR *)trace_code))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)trace_code))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(TRACECODE, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, trace_code))
	{
		return -1;
	}
		
    return 0;
}

static INT8 fct_eol_get_trace_code_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT8 trace_code[TBOX_CFG_TRACECODE_LEN] = {0};

	
	TBOX_CFG_ID_GET(TRACECODE, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, trace_code))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)trace_code))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+code:%s\r\n", trace_code);
    return 0;
}

static INT8 fct_eol_setdefault_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    if(0 != tbox_cfg_reset())
	{
		return -1;
	}

	return 0;
}

static INT8 fct_eol_get_gbf_ip_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_URL_LEN] = {0};
	
	TBOX_CFG_ID_GET(GBFURL, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, url))
	{
		return -1;
	}

	if(0 == strlen(url))
	{
		return -1;
	}
	
	*out_len = snprintf(res, res_len, "+gbfip:%s\r\n", url);
    return 0;
}

static INT8 fct_eol_set_gbf_ip_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_URL_LEN] = {0};
	UINT16 len = strlen("eolsetgbfip:ip=\r\n")+TBOX_CFG_URL_LEN;
	if(strlen(msg) > len)
	{
		return -1;
	}

	if (-1 == sscanf((char *)msg, "eolsetgbfip:ip=%s\r\n", url))
    {
        return -1;
    }

	if(0 == strlen(url))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(GBFURL, cfg_id);
	if(0 != tbox_cfg_write(cfg_id, url))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_gbf_port_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT32 port = 0;

	TBOX_CFG_ID_GET(GBFPORT, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, &port))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+gbfport:%d\r\n", port);
	return 0;
}

static INT8 fct_eol_set_gbf_port_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
	UINT32 port = 0;

	if(-1 == sscanf((char *)msg, "eolsetgbfport:port=%d\r\n", &port))
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(GBFPORT, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &port))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_hpm_murl_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_URL_LEN] = {0};
	
	TBOX_CFG_ID_GET(HPMMURL, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, url))
	{
		return -1;
	}

	if(0 == strlen(url))
	{
		return -1;
	}
	
	*out_len = snprintf(res, res_len, "+hpmmurl:%s\r\n", url);
    return 0;
}

static INT8 fct_eol_set_hpm_murl_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_URL_LEN] = {0};
	UINT16 len = strlen("eolsethpmmurl:url=\r\n")+TBOX_CFG_URL_LEN;
	if(strlen(msg) > len)
	{
		return -1;
	}

	if (-1 == sscanf((char *)msg, "eolsethpmmurl:url=%s\r\n", url))
    {
        return 1;
    }

	if(0 == strlen(url))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(HPMMURL, cfg_id);
	if(0 != tbox_cfg_write(cfg_id, url))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_hpm_mip_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_IP_LEN] = {0};
	
	TBOX_CFG_ID_GET(HPMMIP, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, url))
	{
		return -1;
	}

	if(0 == strlen(url))
	{
		return -1;
	}
	
	*out_len = snprintf(res, res_len, "+hpmmip:%s\r\n", url);
    return 0;
}

static INT8 fct_eol_set_hpm_mip_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_IP_LEN] = {0};
	UINT16 len = strlen("eolsethpmmip:ip=\r\n")+TBOX_CFG_IP_LEN;
	if(strlen(msg) > len)
	{
		return -1;
	}

	if (-1 == sscanf((char *)msg, "eolsethpmmip:ip=%s\r\n", url))
    {
        return 1;
    }

	if(0 == strlen(url))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(HPMMIP, cfg_id);
	if(0 != tbox_cfg_write(cfg_id, url))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_hpm_mport_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT32 port = 0;

	TBOX_CFG_ID_GET(HPMMPORT, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, &port))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+hpmmport:%d\r\n", port);
	return 0;
}

static INT8 fct_eol_set_hpm_mport_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
	UINT32 port = 0;

	if(-1 == sscanf((char *)msg, "eolsethpmmport:port=%d\r\n", &port))
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(HPMMPORT, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &port))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_hpm_surl_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_URL_LEN] = {0};
	
	TBOX_CFG_ID_GET(HPMSURL, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, url))
	{
		return -1;
	}

	if(0 == strlen(url))
	{
		return -1;
	}
	
	*out_len = snprintf(res, res_len, "+hpmsurl:%s\r\n", url);
    return 0;
}

static INT8 fct_eol_set_hpm_surl_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_URL_LEN] = {0};
	UINT16 len = strlen("eolsethpmsurl:url=\r\n")+TBOX_CFG_URL_LEN;
	if(strlen(msg) > len)
	{
		return -1;
	}

	if (-1 == sscanf((char *)msg, "eolsethpmsurl:url=%s\r\n", url))
    {
        return 1;
    }

	if(0 == strlen(url))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(HPMSURL, cfg_id);
	if(0 != tbox_cfg_write(cfg_id, url))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_hpm_sip_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_IP_LEN] = {0};
	
	TBOX_CFG_ID_GET(HPMSIP, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, url))
	{
		return -1;
	}

	if(0 == strlen(url))
	{
		return -1;
	}
	
	*out_len = snprintf(res, res_len, "+hpmsip:%s\r\n", url);
    return 0;
}

static INT8 fct_eol_set_hpm_sip_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    CHAR  url[TBOX_CFG_IP_LEN] = {0};
	UINT16 len = strlen("eolsethpmsip:ip=\r\n")+TBOX_CFG_IP_LEN;
	if(strlen(msg) > len)
	{
		return -1;
	}

	if (-1 == sscanf((char *)msg, "eolsethpmsip:ip=%s\r\n", url))
    {
        return 1;
    }

	if(0 == strlen(url))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(HPMSIP, cfg_id);
	if(0 != tbox_cfg_write(cfg_id, url))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_hpm_sport_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
    UINT32 port = 0;

	TBOX_CFG_ID_GET(HPMSPORT, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, &port))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+hpmsport:%d\r\n", port);
	return 0;
}

static INT8 fct_eol_set_hpm_sport_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
	UINT32 port = 0;

	if(-1 == sscanf((char *)msg, "eolsethpmsport:port=%d\r\n", &port))
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(HPMSPORT, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &port))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_set_telno_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    UINT8  index = 0;
    UINT8  telno[IF_4G_MAX_PHONE_NUM_LEN+1] = {0};
	UINT16 len = strlen("eolsettelno:telno=\r\n")+IF_4G_MAX_PHONE_NUM_LEN;

	if(strlen(msg) > len)
	{
		return -1;
	}

	if(sscanf(msg, "eolsettelno:telno=%s\r\n", (char *)telno))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)telno))
	{
		return -1;
	}

	for(index = 0; index < strlen((CHAR *)telno); index++)
	{
		if(!isdigit(telno[index]))
		{
			return -1;
		}
	}
	
	if(FALSE == if_4g_set_phone_num(telno))
	{
		return -1;
	}

	if_4g_query_phone_num();
	
    return 0;
}

static INT8 fct_eol_get_telno_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    UINT8 telno[IF_4G_MAX_PHONE_NUM_LEN+1] = {0};
	UINT8 len = sizeof(telno);
	UNUSED(msg);

	if(FALSE == if_4g_get_phone_num(telno, &len))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)telno))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+telno:%s\r\n", telno);
	
    return 0;
}

static INT8 fct_eol_get_iccid_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    UINT8 iccid[IF_4G_MAX_ICCID_LEN] = {0};
	UINT8 len = sizeof(iccid);
	UNUSED(msg);

	if(FALSE == if_4g_get_iccid(iccid, &len))
	{
		return -1;
	}

	if(0 == strlen((CHAR *)iccid))
	{
		return -1;
	}
	
	*out_len = snprintf(res, res_len, "+iccid:%s\r\n", iccid);

    return 0;
}

static INT8 fct_eol_get_imei_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    UINT8 imei[IF_4G_MAX_IMEI_LEN] = {0};
	UINT8 len = IF_4G_MAX_IMEI_LEN;
	CHAR *temp_ptr = res;
	UINT8 index;

	*out_len = 0;
	
	if(FALSE == if_4g_get_imei(imei, &len))
	{
		return -1;
	}

	strcpy(temp_ptr, "+imei:");
	*out_len = strlen("+imei:");
	temp_ptr += *out_len;

	for(index = 0; index < len; index++)
	{
		*temp_ptr = imei[index] + '0';
		temp_ptr += 1;
		*out_len += 1;
	}

	strcpy(temp_ptr, "\r\n");
	*out_len += strlen("\r\n");

	return 0;
}

static INT8 fct_eol_get_battype_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static INT8 fct_eol_set_battype_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static INT8 fct_eol_get_pm_mode_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static INT8 fct_eol_set_pm_mode_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    return -1;
}

static FCT_CMD_CMD eol_cmd_table[] = {
    {"eolmcubtlver",   		fct_eol_btlver_proc        		},
    {"eolmcuappver",   		fct_eol_appver_proc        		},
    {"eolgetbattype",		fct_eol_get_battype_proc		},
    {"eolsetbattype",       fct_eol_set_battype_proc        },
    {"eolgetpmmode",		fct_eol_get_pm_mode_proc		},
    {"eolsetpmmode",        fct_eol_set_pm_mode_proc        },
    {"eolgetpubapn",   		fct_eol_get_pubapn_proc        	},
    {"eolsetpubapn",   		fct_eol_set_pubapn_proc        	}, 
    {"eolgetpriapn",   		fct_eol_get_priapn_proc        	},
    {"eolsetpriapn",   		fct_eol_set_priapn_proc        	}, 
    {"eolgetotaapn",   		fct_eol_get_otaapn_proc        	},
    {"eolsetotaapn",   		fct_eol_set_otaapn_proc        	}, 
    {"eolgetcan1baud", 		fct_eol_get_can1baud_proc      	},
    {"eolgetcan2baud", 		fct_eol_get_can2baud_proc      	},
    {"eolgetcan3baud", 		fct_eol_get_can3baud_proc      	},
    {"eolsetcan1baud", 		fct_eol_set_can1baud_proc      	},
    {"eolsetcan2baud", 		fct_eol_set_can2baud_proc      	},    
    {"eolsetcan3baud", 		fct_eol_set_can3baud_proc      	},    
    {"eolsetsn",       		fct_eol_setsn_proc         		},
    {"eolgetsn",       		fct_eol_getsn_proc         		},
    {"eolsettcode",    		fct_eol_set_trace_code_proc		},
    {"eolgettcode",         fct_eol_get_trace_code_proc		},    
    {"eolsetdefault",  		fct_eol_setdefault_proc    		},
    
    {"eolgetgbfip",      	fct_eol_get_gbf_ip_proc     	},
    {"eolsetgbfip",      	fct_eol_set_gbf_ip_proc     	},
    {"eolgetgbfport",    	fct_eol_get_gbf_port_proc   	},
    {"eolsetgbfport",    	fct_eol_set_gbf_port_proc   	},
    
    {"eolgethpmmurl",		fct_eol_get_hpm_murl_proc 		},
    {"eolsethpmmurl",		fct_eol_set_hpm_murl_proc 		},
    {"eolgethpmmip",   		fct_eol_get_hpm_mip_proc    	},
    {"eolsethpmmip",   		fct_eol_set_hpm_mip_proc    	},
    {"eolgethpmmport", 		fct_eol_get_hpm_mport_proc  	},
    {"eolsethpmmport", 		fct_eol_set_hpm_mport_proc  	},
    {"eolgethpmsurl",		fct_eol_get_hpm_surl_proc 		},
    {"eolsethpmsurl",		fct_eol_set_hpm_surl_proc 		},
    {"eolgethpmsip",   		fct_eol_get_hpm_sip_proc    	},
    {"eolsethpmsip",   		fct_eol_set_hpm_sip_proc    	},
    {"eolgethpmsport", 		fct_eol_get_hpm_sport_proc  	},
    {"eolsethpmsport", 		fct_eol_set_hpm_sport_proc  	},
    
    {"eolsettelno",    		fct_eol_set_telno_proc      	},
    {"eolgettelno",    		fct_eol_get_telno_proc      	},
    {"eolgeticcid",    		fct_eol_get_iccid_proc      	},
    {"eolgetimei",     		fct_eol_get_imei_proc       	},    
};


VOID fct_cmd_init(VOID)
{
	//TODO
}

VOID fct_cmd_timeout(VOID)
{
	//TODO
	MODULE_LOG_I(FCT, "fct cmd timeout.");
}

VOID fct_fctcmd_process(const CHAR *indata, UINT16 inlen, CHAR *outdata, UINT32 outsize)
{
	INT8   ret = 0;
	UINT8  index = 0;
	UINT16 outlen = 0;

	for(index = 0; index < ARRAY_SIZE(fct_cmd_table); index++)
	{
        if (0 == strncmp((char *)indata, fct_cmd_table[index].name, strlen(fct_cmd_table[index].name)))
		{
			ret = fct_cmd_table[index].proc(indata, outdata, outsize, &outlen);
			if(0 != ret)
			{
				snprintf(outdata, outsize, "%.*s:ng\r\n", 
					(int)strlen(fct_cmd_table[index].name), fct_cmd_table[index].name);
				return;
			}
			else
			{				
				snprintf(outdata + outlen, outsize - outlen, "%.*s:ok\r\n", 
					(int)strlen(fct_cmd_table[index].name), fct_cmd_table[index].name);
				return;
			}
		}
	}

	snprintf(outdata, outsize, "%.*s:ng\r\n", (int)strlen(indata), indata);
	return;
}

VOID fct_eolcmd_process(const CHAR *indata, UINT16 inlen, CHAR *outdata, UINT32 outsize)
{
	INT8   ret = 0;
	UINT8  index = 0;
	UINT16 outlen = 0;
	
	for(index = 0; index < ARRAY_SIZE(eol_cmd_table); index++)
	{
        if (0 == strncmp((char *)indata, eol_cmd_table[index].name, strlen(eol_cmd_table[index].name)))
		{
			ret = eol_cmd_table[index].proc(indata, outdata, outsize, &outlen);
			if(0 != ret)
			{
				snprintf(outdata, outsize, "%.*s:ng\r\n", 
					(int)strlen(eol_cmd_table[index].name), eol_cmd_table[index].name);
				return;
			}
			else
			{				
				snprintf(outdata + outlen, outsize - outlen, "%.*s:ok\r\n", 
					(int)strlen(eol_cmd_table[index].name), eol_cmd_table[index].name);
				return;
			}
		}
	}
	
	snprintf(outdata, outsize, "%.*s:ng\r\n", (int)strlen(indata), indata);
	return;
}



