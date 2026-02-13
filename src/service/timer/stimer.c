
#include "tbox_common.h"
#include "tbox_config.h"
#include "tbox_core.h"
#include "tbox_dlist.h"
#include "api_rtos.h"
#include "drv_timer.h"
#include "drv_pin.h"
#include "stimer.h"


/*#define STIMER_PDT_INS  1
#define STIMER_UNIT_MAX 20*/
#define STIMER_SPPLEMENTARY_THREASHOLD (600U)
#define STIMER_EVENT_START   (1U << 0U)
#define STIMER_EVENT_STOP    (1U << 1U)
#define STIMER_EVENT_EXIT    (1U << 2U)
#define STIMER_EVENT_INPUT   (1U << 3U)
#define STIMER_EVENT_ALL     (STIMER_EVENT_START | STIMER_EVENT_STOP | STIMER_EVENT_EXIT | STIMER_EVENT_INPUT)

typedef struct
{
    BOOL            used;
    BOOL            start;
    UINT16          active_count;
    STIMER_TYPE     type;
    UINT32          round;
    UINT32          period;
    STIMER_CALLBACK callback;
    DLIST_NODE      link;
} STIMER_INFO;

static INT32 tbox_stimer_init(UINT8 seq);
static INT32 tbox_stimer_inner_init(VOID);
static VOID tbox_stimer_exit(VOID);
static VOID tbox_stimer_task(VOID *param);
static VOID tbox_stimer_interrupt(VOID);
static inline UBaseType_t tbox_stimer_enter_critical(VOID);
static inline VOID tbox_stimer_exit_critical(UBaseType_t x);
static VOID tbox_stimer_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data);
static VOID tbox_stimer_handle_update_task(const CHAR *name, TBOX_MSG_DATA *data);

static TaskHandle_t stimer_task_handle = NULL_PTR;
static STIMER_INFO stimer_info[TBOX_STIMER_NUMBER];
static DLIST_NODE  stimer_wheel[TBOX_STIMER_WHEEL_SLOTE_NUM];
static DLIST_NODE  stimer_timeout_list;
static UINT32      stimer_wheel_current_slot = 0;
static drv_timer_t hw_timer;
static drv_tcb_t   hw_tcb;

