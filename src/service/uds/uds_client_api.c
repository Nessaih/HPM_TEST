#include "uds_config.h"
#include "uds_client_api.h"
#include "uds_service.h"
#include "uds_if.h"
#include "tbox_common.h"
#include "tbox_log.h"
#include "iso14229/client.h"
#include "iso14229/uds.h"
#include "iso14229/tp/isotp_c.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>

typedef struct uds_session_internal_s {
    uint8_t can_node;                   /* CAN 节点（必须在第一个位置，供 adapter 访问） */
    bool in_use;                        /* 是否使用中 */
    uds_session_config_t config;        /* 会话配置 */
    UDSClient_t client;                 /* 开源协议栈客户端（内部已包含 recv_buf 和 send_buf） */
    UDSISOTpC_t tp;                     /* ISO-TP 传输层适配器（包含 UDSTp_t 接口） */
    SemaphoreHandle_t mutex;            /* 互斥锁 */
    SemaphoreHandle_t sync_sem;         /* 同步信号量（用于等待异步操作完成） */
    volatile UDSErr_t last_error;       /* 最后一次操作的错误码 */
    volatile bool operation_done;       /* 操作完成标志 */
} uds_session_internal_t;

static uds_session_internal_t g_sessions[MAX_UDS_SESSIONS];

static SemaphoreHandle_t g_sessions_mutex = NULL_PTR;

/**
 * @brief 同步等待宏
 * 
 * 用法：
 * UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
 *     err = UDSSendXXX(&session->client, ...);
 * } UDS_SYNC_WAIT_END(session, err);
 */
#define UDS_SYNC_WAIT_BEGIN(session, timeout_ms) \
    do { \
        xSemaphoreTake((session)->mutex, portMAX_DELAY); \
        (session)->operation_done = false; \
        (session)->last_error = UDS_OK;

#define UDS_SYNC_WAIT_END(session, send_err) \
        if ((send_err) != UDS_OK) { \
            xSemaphoreGive((session)->mutex); \
            return uds_convert_error((int)(send_err)); \
        } \
        xSemaphoreGive((session)->mutex); \
        TickType_t _timeout = pdMS_TO_TICKS(timeout_ms); \
        BaseType_t _result = xSemaphoreTake((session)->sync_sem, _timeout); \
        if (_result != pdTRUE) { \
            return UDSIF_ERR_TIMEOUT; \
        } \
        if ((session)->last_error != UDS_OK) { \
            return uds_convert_error((int)(session)->last_error); \
        } \
    } while(0)

/**
 * @brief UDS 客户端回调函数（同步机制实现）
 * 
 * @param client UDS 客户端指针
 * @param evt 事件类型
 * @param ev_data 事件数据
 * @return UDS_OK
 */
int uds_client_callback(UDSClient_t *client, UDSEvent_t evt, void *ev_data)
{
    uds_session_internal_t *session = NULL;
    for (int i = 0; i < MAX_UDS_SESSIONS; i++) {
        if (&g_sessions[i].client == client) {
            session = &g_sessions[i];
            break;
        }
    }
    
    if (session == NULL) {
        return UDS_OK;  /* 找不到会话，忽略事件 */
    }
    
    #if UDS_DEBUG_ENABLE
    if (evt != UDS_EVT_Poll) {
        const char *evt_name = "Unknown";
        switch (evt) {
            case UDS_EVT_Err:              evt_name = "Err"; break;
            case UDS_EVT_SendComplete:     evt_name = "SendComplete"; break;
            case UDS_EVT_ResponseReceived: evt_name = "ResponseReceived"; break;
            case UDS_EVT_Idle:             evt_name = "Idle"; break;
            default: break;
        }
        UDS_DEBUG_LOG("UDS event: %s (%d)", evt_name, evt);
    }
    #endif
    
    /* 处理事件 */
    switch (evt) {
        case UDS_EVT_Err:
            /* 错误事件：保存错误码，释放信号量 */
            if (ev_data != NULL) {
                session->last_error = *(UDSErr_t *)ev_data;
            } else {
                session->last_error = UDS_FAIL;  /* 未知错误 */
            }
            session->operation_done = true;
            if (session->sync_sem != NULL) {
                xSemaphoreGive(session->sync_sem);
            }
            break;
            
        case UDS_EVT_ResponseReceived:
            /* 响应接收完成：保存成功状态，释放信号量 */
            session->last_error = UDS_OK;
            session->operation_done = true;
            if (session->sync_sem != NULL) {
                xSemaphoreGive(session->sync_sem);
            }
            break;
            
        case UDS_EVT_Idle:
            /* 空闲事件：操作完全结束 */
            break;
            
        case UDS_EVT_SendComplete:
        case UDS_EVT_Poll:
        default:
            /* 其他事件：不需要处理 */
            break;
    }
    
    return UDS_OK;
}

