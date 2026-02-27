#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tbox_config.h"
#include "tbox_common.h"
#include "tbox_memory.h"
#include "tbox_log_inner.h"
#include "api_rtos.h"
#include "tbox_task.h"
#include "tbox_module_inner.h"
#include "tbox_message_inner.h"

typedef struct tag_tbox_module_mgr_item
{
    BOOL is_used;
    BOOL is_enabled;
    TBOX_ID task_id;
    TBOX_MODULE_STATE state;
    TBOX_MODULE_INFO module_info;
}TBOX_MODULE_MGR_ITEM;

static volatile BOOL tbox_module_is_init = FALSE;
static TBOX_MODULE_MGR_ITEM tbox_modules[TBOX_MODULE_MAX_NUM];

static inline UBaseType_t tbox_module_enter_critical(VOID)
{
    UBaseType_t saved_interrupt_mask = 0U;
    if(pdTRUE == xPortIsInsideInterrupt())
    {
        saved_interrupt_mask = taskENTER_CRITICAL_FROM_ISR();
    }
    else
    {
        taskENTER_CRITICAL();
    }

    return saved_interrupt_mask;    
}

static inline VOID tbox_module_exit_critical(UBaseType_t saved_interrupt_mask)
{
    if(pdTRUE == xPortIsInsideInterrupt())
    {
        taskEXIT_CRITICAL_FROM_ISR(saved_interrupt_mask);
    }
    else
    {
        taskEXIT_CRITICAL();
    }
}

