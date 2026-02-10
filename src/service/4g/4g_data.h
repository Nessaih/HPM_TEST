#ifndef TBOX_4G_DATA_H
#define TBOX_4G_DATA_H

#include "4g_at.h"

extern AT_4G_CMD_RESP data_4g_cur_send;

void data_4g_init(void);

VOID data_4g_recv_push(UINT8 data);

uint8* data_4g_recv_pull(uint16 *len);

void data_4g_recv_remove(uint16 remove_len);

uint8* data_4g_get_send_data(uint16 *len);

boolean data_4g_is_data_sending(void);

void data_4g_send_finish_ind(void);

#endif /* TBOX_4G_DATA_H */

