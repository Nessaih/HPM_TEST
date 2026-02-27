#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "tbox_config.h"
#include "tbox_common.h"
#include "api_rtos.h"
#include "tbox_dlist.h"
#include "tbox_memory.h"
#include "tbox_module.h"
#include "cqueue.h"
#include "tbox_log_inner.h"

#define TBOX_LOG_MAGIC_NO             (0x5F4C)
#define TBOX_LOG_SEQ_NOINIT           (0U)
#define TBOX_LOG_SEQ_STOP             (1U)
#define TBOX_LOG_SEQ_START            (2U)
#define TBOX_LOG_TEMP_BUFF_COUNT      (3U)
#define TBOX_LOG_EVENT_START_BIT      (1 << 0)
#define TBOX_LOG_EVENT_STOP_BIT       (1 << 1)
#define TBOX_LOG_EVENT_OUTPUT_BIT     (1 << 2)
#define TBOX_LOG_EVENT_EXIT_BIT       (1 << 3)
#define TBOX_LOG_EVENT_ALL_BITS       (TBOX_LOG_EVENT_START_BIT | TBOX_LOG_EVENT_STOP_BIT | TBOX_LOG_EVENT_OUTPUT_BIT | TBOX_LOG_EVENT_EXIT_BIT)

typedef struct tag_tbox_log_header
{
    UINT16 magic;
    UINT16 size;
}TBOX_LOG_HEADER;

typedef struct tag_tbox_log_mgr
{
    cqueue_t log_queue;
    CHAR *temp_buffer;
    UINT8 temp_pos;
    SemaphoreHandle_t mutex;
    TaskHandle_t task_handle;
}TBOX_LOG_MGR;

static volatile UINT8 tbox_log_init_seq = TBOX_LOG_SEQ_NOINIT;
static TBOX_LOG_MGR tbox_log_mgr;
static const CHAR *level_str[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL", "NONE"};

static inline UINT32 tbox_log_copydata_to_linebuffer(UINT16 cur_len, CHAR *dst, const CHAR *src)
{
   const CHAR *src_old = src;
   while (*src != 0) 
   {
        if (cur_len++ < TBOX_LOG_LINEBUFF_SIZE)
        {
            *dst++ = *src++;
        } 
        else 
        {
            break;
        }
   }

   return (UINT32)(src - src_old);
}

static VOID tbox_log_task(VOID *param)
{
    UNUSED(param);
    UINT32 wait_tick = portMAX_DELAY;
    UINT32 event_bits = 0;
    TBOX_LOG_HEADER header;
    CHAR *temp_ptr = tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, TBOX_LOG_LINEBUFF_SIZE);
    if(NULL_PTR == temp_ptr)
    {
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        tbox_log_mgr.task_handle = NULL_PTR;
        xSemaphoreGive(tbox_log_mgr.mutex);
        vTaskDelete(NULL_PTR);
        return;        
    }

    for(;;)
    {
        event_bits = 0;
        xTaskNotifyWait(0, TBOX_LOG_EVENT_ALL_BITS, &event_bits, wait_tick);

        if((event_bits & TBOX_LOG_EVENT_EXIT_BIT) == TBOX_LOG_EVENT_EXIT_BIT)
        {
            break;
        }
        
        if((event_bits & TBOX_LOG_EVENT_START_BIT) == TBOX_LOG_EVENT_START_BIT)
        {
            wait_tick = pdMS_TO_TICKS(500U);
        }

        if((event_bits & TBOX_LOG_EVENT_STOP_BIT) == TBOX_LOG_EVENT_STOP_BIT)
        {
            wait_tick = portMAX_DELAY;
            continue;
        }

        for(;;)
        {
            header.magic = 0U;
            header.size = 0U;
            xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
            {
                if(cqueue_datalen(&tbox_log_mgr.log_queue) < sizeof(TBOX_LOG_HEADER))
                {
                    xSemaphoreGive(tbox_log_mgr.mutex);
                    break;
                }
                cqueue_get(&tbox_log_mgr.log_queue, (uint8_t *)&header, sizeof(TBOX_LOG_HEADER));
                if(TBOX_LOG_MAGIC_NO != header.magic || header.size > TBOX_LOG_LINEBUFF_SIZE)
                {
                    tbox_log_mgr.log_queue.in = tbox_log_mgr.log_queue.out = 0U;
                    xSemaphoreGive(tbox_log_mgr.mutex);
                    break;                    
                }
                if(cqueue_datalen(&tbox_log_mgr.log_queue) < header.size)
                {
                    tbox_log_mgr.log_queue.in = tbox_log_mgr.log_queue.out = 0U;
                    xSemaphoreGive(tbox_log_mgr.mutex);
                    break;
                }
                cqueue_get(&tbox_log_mgr.log_queue, (uint8_t *)temp_ptr, header.size);
                tbox_log_mgr.temp_pos++;
            }
            xSemaphoreGive(tbox_log_mgr.mutex);

            tbox_log_raw_output(temp_ptr, header.size);

            xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
            {
                tbox_log_mgr.temp_pos--;
            }
            xSemaphoreGive(tbox_log_mgr.mutex);
        }
    }

    tbox_memory_free(TBOX_MEMORY_TYPE_CORE, temp_ptr);
    xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
    tbox_log_mgr.task_handle = NULL_PTR;
    xSemaphoreGive(tbox_log_mgr.mutex);
    vTaskDelete(NULL_PTR);
}

