#include "uds_shell.h"
#include "uds_if.h"
#include "tbox_common.h"
#include "tbox_log.h"
#include "tbox_shell_if.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 全局测试会话句柄 */
static uds_handle_t g_test_handle = UDS_INVALID_HANDLE;

/**
 * @brief 创建测试会话
 * 用法: udscreate <can_node> <phys_req> <func_req> <response>
 * 
 * 说明:
 * - can_node: CAN 节点编号，1-3（1=CAN1, 2=CAN2, 3=CAN3）
 * - 标准帧 ID: 直接使用 11 位 ID（如 0x7E0）
 * - 扩展帧 ID: 需要添加 0x80000000 标志（如 0x80000700）
 */
static BaseType_t cmd_uds_create(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[32];
    
    /* 获取参数 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: udscreate <can_node> <phys_req> <func_req> <response>\r\n"
                             "  can_node: 1=CAN1, 2=CAN2, 3=CAN3\r\n"
                             "  CAN IDs are parsed as hex (0x prefix optional)\r\n"
                             "  Standard frame: udscreate 1 7E0 7DF 7E8\r\n"
                             "  Extended frame: udscreate 1 80000700 800007DF 80000708\r\n");
        return pdFALSE;
    }
    
    uds_session_config_t config;
    
    /* CAN 节点 */
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    uint8_t can_node_input = (uint8_t)atoi(param_str);
    
    if (can_node_input < 1 || can_node_input > DRV_CAN_INS_COUNT) {
        snprintf(buf, bufsz, "Error: can_node must be 1-%u\r\n", DRV_CAN_INS_COUNT);
        return pdFALSE;
    }
    config.can_node = can_node_input - 1;
    
    /* 物理请求地址 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param_len);
    if (NULL_PTR == param_ptr) {
        snprintf(buf, bufsz, "Error: missing phys_req parameter\r\n");
        return pdFALSE;
    }
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    config.phys_req_addr = (uint32_t)strtoul(param_str, NULL_PTR, 16);
    
    /* 功能请求地址 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 3, &param_len);
    if (NULL_PTR == param_ptr) {
        snprintf(buf, bufsz, "Error: missing func_req parameter\r\n");
        return pdFALSE;
    }
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    config.func_req_addr = (uint32_t)strtoul(param_str, NULL_PTR, 16);
    
    /* 响应地址 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 4, &param_len);
    if (NULL_PTR == param_ptr) {
        snprintf(buf, bufsz, "Error: missing response parameter\r\n");
        return pdFALSE;
    }
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    config.response_addr = (uint32_t)strtoul(param_str, NULL_PTR, 16);
    
    config.timeout_ms = 5000;
    
    g_test_handle = uds_if_session_create(&config);
    if (g_test_handle == UDS_INVALID_HANDLE) {
        snprintf(buf, bufsz, "Failed to create session\r\n");
        return pdFALSE;
    }
    
    snprintf(buf, bufsz, "Session created: handle=%d\r\n", g_test_handle);
    return pdFALSE;
}

/**
 * @brief 销毁测试会话
 * 用法: udsdestroy
 */
static BaseType_t cmd_uds_destroy(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    (void)cmd;
    
    if (g_test_handle == UDS_INVALID_HANDLE) {
        snprintf(buf, bufsz, "No active session\r\n");
        return pdFALSE;
    }
    
    int ret = uds_if_session_destroy(g_test_handle);
    if (ret == 0) {
        snprintf(buf, bufsz, "Session destroyed\r\n");
        g_test_handle = UDS_INVALID_HANDLE;
    } else {
        snprintf(buf, bufsz, "Failed to destroy session\r\n");
    }
    
    return pdFALSE;
}

/**
 * @brief 会话控制
 * 用法: udssession <type>
 */
static BaseType_t cmd_uds_session(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[32];
    
    if (g_test_handle == UDS_INVALID_HANDLE) {
        snprintf(buf, bufsz, "No active session\r\n");
        return pdFALSE;
    }
    
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: udssession <type>\r\n"
                             "  type: 1=default, 2=programming, 3=extended\r\n");
        return pdFALSE;
    }
    
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    uint8_t session_type = (uint8_t)atoi(param_str);
    
    int result = uds_if_session_control(g_test_handle, session_type, 5000);
    
    snprintf(buf, bufsz, "Session control: type=%d, result=%d\r\n", session_type, result);
    return pdFALSE;
}

/**
 * @brief 读取 DID
 * 用法: udsread <did>
 */
