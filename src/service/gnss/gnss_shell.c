#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_shell_if.h"
#include "gnss_if.h"

#include "gnss_shell.h"
#include "gnss_parse.h"

static VOID gnss_shell_tips(VOID)
{	
	tbox_log_print("gnsscmd -h: help\r\n");
	tbox_log_print("gnsscmd info \r\n");
	tbox_log_print("gnsscmd log <switch>  -switch:[0, 1]\r\n");
	tbox_log_print("gnsscmd mod <mode>    -(Reserve)\r\n");
	tbox_log_print("Usage: gnsscmd <command> [parameters]\r\n");
}

static BaseType_t gnss_shell_cmd(char *buf, size_t bufsz, const char *cmd)
{
	const char   *param1_ptr, *param2_ptr;
    BaseType_t    param1_len, param2_len;

	param1_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param1_len);
	if (NULL == param1_ptr) 
	{
		goto GNSS_SHELL_ERR;
    }

	if(0 == strncmp(param1_ptr, "-h", param1_len))
	{
		gnss_shell_tips();
		return pdFALSE;
	}
	else if(0 == strncmp(param1_ptr, "info", param1_len))
	{
		gnss_parse_show_info();
		return pdFALSE;
	}
	else if(0 == strncmp(param1_ptr, "log", param1_len))
	{
		param2_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param2_len);
		if (NULL == param2_ptr) 
		{
			goto GNSS_SHELL_ERR;
		}
		
		if(0 == strncmp(param2_ptr, "0", param2_len))
		{
			gnss_parse_show_nmea(false);
		}
		else if(0 == strncmp(param2_ptr, "1", param2_len))
		{
			gnss_parse_show_nmea(true);
		}
	}
	else if(0 == strncmp(param1_ptr, "mod", param1_len))
	{
		//TODO: 发送数据, 设置GNSS模式
		return pdFALSE;
	}
	else
	{
		gnss_shell_tips();		
		return pdFALSE;
	}

	return pdFALSE;

GNSS_SHELL_ERR:
	gnss_shell_tips();
	return pdFALSE;
}

TBOX_SHELL_DEFINE(gnsscmd, "[gnsscmd -h] show help ", -1, gnss_shell_cmd);

INT32 gnss_shell_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
			TBOX_SHELL_REGISTER(gnsscmd);
            break;
            
        default:
            break;
    }
	
	return 0;
}



