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
#include "Can_Hal.h"
#include "time_if.h"

/*事件位说明：
*1、低8位(0到8位）主要给任务事件使用
*2、接收任务的事件位：每路CAN使用4位，其余24位可供6路CAN使用
*3、发送任务的事件位：每路CAN使用2位
*/
#define CAN_TASK_EVENT_START    0x01U
#define CAN_TASK_EVENT_STOP     0x02U
#define CAN_TASK_EVENT_EXIT     0x04U
#define CAN_TASK_EVENT_CHECK    0x08U
#define CAN_TASK_EVENT_BITNUM   8U
#define CAN_RX_EVENT_RECEIVE    0x01U
#define CAN_RX_EVENT_BUSERROR   0x02U
#define CAN_RX_EVENT_BUSOFF     0x04U
#define CAN_RX_EVENT_BUSOK      0x08U
#define CAN_RX_EVENT_BITNUM     4U
#define CAN_TX_EVENT_START      0x01U
#define CAN_TX_EVENT_DONE       0x02U
#define CAN_TX_EVENT_BITNUM     2U

#define CAN_CHECK_PERIOD        50U     /* 任务检查周期50MS */
#define CAN_TX_CHECK_PERIOD     50U     /* 发送任务检查周期50MS */
#define CAN_TX_SEND_TIMEOUT     100U    /* 发送超时时间100MS */
#define CAN_RX_CECK_PERIOD      1000U   /* 接收任务检查周期1S */
#define CAN_BUS_DEACTIVE_TIME   10000U  /* 总线空闲超时时间10S */
#define CAN_CALLBACK_COUNT      5U      /* 回调表最大数量 */
#define CAN_RX_PROCESS_ONCE_SIZE 5U     /* 单次接收处理最大数量 */
#define CAN_RX_RETRY_COUNT      3U      /* 接收重试次数 */

typedef struct
{
    UINT8 mode;
    UINT32 tx_tick;
    can_list_t rx_list;
    can_list_t tx_list;
}CAN_ITEM;

typedef struct
{
    BOOL bus_active;
    BOOL bus_sleep;
    UINT32 active_tick;
    CAN_ITEM items[DRV_CAN_INS_COUNT];
}CAN_CONTEXT;

static VOID can_check_timer_callback(TimerHandle_t xtimer);
static VOID can_rx_handle_check(VOID);
static VOID can_rx_handle_receive(UINT8 ins);
static VOID can_rx_handle_buserror(UINT8 ins);
static VOID can_rx_handle_busoff(UINT8 ins);
static VOID can_rx_handle_busok(UINT8 ins);
static VOID can_rx_clear_list(VOID);
static VOID can_tx_handle_check(VOID);
static VOID can_tx_handle_start(UINT8 ins);
static VOID can_tx_handle_done(UINT8 ins);
static VOID can_tx_clear_list(VOID);
static VOID can_isr_callback(UINT8 ins, UINT32 event, VOID *para);
inline static VOID can_event_do_callback(CAN_EVENT event, UINT32 arg1, UINT32 arg2);