static BaseType_t cmd_uds_read(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[32];
    INT32       len = 0;
    
    if (g_test_handle == UDS_INVALID_HANDLE) {
        snprintf(buf, bufsz, "No active session\r\n");
        return pdFALSE;
    }
    
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: udsread <did>\r\n"
                             "  DID is parsed as hex (0x prefix optional)\r\n"
                             "  Example: udsread F190\r\n");
        return pdFALSE;
    }
    
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    uint16_t did = (uint16_t)strtoul(param_str, NULL_PTR, 16);
    
    uint8_t data[256];
    uint16_t data_len = 0;
    
    int result = uds_if_read_did(g_test_handle, did, data, sizeof(data), &data_len, 5000);
    
    if (result == UDSIF_OK) {
        len += snprintf(buf + len, bufsz - len, "Read DID 0x%04X: len=%d\r\nData: ", did, data_len);
        for (int i = 0; i < data_len && i < 32; i++) {
            len += snprintf(buf + len, bufsz - len, "%02X ", data[i]);
        }
        len += snprintf(buf + len, bufsz - len, "\r\n");
    } else {
        snprintf(buf, bufsz, "Read DID failed: result=%d\r\n", result);
    }
    
    return pdFALSE;
}

/**
 * @brief 写入 DID
 * 用法: udswrite <did> <data...>
 */
static BaseType_t cmd_uds_write(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[32];
    uint8_t     data[256];
    uint16_t    data_len = 0;
    
    if (g_test_handle == UDS_INVALID_HANDLE) {
        snprintf(buf, bufsz, "No active session\r\n");
        return pdFALSE;
    }
    
    /* 获取 DID */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: udswrite <did> <data...>\r\n"
                             "  DID and data are parsed as hex (0x prefix optional)\r\n"
                             "  Example: udswrite F190 01 02 03\r\n");
        return pdFALSE;
    }
    
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    uint16_t did = (uint16_t)strtoul(param_str, NULL_PTR, 16);
    
    /* 获取数据 */
    for (INT32 i = 2; i <= 256; i++) {
        if (data_len >= sizeof(data)) {
            break;
        }
        param_ptr = FreeRTOS_CLIGetParameter(cmd, i, &param_len);
        if (NULL_PTR == param_ptr || param_len == 0) {
            break;
        }
        param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
        strncpy(param_str, param_ptr, param_len);
        param_str[param_len] = '\0';
        data[data_len++] = (uint8_t)strtoul(param_str, NULL_PTR, 16);
    }
    
    if (data_len == 0) {
        snprintf(buf, bufsz, "Error: no data provided\r\n");
        return pdFALSE;
    }
    
    int result = uds_if_write_did(g_test_handle, did, data, data_len, 5000);
    
    snprintf(buf, bufsz, "Write DID 0x%04X: result=%d\r\n", did, result);
    return pdFALSE;
}

/**
 * @brief ECU 复位
 * 用法: udsreset <type>
 */
static BaseType_t cmd_uds_reset(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[32];
    
    if (g_test_handle == UDS_INVALID_HANDLE) {
        snprintf(buf, bufsz, "No active session\r\n");
        return pdFALSE;
    }
    
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: udsreset <type>\r\n"
                             "  type: 1=hard, 2=soft\r\n");
        return pdFALSE;
    }
    
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    uint8_t reset_type = (uint8_t)atoi(param_str);
    
    int result = uds_if_ecu_reset(g_test_handle, reset_type, 5000);
    
    snprintf(buf, bufsz, "ECU reset: type=%d, result=%d\r\n", reset_type, result);
    return pdFALSE;
}

/**
 * @brief 显示 UDS 会话信息
 * 用法: showuds [handle]
 */
