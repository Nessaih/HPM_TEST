#include <stdio.h>
#include <string.h>

#include "tbox_config.h"
#include "tbox_common.h"
#include "api_rtos.h"
#include "cqueue.h"
#include "tbox_memory.h"
#include "tbox_message.h"
#include "tbox_task.h"
#include "tbox_module.h"
#include "tbox_log_inner.h"
#include "tbox_supervish_inner.h"

#define TBOX_SUPERVISE_QUEUE_SIZE         (128U)
#define TBOX_SUPERVISE_HEADER_MAGICNO     (0x12ABCDEFU)
#define TBOX_SUPERVISE_ABNORMAL_UNHANDLED (0U)
#define TBOX_SUPERVISE_ABNORMAL_HANDLED   (1U)

#define TBOX_SUPERVISE_EVENT_START_BIT      (1 << 0)
#define TBOX_SUPERVISE_EVENT_STOP_BIT       (1 << 1)
#define TBOX_SUPERVISE_EVENT_REPORT_BIT     (1 << 2)
#define TBOX_SUPERVISE_EVENT_EXIT_BIT       (1 << 3)
#define TBOX_SUPERVISE_EVENT_ALL_BITS       (TBOX_SUPERVISE_EVENT_START_BIT |\
                                             TBOX_SUPERVISE_EVENT_STOP_BIT |\
                                             TBOX_SUPERVISE_EVENT_REPORT_BIT |\
                                             TBOX_SUPERVISE_EVENT_EXIT_BIT)

typedef struct tag_tbox_supervise_header
{
    UINT32 magicno;
    TBOX_MODULE_ABNORMAL_TYPE type;
}TBOX_SUPERVISE_HEADER;

typedef struct tag_tbox_supervise_mgr
{
    TBOX_MODULE_ABNORMAL_PROCESS_TYPE abnormal_process[MODULE_ABNORMAL_TYPE_MAX];
    cqueue_t queue;
    TaskHandle_t task;    
}TBOX_SUPERVISE_MGR;

static TBOX_SUPERVISE_MGR tbox_supervise_mgr;

static inline UBaseType_t tbox_supervise_enter_critical(VOID)
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

static inline VOID tbox_supervise_exit_critical(UBaseType_t saved_interrupt_mask)
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

static UINT8 tbox_supervise_handle_memory_overflow(TBOX_MODULE_ABNORMAL_TYPE type,
                                                   TBOX_CORE_MEMORY_ABNORMAL_INFO *memory_info, 
                                                   TBOX_MODULE_ABNORMAL_PROCESS_TYPE process_type)
{
    UNUSED(process_type);

    if(MODULE_ABNORMAL_MEMORY_OVERFLOW != type)
    {
        return TBOX_SUPERVISE_ABNORMAL_UNHANDLED;
    }

    if(NULL_PTR != memory_info)
    {
        MODULE_LOG_F(ICORE, "report memory abnormal type:%d, info:%p", type, memory_info->addr);

        TBOX_MSG_DATA data;
        data.data = (UINT8 *)memory_info;
        data.size = sizeof(TBOX_CORE_MEMORY_ABNORMAL_INFO);
        tbox_message_publish(TBOX_CORE_MEMORY_ABNORMAL_NOTIFY, &data);
    }
    
    return TBOX_SUPERVISE_ABNORMAL_HANDLED;
}

static UINT8 tbox_supervise_handle_memory_underflow(TBOX_MODULE_ABNORMAL_TYPE type,
                                                    TBOX_CORE_MEMORY_ABNORMAL_INFO *memory_info, 
                                                    TBOX_MODULE_ABNORMAL_PROCESS_TYPE process_type)
{
    UNUSED(process_type);

    if(MODULE_ABNORMAL_MEMORY_UNDERFLOW != type)
    {
        return TBOX_SUPERVISE_ABNORMAL_UNHANDLED;
    }

    if(NULL_PTR != memory_info)
    {
        MODULE_LOG_F(ICORE, "report memory abnormal type:%d, info:%p", type, memory_info->addr);

        TBOX_MSG_DATA data;
        data.data = (UINT8 *)memory_info;
        data.size = sizeof(TBOX_CORE_MEMORY_ABNORMAL_INFO);
        tbox_message_publish(TBOX_CORE_MEMORY_ABNORMAL_NOTIFY, &data);
    }
    
    return TBOX_SUPERVISE_ABNORMAL_HANDLED;
}

