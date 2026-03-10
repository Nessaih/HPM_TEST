#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "tbox_log.h"
#include "analog_if.h"
#include "macros.h"
#include "version.h"
#include "gnss_if.h"
#include "4g_if.h"
#include "can_if.h"
#include "tbox_pm_io.h"
#include "tbox_pm_if.h"
#include "tbox_adsp_if.h"

#include "fct.h"
#include "fct_cmd.h"

#define FCT_CMD_MIN_BYTENUM 	(5U)
#define FCT_CMD_MIN_CHARNUM 	(6U)
#define FCT_CMD_DATA_LEN_MAX	(256)

typedef INT8 (*FCT_CMD_CMD_PROC)(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len);

typedef struct
{
    CHAR         	*name;
    FCT_CMD_CMD_PROC proc;
} FCT_CMD_CMD;

static FCT_PM_ACTION_E fct_pm_action;

static INT8  fct_cmd_startup_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	/*
	 * START: IO 控制: 
	 *	PD11[DOL, 35U]->↑
	 *  PC05[DOH, 84U]->↑
	 * IOGET:
	 *  PE12[ACC, 19U]->↓
	 *  PB04[DIL, 28U]->↑
	 * IORESET:
	 *  PD11[DOL, 35U]->↓
	 *  PC05[DOH, 84U]->↓
	 */

	fct_pm_action = FCT_PM_ACTION_IDLE;

	fct_timer_start();
	
	drv_pin_set_level(PIN_ENABLE_DOH, 1U);
	drv_pin_set_level(PIN_ENABLE_DOL, 1U);
	
    return 0;
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
	INT32 ret = 0;
	DEV_TIME time;
	ret = time_if_rtc_get(&time);
	if(0 != ret)
	{
		return -1;
	}

    return 0;
}

static INT8  fct_cmd_setcan_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	INT32 i = 0;
	INT32 port = 0;
	if(-1 == sscanf((char *)msg, "fctsetcan:port=%d\r\n", &port))
	{
		return -1;
	}

	if(port < 1 || port > 3)
	{
		return -1;
	}

	if(2 == port)
	{
		fct_pm_action = FCT_PM_ACTION_CAN2;
	}
	
	for(i = 0; i < port; i++)
	{
		can_if_setbaud(i, 250, 1);
	}
	
    return 0;
}

static INT8  fct_cmd_can_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	fct_pm_action = FCT_PM_ACTION_IDLE;
	fct_timer_stop();
	if(can_if_get_recv_count(0) + can_if_get_recv_count(1) > 0)
	{
		return 0;
	}
    return -1;
}

static INT8  fct_cmd_can0_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	if(can_if_get_recv_count(0) > 0)
	{
		return 0;
	}
    return -1;
}

static INT8  fct_cmd_can1_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	if(can_if_get_recv_count(1) > 0)
	{
		return 0;
	}
    return -1;
}

static INT8  fct_cmd_can2_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	if(can_if_get_recv_count(2) > 0)
	{
		return 0;
	}
    return -1;
}

static INT8 fct_ring_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	//检测4g_sim信息
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
	if(tbox_pm_io_acc_is_active())
	{
		return -1;
	}
	
    fct_pm_action = FCT_PM_ACTION_SLEEP;
	fct_timer_start();
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
	INT32 pe12 = 0;
	INT32 pb04 = 0;
	/*IOGET
	 *  PE12[ACC, 19U]->↓
	 *  PB04[DIL, 28U]->↑
	 */
	pe12 = drv_pin_get_level(PIN_WAKE_ACC);
	pb04 = drv_pin_get_level(PIN_MCU_DIL);

	if(0 == pe12 && 1 == pb04)
	{
		//IORESET
		drv_pin_set_level(PIN_ENABLE_DOH, 0U);
		drv_pin_set_level(PIN_ENABLE_DOL, 0U);
		return 0;
	}

	//IORESET
	drv_pin_set_level(PIN_ENABLE_DOH, 0U);
	drv_pin_set_level(PIN_ENABLE_DOL, 0U);
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

