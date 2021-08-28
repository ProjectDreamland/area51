//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: This is one way to hook the "Edit Here" function of the source pane
// so that it will invoke your own editor. When you type ctrl-E
// or select "edit in visual studio" from the source pane context menu,
// the debugger will attempt to launch an external editor.
//                                                                          
//****************************************************************************
//
// If this script function exists then it will be called to invoke the editor, or 
// if the [opentemplate] string is set in the debugger config it will use that,
// or it will try to use COM to talk to MS Developer Studio to edit the file.
//
// This simple example is for CodeWright and is the script equivalent of having
// the following two lines near the start of your debugger config file.
//
// [opentemplate]
// c:\cw32\cw32.exe %f -G%l
//
// Note: if you want to use the template rather than a script then you should add
// those lines to your default config (they will be used if no local config is present)
// as well as your local config file.
//

#include <stdio.h>
#include <SNscript.h>

//
// This function must be:
//
//	int SN_EditThisFile(char* pathname, int linenumber, int column)
//
// if it is to match the debugger requirements. Otherwise errors will be displayed
// in the DBG TTY stream.
//
int SN_EditThisFile(char* pathname, int linenumber, int column)
{
	char tempbuffer[256];	// arbitrary buffer for temp storage of parameter string

	// The script will be executed in the global (no pane) context.
	// So stdout will go to the debugger's DBG TTY stream.
	printf("OK, so you want to edit line %d of file %s\n", linenumber, pathname);

	// This example is for the CodeWright editor.
	sprintf(tempbuffer, "%s -G%d", pathname, linenumber);
	ShellExecute("open", "c:\\cw32\\cw32.exe", tempbuffer, SW_SHOW);
	return 0;	// return value is currently ignored
}