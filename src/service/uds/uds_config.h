#ifndef UDS_CONFIG_H
#define UDS_CONFIG_H

/**
 * 超时配置
 */
#define UDS_CLIENT_DEFAULT_P2_MS (50U)
#define UDS_CLIENT_DEFAULT_P2_STAR_MS (5000U)

/**
 * 会话管理配置
 */
#define MAX_UDS_SESSIONS 4              /* 最大并发会话数 */
#define UDS_DEFAULT_TIMEOUT_MS 5000     /* 默认超时时间（ms） */

/**
 * ISO-TP 传输层配置
 */
#define ISOTP_DEFAULT_BLOCK_SIZE 0      /* 默认块大小（0 = 无限制） */
#define ISOTP_DEFAULT_ST_MIN 0          /* 默认最小帧间隔（ms） */

#define UDS_ISOTP_MTU 512

/**
 * @brief UDS 调试开关
 */
#define UDS_DEBUG_ENABLE        0

/**
 * @brief UDS 调试日志宏
 */
#if UDS_DEBUG_ENABLE
    #define UDS_DEBUG_LOG(...)  MODULE_LOG_D(UDS, __VA_ARGS__)
#else
    #define UDS_DEBUG_LOG(...)  do {} while(0)
#endif

/**
 * @brief ISO-14229 协议栈日志等级
 * 
 * 可选值：
 * - UDS_LOG_NONE (0)    - 关闭日志
 * - UDS_LOG_ERROR (1)   - 仅错误
 * - UDS_LOG_WARN (2)    - 警告和错误
 * - UDS_LOG_INFO (3)    - 信息、警告和错误
 * - UDS_LOG_DEBUG (4)   - 调试、信息、警告和错误
 * - UDS_LOG_VERBOSE (5) - 详细日志（包含所有）
 * 
 * 注意：此宏必须在包含 log.h 之前定义
 */
#define UDS_LOG_LEVEL           UDS_LOG_ERROR

#endif /* UDS_CONFIG_H */
