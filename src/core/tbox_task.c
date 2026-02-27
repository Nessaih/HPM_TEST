#include <string.h>
#include <stdio.h>

#include "tbox_config.h"
#include "tbox_common.h"
#include "api_rtos.h"
#include "tbox_memory.h"
#include "tbox_dlist.h"
#include "cqueue.h"
#include "tbox_message.h"
#include "tbox_supervish_inner.h"
#include "tbox_log_inner.h"
#include "tbox_task.h"

#define TBOX_MSG_MAGICNUM              (0x544F)
#define TBOX_TASK_MAX_NUM              (TBOX_TASK_LARGER_SIZE_NUM+TBOX_TASK_MEDIUM_SIZE_NUM+TBOX_TASK_SMALL_SIZE_NUM)
#define TBOX_TASK_ABNORMA_BLOCK_NUM    (3U)
#define TBOX_TASK_BLOCK_COUNT          (5U)
#define TBOX_TASK_EVENT_START_BIT      (1 << 0)
#define TBOX_TASK_EVENT_QUIT_BIT       (1 << 1)
#define TBOX_TASK_EVENT_STOP_BIT       (1 << 2)
#define TBOX_TASK_EVENT_MSG_BIT        (1 << 3)
#define TBOX_TASK_EVENT_ALL_BITS       (TBOX_TASK_EVENT_START_BIT | TBOX_TASK_EVENT_QUIT_BIT | TBOX_TASK_EVENT_STOP_BIT |TBOX_TASK_EVENT_MSG_BIT)
#define TBOX_TASK_START_RUNLOOP_BIT    (1 << 0)

typedef struct tag_tbox_block_info
{
    CHAR *msg_name;
    UINT8 con_block_num;
    TBOX_ID task_id;
    BOOL is_block;
}TBOX_BLOCK_INFO;

typedef struct tag_tbox_msg_header
{
    UINT16 msgic_no;
    UINT16 len;
    CHAR * name;
    TBOX_MSG_HADNLER handler;
}TBOX_MSG_HEADER;

typedef struct tag_tbox_task_info
{
    TBOX_TASK_STATE state;
    UINT32 stack_size;
    TBOX_ID module_id;
    TBOX_ID task_id;
    UINT8 priority;
    CHAR *cur_block_msg_name;
    TickType_t cur_block_tick;
    cqueue_t msg_queue;
    TaskHandle_t handle;
    SemaphoreHandle_t mutex;
}TBOX_TASK_INFO;

typedef struct tag_tbox_runloop_info
{
    BOOL is_used;
    UINT8 priority;    
    UINT32 stack_size;
    TBOX_ID module_id;
    TBOX_ID task_id;
    TBOX_TASK_STATE state;
    MODULE_RUNLOOP_FUN runloop_func;
    TaskHandle_t handle;        
}TBOX_RUNLOOP_INFO;

typedef struct tag_tbox_task_mgr
{
    TBOX_BLOCK_INFO block_info[TBOX_TASK_BLOCK_COUNT];
    TBOX_RUNLOOP_INFO runloops[TBOX_RUNLOOP_TASK_NUM];
    SemaphoreHandle_t mutex;
}TBOX_TASK_MGR;

/*the order of task resource[0->max]: small, medium, large*/
static TBOX_TASK_INFO tbox_task_infos[TBOX_TASK_MAX_NUM];
static TBOX_TASK_MGR tbox_task_mgr;

static inline TBOX_ID tbox_taskpool_select_task(UINT32 stack_size)
{
    TBOX_ID task_id = TBOX_ID_INVALID;
    
    /*1、选择空闲任务 */
    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {
        xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
        {
            if(TBOX_TASK_RUN_IDLE == tbox_task_infos[i].state &&
               tbox_task_infos[i].stack_size >= stack_size &&
               tbox_task_infos[i].module_id == TBOX_ID_INVALID)  
            {
                task_id = (TBOX_ID)i;
            }
        }
        xSemaphoreGive(tbox_task_infos[i].mutex);
        
        if(TBOX_ID_INVALID != task_id)
        {
            break;
        }
    }
    if(TBOX_ID_INVALID != task_id)
    {
        return task_id;
    }

    /*2、选择负载最小的任务*/
    UINT32 max_load = 0xFFFFFFFFU;
    UINT32 msg_queue_size = 0U;
    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {
        xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
        {
            msg_queue_size = cqueue_datalen(&tbox_task_infos[i].msg_queue);
            if((TBOX_TASK_RUN_BUSY == tbox_task_infos[i].state ||
               TBOX_TASK_RUN_IDLE == tbox_task_infos[i].state) &&
               tbox_task_infos[i].stack_size >= stack_size &&
               tbox_task_infos[i].module_id == TBOX_ID_INVALID &&
               max_load > msg_queue_size)
            {
                max_load = msg_queue_size;
                task_id = (TBOX_ID)i;
            }
        }
        xSemaphoreGive(tbox_task_infos[i].mutex);
    }

    return task_id;
}

static inline VOID tbox_task_add_block(TBOX_ID task_id, CHAR *msg_name)
{
    xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
    {
        /*1、查找是否存在相同的阻塞信息，如果存在，则增加阻塞次数*/
        for(UINT32 i = 0U; i<TBOX_TASK_BLOCK_COUNT; i++)
        {
            if(TRUE == tbox_task_mgr.block_info[i].is_block &&
               task_id == tbox_task_mgr.block_info[i].task_id &&
               0 == strcmp(msg_name, tbox_task_mgr.block_info[i].msg_name))
            {
                tbox_task_mgr.block_info[i].con_block_num++;
                xSemaphoreGive(tbox_task_mgr.mutex);
                return;
            }
        }
        /*2、如果不存在相同的阻塞信息，则查找空闲的阻塞信息，并且将阻塞信息保存*/
        for(UINT32 i = 0U; i<TBOX_TASK_BLOCK_COUNT; i++)
        {
            if(FALSE == tbox_task_mgr.block_info[i].is_block)
            {
                tbox_task_mgr.block_info[i].is_block = TRUE;
                tbox_task_mgr.block_info[i].task_id = task_id;
                tbox_task_mgr.block_info[i].msg_name = msg_name;
                tbox_task_mgr.block_info[i].con_block_num = 1U;
                xSemaphoreGive(tbox_task_mgr.mutex);
                return;
            }
        }
    }
    xSemaphoreGive(tbox_task_mgr.mutex);    
}

static inline VOID tbox_task_remove_block(TBOX_ID task_id, CHAR *msg_name)
{
    xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
    {
        for(UINT32 i = 0U; i<TBOX_TASK_BLOCK_COUNT; i++)
        {
            if(TRUE == tbox_task_mgr.block_info[i].is_block &&
               task_id == tbox_task_mgr.block_info[i].task_id &&
               0 == strcmp(msg_name, tbox_task_mgr.block_info[i].msg_name))
            {
                tbox_task_mgr.block_info[i].is_block = FALSE;
                tbox_task_mgr.block_info[i].task_id = TBOX_ID_INVALID;
                tbox_task_mgr.block_info[i].msg_name = NULL;
                tbox_task_mgr.block_info[i].con_block_num = 0U;
                xSemaphoreGive(tbox_task_mgr.mutex);
                return;
            }
        }
    }
    xSemaphoreGive(tbox_task_mgr.mutex);
}