INT32 tbox_module_init(VOID)
{
    if(TRUE == tbox_module_is_init)
    {
        return (INT32)TBOX_E_HASINIT;
    }

    for(UINT32 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
    {
        memset(&tbox_modules[i], 0, sizeof(TBOX_MODULE_MGR_ITEM));
        tbox_modules[i].task_id = TBOX_ID_INVALID;
        tbox_modules[i].state = TBOX_MODULE_STATE_NOINIT;
    }
    
    tbox_module_is_init = TRUE;

    return (INT32)TBOX_E_OK;
}

VOID tbox_module_exit(VOID)
{
    TBOX_ID task_id;
    MODULE_EXIT_FUN exit_fun;
    UBaseType_t critical;

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module has been deinit");  
        return;
    }
    tbox_module_is_init = FALSE;

    for(UINT32 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
    {
        task_id = TBOX_ID_INVALID;
        exit_fun = NULL_PTR;
        critical = tbox_module_enter_critical();
        {
            exit_fun = tbox_modules[i].module_info.exit_fun;
            if(TRUE == tbox_modules[i].is_used &&
               FALSE == tbox_modules[i].module_info.is_interface &&
               TRUE == tbox_modules[i].module_info.is_critical && 
               TBOX_MODULE_STATE_NOINIT != tbox_modules[i].state)
            {
                task_id = tbox_modules[i].task_id;
                tbox_modules[i].state = TBOX_MODULE_STATE_NOINIT;
            }
        }
        tbox_module_exit_critical(critical);

        if(TBOX_ID_INVALID != task_id)
        {
            tbox_task_detach(task_id);
        }
        if(NULL_PTR != exit_fun)
        {
            (*exit_fun)();
        }
    }

    MODULE_LOG_I(ICORE, "tbox_module_exit");
}

VOID tbox_module_init_all_regmodule(VOID)
{
    MODULE_INIT_FUN init_fun;
    UBaseType_t critical;

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return;
    }

    for(UINT8 seq = MODULE_INIT_SEQ_OS; seq <= MODULE_INIT_SEQ_MODULE; seq++)
    {
        for(UINT32 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
        {
            critical = tbox_module_enter_critical();
            {
                if(MODULE_INIT_SEQ_MODULE == seq)
                {
                    tbox_modules[i].state = TBOX_MODULE_STATE_INIT;
                }
                if(FALSE == tbox_modules[i].is_used || 
                   FALSE == tbox_modules[i].is_enabled ||
                   NULL_PTR == tbox_modules[i].module_info.init_fun)
                {
                    tbox_module_exit_critical(critical);
                    continue;
                }
                init_fun = tbox_modules[i].module_info.init_fun;
            }
            tbox_module_exit_critical(critical);

            if((INT32)TBOX_E_OK != (*init_fun)(seq))
            {
                TBOX_ID task_id;
                critical = tbox_module_enter_critical();
                {
                    task_id = tbox_modules[i].task_id;
                    tbox_modules[i].task_id = TBOX_ID_INVALID;
                    tbox_modules[i].is_enabled = FALSE;
                    tbox_modules[i].state = TBOX_MODULE_STATE_NOINIT;
                }
                tbox_module_exit_critical(critical);

                if(TBOX_ID_INVALID != task_id)
                {
                    tbox_task_detach(task_id);
                }
                MODULE_LOG_E(ICORE, "init module:%s failed", tbox_modules[i].module_info.name);
            }
        } 
    }

    MODULE_LOG_I(ICORE, "all modules has been started");
}

TBOX_ID tbox_module_register(TBOX_MODULE_INFO *module_info)
{
    if(NULL_PTR == module_info)
    {
        MODULE_LOG_E(ICORE, "module_info is null");
        return TBOX_ID_INVALID;
    }
    if(NULL_PTR == module_info->name)
    {
        MODULE_LOG_E(ICORE, "module_info->name is null");
        return TBOX_ID_INVALID;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return TBOX_ID_INVALID;
    }

    UBaseType_t critical;
    TBOX_ID module_id = TBOX_ID_INVALID;
    critical = tbox_module_enter_critical();
    {
        for(UINT32 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
        {
            if(FALSE == tbox_modules[i].is_used)
            {
                module_id = (TBOX_ID)i;
                tbox_modules[i].is_used = TRUE;
                tbox_modules[i].is_enabled = TRUE;
                tbox_modules[i].task_id = TBOX_ID_INVALID;
                memcpy(&tbox_modules[i].module_info, module_info, sizeof(TBOX_MODULE_INFO));
                if(FALSE == module_info->is_interface &&
                   0U == module_info->stack_size)
                {
                    tbox_modules[i].module_info.stack_size = TBOX_TASK_SMALL_STACK_SIZE;
                }
                break;
            }
        }
    }
    tbox_module_exit_critical(critical);

    if(FALSE == module_info->is_interface &&
       TRUE == module_info->is_critical &&
       TBOX_ID_INVALID != module_id)
    {
        TBOX_ID task_id = tbox_task_attach(module_id, module_info->priority, module_info->stack_size);
        if(TBOX_ID_INVALID != task_id)
        {
            critical = tbox_module_enter_critical();
            {
                tbox_modules[module_id].task_id = task_id;
            }
            tbox_module_exit_critical(critical);
        }
        MODULE_LOG_I(ICORE, "attach task:%d", task_id);
    }
    MODULE_LOG_D(ICORE, "register module:%s, id:%d", module_info->name, module_id);
    return module_id;
}

VOID tbox_module_show_allname(VOID)
{
#define TBOX_MODULE_ONELINE_COUNT 5U
    UINT8 count = 0;
    CHAR *pos = NULL_PTR;
    UINT8 len = 0U;

    CHAR *show_buff = (CHAR *)tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, 128U);
    if(NULL_PTR == show_buff)
    {
        return;
    }

    pos = show_buff;
    len = 128U;
    for(UINT8 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
    {
        if(FALSE == tbox_modules[i].is_used)
        {
            continue;
        }
        snprintf(pos, len, "%-12s ", tbox_modules[i].module_info.name);
        pos += strlen(pos);
        len -= strlen(pos);
        count++;
        if(count >= TBOX_MODULE_ONELINE_COUNT)
        {
            LOG_PRINT("%s\r\n", show_buff);
            count = 0U;
            pos = show_buff;
            len = 128U;
            memset(pos, 0U, len);
        }
    }
    if(count > 0U)
    {
         LOG_PRINT("%s\r\n", show_buff);
    }

    tbox_memory_free(TBOX_MEMORY_TYPE_CORE, show_buff);
}

INT32 tbox_module_start_runloop(TBOX_ID module_id, MODULE_RUNLOOP_FUN runloop_fun)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || 
       module_id < 0 ||
       runloop_fun == NULL_PTR)
    {
        return TBOX_E_INVALID_PARAM;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return TBOX_E_NOINIT;
    }

    UBaseType_t critical;
    TBOX_ID task_id;
    UINT8 priority;
    UINT16 stack_size;
    critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used || 
           FALSE == tbox_modules[module_id].is_enabled)
        {
            tbox_module_exit_critical(critical);
            MODULE_LOG_E(ICORE, "module[%d] not register or has started", module_id);
            return TBOX_E_NOCREATE;
        }
        priority = tbox_modules[module_id].module_info.priority;
        stack_size = tbox_modules[module_id].module_info.stack_size;
    }
    tbox_module_exit_critical(critical);

    task_id = tbox_task_create_runloop(module_id, priority, stack_size, runloop_fun);
    if(TBOX_ID_INVALID == task_id)
    {
        MODULE_LOG_E(ICORE, "create runloop task failed");
        return TBOX_E_FAILED_CREATE;
    }
    critical = tbox_module_enter_critical();
    {
        tbox_modules[module_id].task_id = task_id;
    }
    tbox_module_exit_critical(critical);

    return TBOX_E_OK;
}

