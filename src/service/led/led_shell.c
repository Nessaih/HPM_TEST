#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_shell_if.h"
#include "led_if.h"

#include "led_shell.h"

static VOID led_shell_tips(VOID)
{	
	tbox_log_print("ledcmd -h: help\r\n");
	tbox_log_print("ledcmd tboxcan <switch>  -switch:[0, 1]\r\n");
	tbox_log_print("ledcmd tboxlte <switch>  -switch:[0, 1]\r\n");
	tbox_log_print("ledcmd tboxgns <switch>  -switch:[0, 1]\r\n");
	tbox_log_print("Usage: ledcmd <command> [parameters]\r\n");
}

static BaseType_t led_shell_cmd(char *buf, size_t bufsz, const char *cmd)
{
	const char   *param1_ptr, *param2_ptr;
    BaseType_t    param1_len, param2_len;

	param1_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param1_len);
	if (NULL == param1_ptr) 
	{
		goto LED_SHELL_ERR;
    }

	tbox_log_print("len1: %d, p1: %s\r\n", param1_len, param1_ptr);

	if(0 == strncmp(param1_ptr, "-h", param1_len))
	{
		led_shell_tips();
		return pdFALSE;
	}
	else if(0 == strncmp(param1_ptr, "tboxcan", param1_len))
	{
		param2_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param2_len);
		if (NULL == param2_ptr) 
		{
			goto LED_SHELL_ERR;
		}

		if(0 == strncmp(param2_ptr, "0", param1_len))
		{
			led_tbox_set_can(FALSE);
		}
		else if(0 == strncmp(param2_ptr, "1", param1_len))
		{
			led_tbox_set_can(TRUE);
		}
	}
	else if(0 == strncmp(param1_ptr, "tboxlte", param1_len))
	{
		param2_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param2_len);
		if (NULL == param2_ptr) 
		{
			goto LED_SHELL_ERR;
		}
		
		if(0 == strncmp(param2_ptr, "0", param1_len))
		{
			led_tbox_set_lte(FALSE);
		}
		else if(0 == strncmp(param2_ptr, "1", param1_len))
		{
			led_tbox_set_lte(TRUE);
		}
	}
	else if(0 == strncmp(param1_ptr, "tboxgns", param1_len))
	{
		param2_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param2_len);
		if (NULL == param2_ptr) 
		{
			goto LED_SHELL_ERR;
		}
		
		if(0 == strncmp(param2_ptr, "0", param1_len))
		{
			led_tbox_set_gns(FALSE);
		}
		else if(0 == strncmp(param2_ptr, "1", param1_len))
		{
			led_tbox_set_gns(TRUE);
		}
	}
	else
	{
		led_shell_tips();		
		return pdFALSE;
	}

	return pdFALSE;

LED_SHELL_ERR:
	led_shell_tips();
	return pdFALSE;
}

TBOX_SHELL_DEFINE(ledcmd, "[ledcmd -h] show help", -1, led_shell_cmd);

VOID led_shell_init(VOID)
{
   TBOX_SHELL_REGISTER(ledcmd);
}