/**
 * @brief 将开源库错误码转换为对外接口错误码
 * @param lib_err 开源库错误码 (UDSErr_t)
 * @return 对外接口错误码 (udsif_result_t)
 */
static int uds_convert_error(int lib_err)
{
    if (lib_err == UDS_OK) {
        return UDSIF_OK;
    }
    
    /* NRC 范围 (0x10-0x94)：值相同，直接返回 */
    if (lib_err >= 0x10 && lib_err <= 0x94) {
        return lib_err;
    }
    
    /* 协议栈错误：需要映射 */
    switch (lib_err) {
        case UDS_ERR_TIMEOUT:
            return UDSIF_ERR_TIMEOUT;
        case UDS_ERR_DID_MISMATCH:
            return UDSIF_ERR_DID_MISMATCH;
        case UDS_ERR_SID_MISMATCH:
            return UDSIF_ERR_SID_MISMATCH;
        case UDS_ERR_SUBFUNCTION_MISMATCH:
            return UDSIF_ERR_SUBFUNCTION_MISMATCH;
        case UDS_ERR_TPORT:
            return UDSIF_ERR_TPORT;
        case UDS_ERR_RESP_TOO_SHORT:
            return UDSIF_ERR_RESP_TOO_SHORT;
        case UDS_ERR_BUFSIZ:
            return UDSIF_ERR_BUFSIZ;
        case UDS_ERR_INVALID_ARG:
            return UDSIF_ERR_INVALID_ARG;
        case UDS_ERR_BUSY:
            return UDSIF_ERR_BUSY;
        case UDS_ERR_MISUSE:
            return UDSIF_ERR_MISUSE;
        default:
            /* 未知错误，返回一般错误 */
            return UDSIF_ERR_NEG_RESP;
    }
}

/**
 * @brief 初始化会话池
 */
static void uds_sessions_init(void)
{
    if (g_sessions_mutex == NULL_PTR) {
        g_sessions_mutex = xSemaphoreCreateMutex();
        memset(g_sessions, 0, sizeof(g_sessions));
    }
}

/**
 * @brief 分配会话
 */
static uds_session_internal_t* uds_session_alloc(void)
{
    uds_sessions_init();
    
    xSemaphoreTake(g_sessions_mutex, portMAX_DELAY);
    
    for (int i = 0; i < MAX_UDS_SESSIONS; i++) {
        if (!g_sessions[i].in_use) {
            g_sessions[i].in_use = true;
            xSemaphoreGive(g_sessions_mutex);
            return &g_sessions[i];
        }
    }
    
    xSemaphoreGive(g_sessions_mutex);
    return NULL_PTR;
}

/**
 * @brief 释放会话
 */
static void uds_session_free(uds_session_internal_t* session)
{
    if (session == NULL_PTR) {
        return;
    }
    
    xSemaphoreTake(g_sessions_mutex, portMAX_DELAY);
    session->in_use = false;
    xSemaphoreGive(g_sessions_mutex);
}

/**
 * @brief 句柄转会话指针
 */