static UINT8 tbox_supervise_handle_task_blocked(TBOX_MODULE_ABNORMAL_TYPE type,
                                                TBOX_CORE_TASK_ABNORMAL_INFO *task_info, 
                                                TBOX_MODULE_ABNORMAL_PROCESS_TYPE process_type)
{
    if(MODULE_ABNORMAL_TASK_BLOCKED != type)
    {
        return TBOX_SUPERVISE_ABNORMAL_UNHANDLED;
    }
    
    if(NULL_PTR != task_info)
    {
        MODULE_LOG_F(ICORE, "report task abnormal type:%d, task:%d module:%d, name:%s", 
                     type, 
                     task_info->task_id, task_info->module_id, task_info->msg_name);

        TBOX_MSG_DATA data;
        data.data = (UINT8 *)task_info;
        data.size = sizeof(TBOX_CORE_TASK_ABNORMAL_INFO);
        tbox_message_publish(TBOX_CORE_TASK_ABNORMAL_NOTIFY, &data);
    }
    
    switch(process_type)
    {
        case MODULE_ABNORMAL_PROCESS_RESET:
            tbox_reset_task(task_info->task_id);
            break;

        case MODULE_ABNORMAL_PROCESS_FORBID:
            tbox_message_enable(task_info->msg_name, FALSE);
            tbox_reset_task(task_info->task_id);
            break;

        default:
            break;
    }

    return TBOX_SUPERVISE_ABNORMAL_HANDLED;
}

static UINT8 tbox_supervise_handle_task_deleted(TBOX_MODULE_ABNORMAL_TYPE type,
                                                TBOX_CORE_TASK_ABNORMAL_INFO *task_info, 
                                                TBOX_MODULE_ABNORMAL_PROCESS_TYPE process_type)
{
    if(MODULE_ABNORMAL_TASK_DELETED != type)
    {
        return TBOX_SUPERVISE_ABNORMAL_UNHANDLED;
    }

    if(NULL_PTR != task_info)
    {
        MODULE_LOG_F(ICORE, "report task abnormal type:%d, task:%d module:%d", 
                     type, 
                     task_info->task_id, task_info->module_id);

        TBOX_MSG_DATA data;
        data.data = (UINT8 *)task_info;
        data.size = sizeof(TBOX_CORE_TASK_ABNORMAL_INFO);
        tbox_message_publish(TBOX_CORE_TASK_ABNORMAL_NOTIFY, &data);
    }

    switch(process_type)
    {
        case MODULE_ABNORMAL_PROCESS_RESET:
            tbox_reset_task(task_info->task_id);
            break;

        case MODULE_ABNORMAL_PROCESS_FORBID:
            tbox_forbid_task(task_info->task_id);
            break;

        default:
            break;
    }

    return TBOX_SUPERVISE_ABNORMAL_HANDLED;    
}

static UINT8 tbox_supervise_handle_task_stack_small(TBOX_MODULE_ABNORMAL_TYPE type,
                                                    TBOX_CORE_TASK_ABNORMAL_INFO *task_info, 
                                                    TBOX_MODULE_ABNORMAL_PROCESS_TYPE process_type)
{
    if(MODULE_ABNORMAL_TASKSTACK_SMALL != type)
    {
        return TBOX_SUPERVISE_ABNORMAL_UNHANDLED;
    }

    if(NULL_PTR != task_info)
    {
        MODULE_LOG_F(ICORE, "report task abnormal type:%d, task:%d module:%d", 
                     type, 
                     task_info->task_id, task_info->module_id);

        TBOX_MSG_DATA data;
        data.data = (UINT8 *)task_info;
        data.size = sizeof(TBOX_CORE_TASK_ABNORMAL_INFO);
        tbox_message_publish(TBOX_CORE_TASK_ABNORMAL_NOTIFY, &data);
    }

    switch(process_type)
    {
        case MODULE_ABNORMAL_PROCESS_RESET:
            tbox_task_expend_stack(task_info->task_id);
            break;

        case MODULE_ABNORMAL_PROCESS_FORBID:
            tbox_forbid_task(task_info->task_id);
            break;

        default:
            break;
    }
    
    return TBOX_SUPERVISE_ABNORMAL_HANDLED;        
}