static VOID tbox_task_check_block(VOID)
{
    TickType_t cur_tick = xTaskGetTickCount();
    CHAR *msg_name = NULL_PTR;

    /*1、更新阻塞信息*/
    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {
        if(NULL_PTR == tbox_task_infos[i].mutex ||
           NULL_PTR == tbox_task_infos[i].handle)
        {
            continue;
        }

        msg_name = NULL_PTR;
        xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
        {
            if(TBOX_TASK_RUN_BUSY != tbox_task_infos[i].state)
            {
                xSemaphoreGive(tbox_task_infos[i].mutex);
                continue;
            }
            if(NULL_PTR == tbox_task_infos[i].cur_block_msg_name)
            {
                xSemaphoreGive(tbox_task_infos[i].mutex);
                continue;
            }
            if(cur_tick-tbox_task_infos[i].cur_block_tick > pdMS_TO_TICKS(TBOX_MESSAGE_MAX_HANDLE_TIME))
            {
                msg_name = tbox_task_infos[i].cur_block_msg_name;
                tbox_task_infos[i].cur_block_msg_name = NULL;
                tbox_task_infos[i].cur_block_tick = cur_tick;
            }
        }
        xSemaphoreGive(tbox_task_infos[i].mutex);
        if(NULL_PTR != msg_name)
        {
            tbox_task_add_block((TBOX_ID)i, msg_name);
        }
    }

    /*2、判断同个消息是否连续阻塞{TBOX_TASK_ABNORMA_BLOCK_NUM}次，如果连续阻塞{TBOX_TASK_ABNORMA_BLOCK_NUM}次，
         则上报阻塞异常信息*/
    BOOL is_block;
    TBOX_CORE_TASK_ABNORMAL_INFO abnormal_info;
    for(UINT32 i=0U; i<TBOX_TASK_BLOCK_COUNT; i++)
    {
        is_block = FALSE;
        abnormal_info.task_id = TBOX_ID_INVALID;
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        {
            if(TRUE == tbox_task_mgr.block_info[i].is_block &&
                tbox_task_mgr.block_info[i].con_block_num >= TBOX_TASK_ABNORMA_BLOCK_NUM)
            {
                is_block = TRUE;
                abnormal_info.task_id = tbox_task_mgr.block_info[i].task_id;
                abnormal_info.msg_name = tbox_task_mgr.block_info[i].msg_name;

                /*清除该阻塞信息，防止重复上报*/
                tbox_task_mgr.block_info[i].is_block = FALSE;
                tbox_task_mgr.block_info[i].task_id = TBOX_ID_INVALID;
                tbox_task_mgr.block_info[i].msg_name = NULL;
                tbox_task_mgr.block_info[i].con_block_num = 0U;
            }
        }
        xSemaphoreGive(tbox_task_mgr.mutex);
        if(TRUE == is_block && TBOX_ID_INVALID != abnormal_info.task_id)
        {
            UINT32 index = (UINT32)abnormal_info.task_id;
            xSemaphoreTake(tbox_task_infos[index].mutex, portMAX_DELAY);
            {
                abnormal_info.module_id = tbox_task_infos[index].module_id;
                abnormal_info.task_handle = (VOID *)tbox_task_infos[index].handle;
            }
            xSemaphoreGive(tbox_task_infos[index].mutex);
            tbox_supervish_report_task_abnormal(MODULE_ABNORMAL_TASK_BLOCKED,
                                                FALSE, &abnormal_info);
        }
    }
}

static inline VOID tbox_task_delete_task_by_handle(TaskHandle_t handle)
{
    if(NULL_PTR == handle)
    {
        return;
    }
    eTaskState state = eTaskGetState(handle);
    if(eDeleted == state || eInvalid == state)
    {
        return;
    }
    vTaskDelete(handle);
}

static VOID tbox_task_check_exception(VOID)
{
    UINT32 stack_size;
   // TBOX_TASK_STATE state;
    TBOX_CORE_TASK_ABNORMAL_INFO abnormal_info;
    abnormal_info.msg_name = NULL_PTR;

    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {
        if(NULL_PTR == tbox_task_infos[i].mutex)
        {
            continue;
        }

        xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
        {
            if(NULL_PTR == tbox_task_infos[i].handle)
            {
                xSemaphoreGive(tbox_task_infos[i].mutex);
                continue;
            }
            stack_size = tbox_task_infos[i].stack_size;
           // state = tbox_task_infos[i].state;
            abnormal_info.task_id = tbox_task_infos[i].task_id;
            abnormal_info.module_id = tbox_task_infos[i].module_id;
            abnormal_info.task_handle = (VOID *)tbox_task_infos[i].handle;
        }
        xSemaphoreGive(tbox_task_infos[i].mutex);
        if(NULL_PTR == abnormal_info.task_handle)
        {
            continue;
        }

        eTaskState task_state = eTaskGetState(abnormal_info.task_handle);
        if(task_state == eDeleted ||
           task_state == eInvalid)
        {
            xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
            {
                tbox_task_infos[i].state = TBOX_TASK_ABNORMAL;
                /*清除任务句柄，防止重复上报异常信息*/
                tbox_task_infos[i].handle = NULL_PTR;
            }
            xSemaphoreGive(tbox_task_infos[i].mutex);
            tbox_supervish_report_task_abnormal(MODULE_ABNORMAL_TASK_DELETED,
                                                FALSE, &abnormal_info);
            continue;
        }

        UBaseType_t high_water_mark = uxTaskGetStackHighWaterMark(abnormal_info.task_handle);
        if(high_water_mark < TBOX_TASK_HIGHWATER_THRESHOLD)
        {
            if(stack_size >= (TBOX_TASK_LARGE_STACK_SIZE/sizeof(StackType_t)))
            {
                MODULE_LOG_E(ICORE, "the size of the task[%d] stack is already at its maximum and cannot be expanded", abnormal_info.task_id);
                continue;
            }
            tbox_task_delete_task_by_handle(abnormal_info.task_handle);
            xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
            {
                tbox_task_infos[i].state = TBOX_TASK_ABNORMAL;
                /*清除任务句柄，防止重复上报异常信息*/
                tbox_task_infos[i].handle = NULL_PTR;
            }
            xSemaphoreGive(tbox_task_infos[i].mutex);
            tbox_supervish_report_task_abnormal(MODULE_ABNORMAL_TASKSTACK_SMALL, 
                                                FALSE, &abnormal_info);
        }
    }
}

static VOID tbox_task_check_runloop_exception(VOID)
{
    TaskHandle_t handle;
    TBOX_CORE_TASK_ABNORMAL_INFO abnormal_info;
    abnormal_info.msg_name = NULL_PTR;

    for(UINT8 i=0U; i<TBOX_RUNLOOP_TASK_NUM; i++)
    {
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        {
            if(TRUE == tbox_task_mgr.runloops[i].is_used &&
              NULL_PTR != tbox_task_mgr.runloops[i].handle)
            {
                handle = tbox_task_mgr.runloops[i].handle;
            }
            else
            {
                handle = NULL_PTR;
            }
        }
        xSemaphoreGive(tbox_task_mgr.mutex);

        if(NULL_PTR == handle)
        {
            continue;
        }

        eTaskState task_state = eTaskGetState(handle);
        if(task_state == eDeleted ||
           task_state == eInvalid)
        {
            xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
            {
                abnormal_info.module_id = tbox_task_mgr.runloops[i].module_id;
                abnormal_info.task_id = tbox_task_mgr.runloops[i].task_id;
                abnormal_info.task_handle = (VOID *)tbox_task_mgr.runloops[i].handle;
                tbox_task_mgr.runloops[i].state = TBOX_TASK_ABNORMAL;
                tbox_task_mgr.runloops[i].handle = NULL_PTR;
            }
            xSemaphoreGive(tbox_task_mgr.mutex);
            tbox_supervish_report_task_abnormal(MODULE_ABNORMAL_TASK_DELETED,
                                                FALSE, &abnormal_info);

        }
    }
}

