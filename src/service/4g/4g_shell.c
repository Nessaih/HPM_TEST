#include "4g_depend_header.h"
#include "macros.h"
#include "4g_shell.h"
#include "4g_dev.h"
#include "4g_mgr.h"
#include "4g_power.h"

#define SHELL_4G_AT_CMD_LEN_MAX		(64)

VOID shell_4g_at_send_callback(VOID)
{
	LOG_PRINT("\r\n4g at send seccess \r\n");
}

static BaseType_t shell_4g_send_at(char *buf, size_t bufsz, const char *cmd)
{
	const char   *param_ptr;
	BaseType_t    param_len;
	UINT8          target_name[SHELL_4G_AT_CMD_LEN_MAX] = {0};

	param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
	if (NULL == param_ptr) {
        snprintf(buf, bufsz, "parse arg1 failed.\r\n");
        return pdFALSE;
    }

    if (param_len <= 0 || param_len >= sizeof(target_name)) 
	{
        snprintf(buf, bufsz,"param len err, len: %d\n", param_len);
		return pdFALSE;
    }

	snprintf(buf, bufsz, "AT len: %d, AT: %s", param_len, param_ptr);
	
	// 使用snprintf安全构建带\r\n的字符串
	snprintf((char *)target_name, sizeof(target_name), "%.*s\r\n", param_len, param_ptr);
	
	dev_4g_direct_send(target_name, strlen((char *)target_name), shell_4g_at_send_callback);

    return pdFALSE;
}

static BaseType_t shell_4g_info_dump(char *buf, size_t bufsz, const char *cmd)
{
	UNUSED(buf);
	UNUSED(bufsz);
	UNUSED(cmd);

	mgr_4g_dump_4ginfo();
	return pdFALSE;
}

static BaseType_t shell_4g_sleep(char *buf, size_t bufsz, const char *cmd)
{
	const char   *param_ptr;
	BaseType_t    param_len;
    char *sleep_str = "sleep";
    char *shut_str = "shut";

	param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
	if (NULL == param_ptr || param_len <= 0 ) 
    {
        snprintf(buf, bufsz, "parse arg1 failed.\r\n");
        return pdFALSE;
    }

    if(0 == strncmp(param_ptr, sleep_str, strlen(sleep_str)))
    {
       // power_4g_set_slp_type(POWER_4G_SLP_SLEEP);
        power_4g_sleep();
    }
    else if(0 == strncmp(param_ptr, shut_str, strlen(shut_str)))
    {
        //power_4g_set_slp_type(POWER_4G_SLP_SHUTDOWN);
        power_4g_sleep();
    }
    else
    {
        snprintf(buf, bufsz, "the arg1 is invalid.\r\n");
        return pdFALSE;
    }

    return pdFALSE;
}

static BaseType_t shell_4g_wakeup(char *buf, size_t bufsz, const char *cmd)
{
    UNUSED(buf);
    UNUSED(bufsz);
    UNUSED(cmd);
    
    power_4g_wakeup();

    return pdFALSE;
}

//AT+CGMM: 区分芯片型号
//AT+RFTEMPERATURE?: 查询Lte芯片温度

static const CLI_Command_Definition_t x4gAtCmd   = {"4gat",   "4g at send cmd",  shell_4g_send_at, 	1};
static const CLI_Command_Definition_t x4gInfoCmd = {"4ginfo", "4g info dump", 	 shell_4g_info_dump,0};
static const CLI_Command_Definition_t x4gSleepCmd  = {"4gsleep",  "test 4g sleep[sleep|shut]",  shell_4g_sleep,  1U};
static const CLI_Command_Definition_t x4gWakeupCmd  = {"4gwakeup",  "test 4g wakeup",  shell_4g_wakeup,  0U};

VOID shell_4g_init(VOID)
{
    FreeRTOS_CLIRegisterCommand(&x4gAtCmd);
    FreeRTOS_CLIRegisterCommand(&x4gInfoCmd);
    FreeRTOS_CLIRegisterCommand(&x4gSleepCmd);
    FreeRTOS_CLIRegisterCommand(&x4gWakeupCmd);
    
	#if 0
    dm_shell_reg("setphonenum", "set phone number", mgr_4g_set_phonenum);
    dm_shell_reg("testntp", "test ntp", mgr_4g_test_ntp);
    dm_shell_reg("testcclk", "test the 4g cclk", mgr_4g_test_cclk);
    dm_shell_reg("test4gsend", "test the 4g send data", mgr_4g_test_data);
	#endif
}