static INT32 can_init(UINT8 seq);
static VOID  can_stop(VOID);
static VOID  can_start(VOID);
static VOID  can_exit(VOID);
static VOID  can_task(VOID *param);
TBOX_MODULE_FUN(CAN, can_init, can_stop, can_start, NULL_PTR, can_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(CAN, TBOX_TASK_PRIORITY_MID, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, can_task);
TBOX_MODULE_LOADER(CAN) {}

static INT32 can_tx_init(UINT8 seq);
static VOID  can_tx_stop(VOID);
static VOID  can_tx_start(VOID);
static VOID  can_tx_exit(VOID);
static VOID  can_tx_task(VOID *param);
TBOX_MODULE_FUN(CAN_TX, can_tx_init, can_tx_stop, can_tx_start, NULL_PTR, can_tx_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(CAN_TX, TBOX_TASK_PRIORITY_MID3, LOG_LEVEL_ERROR, TBOX_TASK_SMALL_STACK_SIZE_1, can_tx_task);
TBOX_MODULE_LOADER(CAN_TX) {}

static MODULE_HANDLE can_module_handle;
static MODULE_HANDLE can_tx_module_handle;
static TimerHandle_t can_check_timer;
static can_event_callback_t can_event_cb_tbl[CAN_CALLBACK_COUNT];
static can_msg_t can_process_buffer[CAN_RX_PROCESS_ONCE_SIZE];  /* 静态消息处理缓冲区 */
static CAN_CONTEXT can_context;

INT32 can_if_init(UINT8 ins, UINT32 rate, UINT8 mode)
{
    if ((UINT8)ins >= (UINT8)DRV_CAN_INS_COUNT) 
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    INT32 ret = 0;
    UINT32 baudrate = rate;
    if (rate == 0U) 
    {
        TBOX_CFG_ID cfg_id;
        if (ins == 0U) 
        {
            TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
        } 
        else if (ins == 1U) 
        {
            TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
        } 
        else if (ins == 2U) 
        {
            TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);
        } 
        else 
        {
            cfg_id = CFG_ID_INVALID;
        }
        if (cfg_id == (TBOX_CFG_ID)CFG_ID_INVALID || cfg_id >= TBOX_CFG_ITEM_NUMBER) 
        {
            return (INT32)TBOX_E_INVALID_VALUE;
        }
        ret = tbox_cfg_read(cfg_id, &baudrate);
        if (ret < 0) 
        {
            MODULE_LOG_E(CAN, "read baudrate failed, ret:%d", ret);
            return (INT32)TBOX_E_FAILED;
        }
        if (baudrate == 0U) 
        {
            MODULE_LOG_W(CAN, "can[%d] not config", ins);
            return (INT32)TBOX_E_FAILED;
        }
    }

    /*不需要写入配置，上层业务来控制
    tbox_cfg_write(cfg_id, &baudrate);*/

    taskENTER_CRITICAL();
    can_context.items[ins].mode = mode;
    taskEXIT_CRITICAL();
    drv_can_register(ins, can_isr_callback);
    ret = drv_can_init(ins, baudrate, mode);
    if (ret == 0) 
    {
        can_mgr_update_baudrate(ins, baudrate);
        MODULE_LOG_I(CAN, "can[%d] init success", ins);
        return (INT32)TBOX_E_OK;
    }

    MODULE_LOG_E(CAN, "can[%d] init failed, ret:%d", ins, ret);

    return (INT32)TBOX_E_FAILED;
}

INT32 can_if_deinit(UINT8 ins)
{
    if ((UINT8)ins >= (UINT8)DRV_CAN_INS_COUNT) 
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    taskENTER_CRITICAL();
    can_context.items[ins].mode = DRV_CAN_MODE_NORMAL;
    taskEXIT_CRITICAL();
    can_mgr_update_baudrate(ins, 0U);

    MODULE_LOG_I(CAN, "can[%d] deinit success", ins);

    return drv_can_deinit(ins);
}

INT32  can_if_setbaud(UINT8 ins, UINT32 baudrate, UINT8 reinit)
{
    if ((UINT8)ins >= (UINT8)DRV_CAN_INS_COUNT) 
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    /* 验证波特率 */
    if (baudrate != 0U && baudrate != 250U && baudrate != 500U && baudrate != 1000U) 
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    /* 获取配置 ID */
    TBOX_CFG_ID cfg_id;
    if (ins == 0U) 
    {
        TBOX_CFG_ID_GET(CAN1BAUD, cfg_id);
    } 
    else if (ins == 1U) 
    {
        TBOX_CFG_ID_GET(CAN2BAUD, cfg_id);
    } 
    else if (ins == 2) 
    {
        TBOX_CFG_ID_GET(CAN3BAUD, cfg_id);
    } 
    else 
    {
        cfg_id = CFG_ID_INVALID;
    }
    if (cfg_id == (TBOX_CFG_ID)CFG_ID_INVALID || cfg_id >= TBOX_CFG_ITEM_NUMBER) 
    {
        return (INT32)TBOX_E_INVALID_VALUE;
    }

    /* 写入配置 */
    INT32 ret = tbox_cfg_write(cfg_id, &baudrate);
    if (ret != (INT32)TBOX_E_OK) 
    {
        MODULE_LOG_E(CAN, "write baudrate failed, ret:%d", ret);
        return (INT32)TBOX_E_FAILED;
    }

    if (reinit != 0U) 
    {
        if (drv_can_has_init(ins)) 
        {
            can_if_deinit(ins);
        }
        if (baudrate != 0U) 
        {
            ret = can_if_init(ins, baudrate, DRV_CAN_MODE_NORMAL);
            if (ret != (INT32)TBOX_E_OK) 
            {
                return (INT32)TBOX_E_FAILED_INIT;
            }
        }
    }

    return (INT32)TBOX_E_OK;
}

INT32  can_if_send(can_msg_t *msg)
{
    if(NULL_PTR == msg || 
      (UINT8)msg->ins >= (UINT8)DRV_CAN_INS_COUNT)
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }

    if(!drv_can_has_init(msg->ins) || 
      (DRV_CAN_MODE_NORMAL != can_context.items[msg->ins].mode))
    {
        return (INT32)TBOX_E_NOINIT;
    }

    if(0 != drv_can_send(msg))
    {
        taskENTER_CRITICAL();
        can_node_t *node = can_list_malloc();
        if (NULL_PTR == node)
        {
            taskEXIT_CRITICAL();
            can_mgr_stat_add_droptx_msg(msg->ins);
            MODULE_LOG_E(CAN, "can[%d] malloc node failed", msg->ins);
            return (INT32)TBOX_E_NOMEMORY;
        }
        memcpy(&node->msg, msg, sizeof(can_msg_t));
        can_list_push(&can_context.items[msg->ins].tx_list, node);
        can_context.items[msg->ins].tx_tick = time_if_get_systick_ms();
        taskEXIT_CRITICAL();
    }
    else
    {
        taskENTER_CRITICAL();
        can_context.items[msg->ins].tx_tick = time_if_get_systick_ms();
        taskEXIT_CRITICAL();
        can_mgr_stat_add_send_msg(msg->ins);

        MODULE_LOG_D(CAN, "can[%d] send success", msg->ins);
    }

    return (INT32)TBOX_E_OK;
}

