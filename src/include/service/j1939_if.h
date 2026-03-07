#ifndef __J939_IF_H__
#define __J939_IF_H__

#include "j1939_includes.h"

void    j1939_init(void);
void    j1939_process(void);
void    j1939_receive(unsigned char instance, can_msg_t *msg);
int32_t j1939_send(J1939_TX_MESSAGE_T *msg_ptr);
int32_t j1939_request(uint8_t channel, uint32_t pgn, uint8_t dstaddr, uint8_t saaddr);
int32_t j1939_subscribe(uint8_t target_addr, uint32_t target_pgn, J1939_PGN_CALLBACK_T recv_cb);
void    j1939_show_pgn_filter(void);

#endif //__J939_IF_H__