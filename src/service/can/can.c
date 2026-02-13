#include <string.h>
#include "tbox_common.h"
#include "tbox_core.h"
#include "can_if.h"
#include "can_list.h"
#include "can_buserr.h"
#include "can_busoff.h"
#include "can_mgr.h"
#include "can_shell.h"
#include "tbox_cfg_if.h"
#include "drv_can.h"

static INT32 can_init(UINT8 seq);
static VOID  can_stop(VOID);
static VOID  can_start(VOID);
static VOID  can_exit(VOID);
static VOID  can_task(VOID *param);

TBOX_MODULE_FUN(CAN, can_init, can_stop, can_start, NULL_PTR, can_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(CAN, TBOX_TASK_PRIORITY_MID1, LOG_LEVEL_INFO, TBOX_TASK_MEDIUM_STACK_SIZE, can_task);
TBOX_MODULE_LOADER(CAN) {}

static TBOX_ID can_module_id;
static MODULE_HANDLE can_module_handle;
static UINT8 tbox_can_mode[DRV_CAN_INS_COUNT] = {DRV_CAN_MODE_NORMAL};
static TimerHandle_t xTimerHandle = NULL_PTR;
static can_callback_t can_calls = NULL_PTR;
static can_callback_t can_internal_cb = NULL_PTR;
static can_list_t recv_list;
static can_list_t send_list;

/* 内部回调注册（用于 recovery） */
static VOID can_if_regcb_internal(can_callback_t cb)
{
    can_internal_cb = cb;
}

/* 统一回调：先调用内部回调，再调用应用层回调 */
#define CAN_RUN_CALLBACK(e, p) do { \
    if (can_internal_cb) can_internal_cb((e), (p)); \
    if (can_calls) can_calls((e), (p)); \
} while(0)

/* recovery 回调函数 */
static VOID can_recovery_callback(UINT32 event, VOID *para)
{
    UINT32 ins_id;
    can_msg_t *msg;

    if (event & CAN_IF_EVENT_RECEIVED) {
        msg = (can_msg_t *)para;
        if (msg) {
            can_buserr_callback(msg->ins, FALSE);
            can_busoff_callback(msg->ins, FALSE);
        }
    } else if (event & CAN_IF_EVENT_SEND_DONE) {
        ins_id = (UINT32)para;
        can_buserr_callback(ins_id, FALSE);
        can_busoff_callback(ins_id, FALSE);
    } else if (event & CAN_IF_EVENT_BUS_ERROR) {
        ins_id = (UINT32)para;
        can_buserr_callback(ins_id, TRUE);
    } else if (event & CAN_IF_EVENT_BUSOFF) {
        ins_id = (UINT32)para;
        can_busoff_callback(ins_id, TRUE);
    } else if (event & CAN_IF_EVENT_BUSOK) {
    }
}

static VOID can_recv_handle(can_msg_t *msg)
{
    can_node_t *node;
    BaseType_t need_switch = pdFALSE;
    UINT32 notify_value;

    if (NULL_PTR == msg)
        return;

    UBaseType_t interruptstatus = taskENTER_CRITICAL_FROM_ISR();
    {
        node = can_list_malloc();
        if (NULL_PTR == node)
        {
            taskEXIT_CRITICAL_FROM_ISR(interruptstatus);
            return;
        }
        memcpy(&node->msg, msg, sizeof(can_msg_t));
        can_list_push(&recv_list, node);
    }
    taskEXIT_CRITICAL_FROM_ISR(interruptstatus);
    
    notify_value = msg->ins;
    notify_value = (notify_value << CAN_IF_EVENT_INS_POS) | CAN_IF_EVENT_RECEIVED;
    xTaskNotifyFromISR(can_module_handle, notify_value, eSetBits, &need_switch);
    portYIELD_FROM_ISR(need_switch);
}

static VOID can_send_handle(VOID *para)
{
    BaseType_t need_switch = pdFALSE;
    UINT32 notify_value = (UINT32)para;

    notify_value = (notify_value << CAN_IF_EVENT_INS_POS) | CAN_IF_EVENT_SEND_DONE;
    xTaskNotifyFromISR(can_module_handle, notify_value, eSetBits, &need_switch);
    portYIELD_FROM_ISR(need_switch);
}

