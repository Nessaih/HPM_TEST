/////////////////////////////////////////////////////////////////////////////////////////
// File:         J1939_TL.c
// Module:       J1939 stack
// Description:  J1939 stack Transport Layer
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

typedef struct
{
    J1939_RX_STATE_MACHINE_T machine;
    J1939_RX_PDU_T           pdu;
    J1939_RX_MESSAGE_T       msg;
} J1939_RX_CONTEXT_T;

static J1939_RX_CONTEXT_T J1939_rx_context[HA_CHANNEL_SIZE];

// static J1939_RX_MESSAGE_T       J1939_rx_context[channel].msg;
// static J1939_RX_STATE_MACHINE_T J1939_rx_state_machine;
// static J1939_RX_PDU_T           J1939_rx_context[channel].pdu;

//==========================================================================================
// Transport Layer Interface Functions
//==========================================================================================

void j1939_tl_init(void)
{
    memset(J1939_rx_context, 0, sizeof(J1939_rx_context));

    for (uint8_t i = 0; i < HA_CHANNEL_SIZE; i++)
    {
        J1939_rx_context[i].machine.status = WAIT_FOR_MESSAGE;
        J1939_rx_context[i].machine.TP     = TP_NONE;
    }
}

void j1939_tl_process(uint8_t channel, J1939_RX_PDU_T *pdu_ptr)
{
    if ((pdu_ptr->PGN == TP_CM) || (pdu_ptr->PGN == TP_DT))
    {
        J1939_rx_context[channel].pdu.PGN         = pdu_ptr->PGN;
        J1939_rx_context[channel].pdu.dest_addr   = pdu_ptr->dest_addr;
        J1939_rx_context[channel].pdu.source_addr = pdu_ptr->source_addr;
        J1939_rx_context[channel].pdu.byte_count  = pdu_ptr->byte_count;

        memcpy((void *)J1939_rx_context[channel].pdu.data, (void *)pdu_ptr->data, NUMBER_PDU_BUFFERS);
    }
    else
    {
        J1939_RX_MESSAGE_T msg;

        msg.PGN         = pdu_ptr->PGN;
        msg.dest_addr   = pdu_ptr->dest_addr;
        msg.source_addr = pdu_ptr->source_addr;
        msg.byte_count  = pdu_ptr->byte_count;

        memcpy((void *)msg.data, (void *)pdu_ptr->data, NUMBER_PDU_BUFFERS);

        // send message up the stack to the Network Management Layer
        j1939_nml_process(&msg);
    }
}