static INT8  fct_cmd_nand_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
#define EXFLASH_NAND_ADDR_FCT_TEST 0x00000000
	
	UINT8 ret = 0;
	UINT8 test_data[10] = {0x12, 0x34, 0x67, 0xAC, 0x71, 0xC7, 0x89, 0xDA, 0x65, 0x46};
	UINT8 rcv_data[10]	= {0};

	ret = drv_flash_nand_erase(EXFLASH_NAND_ADDR_FCT_TEST, 1);
	if (ret != 0)
	{
		tbox_log_print("fct erase nand flash data failed, ret: %d\r\n", ret);
		return -1;
	}

	ret = drv_flash_nand_write(EXFLASH_NAND_ADDR_FCT_TEST, test_data, sizeof(test_data));
	if (ret != 0)
	{
		tbox_log_print("fct write nand flash data failed, ret: %d\r\n", ret);
		return -1;
	}

	ret = drv_flash_nand_read(EXFLASH_NAND_ADDR_FCT_TEST, rcv_data, sizeof(test_data));
	if (ret != 0)
	{
		tbox_log_print("fct read nand flash data failed, ret: %d\r\n", ret);
		return -1;
	}

	if(0 != memcmp(test_data, rcv_data, sizeof(rcv_data)))
	{
		tbox_log_print("fct cmp nand flash data failed\r\n");
		return -1;
	}

	return 0;
}

static INT8  fct_cmd_nor_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
#define EXFLASH_NOR_ADDR_FCT_TEST 0x00000000
	
	UINT8 ret = 0;
	UINT8 test_data[10] = {0x12, 0x34, 0x67, 0xAC, 0x71, 0xC7, 0x89, 0xDA, 0x65, 0x46};
	UINT8 rcv_data[10]	= {0};

	ret = drv_flash_nor_erase(EXFLASH_NOR_ADDR_FCT_TEST, 1);
	if(0 != ret)
	{
		tbox_log_print("fct erase nor flash data failed, ret: %d\r\n", ret);
		return -1;
	}

	ret = drv_flash_nor_write(EXFLASH_NOR_ADDR_FCT_TEST, test_data, sizeof(test_data));
	if(0 != ret)
	{
		tbox_log_print("fct write nor flash data failed, ret: %d\r\n", ret);
		return -1;
	}

	ret = drv_flash_nor_read(EXFLASH_NOR_ADDR_FCT_TEST, rcv_data, sizeof(rcv_data));
	if(0 != ret)
	{
		tbox_log_print("fct read nor flash data failed, ret: %d\r\n", ret);
		return -1;
	}

	if(0 != memcmp(test_data, rcv_data, sizeof(rcv_data)))
	{
		tbox_log_print("fct cmp nor flash data failed\r\n");
		return -1;
	}

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

static FCT_CMD_CMD fct_cmd_table[] = {
    {"fctstartup",     fct_cmd_startup_proc       },
    {"fct4gsignal",    fct_cmd_4gsignal_proc      },
    {"fcticcid",       fct_cmd_iccid_proc         },
    {"fctimei",        fct_cmd_imei_proc          },
    {"fctsettelno",    fct_cmd_set_telno_proc     },
    {"fctgettelno",    fct_cmd_get_telno_proc     },
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
	{"fctring",	  	   fct_ring_proc 	   		  },
    {"fctrmainpm",     fct_cmd_mainpm_proc        },
    {"fctbatpm",	   fct_cmd_batpm_proc         },
    {"fctbattmp",      fct_cmd_battmp_proc        },
    {"fctsetsn",       fct_cmd_setsn_proc         },
    {"fctgetsn",       fct_cmd_getsn_proc         },
    {"fctsettcode",    fct_cmd_set_trace_code_proc},
    {"fctgettcode",    fct_cmd_get_trace_code_proc},
    {"fctio",          fct_cmd_io_proc            },
    {"fctnor",         fct_cmd_nor_proc           }, 
    {"fctnand",        fct_cmd_nand_proc          },
    
    {"fctsetdefault",  fct_cmd_setdefault_proc    },
    {"fctsleep",       fct_cmd_sleep_proc         },
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

static INT8 fct_eol_mcuver_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
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

static INT8 fct_eol_mpuver_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	INT32 ret = 0;
    CHAR  ver[32] = {0};
	
	UNUSED(msg);
	
	ret = if_4g_get_chipid((UINT8 *)ver, sizeof(ver));
	if(0 != ret)
	{
		return -1;
	}

	if(0 == strlen(ver))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+mpuappver:%.*s\r\n", (int)strlen(ver), ver);
    return 0;
}

