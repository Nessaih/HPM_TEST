#ifndef __HPM_CAN_H__
#define __HPM_CAN_H__

#include "can_if.h"

#define HPM_CAN_MAX_CB (3UL)

typedef VOID (*hpm_can_recv_cb_t)(can_msg_t *msg, UINT32 count);

VOID hpm_can_init(UINT8 seq);
INT32 hpm_can_register(hpm_can_recv_cb_t cb);

#endif