INT32  can_if_reg_cb(can_event_callback_t cb)
{    
    if (NULL_PTR == cb) 
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    
    taskENTER_CRITICAL();
    for (UINT32 i = 0U; i < CAN_CALLBACK_COUNT; i++) 
    {
        if (NULL_PTR == can_event_cb_tbl[i]) 
        {
            can_event_cb_tbl[i] = cb;
            taskEXIT_CRITICAL();
            return (INT32)TBOX_E_OK;
        }
    }
    taskEXIT_CRITICAL();
    
    MODULE_LOG_E(CAN, "can callback table full");
    return (INT32)TBOX_E_NOMEMORY;
}

INT32  can_if_unreg_cb(can_event_callback_t cb)
{    
    if (NULL_PTR == cb) 
    {
        return (INT32)TBOX_E_INVALID_PARAM;
    }
    
    taskENTER_CRITICAL();
    for (UINT32 i = 0; i < CAN_CALLBACK_COUNT; i++) 
    {
        if (can_event_cb_tbl[i] == cb) 
        {
            can_event_cb_tbl[i] = NULL_PTR;
            taskEXIT_CRITICAL();
            return (INT32)TBOX_E_OK;
        }
    }
    taskEXIT_CRITICAL();

    MODULE_LOG_E(CAN, "can callback not found");
    return (INT32)TBOX_E_NOFOUND;   
}

BOOL   can_if_is_bus_active(VOID)
{
    BOOL active;
    
    taskENTER_CRITICAL();
    active = can_context.bus_active;
    taskEXIT_CRITICAL();

    return active;
}

BOOL   can_if_is_bus_sleep(VOID)
{
    BOOL sleep;
    
    taskENTER_CRITICAL();
    sleep = can_context.bus_sleep;
    taskEXIT_CRITICAL();

    return sleep;
}

static VOID can_check_timer_callback(TimerHandle_t xtimer)
{
    static UINT8 rx_check_count = 0U;
    static UINT8 tx_check_count = 0U;

    if(xtimer != can_check_timer)
    {
        return;
    }

    if(++rx_check_count >= CAN_RX_CECK_PERIOD/CAN_CHECK_PERIOD)
    {
        rx_check_count = 0U;
        if(NULL_PTR != can_module_handle)
        {
            xTaskNotify(can_module_handle, CAN_TASK_EVENT_CHECK, eSetBits);
        }
    }
    if(++tx_check_count >= CAN_TX_CHECK_PERIOD/CAN_CHECK_PERIOD)
    {
        tx_check_count = 0U;
        if(NULL_PTR != can_tx_module_handle)
        {
            xTaskNotify(can_tx_module_handle, CAN_TASK_EVENT_CHECK, eSetBits);
        }
    }
}