static inline VOID tbox_task_delete_task(TBOX_ID task_id)
{
    UINT32 index = (UINT32)task_id;
    TBOX_TASK_INFO *task_info = &tbox_task_infos[index];
    TaskHandle_t task_handle;

    if(NULL_PTR == task_info->mutex)
    {
        task_handle = task_info->handle;
    }
    else
    {
        xSemaphoreTake(task_info->mutex, portMAX_DELAY);
        {
            task_handle = task_info->handle;
        }
        xSemaphoreGive(task_info->mutex);
    }
    if(NULL_PTR != task_handle)
    {
        xTaskNotify(task_handle, TBOX_TASK_EVENT_QUIT_BIT, eSetBits);

        eTaskState state;
        UINT8 i;
        for(i = 0U; i < 4U; i++)
        {
            xSemaphoreTake(task_info->mutex, portMAX_DELAY);
            task_handle = task_info->handle;
            xSemaphoreGive(task_info->mutex);
            if(NULL_PTR == task_handle)
            {
                break;
            }
            state = eTaskGetState(task_handle);
            if(eDeleted == state || eInvalid == state)
            {
                xSemaphoreTake(task_info->mutex, portMAX_DELAY);
                task_info->handle = NULL_PTR;
                xSemaphoreGive(task_info->mutex);
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100U));
        }
        if(i >= 4U)
        {
            xSemaphoreTake(task_info->mutex, portMAX_DELAY);
            task_handle = task_info->handle;
            task_info->handle = NULL_PTR;
            xSemaphoreGive(task_info->mutex);
            if(NULL_PTR != task_handle)
            {
                state = eTaskGetState(task_handle);
                if(eDeleted != state && eInvalid != state)
                {
                    vTaskDelete(task_handle);
                }
            }
        }
    }
}

static inline INT32 tbox_task_start_task(TBOX_ID task_id)
{
    UINT32 index = (UINT32)task_id;
    TBOX_TASK_INFO *task_info = &tbox_task_infos[index];
    TaskHandle_t handle;

    xSemaphoreTake(task_info->mutex, portMAX_DELAY);
    handle = task_info->handle;
    xSemaphoreGive(task_info->mutex);
    if(NULL_PTR == handle)
    {
        return (INT32)TBOX_E_IS_CLOSED;
    }

    eTaskState state = eTaskGetState(handle);
    if(eDeleted == state || eInvalid == state)
    {
        xSemaphoreTake(task_info->mutex, portMAX_DELAY);
        task_info->handle = NULL_PTR;
        xSemaphoreGive(task_info->mutex);
        return (INT32)TBOX_E_IS_CLOSED;
    }
    else if(state == eSuspended)
    {
        vTaskResume(handle);
    } 
    else
    {
        /*TODO*/
    }
    xTaskNotify(handle, TBOX_TASK_EVENT_START_BIT, eSetBits);

    return (INT32)TBOX_E_OK;
}

/*
static inline VOID tbox_task_start_task_by_handle(TaskHandle_t handle)
{
    if(NULL_PTR == handle)
    {
        return;
    }

    eTaskState state = eTaskGetState(handle);
    if(state == eSuspended)
    {
        vTaskResume(handle);
    }
}*/

static inline INT32 tbox_task_stop_task(TBOX_ID task_id)
{
    UINT32 index = (UINT32)task_id;
    TBOX_TASK_INFO *task_info = &tbox_task_infos[index];
    TaskHandle_t handle;

    xSemaphoreTake(task_info->mutex, portMAX_DELAY);
    handle = task_info->handle;
    xSemaphoreGive(task_info->mutex);
    if(NULL_PTR == handle)
    {
        return (INT32)TBOX_E_IS_CLOSED;
    }
    eTaskState state = eTaskGetState(handle);
    if(eDeleted == state || eInvalid == state)
    {
        xSemaphoreTake(task_info->mutex, portMAX_DELAY);
        task_info->handle = NULL_PTR;
        xSemaphoreGive(task_info->mutex);
        return (INT32)TBOX_E_IS_CLOSED;
    }
    xTaskNotify(handle, TBOX_TASK_EVENT_STOP_BIT, eSetBits);

    return (INT32)TBOX_E_OK;
}

/*
static inline VOID tbox_task_stop_task_by_handle(TaskHandle_t handle)
{
    if(NULL_PTR == handle)
    {
        return;
    }

    eTaskState state = eTaskGetState(handle);
    if(eRunning == state || 
      eReady == state || 
      eBlocked == state)
    {
        vTaskSuspend(handle);
    }    
}*/

static VOID tbox_task_process(VOID *param)
{
    UINT32 wait_tick = portMAX_DELAY;
    UINT8 *msg_data_buf = NULL_PTR;
    UINT32 wait_ret_bits = 0U;   
    TBOX_MSG_HEADER msg_header;
    TBOX_MSG_DATA msg_data;
    TBOX_TASK_INFO *task_info = (TBOX_TASK_INFO *)param;

    if(NULL_PTR == task_info || 
       NULL_PTR == task_info->mutex)
    {
        xSemaphoreTake(task_info->mutex, portMAX_DELAY);
        task_info->handle = NULL_PTR;
        task_info->state = TBOX_TASK_ABNORMAL;
        xSemaphoreGive(task_info->mutex);
        vTaskDelete(NULL_PTR);
        MODULE_LOG_F(ICORE, "the task info is invalid, task_id=%d", task_info->task_id);
        return;
    }

    msg_data_buf = tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, TBOX_MESSAGE_DATA_MAX_LEN);
    if(NULL_PTR == msg_data_buf)
    {
        xSemaphoreTake(task_info->mutex, portMAX_DELAY);
        task_info->handle = NULL_PTR;
        task_info->state = TBOX_TASK_ABNORMAL;
        xSemaphoreGive(task_info->mutex);
        vTaskDelete(NULL_PTR);
        MODULE_LOG_F(ICORE, "task %d alloc msg data failed", task_info->task_id);
        return;        
    }
    /*设置任务优先级，有可能任务创建后，优先级已经被修改*/
    if(uxTaskPriorityGet(NULL_PTR) != task_info->priority)
    {
        vTaskPrioritySet(NULL_PTR, task_info->priority);
    }

    for(;;)
    {
        wait_ret_bits = 0U;
        xTaskNotifyWait(0U, TBOX_TASK_EVENT_ALL_BITS, &wait_ret_bits, wait_tick);

        /*退出任务通知处理*/
        if((wait_ret_bits & TBOX_TASK_EVENT_QUIT_BIT) == TBOX_TASK_EVENT_QUIT_BIT)
        {
            /*TODO:通知任务退出成功*/
            break;
        }

        /*任务启动通知处理*/
        if((wait_ret_bits & TBOX_TASK_EVENT_START_BIT) == TBOX_TASK_EVENT_START_BIT)
        {
            wait_tick = pdMS_TO_TICKS(100U);
        }

        /*任务停止通知处理*/
        if((wait_ret_bits & TBOX_TASK_EVENT_STOP_BIT) == TBOX_TASK_EVENT_STOP_BIT)
        {
            wait_tick = portMAX_DELAY;
        }

        /*连续处理消息*/
        for(;;)
        {
            msg_header.name = NULL_PTR;
            msg_header.handler = NULL_PTR;
            xSemaphoreTake(task_info->mutex, portMAX_DELAY);
            {
                if(TBOX_TASK_NO_INIT == task_info->state ||
                    TBOX_TASK_ISFORBIDDEN == task_info->state ||
                    TBOX_TASK_RECOVERING == task_info->state ||
                    TBOX_TASK_ABNORMAL == task_info->state)
                {
                    task_info->handle = NULL_PTR;
                    task_info->state = TBOX_TASK_ABNORMAL;
                    xSemaphoreGive(task_info->mutex);
                    vTaskDelete(NULL_PTR);
                    MODULE_LOG_E(ICORE, "the task has been quit, task_id=%d, state=%d", 
                                 task_info->task_id, 
                                 task_info->state);
                    return;
                }
                if(TBOX_TASK_RUN_IDLE != task_info->state &&
                   TBOX_TASK_RUN_BUSY != task_info->state)
                {
                    xSemaphoreGive(task_info->mutex);
                    MODULE_LOG_W(ICORE, "the task is not running, task_id=%d, state=%d", 
                                 task_info->task_id, 
                                 task_info->state);
                    break;
                }
                if(cqueue_datalen(&task_info->msg_queue) < sizeof(TBOX_MSG_HEADER))
                {
                    xSemaphoreGive(task_info->mutex);
                    break;         
                }
                cqueue_get(&task_info->msg_queue, (UINT8 *)&msg_header, sizeof(TBOX_MSG_HEADER));
                if(cqueue_datalen(&task_info->msg_queue) < msg_header.len)
                {
                    task_info->msg_queue.in = task_info->msg_queue.out = 0U;
                    xSemaphoreGive(task_info->mutex);
                    break;
                }
                cqueue_get(&task_info->msg_queue, msg_data_buf, msg_header.len);
                task_info->state = TBOX_TASK_RUN_BUSY;
                task_info->cur_block_tick = xTaskGetTickCount();
                task_info->cur_block_msg_name = msg_header.name;
            }
            xSemaphoreGive(task_info->mutex);

            if(NULL_PTR != msg_header.name && 
               NULL_PTR != msg_header.handler &&
               TBOX_MSG_MAGICNUM == msg_header.msgic_no)
            {
                if(TRUE == tbox_message_is_enabled(msg_header.name))
                {
                    msg_data.data = msg_data_buf;
                    msg_data.size = msg_header.len;
                    (*msg_header.handler)(msg_header.name, &msg_data);
                }               
            }

            xSemaphoreTake(task_info->mutex, portMAX_DELAY);
            {
                task_info->state = TBOX_TASK_RUN_IDLE;
                task_info->cur_block_tick = xTaskGetTickCount();
                task_info->cur_block_msg_name = NULL_PTR;
            }
            xSemaphoreGive(task_info->mutex);

            if(task_info->task_id > TBOX_ID_INVALID && NULL_PTR != msg_header.name)
            {
                tbox_task_remove_block(task_info->task_id, msg_header.name);
            }
        }
    }
    
    MODULE_LOG_E(ICORE, "the task is quit, task_id=%d, state=%d", 
                                 task_info->task_id, 
                                 task_info->state);

    if(NULL_PTR != msg_data_buf)
    {
        tbox_memory_free(TBOX_MEMORY_TYPE_CORE, msg_data_buf);
    }

    xSemaphoreTake(task_info->mutex, portMAX_DELAY);
    task_info->handle = NULL_PTR;
    task_info->state = TBOX_TASK_ABNORMAL;
    xSemaphoreGive(task_info->mutex);
    vTaskDelete(NULL_PTR);
}