TBOX_MODULE_FUN(STIMER, tbox_stimer_init, NULL_PTR, NULL_PTR, NULL_PTR, tbox_stimer_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(STIMER, TBOX_TASK_PRIORITY_HIGH, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, tbox_stimer_task);
TBOX_MODULE_LOADER(STIMER)
{
    /*TODO 加载其他信息*/
}

VOID tbox_stimer_start(VOID)
{
    drv_timer_control(&hw_timer, DRV_TIMER_CMD_START);
    if(NULL_PTR != stimer_task_handle)
    {
        xTaskNotify(stimer_task_handle, STIMER_EVENT_START, eSetBits);
    }
    
    MODULE_LOG_D(STIMER, "stimer start");
}

VOID tbox_stimer_stop(VOID)
{
    /*TBOX_ID module_id;
    GET_TBOX_MODULE_ID(STIMER, module_id);*/
    drv_timer_control(&hw_timer, DRV_TIMER_CMD_STOP);
    //tbox_module_set_state(module_id, TBOX_MODULE_STATE_STOP);
    if(NULL_PTR != stimer_task_handle)
    {
        xTaskNotify(stimer_task_handle, STIMER_EVENT_STOP, eSetBits);
    }    
    MODULE_LOG_D(STIMER, "stimer stop");
}

STIMER_ID stimer_create(STIMER_TYPE type, STIMER_CALLBACK func)
{
    if((STIMER_TYPE_ONCE != type && 
       STIMER_TYPE_PERIOD != type) ||
       NULL_PTR == func)
    {
        return STIMER_ID_INVALID;
    }
    
    UINT32 index;
    STIMER_INFO *info = NULL_PTR;
    BaseType_t critical = tbox_stimer_enter_critical();
    {
        for(index = 0U; index < TBOX_STIMER_NUMBER; index++)
        {
            if(FALSE == stimer_info[index].used)
            {
                info = &stimer_info[index];
                stimer_info[index].used = TRUE;
                break;
            }
        }
    }
    tbox_stimer_exit_critical(critical);

    if(NULL_PTR == info)
    {
        return STIMER_ID_INVALID;
    }
    info->start  = FALSE;
    info->type   = type;
    info->round  = 0U;
    info->period = 0U;
    info->callback = func;
    dlist_init(&info->link);

    return (STIMER_ID)index;
}

INT32 stimer_start(STIMER_ID id, UINT32 timeout_ms)
{
    if(STIMER_ID_INVALID == id || 
      id >= TBOX_STIMER_NUMBER ||
      0U == timeout_ms)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    
    UINT32 slot_id;
    STIMER_INFO *info = &stimer_info[id];
    if(FALSE == info->used)
    {
        MODULE_LOG_E(STIMER, "stimer id %d not exist", id);
        return (INT32)TBOX_E_NOCREATE;
    }

    BaseType_t critical = tbox_stimer_enter_critical();
    {
        if(TRUE == info->start)
        {
            dlist_del_entry(&info->link);
        }
        info->start  = TRUE;
        info->active_count = 0U;
        info->period = timeout_ms;       
        info->round  = timeout_ms/TBOX_STIMER_WHEEL_SLOTE_NUM;
        slot_id = (stimer_wheel_current_slot + (timeout_ms % TBOX_STIMER_WHEEL_SLOTE_NUM)) % TBOX_STIMER_WHEEL_SLOTE_NUM;
        dlist_add_tail(&info->link, &stimer_wheel[slot_id]);
    }
    tbox_stimer_exit_critical(critical);

    return (INT32)TBOX_E_OK;
}

INT32 stimer_stop(STIMER_ID id)
{
    if(STIMER_ID_INVALID == id || 
      id >= TBOX_STIMER_NUMBER)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    STIMER_INFO *info = &stimer_info[id];
    if(FALSE == info->used)
    {
        MODULE_LOG_E(STIMER, "stimer id %d not exist", id);
        return (INT32)TBOX_E_NOCREATE;
    }

    BaseType_t critical = tbox_stimer_enter_critical();
    {
        if(TRUE == info->start)
        {
            info->start  = FALSE;
            info->period = 0U;       
            info->round  = 0U;
            dlist_del_entry(&info->link);
            dlist_init(&info->link);
        }
    }
    tbox_stimer_exit_critical(critical);

    return (INT32)TBOX_E_OK;
}

INT32 stimer_trigger(STIMER_ID id)
{
    if(STIMER_ID_INVALID == id || 
      id >= TBOX_STIMER_NUMBER)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    STIMER_INFO *info = &stimer_info[id];
    if(FALSE == info->used)
    {
        MODULE_LOG_E(STIMER, "stimer id %d not exist", id);
        return (INT32)TBOX_E_NOCREATE;
    }

    BaseType_t critical = tbox_stimer_enter_critical();
    {   
        if(!dlist_has_node(&stimer_timeout_list, &info->link))
        {
            info->round  = 0U;
            dlist_del_entry(&info->link);
            dlist_add_tail(&info->link, &stimer_timeout_list);
        }
    }
    tbox_stimer_exit_critical(critical);

    return (INT32)TBOX_E_OK;    
}

INT32 stimer_clear(STIMER_ID id)
{
    if(STIMER_ID_INVALID == id || 
      id >= TBOX_STIMER_NUMBER)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    STIMER_INFO *info = &stimer_info[id];
    if(FALSE == info->used)
    {
        MODULE_LOG_E(STIMER, "stimer id %d not exist", id);
        return (INT32)TBOX_E_NOCREATE;
    }
    
    UINT32 slot_id;
    BaseType_t critical = tbox_stimer_enter_critical();
    {
        if(dlist_has_node(&stimer_timeout_list, &info->link))
        {
            dlist_del_entry(&info->link);
            info->round  = info->period/TBOX_STIMER_WHEEL_SLOTE_NUM;
            slot_id = (stimer_wheel_current_slot + (info->period % TBOX_STIMER_WHEEL_SLOTE_NUM)) % TBOX_STIMER_WHEEL_SLOTE_NUM;
            dlist_add_tail(&info->link, &stimer_wheel[slot_id]);             
        }              
    }
    tbox_stimer_exit_critical(critical);
    
    return (INT32)TBOX_E_OK;       
}

BOOL stimer_is_started(STIMER_ID id)
{
    BOOL is_started = FALSE;

    if(STIMER_ID_INVALID == id || 
      id >= TBOX_STIMER_NUMBER)
    {
        return FALSE;
    }
    
    STIMER_INFO *info = &stimer_info[id];
    if(FALSE == info->used)
    {
        MODULE_LOG_E(STIMER, "stimer id %d not exist", id);
        return FALSE;
    }

    BaseType_t critical = tbox_stimer_enter_critical();
    {
        is_started = info->start;
    }
    tbox_stimer_exit_critical(critical);

    return is_started;
}

static INT32 tbox_stimer_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            {
                GET_TBOX_MODULE_HANDLE(STIMER, stimer_task_handle);
                if((INT32)TBOX_E_OK != tbox_stimer_inner_init())
                {
                    MODULE_LOG_E(STIMER, "stimer init failed");
                    return (INT32)TBOX_E_FAILED;
                }
            }
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
           {
                TBOX_ID module_id;
                GET_TBOX_MODULE_ID(STIMER, module_id);
                tbox_module_set_state(module_id, TBOX_MODULE_STATE_START);
                tbox_message_subscribe(TBOX_CORE_TASK_ABNORMAL_NOTIFY, module_id, tbox_stimer_handle_task_abnormal);
                tbox_message_subscribe(TBOX_CORE_TASK_UPDATE_NOTIFY, module_id, tbox_stimer_handle_update_task);                              
           }
           break;

        default:
            break;
    }

    MODULE_LOG_D(STIMER, "stimer init seq:%d", seq);
    return (INT32)TBOX_E_OK;    
}