INT32 tbox_log_init(VOID)
{
    if(TBOX_LOG_SEQ_NOINIT != tbox_log_init_seq)
    {
        return (INT32)TBOX_E_HASINIT;
    }

    tbox_log_init_seq = TBOX_LOG_SEQ_STOP;
    tbox_log_mgr.mutex = NULL_PTR;
    tbox_log_mgr.task_handle = NULL_PTR;
   
    UINT32 size = TBOX_LOG_ITEM_MAX_NUM*TBOX_LOG_LINEBUFF_SIZE;
    UINT8 *buffer = tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, size);
    if(NULL_PTR == buffer)
    {
        return (INT32)TBOX_E_FAILED_ALLOC;
    }
    cqueue_init(&tbox_log_mgr.log_queue, buffer, size);

    size = TBOX_LOG_TEMP_BUFF_COUNT*TBOX_LOG_LINEBUFF_SIZE;
    tbox_log_mgr.temp_buffer = tbox_memory_alloc(TBOX_MEMORY_TYPE_CORE, size);
    if(NULL_PTR == tbox_log_mgr.temp_buffer)
    {
        tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_log_mgr.log_queue.buffer);
        return (INT32)TBOX_E_FAILED_ALLOC;
    }
    tbox_log_mgr.temp_pos = 0U;

    tbox_log_mgr.mutex = xSemaphoreCreateMutex();
    if(NULL_PTR == tbox_log_mgr.mutex)
    {
        tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_log_mgr.log_queue.buffer);
        tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_log_mgr.temp_buffer);
        return (INT32)TBOX_E_FAILED_CREATE;
    }
    
    INT32 ret = xTaskCreate(tbox_log_task, 
                            "TBOXLOG",
                            TBOX_TASK_SMALL_STACK_SIZE/sizeof(StackType_t), 
                            NULL_PTR, 
                            TBOX_TASK_PRIORITY_LOW, 
                            &tbox_log_mgr.task_handle);
    if(pdPASS != ret)
    {
        tbox_log_mgr.mutex = NULL_PTR;
        tbox_log_mgr.task_handle = NULL_PTR;        
        vSemaphoreDelete(tbox_log_mgr.mutex);
        tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_log_mgr.log_queue.buffer);
        tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_log_mgr.temp_buffer);
        return (INT32)TBOX_E_FAILED_CREATE;
    }

    return (INT32)TBOX_E_OK;
}

