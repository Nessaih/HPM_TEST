/////////////////////////////////////////////////////////////////////////////////////////
// File:         J1939_HAL.c
// Module:       J1939 stack
// Description:  J1939 stack Hardware Abstraction Layer
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
#include <string.h>
#include <stdint.h>
#include "task.h"
#include "can_socket.h"
#include "j1939_includes.h"
#include "can/can_data.h"
#include "can/can_tp.h"

//==========================================================================================
// Hardware Abstraction Layer Interface Functions
//==========================================================================================

void j1939_hal_init(void)
{
}

void j1939_hal_periodic(void)
{
}

void j1939_hal_tx(const CAN_PACKET_T *pkt_ptr)
{

    CAN_DATA msg;

    if (pkt_ptr->identifier > 0x7FF)
        msg.msg.is_eid = 1;
    else
        msg.msg.is_eid = 0;
    msg.instance     = 0;
    msg.msg.can_id   = pkt_ptr->identifier;
    msg.msg.data_len = pkt_ptr->byte_count;
    memcpy((void *)msg.msg.data, (void *)pkt_ptr->data, pkt_ptr->byte_count);
    can_tp_send(&msg);
}
void j1939_hal_rx(unsigned char instance, CAN_MSG *msg)
{
    CAN_PACKET_T rev_can_packet;

    (void)instance;

    rev_can_packet.identifier = msg->can_id;
    rev_can_packet.byte_count = msg->data_len;
    memcpy((void *)rev_can_packet.data, (void *)msg->data, msg->data_len);
#if 0
    printf("ID:0x%08X %u %02X %02X %02X %02X %02X %02X %02X %02X\n", \
            rev_can_packet.identifier, \
            rev_can_packet.byte_count, \
            rev_can_packet.data[0], \
            rev_can_packet.data[1], \
            rev_can_packet.data[2], \
            rev_can_packet.data[3], \
            rev_can_packet.data[4], \
            rev_can_packet.data[5], \
            rev_can_packet.data[6], \
            rev_can_packet.data[7]);
#endif
    j1939_dl_process(&rev_can_packet);
}