static VOID tbox_task_runloop_process(VOID *param)
{
    TBOX_RUNLOOP_INFO *runloop_info = (TBOX_RUNLOOP_INFO *)param;
    if(NULL_PTR == runloop_info)
    {
        vTaskDelete(NULL_PTR);
        return;
    }
    MODULE_RUNLOOP_FUN runloop = runloop_info->runloop_func;
    if(NULL_PTR == runloop)
    {
        vTaskDelete(NULL_PTR);
        return;
    }

    xTaskNotifyWait(0U, 0xFFFFFFFFU, NULL_PTR, portMAX_DELAY);
    
    runloop(param);

    xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
    {
        runloop_info->handle = NULL_PTR;
        runloop_info->state = TBOX_TASK_ABNORMAL;
    }
    xSemaphoreGive(tbox_task_mgr.mutex);
    
    vTaskDelete(NULL_PTR);
}

INT32 tbox_task_init(VOID)
{
    CHAR task_name[16];
    UINT32 stack_size = 0U;
    UINT8 *buffer;

    memset(tbox_task_infos, 0, sizeof(tbox_task_infos));
    memset(&tbox_task_mgr, 0, sizeof(tbox_task_mgr));

    /*初始化任务信息
    *1、设置任务初始化状态和初始化栈大小
     2、创建任务互斥锁
     3、创建任务，任务运行后会等待启动事件，启动事件通知后，任务就可以处理消息通知了；
        当任务异常时，需要退出可以发送任务退出事件，通知任务退出；
        当任务需要停止时，可以发送任务停止事件，通知任务停止处理消息。
    */
    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {
        if(i < TBOX_TASK_SMALL_SIZE_NUM)
        {
            stack_size = TBOX_TASK_SMALL_STACK_SIZE/sizeof(StackType_t);
        }
        else if(i < (TBOX_TASK_SMALL_SIZE_NUM+TBOX_TASK_MEDIUM_SIZE_NUM))
        {
            stack_size = TBOX_TASK_MEDIUM_STACK_SIZE/sizeof(StackType_t);
        }
        else
        {
            stack_size = TBOX_TASK_LARGE_STACK_SIZE/sizeof(StackType_t);
        }
        tbox_task_infos[i].state = TBOX_TASK_STOP;
        tbox_task_infos[i].stack_size = stack_size;
        tbox_task_infos[i].priority = TBOX_TASK_PRIORITY_MID;
        tbox_task_infos[i].module_id = TBOX_ID_INVALID;
        tbox_task_infos[i].cur_block_msg_name = NULL_PTR;
        tbox_task_infos[i].cur_block_tick = 0U;
        tbox_task_infos[i].task_id = (TBOX_ID)i;
        tbox_task_infos[i].handle = NULL_PTR;
        cqueue_init(&tbox_task_infos[i].msg_queue, NULL_PTR, 0U);

        buffer = (UINT8 *)tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, 
                                             TBOX_TASK_MSG_QUEUE_SIZE);
        if(NULL_PTR == buffer)
        {
            tbox_task_deinit();
            return (INT32)TBOX_E_FAILED_ALLOC;
        }
        cqueue_init(&tbox_task_infos[i].msg_queue, buffer, TBOX_TASK_MSG_QUEUE_SIZE);
        
        tbox_task_infos[i].mutex = xSemaphoreCreateMutex();
        if(NULL_PTR == tbox_task_infos[i].mutex)
        {
            tbox_task_deinit();
            return (INT32)TBOX_E_FAILED_CREATE;
        }

        snprintf(task_name, 16, "TBOX_TASK%d", i);
        if(pdPASS != xTaskCreate(tbox_task_process, 
                                 task_name, 
                                 stack_size, 
                                 (VOID *)&tbox_task_infos[i], 
                                 tbox_task_infos[i].priority, 
                                 &tbox_task_infos[i].handle))
        {
            tbox_task_deinit();
            return (INT32)TBOX_E_FAILED_CREATE;
        }
    }

    /*初始化管理信息*/
    tbox_task_mgr.mutex = xSemaphoreCreateMutex();
    if(NULL == tbox_task_mgr.mutex)
    {
        tbox_task_deinit();
        return (INT32)TBOX_E_FAILED_CREATE;
    }
    for(UINT32 i=0U; i<TBOX_TASK_BLOCK_COUNT; i++)
    {
        tbox_task_mgr.block_info[i].is_block = FALSE;
        tbox_task_mgr.block_info[i].con_block_num = 0U;
        tbox_task_mgr.block_info[i].msg_name = NULL;
        tbox_task_mgr.block_info[i].task_id = TBOX_ID_INVALID;
    }
    for(UINT32 i=0U; i<TBOX_RUNLOOP_TASK_NUM; i++)
    {
        tbox_task_mgr.runloops[i].is_used = FALSE;
        tbox_task_mgr.runloops[i].task_id = TBOX_ID_INVALID;
        tbox_task_mgr.runloops[i].module_id = TBOX_ID_INVALID;
        tbox_task_mgr.runloops[i].handle = NULL_PTR;
        tbox_task_mgr.runloops[i].state = TBOX_TASK_NO_INIT;
    }

    return (INT32)TBOX_E_OK;
}

