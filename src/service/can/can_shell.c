#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "tbox_common.h"
#include "tbox_log.h"
#include "tbox_shell_if.h"
#include "can_shell.h"
#include "can_if.h"
#include "can_mgr.h"
#include "can_types.h"
#include "tbox_cfg_if.h"
#include "drv_can.h"

/* Shell 命令：显示 CAN 状态和统计信息 */
static BaseType_t show_can_cmd(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    UINT8 i;
    UINT8 state;
    const CHAR *state_str[] = {"NOUSED", "IDLE", "BUSY", "ERROR", "OFF"};
    INT32 len = 0;
    
    (VOID)cmd;
    
    len += snprintf(buf + len, bufsz - len, "\r\n=== CAN Status ===\r\n");
    
    for (i = 0; i < DRV_CAN_INS_COUNT; i++) {
        state = can_if_state_get(i);
        if (state < sizeof(state_str)/sizeof(state_str[0])) {
            len += snprintf(buf + len, bufsz - len, "CAN%u: state=%s, baudrate=%lu kbps, recv=%lu, send=%lu\r\n",
                      i + 1, 
                      state_str[state],
                      can_mgr_get_baudrate(i),
                      can_mgr_stat_get_recv_count(i), 
                      can_mgr_stat_get_send_count(i));
        } else {
            len += snprintf(buf + len, bufsz - len, "CAN%u: state=UNKNOWN, baudrate=%lu kbps, recv=%lu, send=%lu\r\n",
                      i + 1,
                      can_mgr_get_baudrate(i),
                      can_mgr_stat_get_recv_count(i), 
                      can_mgr_stat_get_send_count(i));
        }
    }
    
    len += snprintf(buf + len, bufsz - len, "====================\r\n");
    
    return pdFALSE;
}

/* Shell 命令：显示最近接收的 CAN 消息日志 */
static BaseType_t can_log_cmd(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    UINT8 i, j;
    INT32 len = 0;
    UINT8 msg_count;
    UINT8 display_count;
    
    (VOID)cmd;
    
    msg_count = can_mgr_stat_get_last_msg_count();
    display_count = (msg_count > 5) ? 5 : msg_count;  /* 最多显示5条消息 */
    
    len += snprintf(buf + len, bufsz - len, "\r\n=== CAN Message Log (Last %u) ===\r\n", display_count);
    
    if (msg_count == 0) {
        len += snprintf(buf + len, bufsz - len, "No messages received yet.\r\n");
    } else {
        for (i = 0; i < display_count; i++) {
            can_msg_t *msg = can_mgr_stat_get_last_msg(i);
            if (msg != NULL_PTR) {
                len += snprintf(buf + len, bufsz - len, "[%02u] CAN%u ID=0x%08lX Len=%u Data=", 
                          i + 1, msg->ins + 1, msg->id, msg->len);
                
                for (j = 0; j < msg->len && j < 8; j++) {
                    len += snprintf(buf + len, bufsz - len, "%02X ", msg->data[j]);
                }
                
                len += snprintf(buf + len, bufsz - len, "\r\n");
            }
        }
    }
    
    len += snprintf(buf + len, bufsz - len, "================================\r\n");
    
    return pdFALSE;
}

