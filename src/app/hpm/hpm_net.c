#include "tbox_common.h"
#include "tbox_core.h"
#include "4g_if.h"

#include "hpm_net.h"
#include "hpm_data_recv.h"

static VOID hpm_net_send_cb(UINT8 conn_id, UINT8 resualt)
{
    if (IF_4G_HPM_CONN_ID != conn_id)
        return;

    (void)resualt;
}

static VOID hpm_net_recv_cb(UINT8 conn_id, UINT8 *data, UINT16 len)
{
    INT32 res;

    if (IF_4G_HPM_CONN_ID != conn_id)
    {
        return;
    }

    res = hpm_data_recv_put(data, len);
    if (0 != res)
    {
        MODULE_LOG_E(HPM, "put data fail, len:%u",len);
    }
}

INT32 hpm_net_init(UINT8 seq)
{
    switch (seq)
    {
        case MODULE_INIT_SEQ_OS:
            
            break;

        case MODULE_INIT_SEQ_STORAGE:
            break;

        case MODULE_INIT_SEQ_MODULE:           
			if_4g_reg_transmit_callback(IF_4G_HPM_CONN_ID, hpm_net_send_cb, hpm_net_recv_cb);
            break;
            
        default:
            break;
    }

	return 0;
}

BOOL hpm_net_is_ready(VOID)
{
    INT32 state;

    state = if_4g_get_socket_conn_state(IF_4G_HPM_CONN_ID);
    if (IF_4G_STATE_CONNECTED == state)
        return true;
    return false;
}

INT32 hpm_net_connect(UINT8 *ip, UINT16 port)
{
    return if_4g_socket_connect(IF_4G_PUBLIC_APN, IF_4G_HPM_CONN_ID, IF_4G_SOCKT_TCP, ip, port);
}

INT32 hpm_net_disconnect(VOID)
{
    return if_4g_socket_close(IF_4G_HPM_CONN_ID);
}

INT32 hpm_net_send(UINT8 *data, UINT16 len)
{
    if (NULL == data || 0 == len)
    {
        return -1;
    }
    return if_4g_socket_send(IF_4G_HPM_CONN_ID, data, len);
}

INT32 hpm_net_recv(UINT8 *data, UINT16 *len)
{
    if (NULL == data || NULL == len)
    {
        return -1;
    }
    return hpm_data_recv_get(data, len);
}

