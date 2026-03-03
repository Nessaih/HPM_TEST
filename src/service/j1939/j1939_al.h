/////////////////////////////////////////////////////////////////////////////////////////
// File:         J1939_AL.h
// Module:       J1939 stack
// Description:  Defines for the J1939 stack Application Layer
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

#ifndef _J1939_AL_H_
#define _J1939_AL_H_

#include "j1939.h"




typedef void (*J1939_PGN_CALLBACK_T)(uint8_t *msg, uint16_t len, uint8_t src_addr, uint32_t pgn);

//========================================================================================
// Application Layer Interface Functions
//========================================================================================
 
void j1939_al_init(void);
void j1939_al_process(J1939_RX_MESSAGE_T *msg_ptr);
void j1939_al_periodic(void);
bool j1939_al_subscribe(uint8_t target_addr, uint32_t target_pgn, J1939_PGN_CALLBACK_T recv_cb);

#endif