VOID tbox_log_deinit(VOID)
{
    if(TBOX_LOG_SEQ_NOINIT == tbox_log_init_seq)
    {
        return;
    }

    /*1、先发送退出事件，等待其退出*/
    TaskHandle_t task_handle;
    if(NULL_PTR == tbox_log_mgr.mutex)
    {
        task_handle = tbox_log_mgr.task_handle;
    }
    else
    {
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        task_handle = tbox_log_mgr.task_handle;
        xSemaphoreGive(tbox_log_mgr.mutex);
    }
    if(NULL_PTR != task_handle)
    {
        xTaskNotify(task_handle, TBOX_LOG_EVENT_EXIT_BIT, eSetBits);

        eTaskState state;
        UINT8 i;
        for(i = 0U; i < 4U; i++)
        {
            state = eTaskGetState(task_handle);
            if(eDeleted == state || eInvalid == state)
            {
                xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
                tbox_log_mgr.task_handle = NULL_PTR;
                xSemaphoreGive(tbox_log_mgr.mutex);
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(100U));
            
            xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
            task_handle = tbox_log_mgr.task_handle;
            xSemaphoreGive(tbox_log_mgr.mutex);
            if(NULL_PTR == task_handle)
            {
                break;
            }
        }
        if(i >= 4U)
        {
            xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
            task_handle = tbox_log_mgr.task_handle;
            tbox_log_mgr.task_handle = NULL_PTR;
            xSemaphoreGive(tbox_log_mgr.mutex);
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
    
    /*2、释放其他资源*/
    if(NULL_PTR != tbox_log_mgr.mutex)
    {
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        {
            tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_log_mgr.log_queue.buffer);
            cqueue_init(&tbox_log_mgr.log_queue, NULL_PTR, 0U);
            if(NULL_PTR != tbox_log_mgr.temp_buffer)
            {
                tbox_memory_free(TBOX_MEMORY_TYPE_CORE, tbox_log_mgr.temp_buffer);
                tbox_log_mgr.temp_buffer = NULL_PTR;
                tbox_log_mgr.temp_pos = 0U;
            }
        }
        xSemaphoreGive(tbox_log_mgr.mutex);
        vSemaphoreDelete(tbox_log_mgr.mutex);
        tbox_log_mgr.mutex = NULL_PTR;
    }

    tbox_log_init_seq = TBOX_LOG_SEQ_NOINIT;
}

INT32 tbox_log_start(VOID)
{
    if(TBOX_LOG_SEQ_NOINIT == tbox_log_init_seq || 
       TBOX_LOG_SEQ_START == tbox_log_init_seq)
    {
        return (INT32)TBOX_E_HASSTART;
    }

    TaskHandle_t handle;
    xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
    if(NULL_PTR == tbox_log_mgr.task_handle)
    {
        xSemaphoreGive(tbox_log_mgr.mutex);
        return (INT32)TBOX_E_NOINIT;
    }
    handle = tbox_log_mgr.task_handle;
    xSemaphoreGive(tbox_log_mgr.mutex);
    
    eTaskState state = eTaskGetState(handle);
    if(eDeleted == state || eInvalid == state)
    {
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        tbox_log_mgr.task_handle = NULL_PTR;
        xSemaphoreGive(tbox_log_mgr.mutex);
        return (INT32)TBOX_E_NOEXISTS;
    }
    else if(state == eSuspended)
    {
        vTaskResume(handle);
    } 
    else
    {
        /*TODO*/
    }

    xTaskNotify(handle, TBOX_LOG_EVENT_START_BIT, eSetBits);

    tbox_log_init_seq = TBOX_LOG_SEQ_START;

    return (INT32)TBOX_E_OK;
}

VOID tbox_log_stop(VOID)
{
    if(TBOX_LOG_SEQ_START != tbox_log_init_seq)
    {
        return;
    }

    TaskHandle_t handle;
    xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
    if(NULL_PTR == tbox_log_mgr.task_handle)
    {
        xSemaphoreGive(tbox_log_mgr.mutex);
        return;
    }
    handle = tbox_log_mgr.task_handle;
    xSemaphoreGive(tbox_log_mgr.mutex);

    eTaskState state = eTaskGetState(handle);
    if(eDeleted == state || eInvalid == state)
    {
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        tbox_log_mgr.task_handle = NULL_PTR;
        xSemaphoreGive(tbox_log_mgr.mutex);
        return;
    }

    xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
    {
        cqueue_discard(&tbox_log_mgr.log_queue, TBOX_LOG_ITEM_MAX_NUM*TBOX_LOG_LINEBUFF_SIZE);
    }
    xSemaphoreGive(tbox_log_mgr.mutex);
    
    xTaskNotify(handle, TBOX_LOG_EVENT_STOP_BIT, eSetBits);

    tbox_log_init_seq = TBOX_LOG_SEQ_STOP; 
}

VOID tbox_log_print(const CHAR *format, ...)
{
    if(TBOX_LOG_SEQ_START != tbox_log_init_seq)
    {
        return;
    }

    va_list args;
    BOOL direct_output = FALSE;
    CHAR *temp_ptr = NULL_PTR;
    UINT8 retry_count = 0;
    TaskHandle_t handle;
    xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
    {
        if(tbox_log_mgr.temp_pos >= TBOX_LOG_TEMP_BUFF_COUNT)
        {
            xSemaphoreGive(tbox_log_mgr.mutex);
            return;
        }
        handle = tbox_log_mgr.task_handle;    
        if(cqueue_surplus(&tbox_log_mgr.log_queue) < (sizeof(TBOX_LOG_HEADER)+TBOX_LOG_LINEBUFF_SIZE))
        {
            xSemaphoreGive(tbox_log_mgr.mutex);
            xTaskNotify(handle, TBOX_LOG_EVENT_OUTPUT_BIT, eSetBits);
            while(retry_count++ < 3U)
            {
                if(cqueue_surplus(&tbox_log_mgr.log_queue) >= (sizeof(TBOX_LOG_HEADER)+TBOX_LOG_LINEBUFF_SIZE))
                {
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(20U));
            }

            xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
            if(cqueue_surplus(&tbox_log_mgr.log_queue) < (sizeof(TBOX_LOG_HEADER)+TBOX_LOG_LINEBUFF_SIZE))
            {
                direct_output = TRUE;
            }
        }
        temp_ptr = tbox_log_mgr.temp_buffer + tbox_log_mgr.temp_pos*TBOX_LOG_LINEBUFF_SIZE;
        tbox_log_mgr.temp_pos++;
    }
    xSemaphoreGive(tbox_log_mgr.mutex);

    memset(temp_ptr, 0, TBOX_LOG_LINEBUFF_SIZE);
    va_start(args, format);
    vsnprintf(temp_ptr, TBOX_LOG_LINEBUFF_SIZE-1, format, args);
    va_end(args);

    TBOX_LOG_HEADER header;
    header.magic = TBOX_LOG_MAGIC_NO;
    header.size = strlen(temp_ptr);
    if(TRUE == direct_output)
    {
         tbox_log_raw_output(temp_ptr, header.size);
         xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
         tbox_log_mgr.temp_pos--;
         xSemaphoreGive(tbox_log_mgr.mutex);     
    }
    else
    {
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        {
            cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)&header, sizeof(TBOX_LOG_HEADER));
            cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)temp_ptr, header.size);
            tbox_log_mgr.temp_pos--;
        }
        xSemaphoreGive(tbox_log_mgr.mutex);
        if(NULL_PTR != handle)
        {
            xTaskNotify(handle, TBOX_LOG_EVENT_OUTPUT_BIT, eSetBits);
        }    
    }
}

