#include "tbox_common.h"
#include "tbox_core.h"
#include "fota_if.h"
#include "tbox_log.h"
#include "tbox_cfg_if.h"

#include "hpm.h"
#include "hpm_pack.h"
#include "hpm_net.h"
#include "hpm_socket.h"
#include "hpm_content.h"
#include "hpm_control.h"
#include "hpm_ota_tbox.h"
#include "hpm_session.h"


static VOID hpm_ota_tbox_write_info(VOID);

static HPM_CTRL_FOTA_INFO_T hpm_ota_tbox_info;
static BOOL hpm_ota_need_send_resp;

static INT32 hpm_ota_tbox_parse_file_size(CHAR *url, INT32 *file_sz)
{
	CHAR *filename = NULL;
	CHAR *last_slash = NULL;
	CHAR *dot_pos = NULL;
	CHAR *num_start = NULL;
	CHAR *num_end = NULL;
	CHAR num_buf[32] = {0};
	INT32 num_len = 0;
	
	// 参数检查
	if (NULL == url || NULL == file_sz) {
		MODULE_LOG_E(HPM, "Invalid parameters: url=%p, file_sz=%p", url, file_sz);
		return -1;
	}
	
	last_slash = strrchr(url, '/');
	if (NULL == last_slash) 
	{
		filename = url;
	} else {
		filename = last_slash + 1;
	}
	
	dot_pos = strrchr(filename, '.');
	if (NULL == dot_pos) 
	{
		dot_pos = filename + strlen(filename);
	}
	
	num_end = dot_pos;
	num_start = dot_pos;
	
	while (num_start > filename && isdigit(*(num_start - 1))) {
		num_start--;
	}
	
	if (num_start >= num_end) 
	{
		MODULE_LOG_E(HPM, "No number found in filename: %s", filename);
		return -2;
	}
	
	num_len = num_end - num_start;
	if (num_len >= (INT32)sizeof(num_buf)) 
	{
		MODULE_LOG_E(HPM, "Number too long: %d", num_len);
		return -3;
	}
	
	memcpy(num_buf, num_start, num_len);
	num_buf[num_len] = '\0';
	
	*file_sz = atoi(num_buf);
	if (*file_sz <= 0) 
	{
		MODULE_LOG_E(HPM, "Invalid file size: %d", *file_sz);
		return -4;
	}
	
	MODULE_LOG_I(HPM, "Parsed file size from URL: %s -> %d", filename, *file_sz);
	return 0;
}

INT32 hpm_ota_tbox_handle(HPM_CTRL_FOTA_INFO_T fota_info)
{
	INT32 ret = 0;
	INT32 url_len = 0;
	INT32 moudle_id = 0;
	INT32 file_sz = 0;
	
	hpm_ota_tbox_info.magic = HPM_OTA_TBOX_MAGIC_NO;
	hpm_ota_tbox_info.seq = fota_info.seq;
	hpm_ota_tbox_info.cmd = fota_info.cmd;
	hpm_ota_tbox_info.cmd_sub = fota_info.cmd_sub;
	
	// 安全复制版本号
	strncpy((char *)hpm_ota_tbox_info.fota_ver, (char *)fota_info.fota_ver, sizeof(hpm_ota_tbox_info.fota_ver)-1);
	hpm_ota_tbox_info.fota_ver[sizeof(hpm_ota_tbox_info.fota_ver)-1] = '\0';
	
	// 安全复制URL
	strncpy((char *)hpm_ota_tbox_info.fota_url, (char *)fota_info.fota_url, sizeof(hpm_ota_tbox_info.fota_url)-1);
	hpm_ota_tbox_info.fota_url[sizeof(hpm_ota_tbox_info.fota_url)-1] = '\0';
	
	memcpy(hpm_ota_tbox_info.fota_md5, fota_info.fota_md5, HPM_FOTA_MD5_MAX_LEN);
	hpm_ota_tbox_info.fota_state = HPM_CTRL_FOTA_BUSY;

	hpm_ota_tbox_info_dump();

	moudle_id = hpm_get_moudle_id();
	
	url_len = strlen((char *)fota_info.fota_url);
	ret = hpm_ota_tbox_parse_file_size((CHAR *)fota_info.fota_url, &file_sz);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm parse url file size failed, ret: %d", ret);
	}
	
	ret = fota_do_upgrade_with_info(fota_info.fota_url, url_len,
		moudle_id, fota_info.seq, (CHAR *)fota_info.fota_ver, file_sz);
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "hpm do upgrade failed, ret: %d", ret);
	}

	return ret;
}

