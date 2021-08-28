//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: This scripting example shows how scripts can be used to extend 
// the functionality of the debugger. It shows two ways to hook custom menu items
// to script code.
//                                                                          
//****************************************************************************
//
// 
#include <stdio.h>
#include <SNscript.h>

//
// Because this menu item is bound directly to a function it will just use
// the default global context for stdout etc. i.e., stdout will go to the DBG TTY
// rather than to a pane.
//
void cbfn(SNPARAM hwnd, SNPARAM cmd)
{
	char	mainbuff[256];
	char	subbuff[256];
	char	titlebuff[256];
	PANEINFO pi;

	printf("HWND=%08X, CMD=%d, ", hwnd, cmd);

	// Use the window handle to get other useful information.
	if(SNGetPaneInfo(hwnd, &pi))
		printf("PANETYPE=%d, CURSOR=%08X ", pi.panetype, pi.cursaddr);

	if(SNGetMenuInfo(pi.panetype, cmd, mainbuff, subbuff))
	{
		printf("Menu=<%s>", mainbuff);
		if(*subbuff)
			printf("-><%s>", subbuff);
	}

	if(SNGetPaneTitle(hwnd, titlebuff))
		printf("  Title=<%s>", titlebuff);

	puts("\n");
}

//
// This is a callback, therefore it will have a constant context (default to this pane)
// for stdout etc. and can setup bitmaps and do GDI output.
// Note that scripts that start up scripts do not normally have a pane and so are
// run in the default global context
//
// If you want to do graphics or other output to a script pane then you must
// launch this in a script pane (using "run script") and not in a startup script.
//
// message = callback comamnd number (SM_MENU in case of context menu called)
// param0  = hwnd
// param1  = command (unique to each menu item)
// param2  = pane type (see WT_ defines in SNScript.h)
//
// This example callback merely passes the information onto the "cbfn" function above.
// The same callback could handle other messages like SM_TARGETATN as required.
//
int MyCallback(SNPARAM message, SNPARAM param0, SNPARAM param1, SNPARAM param2)
{
	//printf("MyCallback( MSG=%08x: %08x, %08x, %08x)\n", message, param0, param1, param2);
	if(message == SM_MENU)
		cbfn(param0, param1);
	return 1;
}

int main(int argc, char** argv)
{
	// setup our callback
	SNSetCallback("MyCallback");	// Set our callback handler.
	SNHookMessage(SM_MENU);		// Hook any SM_MENU context menu messages.
	//SNHookMessage(SM_TARGETATN);	// Hook the "target update" message also?

	// Add some custom menus to the memory pane.
	// These ones call the callback above.
	SNAddMenu(WT_MEMORY, "Custom Menu 1", 0, 0);
	SNAddMenu(WT_MEMORY, "Custom Menu 2", 0, 0);
	SNAddMenu(WT_MEMORY, "Custom Menu 3", "submenu 1", 0);
	SNAddMenu(WT_MEMORY, "Custom Menu 3", "submenu 2", 0);
	SNAddMenu(WT_MEMORY, "Custom Menu 3", "submenu 3", 0);

	// These two however call the cbfn() directly, this means that they have no 
        // permanent context to run in.
	SNAddMenu(WT_MEMORY, "Custom Menu 4", 0, "cbfn");
	SNAddMenu(WT_MEMORY, "Custom Menu 5", 0, "cbfn");

	SNTxtHome();
	SNTxtClrEop();	// Clear and home the cursor.

	puts("Several new entries have been added to the Memory pane context menu.\n");
	puts("This script pane is now waiting for the callback messages.\n");
	puts("If you invoke the custom menu items you will see callbacks happen\n");
	// Note that because callbacks were hooked the pane will stay in "execute" mode, 
        // waiting for messages from the callback. If all had been hooked to functions
	// instead, then even if launched in a pane this would just return to immediate mode.
	return 0;
}
