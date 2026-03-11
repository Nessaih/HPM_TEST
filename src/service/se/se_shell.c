#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_cfg_if.h"
#include "tbox_shell_if.h"
#include "tbox_log.h"
#include "se_if.h"

static VOID se_shell_tips(VOID)
{
	tbox_log_print("SE Test Commands:\r\n");
	tbox_log_print("  getuid                  - Get SE UID\r\n");
	tbox_log_print("  setuid <data>           - Set SE UID\r\n");
	tbox_log_print("  getpubkey               - Get SE public key\r\n");
	tbox_log_print("  getchipid               - Get real chip ID\r\n");
	tbox_log_print("  sign <data>             - Generate signature for data\r\n");
	tbox_log_print("Usage: setest <command> [parameters]\r\n");
	return;
}

static BaseType_t se_shell_cmd(char *buf, size_t bufsz, const char *cmd)
{
	const char   *param1_ptr, *param2_ptr;
    BaseType_t    param1_len, param2_len;
	UINT8		  data_buf[128] = {0};
	UINT16		  data_len = 0;
	INT32		  ret = 0;
	
	
    // 获取第一个参数（参数名）
    param1_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param1_len);
	if (NULL == param1_ptr) 
	{
		goto SE_SHELL_ERR;
    }
	
	tbox_log_print("len1: %d, p1: %s\r\n", param1_len, param1_ptr);

	if(0 == strncmp(param1_ptr, "-h", param1_len))
	{
		se_shell_tips();
		return pdFALSE;
	}
	else if(0 == strncmp(param1_ptr, "getuid", param1_len))
	{
		ret = se_get_uid(data_buf, TBOX_CFG_SEID_LEN);
		if(0 != ret)
		{
			tbox_log_print("get uid failde, ret: %d \r\n", ret);
			return pdFALSE;
		}

		tbox_log_print("uid: %s \r\n", (CHAR *)data_buf);
	}
	else if(0 == strncmp(param1_ptr, "setuid", param1_len))
	{
		param2_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param2_len);
		if (NULL == param2_ptr) 
		{
			goto SE_SHELL_ERR;
		}

		tbox_log_print("len2: %d, p2: %s\r\n", param2_len, param2_ptr);

		strncpy((CHAR *)data_buf, param2_ptr, param2_len);
		ret = se_set_uid(data_buf, param2_len);
		if(0 != ret)
		{
			tbox_log_print("set uid failde, ret: %d \r\n", ret);
			return pdFALSE;
		}
	}
	else if(0 == strncmp(param1_ptr, "getver", param1_len))
	{
		ret = se_get_version(data_buf, &data_len);
		if (ret == 0) 
		{
			MODULE_LOG_DUMP(SE, "SE Version", data_buf, data_len);
		} 
		else 
		{
			tbox_log_print("Get SE version failed, ret=%d, len=%d", ret, data_len);
		}
		
	}
	else if(0 == strncmp(param1_ptr, "getpubkey", param1_len))
	{
		data_len = SE_PUBKEY_MAX_LEN;
		ret = se_get_pubkey(data_buf, &data_len);
		
		if (ret == 0 && data_len > 0) 
		{
			MODULE_LOG_DUMP(SE, "SE Public Key", data_buf, data_len);
		} 
		else 
		{
			tbox_log_print("Get SE Public Key failed, ret=%d, len=%d", ret, data_len);
		}
	}
	else if(0 == strncmp(param1_ptr, "getchipid", param1_len))
	{
		data_len = SE_CHIPID_MAX_LEN;
		ret = se_get_real_chip_id(data_buf, &data_len);
		if (ret == 0 && data_len > 0) 
		{
			MODULE_LOG_DUMP(SE, "Chip id", data_buf, data_len);
		} 
		else 
		{
			tbox_log_print("Get Chip Id failed, ret=%d, len=%d", ret, data_len);
		}
	}
	else if(0 == strncmp(param1_ptr, "sign", param1_len))
	{
		UINT8  sign[SE_SINGN_MAX_LEN] = {0};
		UINT16 sig_len = 0;
		
		param2_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param2_len);
		if (NULL == param2_ptr) 
		{
			goto SE_SHELL_ERR;
		}

		tbox_log_print("len2: %d, p2: %s\r\n", param2_len, param2_ptr);
		strncpy((CHAR *)data_buf, param2_ptr, param2_len);
		sig_len = sizeof(sign);
		ret = se_get_signature(data_buf, param2_len, sign, &sig_len);
		if (ret == 0) 
		{
			MODULE_LOG_DUMP(SE, "signature data", sign, sig_len);
		} 
		else 
		{
			tbox_log_print("Get signature failed, ret=%d", ret);
		}
	}
	else
	{
		goto SE_SHELL_ERR;
	}

	
	return pdFALSE;
SE_SHELL_ERR:
	
	se_shell_tips();
	return pdFALSE;
}
 
TBOX_SHELL_DEFINE(secmd, "[secmd -h] show help", -1, se_shell_cmd);
 
 INT32 se_shell_init(UINT8 seq)
 {
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:            
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:			
			TBOX_SHELL_REGISTER(secmd);
            break;
            
        default:
            break;
    }

	return 0;
}
 