static VOID can_rx_handle_check(VOID)
{
    /*检测总线是否ACTIVE*/
    BOOL bus_sleep;
    BOOL is_deactive = FALSE;
    UINT32 tick = time_if_get_systick_ms();

    taskENTER_CRITICAL();
    if(0U == can_context.active_tick ||
       FALSE == can_context.bus_active)
    {
        taskEXIT_CRITICAL();
        MODULE_LOG_I(CAN, "can rx not do check, active_tick:%d, bus_active:%d", 
                           can_context.active_tick, can_context.bus_active);
        return;
    }
    if(tick - can_context.active_tick >= CAN_BUS_DEACTIVE_TIME)
    {
        can_rx_clear_list();
        bus_sleep = can_context.bus_sleep;
        can_context.bus_active = FALSE;
        is_deactive = TRUE;
    }
    taskEXIT_CRITICAL();

    if(TRUE == is_deactive && FALSE == bus_sleep)
    {
        MODULE_LOG_W(CAN, "bus deactive");
        can_event_do_callback(CAN_EVENT_INACTIVE, tick/1000U, 0U);
    }
}

static VOID can_rx_handle_receive(UINT8 ins)
{
    UINT8 msg_count;
    BOOL active_cb;
    can_node_t *node;
    UINT32 tick = time_if_get_systick_ms();

    for(UINT8 retry_count = 0U; retry_count < CAN_RX_RETRY_COUNT; retry_count++)
    {
        node = NULL_PTR;
        msg_count = 0U;
        active_cb = FALSE;

        taskENTER_CRITICAL();
        {
            while (msg_count < CAN_RX_PROCESS_ONCE_SIZE && 
                   (node = can_list_pop(&can_context.items[ins].rx_list)) != NULL_PTR)
            {
                memcpy(&can_process_buffer[msg_count], &node->msg, sizeof(can_msg_t));
                can_list_free(node);
                msg_count++;
            }
            if(0U == msg_count)
            {
                taskEXIT_CRITICAL();
                return;
            }

            can_context.active_tick = tick;
            if(FALSE == can_context.bus_active)
            {
                can_context.bus_active = TRUE;
                active_cb = TRUE;
            }
        }
        taskEXIT_CRITICAL();

        /* 检查总线状态变化 */
        if (TRUE == active_cb) 
        {
            MODULE_LOG_I(CAN, "can bus active");
            can_event_do_callback(CAN_EVENT_ACTIVE, 0, 0);
        }
        
        /* 批量数据接收回调 */
        can_event_do_callback(CAN_EVENT_DATAIN, (UINT32)can_process_buffer, msg_count);
        
        /* 批量更新统计信息 */
        can_mgr_stat_add_recv_msgs(can_process_buffer, msg_count);
        
        /* BUS OFF/ERROR 处理 */
        can_buserr_callback(ins, FALSE);
        can_busoff_callback(ins, FALSE);            
    }
}

static VOID can_rx_handle_buserror(UINT8 ins)
{
    taskENTER_CRITICAL();
    can_rx_clear_list();
    can_tx_clear_list();
    taskEXIT_CRITICAL();

    can_buserr_callback(ins, TRUE);

    MODULE_LOG_D(CAN, "can[%d] bus error", ins);
}

static VOID can_rx_handle_busoff(UINT8 ins)
{
    taskENTER_CRITICAL();
    can_rx_clear_list();
    taskEXIT_CRITICAL();

    can_busoff_callback(ins, TRUE);

    MODULE_LOG_E(CAN, "can[%d] bus off", ins);    
}

static VOID can_rx_handle_busok(UINT8 ins)
{
    UINT32 error = Can_Hal_GetErrorsInfo(ins);
    MODULE_LOG_I(CAN, "can[%d] error:%d", ins, error); 
    if(0 == error)
    {
        can_buserr_callback(ins, FALSE);
        MODULE_LOG_I(CAN, "can[%d] bus ok", ins); 
    }
   /*can_buserr_callback(ins, FALSE);
    can_busoff_callback(ins, FALSE);
    MODULE_LOG_I(CAN, "can[%d] bus ok", ins);  */    
}

static VOID can_rx_clear_list(VOID)
{
    can_node_t *node;
    for (UINT8 i = 0; i < DRV_CAN_INS_COUNT; i++) 
    {
        can_mgr_stat_add_droprx_msgs(i, can_context.items[i].rx_list.size);
        while ((node = can_list_pop(&can_context.items[i].rx_list)) != NULL_PTR) 
        {
            can_list_free(node);
        }
    }    
}