static UINT8 tbox_supervise_handle_stack_overflow(TBOX_MODULE_ABNORMAL_TYPE type,
                                                  TBOX_CORE_TASK_ABNORMAL_INFO *task_info, 
                                                  TBOX_MODULE_ABNORMAL_PROCESS_TYPE process_type)
{
    if(MODULE_ABNORMAL_STACK_OVERFLOW != type)
    {
        return TBOX_SUPERVISE_ABNORMAL_UNHANDLED;
    }

    if(NULL_PTR != task_info)
    {
        MODULE_LOG_I(ICORE, "report task abnormal handle:%p", 
                     type, 
                     task_info->task_handle);

        TBOX_MSG_DATA data;
        data.data = (UINT8 *)task_info;
        data.size = sizeof(TBOX_CORE_TASK_ABNORMAL_INFO);
        tbox_message_publish(TBOX_CORE_TASK_ABNORMAL_NOTIFY, &data);
    }

    TBOX_ID task_id = tbox_task_find_by_handle(task_info->task_handle);
    switch(process_type)
    {
        case MODULE_ABNORMAL_PROCESS_RESET:
            tbox_task_expend_stack(task_id);
            break;

        case MODULE_ABNORMAL_PROCESS_FORBID:
            tbox_forbid_task(task_id);
            break;

        default:
            break;
    }

    return TBOX_SUPERVISE_ABNORMAL_HANDLED;     
}

static VOID tbox_supervise_handle_abnormal(VOID)
{
    TBOX_SUPERVISE_HEADER header;
    TBOX_CORE_MEMORY_ABNORMAL_INFO memory_info;
    TBOX_CORE_TASK_ABNORMAL_INFO task_info;
    TBOX_MODULE_ABNORMAL_PROCESS_TYPE process_type;
    UINT32 size = 0U;

    UBaseType_t interrupt_mask = tbox_supervise_enter_critical();
    {
        size = sizeof(TBOX_SUPERVISE_HEADER)+sizeof(TBOX_CORE_TASK_ABNORMAL_INFO);
        if(cqueue_datalen(&tbox_supervise_mgr.queue) < size)
        {
            size = sizeof(TBOX_SUPERVISE_HEADER)+sizeof(TBOX_CORE_MEMORY_ABNORMAL_INFO);
            if(cqueue_datalen(&tbox_supervise_mgr.queue) < size)
            {
                tbox_supervise_exit_critical(interrupt_mask);
                MODULE_LOG_I(ICORE, "supervise queue is empty, ignore abnormal event");
                return;
            }
        }
        cqueue_get(&tbox_supervise_mgr.queue, (UINT8 *)&header, sizeof(TBOX_SUPERVISE_HEADER));
        if(header.magicno != TBOX_SUPERVISE_HEADER_MAGICNO || header.type >= MODULE_ABNORMAL_TYPE_MAX)
        {
            tbox_supervise_mgr.queue.in = tbox_supervise_mgr.queue.out = 0U;
            tbox_supervise_exit_critical(interrupt_mask);
            MODULE_LOG_I(ICORE, "supervise header magicno or type error, ignore abnormal event");
            return;
        }
        process_type = tbox_supervise_mgr.abnormal_process[header.type];
        if(MODULE_ABNORMAL_MEMORY_OVERFLOW == header.type ||
           MODULE_ABNORMAL_MEMORY_UNDERFLOW == header.type)
        {
            cqueue_get(&tbox_supervise_mgr.queue, (UINT8 *)&memory_info, sizeof(TBOX_CORE_MEMORY_ABNORMAL_INFO));
        }
        else
        {
            cqueue_get(&tbox_supervise_mgr.queue, (UINT8 *)&task_info, sizeof(TBOX_CORE_TASK_ABNORMAL_INFO));
        }
    }
    tbox_supervise_exit_critical(interrupt_mask);

    UINT8 ret = tbox_supervise_handle_memory_overflow(header.type, &memory_info, process_type);
    ret = TBOX_COND_CALL(ret, TBOX_SUPERVISE_ABNORMAL_UNHANDLED, tbox_supervise_handle_memory_underflow(header.type, &memory_info, process_type));
    ret = TBOX_COND_CALL(ret, TBOX_SUPERVISE_ABNORMAL_UNHANDLED, tbox_supervise_handle_task_blocked(header.type, &task_info, process_type));
    ret = TBOX_COND_CALL(ret, TBOX_SUPERVISE_ABNORMAL_UNHANDLED, tbox_supervise_handle_task_deleted(header.type, &task_info, process_type));
    ret = TBOX_COND_CALL(ret, TBOX_SUPERVISE_ABNORMAL_UNHANDLED, tbox_supervise_handle_task_stack_small(header.type, &task_info, process_type)); 
    ret = TBOX_COND_CALL(ret, TBOX_SUPERVISE_ABNORMAL_UNHANDLED, tbox_supervise_handle_stack_overflow(header.type, &task_info, process_type));                    
}

