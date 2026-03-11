#ifndef __UDS_IF_H__
#define __UDS_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef int32_t uds_handle_t;

#define UDS_INVALID_HANDLE  (-1)

/**
 * @brief UDS 返回值枚举
 * 
 * 返回值规则：
 * - 0: 成功
 * - 负值 (-1 ~ -99): 协议栈内部错误
 * - 正值 (0x10-0x93): UDS 负响应码 (NRC)
 */
typedef enum {
    /* ========== 成功 ========== */
    UDSIF_OK = 0,                           /**< 成功 */
    
    /* ========== 协议栈错误（负值：-1 ~ -99） ========== */
    UDSIF_ERR_TIMEOUT = -1,                 /**< 超时 */
    UDSIF_ERR_TPORT = -2,                   /**< 传输层错误 */
    UDSIF_ERR_INVALID_ARG = -3,             /**< 参数错误 */
    UDSIF_ERR_BUSY = -4,                    /**< 忙 */
    UDSIF_ERR_BUFSIZ = -5,                  /**< 缓冲区大小错误 */
    UDSIF_ERR_NEG_RESP = -6,                /**< 负响应 */
    UDSIF_ERR_SID_MISMATCH = -7,            /**< SID 不匹配 */
    UDSIF_ERR_SUBFUNCTION_MISMATCH = -8,    /**< 子功能不匹配 */
    UDSIF_ERR_RESP_TOO_SHORT = -9,          /**< 响应太短 */
    UDSIF_ERR_DID_MISMATCH = -10,           /**< DID 不匹配 */
    UDSIF_ERR_MISUSE = -11,                 /**< 使用错误 */
    
    /* ========== UDS 负响应码 (NRC) - ISO 14229-1 标准 ========== */
    /* 正值：0x10 ~ 0x93 */
    
    UDSIF_NRC_GENERAL_REJECT = 0x10,                            /**< 一般拒绝 */
    UDSIF_NRC_SERVICE_NOT_SUPPORTED = 0x11,                     /**< 不支持该服务 */
    UDSIF_NRC_SUBFUNCTION_NOT_SUPPORTED = 0x12,                 /**< 不支持该子功能 */
    UDSIF_NRC_INCORRECT_MESSAGE_LENGTH = 0x13,                  /**< 消息长度错误或格式无效 */
    UDSIF_NRC_RESPONSE_TOO_LONG = 0x14,                         /**< 响应太长 */
    
    UDSIF_NRC_BUSY_REPEAT_REQUEST = 0x21,                       /**< 忙，请重试 */
    UDSIF_NRC_CONDITIONS_NOT_CORRECT = 0x22,                    /**< 条件不满足 */
    UDSIF_NRC_REQUEST_SEQUENCE_ERROR = 0x24,                    /**< 请求序列错误 */
    UDSIF_NRC_NO_RESPONSE_FROM_SUBNET_COMPONENT = 0x25,         /**< 子网组件无响应 */
    UDSIF_NRC_FAILURE_PREVENTS_EXECUTION = 0x26,                /**< 故障阻止执行 */
    
    UDSIF_NRC_REQUEST_OUT_OF_RANGE = 0x31,                      /**< 请求超出范围 */
    UDSIF_NRC_SECURITY_ACCESS_DENIED = 0x33,                    /**< 安全访问被拒绝 */
    UDSIF_NRC_AUTHENTICATION_REQUIRED = 0x34,                   /**< 需要认证 */
    UDSIF_NRC_INVALID_KEY = 0x35,                               /**< 密钥错误 */
    UDSIF_NRC_EXCEED_NUMBER_OF_ATTEMPTS = 0x36,                 /**< 超过尝试次数 */
    UDSIF_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED = 0x37,           /**< 时间延迟未到 */
    
    UDSIF_NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED = 0x70,              /**< 不接受上传/下载 */
    UDSIF_NRC_TRANSFER_DATA_SUSPENDED = 0x71,                   /**< 传输数据暂停 */
    UDSIF_NRC_GENERAL_PROGRAMMING_FAILURE = 0x72,               /**< 编程失败 */
    UDSIF_NRC_WRONG_BLOCK_SEQUENCE_COUNTER = 0x73,              /**< 块序号错误 */
    
    UDSIF_NRC_RESPONSE_PENDING = 0x78,                          /**< 响应挂起 */
    
    UDSIF_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7E, /**< 当前会话不支持该子功能 */
    UDSIF_NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7F,   /**< 当前会话不支持该服务 */
    
    UDSIF_NRC_RPM_TOO_HIGH = 0x81,                              /**< 转速过高 */
    UDSIF_NRC_RPM_TOO_LOW = 0x82,                               /**< 转速过低 */
    UDSIF_NRC_ENGINE_IS_RUNNING = 0x83,                         /**< 发动机运行中 */
    UDSIF_NRC_ENGINE_IS_NOT_RUNNING = 0x84,                     /**< 发动机未运行 */
    UDSIF_NRC_ENGINE_RUN_TIME_TOO_LOW = 0x85,                   /**< 发动机运行时间过短 */
    UDSIF_NRC_TEMPERATURE_TOO_HIGH = 0x86,                      /**< 温度过高 */
    UDSIF_NRC_TEMPERATURE_TOO_LOW = 0x87,                       /**< 温度过低 */
    UDSIF_NRC_VEHICLE_SPEED_TOO_HIGH = 0x88,                    /**< 车速过高 */
    UDSIF_NRC_VEHICLE_SPEED_TOO_LOW = 0x89,                     /**< 车速过低 */
    UDSIF_NRC_THROTTLE_PEDAL_TOO_HIGH = 0x8A,                   /**< 油门踏板过高 */
    UDSIF_NRC_THROTTLE_PEDAL_TOO_LOW = 0x8B,                    /**< 油门踏板过低 */
    UDSIF_NRC_TRANSMISSION_RANGE_NOT_IN_NEUTRAL = 0x8C,         /**< 变速箱不在空挡 */
    UDSIF_NRC_TRANSMISSION_RANGE_NOT_IN_GEAR = 0x8D,            /**< 变速箱不在挡位 */
    UDSIF_NRC_BRAKE_SWITCH_NOT_CLOSED = 0x8F,                   /**< 制动开关未闭合 */
    UDSIF_NRC_SHIFTER_LEVER_NOT_IN_PARK = 0x90,                 /**< 换挡杆不在驻车位 */
    UDSIF_NRC_TORQUE_CONVERTER_CLUTCH_LOCKED = 0x91,            /**< 变矩器离合器锁止 */
    UDSIF_NRC_VOLTAGE_TOO_HIGH = 0x92,                          /**< 电压过高 */
    UDSIF_NRC_VOLTAGE_TOO_LOW = 0x93,                           /**< 电压过低 */
    UDSIF_NRC_RESOURCE_TEMPORARILY_NOT_AVAILABLE = 0x94,        /**< 资源暂时不可用 */
} udsif_result_t;

