/////////////////////////////////////////////////////////////////////////////////////////
// File:         J1939_AL.c
// Module:       J1939 stack
// Description:  J1939 stack Application Layer
// Originator:   MB
// Derived from: Freescale J1939 stack
// Date created: 09 June 2014
//---------------------------------------------------------------------------------------
// Revision Log:
//   $Log: $
//
//---------------------------------------------------------------------------------------
// Notes:
//
//---------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////
// Copyright (C)2014 by Timespace Technology Ltd.  All rights reserved.
//
// This software is confidential.  It is not to be copied or distributed in any form
// to any third party.
//
// This software is provided by Timespace Technology on an 'as is' basis, and Timespace
// Technology expressly disclaims any and all warranties, expressed or implied including,
// without limitation, warranties of merchantability and fitness for a particular
// purpose. In no event shall Timespace Technology be liable for any direct, indirect,
// incidental, punitive or consequential damages - of any kind whatsoever - with respect
// to the software.
/////////////////////////////////////////////////////////////////////////////////////////

#include "j1939_includes.h"

#define AL_SUBSCRIBE_CNT_MAX 10

typedef struct
{
    volatile uint32_t    sa  : 8;
    volatile uint32_t    pgn : 24;
    J1939_PGN_CALLBACK_T cb;
} AL_PGN_SUBSCRIBE_T;

typedef struct
{
    uint32_t           cnt;
    AL_PGN_SUBSCRIBE_T sub[AL_SUBSCRIBE_CNT_MAX];
} AL_PGN_SUBSCRIBE_TABLE_T;

list_define(al_rx_msg_t, J1939_RX_MESSAGE_T, 4);

AL_PGN_SUBSCRIBE_TABLE_T  al_table;
static struct al_rx_msg_t rx_list;

//
void j1939_al_init(void)
{
    list_init(&rx_list);
}

void j1939_al_periodic(void)
{
    J1939_RX_MESSAGE_T msg;

    if (list_is_empty(&rx_list))
        return;
    list_get(&rx_list, &msg);

    for (size_t i = 0; i < al_table.cnt; i++)
    {
        uint8_t  subscribed_sa  = al_table.sub[i].sa;
        uint32_t subscribed_pgn = al_table.sub[i].pgn;

        if (subscribed_pgn != msg.PGN)
            continue;
        if (subscribed_sa && subscribed_sa != msg.source_addr)
            continue;
        if (al_table.sub[i].cb)
            al_table.sub[i].cb((uint8_t *)msg.data, msg.byte_count, subscribed_sa, subscribed_pgn);
    }
}

void j1939_al_process(J1939_RX_MESSAGE_T *msg_ptr)
{
    if (list_is_full(&rx_list))
        return;
    list_put(&rx_list, *msg_ptr);
}

bool j1939_al_subscribe(uint8_t target_addr, uint32_t target_pgn, J1939_PGN_CALLBACK_T recv_cb)
{

    if (j1939_pgn_filter(target_addr, target_pgn))
    {
        goto __al_subscribe;
    }

    if (j1939_pgn_add(target_addr, target_pgn))
    {
        goto __al_subscribe;
    }
    else
    {
        goto __al_subscribe_failed;
    }

__al_subscribe:

    if (al_table.cnt < AL_SUBSCRIBE_CNT_MAX)
    {
        al_table.sub[al_table.cnt].sa  = target_addr;
        al_table.sub[al_table.cnt].pgn = target_pgn;
        al_table.sub[al_table.cnt].cb  = recv_cb;
        al_table.cnt++;
        return true;
    }
    
__al_subscribe_failed:
    return false;
}
