/////////////////////////////////////////////////////////////////////////////////////////
// File:         J1939.h
// Module:       J1939 stack
// Description:  Defines for the J1939 stack
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

#ifndef _J1939_H_
#define _J1939_H_

// #include "j1939_includes.h"
#define J1939STACK_VERSION          131

// TODO - Command Request/Ack Parameter Group Numbers
#define REQUEST_PGN                 0x00EA00
#define REQUEST_PGN_NUM             15
#define ACK_PGN                     0x00E800
#define ACK_PGN_ACK                 0
#define ACK_PGN_NACK                1
#define ACK_PGN_ACCESS_DENIED       2
#define ACK_PGN_CANNOT_RESPOND      3

// TODO - Proprietary Parameter Group Numbers
#define PROPRIETARY_A               0x00EF00
#define PROPRIETARY_A2              0x01EF00
#define PROPRIETARY_B_START         0x00FF00
#define PROPRIETARY_B_END           0x00FFFF

// Receive state machine
#define WAIT_FOR_MESSAGE            0
#define INIT_REASSEMBLE_STRUCTURE   1
#define CHECK_PACKET                2
#define SEND_ABORT                  3
#define SEND_CTS_WITH_COUNT         4
#define WAIT_FOR_DATA               5
#define CHECK_TIMER                 6
#define RESET_REASSEMBLY_STRUCTURE  7
#define CHECK_DATA_PACKET           8
#define SAVE_DATA                   9
#define SEND_EOM                    10
#define SEND_CTS                    11
#define FILL_USER_MESSAGE           12

// Hardware Abstraction Layer
#define HA_CHANNEL_SIZE             2

// Data Link Layer
#define DL_SA_MASK                  0xFF
#define DL_PGN_MASK                 0x3FFFF

// Transport Protocol Layer
#define TP_CM                       0x00EC00
#define TP_CM_RTS                   16
#define TP_CM_CTS                   17
#define TP_CM_END_OF_MSG_ACK        19
#define TP_CM_CONN_ABORT            255
#define TP_CM_BAM                   32
#define TP_DT                       0x00EB00
#define TP_NONE                     0

// TODO - Network Management Layer - PGNs
#define REQUEST_FOR_ADDRESS_CLAIMED 0x00EA00
#define ADDRESS_CLAIMED             0x00EE00
#define CANNOT_CLAIM                0x00EE00
#define COMMANDED_ADDRESS           0x00FED8

// Diagnostic Trouble Code (DTC)
#define PRIMED                      1
#define NOTPRIMED                   0

#define J1939_PERIODIC_TICK         10 // 10ms
#define J1939_MAX_MESSAGE_LENGTH    1785
#define NUMBER_TRANS_RX_BUFFERS     128
#define NUMBER_TRANS_TX_BUFFERS     8
#define NUMBER_PDU_BUFFERS          8
#define CAN_MAX_BYTE_COUNT          8

#define NODEADDR                    0xD8
#define GLOBADDR                    0xFF
#define NULLADDR                    0xFE

typedef struct
{
    volatile uint32_t PGN;
    volatile uint8_t  data[NUMBER_TRANS_TX_BUFFERS];
    volatile uint16_t byte_count;
    volatile uint8_t  priority;
    volatile uint8_t  dest_addr;
    volatile uint8_t  sa_addr;
    volatile uint8_t  channel;
    volatile int8_t   status;
} J1939_TX_MESSAGE_T;

typedef struct
{
    volatile uint32_t PGN;
    volatile uint8_t  data[NUMBER_TRANS_RX_BUFFERS];
    volatile uint16_t byte_count;
    volatile uint8_t  source_addr;
    volatile uint8_t  dest_addr;
} J1939_RX_MESSAGE_T;

typedef struct
{
    volatile uint32_t PGN;
    volatile uint8_t  data[NUMBER_PDU_BUFFERS];
    volatile uint16_t byte_count;
    volatile uint8_t  source_addr;
    volatile uint8_t  dest_addr;
} J1939_RX_PDU_T;

typedef struct
{
    volatile uint32_t PGN;
    volatile uint8_t  channel;
    volatile uint8_t  status;
    volatile uint8_t  packet_number;
    volatile uint8_t  total_packet_number;
    volatile uint16_t byte_count;
    volatile uint16_t timer_counter;
    volatile uint8_t  source_addr;
    volatile uint8_t  dest_addr;
    volatile uint8_t  TP;
} J1939_RX_STATE_MACHINE_T;

typedef struct
{
    volatile uint32_t identifier;
    volatile uint8_t  data[CAN_MAX_BYTE_COUNT];
    volatile uint8_t  byte_count;
    volatile uint8_t  channel;
} CAN_PACKET_T;

typedef struct
{
    CAN_PACKET_T     *buffer;
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t buffer_size;
} RING_T;

typedef struct
{
    uint8_t dtc_len;
    uint8_t dtc_cnt;
    uint8_t dtc[128];
} J1939_DM1_DTC;

//========================================================================================
// J1939 protocol stack Interface Functions
//========================================================================================
void j1939_stk_init(void);
void j1939_stk_periodic(void);

#endif