static INT8 fct_eol_fwver_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	INT32 ret = 0;
    CHAR  ver[32] = {0};
	
	UNUSED(msg);
	
	ret = if_4g_get_fwversion((UINT8 *)ver, sizeof(ver));
	if(0 != ret)
	{
		return -1;
	}

	if(0 == strlen(ver))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+mpufwver:%.*s\r\n", (int)strlen(ver), ver);
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

	if((0 != baud) && (250 != baud) && (500 != baud) && (1000 != baud))
	{
		return -1;
	}

	TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &baud))
	{
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

	if((0 != baud) && (250 != baud) && (500 != baud) && (1000 != baud))
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &baud))
	{
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

	if((0 != baud) && (250 != baud) && (500 != baud) && (1000 != baud))
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &baud))
	{
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

static INT8 fct_eol_set_time_zone_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
	UINT8 zone = 0;

	if(-1 == sscanf((char *)msg, "eolsettimezone:zone=%hhu\r\n", &zone))
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(TIMEZONE, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &zone))
	{
		return -1;
	}
	
    return 0;
}

static INT8 fct_eol_get_time_zone_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
    TBOX_CFG_ID cfg_id;
	UINT8 zone = 0;
	
	TBOX_CFG_ID_GET(TIMEZONE, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, &zone))
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+timezone:%d\r\n", zone);
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
	TBOX_CFG_ID cfg_id;
    UINT8 type = 0;

	TBOX_CFG_ID_GET(BATTYPE, cfg_id);
	if(0 != tbox_cfg_read(cfg_id, &type))
	{
		return -1;
	}

	if(type > 1)
	{
		return -1;
	}

	*out_len = snprintf(res, res_len, "+battype:%d\r\n", type);
	return 0;	
}

static INT8 fct_eol_set_battype_proc(const CHAR *msg, CHAR *res, UINT16 res_len, UINT16 *out_len)
{
	TBOX_CFG_ID cfg_id;
	UINT8 type = 0;

	if(-1 == sscanf((char *)msg, "eolsetbattype:type=%hhu\r\n", &type))
	{
		return -1;
	}

	if(type > 1)
	{
		return -1;
	}
	
	TBOX_CFG_ID_GET(BATTYPE, cfg_id);	
	if(0 != tbox_cfg_write(cfg_id, &type))
	{
		return -1;
	}
	
    return 0;
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
    {"eolmcuappver",   		fct_eol_mcuver_proc        		},
    {"eolmpuappver",   		fct_eol_mpuver_proc        		},
    {"eolfwver",   			fct_eol_fwver_proc        		},
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
    {"eolsettimezone",      fct_eol_set_time_zone_proc		},    
    {"eolgettimezone",      fct_eol_get_time_zone_proc		},    
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


INT32 fct_cmd_init(UINT8 seq)
{
	switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
			fct_pm_action = FCT_PM_ACTION_IDLE;
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            break;
            
        default:
            break;
    }
	return 0;
}

static VOID fct_cmd_sleep(VOID)
{	
	fct_pm_action = FCT_PM_ACTION_IDLE;
	tbox_pm_fctsleep();
}

