// ***************************************************************************
//
// Copyright 2003 SN Systems Ltd
//
// Description: Code to initiate ProView
//
// ***************************************************************************
//
// Change History:
//
// Vers  Date            Author          Changes
// 1.00  10-Apr-2003     Tom             First created.
// 1.01  05-Dec-2003     Tom             Now pass SCE_LIBRARY_VERSION to ProView().
//
// ***************************************************************************

#include <sifdev.h>
#include <libver.h>
#include "ProView2.h"

int main(int argc, char * argv[])
{
	return ProView(argc, argv, IOP_IMAGE_FILE, SCE_LIBRARY_VERSION);
}
