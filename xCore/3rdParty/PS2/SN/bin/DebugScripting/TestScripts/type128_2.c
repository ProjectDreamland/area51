//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: type128.c is an example that sets up a custom display for 128 
// bit types.
//                                                                           
//****************************************************************************
//
// Watch panes can already display 128 bit integers as 4 floats but hover-tooltip 
// evals cannot because they cannot pick from a range of display formats. They 
// have to go with the first one.
//
// But if we make the first one scripted we can make tooltips display the long128 
// however we like, e.g., this example makes the tooltips display "long long" 
// as four floats:
//

#include <stdio.h>
#include <stdlib.h>	// Needed for memcpy.
#include <SNscript.h>

char* MyInts(int unitnum, char* pexpr)
{
	snval_t	v;
	char*	err;
	static char tbuff[256];

	err = SNEvaluate(&v, unitnum, pexpr);		// Get the 128 bit integer to v.

	if(!err)
	{
		sprintf(tbuff, "%08X %08X %08X %08X", v.val.u128.word[0]
											, v.val.u128.word[1]
											, v.val.u128.word[2]
											, v.val.u128.word[3]	);

		return tbuff;
	}
	return NULL;	// Fail so that debugger will use default display.
}

char* MyFloats(int unitnum, char* pexpr)
{
	snval_t	v;
	char*	err;
	static char tbuff[256];

	float	fp[4];

	err = SNEvaluate(&v, unitnum, pexpr);		// Get the 128 bit integer to v.

	if(!err)
	{
		memcpy(fp, &(v.val.u128), 16);
		sprintf(tbuff, "%f %f %f %f", fp[0], fp[1], fp[2], fp[3]);

		return tbuff;
	}
	return NULL;	// Fail so that debugger will use default display.
}

//
// When this script is "RUN" it will start executing here.
// This just sets up the custom display formats for the specific types 
// that we are going to support.
//
int main(int argc, char** argv)
{
	static char* bigtype1 = "unsigned long long";
	static char* bigtype2 = "unsigned long128";
	static char* testtype = "class test";

	SNDelTypeHandlers(testtype);		// Make sure there are no previous handlers
	SNSetTypeDisplay( testtype, "[test]a=%d{@.m_a} b=%d{@.m_b} c=%d{@.m_c}");

	SNDelTypeHandlers(bigtype1);		// Make sure there are no previous handlers
	SNSetTypeDisplay( bigtype1, "[4floats]:MyFloats");	// so this will be the first
	SNSetTypeDisplay( bigtype1, "[4ints]:MyInts");		// but still have this option for watch panes.

	// Note that you can mix & match both the script-fn call templates and the text printf templates
	SNDelTypeHandlers(bigtype2);
	SNSetTypeDisplay( bigtype2, "[4floats]:MyFloats");	// these two will call the named script function
	SNSetTypeDisplay( bigtype2, "[4ints]:MyInts");
	SNSetTypeDisplay( bigtype2, "[two 64s]%016lx{@}, %016lx{@>>64}");	// and these two just specify a format
	SNSetTypeDisplay( bigtype2, "[two 64s dec]%ld{@}, %ld{@>>64}");
	return 0;
}