/**
 * @brief 会话配置结构
 */
typedef struct {
    uint8_t can_node;          /**< CAN 节点（0, 1, 2） */
    uint32_t phys_req_addr;    /**< 物理请求地址（扩展帧最高位置 1） */
    uint32_t func_req_addr;    /**< 功能请求地址 */
    uint32_t response_addr;    /**< 响应地址 */
    uint32_t timeout_ms;       /**< 默认超时时间（ms） */
} uds_session_config_t;

/**
 * @brief 密钥计算函数类型
 * @param seed 种子数据
 * @param seed_len 种子长度
 * @param key 输出：密钥数据
 * @param key_buf_size 密钥缓冲区大小
 * @param key_len 输出：实际密钥长度
 * @return 0=成功，-1=失败
 */
typedef int (*uds_key_calculator_t)(const uint8_t* seed,
                                    uint16_t seed_len,
                                    uint8_t* key,
                                    uint16_t key_buf_size,
                                    uint16_t* key_len);

/* ========== 会话管理接口 ========== */

/**
 * @brief 创建 UDS 诊断会话
 * @param config 会话配置
 * @return 会话句柄，失败返回 UDS_INVALID_HANDLE
 */
extern uds_handle_t uds_if_session_create(const uds_session_config_t* config);

/**
 * @brief 销毁 UDS 诊断会话
 * @param handle 会话句柄
 * @return 0=成功，-1=失败
 */
