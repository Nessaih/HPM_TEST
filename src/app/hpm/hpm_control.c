#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_pm_if.h"

#include "hpm_content.h"
#include "hpm_control.h"
#include "hpm_pack.h"
#include "hpm_socket.h"
#include "hpm_net.h"
#include "hpm_cfg.h"
#include "hpm_ota_tbox.h"
#include "hpm_ota_ecu.h"
#include "hpm_param_fetch.h"

#define HPM_CTROL_TIME_LEN	(6)
#define HPM_CTROL_RES_LEN	(512)
#define HPM_CTROL_RES_POS	(5)

typedef enum
{
	HPM_CTRL_RES_NG	= 0x00,
	HPM_CTRL_RES_OK	= 0x01,
}HPM_CONTROL_RESULT_E;

typedef struct
{
	UINT8  time[6];
	UINT16 seq;
	UINT8  cmd;
	UINT8  cmd_sub;
	UINT16 len;
}HPM_CONTROL_INFO_T;

typedef struct
{
	BOOL   need_res;
	BOOL   result;
	UINT8  data[HPM_CTROL_RES_LEN];
	UINT16 data_len;
	UINT16 body_len;
}HPM_CONTROL_RES_T;

static HPM_CTRL_FOTA_INFO_T hpm_ctrl_fota_info;

VOID hpm_control_get_fota_info(HPM_CTRL_FOTA_INFO_T *fota_info)
{
	memcpy(fota_info, &hpm_ctrl_fota_info, sizeof(HPM_CTRL_FOTA_INFO_T));
}

VOID hpm_control_reset_fota_info(VOID)
{
	memcpy(&hpm_ctrl_fota_info, 0, sizeof(HPM_CTRL_FOTA_INFO_T));
}

static INT32 hpm_control_upgrade(UINT8 *in_data, UINT16 in_len,UINT8 *out_data, UINT16 *out_len, HPM_CONTROL_INFO_T info)
{
	INT32  ret = 0;
	UINT16 pos = 0;
	//UINT8  url[128] = {0};
	if(hpm_ota_tbox_status() || hpm_ota_ecu_status())	//tbox在升级||ecu在升级
	{
		return -1;
	}

	hpm_ctrl_fota_info.seq  = info.seq;
	hpm_ctrl_fota_info.cmd  = info.cmd;
	hpm_ctrl_fota_info.cmd_sub  = info.cmd_sub;
	hpm_ctrl_fota_info.type = in_data[pos++];
	snprintf((char *)(hpm_ctrl_fota_info.fota_url), sizeof(hpm_ctrl_fota_info.fota_url), "%.*s", 
		(int)(sizeof(hpm_ctrl_fota_info.fota_url)-1), (char *)(in_data+pos));
	pos += strlen((char *)(hpm_ctrl_fota_info.fota_url));
	pos += 1;
	snprintf((char *)(hpm_ctrl_fota_info.fota_ver), sizeof(hpm_ctrl_fota_info.fota_ver), "%.*s", 
		(int)(sizeof(hpm_ctrl_fota_info.fota_ver)-1), (char *)(in_data+pos));
	pos += strlen((char *)(hpm_ctrl_fota_info.fota_ver));
	pos += 1;

	memcpy(hpm_ctrl_fota_info.fota_md5, in_data+pos, HPM_FOTA_MD5_MAX_LEN);
	
	
	switch (hpm_ctrl_fota_info.type)
	{
		case HPM_CTRL_FOTA_TBOX:
			hpm_ota_tbox_handle();
			break;
		case HPM_CTRL_FOTA_ECU:
			//TODO: OTA ECU
			ret = -1;
			break;
		default:
			ret = -1;
			break;
	}
	
	return ret;
}


static VOID hpm_control_send_resp(UINT8 *res, UINT16 res_len)
{
	UINT8 *buf = NULL;
	UINT16 len = 0;

	buf = mempool_alloc(HPM_SESSION_COMMON_MEM_SIZE);
	if(NULL == buf)
	{
		MODULE_LOG_E(HPM, "hpm control buf memalloc failed");
		return;
	}	
	
	len = hpm_pack_common_resp(buf, res, res_len);
    MODULE_LOG_DUMP(HPM, "hpm control resp:", buf, len);

	if(0 != hpm_net_send(buf, len))
	{		
		hpm_socket_reset();
		MODULE_LOG_E(HPM, "hpm send control response failed");
	}

	mempool_free(buf);
}

