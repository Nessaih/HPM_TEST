#include "uds_config.h"
#include "tbox_common.h"
#include "tbox_core.h"
#include "can_if.h"
#include "api_rtos.h"
#include "uds_service.h"
#include "uds_client_api.h"
#include "uds_shell.h"
#include "iso14229/client.h"
#include "iso14229/tp/isotp_c.h"
#include "isotp.h"
#include <string.h>

typedef struct {
    uint8_t can_node;          /* CAN 节点（0, 1, 2） */
    uint32_t can_id;           /* CAN ID（包含扩展帧标志） */
    uds_handle_t handle;       /* 会话句柄 */
    bool in_use;               /* 是否使用中 */
} uds_can_id_map_t;

static uds_can_id_map_t g_can_id_map[MAX_UDS_SESSIONS];
static SemaphoreHandle_t g_can_id_map_mutex = NULL_PTR;

static INT32 uds_svr_init(UINT8 seq);
static VOID  uds_svr_stop(VOID);
static VOID  uds_svr_start(VOID);
static VOID  uds_svr_exit(VOID);
static VOID  uds_svr_task(VOID *param);
static INT32 uds_svr_can_handler(CAN_EVENT event, UINT32 arg1, UINT32 arg2);

TBOX_MODULE_FUN(UDS, uds_svr_init, uds_svr_stop, uds_svr_start, NULL_PTR, uds_svr_exit, NULL_PTR);
TBOX_RUNLOOP_MODULE(UDS, TBOX_TASK_PRIORITY_LOW2, LOG_LEVEL_ERROR, TBOX_TASK_MEDIUM_STACK_SIZE, uds_svr_task);
TBOX_MODULE_LOADER(UDS)
{
}

VOID uds_receive(UINT8 ins, const can_msg_t* msg);

static TBOX_ID       uds_svr_module_id;

static INT32 uds_svr_init(UINT8 seq)
{
    INT32 ret = (INT32)TBOX_E_OK;

    switch (seq)
    {
    case MODULE_INIT_SEQ_OS:
        GET_TBOX_MODULE_ID(UDS, uds_svr_module_id);
        break;
    case MODULE_INIT_SEQ_STORAGE:
        break;
    case MODULE_INIT_SEQ_MODULE:
        memset(g_can_id_map, 0, sizeof(g_can_id_map));
        g_can_id_map_mutex = xSemaphoreCreateMutex();
        can_if_reg_cb(uds_svr_can_handler);
        uds_shell_init();

        MODULE_LOG_I(UDS, "UDS module initialized");
        break;
    default:
        break;
    }

    return ret;
}

static VOID uds_svr_stop(VOID)
{
    tbox_module_set_state(uds_svr_module_id, TBOX_MODULE_STATE_STOP);
}

static VOID uds_svr_start(VOID)
{
    tbox_module_set_state(uds_svr_module_id, TBOX_MODULE_STATE_START);
}

static VOID uds_svr_exit(VOID)
{
}

