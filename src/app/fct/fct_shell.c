#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_shell_if.h"
#include "fct_if.h"

#include "fct_shell.h"
#include "fct_cmd.h"

static VOID fct_shell_tips(VOID)
{	
	tbox_log_print("FCT Test Commands:\r\n");
	tbox_log_print("  fct <command>    - FCT test commands\r\n");
	tbox_log_print("  eol <command>    - EOL test commands\r\n");
	tbox_log_print("  -h               - Show this help\r\n");
	tbox_log_print("Usage: fctcmd <command> [parameters]\r\n");
}

static BaseType_t fct_shell_proc(char *buf, size_t bufsz, const char *cmd)
{
	const char   *param1_ptr;
    BaseType_t    param1_len;

	param1_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param1_len);
	if (NULL == param1_ptr) 
	{
		goto FCT_SHELL_ERR;
    }

	MODULE_LOG_I(FCT, "len1: %d, p1: %s, bufsz: %d\r\n", param1_len, param1_ptr, bufsz);

	if(param1_len < 3)
	{
		goto FCT_SHELL_ERR;
	}

	if(0 == strncmp(param1_ptr, "fct", 3))
	{
		fct_fctcmd_process(param1_ptr, param1_len, buf, bufsz);
	}
	else if(0 == strncmp(param1_ptr, "eol", 3))
	{
		fct_eolcmd_process(param1_ptr, param1_len, buf, bufsz);
	}
	else
	{
		fct_shell_tips();		
		return pdFALSE;
	}

	return pdFALSE;

FCT_SHELL_ERR:
	fct_shell_tips();
	return pdFALSE;
}

static const CLI_Command_Definition_t xFctCmd	= {"dbg.bin",   "fct eol cmd", fct_shell_proc, 1};

VOID fct_shell_init(VOID)
{
   FreeRTOS_CLIRegisterCommand(&xFctCmd);
}

