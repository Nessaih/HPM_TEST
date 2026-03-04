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

static INT32 hpm_control_upgrade(UINT8 *in_data, UINT16 in_len, UINT8 *out_data, UINT16 *out_len, HPM_CONTROL_INFO_T info)
{
	INT32  ret = 0;
	UINT16 pos = 0;
	static HPM_CTRL_FOTA_INFO_T fota_info;
	
	if(hpm_ota_tbox_upgrading() || hpm_ota_ecu_upgrading())	//tbox在升级||ecu在升级
	{
		return -1;
	}

	memset(&fota_info, 0, sizeof(fota_info));

	fota_info.seq  = info.seq;
	fota_info.cmd  = info.cmd;
	fota_info.cmd_sub  = info.cmd_sub;
	fota_info.type = in_data[pos++];
	
	// 解析FTP URL（找到第一个0x00结束符）
	UINT16 url_len = 0;
	while (pos < in_len && in_data[pos] != 0x00) {
		if (url_len < sizeof(fota_info.fota_url) - 1) {
			fota_info.fota_url[url_len++] = in_data[pos];
		}
		pos++;
	}
	fota_info.fota_url[url_len] = '\0';
	if (pos < in_len) pos++; // 跳过结束符（如果还有数据）
	
	// 解析版本号（找到下一个0x00结束符）
	UINT16 ver_len = 0;
	while (pos < in_len && in_data[pos] != 0x00) {
		if (ver_len < sizeof(fota_info.fota_ver) - 1) {
			fota_info.fota_ver[ver_len++] = in_data[pos];
		}
		pos++;
	}
	fota_info.fota_ver[ver_len] = '\0';
	if (pos < in_len) pos++; // 跳过结束符（如果还有数据）

	memcpy(fota_info.fota_md5, in_data+pos, HPM_FOTA_MD5_MAX_LEN);
	
	switch (fota_info.type)
	{
		case HPM_CTRL_FOTA_TBOX:
			ret = hpm_ota_tbox_handle(fota_info);
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
		hpm_socket_force_stop();
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
			ret = hpm_param_fetch_ftp_start(data + pos, ctrl_info.len);
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
			ctrl_res.need_res = FALSE;
			ret = hpm_control_upgrade(data+pos, ctrl_info.len, ctrl_res.data+HPM_CTROL_RES_POS, &ctrl_res.body_len, ctrl_info);
			if(0 != ret)
			{	
				MODULE_LOG_E(HPM, "hpm handle upgrade failed, ret: %d", ret);
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

VOID hpm_control_process(VOID)
{
	hpm_ota_tbox_process();
}

