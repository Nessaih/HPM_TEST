#ifndef __CAN_TYPES_H__
#define __CAN_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* CAN 消息结构体定义 */
#pragma pack(1)
typedef struct
{
    uint32_t id;
    uint8_t  ins;
    uint8_t  len;
    uint8_t  data[8];
} can_msg_t;
#pragma pack()

#define DRV_CAN_INS_COUNT       3U

#ifdef __cplusplus
}
#endif

#endif /* __CAN_TYPES_H__ */