BOOL hpm_ota_tbox_upgrading(VOID)
{
	return hpm_ota_tbox_info.fota_state?TRUE:FALSE;
}

static VOID hpm_ota_tbox_send_resp(VOID)
{
	UINT8 *buf = NULL;
	UINT8 *res = NULL;
	UINT16 len = 0;
	UINT16 res_len  = 0;
	UINT16 body_len = 2;

	buf = mempool_alloc(HPM_SESSION_COMMON_MEM_SIZE);
	if(NULL == buf)
	{
		MODULE_LOG_E(HPM, "hpm control buf memalloc failed");
		return;
	}

	res = mempool_alloc(16);
	if(NULL == res)
	{
		MODULE_LOG_E(HPM, "hpm control res memalloc failed");
		mempool_free(buf);
		return;
	}
	
	HPM_MUTEX_LOCK();
	res[res_len++] = hpm_ota_tbox_info.seq >> 8;
	res[res_len++] = hpm_ota_tbox_info.seq;
	res[res_len++] = hpm_ota_tbox_info.cmd;
	res[res_len++] = hpm_ota_tbox_info.cmd_sub;
	res[res_len++] = hpm_ota_tbox_info.result;
	res[res_len++] = body_len >> 8;
	res[res_len++] = body_len;
	res[res_len++] = hpm_ota_tbox_info.type;
	res[res_len++] = hpm_ota_tbox_info.err_code;
	HPM_MUTEX_UNLOCK();
	
	
	len = hpm_pack_common_resp(buf, res, res_len);
    MODULE_LOG_DUMP(HPM, "hpm ota tbox resp:", buf, len);

	if(0 != hpm_net_send(buf, len))
	{		
		hpm_socket_force_stop();
		MODULE_LOG_E(HPM, "hpm send ota tbox resp failed");
	}
	else
	{
		hpm_ota_need_send_resp = FALSE;
		if((HPM_CTRL_RES_NG == hpm_ota_tbox_info.result) || (HPM_FOTA_STA_OTA_OK == hpm_ota_tbox_info.err_code))
		{
			memset(&hpm_ota_tbox_info, 0, sizeof(hpm_ota_tbox_info));
		}

		hpm_ota_tbox_write_info();
	}

	mempool_free(res);
	mempool_free(buf);
}

VOID hpm_ota_tbox_process(VOID)
{
	BOOL need_resp = FALSE;
	
	HPM_MUTEX_LOCK();
	need_resp = hpm_ota_need_send_resp;
	HPM_MUTEX_UNLOCK();
	
	if(FALSE == need_resp)
	{
		return;
	}

	if(HPM_SESSION_STEP_SESSION != hpm_session_get_step())
	{
		return;
	}

	hpm_ota_tbox_send_resp();
}