VOID tbox_module_get_config(TBOX_ID module_id, TBOX_MODULE_INFO *config)
{
    if(NULL_PTR == config)
    {
        return;
    }

    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return;
    }

    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(TRUE == tbox_modules[module_id].is_used)
        {
            memcpy(config, &tbox_modules[module_id].module_info, sizeof(TBOX_MODULE_INFO));
        }
    }
    tbox_module_exit_critical(critical);
}

VOID tbox_module_start(VOID)
{
    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return;
    }

    UBaseType_t critical;
    MODULE_START_FUN start_fun;
    for(UINT32 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
    {
        critical = tbox_module_enter_critical();
        {
            if(FALSE == tbox_modules[i].is_used || 
               FALSE == tbox_modules[i].is_enabled ||
               TBOX_MODULE_STATE_START == tbox_modules[i].state)
            {
                tbox_module_exit_critical(critical);
                continue;
            }
            tbox_modules[i].state = TBOX_MODULE_STATE_START;
            start_fun = tbox_modules[i].module_info.start_fun;
        }
        tbox_module_exit_critical(critical);

        if (NULL_PTR != start_fun)
        {
            (*start_fun)();
        }
    }

    MODULE_LOG_I(ICORE, "all modules has been started");
}

VOID tbox_module_start_specific(TBOX_ID module_id)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        MODULE_LOG_E(ICORE, "module id:%d is invalid", (INT32)module_id);
        return;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return;
    }

    MODULE_START_FUN start_fun;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used || 
           FALSE == tbox_modules[module_id].is_enabled ||
           TBOX_MODULE_STATE_START == tbox_modules[module_id].state)
        {
            tbox_module_exit_critical(critical);
            return;
        }
        tbox_modules[module_id].state = TBOX_MODULE_STATE_START;
        start_fun = tbox_modules[module_id].module_info.start_fun;
    }
    tbox_module_exit_critical(critical);

    if (NULL_PTR != start_fun)
    {
        (*start_fun)();
    }
}

VOID tbox_module_stop(VOID)
{
    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return;
    }
    
    UBaseType_t critical;
    MODULE_STOP_FUN stop_fun;
    for(INT32 i = TBOX_MODULE_MAX_NUM-1; i >= 0; i--)
    {
        critical = tbox_module_enter_critical();
        {
            if(FALSE == tbox_modules[i].is_used || 
               FALSE == tbox_modules[i].is_enabled ||
               TBOX_MODULE_STATE_STOP == tbox_modules[i].state)
            {
                tbox_module_exit_critical(critical);
                continue;
            }
            if(NULL_PTR == tbox_modules[i].module_info.stop_fun)
            {
                tbox_modules[i].state = TBOX_MODULE_STATE_STOP;
            }
            stop_fun = tbox_modules[i].module_info.stop_fun;
        }
        tbox_module_exit_critical(critical);

        if(NULL_PTR != stop_fun)
        {
            (*stop_fun)();
        }

        /*停止处理需要异步处理，这里只是发起停止请求，停止完成后，模块可以设置状态为TBOX_MODULE_STATE_STOP*/
        /*critical = tbox_module_enter_critical();
        {
            tbox_modules[i].state = TBOX_MODULE_STATE_STOP;
        }
        tbox_module_exit_critical(critical);*/        
    }
}