static VOID can_event_handle(VOID *para)
{
    BaseType_t need_switch = pdFALSE;
    UINT32 notify_value = (UINT32)para;

    xTaskNotifyFromISR(can_module_handle, notify_value, eSetBits, &need_switch);
    portYIELD_FROM_ISR(need_switch);
}

static VOID can_isr_callback(UINT8 ins, UINT32 event, VOID *para)
{
    UINT32 ins_id = ins;

    switch (event) {
    case DRV_CAN_EVENT_RX_DONE:
        can_recv_handle((can_msg_t *)para);
        break;

    case DRV_CAN_EVENT_TX_DONE:
        can_send_handle((VOID *)ins_id);
        break;

    case DRV_CAN_EVENT_BUS_ERROR: {
        UINT32 notify_value;
        notify_value = (ins_id << CAN_IF_EVENT_INS_POS) | CAN_IF_EVENT_BUS_ERROR;
        can_event_handle((VOID *)notify_value);
        break;
    }

    case DRV_CAN_EVENT_BUS_OFF: {
        UINT32 notify_value;
        UINT32 error_code = (UINT32)para;

        if (error_code)
            notify_value = (ins_id << CAN_IF_EVENT_INS_POS) | CAN_IF_EVENT_BUSOFF;
        else
            notify_value = (ins_id << CAN_IF_EVENT_INS_POS) | CAN_IF_EVENT_BUSOK;
        can_event_handle((VOID *)notify_value);
        break;
    }

    case DRV_CAN_EVENT_WAKEUP:
        break;

    default:
        break;
    }
}

static VOID can_timer_callback(TimerHandle_t xTimer)
{
    xTaskNotify(can_module_handle, CAN_IF_EVENT_SEND_START, eSetBits);
}

static VOID can_task(VOID *param)
{
    UINT32 notify_value = 0;
    UINT32 ins_id;
    INT8 recv_size = 0U;
    can_node_t *node;
    can_msg_t msg;
    
    (VOID)param;
    
    for(;;) 
    {
        /* 检查模块状态 */
        if(tbox_module_get_state(can_module_id) != TBOX_MODULE_STATE_START)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        notify_value = 0;
        node = NULL_PTR;
        xTaskNotifyWait(0U, 0xFFFFFFFFU, &notify_value, portMAX_DELAY);
        
        if (notify_value & CAN_IF_EVENT_RECEIVED)
        {            
            taskENTER_CRITICAL();
            {
                recv_size = recv_list.size - 1;
                node = can_list_pop(&recv_list);
                if(NULL_PTR != node)
                {
                    memcpy(&msg, &node->msg, sizeof(can_msg_t));
                    can_list_free(node);
                }                           
            }
            taskEXIT_CRITICAL();
            
            if (NULL_PTR != node)
            {
                /* 更新统计信息 */
                can_mgr_stat_add_recv_msg(&msg);
                
                CAN_RUN_CALLBACK(CAN_IF_EVENT_RECEIVED, &msg);
            }
            
            if(recv_size > 0U)
            {
                xTaskNotify(can_module_handle, CAN_IF_EVENT_RECEIVED, eSetBits);
            }
        }

        if (notify_value & CAN_IF_EVENT_SEND_DONE || 
            (notify_value & CAN_IF_EVENT_SEND_START))
        {
            taskENTER_CRITICAL();
            node = can_list_pop(&send_list);
            if (NULL_PTR != node) 
            {
                memcpy(&msg, &node->msg, sizeof(can_msg_t));
                can_list_free(node);
            }
            taskEXIT_CRITICAL();

            if(pdTRUE == xTimerIsTimerActive(xTimerHandle))
            {
                if(NULL_PTR != xTimerHandle) {
                    xTimerStop(xTimerHandle, 0);
                }
            }
            
            if(NULL_PTR != node)
            {
                if(0U == drv_can_send(&msg))
                {
                    xTimerStart(xTimerHandle, 0);                  
                }
            }

            if(notify_value & CAN_IF_EVENT_SEND_DONE)
            {
                ins_id = (notify_value & CAN_IF_EVENT_INS_MASK) >> CAN_IF_EVENT_INS_POS;
                
                /* 更新统计信息 */
                can_mgr_stat_add_send_msg(ins_id);
                
                CAN_RUN_CALLBACK(CAN_IF_EVENT_SEND_DONE, (VOID *)ins_id);
            }
        }
        
        if (notify_value & CAN_IF_EVENT_BUSOFF) 
        {
            ins_id = (notify_value & CAN_IF_EVENT_INS_MASK) >> CAN_IF_EVENT_INS_POS;
            CAN_RUN_CALLBACK(CAN_IF_EVENT_BUSOFF, (VOID *)ins_id);
        }

        if (notify_value & CAN_IF_EVENT_BUS_ERROR) 
        {
            ins_id = (notify_value & CAN_IF_EVENT_INS_MASK) >> CAN_IF_EVENT_INS_POS;
            CAN_RUN_CALLBACK(CAN_IF_EVENT_BUS_ERROR, (VOID *)ins_id);
        }
        
        if (notify_value & CAN_IF_EVENT_BUSOK)
        {
            ins_id = (notify_value & CAN_IF_EVENT_INS_MASK) >> CAN_IF_EVENT_INS_POS;
            CAN_RUN_CALLBACK(CAN_IF_EVENT_BUSOK, (VOID *)ins_id);
        }
    }
}

