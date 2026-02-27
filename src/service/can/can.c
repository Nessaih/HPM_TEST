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
#include "time_if.h"

static INT32 can_init(UINT8 seq);
static VOID  can_stop(VOID);
static VOID  can_start(VOID);
static VOID  can_exit(VOID);
static VOID  can_task(VOID *param);
TBOX_MODULE_FUN(CAN, can_init, can_stop, can_start, NULL_PTR, can_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(CAN, TBOX_TASK_PRIORITY_MID1, LOG_LEVEL_INFO, TBOX_TASK_MEDIUM_STACK_SIZE, can_task);
TBOX_MODULE_LOADER(CAN) {}

static INT32 can_tx_init(UINT8 seq);
static VOID  can_tx_stop(VOID);
static VOID  can_tx_start(VOID);
static VOID  can_tx_exit(VOID);
static VOID  can_tx_task(VOID *param);

TBOX_MODULE_FUN(CAN_TX, can_tx_init, can_tx_stop, can_tx_start, NULL_PTR, can_tx_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(CAN_TX, TBOX_TASK_PRIORITY_MID1 + 1, LOG_LEVEL_INFO, TBOX_TASK_MEDIUM_STACK_SIZE, can_tx_task);
TBOX_MODULE_LOADER(CAN_TX) {}

#define CAN_MAX_CALLBACK        16	   /* 最大回调数量 */
#define CAN_PROCESS_BATCH_SIZE  32     /* 每批处理的最大消息数 */
#define CAN_BUS_TIMEOUT_MS      10000  /* 10秒超时 */
#define CAN_SEND_RETRY_MS       100    /* 发送重试周期 */

static TBOX_ID can_module_id;
static MODULE_HANDLE can_module_handle;
static TBOX_ID can_tx_module_id;
static MODULE_HANDLE can_tx_module_handle;
static UINT8 tbox_can_mode[DRV_CAN_INS_COUNT] = {DRV_CAN_MODE_NORMAL};
static TimerHandle_t xSendTimerHandle = NULL_PTR;
static TimerHandle_t xBusTimerHandle = NULL_PTR;
static can_callback_t can_internal_cb = NULL_PTR;
static can_event_callback_t can_event_cb_tbl[CAN_MAX_CALLBACK];
static can_list_t recv_list;
static can_list_t send_list[DRV_CAN_INS_COUNT];  /* 每个CAN实例独立的发送队列 */
static bool canbus_active = false;
static bool canbus_sleep = false;
static can_msg_t can_process_buffer[CAN_PROCESS_BATCH_SIZE];  /* 静态消息处理缓冲区 */

/* 事件回调表管理 */
static VOID can_event_do_callback(CAN_EVENT event, UINT32 arg1, UINT32 arg2)
{
    INT32 i;
    for (i = 0; i < CAN_MAX_CALLBACK && can_event_cb_tbl[i]; i++) {
        can_event_cb_tbl[i](event, arg1, arg2);
    }
}

INT32 can_if_reg_cb(can_event_callback_t cb)
{
    INT32 i;
    
    if (NULL_PTR == cb) {
        return -1;
    }
    
    taskENTER_CRITICAL();
    for (i = 0; i < CAN_MAX_CALLBACK; i++) {
        if (NULL_PTR == can_event_cb_tbl[i]) {
            can_event_cb_tbl[i] = cb;
            taskEXIT_CRITICAL();
            return 0;
        }
    }
    taskEXIT_CRITICAL();
    
    return -1;  /* 回调表已满 */
}

INT32 can_if_unreg_cb(can_event_callback_t cb)
{
    INT32 i;
    
    if (NULL_PTR == cb) {
        return -1;
    }
    
    taskENTER_CRITICAL();
    for (i = 0; i < CAN_MAX_CALLBACK; i++) {
        if (can_event_cb_tbl[i] == cb) {
            can_event_cb_tbl[i] = NULL_PTR;
            taskEXIT_CRITICAL();
            return 0;
        }
    }
    taskEXIT_CRITICAL();
    
    return -1;  /* 未找到回调 */
}

/* 内部回调注册（用于 recovery） */
static VOID can_if_regcb_internal(can_callback_t cb)
{
    can_internal_cb = cb;
}
/* recovery 回调函数 */
static VOID can_recovery_callback(UINT32 event, VOID *para, UINT32 count)
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
    xTaskNotifyFromISR(can_tx_module_handle, notify_value, eSetBits, &need_switch);
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

/* 清除接收缓存 */
static VOID can_clear_recv_buffer(VOID)
{
    can_node_t *node;
    
    taskENTER_CRITICAL();
    while ((node = can_list_pop(&recv_list)) != NULL_PTR) {
        can_list_free(node);
    }
    taskEXIT_CRITICAL();
}

/* 清除发送缓存 */
static VOID can_clear_send_buffer(VOID)
{
    can_node_t *node;
    
    taskENTER_CRITICAL();
    for (UINT8 i = 0; i < DRV_CAN_INS_COUNT; i++) {
        while ((node = can_list_pop(&send_list[i])) != NULL_PTR) {
            can_list_free(node);
        }
    }
    taskEXIT_CRITICAL();
}

/* CAN总线超时回调 */
static VOID can_bus_timeout_callback(TimerHandle_t xTimer)
{
    UINT32 uptime = time_if_get_systick_s();
    
    if (canbus_active) {
        canbus_active = false;
        MODULE_LOG_W(CAN, "CAN bus timeout");
        can_clear_recv_buffer();
    }
    
    if (!canbus_sleep) {
        can_event_do_callback(CAN_EVENT_INACTIVE, uptime, 0);
    }
}

/* 处理接收到的CAN消息 */
static VOID can_process_recv_messages(VOID)
{
    can_node_t *node;
    INT32 msg_count;
    bool has_more;
    
    do {
        msg_count = 0;
        has_more = false;
        
        /* 批量取出消息（最多CAN_PROCESS_BATCH_SIZE条） */
        taskENTER_CRITICAL();
        {
            while (msg_count < CAN_PROCESS_BATCH_SIZE && 
                   (node = can_list_pop(&recv_list)) != NULL_PTR) {
                memcpy(&can_process_buffer[msg_count], &node->msg, sizeof(can_msg_t));
                can_list_free(node);
                msg_count++;
            }
            
            /* 检查是否还有更多消息 */
            has_more = (recv_list.size > 0);
        }
        taskEXIT_CRITICAL();
        
        if (msg_count <= 0) {
            return;
        }
        
        /* 重启总线超时定时器 */
        if (xBusTimerHandle != NULL_PTR) {
            xTimerReset(xBusTimerHandle, 0);
        }
        
        /* 检查总线状态变化 */
        if (!canbus_active) {
            canbus_active = true;
            MODULE_LOG_I(CAN, "CAN bus active");
            can_event_do_callback(CAN_EVENT_ACTIVE, 0, 0);
        }
        
        /* 批量数据接收回调 */
        can_event_do_callback(CAN_EVENT_DATAIN, (UINT32)can_process_buffer, msg_count);
        
        /* 批量更新统计信息 */
        can_mgr_stat_add_recv_msgs(can_process_buffer, msg_count);
        
        /* 内部回调 */
        if (can_internal_cb) {
            can_internal_cb(CAN_IF_EVENT_RECEIVED, can_process_buffer, msg_count);
        }
    } while (has_more);
}

static VOID can_send_timer_callback(TimerHandle_t xTimer)
{
    xTaskNotify(can_tx_module_handle, CAN_IF_EVENT_SEND_START, eSetBits);
}

/* CAN TX任务 - 专门处理发送事件 */
static VOID can_tx_task(VOID *param)
{
    UINT32 notify_value = 0;
    UINT32 ins_id;
    can_node_t *node;
    can_msg_t msg;
    
    (VOID)param;
    
    for(;;) 
    {
        /* 检查模块状态 */
        if(tbox_module_get_state(can_tx_module_id) != TBOX_MODULE_STATE_START)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        notify_value = 0;
        node = NULL_PTR;
        xTaskNotifyWait(0U, 0xFFFFFFFFU, &notify_value, portMAX_DELAY);
        
        /* 处理发送事件 */
        if (notify_value & CAN_IF_EVENT_SEND_DONE || 
            (notify_value & CAN_IF_EVENT_SEND_START))
        {
            UINT8 start_ins = 0;
            
            /* 如果是发送完成事件，优先处理该通道 */
            if (notify_value & CAN_IF_EVENT_SEND_DONE) {
                start_ins = (notify_value & CAN_IF_EVENT_INS_MASK) >> CAN_IF_EVENT_INS_POS;
            }
            
            /* 轮询所有通道（从刚完成的通道开始） */
            for (UINT8 i = 0; i < DRV_CAN_INS_COUNT; i++) {
                UINT8 check_ins = (start_ins + i) % DRV_CAN_INS_COUNT;
                
                taskENTER_CRITICAL();
                node = can_list_pop(&send_list[check_ins]);
                if (NULL_PTR != node) {
                    memcpy(&msg, &node->msg, sizeof(can_msg_t));
                    can_list_free(node);
                }
                taskEXIT_CRITICAL();
                
                if (NULL_PTR != node) {
                    if (pdTRUE == xTimerIsTimerActive(xSendTimerHandle)) {
                        if (NULL_PTR != xSendTimerHandle) {
                            xTimerStop(xSendTimerHandle, 0);
                        }
                    }
                    
                    if (0U == drv_can_send(&msg)) {
                        xTimerStart(xSendTimerHandle, 0);
                    }
                    break;  /* 发送一条后退出 */
                }
            }

            if(notify_value & CAN_IF_EVENT_SEND_DONE)
            {
                ins_id = (notify_value & CAN_IF_EVENT_INS_MASK) >> CAN_IF_EVENT_INS_POS;
                
                /* 发送完成回调 */
                if (can_internal_cb) {
                    can_internal_cb(CAN_IF_EVENT_SEND_DONE, (VOID *)ins_id, 0);
                }
            }
        }
    }
}

static VOID can_task(VOID *param)
{
    UINT32 notify_value = 0;
    UINT32 ins_id;
    
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
        xTaskNotifyWait(0U, 0xFFFFFFFFU, &notify_value, portMAX_DELAY);
        
        /* 处理接收事件 */
        if (notify_value & CAN_IF_EVENT_RECEIVED)
        {
            can_process_recv_messages();
        }
        
        /* 处理异常事件 */
        if (notify_value & CAN_IF_EVENT_BUSOFF) 
        {
            ins_id = (notify_value & CAN_IF_EVENT_INS_MASK) >> CAN_IF_EVENT_INS_POS;
            if (can_internal_cb) {
                can_internal_cb(CAN_IF_EVENT_BUSOFF, (VOID *)ins_id, 0);
            }
        }

        if (notify_value & CAN_IF_EVENT_BUS_ERROR) 
        {
            ins_id = (notify_value & CAN_IF_EVENT_INS_MASK) >> CAN_IF_EVENT_INS_POS;
            if (can_internal_cb) {
                can_internal_cb(CAN_IF_EVENT_BUS_ERROR, (VOID *)ins_id, 0);
            }
        }
        
        if (notify_value & CAN_IF_EVENT_BUSOK)
        {
            ins_id = (notify_value & CAN_IF_EVENT_INS_MASK) >> CAN_IF_EVENT_INS_POS;
            if (can_internal_cb) {
                can_internal_cb(CAN_IF_EVENT_BUSOK, (VOID *)ins_id, 0);
            }
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
            
            /* 初始化回调表 */
            memset(can_event_cb_tbl, 0, sizeof(can_event_cb_tbl));
            
            /* 创建总线超时定时器 */
            xBusTimerHandle = xTimerCreate(
                "CanBTmr",                      /* 定时器名称 */
                pdMS_TO_TICKS(CAN_BUS_TIMEOUT_MS), /* 10秒超时 */
                pdFALSE,                        /* 自动重载 */
                (VOID *)0,                      /* 定时器ID */
                can_bus_timeout_callback);      /* 回调函数 */
            
            if (NULL_PTR == xBusTimerHandle) {
                MODULE_LOG_E(CAN, "create bus timer failed");
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
    ulTaskNotifyValueClear(can_module_handle, 0xFFFFFFFFU);
    
    /* 停止定时器 */
    if (xBusTimerHandle != NULL_PTR && pdTRUE == xTimerIsTimerActive(xBusTimerHandle)) {
        xTimerStop(xBusTimerHandle, 0);
    }
    
    /* 设置休眠标志 */
    canbus_sleep = true;
    canbus_active = false;
    
    /* 触发休眠事件 */
    can_event_do_callback(CAN_EVENT_SLEEP, 0, 0);
    
    can_buserr_sleep();
    can_busoff_sleep();
    
    /* 清除接收缓存 */
    can_clear_recv_buffer();
    
    can_mgr_deinit();
    
    tbox_module_set_state(can_module_id, TBOX_MODULE_STATE_STOP);
    
    MODULE_LOG_I(CAN, "CAN module stopped");
}

static VOID can_start(VOID)
{
    /* 清除休眠标志 */
    canbus_sleep = false;
    
    /* 触发唤醒事件 */
    can_event_do_callback(CAN_EVENT_WAKEUP, 0, 0);
    
    can_mgr_init();
    
    tbox_module_set_state(can_module_id, TBOX_MODULE_STATE_START);
    
    MODULE_LOG_I(CAN, "CAN module started");
}

static VOID can_exit(VOID)
{
    if (NULL_PTR != xBusTimerHandle) {
        xTimerDelete(xBusTimerHandle, 0);
        xBusTimerHandle = NULL_PTR;
    }
    
    MODULE_LOG_I(CAN, "CAN module exited");
}

static INT32 can_tx_init(UINT8 seq)
{
    INT32 ret = (INT32)TBOX_E_OK;
    
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            GET_TBOX_MODULE_ID(CAN_TX, can_tx_module_id);
            GET_TBOX_MODULE_HANDLE(CAN_TX, can_tx_module_handle);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            for (UINT8 i = 0; i < DRV_CAN_INS_COUNT; i++) {
                can_list_init(&send_list[i]);
            }
            
            /* 创建发送定时器 */
            xSendTimerHandle = xTimerCreate(
                "CanSTmr",                      /* 定时器名称 */
                pdMS_TO_TICKS(CAN_SEND_RETRY_MS), /* 周期（ms） */
                pdFALSE,                        /* 自动重载（一次性定时器） */
                (VOID *)0,                      /* 定时器ID */
                can_send_timer_callback);       /* 回调函数 */
            
            if (NULL_PTR == xSendTimerHandle) {
                MODULE_LOG_E(CAN, "create send timer failed");
                return (INT32)TBOX_E_FAILED_INIT;
            }
            
            tbox_module_set_state(can_tx_module_id, TBOX_MODULE_STATE_START);
            
            MODULE_LOG_D(CAN, "CAN TX module initialized");
            break;
            
        default:
            break;
    }
    
    return ret;
}

static VOID can_tx_stop(VOID)
{
    if (xSendTimerHandle != NULL_PTR && pdTRUE == xTimerIsTimerActive(xSendTimerHandle)) {
        xTimerStop(xSendTimerHandle, 0);
    }
    
	can_clear_send_buffer();
    
    ulTaskNotifyValueClear(can_tx_module_handle, 0xFFFFFFFFU);
    
    tbox_module_set_state(can_tx_module_id, TBOX_MODULE_STATE_STOP);
}

static VOID can_tx_start(VOID)
{
}

static VOID can_tx_exit(VOID)
{
    if (NULL_PTR != xSendTimerHandle) {
        xTimerDelete(xSendTimerHandle, 0);
        xSendTimerHandle = NULL_PTR;
    }
    
    MODULE_LOG_I(CAN, "CAN TX module exited");
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
            can_list_push(&send_list[msg->ins], node);
            taskEXIT_CRITICAL();
            
            can_mgr_stat_add_send_msg(msg->ins);
        }
        else
        {
            taskEXIT_CRITICAL();
            /* 队列满，发送失败 */
            return -1;
        }
    }
    else
    {
        can_mgr_stat_add_send_msg(msg->ins);
        
        if(pdFALSE == xTimerIsTimerActive(xSendTimerHandle))
        {
            xTimerStart(xSendTimerHandle, 0);
        }
    }

    return 0;
}

BOOL can_if_is_bus_active(VOID)
{
    return canbus_active;
}

BOOL can_if_is_bus_sleep(VOID)
{
    return canbus_sleep;
}