VOID tbox_module_stop_specific(TBOX_ID module_id)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        MODULE_LOG_E(ICORE, "module id:%d is invalid", (INT32)module_id);
        return;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return;
    }

    MODULE_STOP_FUN stop_fun;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used || 
           FALSE == tbox_modules[module_id].is_enabled ||
           TBOX_MODULE_STATE_STOP == tbox_modules[module_id].state)
        {
            tbox_module_exit_critical(critical);
            return;
        }
        if(NULL_PTR == tbox_modules[module_id].module_info.stop_fun)
        {
            tbox_modules[module_id].state = TBOX_MODULE_STATE_STOP;
        }
        stop_fun = tbox_modules[module_id].module_info.stop_fun;
    }
    tbox_module_exit_critical(critical);

    if(NULL_PTR != stop_fun)
    {
        (*stop_fun)();
    }
}

INT32 tbox_module_enable(TBOX_ID module_id, BOOL enable)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return (INT32)TBOX_E_NOINIT;
    }

    MODULE_ENABLE_FUN enable_fun;
    TBOX_ID task_id;
    BOOL need_attach;
    UINT8 priority;
    UINT16 stack_size;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used)
        {
            tbox_module_exit_critical(critical);
            return (INT32)TBOX_E_NOREADY;
        }
        need_attach = (TRUE == tbox_modules[module_id].module_info.is_critical 
                    && FALSE == tbox_modules[module_id].module_info.is_interface) ? TRUE : FALSE;
        tbox_modules[module_id].is_enabled = enable;
        task_id = tbox_modules[module_id].task_id;
        enable_fun = tbox_modules[module_id].module_info.enable_fun;
        priority = tbox_modules[module_id].module_info.priority;
        stack_size = tbox_modules[module_id].module_info.stack_size;
    }
    tbox_module_exit_critical(critical);

    if(TRUE == enable)
    {
        if(TRUE == need_attach && TBOX_ID_INVALID == task_id)
        {
            task_id = tbox_task_attach(module_id, priority, stack_size);
            critical = tbox_module_enter_critical();
            {
                tbox_modules[module_id].task_id = task_id;
            }
            tbox_module_exit_critical(critical);
            MODULE_LOG_I(ICORE, "attach task:%d", task_id);
        }
    }
    else
    {
        if(TBOX_ID_INVALID != task_id)
        {
            tbox_task_detach(task_id);
            critical = tbox_module_enter_critical();
            {
                tbox_modules[module_id].task_id = TBOX_ID_INVALID;
            }
            tbox_module_exit_critical(critical);
            MODULE_LOG_I(ICORE, "detach task:%d", task_id);            
        }
    }

    if(NULL_PTR != enable_fun)
    {
        return (*enable_fun)(enable);
    }
    
    MODULE_LOG_I(ICORE, "enable module:%s, id:%d", tbox_modules[module_id].module_info.name, module_id);

    return (INT32)TBOX_E_OK;
}

BOOL  tbox_module_is_enabled(TBOX_ID module_id)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return FALSE;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return FALSE;
    }

    BOOL is_enabled;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used)
        {
            is_enabled = FALSE;
        }
        else
        {
            is_enabled = tbox_modules[module_id].is_enabled;
        }
    }
    tbox_module_exit_critical(critical);

    return is_enabled;
}

CHAR *tbox_module_get_name(TBOX_ID module_id)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return "UNKNOWN";
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return "UNKNOWN";
    }

    CHAR *name;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used)
        {
            name = "UNKNOWN";
        }
        else
        {
            name = tbox_modules[module_id].module_info.name;
        }
    }
    tbox_module_exit_critical(critical);

    return name;
}