static BaseType_t cmd_uds_show(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[32];
    INT32       len = 0;
    
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    
    if (param_ptr != NULL_PTR && param_len > 0) {
        /* 显示指定会话的详细信息 */
        param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
        strncpy(param_str, param_ptr, param_len);
        param_str[param_len] = '\0';
        uds_handle_t handle = (uds_handle_t)atoi(param_str);
        
        uds_session_info_t info;
        int ret = uds_if_get_session_info(handle, &info);
        
        if (ret != UDSIF_OK) {
            snprintf(buf, bufsz, "Error: failed to get session info for handle %d (ret=%d)\r\n", handle, ret);
            return pdFALSE;
        }
        
        const char* state_names[] = {"Idle", "Sending", "AwaitSendComplete", "AwaitResponse"};
        const char* state_name = (info.client_state < 4) ? state_names[info.client_state] : "Unknown";
        
        len += snprintf(buf + len, bufsz - len, "=== UDS Session %d Details ===\r\n", handle);
        len += snprintf(buf + len, bufsz - len, "Status      : %s\r\n", info.in_use ? "Active" : "Inactive");
        len += snprintf(buf + len, bufsz - len, "CAN Node    : %d (CAN%d)\r\n", info.can_node, info.can_node + 1);
        len += snprintf(buf + len, bufsz - len, "Phys Req    : 0x%08X\r\n", info.config.phys_req_addr);
        len += snprintf(buf + len, bufsz - len, "Func Req    : 0x%08X\r\n", info.config.func_req_addr);
        len += snprintf(buf + len, bufsz - len, "Response    : 0x%08X\r\n", info.config.response_addr);
        len += snprintf(buf + len, bufsz - len, "Timeout     : %u ms\r\n", info.config.timeout_ms);
        len += snprintf(buf + len, bufsz - len, "Client State: %s (%d)\r\n", state_name, info.client_state);
        len += snprintf(buf + len, bufsz - len, "P2 Timeout  : %u ms\r\n", info.p2_ms);
        len += snprintf(buf + len, bufsz - len, "P2* Timeout : %u ms\r\n", info.p2_star_ms);
        len += snprintf(buf + len, bufsz - len, "Last Error  : 0x%08X\r\n", info.last_error);
        len += snprintf(buf + len, bufsz - len, "Recv Size   : %u bytes\r\n", info.recv_size);
        len += snprintf(buf + len, bufsz - len, "Send Size   : %u bytes\r\n", info.send_size);
        len += snprintf(buf + len, bufsz - len, "TP Phys SA  : 0x%08X\r\n", info.tp_phys_sa);
        len += snprintf(buf + len, bufsz - len, "TP Phys TA  : 0x%08X\r\n", info.tp_phys_ta);
        len += snprintf(buf + len, bufsz - len, "TP Func SA  : 0x%08X\r\n", info.tp_func_sa);
        len += snprintf(buf + len, bufsz - len, "TP Func TA  : 0x%08X\r\n", info.tp_func_ta);
        
    } else {
        /* 显示所有会话的概览信息 */
        uds_session_overview_t sessions[4];
        uint8_t session_count = 0;
        
        int ret = uds_if_get_sessions_overview(sessions, sizeof(sessions)/sizeof(sessions[0]), &session_count);
        
        if (ret != UDSIF_OK) {
            snprintf(buf, bufsz, "Error: failed to get sessions overview (ret=%d)\r\n", ret);
            return pdFALSE;
        }
        
        len += snprintf(buf + len, bufsz - len, "=== UDS Sessions Overview ===\r\n");
        
        if (session_count == 0) {
            len += snprintf(buf + len, bufsz - len, "No active sessions\r\n");
        } else {
            len += snprintf(buf + len, bufsz - len, "Handle | Node | Phys Req | Func Req | Response | State\r\n");
            len += snprintf(buf + len, bufsz - len, "-------|------|----------|----------|----------|------\r\n");
            
            for (uint8_t i = 0; i < session_count; i++) {
                const char* state_names[] = {"Idle", "Send", "Wait", "Resp"};
                const char* state_name = (sessions[i].client_state < 4) ? state_names[sessions[i].client_state] : "Unkn";
                
                len += snprintf(buf + len, bufsz - len, "  %2d   |  %d   | %08X | %08X | %08X | %s\r\n",
                               sessions[i].handle,
                               sessions[i].can_node + 1,
                               sessions[i].phys_req_addr,
                               sessions[i].func_req_addr,
                               sessions[i].response_addr,
                               state_name);
            }
        }
        
        /* 显示当前测试会话 */
        if (g_test_handle != UDS_INVALID_HANDLE) {
            len += snprintf(buf + len, bufsz - len, "\r\nCurrent test session: %d\r\n", g_test_handle);
        } else {
            len += snprintf(buf + len, bufsz - len, "\r\nNo current test session\r\n");
        }
        
        len += snprintf(buf + len, bufsz - len, "\r\nUsage: showuds [handle] - show details for specific session\r\n");
    }
    
    return pdFALSE;
}

TBOX_SHELL_DEFINE(udscreate, "Create UDS session: udscreate <can_node> <phys_req> <func_req> <response> (node: 1-3)", 4, cmd_uds_create);
TBOX_SHELL_DEFINE(udsdestroy, "Destroy UDS session", 0, cmd_uds_destroy);
TBOX_SHELL_DEFINE(udssession, "Session control: udssession <type>", 1, cmd_uds_session);
TBOX_SHELL_DEFINE(udsread, "Read DID: udsread <did>", 1, cmd_uds_read);
TBOX_SHELL_DEFINE(udswrite, "Write DID: udswrite <did> <data...>", -1, cmd_uds_write);
TBOX_SHELL_DEFINE(udsreset, "ECU reset: udsreset <type>", 1, cmd_uds_reset);
TBOX_SHELL_DEFINE(showuds, "Show UDS sessions: showuds [handle]", -1, cmd_uds_show);

VOID uds_shell_init(VOID)
{
    TBOX_SHELL_REGISTER(udscreate);
    TBOX_SHELL_REGISTER(udsdestroy);
    TBOX_SHELL_REGISTER(udssession);
    TBOX_SHELL_REGISTER(udsread);
    TBOX_SHELL_REGISTER(udswrite);
    TBOX_SHELL_REGISTER(udsreset);
    TBOX_SHELL_REGISTER(showuds);
}