static VOID tbox_supervise_task(VOID *param)
{
    UNUSED(param);
    
    UINT32 wait_tick = portMAX_DELAY;
    UINT32 event_bits; 
    for(;;)
    {
        event_bits = 0U;
        xTaskNotifyWait(0U, TBOX_SUPERVISE_EVENT_ALL_BITS, &event_bits, wait_tick);
        if(0U != event_bits)
        {
            if((event_bits & TBOX_SUPERVISE_EVENT_EXIT_BIT) == TBOX_SUPERVISE_EVENT_EXIT_BIT)
            {
                break;
            }
            if ((event_bits & TBOX_SUPERVISE_EVENT_START_BIT) == TBOX_SUPERVISE_EVENT_START_BIT)
            {
                wait_tick = pdMS_TO_TICKS(2000U);
            }
            if ((event_bits & TBOX_SUPERVISE_EVENT_STOP_BIT) == TBOX_SUPERVISE_EVENT_STOP_BIT)
            {
                wait_tick = portMAX_DELAY;
                continue;
            }
            tbox_supervise_handle_abnormal();
        }
        else
        {
            tbox_task_check();
            tbox_memory_check(TBOX_MEMORY_TYPE_CORE, tbox_supervise_mgr.queue.buffer);       
        }
    }
    
    UBaseType_t interrupt_mask = tbox_supervise_enter_critical();
    {
        tbox_supervise_mgr.task = NULL_PTR;
    }
    tbox_supervise_exit_critical(interrupt_mask);
    vTaskDelete(NULL_PTR);
}

INT32 tbox_supervise_init(VOID)
{
    tbox_supervise_mgr.task = NULL_PTR;
    tbox_supervise_mgr.abnormal_process[MODULE_ABNORMAL_MEMORY_OVERFLOW] = MODULE_ABNORMAL_PROCESS_WARNING;
    tbox_supervise_mgr.abnormal_process[MODULE_ABNORMAL_MEMORY_UNDERFLOW] = MODULE_ABNORMAL_PROCESS_WARNING;
    tbox_supervise_mgr.abnormal_process[MODULE_ABNORMAL_TASK_BLOCKED] = MODULE_ABNORMAL_PROCESS_RESET;
    tbox_supervise_mgr.abnormal_process[MODULE_ABNORMAL_TASK_DELETED] = MODULE_ABNORMAL_PROCESS_RESET;
    tbox_supervise_mgr.abnormal_process[MODULE_ABNORMAL_TASKSTACK_SMALL] = MODULE_ABNORMAL_PROCESS_RESET;
    tbox_supervise_mgr.abnormal_process[MODULE_ABNORMAL_STACK_OVERFLOW] = MODULE_ABNORMAL_PROCESS_RESET;
    cqueue_init(&tbox_supervise_mgr.queue, NULL_PTR, 0U);

    UINT8 *buffer = tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, TBOX_SUPERVISE_QUEUE_SIZE);
    if(NULL_PTR == buffer)
    {
        tbox_supervise_deinit();
        return (INT32)TBOX_E_FAILED_ALLOC;
    }
    cqueue_init(&tbox_supervise_mgr.queue, buffer, TBOX_SUPERVISE_QUEUE_SIZE);

    if(pdPASS != xTaskCreate(tbox_supervise_task, 
                             "supervise_task", 
                             768U/sizeof(StackType_t), 
                             NULL_PTR, 
                             TBOX_TASK_PRIORITY_LOW, 
                             &tbox_supervise_mgr.task))
    {
        tbox_supervise_mgr.task = NULL_PTR;
        tbox_supervise_deinit();
        return (INT32)TBOX_E_FAILED_CREATE;
    }

    return (INT32)TBOX_E_OK;
}

