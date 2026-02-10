#include "tbox_core.h"
#include "tbox_adsp_if.h"

#define TBOX_ADSP_EVENT_START   (1U << 0U)
#define TBOX_ADSP_EVENT_STOP    (1U << 1U)
#define TBOX_ADSP_EVENT_EXIT    (1U << 2U)
#define TBOX_ADSP_EVENT_INPUT   (1U << 3U)
#define TBOX_ADSP_EVENT_ALL     (TBOX_ADSP_EVENT_START | TBOX_ADSP_EVENT_STOP | TBOX_ADSP_EVENT_EXIT | TBOX_ADSP_EVENT_INPUT)

typedef struct 
{
    TBOX_ADSP_IS_MATCH_FUNC match_func;
    TBOX_ADSP_CB_FUNC cb_func;
    TBOX_ADSP_IS_EXIT_FUNC exit_func;
}TBOX_ADSP_CB_INFO;

static TBOX_ADSP_CB_INFO tbox_adsp_cb_info[TBOX_ADSP_TYPE_MAX];
static volatile UINT8 tbox_adsp_curret_type;
static TaskHandle_t tbox_adsp_task_handle = NULL_PTR;
static INT32 tbox_adsp_init(UINT8 seq);
static VOID tbox_adsp_start(VOID);
static VOID tbox_adsp_stop(VOID);
static VOID tbox_adsp_exit(VOID);
static VOID tbox_adsp_task(VOID *param);
static VOID tbox_adsp_input(VOID);
static VOID tbox_adsp_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data);
static VOID tbox_adsp_handle_update_task(const CHAR *name, TBOX_MSG_DATA *data);
TBOX_MODULE_FUN(TBOXADSP, tbox_adsp_init, tbox_adsp_stop, tbox_adsp_start, NULL_PTR, tbox_adsp_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(TBOXADSP, TBOX_TASK_PRIORITY_LOW1, LOG_LEVEL_ERROR, TBOX_TASK_LARGE_STACK_SIZE, tbox_adsp_task);
TBOX_MODULE_LOADER(TBOXADSP)
{
    /*TODO 加载其他信息*/
}

INT32 tbox_adsp_register(TBOX_ADSP_TYPE type, 
                         TBOX_ADSP_IS_MATCH_FUNC is_match_func, 
                         TBOX_ADSP_CB_FUNC cb_func, 
                         TBOX_ADSP_IS_EXIT_FUNC is_exit_func)
{
    if(type <= TBOX_ADSP_TYPE_NONE ||
       type >= TBOX_ADSP_TYPE_MAX)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    tbox_adsp_cb_info[type].match_func = is_match_func;
    tbox_adsp_cb_info[type].cb_func = cb_func;
    tbox_adsp_cb_info[type].exit_func = is_exit_func;
    return (INT32)TBOX_E_OK;
}

static INT32 tbox_adsp_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            {
                tbox_adsp_curret_type = (UINT8)TBOX_ADSP_TYPE_NONE;
                for(UINT8 i = 0U; i < TBOX_ADSP_TYPE_MAX; i++)
                {
                    tbox_adsp_cb_info[i].match_func = NULL_PTR;
                    tbox_adsp_cb_info[i].cb_func = NULL_PTR;
                    tbox_adsp_cb_info[i].exit_func = NULL_PTR;
                }

                GET_TBOX_MODULE_HANDLE(TBOXADSP, tbox_adsp_task_handle);
                if((INT32)TBOX_E_OK != drv_uart_register(tbox_adsp_input))
                {
                    MODULE_LOG_E(TBOXADSP, "register uart input failed");
                    return (INT32)TBOX_E_FAILED;
                }
            }
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            {
                TBOX_ID module_id;
                GET_TBOX_MODULE_ID(TBOXADSP, module_id);
                tbox_message_subscribe(TBOX_CORE_TASK_ABNORMAL_NOTIFY, module_id, tbox_adsp_handle_task_abnormal);
                tbox_message_subscribe(TBOX_CORE_TASK_UPDATE_NOTIFY, module_id, tbox_adsp_handle_update_task);       
            }
            break;

        default:
            break;
    }

    MODULE_LOG_D(TBOXADSP, "tbox adsp init seq:%d", seq);
    return (INT32)TBOX_E_OK;
}

static VOID tbox_adsp_start(VOID)
{
    tbox_adsp_curret_type = (UINT8)TBOX_ADSP_TYPE_NONE;
    if(NULL_PTR != tbox_adsp_task_handle)
    {
        xTaskNotify(tbox_adsp_task_handle, TBOX_ADSP_EVENT_START, eSetBits);
    }

    MODULE_LOG_D(TBOXADSP, "tbox adsp started");
}

static VOID tbox_adsp_stop(VOID)
{
    if(NULL_PTR != tbox_adsp_task_handle)
    {
        xTaskNotify(tbox_adsp_task_handle, TBOX_ADSP_EVENT_STOP, eSetBits);
    }

    MODULE_LOG_D(TBOXADSP, "tbox adsp stop");     
}

static VOID tbox_adsp_exit(VOID)
{
    tbox_adsp_curret_type = (UINT8)TBOX_ADSP_TYPE_NONE;
    if(NULL_PTR != tbox_adsp_task_handle)
    {
        xTaskNotify(tbox_adsp_task_handle, TBOX_ADSP_EVENT_EXIT, eSetBits);
    }
    
    MODULE_LOG_D(TBOXADSP, "tbox adsp exited");        
}