VOID tbox_task_deinit(VOID)
{
    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {
        tbox_task_delete_task((TBOX_ID)i);

        if(NULL_PTR != tbox_task_infos[i].mutex)
        {
            xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
        }
        if(NULL_PTR != tbox_task_infos[i].msg_queue.buffer)
        {
            tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_task_infos[i].msg_queue.buffer);
            cqueue_init(&tbox_task_infos[i].msg_queue, NULL_PTR, 0U);
        }
        tbox_task_infos[i].state = TBOX_TASK_NO_INIT;
        if(NULL_PTR != tbox_task_infos[i].mutex)
        {
            xSemaphoreGive(tbox_task_infos[i].mutex);
        }
        if(NULL_PTR != tbox_task_infos[i].mutex)
        {
            vSemaphoreDelete(tbox_task_infos[i].mutex);
            tbox_task_infos[i].mutex = NULL_PTR;
        }
    }

    if(NULL_PTR != tbox_task_mgr.mutex)
    {
        TaskHandle_t handle;
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        for(UINT32 i=0U; i<TBOX_TASK_BLOCK_COUNT; i++)
        {
            tbox_task_mgr.block_info[i].is_block = FALSE;
            tbox_task_mgr.block_info[i].con_block_num = 0U;
            tbox_task_mgr.block_info[i].msg_name = NULL;
            tbox_task_mgr.block_info[i].task_id = TBOX_ID_INVALID;
        }
        for(UINT32 i=0U; i<TBOX_RUNLOOP_TASK_NUM; i++)
        {
            if(FALSE == tbox_task_mgr.runloops[i].is_used ||
              NULL_PTR == tbox_task_mgr.runloops[i].handle)
            {
                continue;
            }

            tbox_task_mgr.runloops[i].is_used = FALSE;
            tbox_task_mgr.runloops[i].task_id = TBOX_ID_INVALID;
            tbox_task_mgr.runloops[i].module_id = TBOX_ID_INVALID;
            handle = tbox_task_mgr.runloops[i].handle;
            tbox_task_mgr.runloops[i].handle = NULL_PTR;
            tbox_task_mgr.runloops[i].state = TBOX_TASK_NO_INIT;
            xSemaphoreGive(tbox_task_mgr.mutex);

            tbox_task_delete_task_by_handle(handle);

            xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        }
        xSemaphoreGive(tbox_task_mgr.mutex);
        vSemaphoreDelete(tbox_task_mgr.mutex);
        tbox_task_mgr.mutex = NULL_PTR;
    }

    MODULE_LOG_D(ICORE, "task deinit success");
}

INT32 tbox_task_start(VOID)
{
    /*1、重置任务阻塞信息*/
    xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
    {
        for(UINT32 i=0U; i<TBOX_TASK_BLOCK_COUNT; i++)
        {
            tbox_task_mgr.block_info[i].is_block = FALSE;
            tbox_task_mgr.block_info[i].con_block_num = 0U;
            tbox_task_mgr.block_info[i].msg_name = NULL;
            tbox_task_mgr.block_info[i].task_id = TBOX_ID_INVALID;
        }        
    }
    xSemaphoreGive(tbox_task_mgr.mutex);

    /*启动各个任务*/
    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {        
        if(NULL_PTR == tbox_task_infos[i].mutex)
        {
            xSemaphoreGive(tbox_task_infos[i].mutex);
            continue;
        }

        xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
        {
            if(TBOX_TASK_STOP != tbox_task_infos[i].state)  
            {
                xSemaphoreGive(tbox_task_infos[i].mutex);
                continue;
            }
            tbox_task_infos[i].state = TBOX_TASK_RUN_IDLE;
            tbox_task_infos[i].cur_block_msg_name = NULL_PTR;
            tbox_task_infos[i].cur_block_tick = 0U;
            tbox_task_infos[i].msg_queue.in = tbox_task_infos[i].msg_queue.out = 0U;
        }
        xSemaphoreGive(tbox_task_infos[i].mutex);

        if((INT32)TBOX_E_IS_CLOSED == tbox_task_start_task((TBOX_ID)i))
        {
            xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
            {
               tbox_task_infos[i].state = TBOX_TASK_ABNORMAL; 
            }
            xSemaphoreGive(tbox_task_infos[i].mutex);
            continue;
        }
    }

    TaskHandle_t handle;
    xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
    {
        for(UINT8 i=0U; i<TBOX_RUNLOOP_TASK_NUM; i++)
        {
            if(FALSE == tbox_task_mgr.runloops[i].is_used ||
               NULL_PTR == tbox_task_mgr.runloops[i].handle)
            {
                continue;
            }
            handle = tbox_task_mgr.runloops[i].handle;
            xSemaphoreGive(tbox_task_mgr.mutex);

            xTaskNotify(handle, TBOX_TASK_START_RUNLOOP_BIT, eNoAction);

            xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY); 
        }
    }
    xSemaphoreGive(tbox_task_mgr.mutex);    

    MODULE_LOG_D(ICORE, "start task success");

    return (INT32)TBOX_E_OK;
}

INT32 tbox_task_stop(VOID)
{
    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {
        if(NULL_PTR == tbox_task_infos[i].mutex)
        {
            continue;
        }

        xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
        {
            if(TBOX_TASK_NO_INIT == tbox_task_infos[i].state ||
               TBOX_TASK_ISFORBIDDEN == tbox_task_infos[i].state ||
               TBOX_TASK_STOP == tbox_task_infos[i].state)  
            {
                xSemaphoreGive(tbox_task_infos[i].mutex);
                continue;
            }
            tbox_task_infos[i].state = TBOX_TASK_STOP;
            tbox_task_infos[i].msg_queue.in = tbox_task_infos[i].msg_queue.out = 0U;         
        }
        xSemaphoreGive(tbox_task_infos[i].mutex);

        if((INT32)TBOX_E_IS_CLOSED == tbox_task_stop_task((TBOX_ID)i))
        {
             xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
             {
                tbox_task_infos[i].state = TBOX_TASK_ISFORBIDDEN; 
             }
             xSemaphoreGive(tbox_task_infos[i].mutex);
             continue;
        }
    }

    MODULE_LOG_D(ICORE, "stop task success");

    return (INT32)TBOX_E_OK;
}

