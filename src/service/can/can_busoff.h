#ifndef __CAN_BUSOFF_H__
#define __CAN_BUSOFF_H__

#include <stdint.h>
#include <stdbool.h>

#define CAN_BUSOFF_THRD         (8)      /* 快恢复次数阈值 */
#define CAN_BUSOFF_RECOVER_FAST (100)    /* 快恢复间隔 100ms */
#define CAN_BUSOFF_RECOVER_SLOW (1000)   /* 慢恢复间隔 1000ms */

void can_busoff_init(void);

void can_busoff_callback(uint8_t port, bool is_busoff);

bool can_is_busoff_recovering(uint8_t port);

void can_busoff_sleep(void);

#endif /* __CAN_BUSOFF_H__ */