VOID tbox_supervise_deinit(VOID)
{
    TaskHandle_t task_handle;
    UBaseType_t interrupt_mask = tbox_supervise_enter_critical();
    {
        task_handle = tbox_supervise_mgr.task;
        tbox_supervise_mgr.task = NULL_PTR;
        if(NULL_PTR != tbox_supervise_mgr.queue.buffer)
        {
            tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_supervise_mgr.queue.buffer);
            cqueue_init(&tbox_supervise_mgr.queue, NULL_PTR, 0);
        }
    }
    tbox_supervise_exit_critical(interrupt_mask);

    if(NULL_PTR != task_handle)
    {
        xTaskNotify(task_handle, TBOX_SUPERVISE_EVENT_EXIT_BIT, eSetBits);

        eTaskState state;
        UINT8 i;
        for(i = 0U; i < 4U; i++)
        {
            state = eTaskGetState(task_handle);
            if(eDeleted == state || eInvalid == state)
            {
                interrupt_mask = tbox_supervise_enter_critical();
                tbox_supervise_mgr.task = NULL_PTR;
                tbox_supervise_exit_critical(interrupt_mask);
                MODULE_LOG_I(ICORE, "the task has been deleted");
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(100U));
            
            interrupt_mask = tbox_supervise_enter_critical();
            task_handle = tbox_supervise_mgr.task;
            tbox_supervise_exit_critical(interrupt_mask);
            if(NULL_PTR == task_handle)
            {
                MODULE_LOG_I(ICORE, "the task has been exited");
                break;
            }
        }
        if(i >= 4U)
        {
            if(NULL_PTR != task_handle)
            {
                state = eTaskGetState(task_handle);
                if(eDeleted != state && eInvalid != state)
                {
                    vTaskDelete(task_handle);
                }
                interrupt_mask = tbox_supervise_enter_critical();
                tbox_supervise_mgr.task = NULL_PTR;
                tbox_supervise_exit_critical(interrupt_mask);                            
            }
        }        
    }

    MODULE_LOG_D(ICORE, "supervise deinit");
}

INT32 tbox_supervise_start(VOID)
{
    TaskHandle_t task_handle;
    UBaseType_t interrupt_mask = tbox_supervise_enter_critical();
    {
         task_handle = tbox_supervise_mgr.task;
    }
    tbox_supervise_exit_critical(interrupt_mask);
    if(NULL_PTR == task_handle)
    {
        MODULE_LOG_E(ICORE, "the task has been exited");
        return (INT32)TBOX_E_NOINIT;
    }
    eTaskState state = eTaskGetState(task_handle);
    if(eDeleted == state || eInvalid == state)
    {
        MODULE_LOG_E(ICORE, "the task has been deleted");
        return (INT32)TBOX_E_NOEXISTS;
    }
    else if(eSuspended == state)
    {
        vTaskResume(task_handle);
    }
    else
    {
        /*TODO*/
    }

    xTaskNotify(task_handle, TBOX_SUPERVISE_EVENT_START_BIT, eSetBits);

    MODULE_LOG_D(ICORE, "supervise start finish");

    return (INT32)TBOX_E_OK;
}

VOID tbox_supervise_stop(VOID)
{
    TaskHandle_t task_handle;
    UBaseType_t interrupt_mask = tbox_supervise_enter_critical();
    {
         task_handle = tbox_supervise_mgr.task;
    }
    tbox_supervise_exit_critical(interrupt_mask);
    if(NULL_PTR == task_handle)
    {
        MODULE_LOG_E(ICORE, "the task has been exited");
        return;
    }
    eTaskState state = eTaskGetState(task_handle);
    if(eDeleted == state || eInvalid == state)
    {
        MODULE_LOG_E(ICORE, "the task has been deleted");
        return;
    }
    else
    {
        /*TODO*/
    }

    xTaskNotify(task_handle, TBOX_SUPERVISE_EVENT_STOP_BIT, eSetBits);

    MODULE_LOG_D(ICORE, "supervise stop");
}

