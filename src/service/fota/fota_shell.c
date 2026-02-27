#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_shell_if.h"
#include "fota_if.h"

#include "fota_shell.h"
#include "fota_com.h"

static VOID fota_shell_tips(VOID)
{	
	tbox_log_print("fotacmd -h: help\r\n");
	tbox_log_print("fotacmd info \r\n");
	tbox_log_print("fotacmd reset \r\n");
	tbox_log_print("fotacmd ftp [url] \r\n");
	tbox_log_print("Usage: fotacmd <command> [parameters]\r\n");
}

static BaseType_t fota_shell_cmd(char *buf, size_t bufsz, const char *cmd)
{
	const char   *param1_ptr, *param2_ptr;
    BaseType_t    param1_len, param2_len;
	INT32 fota_id = 0;
	INT32 ret = 0;
	
	param1_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param1_len);
	if (NULL == param1_ptr) 
	{
		goto FOTA_SHELL_ERR;
    }

	if(0 == strncmp(param1_ptr, "-h", param1_len))
	{
		fota_shell_tips();
		return pdFALSE;
	}
	else if(0 == strncmp(param1_ptr, "info", param1_len))
	{
		fota_com_info_dump();
	}
	else if(0 == strncmp(param1_ptr, "reset", param1_len))
	{
		fota_com_reset();
	}
	else if(0 == strncmp(param1_ptr, "ftp", param1_len))
	{
		param2_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param2_len);
		if (NULL == param2_ptr) 
		{
			goto FOTA_SHELL_ERR;
		}
		
		tbox_log_print("param2: %s\r\n", param2_ptr);

		//ftp://gramftp:TD9MJK7mDfbKhCok@114.215.177.162:12021/upgrade/VPU.HPM80STUI004.02.02.250312.40/upgrade_289396.bin\r\n

		GET_TBOX_MODULE_ID(FOTA, fota_id);
		ret = fota_do_upgrade((UINT8 *)param2_ptr, param2_len, fota_id, 0);
		if(0 != ret)
		{
			MODULE_LOG_E(FOTA, "fota do upgrade failed, ret: %d", ret);
		}
	}

	return pdFALSE;

FOTA_SHELL_ERR:
	fota_shell_tips();
	return pdFALSE;
}

TBOX_SHELL_DEFINE(fotacmd, "[fotacmd -h] show help ", -1, fota_shell_cmd);

INT32 fota_shell_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
			TBOX_SHELL_REGISTER(fotacmd);
            break;
            
        default:
            break;
    }
	
	return 0;
}

VOID fota_shell_value_changed_handle(TBOX_MSG_DATA *data)
{
	INT32 fota_id;
	FOTA_RESULT_INFO_T info;

	GET_TBOX_MODULE_ID(FOTA, fota_id);
	
	if(data->size != sizeof(FOTA_RESULT_INFO_T))
	{
		MODULE_LOG_E(FOTA, "msg size err, size: %d", data->size);
		return;
	}
	
	memcpy(&info, data->data, data->size);
	if(info.id != fota_id)
	{
		return;
	}

	tbox_log_print("id: %d\r\n", info.id);
	tbox_log_print("seq: %d\r\n", info.seq);
	tbox_log_print("rst: %d\r\n", info.rst);
	tbox_log_print("err_code: %d\r\n", info.err_code);
	return;
}