static INT32 tbox_stimer_inner_init(VOID)
{
    UINT32 i;
    for (i = 0U; i < TBOX_STIMER_NUMBER; i++) 
    {
        stimer_info[i].used   = FALSE;
        stimer_info[i].start  = FALSE;
        stimer_info[i].active_count = 0U;
        stimer_info[i].type   = STIMER_TYPE_PERIOD;
        stimer_info[i].round  = 0U;
        stimer_info[i].period = 0U;
        stimer_info[i].callback = NULL;
        dlist_init(&stimer_wheel[i]);
    }
    for(i = 0U; i < TBOX_STIMER_WHEEL_SLOTE_NUM; i++)
    {
        dlist_init(&stimer_wheel[i]);
    }
    dlist_init(&stimer_timeout_list);
    stimer_wheel_current_slot = 0U;

    drv_timer_control(&hw_timer, DRV_TIMER_CMD_SET_CFG, DRV_TIMER_INS_TIMER0, 1, DRV_TIMER_MODE_INTERRUPT | DRV_TIMER_MODE_STARTUP);
    drv_timer_control(&hw_timer, DRV_TIMER_CMD_ATTACH, tbox_stimer_interrupt, &hw_tcb);
    drv_timer_init(&hw_timer);

    return (INT32)TBOX_E_OK;   
}

static VOID tbox_stimer_exit(VOID)
{
    drv_timer_deinit(&hw_timer);
    if(NULL_PTR != stimer_task_handle)
    {
        xTaskNotify(stimer_task_handle, STIMER_EVENT_EXIT, eSetBits);
    }        
    stimer_task_handle = NULL_PTR;
    MODULE_LOG_D(STIMER, "stimer exit");    
}