void j1939_tl_periodic(void)
{
    uint8_t go_on;

    for (uint8_t channel = 0; channel < HA_CHANNEL_SIZE; ++channel)
    {
        go_on = true;

        J1939_rx_context[channel].machine.timer_counter++;

        while (go_on)
        {

            switch (J1939_rx_context[channel].machine.status)
            {
            case WAIT_FOR_MESSAGE:
            {
                if (J1939_rx_context[channel].pdu.PGN == TP_CM &&
                    (J1939_rx_context[channel].pdu.dest_addr == GLOBADDR || J1939_rx_context[channel].pdu.dest_addr == NODEADDR))
                {
                    if (J1939_rx_context[channel].pdu.data[0] == TP_CM_RTS || J1939_rx_context[channel].pdu.data[0] == TP_CM_BAM)
                    {
                        uint32_t volatile_pgn;
                        uint32_t volatile_byte_count;

                        volatile_pgn                                          = J1939_rx_context[channel].pdu.data[6];
                        volatile_pgn                                          = volatile_pgn << 8;
                        volatile_pgn                                          = volatile_pgn + J1939_rx_context[channel].pdu.data[5];
                        volatile_byte_count                                   = J1939_rx_context[channel].pdu.data[2];
                        volatile_byte_count                                   = volatile_byte_count << 8;
                        volatile_byte_count                                   = volatile_byte_count + J1939_rx_context[channel].pdu.data[1];
                        J1939_rx_context[channel].machine.PGN                 = volatile_pgn;
                        J1939_rx_context[channel].machine.dest_addr           = J1939_rx_context[channel].pdu.dest_addr;
                        J1939_rx_context[channel].machine.source_addr         = J1939_rx_context[channel].pdu.source_addr;
                        J1939_rx_context[channel].machine.packet_number       = 0;
                        J1939_rx_context[channel].machine.timer_counter       = 0;
                        J1939_rx_context[channel].machine.total_packet_number = J1939_rx_context[channel].pdu.data[3];
                        J1939_rx_context[channel].machine.byte_count          = volatile_byte_count;
                        J1939_rx_context[channel].machine.status              = INIT_REASSEMBLE_STRUCTURE;
                        J1939_rx_context[channel].machine.TP                  = J1939_rx_context[channel].pdu.data[0];
                    }
                    else
                    {
                        go_on = false; // break;
                    }
                }
                else
                {
                    go_on = false; // break;
                }
                break;
            }

            case INIT_REASSEMBLE_STRUCTURE:
            {
                if (J1939_rx_context[channel].machine.TP == TP_CM_RTS)
                {
                }
                else if (J1939_rx_context[channel].machine.TP == TP_CM_BAM)
                {
                    if (J1939_rx_context[channel].machine.byte_count < NUMBER_TRANS_RX_BUFFERS)
                    {
                        J1939_rx_context[channel].msg.PGN         = J1939_rx_context[channel].machine.PGN;
                        J1939_rx_context[channel].msg.dest_addr   = J1939_rx_context[channel].machine.dest_addr;
                        J1939_rx_context[channel].msg.source_addr = J1939_rx_context[channel].machine.source_addr;
                        J1939_rx_context[channel].msg.byte_count  = J1939_rx_context[channel].machine.byte_count;

                        memset((void *)J1939_rx_context[channel].msg.data, 0, NUMBER_TRANS_RX_BUFFERS);

                        J1939_rx_context[channel].machine.status = WAIT_FOR_DATA;
                    }
                    else
                    {
                        J1939_rx_context[channel].machine.status        = WAIT_FOR_MESSAGE;
                        J1939_rx_context[channel].machine.timer_counter = 0;
                        go_on                                           = false;
                        // break;
                    }
                }
                else
                {
                    J1939_rx_context[channel].machine.status        = WAIT_FOR_MESSAGE;
                    J1939_rx_context[channel].machine.timer_counter = 0;
                    go_on                                           = false;
                    // break;
                }
                break;
            }

            case CHECK_PACKET:
                break;

            case SEND_ABORT:
                break;

            case SEND_CTS_WITH_COUNT:
                break;

            case WAIT_FOR_DATA:
            {
                if (J1939_rx_context[channel].pdu.PGN == TP_DT)
                {
                    J1939_rx_context[channel].machine.status = CHECK_DATA_PACKET;
                    // break;
                }
                else
                {
                    J1939_rx_context[channel].machine.status = CHECK_TIMER;
                    // break;
                }
                break;
            }

            case CHECK_TIMER:
            {
                if (J1939_rx_context[channel].machine.timer_counter > (750 / J1939_PERIODIC_TICK)) // 750ms
                {
                    J1939_rx_context[channel].machine.status = RESET_REASSEMBLY_STRUCTURE;
                    // J1939_rx_context[channel].machine.timer_counter = 0;
                    // break;
                    // printf("TP_TD timeout\n");
                }
                else
                {
                    J1939_rx_context[channel].machine.status = WAIT_FOR_DATA;
                    go_on                                    = false; // break;
                }
                break;
            }

            case RESET_REASSEMBLY_STRUCTURE:
            {
                memset(&J1939_rx_context[channel].pdu, 0, sizeof(J1939_rx_context[channel].pdu));
                memset(&J1939_rx_context[channel].machine, 0, sizeof(J1939_rx_context[channel].machine));
                J1939_rx_context[channel].machine.status = WAIT_FOR_MESSAGE;
                go_on                                    = false;
                break;
            }
            case CHECK_DATA_PACKET:
            {
                if (J1939_rx_context[channel].machine.TP == TP_CM_BAM)
                {
                    uint8_t volatile_pdu_src_addr = J1939_rx_context[channel].pdu.source_addr;
                    uint8_t volatile_pdu_dst_addr = J1939_rx_context[channel].pdu.dest_addr;

                    if (volatile_pdu_src_addr == J1939_rx_context[channel].machine.source_addr &&
                        volatile_pdu_dst_addr == J1939_rx_context[channel].machine.dest_addr)
                    {
                        uint8_t volatile_packet_number = J1939_rx_context[channel].machine.packet_number;

                        if (volatile_packet_number == J1939_rx_context[channel].pdu.data[0] - 1)
                        {
                            J1939_rx_context[channel].machine.status        = SAVE_DATA;
                            J1939_rx_context[channel].machine.timer_counter = 0;
                            J1939_rx_context[channel].pdu.data[0]           = 0;
                        }
                        else if (J1939_rx_context[channel].pdu.data[0] != 0)
                        {
                            J1939_rx_context[channel].machine.status = RESET_REASSEMBLY_STRUCTURE;
                        }
                        else
                        {
                            J1939_rx_context[channel].machine.status = WAIT_FOR_DATA;
                            go_on                                    = false;
                        }
                    }
                    else
                    {
                        J1939_rx_context[channel].machine.status = WAIT_FOR_DATA;
                        go_on                                    = false;
                    }
                }
                else if (J1939_rx_context[channel].machine.TP == TP_CM_RTS)
                {
                }
                else
                {
                }
                break;
            }

            case SAVE_DATA:
            {
                uint8_t i                      = 0;
                uint8_t volatile_packet_number = J1939_rx_context[channel].machine.packet_number * 7 + i;

                while ((i < 7) && (volatile_packet_number <= J1939_rx_context[channel].machine.byte_count))
                {
                    volatile_packet_number                        = J1939_rx_context[channel].machine.packet_number * 7 + i;
                    J1939_rx_context[channel].msg.data[volatile_packet_number] = J1939_rx_context[channel].pdu.data[i + 1];
                    J1939_rx_context[channel].pdu.data[i + 1]     = 0;
                    i++;
                }
                J1939_rx_context[channel].machine.packet_number++;
                volatile_packet_number = J1939_rx_context[channel].machine.packet_number;
                if (volatile_packet_number == J1939_rx_context[channel].machine.total_packet_number)
                {
                    J1939_rx_context[channel].machine.status = FILL_USER_MESSAGE;
                }
                else
                {
                    J1939_rx_context[channel].machine.status = WAIT_FOR_DATA;
                    go_on                                    = false;
                }
                break;
            }

            case SEND_EOM:

            case SEND_CTS:

            case FILL_USER_MESSAGE:
            {
                J1939_rx_context[channel].machine.status        = RESET_REASSEMBLY_STRUCTURE;
                J1939_rx_context[channel].machine.timer_counter = 0;

                // send message up the stack to the Network Management Layer
                j1939_nml_process(&J1939_rx_context[channel].msg);

                // go_on = false;
                break;
            }

            default:
                go_on = false;
                break;

            } // end switch
        } // end while
    } // end for
}

uint8_t j1939_tl_send(J1939_TX_MESSAGE_T *msg_ptr)
{
    if (msg_ptr->byte_count > 8)
    {
#if NOT_YET
        // Transport layer for transmission of J1939 messages > 8 bytes not implemented yet
#endif
    }
    else
    {
        j1939_dl_tx(msg_ptr);
    }

    return true;
}