static VOID tbox_adsp_task(VOID *param)
{
#define TBOX_ADPS_ONCE_READ_LEN  (256U)
    UINT32 event_bits;
    INT32 read_len;
    UINT8 *read_buffer = (UINT8 *)mempool_alloc(TBOX_ADPS_ONCE_READ_LEN);
    if(NULL_PTR == read_buffer)
    {
        MODULE_LOG_E(TBOXADSP, "tbox adsp task mempool alloc failed");
        vTaskDelete(NULL);
        return;
    }

    for (;;) 
    {
        xTaskNotifyWait(0, TBOX_ADSP_EVENT_ALL, &event_bits, portMAX_DELAY); // 等待通知
        if((event_bits & TBOX_ADSP_EVENT_EXIT) == TBOX_ADSP_EVENT_EXIT)
        {
            break;
        }
        if((event_bits & TBOX_ADSP_EVENT_START) == TBOX_ADSP_EVENT_START)
        {
            /*TODO:*/
            continue;
        }
        if((event_bits & TBOX_ADSP_EVENT_STOP) == TBOX_ADSP_EVENT_STOP)
        {
            TBOX_ID module_id;
            GET_TBOX_MODULE_ID(TBOXADSP, module_id);
            tbox_module_set_state(module_id, TBOX_MODULE_STATE_STOP);
            MODULE_LOG_D(TBOXADSP, "tbox adsp stopped");      
            continue;
        }

        read_len = drv_uart_rx(read_buffer, TBOX_ADPS_ONCE_READ_LEN);
        if(read_len <= 0)
        {
            continue;
        }
        if((UINT8)TBOX_ADSP_TYPE_NONE != tbox_adsp_curret_type)
        {
            if(NULL_PTR != tbox_adsp_cb_info[tbox_adsp_curret_type].cb_func)
            {
                tbox_adsp_cb_info[tbox_adsp_curret_type].cb_func(read_buffer, (UINT32)read_len);
            }
        }
        else
        {
            for(UINT8 i = 0U; i < TBOX_ADSP_TYPE_MAX; i++)
            {
                if(NULL_PTR != tbox_adsp_cb_info[i].match_func && 
                   TRUE == tbox_adsp_cb_info[i].match_func(read_buffer, (UINT32)read_len))
                {
                    tbox_adsp_curret_type = i;
                    if(NULL_PTR != tbox_adsp_cb_info[i].cb_func)
                    {
                        tbox_adsp_cb_info[i].cb_func(read_buffer, (UINT32)read_len);
                    }
                    break;
                }
            }
        }
        if((UINT8)TBOX_ADSP_TYPE_NONE != tbox_adsp_curret_type)
        {
            if(NULL_PTR != tbox_adsp_cb_info[tbox_adsp_curret_type].exit_func)
            {
                if(TRUE == tbox_adsp_cb_info[tbox_adsp_curret_type].exit_func(read_buffer, (UINT32)read_len))
                {
                    tbox_adsp_curret_type = (UINT8)TBOX_ADSP_TYPE_NONE;
                }
            }
        }
    }

    mempool_free(read_buffer);
    tbox_adsp_task_handle = NULL_PTR;      
    vTaskDelete(NULL);
}

static VOID tbox_adsp_input(VOID)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if(NULL_PTR != tbox_adsp_task_handle)
    {
        xTaskNotifyFromISR(tbox_adsp_task_handle, TBOX_ADSP_EVENT_INPUT, eSetBits, &xHigherPriorityTaskWoken);
        //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static VOID tbox_adsp_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data)
{
    TBOX_CORE_TASK_ABNORMAL_INFO *abnormal_info;
    if(0U != strncmp(TBOX_CORE_TASK_ABNORMAL_NOTIFY, name, strlen(TBOX_CORE_TASK_ABNORMAL_NOTIFY)) ||
       NULL_PTR == data)
    {
        return;
    }

    if(data->size < sizeof(TBOX_CORE_TASK_ABNORMAL_INFO))
    {
        return;
    }
    abnormal_info = (TBOX_CORE_TASK_ABNORMAL_INFO *)data->data;
    if(NULL_PTR == abnormal_info)
    {
        return;
    }
    if(abnormal_info->task_handle == tbox_adsp_task_handle)
    {
        tbox_adsp_task_handle = NULL_PTR;
    }
}

static VOID tbox_adsp_handle_update_task(const CHAR *name, TBOX_MSG_DATA *data)
{
    TBOX_CORE_TASK_UPDATE_INFO *update_info;
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOXADSP, module_id);

    if(0U != strncmp(TBOX_CORE_TASK_UPDATE_NOTIFY, name, strlen(TBOX_CORE_TASK_UPDATE_NOTIFY)) ||
       NULL_PTR == data)
    {
        return;
    }
    if(data->size < sizeof(TBOX_CORE_TASK_UPDATE_INFO))
    {
        return;
    } 
    update_info = (TBOX_CORE_TASK_UPDATE_INFO *)data->data;
    if(NULL_PTR == update_info || 
       NULL_PTR == update_info->task_handle ||
       module_id != update_info->module_id)
    {
        return;
    }
    tbox_adsp_task_handle = update_info->task_handle;
}

VOID tbox_log_raw_output(const CHAR *str, UINT16 len)
{
    drv_uart_tx((const UINT8 *)str, len);
}

VOID tbox_log_flush(VOID)
{
    drv_uart_flush();
}