/////////////////////////////////////////////////////////////////////////////////////////
// File:         J1939_DL.c
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

list_define(dl_rx_list_t, CAN_PACKET_T, 32);
list_define(dl_tx_list_t, CAN_PACKET_T, 8);

static struct dl_rx_list_t rx_list;
static struct dl_tx_list_t tx_list;

uint8_t dl_state;

//========================================================================================
// Datalink Layer Interface Functions
//========================================================================================

void j1939_dl_init(void)
{

    list_init(&rx_list);
    list_init(&tx_list);

    dl_state = NOTPRIMED;

    j1939_pgn_init();
}

// #pragma CODE_SEG NON_BANKED
void j1939_dl_process(const CAN_PACKET_T *pkt)
{
    uint8_t  addr;
    uint32_t pgn;

    addr = (uint8_t)pkt->identifier & DL_SA_MASK;
    pgn  = (pkt->identifier >> 8) & DL_PGN_MASK;

    if (!j1939_pgn_filter(addr, pgn))
        return;

    if (list_is_full(&rx_list))
        return;

    list_put(&rx_list, *pkt);
}

// #pragma CODE_SEG NON_BANKED
void j1939_dl_periodic(void)
{
    uint8_t        m_PF, m_PS, m_DA, m_SA, m_CH;
    uint32_t       m_ID, m_PGN;
    CAN_PACKET_T   rpkt;
    CAN_PACKET_T   tpkt;
    J1939_RX_PDU_T pdu;

    if (PRIMED == dl_state)
    {
        if (list_is_empty(&tx_list))
        {
            dl_state = NOTPRIMED;
        }
        else
        {
            list_get(&tx_list, &tpkt);
            j1939_hal_tx(&tpkt);
        }
    }

    if (list_is_empty(&rx_list))
        return;

    list_get(&rx_list, &rpkt);

    m_CH  = rpkt.channel;
    m_ID  = rpkt.identifier;
    m_PGN = (uint32_t)((m_ID >> 8) & DL_PGN_MASK);
    m_PF  = (uint8_t)(m_ID >> 16);
    m_PS  = (uint8_t)(m_ID >> 8);
    m_DA  = m_PS;
    m_SA  = (uint8_t)m_ID;

    // Handle PDU1
    if (m_PF < 240)
    {
        if (m_DA != GLOBADDR && m_DA != NODEADDR)
        {
            return;
        }
        pdu.PGN         = m_PGN & 0xFF00;
        pdu.byte_count  = rpkt.byte_count;
        pdu.dest_addr   = m_DA;
        pdu.source_addr = m_SA;
        memcpy((void *)pdu.data, (void *)rpkt.data, pdu.byte_count);
    }
    // Handle PDU2
    else
    {
        pdu.PGN         = m_PGN;
        pdu.byte_count  = rpkt.byte_count;
        pdu.dest_addr   = GLOBADDR;
        pdu.source_addr = m_SA;
        memcpy((void *)pdu.data, (void *)rpkt.data, pdu.byte_count);
    }
    // send PDU up the stack to the Transport Layer
    j1939_tl_process(m_CH, &pdu);
}

void j1939_dl_tx(J1939_TX_MESSAGE_T *msg_ptr)
{
    CAN_PACKET_T pkt;
    uint32_t     temp_identifier;

    pkt.channel     = msg_ptr->channel;
    pkt.byte_count  = (uint8_t)msg_ptr->byte_count;
    pkt.identifier  = msg_ptr->priority;
    temp_identifier = pkt.identifier << 18;
    pkt.identifier  = temp_identifier + msg_ptr->PGN;

    // PDU1 - Peer-to-Peer
    if (msg_ptr->PGN < 0xF000)
    {
        temp_identifier = pkt.identifier;
        pkt.identifier  = temp_identifier + msg_ptr->dest_addr;
    }

    temp_identifier = pkt.identifier << 18;
    pkt.identifier  = temp_identifier + msg_ptr->sa_addr;

    for (uint8_t i = 0; i < pkt.byte_count; i++)
    {
        pkt.data[i] = msg_ptr->data[i];
    }

    if (list_is_full(&tx_list))
        return;
    list_put(&tx_list, pkt);

    dl_state = PRIMED;
}