static uds_session_internal_t* uds_handle_to_session(uds_handle_t handle)
{
    if (handle < 0 || handle >= MAX_UDS_SESSIONS) {
        return NULL_PTR;
    }
    
    uds_session_internal_t* session = &g_sessions[handle];
    return session->in_use ? session : NULL_PTR;
}

/**
 * @brief 会话指针转句柄
 */
static uds_handle_t uds_session_to_handle(uds_session_internal_t* session)
{
    if (session == NULL_PTR) {
        return UDS_INVALID_HANDLE;
    }
    
    return (uds_handle_t)(session - g_sessions);
}

/* ========== 会话管理接口 ========== */
uds_handle_t uds_if_session_create(const uds_session_config_t* config)
{
    uds_handle_t handle = UDS_INVALID_HANDLE;

    if (config == NULL_PTR) {
        MODULE_LOG_E(UDS, "config is NULL");
        return UDS_INVALID_HANDLE;
    }
    
    MODULE_LOG_I(UDS, "Creating session: node=%d, phys=0x%08X, func=0x%08X, resp=0x%08X",
                 config->can_node, config->phys_req_addr, config->func_req_addr, config->response_addr);
    
    /* 步骤 0: 资源初始化 */
    uds_session_internal_t* session = uds_session_alloc();
    if (session == NULL_PTR) {
        MODULE_LOG_E(UDS, "no free session");
        return UDS_INVALID_HANDLE;
    }
    
    memcpy(&session->config, config, sizeof(uds_session_config_t));
    session->can_node = config->can_node;

    session->mutex = xSemaphoreCreateMutex();
    if (session->mutex == NULL_PTR) {
        MODULE_LOG_E(UDS, "create mutex failed");
        uds_session_free(session);
        return UDS_INVALID_HANDLE;
    }
    
    session->sync_sem = xSemaphoreCreateBinary();
    if (session->sync_sem == NULL_PTR) {
        MODULE_LOG_E(UDS, "create sync semaphore failed");
        vSemaphoreDelete(session->mutex);
        uds_session_free(session);
        return UDS_INVALID_HANDLE;
    }
    
    session->last_error = UDS_OK;
    session->operation_done = false;
    
    /* 步骤 1: 初始化 UDS Client */
    UDSClientInit(&session->client);
   
    /* 步骤 2: 初始化 ISO-TP 传输层适配器 */
    UDSISOTpCConfig_t tp_config = {
        .source_addr = config->response_addr,
        .target_addr = config->phys_req_addr,
        .source_addr_func = config->response_addr,
        .target_addr_func = config->func_req_addr
    };
    
    UDSErr_t tp_err = UDSISOTpCInit(&session->tp, &tp_config);
    if (tp_err != UDS_OK) {
        MODULE_LOG_E(UDS, "ISO-TP init failed: %d", tp_err);
        vSemaphoreDelete(session->mutex);
        uds_session_free(session);
        return UDS_INVALID_HANDLE;
    }
    
    /* 步骤 3: 设置 ISO-TP 用户参数 */
    session->tp.phys_link.user_send_can_arg = session;
    session->tp.func_link.user_send_can_arg = session;

    /* 步骤 4: 设置 UDS Client 的传输层指针 */
    session->client.tp = &session->tp.hdl;
 
    /* 步骤 5: 设置回调函数 */  
    session->client.fn = uds_client_callback; 
    
    /* 步骤 6: 注册响应地址到 CAN ID 映射 */
    handle = uds_session_to_handle(session);
    int ret = uds_register_can_id(config->can_node, config->response_addr, handle);
    if (ret != 0) {
        MODULE_LOG_E(UDS, "register CAN ID failed: ret=%d", ret);
        vSemaphoreDelete(session->mutex);
        uds_session_free(session);
        return UDS_INVALID_HANDLE;
    }
  
    MODULE_LOG_I(UDS, "session created: handle=%d, node=%d", handle, config->can_node);

    return handle;
}

