#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tbox_cfg_if.h"
#include "tbox_common.h"
#include "tbox_log.h"
#include "tbox_shell_if.h"
#include "can_if.h"
#include "can_mgr.h"
#include "can_shell.h"
#include "can_types.h"
#include "drv_can.h"

/* Shell 命令：显示 CAN 状态和统计信息 */
static BaseType_t show_can_cmd(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    UINT8       i;
    UINT8       state;
    const CHAR *state_str[] = {"NOUSED", "IDLE", "BUSY", "ERROR", "OFF"};
    INT32       len         = 0;
    double      recv_rate, send_rate;

    (VOID) cmd;

    len += snprintf(buf + len, bufsz - len, "\r\n=== CAN Status ===\r\n");
    len += snprintf(buf + len, bufsz - len, "Bus Active: %s\r\n", can_if_is_bus_active() ? "YES" : "NO");
    len += snprintf(buf + len, bufsz - len, "Bus Sleep:  %s\r\n", can_if_is_bus_sleep() ? "YES" : "NO");

    for (i = 0; i < DRV_CAN_INS_COUNT; i++) {
        state = can_if_state_get(i);
        recv_rate = can_mgr_stat_get_recv_rate(i);
        send_rate = can_mgr_stat_get_send_rate(i);
        
        if (state < sizeof(state_str) / sizeof(state_str[0])) {
            len += snprintf(buf + len, bufsz - len, 
                "CAN%u: state=%s, baudrate=%lu kbps, recv=%lu(%.1f fps), drop_recv=%lu, send=%lu(%.1f fps), drop_send=%lu\r\n", 
                i + 1, state_str[state],
                can_mgr_get_baudrate(i), 
                can_mgr_stat_get_recv_count(i), recv_rate, can_mgr_stat_get_droprx_count(i),
                can_mgr_stat_get_send_count(i), send_rate, can_mgr_stat_get_droptx_count(i));
        } else {
            len += snprintf(buf + len, bufsz - len, 
                "CAN%u: state=UNKNOWN, baudrate=%lu kbps, recv=%lu(%.1f fps), drop_recv=%lu, send=%lu(%.1f fps), drop_send=%lu\r\n", 
                i + 1, 
                can_mgr_get_baudrate(i),
                can_mgr_stat_get_recv_count(i), recv_rate, can_mgr_stat_get_droprx_count(i),
                can_mgr_stat_get_send_count(i), send_rate, can_mgr_stat_get_droptx_count(i));
        }
    }

    len += snprintf(buf + len, bufsz - len, "====================\r\n");

    return pdFALSE;
}

/* Shell 命令：显示最近接收的 CAN 消息日志 */
static BaseType_t can_log_cmd(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    static UINT8 current_can = 0; /* 静态变量记录当前输出的 CAN 通道 */
    static UINT8 current_msg = 0; /* 静态变量记录当前输出的消息索引 */
    static UINT8 first_call  = 1; /* 是否首次调用 */
    UINT8        j;
    INT32        len = 0;
    UINT8        msg_count;
    UINT8        batch_count;
    can_msg_t   *msg;

    (VOID) cmd;

    /* 首次调用，输出标题并初始化 */
    if (first_call != 0) {
        current_can = 0;
        current_msg = 0;
        first_call  = 0;
        len += snprintf(buf + len, bufsz - len, "\r\n=== CAN Message Log ===\r\n");
        return pdTRUE;
    }

    /* 遍历每个 CAN 通道 */
    while (current_can < DRV_CAN_INS_COUNT) {
        msg_count = can_mgr_stat_get_last_msg_count(current_can);

        /* 输出当前 CAN 通道的标题 */
        if (current_msg == 0) {
            len += snprintf(buf + len, bufsz - len, "\r\n--- CAN%u (Total: %u) ---\r\n", current_can + 1, msg_count);

            if (msg_count == 0) {
                len += snprintf(buf + len, bufsz - len, "No messages received yet.\r\n");
                current_can++;
                continue;
            }
        }

        /* 输出当前 CAN 通道的消息（每次最多输出 5 条，避免超出缓冲区） */
        batch_count = 0;
        while (current_msg < msg_count && batch_count < 5) {
            msg = can_mgr_stat_get_last_msg(current_can, current_msg);
            if (msg != NULL_PTR) {
                len += snprintf(buf + len, bufsz - len, "[%02u] CAN%u ID=0x%08lX Len=%u Data=", current_msg + 1, current_can + 1, msg->id, msg->len);

                for (j = 0; j < msg->len && j < 8; j++) {
                    len += snprintf(buf + len, bufsz - len, "%02X ", msg->data[j]);
                }

                len += snprintf(buf + len, bufsz - len, "\r\n");
            }
            current_msg++;
            batch_count++;
        }

        /* 当前 CAN 通道输出完毕，切换到下一个 */
        if (current_msg >= msg_count) {
            current_can++;
            current_msg = 0;
        }

        /* 如果缓冲区快满了，先返回，下次继续 */
        if (len > (INT32)(bufsz - 256)) {
            return pdTRUE;
        }
    }

    len += snprintf(buf + len, bufsz - len, "================================\r\n");
    first_call = 1;
    return pdFALSE; /* 命令执行完毕 */
}

