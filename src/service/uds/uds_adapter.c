#include "uds_config.h"
#include "uds_adapter.h"
#include "tbox_common.h"
#include "tbox_log.h"
#include "can_if.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
typedef struct {
    uint8_t can_node;
} uds_session_info_t;

/** @brief CAN 发送函数 */
int isotp_user_send_can(uint32_t arbitration_id,
                        const uint8_t* data,
                        uint8_t size,
                        void* user_arg)
{
    UDS_DEBUG_LOG("send_can called: id=0x%08X, size=%d", arbitration_id, size);
    
    if (data == NULL_PTR || size == 0 || size > 8) {
        MODULE_LOG_E(UDS, "send_can: invalid params (data=%p, size=%d)", data, size);
        return -1;
    }
    
    uds_session_info_t* info = (uds_session_info_t*)user_arg;
    if (info == NULL_PTR) {
        MODULE_LOG_E(UDS, "send_can: user_arg is NULL");
        return -1;
    }
    
    UDS_DEBUG_LOG("send_can: user_arg=%p, can_node=%d (raw byte: 0x%02X)", 
                  user_arg, info->can_node, *(uint8_t*)user_arg);
    
    can_msg_t msg;
    msg.ins = info->can_node;
    msg.id = arbitration_id;
    msg.len = size;
    
    memcpy(msg.data, data, size);
    
    MODULE_LOG_D(UDS, "send_can: node=%d, id=0x%08X, len=%d", msg.ins, msg.id, msg.len);
    
    INT32 ret = can_if_send(&msg);
    
    if (ret != TBOX_E_OK) {
        MODULE_LOG_E(UDS, "send_can failed: ret=%d", ret);
    }
    
    return (ret == TBOX_E_OK) ? 0 : -1;
}

/** @brief 获取微秒时间戳 */
uint32_t isotp_user_get_us(void)
{
    return xTaskGetTickCount() * 1000;
}

/** @brief 调试输出函数 */
void isotp_user_debug(const char* message, ...)
{
    if (message == NULL_PTR) {
        return;
    }
    
    char buffer[256];
    va_list args;
    
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);
    
    MODULE_LOG_D(UDS, "%s", buffer);
}

/** @brief 获取毫秒时间戳 */
uint32_t UDSMillis(void)
{
    return xTaskGetTickCount();
}