int32_t uds_if_session_destroy(uds_handle_t handle)
{
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        MODULE_LOG_E(UDS, "invalid handle: %d", handle);
        return -1;
    }
    
    /* 注销 CAN ID 映射 */
    uds_unregister_can_id(handle);
    
    /* 删除互斥锁 */
    if (session->mutex != NULL_PTR) {
        vSemaphoreDelete(session->mutex);
        session->mutex = NULL_PTR;
    }
    
    /* 删除同步信号量 */
    if (session->sync_sem != NULL_PTR) {
        vSemaphoreDelete(session->sync_sem);
        session->sync_sem = NULL_PTR;
    }
    
    /* 释放会话 */
    uds_session_free(session);
    
    MODULE_LOG_I(UDS, "session destroyed: handle=%d", handle);
    return 0;
}

UDSClient_t* uds_get_client(uds_handle_t handle)
{
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return NULL_PTR;
    }
    
    return &session->client;
}

void uds_client_poll_all(void)
{  
    for (int i = 0; i < MAX_UDS_SESSIONS; i++) {
        if (g_sessions[i].in_use) {
            UDSClientPoll(&g_sessions[i].client);
        }
    }
}

int uds_if_session_control(uds_handle_t handle,
                            uint8_t session_type,
                            uint32_t timeout_ms)
{
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_I(UDS, "session_control: handle=%d, type=%d, timeout=%u", 
                 handle, session_type, timeout_ms);
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendDiagSessCtrl(&session->client, session_type);
    } UDS_SYNC_WAIT_END(session, err);
    
    MODULE_LOG_I(UDS, "session control OK: type=%d", session_type);
    return UDSIF_OK;
}

int uds_if_ecu_reset(uds_handle_t handle,
                     uint8_t reset_type,
                     uint32_t timeout_ms)
{
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_I(UDS, "ecu_reset: handle=%d, type=%d, timeout=%u", 
                 handle, reset_type, timeout_ms);
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendECUReset(&session->client, reset_type);
    } UDS_SYNC_WAIT_END(session, err);
    
    MODULE_LOG_I(UDS, "ECU reset OK: type=%d", reset_type);
    return UDSIF_OK;
}

int uds_if_read_did(uds_handle_t handle,
                    uint16_t did,
                    uint8_t* data,
                    uint16_t data_buf_size,
                    uint16_t* data_len,
                    uint32_t timeout_ms)
{
    if (data == NULL_PTR || data_len == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_D(UDS, "read_did: handle=%d, did=0x%04X, timeout=%u", 
                 handle, did, timeout_ms);
    
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendRDBI(&session->client, &did, 1);
    } UDS_SYNC_WAIT_END(session, err);
    
    xSemaphoreTake(session->mutex, portMAX_DELAY);
    
    uint16_t min_len = 1 + 2;  /* SID + DID */
    if (session->client.recv_size < min_len) {
        xSemaphoreGive(session->mutex);
        MODULE_LOG_E(UDS, "read DID response too short: did=0x%04X, len=%d", 
                     did, session->client.recv_size);
        return UDSIF_ERR_RESP_TOO_SHORT;
    }
    
    /* 检查 SID */
    if (session->client.recv_buf[0] != 0x62) {
        xSemaphoreGive(session->mutex);
        MODULE_LOG_E(UDS, "read DID invalid SID: did=0x%04X, sid=0x%02X", 
                     did, session->client.recv_buf[0]);
        return UDSIF_ERR_SID_MISMATCH;
    }
    
    /* 检查 DID */
    uint16_t resp_did = (uint16_t)((session->client.recv_buf[1] << 8) | 
                                    session->client.recv_buf[2]);
    if (resp_did != did) {
        xSemaphoreGive(session->mutex);
        MODULE_LOG_E(UDS, "read DID mismatch: expect=0x%04X, got=0x%04X", 
                     did, resp_did);
        return UDSIF_ERR_DID_MISMATCH;
    }
    
    /* 提取数据 */
    uint16_t data_size = session->client.recv_size - min_len;
    if (data_size > data_buf_size) {
        data_size = data_buf_size;
    }
    memcpy(data, &session->client.recv_buf[min_len], data_size);
    *data_len = data_size;
    
    xSemaphoreGive(session->mutex);
    
    MODULE_LOG_D(UDS, "read DID OK: did=0x%04X, len=%d", did, *data_len);
    return UDSIF_OK;
}

