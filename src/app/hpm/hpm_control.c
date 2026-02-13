#include "tbox_common.h"
#include "tbox_core.h"

#include "hpm_content.h"
#include "hpm_control.h"
#include "hpm_pack.h"
#include "hpm_socket.h"
#include "hpm_net.h"
#include "hpm_cfg.h"

#define HPM_CTROL_TIME_LEN	(6)
#define HPM_CTROL_RES_LEN	(512)
#define HPM_CTROL_RES_POS	(7)

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
	UINT16 len;
}HPM_CONTROL_INFO_T;

typedef struct
{
	BOOL   need_res;
	BOOL   result;
	UINT8  data[HPM_CTROL_RES_LEN];
	UINT16 data_len;
}HPM_CONTROL_RES_T;

static VOID hpm_control_send_resp(HPM_CONTROL_RES_T ctrl_res)
{
	UINT8 *buf = NULL;
	UINT16 len = 0;

	buf = mempool_alloc(HPM_SESSION_COMMON_MEM_SIZE);
	if(NULL == buf)
	{
		MODULE_LOG_E(HPM, "hpm control buf memalloc failed");
		return;
	}	
	
	len = hpm_pack_common_resp(buf, ctrl_res.data, ctrl_res.data_len);

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
	pos += HPM_CTROL_TIME_LEN;
	ctrl_info.seq = (data[pos] << 8) | data[pos + 1];
	pos += 2;
	ctrl_info.cmd = data[pos++];
	ctrl_info.len = (data[pos] << 8) | data[pos + 1];
	pos += 2;
	
	MODULE_LOG_I(HPM, "hpm control, cmd: 0x%02x", ctrl_info.cmd);

	switch (ctrl_info.cmd)
	{
		case HPM_CTRL_DATA_REQ:
			//发送一次实时数据
			break;
		case HPM_CTRL_CFG_READ:
			ctrl_res.need_res = TRUE;
			ret = hpm_cfg_tsp_get_param(data+pos, ctrl_info.len, ctrl_res.data+HPM_CTROL_RES_POS, &ctrl_res.data_len);
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
			ret = hpm_cfg_tsp_set_param(data+pos, ctrl_info.len, ctrl_res.data+HPM_CTROL_RES_POS, &ctrl_res.data_len);
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
			break;
		case HPM_CTRL_RESET:
			break;
		case HPM_CTRL_REBOOT:
			break;
		case HPM_CTRL_UPGRADE:
			break;
		case HPM_CTRL_TRANSMIS:
			break;
		default:
			break;		
	}

	if(ctrl_res.need_res)
	{
		ctrl_res.data[0] = ctrl_info.seq >> 8;
		ctrl_res.data[1] = ctrl_info.seq;
		ctrl_res.data[2] = HPM_CMD_CONTROL;
		ctrl_res.data[3] = ctrl_info.cmd;
		ctrl_res.data[4] = ctrl_res.result;
		ctrl_res.data[5] = ctrl_res.data_len >> 8;
		ctrl_res.data[6] = ctrl_res.data_len;
		hpm_control_send_resp(ctrl_res);
	}
}


