

#include "j1939_includes.h"

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
    if (true == j1939_tl_send(msg_ptr))
        return 0;
    return -1;
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
