#ifndef __HPM_DATA_RECV_H__
#define __HPM_DATA_RECV_H__

INT32 hpm_data_recv_init(UINT8 seq);

BOOL hpm_data_recv_empty(VOID);

INT32 hpm_data_recv_put(UINT8 *data, UINT16 len);

INT32 hpm_data_recv_get(UINT8 *data, UINT16 *len);

#endif
