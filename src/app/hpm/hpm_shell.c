#include "tbox_common.h"
#include "tbox_core.h"
#include "tbox_shell_if.h"

#include "hpm_shell.h"
#include "hpm_param_fetch.h"
#include "hpm_cfg.h"
#include "hpm_control.h"
#include "hpm_flash.h"
#include "hpm_ota_tbox.h"

static VOID hpm_shell_tips(VOID)
{
    tbox_log_print("hpmcmd -h: help\r\n");
    tbox_log_print("hpmcmd odometer \r\n");
    tbox_log_print("hpmcmd otainfo \r\n");
    tbox_log_print("hpmcmd showcan \r\n");
    tbox_log_print("hpmcmd regcan [line]\r\n");
    tbox_log_print("hpmcmd unregcan canid\r\n");
    tbox_log_print("hpmcmd dumpflash\r\n");
    tbox_log_print("hpmcmd clearflash\r\n");
    tbox_log_print("Usage: hpmcmd <command> [parameters]\r\n");
}

static VOID hpm_shell_regcan_tips(VOID)
{
    tbox_log_print("eg: hpmcmd regcan 1,1,18FEEE00\r\n");
    tbox_log_print("eg: hpmcmd regcan 2,1,18FEFF01,8,0\r\n");
    tbox_log_print("eg: hpmcmd regcan 3,1,00,FECA\r\n");
    tbox_log_print("eg: hpmcmd regcan 4,1,21,30,FEE5\r\n");
    tbox_log_print("eg: hpmcmd regcan 5,1,07E0,07E8,F190\r\n");
}

static BaseType_t hpm_shell_cmd(char *buf, size_t bufsz, const char *cmd)
{
    const char *param_ptr;
    BaseType_t param_len;

    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL == param_ptr)
    {
        goto HPM_SHELL_ERR;
    }

    if (0 == strncmp(param_ptr, "-h", param_len))
    {
        hpm_shell_tips();
        return pdFALSE;
    }
    else if (0 == strncmp(param_ptr, "odometer", param_len))
    {
        hpm_cfg_show_odomter();
        return pdFALSE;
    }
    else if (0 == strncmp(param_ptr, "otainfo", param_len))
    {
        hpm_ota_tbox_info_dump();
    }
    else if (0 == strncmp(param_ptr, "showcan", param_len))
    {
        hpm_param_show_cfg();
        return pdFALSE;
    }
    else if (0 == strncmp(param_ptr, "regcan", param_len))
    {
        param_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param_len);
        if (NULL == param_ptr)
        {
            hpm_shell_regcan_tips();
            return pdFALSE;
        }
        hpm_param_register(param_ptr, (INT32)param_len);
        return pdFALSE;
    }
    else if (0 == strncmp(param_ptr, "unregcan", param_len))
    {
        param_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param_len);
        if (NULL == param_ptr)
        {
            tbox_log_print("eg: hpmcmd unregcan 18FECA00\r\n");
            return pdFALSE;
        }
        hpm_param_unregister(param_ptr, (INT32)param_len);
        return pdFALSE;
    }
    else if (0 == strncmp(param_ptr, "dumpflash", param_len))
    {
        hpm_flash_print_info();
    }
    else if (0 == strncmp(param_ptr, "clearflash", param_len))
    {
        hpm_flash_clear_info();
        tbox_log_print("clear flash info success.\r\n");
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