/* Shell 命令：发送 CAN 数据 */
static BaseType_t can_send_cmd(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[64];
    UINT8       port     = 0;
    UINT32      id       = 0;
    UINT8       data_len = 0;
    UINT8       data[8]  = {0};
    can_msg_t   msg      = {0};
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
    port                 = (UINT8)atoi(param_str);

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
    id                   = (UINT32)strtoul(param_str, NULL_PTR, 16);

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
    data_len             = (UINT8)atoi(param_str);

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
        data[i]              = (UINT8)strtoul(param_str, NULL_PTR, 16);
    }

    /* 构造消息并发送 */
    msg.ins = port;
    msg.id  = id;
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
    UINT8       port     = 0;
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
    port                 = (UINT8)atoi(param_str);

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
    baudrate             = (UINT32)atoi(param_str);

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

/* CAN事件测试统计 */
static struct {
    uint32_t active_count;
    uint32_t inactive_count;
    uint32_t datain_count;
    uint32_t sleep_count;
    uint32_t wakeup_count;
    uint32_t total_msg_count;
} can_event_stat = {0};

static bool can_event_test_enabled = false;

/**
 * @brief CAN事件回调测试函数
 */
static int32_t can_event_test_callback(CAN_EVENT event, uint32_t arg1, uint32_t arg2)
{
    if (!can_event_test_enabled) {
        return 0;
    }
    
    switch (event) {
    case CAN_EVENT_ACTIVE:
        can_event_stat.active_count++;
        tbox_log_print("[CAN_EVENT_TEST] Bus active, count=%u\r\n", can_event_stat.active_count);
        break;
        
    case CAN_EVENT_INACTIVE:
        can_event_stat.inactive_count++;
        tbox_log_print("[CAN_EVENT_TEST] Bus inactive, uptime=%u, count=%u\r\n", 
                       arg1, can_event_stat.inactive_count);
        break;
        
    case CAN_EVENT_DATAIN: {
        can_msg_t *msgs = (can_msg_t *)arg1;
        uint32_t count = arg2;
        can_event_stat.datain_count++;
        can_event_stat.total_msg_count += count;
        
        tbox_log_print("[CAN_EVENT_TEST] Received %u messages, total_datain=%u, total_msg=%u\r\n", 
                       count, can_event_stat.datain_count, can_event_stat.total_msg_count);
        
        /* 打印前2条消息详情 */
        for (uint32_t i = 0; i < count && i < 2; i++) {
            tbox_log_print("  [%u] ins=%u, ID=0x%X, len=%u\r\n", 
                           i, msgs[i].ins, msgs[i].id, msgs[i].len);
        }
        break;
    }
    
    case CAN_EVENT_SLEEP:
        can_event_stat.sleep_count++;
        tbox_log_print("[CAN_EVENT_TEST] Sleep, count=%u\r\n", can_event_stat.sleep_count);
        break;
        
    case CAN_EVENT_WAKEUP:
        can_event_stat.wakeup_count++;
        tbox_log_print("[CAN_EVENT_TEST] Wakeup, count=%u\r\n", can_event_stat.wakeup_count);
        break;
    }
    
    return 0;
}

/**
 * @brief 启动CAN事件测试
 */
void test_can_event_start(void)
{
    int32_t ret;
    
    ret = can_if_reg_cb(can_event_test_callback);
    if (ret == 0) {
        can_event_test_enabled = true;
        memset(&can_event_stat, 0, sizeof(can_event_stat));
        tbox_log_print("[CAN_EVENT_TEST] Started\r\n");
    } else {
        tbox_log_print("[CAN_EVENT_TEST] Failed to start, ret=%d\r\n", ret);
    }
}

/**
 * @brief 停止CAN事件测试
 */