VOID tbox_log_output(TBOX_ID modlue_id, TBOX_LOG_LEVEL level, const CHAR *fun, const INT32 line, const CHAR *fmt, ...)
{  
    if(TBOX_LOG_SEQ_START != tbox_log_init_seq)
    {
        return;
    }

    va_list args;
    BOOL direct_output = FALSE;
    UINT8 retry_count = 0;

    TBOX_MODULE_INFO config = {.name = NULL_PTR, .log_level = LOG_LEVEL_NONE};
    tbox_module_get_config(modlue_id, &config);
    if(NULL_PTR == config.name ||
       level >= LOG_LEVEL_NONE ||
       level < config.log_level)
    {
        return;
    }

    CHAR *temp_ptr = NULL_PTR;
    TaskHandle_t handle;
    xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
    {
        if(tbox_log_mgr.temp_pos >= TBOX_LOG_TEMP_BUFF_COUNT)
        {
            xSemaphoreGive(tbox_log_mgr.mutex);
            return;
        }
        handle = tbox_log_mgr.task_handle;
        if(cqueue_surplus(&tbox_log_mgr.log_queue) < (sizeof(TBOX_LOG_HEADER)+TBOX_LOG_LINEBUFF_SIZE))
        {
            xSemaphoreGive(tbox_log_mgr.mutex);
            xTaskNotify(handle, TBOX_LOG_EVENT_OUTPUT_BIT, eSetBits);
            while(retry_count++ < 3U)
            {
                if(cqueue_surplus(&tbox_log_mgr.log_queue) >= (sizeof(TBOX_LOG_HEADER)+TBOX_LOG_LINEBUFF_SIZE))
                {
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(20U));
            }

            xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
            if(cqueue_surplus(&tbox_log_mgr.log_queue) < (sizeof(TBOX_LOG_HEADER)+TBOX_LOG_LINEBUFF_SIZE))
            {
                direct_output = TRUE;
            }
        }
        temp_ptr = tbox_log_mgr.temp_buffer + tbox_log_mgr.temp_pos*TBOX_LOG_LINEBUFF_SIZE;
        tbox_log_mgr.temp_pos++;
    }
    xSemaphoreGive(tbox_log_mgr.mutex);

    TBOX_LOG_HEADER header;
    header.magic = TBOX_LOG_MAGIC_NO;
    memset(temp_ptr, 0, TBOX_LOG_LINEBUFF_SIZE);
    snprintf(temp_ptr, TBOX_LOG_LINEBUFF_SIZE-1, "[%s %s:%d][%s]", config.name, fun, line, level_str[(UINT8)level]);
    header.size = strlen(temp_ptr);
    if(header.size < TBOX_LOG_LINEBUFF_SIZE-1)
    {
        va_start(args, fmt);
        vsnprintf(temp_ptr+header.size, TBOX_LOG_LINEBUFF_SIZE-1-header.size, fmt, args);
        va_end(args);
    }
    header.size = strlen(temp_ptr);
    if(header.size <= TBOX_LOG_LINEBUFF_SIZE-3)
    {
        temp_ptr[header.size++] = '\r';
        temp_ptr[header.size++] = '\n';
        temp_ptr[header.size++] = '\0';        
    }

    if(TRUE == direct_output)
    {
        tbox_log_raw_output(temp_ptr, header.size);
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        tbox_log_mgr.temp_pos--;
        xSemaphoreGive(tbox_log_mgr.mutex);          
    }
    else
    {
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        {
            cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)&header, sizeof(TBOX_LOG_HEADER));
            cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)temp_ptr, header.size);
            tbox_log_mgr.temp_pos--;
        }
        xSemaphoreGive(tbox_log_mgr.mutex);

        if(NULL_PTR != handle)
        {
            xTaskNotify(handle, TBOX_LOG_EVENT_OUTPUT_BIT, eSetBits);
        }
    }
}