static VOID can_tx_handle_check(VOID)
{
    UINT32 tick = time_if_get_systick_ms();

    for (UINT8 i = 0U; i < DRV_CAN_INS_COUNT; i++) 
    {
        taskENTER_CRITICAL();
        if(0U == can_context.items[i].tx_tick)
        {
            taskEXIT_CRITICAL();
            continue;
        }
        if(tick - can_context.items[i].tx_tick >= CAN_TX_SEND_TIMEOUT)
        {
            can_context.items[i].tx_tick = 0U;
        }
        taskEXIT_CRITICAL();
        if(0U == can_context.items[i].tx_tick)
        {
            MODULE_LOG_W(CAN, "can[%d] tx timeout, tick:%d", i, tick);

            if(NULL_PTR != can_tx_module_handle)
            {
                xTaskNotify(can_tx_module_handle, CAN_TX_EVENT_START << (CAN_TASK_EVENT_BITNUM+i*CAN_TX_EVENT_BITNUM), eSetBits);
            }            
        }
    }
}

static VOID can_tx_handle_start(UINT8 ins)
{
    can_node_t *node;
    can_msg_t msg;

    taskENTER_CRITICAL();
    node = can_list_pop(&can_context.items[ins].tx_list);
    if (NULL_PTR != node) 
    {
        memcpy(&msg, &node->msg, sizeof(can_msg_t));
        can_list_free(node);
    }
    taskEXIT_CRITICAL();

    if(NULL_PTR == node)
    {
        return;
    }

    if(0 == drv_can_send(&msg))
    {
        can_mgr_stat_add_send_msg(ins);
        MODULE_LOG_I(CAN, "can[%d] send success", ins);
    }
    else
    {
        MODULE_LOG_E(CAN, "can[%d] tx send failed", ins);
        can_mgr_stat_add_droptx_msg(ins);
    }

    taskENTER_CRITICAL();
    can_context.items[ins].tx_tick =  time_if_get_systick_ms();
    taskEXIT_CRITICAL();    
}

static VOID can_tx_handle_done(UINT8 ins)
{
    can_node_t *node;
    can_msg_t msg;

    MODULE_LOG_I(CAN, "can[%d] tx done", ins);

    taskENTER_CRITICAL();
    node = can_list_pop(&can_context.items[ins].tx_list);
    if (NULL_PTR != node) 
    {
        memcpy(&msg, &node->msg, sizeof(can_msg_t));
        can_list_free(node);
    }
    can_context.items[ins].tx_tick = 0U;
    taskEXIT_CRITICAL();

    can_buserr_callback(ins, FALSE);
    can_busoff_callback(ins, FALSE); 

    if(NULL_PTR == node)
    {
        return;
    }

    if(0 == drv_can_send(&msg))
    {
        can_mgr_stat_add_send_msg(ins);
        MODULE_LOG_I(CAN, "can[%d] send success", ins);        
    }
    else
    {
        MODULE_LOG_E(CAN, "can[%d] tx send failed", ins);
        can_mgr_stat_add_droptx_msg(ins);
    }

    taskENTER_CRITICAL();
    can_context.items[ins].tx_tick =  time_if_get_systick_ms();
    taskEXIT_CRITICAL();        
}

static VOID can_tx_clear_list(VOID)
{
    can_node_t *node;
    for (UINT8 i = 0; i < DRV_CAN_INS_COUNT; i++) 
    {
        while ((node = can_list_pop(&can_context.items[i].tx_list)) != NULL_PTR) 
        {
            can_list_free(node);
            can_mgr_stat_add_droptx_msg(i);
        }
    }   
}