int uds_if_write_did(uds_handle_t handle,
                     uint16_t did,
                     const uint8_t* data,
                     uint16_t data_len,
                     uint32_t timeout_ms)
{
    if (data == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_D(UDS, "write_did: handle=%d, did=0x%04X, len=%d, timeout=%u", 
                 handle, did, data_len, timeout_ms);
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendWDBI(&session->client, did, data, data_len);
    } UDS_SYNC_WAIT_END(session, err);
    
    MODULE_LOG_D(UDS, "write DID OK: did=0x%04X", did);
    return UDSIF_OK;
}

int uds_if_communication_control(uds_handle_t handle,
                                 uint8_t control_type,
                                 uint8_t communication_type,
                                 uint32_t timeout_ms)
{
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_I(UDS, "comm_control: handle=%d, ctrl=%d, comm=%d, timeout=%u", 
                 handle, control_type, communication_type, timeout_ms);
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendCommCtrl(&session->client, control_type, communication_type);
    } UDS_SYNC_WAIT_END(session, err);
    
    MODULE_LOG_I(UDS, "communication control OK: ctrl=%d, comm=%d", 
                 control_type, communication_type);
    return UDSIF_OK;
}

int uds_if_tester_present(uds_handle_t handle,
                          uint32_t timeout_ms)
{
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_D(UDS, "tester_present: handle=%d, timeout=%u", handle, timeout_ms);
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendTesterPresent(&session->client);
    } UDS_SYNC_WAIT_END(session, err);
    
    MODULE_LOG_D(UDS, "tester present OK");
    return UDSIF_OK;
}

int uds_if_read_dtc_information(uds_handle_t handle,
                                uint8_t sub_function,
                                const uint8_t* data,
                                uint16_t data_len,
                                uint8_t* response,
                                uint16_t response_buf_size,
                                uint16_t* response_len,
                                uint32_t timeout_ms)
{
    if (response == NULL_PTR || response_len == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_D(UDS, "read_dtc_info: handle=%d, sub=0x%02X, timeout=%u", 
                 handle, sub_function, timeout_ms);
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendReadDTCInformation(&session->client, sub_function, data, data_len);
    } UDS_SYNC_WAIT_END(session, err);
    
    /* 响应接收成功后，提取数据 */
    xSemaphoreTake(session->mutex, portMAX_DELAY);
    if (session->client.recv_size > 2) {
        uint16_t copy_len = session->client.recv_size - 2;
        if (copy_len > response_buf_size) {
            copy_len = response_buf_size;
        }
        memcpy(response, &session->client.recv_buf[2], copy_len);
        *response_len = copy_len;
        MODULE_LOG_D(UDS, "read DTC info OK: sub=0x%02X, len=%d", sub_function, copy_len);
    } else {
        *response_len = 0;
    }
    xSemaphoreGive(session->mutex);
    
    return UDSIF_OK;
}

