#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "drv_log.h"
#include "tbox_config.h"

static UINT8 drv_log_level = (UINT8)LOG_LEVEL_ERROR;
static volatile UINT8 drv_log_output_flag = 0U;
static CHAR drv_log_buf[TBOX_LOG_LINEBUFF_SIZE];
static const CHAR *log_level_str[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL", "NONE"};

INT32 drv_log_output(const CHAR *drv_name, TBOX_LOG_LEVEL level, const CHAR *fun, const INT32 line, const CHAR *fmt, ...)
{
    UINT8 len;
    va_list args;

    if(1U == drv_log_output_flag)
    {
        return TBOX_E_IS_BUSY;
    }
    
    if(NULL_PTR == drv_name ||
       level >= LOG_LEVEL_NONE ||
       level < drv_log_level)
    {
        return TBOX_E_INVALID_PARAM;
    }

    drv_log_output_flag = 1U;
    
    memset(drv_log_buf, 0, TBOX_LOG_LINEBUFF_SIZE);
    snprintf(drv_log_buf, TBOX_LOG_LINEBUFF_SIZE-1, "[%s %s:%d][%s]", drv_name, fun, line, log_level_str[(UINT8)level]);
    len = strlen(drv_log_buf);
    if(len < TBOX_LOG_LINEBUFF_SIZE-1)
    {
        va_start(args, fmt);
        vsnprintf(drv_log_buf+len, TBOX_LOG_LINEBUFF_SIZE-1-len, fmt, args);
        va_end(args);
    }
    len = strlen(drv_log_buf);
    if(len <= TBOX_LOG_LINEBUFF_SIZE-3)
    {
        drv_log_buf[len++] = '\r';
        drv_log_buf[len++] = '\n';
        drv_log_buf[len++] = '\0';        
    }    
    tbox_log_raw_output(drv_log_buf, len);

    drv_log_output_flag = 0U;
    
    return (INT32)TBOX_E_OK;
}

VOID drv_log_set_level(TBOX_LOG_LEVEL level)
{
    drv_log_level = (UINT8)level;
}