extern int32_t uds_if_session_destroy(uds_handle_t handle);

/* ========== 基础服务接口 ========== */

/**
 * @brief 会话控制 (0x10)
 * @param handle 会话句柄
 * @param session_type 会话类型（0x01=默认，0x02=编程，0x03=扩展）
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_session_control(uds_handle_t handle,
                                   uint8_t session_type,
                                   uint32_t timeout_ms);

/**
 * @brief ECU 复位 (0x11)
 * @param handle 会话句柄
 * @param reset_type 复位类型（0x01=硬复位，0x02=软复位）
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_ecu_reset(uds_handle_t handle,
                             uint8_t reset_type,
                             uint32_t timeout_ms);

/**
 * @brief 读取数据标识符 (0x22)
 * @param handle 会话句柄
 * @param did 数据标识符
 * @param data 输出：数据缓冲区
 * @param data_buf_size 数据缓冲区大小
 * @param data_len 输出：实际数据长度
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_read_did(uds_handle_t handle,
                            uint16_t did,
                            uint8_t* data,
                            uint16_t data_buf_size,
                            uint16_t* data_len,
                            uint32_t timeout_ms);

/**
 * @brief 写入数据标识符 (0x2E)
 * @param handle 会话句柄
 * @param did 数据标识符
 * @param data 数据
 * @param data_len 数据长度
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_write_did(uds_handle_t handle,
                             uint16_t did,
                             const uint8_t* data,
                             uint16_t data_len,
                             uint32_t timeout_ms);

/**
 * @brief 通信控制 (0x28)
 * @param handle 会话句柄
 * @param control_type 控制类型
 * @param communication_type 通信类型
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_communication_control(uds_handle_t handle,
                                        uint8_t control_type,
                                        uint8_t communication_type,
                                        uint32_t timeout_ms);

/**
 * @brief 测试仪在线 (0x3E)
 * @param handle 会话句柄
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_tester_present(uds_handle_t handle,
                                  uint32_t timeout_ms);

/**
 * @brief 读取 DTC 信息 (0x19)
 * @param handle 会话句柄
 * @param sub_function 子功能（0x01-0x19）
 * @param data 输入数据（可选）
 * @param data_len 输入数据长度
 * @param response 输出：响应数据
 * @param response_buf_size 响应缓冲区大小
 * @param response_len 输出：实际响应长度
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_read_dtc_information(uds_handle_t handle,
                                        uint8_t sub_function,
                                        const uint8_t* data,
                                        uint16_t data_len,
                                        uint8_t* response,
                                        uint16_t response_buf_size,
                                        uint16_t* response_len,
                                        uint32_t timeout_ms);

/**
 * @brief 安全访问 (0x27) - 完整流程
 * @param handle 会话句柄
 * @param level 安全级别（奇数）
 * @param key_calculator 密钥计算函数
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_security_access(uds_handle_t handle,
                                   uint8_t level,
                                   uds_key_calculator_t key_calculator,
                                   uint32_t timeout_ms);

/**
 * @brief 例程控制 (0x31)
 * @param handle 会话句柄
 * @param routine_type 例程类型（0x01=启动，0x02=停止，0x03=获取结果）
 * @param routine_id 例程标识符
 * @param data 输入数据
 * @param data_len 输入数据长度
 * @param response 输出：响应数据
 * @param response_buf_size 响应缓冲区大小
 * @param response_len 输出：实际响应长度
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_routine_control(uds_handle_t handle,
                                   uint8_t routine_type,
                                   uint16_t routine_id,
                                   const uint8_t* data,
                                   uint16_t data_len,
                                   uint8_t* response,
                                   uint16_t response_buf_size,
                                   uint16_t* response_len,
                                   uint32_t timeout_ms);

/**
 * @brief 请求下载 (0x34)
 * @param handle 会话句柄
 * @param memory_address 内存地址
 * @param memory_size 内存大小
 * @param data_format 数据格式标识符
 * @param max_block_length 输出：最大块长度
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_request_download(uds_handle_t handle,
                                    uint32_t memory_address,
                                    uint32_t memory_size,
                                    uint8_t data_format,
                                    uint16_t* max_block_length,
                                    uint32_t timeout_ms);

/**
 * @brief 传输数据 (0x36)
 * @param handle 会话句柄
 * @param block_sequence 块序号
 * @param data 数据
 * @param data_len 数据长度
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_transfer_data(uds_handle_t handle,
                                 uint8_t block_sequence,
                                 const uint8_t* data,
                                 uint16_t data_len,
                                 uint32_t timeout_ms);

/**
 * @brief 退出传输 (0x37)
 * @param handle 会话句柄
 * @param timeout_ms 超时时间（ms）
 * @return 0=成功，负值=错误，正值=NRC
 */
