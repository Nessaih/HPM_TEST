#ifndef __HPM_DATA_RECV_H__
#define __HPM_DATA_RECV_H__

#include "hpm_pack.h"

INT32 hpm_data_recv_init(UINT8 seq);
VOID hpm_data_recv_process(VOID);

UINT16 hpm_data_recv_push(UINT8 *data, UINT16 size);
UINT16 hpm_data_recv_pop(UINT16 size);
UINT16 hpm_data_recv_peek(UINT8 *data, UINT16 size);
UINT16 hpm_data_recv_count(VOID);
BOOL hpm_data_recv_empty(VOID);
VOID hpm_data_recv_clear(VOID);

hpm_pack_recv_t *hpm_data_recv_info_get(VOID);

#endif