static VOID can_isr_callback(UINT8 ins, UINT32 event, VOID *para)
{
    UINT32 notify_value;

    if(ins >= DRV_CAN_INS_COUNT)
    {
        return;
    }

    switch (event) 
    {
        case DRV_CAN_EVENT_RX_DONE:
            {
                can_node_t *node;
                can_msg_t *msg = (can_msg_t *)para;
                if(NULL_PTR == msg)
                {
                    break;
                }

                UBaseType_t interruptstatus = taskENTER_CRITICAL_FROM_ISR();
                {
                    node = can_list_malloc();
                    if (NULL_PTR == node)
                    {
                        can_mgr_stat_add_droprx_msgs(ins, 1U);
                    }
                    else
                    {
                        memcpy(&node->msg, msg, sizeof(can_msg_t));
                        can_list_push(&can_context.items[ins].rx_list, node);
                    }
                }
                taskEXIT_CRITICAL_FROM_ISR(interruptstatus);
                if(NULL_PTR != can_module_handle)
                {
                    notify_value = CAN_RX_EVENT_RECEIVE << (CAN_TASK_EVENT_BITNUM+ins*CAN_RX_EVENT_BITNUM);
                    xTaskNotifyFromISR(can_module_handle, notify_value, eSetBits, NULL);
                }
            }
            break;

        case DRV_CAN_EVENT_TX_DONE:
            {
                if(NULL_PTR != can_tx_module_handle)
                {
                    notify_value = CAN_TX_EVENT_DONE << (CAN_TASK_EVENT_BITNUM+ins*CAN_TX_EVENT_BITNUM);
                    xTaskNotifyFromISR(can_tx_module_handle, notify_value, eSetBits, NULL);
                }
            }
            break;

        case DRV_CAN_EVENT_BUS_ERROR:
            {
                if(NULL_PTR != can_module_handle)
                {
                    notify_value = CAN_RX_EVENT_BUSERROR << (CAN_TASK_EVENT_BITNUM+ins*CAN_RX_EVENT_BITNUM);
                    xTaskNotifyFromISR(can_module_handle, notify_value, eSetBits, NULL);
                }
            }
            break;

        case DRV_CAN_EVENT_BUS_OFF:
            {
                UINT32 error_code = (UINT32)para;
                if(NULL_PTR != can_module_handle)
                {
                    if(error_code)
                    {
                        notify_value = CAN_RX_EVENT_BUSOFF << (CAN_TASK_EVENT_BITNUM+ins*CAN_RX_EVENT_BITNUM);
                    }
                    else
                    {
                        notify_value = CAN_RX_EVENT_BUSOK << (CAN_TASK_EVENT_BITNUM+ins*CAN_RX_EVENT_BITNUM);
                    }
                    xTaskNotifyFromISR(can_module_handle, notify_value, eSetBits, NULL);
                }
            }
            break;

        case DRV_CAN_EVENT_WAKEUP:
            break;

        default:
            break;
    }
}

/* 事件回调表管理 */
inline static VOID can_event_do_callback(CAN_EVENT event, UINT32 arg1, UINT32 arg2)
{
    for (UINT8 i = 0; i < CAN_CALLBACK_COUNT && can_event_cb_tbl[i]; i++) 
    {
        can_event_cb_tbl[i](event, arg1, arg2);
    }
}

static INT32 can_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            can_context.active_tick = 0U;
            can_context.bus_active = FALSE;
            can_context.bus_sleep = FALSE;
            for(UINT8 i = 0U; i < DRV_CAN_INS_COUNT; i++)
            {
                can_context.items[i].mode = DRV_CAN_MODE_NORMAL;
                can_context.items[i].tx_tick = 0U;
                can_list_init(&can_context.items[i].rx_list);
                can_list_init(&can_context.items[i].tx_list);
            }
            memset(can_event_cb_tbl, 0, sizeof(can_event_cb_tbl));
            GET_TBOX_MODULE_HANDLE(CAN, can_module_handle);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            can_check_timer = xTimerCreate(
                "CANCHECK",                      /* 定时器名称 */
                pdMS_TO_TICKS(CAN_CHECK_PERIOD), /* 50MS超时 */
                pdTRUE,                          /* 自动重载 */
                (VOID *)0,                       /* 定时器ID */
                can_check_timer_callback);       /* 回调函数 */
            if (NULL_PTR == can_check_timer) 
            {
                MODULE_LOG_E(CAN, "create bus timer failed");
                return (INT32)TBOX_E_FAILED_INIT;
            }

            can_buserr_init();
            can_busoff_init();
            can_mgr_init();
            can_shell_init();

            TBOX_ID can_module_id;
            GET_TBOX_MODULE_ID(CAN, can_module_id);
            tbox_module_set_state(can_module_id, TBOX_MODULE_STATE_START);
            
            xTimerStart(can_check_timer, 0U);

            MODULE_LOG_D(CAN, "can module initialized");
            break;
            
        default:
            break;
    }

    return (INT32)TBOX_E_OK;    
}

