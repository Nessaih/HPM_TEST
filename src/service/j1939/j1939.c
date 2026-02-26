/////////////////////////////////////////////////////////////////////////////////////////
// File:         J1939.c
// Module:       J1939 stack
// Description:  Main routines for the J1939 stack
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

//========================================================================================
// protocol stack Interface Functions
//========================================================================================

void j1939_stk_init(void)
{
    j1939_hal_init();
    j1939_dl_init();
    j1939_tl_init();
    j1939_nml_init();
    j1939_al_init();
}

void j1939_stk_periodic(void)
{
    j1939_hal_periodic();
    j1939_dl_periodic();
    j1939_tl_periodic();
    j1939_nml_periodic();
    j1939_al_periodic();
}
