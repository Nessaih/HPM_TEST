#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_shell_if.h"

#include "hpm_shell.h"
#include "hpm_param_fetch.h"
#include "hpm_cfg.h"

static VOID hpm_shell_tips(VOID)
{
    tbox_log_print("hpmcmd -h: help\r\n");
    tbox_log_print("hpmcmd param \r\n");
    tbox_log_print("hpmcmd odometer \r\n");
    tbox_log_print("Usage: hpmcmd <command> [parameters]\r\n");
}

static BaseType_t hpm_shell_cmd(char *buf, size_t bufsz, const char *cmd)
{
    const char   *param1_ptr;
    BaseType_t    param1_len;

    param1_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param1_len);
    if (NULL == param1_ptr) 
    {
        goto HPM_SHELL_ERR;
    }

    if(0 == strncmp(param1_ptr, "-h", param1_len))
    {
        hpm_shell_tips();
        return pdFALSE;
    }
    else if(0 == strncmp(param1_ptr, "param", param1_len))
    {
        hpm_param_show_cfg();
        return pdFALSE;
    }
    else if (0 == strncmp(param1_ptr, "odometer", param1_len))
    {
        hpm_cfg_show_odomter();
        return pdFALSE;
    }
    else
    {
        hpm_shell_tips();		
    }

    return pdFALSE;

HPM_SHELL_ERR:
    hpm_shell_tips();
    return pdFALSE;
}

TBOX_SHELL_DEFINE(hpmcmd, "[hpmcmd -h] show help", -1, hpm_shell_cmd);

INT32 hpm_shell_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:           
            TBOX_SHELL_REGISTER(hpmcmd);
            break;
            
        default:
            break;
    }
    return 0;
}