VOID tbox_supervice_set_abnormal_process(TBOX_MODULE_ABNORMAL_TYPE abnormal_type, 
                                         TBOX_MODULE_ABNORMAL_PROCESS_TYPE process_type)
{
    if(abnormal_type >= MODULE_ABNORMAL_TYPE_MAX)
    {
        return;
    }
    
    UBaseType_t interrupt_mask = tbox_supervise_enter_critical();
    {
        tbox_supervise_mgr.abnormal_process[abnormal_type] = process_type;
    }
    tbox_supervise_exit_critical(interrupt_mask);

    MODULE_LOG_I(ICORE, "set abnormal process type:%d, process:%d", abnormal_type, process_type);
}

VOID tbox_supervish_report_memory_abnormal(TBOX_MODULE_ABNORMAL_TYPE type, 
                                            const TBOX_CORE_MEMORY_ABNORMAL_INFO *info)
{

    if(NULL_PTR == info)
    {
        return;
    }

    TaskHandle_t task_handle;
    TBOX_SUPERVISE_HEADER header;
    UINT32 size = sizeof(TBOX_SUPERVISE_HEADER) + sizeof(TBOX_CORE_MEMORY_ABNORMAL_INFO);       
    header.magicno = TBOX_SUPERVISE_HEADER_MAGICNO;
    header.type = type;
    UBaseType_t interrupt_mask = tbox_supervise_enter_critical();
    {
        task_handle = tbox_supervise_mgr.task;
        if(NULL_PTR != task_handle)
        {
            if(size > cqueue_surplus(&tbox_supervise_mgr.queue))
            {
                tbox_supervise_exit_critical(interrupt_mask);
                return;
            }
            cqueue_put(&tbox_supervise_mgr.queue, (UINT8 *)&header, sizeof(TBOX_SUPERVISE_HEADER));
            cqueue_put(&tbox_supervise_mgr.queue, (UINT8 *)info, sizeof(TBOX_CORE_MEMORY_ABNORMAL_INFO));
        }
    }
    tbox_supervise_exit_critical(interrupt_mask);
    
    if(NULL_PTR != task_handle)
    {
        xTaskNotify(task_handle, TBOX_SUPERVISE_EVENT_REPORT_BIT, eSetBits);
    }
}

VOID tbox_supervish_report_task_abnormal(TBOX_MODULE_ABNORMAL_TYPE type, 
                                        BOOL is_priavte,
                                        const TBOX_CORE_TASK_ABNORMAL_INFO *info)
{
    UNUSED(is_priavte);

    if(NULL_PTR == info)
    {
        return;
    }

    TaskHandle_t task_handle;
    TBOX_SUPERVISE_HEADER header;
    UINT32 size = sizeof(TBOX_SUPERVISE_HEADER) + sizeof(TBOX_CORE_TASK_ABNORMAL_INFO);    
    header.magicno = TBOX_SUPERVISE_HEADER_MAGICNO;
    header.type = type;
    UBaseType_t interrupt_mask = tbox_supervise_enter_critical();
    {
        task_handle = tbox_supervise_mgr.task;
        if(NULL_PTR != task_handle)
        {
            if(size > cqueue_surplus(&tbox_supervise_mgr.queue))
            {
                tbox_supervise_exit_critical(interrupt_mask);
                return;
            }
            cqueue_put(&tbox_supervise_mgr.queue, (UINT8 *)&header, sizeof(TBOX_SUPERVISE_HEADER));
            cqueue_put(&tbox_supervise_mgr.queue, (UINT8 *)info, sizeof(TBOX_CORE_TASK_ABNORMAL_INFO));
        }
    }
    tbox_supervise_exit_critical(interrupt_mask);
    if(NULL_PTR != task_handle)
    {
        xTaskNotify(task_handle, TBOX_SUPERVISE_EVENT_REPORT_BIT, eSetBits);
    }
}