void test_can_event_stop(void)
{
    int32_t ret;
    
    can_event_test_enabled = false;
    
    ret = can_if_unreg_cb(can_event_test_callback);
    if (ret == 0) {
        tbox_log_print("[CAN_EVENT_TEST] Stopped\r\n");
    } else {
        tbox_log_print("[CAN_EVENT_TEST] Failed to stop, ret=%d\r\n", ret);
    }
    
    /* 打印统计 */
    tbox_log_print("=== CAN Event Test Statistics ===\r\n");
    tbox_log_print("  Active:   %u\r\n", can_event_stat.active_count);
    tbox_log_print("  Inactive: %u\r\n", can_event_stat.inactive_count);
    tbox_log_print("  DataIn:   %u\r\n", can_event_stat.datain_count);
    tbox_log_print("  Sleep:    %u\r\n", can_event_stat.sleep_count);
    tbox_log_print("  Wakeup:   %u\r\n", can_event_stat.wakeup_count);
    tbox_log_print("  Total:    %u\r\n", can_event_stat.total_msg_count);
    tbox_log_print("=================================\r\n");
}

/**
 * @brief 获取CAN事件测试统计
 */
void test_can_event_stat(void)
{
    tbox_log_print("=== CAN Event Test Statistics ===\r\n");
    tbox_log_print("  Active:   %u\r\n", can_event_stat.active_count);
    tbox_log_print("  Inactive: %u\r\n", can_event_stat.inactive_count);
    tbox_log_print("  DataIn:   %u\r\n", can_event_stat.datain_count);
    tbox_log_print("  Sleep:    %u\r\n", can_event_stat.sleep_count);
    tbox_log_print("  Wakeup:   %u\r\n", can_event_stat.wakeup_count);
    tbox_log_print("  Total:    %u\r\n", can_event_stat.total_msg_count);
    tbox_log_print("=================================\r\n");
}

/* Shell 命令：CAN 事件测试 */
static BaseType_t can_event_test_cmd(CHAR *buf, UINT32 bufsz, const CHAR *cmd)
{
    const CHAR *param_ptr;
    BaseType_t  param_len;
    CHAR        param_str[32];

    /* 格式: canevent <start|stop|stat> */

    /* 获取子命令 */
    param_ptr = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    if (NULL_PTR == param_ptr || param_len == 0) {
        snprintf(buf, bufsz, "Usage: canevent <start|stop|stat>\r\n"
                             "  start - Start event callback test\r\n"
                             "  stop  - Stop event callback test and show statistics\r\n"
                             "  stat  - Show current statistics\r\n");
        return pdFALSE;
    }

    param_len = (param_len < (INT32)sizeof(param_str)) ? param_len : (INT32)sizeof(param_str) - 1;
    strncpy(param_str, param_ptr, param_len);
    param_str[param_len] = '\0';

    if (strcmp(param_str, "start") == 0) {
        test_can_event_start();
        snprintf(buf, bufsz, "CAN event test started\r\n");
    } else if (strcmp(param_str, "stop") == 0) {
        test_can_event_stop();
        snprintf(buf, bufsz, "CAN event test stopped (see log for statistics)\r\n");
    } else if (strcmp(param_str, "stat") == 0) {
        test_can_event_stat();
        snprintf(buf, bufsz, "See log for statistics\r\n");
    } else {
        snprintf(buf, bufsz, "Unknown subcommand: %s\r\n", param_str);
    }

    return pdFALSE;
}

TBOX_SHELL_DEFINE(showcan, "Show CAN status and statistics", 0, show_can_cmd);
TBOX_SHELL_DEFINE(canlog, "Show last received CAN messages", 0, can_log_cmd);
TBOX_SHELL_DEFINE(cansend, "Send CAN message: cansend <port> <id> <len> <data...> (port: 1-3)", -1, can_send_cmd);
TBOX_SHELL_DEFINE(setcanbaud, "Set CAN baudrate: cansetbaud <port> <baudrate> (port: 1-3)", 2, can_setbaud_cmd);
TBOX_SHELL_DEFINE(canevent, "CAN event callback test: canevent <start|stop|stat>", 1, can_event_test_cmd);

VOID can_shell_init(VOID)
{
    TBOX_SHELL_REGISTER(showcan);
    TBOX_SHELL_REGISTER(canlog);
    TBOX_SHELL_REGISTER(cansend);
    TBOX_SHELL_REGISTER(setcanbaud);
    TBOX_SHELL_REGISTER(canevent);
}
