#ifndef __DRV_CAN_H__
#define __DRV_CAN_H__

#include <stdbool.h>
#include <stdint.h>
#include "can_types.h"


#define DRV_CAN_EVENT_RX_DONE   0U
#define DRV_CAN_EVENT_TX_DONE   1U
#define DRV_CAN_EVENT_BUS_ERROR 2U
#define DRV_CAN_EVENT_BUS_OFF   3U
#define DRV_CAN_EVENT_WAKEUP    4U

#define DRV_CAN_MODE_NORMAL     0U
#define DRV_CAN_MODE_LISTEN     1U
#define DRV_CAN_MODE_LOOPBACK   2U

#define DRV_CAN_EXTEND_ID_MASK  0x80000000U

typedef void (*drv_can_reg_cb_t)(uint8_t ins, uint32_t event, void *para);

int32_t drv_can_init(uint8_t ins, uint32_t rate, uint8_t mode);
int32_t drv_can_deinit(uint8_t ins);
int32_t drv_can_register(uint8_t ins, drv_can_reg_cb_t cb);
int32_t drv_can_send(const can_msg_t *msg);
bool    drv_can_busy(uint8_t ins);
bool    drv_can_has_init(uint8_t ins);
bool    drv_can_is_bus_off(uint8_t ins);
bool    drv_can_is_sending(uint8_t ins);
int32_t drv_can_enable_error_report(uint8_t ins, bool enable);

#endif 