static VOID fct_cmd_send_test_can(VOID)
{
	INT32 ret = 0;
	can_msg_t  msg;
	static UINT32 i = 0;

	memset(&msg, 0x00, sizeof(can_msg_t));

	msg.ins = 0;
    msg.id  = 0x33;
	msg.data[0] = i&0xFF;
	msg.data[1] = (i>>8)&0xFF;
	msg.data[2] = (i>>16)&0xFF;
	msg.data[3] = (i>>24)&0xFF;
	msg.data[4] = 0;
	msg.data[5] = 0;
	msg.data[6] = 0;
	msg.data[7] = 0;

	i++;
	ret = can_if_send(&msg);
	if(0 != ret)
	{
		tbox_log_print("fct send can data failed, ret: %d\r\n", ret);
	}
}

VOID fct_cmd_timeout(VOID)
{
	MODULE_LOG_I(FCT, "fct cmd timeout.");
	switch (fct_pm_action)
	{
		case FCT_PM_ACTION_IDLE:
			break;
		case FCT_PM_ACTION_LISTEN:
			break;
		case FCT_PM_ACTION_SLEEP:
			fct_cmd_sleep();
			break;
		case FCT_PM_ACTION_CAN2:
			fct_cmd_send_test_can();
			break;
		default:
			break;
	}
}

static VOID fct_fctcmd_process(const CHAR *indata, UINT16 inlen, CHAR *outdata, UINT32 outsize)
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

static VOID fct_eolcmd_process(const CHAR *indata, UINT16 inlen, CHAR *outdata, UINT32 outsize)
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

TBOX_ADSP_MATCH_RESULT fct_cmd_is_match(UINT8 *data, UINT32 len)
{
#define FCT_MATCH_MIN_LEN 11U

	if(data[len-1] != '\n' && len <= FCT_MATCH_MIN_LEN)
	{
		return TBOX_ADSP_NOT_MATCH;
	}
	if(0 == strncmp((char *)data, "dbg.bin fct", FCT_MATCH_MIN_LEN))
	{
		return TBOX_ADSP_MATCH_OK;
	}
	if(0 == strncmp((char *)data, "dbg.bin eol", FCT_MATCH_MIN_LEN))
	{
		return TBOX_ADSP_MATCH_OK;
	}

	return TBOX_ADSP_NOT_MATCH;
}

TBOX_ADSP_PROCESS_RESULT fct_cmd_callback(UINT8 *data, UINT32 len)
{	
	INT32 tmp_len = 0;
	CHAR  tmp_buf[FCT_CMD_DATA_LEN_MAX]; 

	memset(tmp_buf, 0, FCT_CMD_DATA_LEN_MAX);
	if (data[len - 2] != '\r' || data[len - 1] != '\n')
    {
        return TBOX_ADSP_NOT_PROCESS;
    }
	data[len] = '\0';
	tbox_log_print("~ #%s", data);
	tmp_len = strlen("dbg.bin ");
	if(0 == strncmp((char *)data, "dbg.bin fct", strlen("dbg.bin fct")))
	{
		data = data+tmp_len;
		len  = len-tmp_len;
		fct_fctcmd_process((char *)data, len, tmp_buf, FCT_CMD_DATA_LEN_MAX);
	}
	else if(0 == strncmp((char *)data, "dbg.bin eol", strlen("dbg.bin eol")))
	{
		data = data+tmp_len;
		len  = len-tmp_len;
		
		fct_eolcmd_process((char *)data, len, tmp_buf, FCT_CMD_DATA_LEN_MAX);
	}

	tbox_log_print("%s", tmp_buf);

	return TBOX_ADSP_PROCESS_OK;
}

BOOL fct_cmd_is_exit(UINT8 *data, UINT32 len)
{
	UNUSED(data);
	UNUSED(len);
	return TRUE;
}