VOID hpm_control_cmd_handle(UINT16 cmd, UINT8 *data, UINT16 len)
{
	UINT16 pos = 0;
	INT32  ret = 0;
	HPM_CONTROL_INFO_T ctrl_info;
	HPM_CONTROL_RES_T  ctrl_res;
	
	if(HPM_CMD_CONTROL != cmd)
	{
		MODULE_LOG_I(HPM, "cmd err, cmd: 0x%02x", cmd);
		return;
	}

	memset(&ctrl_info, 0, sizeof(HPM_CONTROL_INFO_T));
	memset(&ctrl_res, 0, sizeof(HPM_CONTROL_RES_T));
	memcpy(ctrl_info.time, data, HPM_CTROL_TIME_LEN);
	ctrl_info.cmd = cmd;
	
	pos += HPM_CTROL_TIME_LEN;
	ctrl_info.seq = (data[pos] << 8) | data[pos + 1];
	pos += 2;
	ctrl_info.cmd_sub = data[pos++];
	ctrl_info.len = (data[pos] << 8) | data[pos + 1];
	pos += 2;
	
	MODULE_LOG_I(HPM, "hpm control, cmd: 0x%02x", ctrl_info.cmd_sub);

	switch (ctrl_info.cmd_sub)
	{
		case HPM_CTRL_DATA_REQ:
			//发送一次实时数据, 无通用应答
			break;
		case HPM_CTRL_CFG_READ:
			ctrl_res.need_res = TRUE;
			ret = hpm_cfg_tsp_get_param(data+pos, ctrl_info.len, ctrl_res.data+HPM_CTROL_RES_POS, &ctrl_res.body_len);
			if(0 == ret)
			{
				ctrl_res.result = HPM_CTRL_RES_OK;
			}
			else
			{
				ctrl_res.result = HPM_CTRL_RES_NG;
			}
			break;
		case HPM_CTRL_CFG_SET:
			ctrl_res.need_res = TRUE;
			ret = hpm_cfg_tsp_set_param(data+pos, ctrl_info.len, ctrl_res.data+HPM_CTROL_RES_POS, &ctrl_res.body_len);
			if(0 == ret)
			{
				ctrl_res.result = HPM_CTRL_RES_OK;
			}
			else
			{
				ctrl_res.result = HPM_CTRL_RES_NG;
			}
			break;
		case HPM_CTRL_LOCK_ENB:			
			break;
		case HPM_CTRL_LOCK_RELAY:
			break;
		case HPM_CTRL_LOCK_CTRL:
			break;
		case HPM_CTRL_CAN_FILE:
			ctrl_res.need_res = TRUE;
			ret = hpm_param_download_req(data+pos, ctrl_info.len, ctrl_res.data+HPM_CTROL_RES_POS, &ctrl_res.body_len);
			if(0 == ret)
			{
				ctrl_res.result = HPM_CTRL_RES_OK;
			}
			else
			{
				ctrl_res.result = HPM_CTRL_RES_NG;
			}
			break;
		case HPM_CTRL_RESET:
			break;
		case HPM_CTRL_REBOOT:
			ctrl_res.need_res = TRUE;
			ret = tbox_pm_reboot(TBOX_PM_REBOOT_4G_MCU);
			if(0 == ret)
			{
				ctrl_res.result = HPM_CTRL_RES_OK;
			}
			else
			{
				ctrl_res.result = HPM_CTRL_RES_NG;
			}
			break;
		case HPM_CTRL_UPGRADE:
			ctrl_res.need_res = TRUE;
			ret = hpm_control_upgrade(data+pos, ctrl_info.len, ctrl_res.data+HPM_CTROL_RES_POS, &ctrl_res.body_len, ctrl_info);
			if(0 == ret)
			{	
				ctrl_res.result = HPM_CTRL_RES_OK;
			}
			else
			{
				ctrl_res.result = HPM_CTRL_RES_NG;
			}
			break;
		case HPM_CTRL_TRANSMIS:
			break;
		default:
			break;		
	}

	if(ctrl_res.need_res)
	{
		pos = 0;
		ctrl_res.data[pos++] = ctrl_info.seq >> 8;
		ctrl_res.data[pos++] = ctrl_info.seq;
		ctrl_res.data[pos++] = ctrl_info.cmd;
		ctrl_res.data[pos++] = ctrl_info.cmd_sub;
		ctrl_res.data[pos++] = ctrl_res.result;
		ctrl_res.data_len += pos;
		ctrl_res.data_len += ctrl_res.body_len;
		hpm_control_send_resp(ctrl_res.data, ctrl_res.data_len);
	}
}


