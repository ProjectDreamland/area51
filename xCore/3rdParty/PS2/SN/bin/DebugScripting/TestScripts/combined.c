//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: This provides a quick example of how to load other scripts from this one and call them.
//                                                                          
//****************************************************************************
//
// The debugger will look for these included files in the current directory
// and in the include path. Therefore you must copy the scripts to the debugger's working
// directory for your project or put them in another directory and add it to your include
// path in autoexec.eic.
//
// Note that you will not be able to run this example unless you have the two C files below
// either on your search path or copied to your normal debugger script include directory.
//

#define __NO_MAIN		// use this to supress the main() functions in these included files
#include "kversion.c"	// see the #ifndef around main() for more details but if your scripts
#include "dumpmem.c"	// don't have main() then you don't need this precaution
#undef __NO_MAIN

int main(int argc, char** argv)
{
	// ANSI text foreground colours
	static char*   black="\x1B[" "30m";
	static char*     red="\x1B[" "31m";
	static char*   green="\x1B[" "32m";
	static char*   brown="\x1B[" "33m";
	static char*    blue="\x1B[" "34m";
	static char* magenta="\x1B[" "35m";
	static char*    cyan="\x1B[" "36m";

	SNTxtHome();

	puts(red);		// set text color
	RomVer();		// call code from kversion.c to print the PS2 kernel version
	puts("\n\n");

	puts(blue);		// set text colour
	dumpmem(0xBFC00100, 0xBFC00180, 4);	// call code from dumpmem.c to dump memory
	puts(black);	// set text colour

	return 0;	// to avoid a warning
}