#ifndef __CAN_IF_H__
#define __CAN_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "can_types.h"

#define CAN_IF_EVENT_SEND_START 0x01U
#define CAN_IF_EVENT_SEND_DONE  0x02U
#define CAN_IF_EVENT_RECEIVED   0x04U
#define CAN_IF_EVENT_BUSOK      0x08U
#define CAN_IF_EVENT_BUSOFF     0x10U
#define CAN_IF_EVENT_BUS_ERROR  0x20U
#define CAN_IF_EVENT_INS_POS    0x18U
#define CAN_IF_EVENT_INS_MASK   0xFF000000U

typedef enum
{
    CAN_INSTANCE_NOUSED = 0,   // 未使用
    CAN_INSTANCE_IDLE,         // 空闲（10s内无数据）
    CAN_INSTANCE_BUSY,         // 忙碌（10s内有数据）
    CAN_INSTANCE_ERROR,        // 错误
    CAN_INSTANCE_OFF           // 关闭
} CAN_STATE;

/* CAN总线事件定义 */
typedef enum {
    CAN_EVENT_ACTIVE = 1,      /* 总线激活 */
    CAN_EVENT_INACTIVE,        /* 总线不活跃 */
    CAN_EVENT_DATAIN,          /* 接收CAN */
    CAN_EVENT_SLEEP,           /* 休眠 */
    CAN_EVENT_WAKEUP           /* 唤醒 */
} CAN_EVENT;

typedef void (*can_callback_t)(uint32_t event, void *para, uint32_t count);
typedef int32_t (*can_event_callback_t)(CAN_EVENT event, uint32_t arg1, uint32_t arg2);

extern int32_t  can_if_setbaud(uint8_t ins, uint32_t baudrate, uint8_t reinit);
extern int32_t  can_if_send(can_msg_t *msg);
extern int32_t  can_if_reg_cb(can_event_callback_t cb);
extern int32_t  can_if_unreg_cb(can_event_callback_t cb);
extern uint8_t  can_if_state_get(uint8_t instance);
extern uint32_t can_if_get_recv_count(uint8_t instance);
extern BOOL     can_if_is_bus_active(void);
extern BOOL     can_if_is_bus_sleep(void);

#ifdef __cplusplus
}
#endif

#endif //__CAN_IF_H__