TBOX_ID tbox_task_create_runloop(TBOX_ID module_id, 
                               CHAR *task_name, 
                               UINT8 pripority, 
                               UINT32 stack_size, 
                               MODULE_RUNLOOP_FUN runloop)
{
    TBOX_ID task_id = TBOX_ID_INVALID;
    TaskHandle_t handle = NULL_PTR;
    TBOX_RUNLOOP_INFO *runloop_info = NULL_PTR;

    if(module_id <= TBOX_ID_INVALID || NULL_PTR == runloop)
    {
        MODULE_LOG_E(ICORE, "the module id or runloop is invalid");
        return task_id;
    }

    xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
    {
        for(UINT8 i=0U; i<TBOX_RUNLOOP_TASK_NUM; i++)
        {
            if(FALSE == tbox_task_mgr.runloops[i].is_used)
            {
                runloop_info = &tbox_task_mgr.runloops[i];
                task_id = (TBOX_ID)i;
                break;
            }
        }
    }
    xSemaphoreGive(tbox_task_mgr.mutex);
    if(NULL_PTR == runloop_info)
    {
        MODULE_LOG_E(ICORE, "no resource for create runloop task");
        return task_id;        
    }

    if(pdPASS != xTaskCreate(tbox_task_runloop_process, 
                             task_name,
                             stack_size/sizeof(StackType_t),
                             (VOID *)runloop_info, 
                             pripority,
                             &handle))
    {
        MODULE_LOG_E(ICORE, "failed to create runloop task");
        return TBOX_ID_INVALID;        
    }

    xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
    {
        runloop_info->is_used = TRUE;
        runloop_info->module_id = module_id;
        runloop_info->handle = handle;
        runloop_info->priority = pripority;
        runloop_info->stack_size = stack_size;
        runloop_info->runloop_func = runloop;
        runloop_info->task_id = task_id+TBOX_TASK_MAX_NUM;
        runloop_info->state = TBOX_TASK_RUN_IDLE;
    }
    xSemaphoreGive(tbox_task_mgr.mutex);

    return (task_id+TBOX_TASK_MAX_NUM);   
}

MODULE_HANDLE tbox_task_get_handle(TBOX_ID task_id)
{
    MODULE_HANDLE handle = NULL_PTR;
    
    if(task_id < 0 || task_id >= (TBOX_TASK_MAX_NUM+TBOX_RUNLOOP_TASK_NUM))
    {
        return handle;
    }

    if(task_id < TBOX_TASK_MAX_NUM)
    {
        xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
        {
            handle = tbox_task_infos[task_id].handle;
        }
        xSemaphoreGive(tbox_task_infos[task_id].mutex);
    }
    else
    {
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        {
            handle = tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].handle;
        }
        xSemaphoreGive(tbox_task_mgr.mutex);
    }

    return handle;
}

TBOX_ID tbox_task_find_by_handle(VOID *handle)
{
    TBOX_ID task_id = TBOX_ID_INVALID;

    if(NULL_PTR == handle)
    {
        return TBOX_ID_INVALID;
    }

    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {
        if(NULL_PTR == tbox_task_infos[i].mutex ||
           NULL_PTR == tbox_task_infos[i].handle)
        {
            continue;
        }
        
        xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
        {
            if(tbox_task_infos[i].handle == handle)
            {
                task_id = tbox_task_infos[i].task_id;
            }
        }
        xSemaphoreGive(tbox_task_infos[i].mutex);
        if(TBOX_ID_INVALID != task_id)
        {
            break;
        }  
    }
    if(TBOX_ID_INVALID == task_id)
    {
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        {
            for(UINT8 i=0U; i<TBOX_RUNLOOP_TASK_NUM; i++)
            {
                if(tbox_task_mgr.runloops[i].handle == handle)
                {
                    task_id = tbox_task_mgr.runloops[i].task_id;
                    break;
                }
            }
        }
        xSemaphoreGive(tbox_task_mgr.mutex);
    }

    return task_id;
}

TBOX_ID tbox_task_attach(TBOX_ID module_id, UINT8 pripority, UINT32 stack_size)
{
    TBOX_ID task_id = TBOX_ID_INVALID;

    if(module_id <= TBOX_ID_INVALID)
    {
        return task_id;
    }
    if(pripority <  TBOX_TASK_PRIORITY_LOW ||
       pripority >  TBOX_TASK_PRIORITY_HIGH)
    {
        return task_id;
    }
    if(stack_size > TBOX_TASK_LARGE_STACK_SIZE)
    {
        return task_id;
    }
    stack_size = stack_size/sizeof(StackType_t);
    
    for(UINT32 i=0U; i<TBOX_TASK_MAX_NUM; i++)
    {
        if(NULL_PTR == tbox_task_infos[i].mutex)
        {
            continue;
        }

        task_id = TBOX_ID_INVALID;
        xSemaphoreTake(tbox_task_infos[i].mutex, portMAX_DELAY);
        {
            if(NULL_PTR == tbox_task_infos[i].handle)
            {
                xSemaphoreGive(tbox_task_infos[i].mutex);
                continue;
            }
            if(TBOX_TASK_NO_INIT == tbox_task_infos[i].state ||
               TBOX_TASK_ISFORBIDDEN == tbox_task_infos[i].state ||
               TBOX_TASK_ABNORMAL == tbox_task_infos[i].state)  
            {
                xSemaphoreGive(tbox_task_infos[i].mutex);
                continue;
            }
            if(module_id == tbox_task_infos[i].module_id)
            {
                task_id = (TBOX_ID)i;
                xSemaphoreGive(tbox_task_infos[i].mutex);
                break;     
            }
            else
            {
                if(tbox_task_infos[i].stack_size >= stack_size && 
                   tbox_task_infos[i].module_id == TBOX_ID_INVALID)
                {
                    task_id = (TBOX_ID)i; 
                    tbox_task_infos[i].module_id = module_id;
                    tbox_task_infos[i].priority = pripority;
                    tbox_task_infos[i].cur_block_msg_name = NULL_PTR;
                    tbox_task_infos[i].cur_block_tick = 0U;
                    xSemaphoreGive(tbox_task_infos[i].mutex);
                    break;  
                }
            }
        }
        xSemaphoreGive(tbox_task_infos[i].mutex);
    }

    return task_id;
}

INT32 tbox_task_detach(TBOX_ID task_id)
{
    if(task_id < 0 || task_id >= TBOX_TASK_MAX_NUM)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(NULL_PTR == tbox_task_infos[task_id].mutex)
    {
        return (INT32)TBOX_E_NOINIT;
    }

    xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
    {
        tbox_task_infos[task_id].module_id = TBOX_ID_INVALID;
        tbox_task_infos[task_id].msg_queue.in = tbox_task_infos[task_id].msg_queue.out = 0U; 
    }
    xSemaphoreGive(tbox_task_infos[task_id].mutex);
    
    return (INT32)TBOX_E_OK;
}

INT32 tbox_task_get_priority(TBOX_ID task_id)
{
    UINT8 priority;

    if(task_id < 0 || task_id >= (TBOX_TASK_MAX_NUM+TBOX_RUNLOOP_TASK_NUM))
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    if(task_id < TBOX_TASK_MAX_NUM)
    {
        if(NULL_PTR == tbox_task_infos[task_id].mutex)
        {
            return (INT32)TBOX_E_NOINIT;
        }
        
        xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
        priority = tbox_task_infos[task_id].priority;
        xSemaphoreGive(tbox_task_infos[task_id].mutex);
    }
    else
    {
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        {
            priority = tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].priority;
        }
        xSemaphoreGive(tbox_task_mgr.mutex);
    }

    return priority;   
}

