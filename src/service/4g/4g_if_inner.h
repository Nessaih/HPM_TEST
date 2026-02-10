#ifndef TBOX_4G_IF_INNER_H
#define TBOX_4G_IF_INNER_H

#include "4g_if.h"

void if_4g_init(void);

IF_4G_SEND_CALLBACK if_4g_get_send_callback(uint8 conn_id);

IF_4G_RECV_CALLBAKC if_4g_get_recv_callback(uint8 conn_id);

void if_4g_periodic(void);

#endif /* TBOX_4G_IF_INNER_H */