UINT8 tbox_module_get_log_level(TBOX_ID module_id)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return 0xFFU;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return 0xFFU;
    }

    UINT8 log_level;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used)
        {
            log_level = 0xFFU;
        }
        else
        {
            log_level = tbox_modules[module_id].module_info.log_level;
        }
    }
    tbox_module_exit_critical(critical);

    return log_level;
}

VOID  tbox_module_set_log_level(TBOX_ID module_id, UINT8 log_level)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return;
    }

    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(TRUE == tbox_modules[module_id].is_used)
        {
            tbox_modules[module_id].module_info.log_level = log_level;
        }
    }
    tbox_module_exit_critical(critical);
}

INT32  tbox_module_set_log_level_byname(CHAR *name, UINT8 log_level)
{
    if(NULL_PTR == name)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    
    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return (INT32)TBOX_E_NOINIT;
    }
    
    UINT32 i = 0U;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        for(; i < TBOX_MODULE_MAX_NUM; i++)
        {
            if(FALSE == tbox_modules[i].is_used)
            {
                continue;
            }
            if(0 == strncmp(name, tbox_modules[i].module_info.name, strlen(name)))
            {
                tbox_modules[i].module_info.log_level = log_level;
                break;
            }
        }
    }
    tbox_module_exit_critical(critical);
    if(i >= TBOX_MODULE_MAX_NUM)
    {
        return (INT32)TBOX_E_NOMATCH;
    }
    return (INT32)TBOX_E_OK;
}

BOOL tbox_module_is_interface(TBOX_ID module_id)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return TRUE;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return TRUE;
    }

    BOOL is_interface;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used)
        {
            is_interface = TRUE;
        }
        else
        {
            is_interface = tbox_modules[module_id].module_info.is_interface;
        }
    }
    tbox_module_exit_critical(critical);

    return is_interface;
}

BOOL tbox_module_is_critical(TBOX_ID module_id)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return FALSE;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return FALSE;
    }

    BOOL is_critical;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used)
        {
            is_critical = FALSE;
        }
        else
        {
            is_critical = tbox_modules[module_id].module_info.is_critical;
        }
    }
    tbox_module_exit_critical(critical);

    return is_critical;
}

MODULE_HANDLE tbox_module_get_handle(TBOX_ID module_id)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return NULL_PTR;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return NULL_PTR;
    }

    TBOX_ID task_id = TBOX_ID_INVALID;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(TRUE == tbox_modules[module_id].is_used)
        {
            task_id = tbox_modules[module_id].task_id;
        }
    }
    tbox_module_exit_critical(critical);
    
    if(task_id == TBOX_ID_INVALID)
    {
        return NULL_PTR;
    }

    return tbox_task_get_handle(task_id);
}

INT32 tbox_module_send_message(TBOX_ID module_id,                         
                               CHAR *msg_name,
                               TBOX_MSG_HADNLER msg_handle,  
                               UINT16 msg_len,
                               UINT8 *msg_data)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        MODULE_LOG_E(ICORE, "invalid param");
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return (INT32)TBOX_E_NOINIT;
    }

    if(FALSE == tbox_message_is_enabled(msg_name))
    {
        MODULE_LOG_W(ICORE, "message:%s is not enabled", msg_name);
        return (INT32)TBOX_E_NOPRIVILEGE;
    }

    TBOX_ID task_id;
    UINT32 stack_size;
    BOOL is_runloop;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(FALSE == tbox_modules[module_id].is_used ||
           FALSE == tbox_modules[module_id].is_enabled ||
           TRUE == tbox_modules[module_id].module_info.is_interface)
        {
            tbox_module_exit_critical(critical);
            MODULE_LOG_W(ICORE, "module:%s is interface or not enabled", tbox_modules[module_id].module_info.name);
            return (INT32)TBOX_E_NOREADY;
        }
        task_id = tbox_modules[module_id].task_id;
        stack_size = tbox_modules[module_id].module_info.stack_size;
        is_runloop = tbox_modules[module_id].module_info.is_runloop;
    }
    tbox_module_exit_critical(critical);

    MODULE_LOG_D(ICORE, "add callable:%s, module:%s, task:%d", msg_name, 
                                                              tbox_modules[module_id].module_info.name, 
                                                              task_id);

    if(TBOX_ID_INVALID == task_id || TRUE == is_runloop)
    {
        return tbox_taskpool_add_msg(stack_size, msg_name, msg_handle, msg_len, msg_data);
    }

    return tbox_task_add_msg(task_id, msg_name, msg_handle, msg_len, msg_data);
}

