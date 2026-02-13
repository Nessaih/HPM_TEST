#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_string.h"
#include "tbox_shell_if.h"
#include "macros.h"
#include "version.h"

#if (configUSE_STATS_FORMATTING_FUNCTIONS == 1)
static BaseType_t print_task_info(char *buf, size_t bufsz, const char *cmd)
{
    static TaskStatus_t *task_list  = NULL;
    static uint32_t      task_count = 0U;
    static uint32_t      task_index = 0U;
    static uint32_t      total_time = 0U;
    static uint8_t       xPhase     = 0U;
    BaseType_t           xReturn    = pdFALSE;
    int32_t              len        = 0U;

    switch (xPhase) {

    case 0: {
        task_count = uxTaskGetNumberOfTasks();
        if (NULL == task_list) {
            task_list = pvPortMalloc(task_count * sizeof(TaskStatus_t));
        }
        if (NULL == task_list) {
            xPhase = 4;
        } else {
            task_count = uxTaskGetSystemState(task_list, task_count, &total_time);
            total_time = total_time / 100U;
            task_index = 0;
            xPhase     = 1;
        }
        break;
    }

    case 1: {
        len = 0;
#if (configGENERATE_RUN_TIME_STATS == 1)
        len += snprintf(&buf[len], bufsz - (uint32_t)len, "┌─────────────────┬───────┬──────┬───────────┬────────┬───────┐\n");
        len += snprintf(&buf[len], bufsz - (uint32_t)len, "│    Task Name    │ State │ Prio │ StackFree │ TaskID │ Usage │\n");
        len += snprintf(&buf[len], bufsz - (uint32_t)len, "├─────────────────┼───────┼──────┼───────────┼────────┼───────┤\n");
#else
        len += snprintf(&buf[len], bufsz - (uint32_t)len, "┌─────────────────┬───────┬──────┬───────────┬────────┐\n");
        len += snprintf(&buf[len], bufsz - (uint32_t)len, "│    Task Name    │ State │ Prio │ StackFree │ TaskID │\n");
        len += snprintf(&buf[len], bufsz - (uint32_t)len, "├─────────────────┼───────┼──────┼───────────┼────────┤\n");
#endif

        if (len > 0) {
            xPhase  = 2;
            xReturn = pdTRUE;
        } else {
            xPhase  = 4;
            xReturn = pdTRUE;
        }
        break;
    }

    case 2: {

        if (task_index < task_count) {
            TaskStatus_t *task = &task_list[task_index];
            char          state;
            // clang-format off
            switch (task->eCurrentState) {
            case eRunning:  state = 'R'; break;
            case eReady: 	state = 'X'; break;
            case eBlocked: 	state = 'B'; break;
            case eSuspended:state = 'S'; break;
            case eDeleted:  state = 'D'; break;
            default: 		state = '?'; break;
            }

#if (configGENERATE_RUN_TIME_STATS == 1)
            uint32_t usage = task->ulRunTimeCounter  / total_time;

            if(usage >0 ){
                len = snprintf(&buf[len], bufsz - (uint32_t)len, "│ %-10s│ %-5c │ %-4u │ %-9u │ %-7u│ %5u%%│\n",
                    task->pcTaskName,
                    state,
                    task->uxCurrentPriority,
                    task->usStackHighWaterMark,
                    task->xTaskNumber,
                    usage);
            }else{
                len = snprintf(&buf[len], bufsz - (uint32_t)len, "│ %-10s│ %-5c │ %-4u │ %-9u │ %-7u│   < 1%%│\n",
                    task->pcTaskName,
                    state,
                    task->uxCurrentPriority,
                    task->usStackHighWaterMark,
                    task->xTaskNumber);
            }
#else
            len = snprintf(&buf[len], bufsz - (uint32_t)len, "│ %-16s│ %-5c │ %-4u │ %-9u │ %-7u│\n",
                task->pcTaskName,
                state,
                task->uxCurrentPriority,
                task->usStackHighWaterMark,
                task->xTaskNumber);
#endif
            // clang-format on

            if (len > 0) {
                xPhase  = 2;
                xReturn = pdTRUE;
            } else {
                xPhase  = 4;
                xReturn = pdTRUE;
            }
            task_index++;
        } else {
            xPhase  = 3;
            xReturn = pdTRUE;
        }

        break;
    }

    case 3: {
#if (configGENERATE_RUN_TIME_STATS == 1)
        len = 0;
        len += snprintf(&buf[len], bufsz - (uint32_t)len, "└─────────────────┴───────┴──────┴───────────┴────────┴───────┘\n");
#else
        len += snprintf(&buf[len], bufsz - (uint32_t)len, "└─────────────────┴───────┴──────┴───────────┴────────┘\n");
#endif
        xReturn = pdTRUE;
        xPhase  = 4;
        break;
    }

    case 4: {
        if (task_list != NULL) {
            vPortFree(task_list);
            task_list = NULL;
        }
        task_count = 0;
        task_index = 0;
        total_time = 0;
        xPhase     = 0;
        xReturn    = pdFALSE; 
        break;
    }

    default: {
        xPhase  = 0;
        xReturn = pdFALSE;
        break;
    }
    }

    return xReturn;
}
#endif //(configUSE_STATS_FORMATTING_FUNCTIONS == 1)