static VOID  can_start(VOID)
{
    taskENTER_CRITICAL();
    can_context.active_tick = 0U;
    can_context.bus_active = FALSE;
    can_context.bus_sleep = FALSE;
    taskEXIT_CRITICAL();
    
    can_mgr_init();

    if(pdFALSE == xTimerIsTimerActive(can_check_timer))
    {
        xTimerStart(can_check_timer, 0U);
    }
    
    can_event_do_callback(CAN_EVENT_WAKEUP, 0U, 0U);

    TBOX_ID can_module_id;
    GET_TBOX_MODULE_ID(CAN, can_module_id);   
    tbox_module_set_state(can_module_id, TBOX_MODULE_STATE_START);
    
    if(NULL_PTR != can_module_handle)
    {
        xTaskNotify(can_module_handle, CAN_TASK_EVENT_START, eSetBits);
    }

    MODULE_LOG_D(CAN, "can module started");    
}

static VOID  can_stop(VOID)
{
    if(NULL_PTR != can_module_handle)
    {
        ulTaskNotifyValueClear(can_module_handle, 0xFFFFFFFFU);
    }
    if (pdTRUE == xTimerIsTimerActive(can_check_timer)) 
    {
        xTimerStop(can_check_timer, 0U);
    }
    
    taskENTER_CRITICAL();
    can_context.active_tick = 0U;
    can_context.bus_active = FALSE;
    can_context.bus_sleep = TRUE;
    can_rx_clear_list();
    taskEXIT_CRITICAL();

    can_buserr_sleep();
    can_busoff_sleep();
    can_mgr_deinit();

    /* 触发休眠事件 */
    can_event_do_callback(CAN_EVENT_SLEEP, 0U, 0U);
    
    TBOX_ID can_module_id;
    GET_TBOX_MODULE_ID(CAN, can_module_id); 
    tbox_module_set_state(can_module_id, TBOX_MODULE_STATE_STOP);

    if(NULL_PTR != can_module_handle)
    {
        xTaskNotify(can_module_handle, CAN_TASK_EVENT_STOP, eSetBits);
    }
    
    MODULE_LOG_D(CAN, "can module stopped");
}

static VOID  can_exit(VOID)
{
    if(NULL_PTR != can_module_handle)
    {
        ulTaskNotifyValueClear(can_module_handle, 0xFFFFFFFFU);
    }
    if (pdTRUE == xTimerIsTimerActive(can_check_timer)) 
    {
        xTimerStop(can_check_timer, 0U);
    }
    
    taskENTER_CRITICAL();
    can_context.active_tick = 0U;
    can_context.bus_active = FALSE;
    can_context.bus_sleep = TRUE;
    can_rx_clear_list();
    taskEXIT_CRITICAL();

    can_buserr_sleep();
    can_busoff_sleep();
    can_mgr_deinit();

    if(NULL_PTR != can_module_handle)
    {
        xTaskNotify(can_module_handle, CAN_TASK_EVENT_EXIT, eSetBits);
    }
    
    MODULE_LOG_D(CAN, "can module exited");
}

static VOID  can_task(VOID *param)
{
    (VOID)param;

    UINT32 notify_value = 0;
    UINT32 event_bits;
    UINT8 int_id;
    
    for(;;) 
    {
        notify_value = 0;
        xTaskNotifyWait(0U, 0xFFFFFFFFU, &notify_value, portMAX_DELAY);

        if (notify_value & CAN_TASK_EVENT_EXIT)
        {
            break;
        }

        if(TRUE == can_context.bus_sleep)
        {
            continue;
        }

        if (notify_value & CAN_TASK_EVENT_CHECK)
        {
            can_rx_handle_check();
        }
        for(int_id = 0U; int_id < DRV_CAN_INS_COUNT; int_id++)
        {
            event_bits = notify_value;
            event_bits >>= (CAN_TASK_EVENT_BITNUM+int_id*CAN_RX_EVENT_BITNUM);
            if(event_bits & CAN_RX_EVENT_RECEIVE)
            {
                can_rx_handle_receive(int_id);
            }
            if(event_bits & CAN_RX_EVENT_BUSERROR)
            {
                can_rx_handle_buserror(int_id);
            }
            if(event_bits & CAN_RX_EVENT_BUSOFF)
            {
                can_rx_handle_busoff(int_id);
            }
            if(event_bits & CAN_RX_EVENT_BUSOK)
            {
                can_rx_handle_busok(int_id);
            }
        }
    }

    MODULE_LOG_F(CAN, "can task exited");

    if(NULL_PTR != can_check_timer)
    {
        xTimerDelete(can_check_timer, 0U);
    }
    if(NULL_PTR != can_module_handle)
    {
        can_module_handle = NULL_PTR;
    }
    vTaskDelete(NULL);
}

