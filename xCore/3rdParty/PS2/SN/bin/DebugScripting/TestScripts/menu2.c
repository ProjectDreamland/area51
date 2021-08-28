//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: This small, graphical example shows how you can use scripts to 
// extend the functionality of the debugger.
//                                                                          
//****************************************************************************
//
//

#include <stdio.h>	// For printf etc.
#include <string.h>	// For strcmp.
#include <SNscript.h>	// For SN builtin functions.

int g_xwin,g_ywin;	// Globals to store the pane size in.
//
// Because this is a callback it will have a constant context (default to this pane)
// for stdout etc. and can therefore setup bitmaps and do GDI output.
// Note that scripts that start up scripts do not normally have a pane and so are
// run in the default global context.
//
// If you want to do graphics or other output to a script pane then you must
// launch this in a script pane (using "run script") and not in a startup script.
//
// message = callback comamnd number (SM_MENU in case of context menu called).
// param0  = hwnd.
// param1  = command (unique to each menu item).
// param2  = pane type (see WT_ defines in SNScript.h).
//
int MyCallback(SNPARAM message, SNPARAM param0, SNPARAM param1, SNPARAM param2)
{
	unsigned int hwnd, cmd, panetype;
	char	mainbuff[256];

	printf("MyCallback( MSG=%08x: %08x, %08x, %08x)\n", message, param0, param1, param2);
	if(message == SM_MENU)
	{
		hwnd = param0;	// just to make it more legible
		cmd  = param1;
		panetype = param2;

		if(SNGetMenuInfo(panetype, cmd, mainbuff, 0))	// note 0 => we do not need the submenutext
		{
			printf("Menu = %s\n", mainbuff);
			if(strcmp(mainbuff, "Rectangle") == 0)
			{
				SNClearBmp();
				SNSetColor(0,0,255);
				SNRectangle(g_xwin/3, g_ywin/3, g_xwin/3*2, g_ywin/3*2);
			}
			else if(strcmp(mainbuff, "Ellipse") == 0)
			{
				SNClearBmp();
				SNSetColor(255,255,0);
				SNEllipse(0, 0, g_xwin, g_ywin);
			}
			else
			{
				SNClearBmp();
				SNSetColor(255,0,0);
				SNSetFont(24, 0, "Arial");
				SNTextOut(50,50, "Not Rectangle or Ellipse!");
			}
		}

	}
	return 1;
}

int main(int argc, char** argv)
{
	// First give the pane a plain ordinary bitmap.
	SNGetWindowSize(&g_xwin, &g_ywin);
	SNCreateBmp(g_xwin, g_ywin);

	// Setup our callback.
	SNSetCallback("MyCallback");	// Set our callback handler.
	SNHookMessage(SM_MENU);		// Hook any Context Menu messages.

	// Add some custom menus to the memory pane.
	// These ones call the callback above.
	SNAddMenu(WT_MEMORY, "Rectangle", 0, 0);
	SNAddMenu(WT_MEMORY, "Ellipse", 0, 0);
	SNAddMenu(WT_MEMORY, "Some GDI Text", 0, 0);

//	SNTxtHome();
//	SNTxtClrEop();	// Clear and home the cursor.

	puts("This script pane is now waiting for callback messages\n");
	puts("So go ahead and pick one of the new items on the memory pane context menu\n");
	// Note that because callbacks were hooked the pane will stay in "execute" mode
	// waiting for messages from the callback.
	return 0;
}