VOID hpm_ota_tbox_handle_msg(TBOX_MSG_DATA *data)
{
	FOTA_RESULT_INFO_T info;
	
	if(data->size != sizeof(FOTA_RESULT_INFO_T))
	{
		MODULE_LOG_E(HPM, "msg size err, size: %d", data->size);
		return;
	}
	
	memcpy(&info, data->data, data->size);
	if(info.id != hpm_get_moudle_id())
	{
		return;
	}
	
	HPM_MUTEX_LOCK();
	if(FOTA_RST_NG == info.rst)
	{
		hpm_ota_tbox_info.result = HPM_CTRL_RES_NG;
	}
	else
	{
		hpm_ota_tbox_info.result = HPM_CTRL_RES_OK;
	}
	hpm_ota_tbox_info.err_code = info.err_code;
	hpm_ota_need_send_resp = TRUE;	
	HPM_MUTEX_UNLOCK();
	
	MODULE_LOG_I(HPM, "id: %d\r\n", info.id);
	MODULE_LOG_I(HPM, "seq: %d\r\n", info.seq);
	MODULE_LOG_I(HPM, "rst: %d\r\n", info.rst);
	MODULE_LOG_I(HPM, "err_code: %d\r\n", info.err_code);
	return;
}

static VOID hpm_ota_tbox_write_info(VOID)
{
	INT32 ret = 0;
	
	hpm_ota_tbox_info.magic = HPM_OTA_TBOX_MAGIC_NO;
	ret = tbox_cfg_setkv(HPM_OTA_TBOX_INFO_NAME, &hpm_ota_tbox_info, sizeof(hpm_ota_tbox_info));
	if(0 != ret)
	{
		MODULE_LOG_E(HPM, "fota write context info failed, ret: %d", ret);
	}
}

static VOID hpm_ota_tbox_read_info(VOID)
{
	INT32 ret = 0;
	ret = tbox_cfg_getkv(HPM_OTA_TBOX_INFO_NAME, &hpm_ota_tbox_info, sizeof(hpm_ota_tbox_info));
	if(0 != ret)
	{
		hpm_ota_tbox_write_info();
	}
	else
	{
		if(HPM_OTA_TBOX_MAGIC_NO != hpm_ota_tbox_info.magic)
		{
			hpm_ota_tbox_write_info();
		}
	}
}

INT32 hpm_ota_tbox_init(UINT8 seq)
{
	switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
			hpm_ota_need_send_resp = FALSE;
			memset(&hpm_ota_tbox_info, 0, sizeof(hpm_ota_tbox_info));
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
			hpm_ota_tbox_read_info();
            break;
            
        default:
            break;
    }
	return 0;
}

VOID hpm_ota_tbox_wake(VOID)
{
	hpm_ota_need_send_resp = FALSE;
	memset(&hpm_ota_tbox_info, 0, sizeof(hpm_ota_tbox_info));
	hpm_ota_tbox_write_info();
}

VOID hpm_ota_tbox_sleep(VOID)
{	
}

VOID hpm_ota_tbox_info_dump(VOID)
{
	INT32 i = 0;
	tbox_log_print("=== hpm ota tbox info dump ===\r\n");
	tbox_log_print("magic: 0x%08x\r\n", hpm_ota_tbox_info.magic);
	tbox_log_print("seq: 0x%02x\r\n", hpm_ota_tbox_info.seq);
	tbox_log_print("cmd: 0x%02x\r\n", hpm_ota_tbox_info.cmd);
	tbox_log_print("cmd_sub: 0x%02x\r\n", hpm_ota_tbox_info.cmd_sub);
	tbox_log_print("result: %d\r\n", hpm_ota_tbox_info.result);
	tbox_log_print("type: %d\r\n", hpm_ota_tbox_info.type);
	tbox_log_print("fota_state: %d\r\n", hpm_ota_tbox_info.fota_state);
	tbox_log_print("err_code: %d\r\n", hpm_ota_tbox_info.err_code);
	tbox_log_print("fota_url: %s\r\n", hpm_ota_tbox_info.fota_url);
	tbox_log_print("fota_ver: %s\r\n", hpm_ota_tbox_info.fota_ver);
	tbox_log_print("fota_md5: \r\n", hpm_ota_tbox_info.fota_md5);
	for(i = 0; i < HPM_FOTA_MD5_MAX_LEN; i++)
	{
		tbox_log_print("%02x ", hpm_ota_tbox_info.fota_md5[i]);
	}
	tbox_log_print("\r\n");
	
}