VOID tbox_log_dump(TBOX_ID modlue_id, const CHAR *tag, const UINT8 *data, const UINT16 len)
{
    if(TBOX_LOG_SEQ_START != tbox_log_init_seq)
    {
        return;
    }

    TBOX_MODULE_INFO config = {.name = NULL_PTR, .log_level = LOG_LEVEL_NONE};
    tbox_module_get_config(modlue_id, &config);
    if(NULL_PTR == config.name ||
       config.log_level >= LOG_LEVEL_WARN)
    {
        return;
    }

    CHAR *temp_ptr = NULL_PTR;
    CHAR *line_buff = NULL_PTR;
    TaskHandle_t handle;
    xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
    {
        if((tbox_log_mgr.temp_pos+1U) >= TBOX_LOG_TEMP_BUFF_COUNT)
        {
            xSemaphoreGive(tbox_log_mgr.mutex);
            return;
        }
        temp_ptr = tbox_log_mgr.temp_buffer + tbox_log_mgr.temp_pos*TBOX_LOG_LINEBUFF_SIZE;
        tbox_log_mgr.temp_pos++;
        line_buff = tbox_log_mgr.temp_buffer + tbox_log_mgr.temp_pos*TBOX_LOG_LINEBUFF_SIZE;
        tbox_log_mgr.temp_pos++;
        handle = tbox_log_mgr.task_handle;
    }
    xSemaphoreGive(tbox_log_mgr.mutex);
    
    TBOX_LOG_HEADER header;
    header.magic = TBOX_LOG_MAGIC_NO;
    memset(temp_ptr, 0, TBOX_LOG_LINEBUFF_SIZE);
    memset(line_buff, 0, TBOX_LOG_LINEBUFF_SIZE);

    UINT16 frame_num, remainder;
    CHAR *ibuf = (CHAR *)line_buff;
    UINT8 *pos = (UINT8 *)data;
    snprintf(temp_ptr, TBOX_LOG_LINEBUFF_SIZE-1U,
             "\r\n[%s]dump info:%s, len %u\r\n", config.name, tag, len);
    header.size = strlen(temp_ptr);

    frame_num = (len/16) * 16;
    remainder = (len%16);
    UINT16 i, j;
    for (i = 0; i < frame_num; i += 16U)
    {
        ibuf = (CHAR *)line_buff;
        snprintf(ibuf, 13U, "0x%08x  ", (INT32)(data+i));
        ibuf += 12U;
        for (j = 0; j < 16U; j++)
        {
            snprintf(ibuf, 4U, "%02x ", pos[j]);
            ibuf += 3U;
        }
        snprintf(ibuf, 3U, "\r\n");
		ibuf += 2U;
		pos += 16U;
		if((header.size + (ibuf-line_buff)) >= (TBOX_LOG_LINEBUFF_SIZE-1U))
		{
            xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
            {
                if(cqueue_surplus(&tbox_log_mgr.log_queue) >= (sizeof(TBOX_LOG_HEADER)+TBOX_LOG_LINEBUFF_SIZE))
                {
                    cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)&header, sizeof(TBOX_LOG_HEADER));
                    cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)temp_ptr, header.size);                    
                }
                header.size = 0U;
            }
            xSemaphoreGive(tbox_log_mgr.mutex);
            if(NULL_PTR != handle)
            {
                xTaskNotify(handle, TBOX_LOG_EVENT_OUTPUT_BIT, eSetBits);
            }
		}
		header.size += tbox_log_copydata_to_linebuffer(header.size, temp_ptr+header.size, line_buff);
    }
	if(0U != remainder)
	{
	    ibuf =  line_buff;
	    snprintf(ibuf, 13U, "0x%08x  ", (INT32)data + ((len >> 4) << 4));
	    ibuf += 12U;
	    for (i = 0U; i < remainder; i++)
	    {
	        snprintf(ibuf, 4U, "%02x ", pos[i]);
	        ibuf += 3U;
	    }
	    for (i = remainder; i < 16U; i++)
	    {
	        snprintf(ibuf, 4U, "   ");
	        ibuf += 3U;
	    }
	    snprintf(ibuf, 3U,"\r\n");
        ibuf += 2U;
		if((header.size + (ibuf-line_buff)) >= (TBOX_LOG_LINEBUFF_SIZE-1U))
		{
            xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
            {
                if(cqueue_surplus(&tbox_log_mgr.log_queue) >= (sizeof(TBOX_LOG_HEADER)+TBOX_LOG_LINEBUFF_SIZE))
                {
                    cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)&header, sizeof(TBOX_LOG_HEADER));
                    cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)temp_ptr, header.size);                    
                }
                header.size = 0U;
            }
            xSemaphoreGive(tbox_log_mgr.mutex);
            if(NULL_PTR != handle)
            {
                xTaskNotify(handle, TBOX_LOG_EVENT_OUTPUT_BIT, eSetBits);
            }
		}
		header.size += tbox_log_copydata_to_linebuffer(header.size, temp_ptr+header.size, line_buff);
	}
    if(header.size > 0U)
    {
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        {
            if(cqueue_surplus(&tbox_log_mgr.log_queue) >= (sizeof(TBOX_LOG_HEADER)+TBOX_LOG_LINEBUFF_SIZE))
            {
                cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)&header, sizeof(TBOX_LOG_HEADER));
                cqueue_put(&tbox_log_mgr.log_queue, (UINT8 *)temp_ptr, header.size);                    
            }
            tbox_log_mgr.temp_pos -= 2U;
            header.size = 0U;
        }
        xSemaphoreGive(tbox_log_mgr.mutex);
        if(NULL_PTR != handle)
        {
            xTaskNotify(handle, TBOX_LOG_EVENT_OUTPUT_BIT, eSetBits);
        }
    }
    else
    {
        xSemaphoreTake(tbox_log_mgr.mutex, portMAX_DELAY);
        {
            tbox_log_mgr.temp_pos -= 2U;
        }
        xSemaphoreGive(tbox_log_mgr.mutex);        
    }   
}

__weak VOID tbox_log_get_time(TBOX_LOG_TIME *time)
{
    UNUSED(time);
    /*TODO get current time */
}

__weak VOID tbox_log_raw_output(const CHAR *str, UINT16 len)
{
    UNUSED(str);
    UNUSED(len);
    /*TODO output log to console、file or uart*/
}

__weak VOID tbox_log_flush(VOID)
{
    /*TODO flush all log to file、uart or console */
}