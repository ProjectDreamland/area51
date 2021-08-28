//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: gdi.c shows a short example of some GDI function calls.
//                                                                          
//****************************************************************************
//
//
#include "SNScript.h"

int main(int argc, char**argv)
{
	int x,y;

	SNGetWindowSize(&x, &y);	// Request the size of the script pane.
	SNCreateBmp(x,y);		// Create a BITMAP overlay the same size.
	// Default is not stretched and overlayed with text layer.

	SNSetColor(255,255,0);					// R+G = yellow.
	SNEllipse(0, 0, x, y);					// Ellipse.

	SNSetColor(0,0,255);					// Blue.
	SNRectangle(x/3, y/3, x/3*2, y/3*2);	// Rectangle

	SNSetColor(255,0,0);					// Some GDI text output.
	SNSetFont(24, 0, "Arial");
	SNTextOut(250,50, "This is a TEST STRING!");

	return 0;
}