static BaseType_t print_version_info(char *buf, size_t bufsz, const char *cmd)
{

    static uint8_t xPhase = 0;
    BaseType_t     xReturn;
    char          *ver = NULL;
    int32_t        len = 0;

    (void)cmd;

    switch (xPhase) {
    case 0:
        len += snprintf(&buf[len], bufsz - len, "┌──────┬────────────────────────────────────┬──────────────────────────┐\n");
        xPhase++;
        xReturn = pdTRUE;
        break;

    case 1:
        len += snprintf(&buf[len], bufsz - len, "│      │ Version                            │ Compile Date             │\n");
        xPhase++;
        xReturn = pdTRUE;
        break;

    case 2:
        len += snprintf(&buf[len], bufsz - len, "├──────┼────────────────────────────────────┼──────────────────────────┤\n");
        xPhase++;
        xReturn = pdTRUE;
        break;

    case 3:
        ver = (char *)version_get(VERSION_TYPE_BOOT);
        len += snprintf(&buf[len], bufsz - len, "│ Boot │ %s │ %s │\n", ver, ver + 35);
        xPhase++;
        xReturn = pdTRUE;
        break;

    case 4:
        ver = (char *)version_get(VERSION_TYPE_APP);
        len += snprintf(&buf[len], bufsz - len, "│ App  │ %s │ %s │\n", ver, ver + 35);
        xPhase++;
        xReturn = pdTRUE;
        break;

    case 5:
        len += snprintf(&buf[len], bufsz - len, "└──────┴────────────────────────────────────┴──────────────────────────┘\n");
        xPhase  = 0;
        xReturn = pdFALSE;
        break;

    default:
        break;
    }

    return xReturn;
}

static BaseType_t log_level_control(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    UINT8 i;
    const CHAR log_level_char[] = {'D', 'I', 'W', 'E', 'F', 'N'};
    const CHAR   *param_ptr;
    CHAR *target_name = NULL_PTR;   
    BaseType_t    param_len = 0;

    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL == param_ptr || param_len == 0) 
    {
        snprintf(buf, bufsz, "parse module name failed.\r\n");
        return pdFALSE;
    }
    target_name = mempool_alloc(param_len+1);
    if(NULL_PTR == target_name)
    {
        snprintf(buf, bufsz, "failed to alloc memory.\r\n");
        return pdFALSE;
    }
    strncpy(target_name, param_ptr, param_len);
    target_name[param_len] = '\0'; // 确保字符串终止
    tbox_string_toupper(target_name);

    param_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param_len);
    if (NULL == param_ptr) 
    {
        snprintf(buf, bufsz, "parse log level failed.\r\n");
        mempool_free(target_name);
        return pdFALSE;
    }
    for(i = 0; i < sizeof(log_level_char); ++i)
    {
        if(log_level_char[i] == toupper(*param_ptr))
        {
            break;
        }
    }
    if(i >= sizeof(log_level_char))
    {
        snprintf(buf, bufsz, "invalid log level.\r\n");
        mempool_free(target_name);
        return pdFALSE;
    }

    INT32 ret = tbox_module_set_log_level_byname(target_name, i);
    if((INT32)TBOX_E_NOMATCH == ret)
    {
        snprintf(buf, bufsz, "the module %s not found, show all modules name:\r\n", target_name);
        LOG_PRINT("%s", buf);
        memset(buf, 0U, bufsz);
        tbox_module_show_allname();
    }
    else if((INT32)TBOX_E_OK != ret)
    {
        snprintf(buf, bufsz, "set log level failed.\r\n");
    }
    else
    {
        snprintf(buf, bufsz, "set log level of %s to %c.\r\n", target_name, log_level_char[i]);
    }

    mempool_free(target_name);

    return pdFALSE;
}

#if (configUSE_STATS_FORMATTING_FUNCTIONS == 1)
TBOX_SHELL_DEFINE(taskinfo, "print task list and runtime stats", 0, print_task_info);
#endif
TBOX_SHELL_DEFINE(showver, "print version info", 0, print_version_info);
TBOX_SHELL_DEFINE(setlog, "[name] [level] set module log level", 2, log_level_control);

VOID tbox_default_shell_init(VOID)
{
    /*TODO: [LGC]命令移动对应模块*/
#if (configUSE_STATS_FORMATTING_FUNCTIONS == 1)
    TBOX_SHELL_REGISTER(taskinfo);
#endif
    TBOX_SHELL_REGISTER(showver);
    TBOX_SHELL_REGISTER(setlog);
}