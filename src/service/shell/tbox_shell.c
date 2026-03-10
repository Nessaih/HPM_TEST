#include "tbox_common.h"       
#include "tbox_config.h"       
#include "tbox_core.h"         
#include "tbox_shell_if.h"     
#include "tbox_default_shell.h"
#include "macros.h"            
#include "version.h"           
#include "tbox_adsp_if.h"  

static INT32        tbox_shell_init(UINT8 seq);
static inline INT32 tbox_shell_rstip(CHAR *str, UINT16 len);
static TBOX_ADSP_MATCH_RESULT tbox_shell_is_match(UINT8 *data, UINT32 len);
static TBOX_ADSP_PROCESS_RESULT tbox_shell_callback(UINT8 *data, UINT32 len);
static BOOL         tbox_shell_is_exit(UINT8 *data, UINT32 len);

TBOX_MODULE_FUN(TBOXSHELL, tbox_shell_init, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR);
TBOX_MODULE(TBOXSHELL, TBOX_TASK_PRIORITY_LOW, LOG_LEVEL_ERROR, TBOX_TASK_SMALL_STACK_SIZE, TRUE, FALSE);
TBOX_MODULE_LOADER(TBOXSHELL)
{
    /*TODO 加载其他信息*/
}

static INT32 tbox_shell_init(UINT8 seq)
{
    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        break;

    case MODULE_INIT_SEQ_STORAGE:
        break;

    case MODULE_INIT_SEQ_MODULE:
    {
        tbox_default_shell_init();
        tbox_adsp_register(TBOX_ADSP_TYPE_SHELL, tbox_shell_is_match, tbox_shell_callback, tbox_shell_is_exit);
    }
    break;

    default:
        break;
    }

    MODULE_LOG_D(TBOXSHELL, "tbox_shell_init seq:%d", seq);
    return (INT32)TBOX_E_OK;
}

static inline INT32 tbox_shell_rstip(CHAR *str, UINT16 len)
{
    UINT16 org_len = len;

    if (NULL_PTR == str)
    {
        return -1;
    }

    if (len >= 1U)
    {
        --len;
        if (str[len] == '\r' || str[len] == '\n')
        {
            str[len] = '\0';
        }
    }
    if (len >= 1U)
    {
        --len;
        if (str[len] == '\r' || str[len] == '\n')
        {
            str[len] = '\0';
        }
    }

    return (INT32)org_len;
}

static TBOX_ADSP_MATCH_RESULT tbox_shell_is_match(UINT8 *data, UINT32 len)
{
#define TBOX_SHELL_MIN_BYTENUM (2U)
#define TBOX_SHELL_MIN_CHARNUM (3U)
    if (len < TBOX_SHELL_MIN_BYTENUM)
    {
        return TBOX_ADSP_MATCH_NEED_MORE_DATA;
    }
    else if (len <= TBOX_SHELL_MIN_CHARNUM)
    {
        len = len - 1U;
        if (data[len] != '\r' && data[len] != '\n' && data[len] != '\0')
        {
            return TBOX_ADSP_NOT_MATCH;
        }
        len = len - 1U;
        if (data[len] != '\r' && data[len] != '\n')
        {
            return TBOX_ADSP_NOT_MATCH;
        }
    }
    else
    {
        /*都是可见字符*/
        for (UINT8 i = 0U; i < len && data[i] != '\0' && data[i] != '\r' && data[i] != '\n'; i++)
        {
            if (!isprint(data[i]))
            {
                return TBOX_ADSP_NOT_MATCH;
            }
        }
    }

    return TBOX_ADSP_MATCH_OK;
}

static TBOX_ADSP_PROCESS_RESULT tbox_shell_callback(UINT8 *data, UINT32 len)
{
    BaseType_t continued;
    INT32      ret_len;
    CHAR      *output = FreeRTOS_CLIGetOutputBuffer();

    ret_len = tbox_shell_rstip((CHAR *)data, (UINT16)len);
    if (ret_len < 0)
    {
        return TBOX_ADSP_NOT_PROCESS;
    }

    do
    {
        output[0] = '\0';
        continued = FreeRTOS_CLIProcessCommand((CHAR *)data, output, configCOMMAND_INT_MAX_OUTPUT_SIZE);
        tbox_log_raw_output(output, strlen(output));
    } while (continued);

    return TBOX_ADSP_PROCESS_OK;
}

static BOOL tbox_shell_is_exit(UINT8 *data, UINT32 len)
{
    UNUSED(data);
    UNUSED(len);
    return TRUE;
}