INT32 tbox_task_add_msg(TBOX_ID task_id, 
                        CHAR *msg_name, 
                        TBOX_MSG_HADNLER msg_handle, 
                        UINT16 msg_len, 
                        UINT8 *msg_data)
{
    if(task_id < 0 || 
       task_id >= TBOX_TASK_MAX_NUM || 
       NULL_PTR == msg_name || 
       NULL_PTR == msg_handle)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(NULL_PTR == tbox_task_infos[task_id].mutex)
    {
        MODULE_LOG_E(ICORE, "the task is no init");
        return (INT32)TBOX_E_NOINIT;
    }

    TaskHandle_t handle = NULL_PTR;
    TBOX_MSG_HEADER header;
    header.msgic_no = TBOX_MSG_MAGICNUM;
    header.name = msg_name;
    header.len = msg_len;
    header.handler = msg_handle;
    xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
    {
        if(NULL_PTR == tbox_task_infos[task_id].handle)
        {
            xSemaphoreGive(tbox_task_infos[task_id].mutex);
            MODULE_LOG_E(ICORE, "the task handle is null");
            return (INT32)TBOX_E_INVALID_HANDLE;
        }
        if(TBOX_TASK_RUN_IDLE != tbox_task_infos[task_id].state &&
           TBOX_TASK_RUN_BUSY != tbox_task_infos[task_id].state)  
        {
            xSemaphoreGive(tbox_task_infos[task_id].mutex);
            MODULE_LOG_W(ICORE, "the task state is invalid, state:%d", tbox_task_infos[task_id].state);   
            return (INT32)TBOX_E_INVALID_STATE;
        }
        if(cqueue_surplus(&tbox_task_infos[task_id].msg_queue) < (msg_len+sizeof(TBOX_MSG_HEADER)))
        {
            xSemaphoreGive(tbox_task_infos[task_id].mutex);
            MODULE_LOG_W(ICORE, "the task memory is not enough");
            return (INT32)TBOX_E_NOMEMORY;
        }
        cqueue_put(&tbox_task_infos[task_id].msg_queue, (UINT8*)&header, sizeof(TBOX_MSG_HEADER));
        if(NULL_PTR != msg_data)
        {
            cqueue_put(&tbox_task_infos[task_id].msg_queue, msg_data, msg_len);
        }
        handle = tbox_task_infos[task_id].handle;
    }
    xSemaphoreGive(tbox_task_infos[task_id].mutex);

    xTaskNotify(handle, TBOX_TASK_EVENT_MSG_BIT, eSetBits);

    MODULE_LOG_D(ICORE, "add message:%s to task[%d]", msg_name, task_id);

    return (INT32)TBOX_E_OK;
}

INT32 tbox_taskpool_add_msg(UINT32 stack_size, 
                             CHAR *msg_name, 
                             TBOX_MSG_HADNLER msg_handle, 
                             UINT16 msg_len, 
                             UINT8 *msg_data)
{
    if(NULL_PTR == msg_name || 
       NULL_PTR == msg_handle ||
       stack_size > TBOX_TASK_LARGE_STACK_SIZE)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    stack_size = stack_size/sizeof(StackType_t);
    TBOX_ID task_id = tbox_taskpool_select_task(stack_size);
    if(TBOX_ID_INVALID == task_id)
    {
        MODULE_LOG_W(ICORE, "it is no task to handle callable");
        return (INT32)TBOX_E_IS_BUSY;
    }
    
    TaskHandle_t handle = NULL_PTR;
    TBOX_MSG_HEADER header;
    header.msgic_no = TBOX_MSG_MAGICNUM;
    header.name = msg_name;
    header.len = msg_len;
    header.handler = msg_handle;
    xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
    {
        if(TBOX_TASK_RUN_IDLE != tbox_task_infos[task_id].state &&
           TBOX_TASK_RUN_BUSY != tbox_task_infos[task_id].state)  
        {
            xSemaphoreGive(tbox_task_infos[task_id].mutex);
            MODULE_LOG_W(ICORE, "the task state is invalid, state:%d", tbox_task_infos[task_id].state);   
            return (INT32)TBOX_E_INVALID_STATE;
        }
        if(cqueue_surplus(&tbox_task_infos[task_id].msg_queue) < (msg_len+sizeof(TBOX_MSG_HEADER)))
        {
            xSemaphoreGive(tbox_task_infos[task_id].mutex);
            MODULE_LOG_W(ICORE, "the task memory is not enough");
            return (INT32)TBOX_E_NOMEMORY;
        }
        cqueue_put(&tbox_task_infos[task_id].msg_queue, (UINT8*)&header, sizeof(TBOX_MSG_HEADER));
        if(NULL_PTR != msg_data)
        {
            cqueue_put(&tbox_task_infos[task_id].msg_queue, msg_data, msg_len);
        }
        handle = tbox_task_infos[task_id].handle;
    }
    xSemaphoreGive(tbox_task_infos[task_id].mutex);

    xTaskNotify(handle, TBOX_TASK_EVENT_MSG_BIT, eSetBits);

    MODULE_LOG_D(ICORE, "add callable success, message:%s task_id:%d", msg_name, task_id);

    return (INT32)TBOX_E_OK;    
}

TBOX_TASK_STATE tbox_task_get_state(TBOX_ID task_id)
{
    TBOX_TASK_STATE state = TBOX_TASK_STATE_INVALID;
    if(task_id < 0 || task_id >= TBOX_TASK_MAX_NUM+TBOX_RUNLOOP_TASK_NUM)
    {
        return state;
    }

    if(task_id < TBOX_TASK_MAX_NUM)
    {
        if(NULL_PTR == tbox_task_infos[task_id].mutex)
        {
            return state;
        }

        xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
        {
            state = tbox_task_infos[task_id].state;
        }
        xSemaphoreGive(tbox_task_infos[task_id].mutex);
    }
    else
    {
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        {
            state = tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].state;
        }
        xSemaphoreGive(tbox_task_mgr.mutex);
    }

    return state;
}

VOID  tbox_reset_task(TBOX_ID task_id)
{
    if(task_id < 0 || task_id >= TBOX_TASK_MAX_NUM+TBOX_RUNLOOP_TASK_NUM)
    {
        return;
    }
    
    TaskHandle_t handle = NULL_PTR;
    TBOX_MSG_DATA data;
    data.size = sizeof(TBOX_CORE_TASK_UPDATE_INFO);
    if(task_id < TBOX_TASK_MAX_NUM)
    {
        if(NULL_PTR == tbox_task_infos[task_id].mutex)
        {
            return;
        }

        UINT32 stack_size;
        UINT32 priority;
        xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
        {
            if(TBOX_TASK_NO_INIT == tbox_task_infos[task_id].state ||
              TBOX_TASK_RECOVERING == tbox_task_infos[task_id].state)
            {
                xSemaphoreGive(tbox_task_infos[task_id].mutex);
                MODULE_LOG_W(ICORE, "the task stat is invalid, state:%d", tbox_task_infos[task_id].state);
                return;
            }
            tbox_task_infos[task_id].msg_queue.in = tbox_task_infos[task_id].msg_queue.out = 0U;     
            tbox_task_infos[task_id].state = TBOX_TASK_RECOVERING;
            stack_size = tbox_task_infos[task_id].stack_size;
            priority = tbox_task_infos[task_id].priority;           
        }
        xSemaphoreGive(tbox_task_infos[task_id].mutex);

        tbox_task_delete_task(task_id);

        UINT32 system_size = xPortGetFreeHeapSize();
        if(system_size <= stack_size*sizeof(StackType_t))
        {
            xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
            {
                tbox_task_infos[task_id].state = TBOX_TASK_ABNORMAL;
            }
            xSemaphoreGive(tbox_task_infos[task_id].mutex);
            MODULE_LOG_W(ICORE, "the system memory is not enough, system_size:%d, stack_size:%d", system_size, stack_size*sizeof(StackType_t));
            return;         
        }

        CHAR task_name[16];
        snprintf(task_name, 16, "TBOX_TASK%d", task_id);
        if(pdPASS != xTaskCreate(tbox_task_process, 
                                task_name, 
                                stack_size, 
                                (VOID *)&tbox_task_infos[task_id], 
                                priority, 
                                &handle))
        {
            xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
            {
                tbox_task_infos[task_id].state = TBOX_TASK_ABNORMAL;     
            }
            xSemaphoreGive(tbox_task_infos[task_id].mutex);
            MODULE_LOG_E(ICORE, "create task failed, task_id:%d task_name:%s", task_id, task_name);
        }
        else
        {
            TBOX_CORE_TASK_UPDATE_INFO updata_info;
            xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
            {
                tbox_task_infos[task_id].state = TBOX_TASK_RUN_IDLE;
                tbox_task_infos[task_id].handle = handle;
                updata_info.module_id = tbox_task_infos[task_id].module_id;
                updata_info.task_handle = handle;
            }
            xSemaphoreGive(tbox_task_infos[task_id].mutex);
            xTaskNotify(handle, TBOX_TASK_EVENT_START_BIT, eSetBits);
            data.data = (UINT8 *)&updata_info;
            tbox_message_publish(TBOX_CORE_TASK_UPDATE_NOTIFY, &data);
            MODULE_LOG_I(ICORE, "reset task success, task_id:%d task_name:%s", task_name);
        }
    }
    else
    {
        UINT8 priority;
        UINT32 stack_size;
        MODULE_RUNLOOP_FUN runloop_fun;
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        {
            if( FALSE == tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].is_used ||
                TBOX_TASK_RECOVERING == tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].state)
            {
                xSemaphoreGive(tbox_task_mgr.mutex);
                MODULE_LOG_W(ICORE, "the task stat is invalid, state:%d", tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].state);
                return;
            }
            handle = tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].handle;
            priority = tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].priority;
            stack_size = tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].stack_size;
            runloop_fun = tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].runloop_func;
        }
        xSemaphoreGive(tbox_task_mgr.mutex);

        tbox_task_delete_task_by_handle(handle);

        CHAR task_name[16];
        snprintf(task_name, 16, "TBOX_RUNLOOP%d", task_id-TBOX_TASK_MAX_NUM);
        if(pdPASS != xTaskCreate(runloop_fun, 
                                task_name, 
                                stack_size, 
                                NULL_PTR, 
                                priority, 
                                &handle))
        {
            MODULE_LOG_E(ICORE, "create task failed, task_id:%d name:%s", task_id, task_name);
            handle = NULL_PTR;
        }

        TBOX_CORE_TASK_UPDATE_INFO updata_info;
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        {
            if(NULL_PTR == handle)
            {
                tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].state = TBOX_TASK_ABNORMAL;
            }
            else
            {
                tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].handle = handle;
                tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].state = TBOX_TASK_RUN_IDLE;
                updata_info.module_id = tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].module_id;
                updata_info.task_handle = handle;
            }
        }
        xSemaphoreGive(tbox_task_mgr.mutex);
        if(NULL_PTR != handle)
        {
            xTaskNotify(handle, TBOX_TASK_EVENT_START_BIT, eNoAction);
            data.data = (UINT8 *)&updata_info;
            tbox_message_publish(TBOX_CORE_TASK_UPDATE_NOTIFY, &data);
        }        
    }
}