int uds_if_security_access(uds_handle_t handle,
                           uint8_t level,
                           uds_key_calculator_t key_calculator,
                           uint32_t timeout_ms)
{
    if (key_calculator == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_I(UDS, "security_access: handle=%d, level=%d, timeout=%u", 
                 handle, level, timeout_ms);
    
    /* 步骤 1: 请求种子（同步等待） */
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendSecurityAccess(&session->client, level, NULL_PTR, 0);
    } UDS_SYNC_WAIT_END(session, err);
    
    /* 从响应中提取种子 */
    xSemaphoreTake(session->mutex, portMAX_DELAY);
    struct SecurityAccessResponse resp;
    err = UDSUnpackSecurityAccessResponse(&session->client, &resp);
    if (err != UDS_OK) {
        xSemaphoreGive(session->mutex);
        MODULE_LOG_E(UDS, "unpack seed failed: level=%d, err=%d", level, err);
        return uds_convert_error((int)err);
    }
    
    /* 步骤 2: 计算密钥 */
    uint8_t key[256];
    uint16_t key_len = 0;
    int ret = key_calculator(resp.securitySeed, resp.securitySeedLength, key, sizeof(key), &key_len);
    if (ret != 0) {
        xSemaphoreGive(session->mutex);
        MODULE_LOG_E(UDS, "calculate key failed: level=%d", level);
        return UDSIF_ERR_INVALID_ARG;
    }
    xSemaphoreGive(session->mutex);
    
    /* 步骤 3: 发送密钥（同步等待） */
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendSecurityAccess(&session->client, level + 1, key, key_len);
    } UDS_SYNC_WAIT_END(session, err);
    
    MODULE_LOG_I(UDS, "security access OK: level=%d", level);
    return UDSIF_OK;
}

int uds_if_routine_control(uds_handle_t handle,
                           uint8_t routine_type,
                           uint16_t routine_id,
                           const uint8_t* data,
                           uint16_t data_len,
                           uint8_t* response,
                           uint16_t response_buf_size,
                           uint16_t* response_len,
                           uint32_t timeout_ms)
{
    if (response == NULL_PTR || response_len == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_D(UDS, "routine_control: handle=%d, id=0x%04X, type=%d, timeout=%u", 
                 handle, routine_id, routine_type, timeout_ms);
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendRoutineCtrl(&session->client, routine_type, routine_id, data, data_len);
    } UDS_SYNC_WAIT_END(session, err);
    
    /* 响应接收成功后，解包数据 */
    xSemaphoreTake(session->mutex, portMAX_DELAY);
    struct RoutineControlResponse resp;
    err = UDSUnpackRoutineControlResponse(&session->client, &resp);
    if (err == UDS_OK) {
        uint16_t copy_len = (resp.routineStatusRecordLength < response_buf_size) ? 
                            resp.routineStatusRecordLength : response_buf_size;
        memcpy(response, resp.routineStatusRecord, copy_len);
        *response_len = copy_len;
        MODULE_LOG_D(UDS, "routine control OK: id=0x%04X, type=%d", routine_id, routine_type);
    } else {
        MODULE_LOG_E(UDS, "unpack routine control failed: id=0x%04X, err=%d", routine_id, err);
    }
    xSemaphoreGive(session->mutex);
    
    return uds_convert_error((int)err);
}

int uds_if_request_download(uds_handle_t handle,
                            uint32_t memory_address,
                            uint32_t memory_size,
                            uint8_t data_format,
                            uint16_t* max_block_length,
                            uint32_t timeout_ms)
{
    if (max_block_length == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_I(UDS, "request_download: handle=%d, addr=0x%08X, size=%u, timeout=%u", 
                 handle, memory_address, memory_size, timeout_ms);
    
    /* addressAndLengthFormatIdentifier: 高4位=地址长度，低4位=大小长度 */
    uint8_t addr_len_format = 0x44;  /* 4字节地址 + 4字节大小 */
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendRequestDownload(&session->client, data_format, addr_len_format,
                                     (size_t)memory_address, (size_t)memory_size);
    } UDS_SYNC_WAIT_END(session, err);
    
    /* 响应接收成功后，解包获取最大块长度 */
    xSemaphoreTake(session->mutex, portMAX_DELAY);
    struct RequestDownloadResponse resp;
    err = UDSUnpackRequestDownloadResponse(&session->client, &resp);
    if (err == UDS_OK) {
        *max_block_length = (uint16_t)resp.maxNumberOfBlockLength;
        MODULE_LOG_I(UDS, "request download OK: addr=0x%08X, size=%u, max_block=%d",
                     memory_address, memory_size, *max_block_length);
    } else {
        MODULE_LOG_E(UDS, "unpack request download failed: addr=0x%08X, err=%d", 
                     memory_address, err);
    }
    xSemaphoreGive(session->mutex);
    
    return uds_convert_error((int)err);
}