static INT32 can_tx_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            for(UINT8 i = 0U; i < DRV_CAN_INS_COUNT; i++)
            {
                can_context.items[i].tx_tick = 0U;
                can_list_init(&can_context.items[i].tx_list);
            }
            GET_TBOX_MODULE_HANDLE(CAN_TX, can_tx_module_handle);
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:
            {
              TBOX_ID can_tx_module_id;
              GET_TBOX_MODULE_ID(CAN_TX, can_tx_module_id); 
              tbox_module_set_state(can_tx_module_id, TBOX_MODULE_STATE_START);            
            }
            MODULE_LOG_D(CAN_TX, "CAN TX module initialized");
            break;
            
        default:
            break;
    }
    
    return (INT32)TBOX_E_OK;
}

static VOID  can_tx_start(VOID)
{
    taskENTER_CRITICAL();
    for(UINT8 i = 0U; i < DRV_CAN_INS_COUNT; i++)
    {
        can_context.items[i].tx_tick = 0U;
    }
    taskEXIT_CRITICAL();
    
    TBOX_ID can_tx_module_id;
    GET_TBOX_MODULE_ID(CAN_TX, can_tx_module_id);   
    tbox_module_set_state(can_tx_module_id, TBOX_MODULE_STATE_START);
    
    if(NULL_PTR != can_tx_module_handle)
    {
        xTaskNotify(can_tx_module_handle, CAN_TASK_EVENT_START, eSetBits);
    }

    MODULE_LOG_D(CAN_TX, "CAN TX module started");    
}

static VOID  can_tx_stop(VOID)
{
    if(NULL_PTR != can_tx_module_handle)
    {
        ulTaskNotifyValueClear(can_tx_module_handle, 0xFFFFFFFFU);
    }

    taskENTER_CRITICAL();
    can_tx_clear_list();
    taskEXIT_CRITICAL();

    TBOX_ID can_tx_module_id;
    GET_TBOX_MODULE_ID(CAN_TX, can_tx_module_id);       
    tbox_module_set_state(can_tx_module_id, TBOX_MODULE_STATE_STOP);

    if(NULL_PTR != can_tx_module_handle)
    {
        xTaskNotify(can_tx_module_handle, CAN_TASK_EVENT_STOP, eSetBits);
    }

    MODULE_LOG_D(CAN_TX, "CAN TX module stoped");   
}

static VOID  can_tx_exit(VOID)
{
    if(NULL_PTR != can_tx_module_handle)
    {
        ulTaskNotifyValueClear(can_tx_module_handle, 0xFFFFFFFFU);
    }

    taskENTER_CRITICAL();
    can_tx_clear_list();
    taskEXIT_CRITICAL();

    if(NULL_PTR != can_tx_module_handle)
    {
        xTaskNotify(can_tx_module_handle, CAN_TASK_EVENT_EXIT, eSetBits);
    } 
}

static VOID  can_tx_task(VOID *param)
{
    (VOID)param;

    UINT32 notify_value = 0;
    UINT32 event_bits;
    UINT8 int_id;
    
    for(;;) 
    {
        notify_value = 0;
        xTaskNotifyWait(0U, 0xFFFFFFFFU, &notify_value, portMAX_DELAY);

        if (notify_value & CAN_TASK_EVENT_EXIT)
        {
            break;
        }

        if(TRUE == can_context.bus_sleep)
        {
            continue;
        }

        if (notify_value & CAN_TASK_EVENT_CHECK)
        {
            can_tx_handle_check();
        }
        for(int_id = 0U; int_id < DRV_CAN_INS_COUNT; int_id++)
        {
            event_bits = notify_value;
            event_bits >>= (CAN_TASK_EVENT_BITNUM+int_id*CAN_TX_EVENT_BITNUM);
            if(event_bits & CAN_TX_EVENT_START)
            {
                can_tx_handle_start(int_id);
            }
            if(event_bits & CAN_TX_EVENT_DONE)
            {
                can_tx_handle_done(int_id);
            }
        }
    }

    MODULE_LOG_F(CAN_TX, "can tx task exited");

    if(NULL_PTR != can_tx_module_handle)
    {
        can_tx_module_handle = NULL_PTR;
    }
    vTaskDelete(NULL);
}
