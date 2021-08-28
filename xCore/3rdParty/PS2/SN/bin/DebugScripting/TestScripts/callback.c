//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: Using a debugger script callback function to hook a debugger event.
//                                                                          
//****************************************************************************
//
// ADB: Callback.c
//
// An example of the use of a debugger script callback function to hook a debugger event.
// This example hooks the SM_TARGETATN message, which occurs when the target changes state. 
// For example, when the target starts or stops executing code if it hits a breakpoint.
// Whenever the target changes state this code will dump a small amount of memory to the script pane.
//
#include <stdio.h>
#include <SNscript.h>

#define BYTES2GET 64

int MyCallback(SNPARAM message, SNPARAM param0, SNPARAM param1, SNPARAM param2)
{
	int 	i;
	char	qw[BYTES2GET];

	switch(message)
	{
	case SM_TARGETATN:	// message for "target update happened"
		if(param0==0)	// unit number 0 => EE
		{
			SNTxtHome();
			SNTxtClrEol();
			printf("MyCallback( MSG=%08x: %08x, %08x, %08x)\n", message, param0, param1, param2);
			SNTxtClrEol();
			puts("\n");
			SNTxtClrEol();

			// fetch some memory from EE
			SNGetMemory(qw, 0, 0x80100000, BYTES2GET);
			for(i=0; i<BYTES2GET; i++)
			{
				// print it out as bytes
				printf("%02X ", qw[i]&0xFF);
				if( (i&15) == 15)
				{
					SNTxtClrEol();
					puts("\n");
				}
			}
			SNTxtClrEop();
		}
		break;

	case SM_CHAR:
	default:
		;
	}
	return param0;
}

int main(int argc, char** argv)
{
	SNSetCallback("MyCallback");	// set our callback handler
	SNHookMessage(SM_TARGETATN);	// hook the "target update" message
	//SNTxtSetConsole(80, 50);		// put the pane into 2D charmap mode (default is TTY mode)

	SNTxtHome();	// home the cursor (in TTY mode this will discard the TTY buffer)
	//SNTxtClrEop();	// and clear to end of the window (only effective in 2D output mode)

	puts("\x1B\x20" "In GREEN: Now this script is waiting for a callback..." "\x1B\x1E");
	fflush(stdout);

	return 0;	// because main must return something
}