int uds_if_transfer_data(uds_handle_t handle,
                         uint8_t block_sequence,
                         const uint8_t* data,
                         uint16_t data_len,
                         uint32_t timeout_ms)
{
    if (data == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_D(UDS, "transfer_data: handle=%d, seq=%d, len=%d, timeout=%u", 
                 handle, block_sequence, data_len, timeout_ms);
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendTransferData(&session->client, block_sequence, 
                                  data_len, data, data_len);
    } UDS_SYNC_WAIT_END(session, err);
    
    MODULE_LOG_D(UDS, "transfer data OK: seq=%d", block_sequence);
    return UDSIF_OK;
}

int uds_if_request_transfer_exit(uds_handle_t handle,
                                 uint32_t timeout_ms)
{
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDS_ERR_INVALID_ARG;
    }
    
    MODULE_LOG_I(UDS, "request_transfer_exit: handle=%d, timeout=%u", handle, timeout_ms);
    
    UDSErr_t err;
    UDS_SYNC_WAIT_BEGIN(session, timeout_ms) {
        err = UDSSendRequestTransferExit(&session->client);
    } UDS_SYNC_WAIT_END(session, err);
    
    MODULE_LOG_I(UDS, "transfer exit OK");
    return UDSIF_OK;
}
/* ========== 调试和信息接口 ========== */

/**
 * @brief 获取会话信息
 * @param handle 会话句柄
 * @param info 会话信息结构体指针
 * @return 0=成功，其他=失败
 */
int uds_if_get_session_info(uds_handle_t handle, uds_session_info_t* info)
{
    if (info == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    uds_session_internal_t* session = uds_handle_to_session(handle);
    if (session == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    xSemaphoreTake(session->mutex, portMAX_DELAY);
    
    info->handle = handle;
    info->in_use = session->in_use;
    info->can_node = session->can_node;
    memcpy(&info->config, &session->config, sizeof(uds_session_config_t));
    info->client_state = session->client.state;
    info->p2_ms = session->client.p2_ms;
    info->p2_star_ms = session->client.p2_star_ms;
    info->recv_size = session->client.recv_size;
    info->send_size = session->client.send_size;
    info->operation_done = session->operation_done;
    info->last_error = session->last_error;
    info->tp_phys_sa = session->tp.phys_sa;
    info->tp_phys_ta = session->tp.phys_ta;
    info->tp_func_sa = session->tp.func_sa;
    info->tp_func_ta = session->tp.func_ta;
    
    xSemaphoreGive(session->mutex);
    
    return UDSIF_OK;
}

/**
 * @brief 获取所有会话的概览信息
 * @param sessions 会话概览数组
 * @param max_sessions 数组最大长度
 * @param actual_count 实际会话数量
 * @return 0=成功，其他=失败
 */
int uds_if_get_sessions_overview(uds_session_overview_t* sessions, 
                                 uint8_t max_sessions, 
                                 uint8_t* actual_count)
{
    if (sessions == NULL_PTR || actual_count == NULL_PTR) {
        return UDSIF_ERR_INVALID_ARG;
    }
    
    *actual_count = 0;
    
    for (int i = 0; i < MAX_UDS_SESSIONS && *actual_count < max_sessions; i++) {
        if (g_sessions[i].in_use) {
            sessions[*actual_count].handle = i;
            sessions[*actual_count].can_node = g_sessions[i].can_node;
            sessions[*actual_count].phys_req_addr = g_sessions[i].config.phys_req_addr;
            sessions[*actual_count].func_req_addr = g_sessions[i].config.func_req_addr;
            sessions[*actual_count].response_addr = g_sessions[i].config.response_addr;
            sessions[*actual_count].client_state = g_sessions[i].client.state;
            (*actual_count)++;
        }
    }
    
    return UDSIF_OK;
}