static VOID tbox_stimer_task(VOID *param)
{
    UNUSED(param);
    
    DLIST_NODE *node;
    STIMER_INFO *info;
    UINT32 slot_id;
    UINT32 event_bits;
    UINT32 period;
    BaseType_t critical;
    STIMER_CALLBACK callback;

    for(;;)
    {
        xTaskNotifyWait(0, STIMER_EVENT_ALL, &event_bits, portMAX_DELAY); // 等待通知
        if((event_bits & STIMER_EVENT_EXIT) == STIMER_EVENT_EXIT)
        {
            break;
        }
        if((event_bits & STIMER_EVENT_START) == STIMER_EVENT_START)
        {
            /*TODO:*/
            continue;
        }
        if((event_bits & STIMER_EVENT_STOP) == STIMER_EVENT_STOP)
        {
            TBOX_ID module_id;
            GET_TBOX_MODULE_ID(STIMER, module_id);
            tbox_module_set_state(module_id, TBOX_MODULE_STATE_STOP);
            MODULE_LOG_D(STIMER, "stimer task stopped");      
            continue;
        }
        for (;;) 
        {
            critical = tbox_stimer_enter_critical();
            {
                node = dlist_pop_first_node(&stimer_timeout_list);
                if(NULL_PTR == node)
                {
                    tbox_stimer_exit_critical(critical);
                    break;      
                }
                info = TBOX_CONTAINER(node, STIMER_INFO, link);
                if(FALSE == info->start)
                {
                    tbox_stimer_exit_critical(critical);
                    continue;                
                }
                callback = info->callback;
                if(STIMER_TYPE_PERIOD == info->type)
                {
                    /*因为中断周期为1ms，超时时只会将定时器添加到timer超时列表，不会实时处理，所以这里需要将定时器的超时时间减1ms*/
                    period = info->period;
                    if(period > 1U)
                    {
                        period = period - 1U;
                    }
                    if(period > 1U && info->active_count >= STIMER_SPPLEMENTARY_THREASHOLD)
                    {
                        info->active_count = 0U;
                        period = period - 1U;
                    }
                    info->round  = period/TBOX_STIMER_WHEEL_SLOTE_NUM;
                    slot_id = (stimer_wheel_current_slot + (period % TBOX_STIMER_WHEEL_SLOTE_NUM)) % TBOX_STIMER_WHEEL_SLOTE_NUM;
                    dlist_add_tail(&info->link, &stimer_wheel[slot_id]);
                }
                else
                {
                    info->start = FALSE;
                    info->round  = 0U;
                    info->period = 0U;
                }                
            }
            tbox_stimer_exit_critical(critical);
            
            if(NULL_PTR != callback)
            {
                callback();
            }
        }
    }

    vTaskDelete(NULL_PTR);    
}

static VOID tbox_stimer_interrupt(VOID)
{
    DLIST_NODE *node;
    DLIST_NODE *wheel_list;
    STIMER_INFO *info;
    UINT32 count;

    BaseType_t critical = tbox_stimer_enter_critical();
    {
        wheel_list = &stimer_wheel[stimer_wheel_current_slot];
        count = dlist_count(wheel_list);
        for(UINT32 i = 0U; i < count; i++)
        {
            node = dlist_pop_first_node(wheel_list);
            if(NULL_PTR != node)
            {
                info = TBOX_CONTAINER(node, STIMER_INFO, link);
                if(FALSE == info->start)
                {
                    continue;
                }
                if(info->round > 0U)
                {
                    info->round--;
                    dlist_add_tail(node, wheel_list);
                }
                else
                {
                    info->active_count++;
                    dlist_add_tail(&info->link, &stimer_timeout_list);
                }
            }
        }        
        stimer_wheel_current_slot = (stimer_wheel_current_slot + 1U) % TBOX_STIMER_WHEEL_SLOTE_NUM;
    }
    tbox_stimer_exit_critical(critical);

    if(!dlist_empty(&stimer_timeout_list) && 
       NULL_PTR != stimer_task_handle)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyFromISR(stimer_task_handle, STIMER_EVENT_INPUT, eSetBits, &xHigherPriorityTaskWoken);
        //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static inline UBaseType_t tbox_stimer_enter_critical(VOID)
{
    UBaseType_t x = 0;

    if (xPortIsInsideInterrupt()) {
        x = taskENTER_CRITICAL_FROM_ISR();
    } else {
        taskENTER_CRITICAL();
    }

    return x;
}

static inline VOID tbox_stimer_exit_critical(UBaseType_t x)
{
    if (xPortIsInsideInterrupt()) {
        taskEXIT_CRITICAL_FROM_ISR(x);
    } else {
        taskEXIT_CRITICAL();
    }
}

static VOID tbox_stimer_handle_task_abnormal(const CHAR *name, TBOX_MSG_DATA *data)
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
    if(abnormal_info->task_handle == stimer_task_handle)
    {
        stimer_task_handle = NULL_PTR;
    }
}

static VOID tbox_stimer_handle_update_task(const CHAR *name, TBOX_MSG_DATA *data)
{
    TBOX_CORE_TASK_UPDATE_INFO *update_info;
    TBOX_ID module_id;
    GET_TBOX_MODULE_ID(TBOXSHELL, module_id);

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
    stimer_task_handle = update_info->task_handle;
}