VOID  tbox_forbid_task(TBOX_ID task_id)
{
    if(task_id < 0 || task_id >= TBOX_TASK_MAX_NUM+TBOX_RUNLOOP_TASK_NUM)
    {
        return;
    }

    if(task_id < TBOX_TASK_MAX_NUM)
    {
        if(NULL_PTR == tbox_task_infos[task_id].mutex)
        {
            return;
        }

        xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
        {
            if(TBOX_TASK_NO_INIT == tbox_task_infos[task_id].state ||
            TBOX_TASK_ISFORBIDDEN == tbox_task_infos[task_id].state)
            {
                xSemaphoreGive(tbox_task_infos[task_id].mutex);
                MODULE_LOG_W(ICORE, "the task stat is invalid, state:%d", tbox_task_infos[task_id].state);
                return;
            }
            tbox_task_infos[task_id].state = TBOX_TASK_ISFORBIDDEN;
            tbox_task_infos[task_id].msg_queue.in = tbox_task_infos[task_id].msg_queue.out = 0U;             
        }
        xSemaphoreGive(tbox_task_infos[task_id].mutex);

        tbox_task_delete_task(task_id);
    }
    else
    {
        TaskHandle_t handle;
        xSemaphoreTake(tbox_task_mgr.mutex, portMAX_DELAY);
        {
            if( FALSE == tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].is_used ||
                TBOX_TASK_ISFORBIDDEN == tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].state)
            {
                xSemaphoreGive(tbox_task_mgr.mutex);
                MODULE_LOG_W(ICORE, "the task stat is invalid, state:%d", tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].state);
                return;
            }
            handle = tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].handle;
            tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].handle = NULL_PTR;
            tbox_task_mgr.runloops[(UINT32)task_id-TBOX_TASK_MAX_NUM].state = TBOX_TASK_ISFORBIDDEN;            
        }
        xSemaphoreGive(tbox_task_mgr.mutex);

        tbox_task_delete_task_by_handle(handle);
    }

    MODULE_LOG_I(ICORE, "forbid task success, task_id:%d", task_id);
}

INT32 tbox_task_expend_stack(TBOX_ID task_id)
{
    if(task_id < 0 || task_id >= TBOX_TASK_MAX_NUM)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    if(NULL_PTR == tbox_task_infos[task_id].mutex)
    {
        return (INT32)TBOX_E_NOINIT;
    }

    UINT32 stack_size;
    if(task_id < TBOX_TASK_SMALL_SIZE_NUM && task_id >= 0)
    {
       stack_size = TBOX_TASK_MEDIUM_STACK_SIZE/sizeof(StackType_t);
    }
    else if(task_id < (TBOX_TASK_SMALL_SIZE_NUM+TBOX_TASK_MEDIUM_SIZE_NUM) && 
            task_id >= TBOX_TASK_SMALL_SIZE_NUM)
    {
        stack_size = TBOX_TASK_LARGE_STACK_SIZE/sizeof(StackType_t);
    }
    else
    {
        return (INT32)TBOX_E_FAILED;
    }
    xSemaphoreTake(tbox_task_infos[task_id].mutex, portMAX_DELAY);
    {
        tbox_task_infos[task_id].stack_size = stack_size;
    }
    xSemaphoreGive(tbox_task_infos[task_id].mutex);

    tbox_reset_task(task_id);

    MODULE_LOG_I(ICORE, "expend stack success, task_id:%d, stack_size:%d", task_id, stack_size*sizeof(StackType_t));

    return (INT32)TBOX_E_OK;
}

VOID  tbox_task_check(VOID)
{
    /*1、检查是否有任务在处理耗时很长的消息或者被阻塞*/
    tbox_task_check_block();

    /*2、检查任务是否异常*/
    tbox_task_check_exception();

    /*3、检查runloop是否异常*/
    tbox_task_check_runloop_exception();
}

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    CHAR log_info[32] = "\0";
    snprintf(log_info, 32U, "stack overflow task:%s\r\n", pcTaskName);
    tbox_log_raw_output(log_info, 32U);
    /*TBOX_CORE_TASK_ABNORMAL_INFO abnormal_info;
    abnormal_info.task_id = TBOX_ID_INVALID;
    abnormal_info.msg_name = pcTaskName;
    abnormal_info.task_handle = (VOID *)xTask;
    abnormal_info.module_id = TBOX_ID_INVALID;
    tbox_supervish_report_task_abnormal(MODULE_ABNORMAL_STACK_OVERFLOW, 
                                        TRUE, &abnormal_info);*/
}
#endif

#if (configUSE_MALLOC_FAILED_HOOK > 0)
void vApplicationMallocFailedHook(void) 
{
    CHAR log_info[32] = "\0";
    snprintf(log_info, 32U, "freertos malloc failed \r\n");
    tbox_log_raw_output(log_info, 32U);
    
    /*TBOX_CORE_MEMORY_ABNORMAL_INFO abnormal_info;
    abnormal_info.addr = NULL_PTR;
    tbox_supervish_report_memory_abnormal(MODULE_ABNORMAL_MEMORY_OVERFLOW, &abnormal_info);*/
}
#endif
