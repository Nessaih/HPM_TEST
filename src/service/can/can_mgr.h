#ifndef __CAN_MGR_H__
#define __CAN_MGR_H__

#include <stdint.h>
#include "api_rtos.h"
#include "can_types.h"

void can_mgr_init(void);

void can_mgr_deinit(void);

void can_mgr_stat_add_recv_msg(can_msg_t *msg);

void can_mgr_stat_add_send_msg(uint8_t ins);

uint32_t can_mgr_stat_get_recv_count(uint8_t ins);

uint32_t can_mgr_stat_get_send_count(uint8_t ins);

uint8_t can_mgr_stat_get_last_msg_count(uint8_t ins);

can_msg_t *can_mgr_stat_get_last_msg(uint8_t ins, uint8_t idx);

uint32_t can_mgr_get_baudrate(uint8_t ins);

void can_mgr_update_baudrate(uint8_t ins, uint32_t baudrate);

#endif /* __CAN_MGR_H__ */
