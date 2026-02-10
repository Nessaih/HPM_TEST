#ifndef __CAN_BUSERR_H__
#define __CAN_BUSERR_H__

#include <stdint.h>
#include <stdbool.h>

#define CAN_BUSERR_THRD         (5)      /* 错误次数阈值 */
#define CAN_BUSERR_RECOVER_TIME (100)    /* 错误恢复间隔 100ms */

void can_buserr_init(void);

void can_buserr_callback(uint8_t port, bool is_error);

bool can_is_buserr_recovering(uint8_t port);

void can_buserr_sleep(void);

#endif /* __CAN_BUSERR_H__ */
