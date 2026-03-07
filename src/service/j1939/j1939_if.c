

#include "tbox_common.h"
#include "tbox_core.h"
#include "j1939_includes.h"

extern VOID j1939_svr_active(VOID);
extern VOID j1939_svr_inactive(VOID);

void j1939_init(void)
{
    j1939_stk_init();
}

void j1939_process(void)
{
    j1939_stk_periodic();
}

void j1939_receive(unsigned char instance, can_msg_t *msg)
{
    j1939_hal_rx(instance, msg);
}

int32_t j1939_send(J1939_TX_MESSAGE_T *msg_ptr)
{
    if (j1939_tl_send(msg_ptr, j1939_svr_inactive))
    {
        j1939_svr_active();
        return 0;
    }
    return -1;
}

int32_t j1939_request(uint8_t channel, uint32_t pgn, uint8_t dstaddr, uint8_t saaddr)
{
    J1939_TX_MESSAGE_T msg;
    msg.PGN        = REQUEST_PGN;
    msg.priority   = 6;
    msg.dest_addr  = dstaddr;
    msg.sa_addr    = saaddr;
    msg.byte_count = 8;
    msg.channel    = channel;
    msg.data[0]    = pgn;
    msg.data[1]    = pgn >> 8;
    msg.data[2]    = 0xFF;
    msg.data[3]    = 0xFF;
    msg.data[4]    = 0xFF;
    msg.data[5]    = 0xFF;
    msg.data[6]    = 0xFF;
    msg.data[7]    = 0xFF;

    return j1939_send(&msg);
}

int32_t j1939_subscribe(uint8_t target_addr, uint32_t target_pgn, J1939_PGN_CALLBACK_T recv_cb)
{
    if (true == j1939_al_subscribe(target_addr, target_pgn, recv_cb))
        return 0;
    return -1;
}

void j1939_show_pgn_filter(void)
{
    j1939_pgn_show();
}