extern int uds_if_request_transfer_exit(uds_handle_t handle,
                                         uint32_t timeout_ms);

/* ========== 调试和信息接口 ========== */

/**
 * @brief 会话详细信息结构
 */
typedef struct {
    uds_handle_t handle;                    /**< 会话句柄 */
    bool in_use;                            /**< 是否使用中 */
    uint8_t can_node;                       /**< CAN 节点 */
    uds_session_config_t config;            /**< 会话配置 */
    uint8_t client_state;                   /**< 客户端状态（0=空闲，1=发送中，2=等待发送完成，3=等待响应） */
    uint16_t p2_ms;                         /**< P2 超时时间（ms） */
    uint32_t p2_star_ms;                    /**< P2* 超时时间（ms） */
    uint16_t recv_size;                     /**< 接收数据大小 */
    uint16_t send_size;                     /**< 发送数据大小 */
    bool operation_done;                    /**< 操作完成标志 */
    uint32_t last_error;                    /**< 最后一次错误码 */
    uint32_t tp_phys_sa;                    /**< 物理源地址 */
    uint32_t tp_phys_ta;                    /**< 物理目标地址 */
    uint32_t tp_func_sa;                    /**< 功能源地址 */
    uint32_t tp_func_ta;                    /**< 功能目标地址 */
} uds_session_info_t;

/**
 * @brief 会话概览信息结构
 */
typedef struct {
    uds_handle_t handle;                    /**< 会话句柄 */
    uint8_t can_node;                       /**< CAN 节点 */
    uint32_t phys_req_addr;                 /**< 物理请求地址 */
    uint32_t func_req_addr;                 /**< 功能请求地址 */
    uint32_t response_addr;                 /**< 响应地址 */
    uint8_t client_state;                   /**< 客户端状态 */
} uds_session_overview_t;

/**
 * @brief 获取会话详细信息
 * @param handle 会话句柄
 * @param info 输出：会话信息
 * @return 0=成功，负值=错误
 */
extern int uds_if_get_session_info(uds_handle_t handle, uds_session_info_t* info);

/**
 * @brief 获取所有会话的概览信息
 * @param sessions 输出：会话概览数组
 * @param max_sessions 数组最大长度
 * @param actual_count 输出：实际会话数量
 * @return 0=成功，负值=错误
 */
extern int uds_if_get_sessions_overview(uds_session_overview_t* sessions, 
                                        uint8_t max_sessions, 
                                        uint8_t* actual_count);

#ifdef __cplusplus
}
#endif

#endif /* __UDS_IF_H__ */
