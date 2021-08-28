//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: update.c is an example script that forces the update of EE 
// register panes if VU0 or VU1 change execution state.
//                                                                          
//****************************************************************************
//
//
// You can run this in a script pane but this is probably more appropriately
// called as a startup script, (which runs in a default global context that
// is not associated with a script pane) because it does not need a dedicated
// pane for output.
//
#include <stdio.h>
#include "SNscript.h"

int MyCallback(SNPARAM message, SNPARAM param0, SNPARAM param1, SNPARAM param2)
{
	static int count = 0;
	if(message == SM_TARGETATN && param0 == 1 || param0 == 2)	// If it's an update of VU0 or VU1.
	{
		printf("Callback Count = %d\n", ++count);
		SNUpdateWindows(M_REGISTERS, 0);	// Update all EE register panes.
	}
	return 0;
}

int main(int argc, char** argv)
{
	SNSetCallback("MyCallback");		// Set our callback handler.
	SNHookMessage(SM_TARGETATN);		// Hook the "target update" message.
	MyCallback(SM_TARGETATN, 2,0,0);	// Execute it once as a test.
	return 0;
}