static INT32 can_init(UINT8 seq)
{
    INT32 ret = (INT32)TBOX_E_OK;
    
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            GET_TBOX_MODULE_ID(CAN, can_module_id);
            GET_TBOX_MODULE_HANDLE(CAN, can_module_handle);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            can_list_init(&recv_list);
            can_list_init(&send_list);

            xTimerHandle = xTimerCreate(
                "Can STmr",          /* 定时器名称 */
                pdMS_TO_TICKS(100), /* 周期（ms） */
                pdFALSE,            /* 自动重载（一次性定时器） */
                (VOID *)0,          /* 定时器ID（可自定义） */
                can_timer_callback); /* 回调函数 */
            
            if (NULL_PTR == xTimerHandle) {
                MODULE_LOG_E(CAN, "create timer failed");
                return (INT32)TBOX_E_FAILED_INIT;
            }

            can_buserr_init();
            can_busoff_init();
            
            can_if_regcb_internal(can_recovery_callback);
            
            can_mgr_init();
            
            can_shell_init();
            
            tbox_module_set_state(can_module_id, TBOX_MODULE_STATE_START);
            
            MODULE_LOG_D(CAN, "CAN module initialized");
            break;
            
        default:
            break;
    }

    MODULE_LOG_D(CAN, "init seq:%d, ret:%d", seq, ret);
    return ret;
}

static VOID can_stop(VOID)
{
    UINT32 list_count = 0;
    UINT8 i;
    can_node_t *node;

    ulTaskNotifyValueClear(can_module_handle, 0xFFFFFFFFU);
    
    can_buserr_sleep();
    can_busoff_sleep();
    
    taskENTER_CRITICAL();
    list_count = recv_list.size;
    for(i = 0; i < list_count; i++)
    {
        node = can_list_pop(&recv_list);
        if(NULL_PTR == node)
        {
            break;
        }
        can_list_free(node);
    }
    list_count = send_list.size;
    for(i = 0; i < list_count; i++)
    {
        node = can_list_pop(&send_list);
        if(NULL_PTR == node)
        {
            break;
        }
        can_list_free(node);
    }
    taskEXIT_CRITICAL();
    
    can_mgr_deinit();
    
    tbox_module_set_state(can_module_id, TBOX_MODULE_STATE_STOP);
    
    MODULE_LOG_I(CAN, "CAN module stopped");
}

static VOID can_start(VOID)
{
    can_mgr_init();
    
    tbox_module_set_state(can_module_id, TBOX_MODULE_STATE_START);
    
    MODULE_LOG_I(CAN, "CAN module started");
}

static VOID can_exit(VOID)
{
    if (NULL_PTR != xTimerHandle) {
        xTimerDelete(xTimerHandle, 0);
        xTimerHandle = NULL_PTR;
    }
    
    MODULE_LOG_I(CAN, "CAN module exited");
}