VOID tbox_module_set_state(TBOX_ID module_id, TBOX_MODULE_STATE state)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return;
    }

    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(TRUE == tbox_modules[module_id].is_used)
        {
            tbox_modules[module_id].state = state;
        }
    }
    tbox_module_exit_critical(critical);
}

TBOX_MODULE_STATE tbox_module_get_state(TBOX_ID module_id)
{
    if(module_id >= TBOX_MODULE_MAX_NUM || module_id < 0)
    {
        return TBOX_MODULE_STATE_INVALID;
    }

    if(FALSE == tbox_module_is_init)
    {
        MODULE_LOG_E(ICORE, "module is not init");       
        return TBOX_MODULE_STATE_INVALID;
    }

    TBOX_MODULE_STATE state;
    UBaseType_t critical = tbox_module_enter_critical();
    {
        if(TRUE == tbox_modules[module_id].is_used)
        {
            state = tbox_modules[module_id].state;
        }
        else
        {
            state = TBOX_MODULE_STATE_INVALID;
        }
    }
    tbox_module_exit_critical(critical);

    return state;
}

BOOL tbox_allmodule_canbe_stop(VOID)
{
    if(FALSE == tbox_module_is_init)
    {
        return TRUE;
    }

    UBaseType_t critical;
    MODULE_CANBE_STOP_FUN canbe_stop_fun;
    for(UINT32 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
    {
        critical = tbox_module_enter_critical();
        {
            if(FALSE == tbox_modules[i].is_used || 
               FALSE == tbox_modules[i].is_enabled ||
               TBOX_MODULE_STATE_STOP == tbox_modules[i].state)
            {
                tbox_module_exit_critical(critical);
                continue;
            }
            canbe_stop_fun = tbox_modules[i].module_info.canbe_stop_fun;
        }
        tbox_module_exit_critical(critical);

        if(NULL_PTR != canbe_stop_fun && 
           FALSE == (*canbe_stop_fun)())
        {
            MODULE_LOG_I(ICORE, "module:%s can not be stoped", tbox_modules[i].module_info.name);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL tbox_allmodule_isstoped(VOID)
{
    if(FALSE == tbox_module_is_init)
    {
        return TRUE;
    }

    UBaseType_t critical = tbox_module_enter_critical();
    {
        for(UINT32 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
        {
            if(TRUE == tbox_modules[i].is_used &&
               TRUE == tbox_modules[i].is_enabled &&
               TBOX_MODULE_STATE_START == tbox_modules[i].state)
            {
                tbox_module_exit_critical(critical);
                MODULE_LOG_W(ICORE, "module:%s is not stoped", tbox_modules[i].module_info.name);
                return FALSE;
            }
        }
    }
    tbox_module_exit_critical(critical);

    return TRUE;
}

BOOL tbox_allmodule_istarted(VOID)
{
    if(FALSE == tbox_module_is_init)
    {
        return TRUE;
    }

    UBaseType_t critical = tbox_module_enter_critical();
    {
        for(UINT32 i = 0U; i < TBOX_MODULE_MAX_NUM; i++)
        {
            if(TRUE == tbox_modules[i].is_used &&
               TRUE == tbox_modules[i].is_enabled &&
               TBOX_MODULE_STATE_START != tbox_modules[i].state)
            {
                tbox_module_exit_critical(critical);
                return FALSE;
            }
        }
    }
    tbox_module_exit_critical(critical);

    return TRUE;    
}