#ifndef CAN_IF_H
#define CAN_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "can_types.h"

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

typedef void (*can_callback_t)(UINT32 event, VOID *para, UINT32 count);
typedef INT32 (*can_event_callback_t)(CAN_EVENT event, UINT32 arg1, UINT32 arg2);

extern INT32  can_if_setbaud(UINT8 ins, UINT32 baudrate, UINT8 reinit);
extern INT32  can_if_send(can_msg_t *msg);
extern INT32  can_if_reg_cb(can_event_callback_t cb);
extern INT32  can_if_unreg_cb(can_event_callback_t cb);
extern UINT8  can_if_state_get(UINT8 instance);
extern UINT32 can_if_get_recv_count(UINT8 instance);
extern BOOL   can_if_is_bus_active(VOID);
extern BOOL   can_if_is_bus_sleep(VOID);

#ifdef __cplusplus
}
#endif

#endif //__CAN_IF_H__