INT32 can_if_init(UINT8 ins, UINT32 rate, UINT8 mode)
{
    INT32 ret = 0;
    UINT32 baudrate = rate;
    TBOX_CFG_ID cfg_id = CFG_ID_INVALID;

    if ((UINT8)ins >= (UINT8)DRV_CAN_INS_COUNT) 
    {
        return -1;
    }

    /* 如果 rate 为 0，则从配置中读取 */
    if (rate == 0) {
        if (ins == 0) {
            TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
        } else if (ins == 1) {
            TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
        } else if (ins == 2) {
            TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);
        } else {
            return -1;
        }

        if (cfg_id == (TBOX_CFG_ID)CFG_ID_INVALID || cfg_id >= TBOX_CFG_ITEM_NUMBER) {
            return -1;
        }

        ret = tbox_cfg_read(cfg_id, &baudrate);
        if (ret < 0) {
            return -1;
        }

        if (baudrate == 0) {
            return -1;
        }
    } else {
        /* 如果 rate 不为 0，更新配置 */
        if (ins == 0) {
            TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
        } else if (ins == 1) {
            TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
        } else if (ins == 2) {
            TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);
        } else {
            return -1;
        }

        if (cfg_id != (TBOX_CFG_ID)CFG_ID_INVALID && cfg_id < TBOX_CFG_ITEM_NUMBER) {
            ret = tbox_cfg_write(cfg_id, &baudrate);
            if (ret < 0) {
                /* 配置写入失败，但不影响初始化，继续执行 */
            }
        }
    }

    tbox_can_mode[ins] = mode;
    drv_can_register(ins, can_isr_callback);
    ret = drv_can_init(ins, baudrate, mode);
    if (ret == 0) {
        can_mgr_update_baudrate(ins, baudrate);
    }
    return ret;
}

INT32 can_if_deinit(UINT8 ins)
{
    if ((UINT8)ins >= (UINT8)DRV_CAN_INS_COUNT) 
    {
        return -1;
    }
    tbox_can_mode[ins] = DRV_CAN_MODE_NORMAL;
    can_mgr_update_baudrate(ins, 0);
    return drv_can_deinit(ins);
}

INT32 can_if_setbaud(UINT8 ins, UINT32 baudrate, UINT8 reinit)
{
    TBOX_CFG_ID cfg_id = CFG_ID_INVALID;
    INT32 ret;

    if ((UINT8)ins >= (UINT8)DRV_CAN_INS_COUNT) 
    {
        return -1;
    }

    /* 验证波特率 */
    if (baudrate != 0 && baudrate != 250 && baudrate != 500 && baudrate != 1000) {
        return -2;
    }

    /* 获取配置 ID */
    if (ins == 0) {
        TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
    } else if (ins == 1) {
        TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
    } else if (ins == 2) {
        TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);
    } else {
        return -1;
    }

    if (cfg_id == (TBOX_CFG_ID)CFG_ID_INVALID || cfg_id >= TBOX_CFG_ITEM_NUMBER) {
        return -3;
    }

    /* 写入配置 */
    ret = tbox_cfg_write(cfg_id, &baudrate);
    if (ret != 0) {
        return -4;
    }

    if (reinit != 0) {
        if (drv_can_has_init(ins)) {
            can_if_deinit(ins);
        }
        
        if (baudrate != 0) {
            ret = can_if_init(ins, baudrate, DRV_CAN_MODE_NORMAL);
            if (ret != 0) {
                return -5;
            }
        }
    }

    return 0;
}

INT32 can_if_send(can_msg_t *msg)
{
    if(NULL_PTR == msg || 
      (UINT8)msg->ins >= (UINT8)DRV_CAN_INS_COUNT)
    {
        return -1;
    }

    if(!drv_can_has_init(msg->ins) || 
      (DRV_CAN_MODE_NORMAL != tbox_can_mode[msg->ins]))
    {
        return -1;
    }

    if(0 != drv_can_send(msg))
    {
        taskENTER_CRITICAL();
        can_node_t *node = can_list_malloc();
        if (NULL_PTR != node)
        {
            memcpy(&node->msg, msg, sizeof(can_msg_t));
            can_list_push(&send_list, node);
        }
        taskEXIT_CRITICAL();
    }
    else
    {
        if(pdFALSE == xTimerIsTimerActive(xTimerHandle))
        {
            xTimerStart(xTimerHandle, 0);
        }
    }

    return 0;
}

INT32 can_if_regcb(can_callback_t cb)
{
    if (NULL_PTR == cb)
        return -1;

    can_calls = cb;
    return 0;
}

UINT32 can_if_get_send_size(VOID)
{
    return send_list.size;
}

UINT32 can_if_get_recv_size(VOID)
{
    return recv_list.size;
}