/* Shell 命令：发送 CAN 数据 */
static BaseType_t can_send_cmd(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[64];
    UINT8       port = 0;
    UINT32      id = 0;
    UINT8       data_len = 0;
    UINT8       data[8] = {0};
    can_msg_t   msg = {0};
    INT32       ret;
    INT32       i;
    
    /* 格式: cansend <port> <id> <len> <data...> */
    /* 例如: cansend 1 0x123 8 01 02 03 04 05 06 07 08 (port: 1=CAN1, 2=CAN2, 3=CAN3) */
    
    /* 获取端口号 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: cansend <port> <id> <len> <data...> (port: 1-3)\r\n");
        return pdFALSE;
    }
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    port = (UINT8)atoi(param_str);
    
    if (port < 1 || port > DRV_CAN_INS_COUNT) {
        snprintf(buf, bufsz, "Error: port must be 1-%u\r\n", DRV_CAN_INS_COUNT);
        return pdFALSE;
    }
    port = port - 1;
    
    /* 获取 ID */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: cansend <port> <id> <len> <data...>\r\n");
        return pdFALSE;
    }
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    id = (UINT32)strtoul(param_str, NULL_PTR, 16);

    if (0U == (id & DRV_CAN_EXTEND_ID_MASK)) {
        if (id > 0x7FFU) {
            id |= DRV_CAN_EXTEND_ID_MASK;
        }
    }

    /* 获取长度 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 3, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: cansend <port> <id> <len> <data...>\r\n");
        return pdFALSE;
    }
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    data_len = (UINT8)atoi(param_str);
    
    if (data_len > 8) {
        snprintf(buf, bufsz, "Error: len must be <= 8\r\n");
        return pdFALSE;
    }
    
    /* 获取数据 */
    for (i = 0; i < data_len; i++) {
        param_ptr = FreeRTOS_CLIGetParameter(cmd, 4 + i, &param_len);
        if (NULL_PTR == param_ptr || param_len == 0) {
            snprintf(buf, bufsz, "Usage: cansend <port> <id> <len> <data...>\r\n");
            return pdFALSE;
        }
        param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
        strncpy(param_str, param_ptr, param_len);
        param_str[param_len] = '\0';
        data[i] = (UINT8)strtoul(param_str, NULL_PTR, 16);
    }
    
    /* 构造消息并发送 */
    msg.ins = port;
    msg.id = id;
    msg.len = data_len;
    memcpy(msg.data, data, data_len);
    
    ret = can_if_send(&msg);
    if (ret == 0) {
        snprintf(buf, bufsz, "CAN%u send: ID=0x%lX Len=%u Data=", port + 1, id, data_len);
        for (i = 0; i < data_len; i++) {
            CHAR tmp[8];
            snprintf(tmp, sizeof(tmp), "%02X ", data[i]);
            strncat(buf, tmp, bufsz - strlen(buf) - 1);
        }
        strncat(buf, "\r\n", bufsz - strlen(buf) - 1);
    } else {
        snprintf(buf, bufsz, "CAN%u send failed, ret=%d\r\n", port + 1, ret);
    }
    
    return pdFALSE;
}

/* Shell 命令：设置 CAN 波特率 */
static BaseType_t can_setbaud_cmd(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[32];
    UINT8       port = 0;
    UINT32      baudrate = 0;
    INT32       ret;
    
    /* 格式: cansetbaud <port> <baudrate> */
    /* 例如: cansetbaud 1 500 (port: 1=CAN1, 2=CAN2, 3=CAN3) */
    
    /* 获取端口号 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: cansetbaud <port> <baudrate> (port: 1-3)\r\n");
        return pdFALSE;
    }
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    port = (UINT8)atoi(param_str);
    
    if (port < 1 || port > DRV_CAN_INS_COUNT) {
        snprintf(buf, bufsz, "Error: port must be 1-%u\r\n", DRV_CAN_INS_COUNT);
        return pdFALSE;
    }
    port = port - 1;
    
    /* 获取波特率 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 2, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: cansetbaud <port> <baudrate>\r\n");
        return pdFALSE;
    }
    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';
    baudrate = (UINT32)atoi(param_str);
    
    /* 验证波特率 */
    if (baudrate != 0 && baudrate != 250 && baudrate != 500 && baudrate != 1000) {
        snprintf(buf, bufsz, "Error: baudrate must be 0, 250, 500, or 1000 (kbps)\r\n");
        return pdFALSE;
    }
    
    ret = can_if_setbaud(port, baudrate, 1);
    if (ret == 0) {
        snprintf(buf, bufsz, "Success: CAN%u baudrate set to %lu kbps\r\n", port + 1, baudrate);
    } else {
        snprintf(buf, bufsz, "Failed: CAN%u baudrate set to %lu kbps, ret=%d\r\n", port + 1, baudrate, ret);
    }
    return pdFALSE;
}

TBOX_SHELL_DEFINE(showcan, "Show CAN status and statistics", 0, show_can_cmd);
TBOX_SHELL_DEFINE(canlog, "Show last received CAN messages", 0, can_log_cmd);
TBOX_SHELL_DEFINE(cansend, "Send CAN message: cansend <port> <id> <len> <data...> (port: 1-3)", -1, can_send_cmd);
TBOX_SHELL_DEFINE(setcanbaud, "Set CAN baudrate: cansetbaud <port> <baudrate> (port: 1-3)", 2, can_setbaud_cmd);

VOID can_shell_init(VOID)
{
    TBOX_SHELL_REGISTER(showcan);
    TBOX_SHELL_REGISTER(canlog);
    TBOX_SHELL_REGISTER(cansend);
    TBOX_SHELL_REGISTER(setcanbaud);
}
