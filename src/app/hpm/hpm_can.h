#ifndef __HPM_CAN_H__
#define __HPM_CAN_H__

VOID hpm_can_init(UINT8 seq);
VOID hpm_can_deinit(VOID);
INT32 hpm_can_report(UINT8 *data);

#endif