static VOID uds_svr_task(VOID *param)
{
    TickType_t last_wake = xTaskGetTickCount();

    for (;;)
    {
        if(tbox_module_get_state(uds_svr_module_id) != TBOX_MODULE_STATE_START)
        {
            vTaskDelay(pdMS_TO_TICKS(100));  // 模块未启动时休眠
            continue;
        }

        uds_client_poll_all();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}

static INT32 uds_svr_can_handler(CAN_EVENT event, UINT32 arg1, UINT32 arg2)
{
    switch (event)
    {
    case CAN_EVENT_DATAIN:
    {
        can_msg_t *msg   = (can_msg_t *)arg1;
        UINT32     count = arg2;

        for (UINT32 i = 0; i < count; i++)
        {
            uds_receive(msg[i].ins, &msg[i]);
        }
        break;
    }

    default:
        break;
    }

    return 0;
}

VOID uds_receive(UINT8 ins, const can_msg_t* msg)
{
    if (msg == NULL_PTR) {
        return;
    }
    
    uint32_t can_id = msg->id;
    uint8_t can_node = ins;
    
    if (g_can_id_map_mutex != NULL_PTR) {
        xSemaphoreTake(g_can_id_map_mutex, portMAX_DELAY);
    }
    
    for (int i = 0; i < MAX_UDS_SESSIONS; i++) {
        if (g_can_id_map[i].in_use && 
            g_can_id_map[i].can_node == can_node &&
            g_can_id_map[i].can_id == can_id) {
            UDSClient_t* client = uds_get_client(g_can_id_map[i].handle);
            if (client != NULL_PTR && client->tp != NULL_PTR) {
                UDSISOTpC_t* tp = (UDSISOTpC_t*)client->tp;
                isotp_on_can_message(&tp->phys_link, msg->data, msg->len);
            }
            
            if (g_can_id_map_mutex != NULL_PTR) {
                xSemaphoreGive(g_can_id_map_mutex);
            }
            return;
        }
    }
    
    if (g_can_id_map_mutex != NULL_PTR) {
        xSemaphoreGive(g_can_id_map_mutex);
    }
    
    /* 未找到对应的会话，忽略 */
}

INT32 uds_register_can_id(uint8_t can_node, uint32_t can_id, uds_handle_t handle)
{
    if (g_can_id_map_mutex != NULL_PTR) {
        xSemaphoreTake(g_can_id_map_mutex, portMAX_DELAY);
    }
    
    /* 检查是否已存在相同的 CAN 节点 + CAN ID 组合 */
    for (int i = 0; i < MAX_UDS_SESSIONS; i++) {
        if (g_can_id_map[i].in_use && 
            g_can_id_map[i].can_node == can_node &&
            g_can_id_map[i].can_id == can_id) {
            if (g_can_id_map_mutex != NULL_PTR) {
                xSemaphoreGive(g_can_id_map_mutex);
            }
            MODULE_LOG_E(UDS, "CAN ID conflict: node=%d, id=0x%08X already registered", 
                         can_node, can_id);
            return -2;  /* CAN ID 冲突 */
        }
    }
    
    /* 查找空闲位置 */
    for (int i = 0; i < MAX_UDS_SESSIONS; i++) {
        if (!g_can_id_map[i].in_use) {
            g_can_id_map[i].can_node = can_node;
            g_can_id_map[i].can_id = can_id;
            g_can_id_map[i].handle = handle;
            g_can_id_map[i].in_use = TRUE;
            MODULE_LOG_D(UDS, "register CAN ID: node=%d, id=0x%08X -> handle %d", 
                         can_node, can_id, handle);
            
            if (g_can_id_map_mutex != NULL_PTR) {
                xSemaphoreGive(g_can_id_map_mutex);
            }
            return 0;
        }
    }
    
    if (g_can_id_map_mutex != NULL_PTR) {
        xSemaphoreGive(g_can_id_map_mutex);
    }
    
    MODULE_LOG_E(UDS, "CAN ID map full");
    return -1;
}

VOID uds_unregister_can_id(uds_handle_t handle)
{
    if (g_can_id_map_mutex != NULL_PTR) {
        xSemaphoreTake(g_can_id_map_mutex, portMAX_DELAY);
    }
    
    for (int i = 0; i < MAX_UDS_SESSIONS; i++) {
        if (g_can_id_map[i].in_use && g_can_id_map[i].handle == handle) {
            MODULE_LOG_D(UDS, "unregister CAN ID: node=%d, id=0x%08X", 
                         g_can_id_map[i].can_node, g_can_id_map[i].can_id);
            g_can_id_map[i].in_use = FALSE;
            
            if (g_can_id_map_mutex != NULL_PTR) {
                xSemaphoreGive(g_can_id_map_mutex);
            }
            return;
        }
    }
    
    if (g_can_id_map_mutex != NULL_PTR) {
        xSemaphoreGive(g_can_id_map_mutex);
    }
}
