//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: MyInt.c shows an example of the two ways to use scripts to setup 
// custom type handlers. The selected types will be displayed using custom display 
// templates. This works for watch and locals panes as well as "floating" tooltip 
// displays.
//
// Note the use of plain text templates in simple cases plus the use of a script callback
// for a more complex example.
//
// You can use either approach to setup custom display formats for your own types
// such as vector classes, important structures, unions etc.
// This can save you a lot of clicking to open expandable types in watch panes.
//                                                                          
//****************************************************************************
//
//
#include <stdio.h>
#include <string.h>
#include "SNscript.h"

//
// Our custom display handler.
// Input includes the text from the expression pane (or tooltip).
// Output is the string to be displayed.
//
// This script function will be the new display handler for type "int", it is 
// unlikely that anyone would ever want to display their ints Nibble-Reversed
// but you can see that just about anything is possible.
// This function could also use all the other script builtin functions
// to do things like evaluate other variables and fetch memory etc.
//
char* MyIntHandler(int unitnum, char* pexpr)
{
   	snval_t	result;
	char* pexperr = NULL;

	int		i,v;
	char*		pout;
	static char	tbuff[256];
	static char*	hexdigs = "0123456789ABCDEF";

	pexperr = SNEvaluate(&result, unitnum, pexpr);

	pout = tbuff;
	v = result.val.s32;
	for(i=0; i<8; i++)
	{
	   *pout++ = hexdigs[v&15];
	   v = v >> 4;
	}
	strcpy(pout, " (nibble reversed)");  			
	return tbuff;
}

//
// When the script is run, execution will begin here.
//
int main(int argc, char** argv)
{
	int	a,b,c,d,e;

	// Note: the thing in [] is an optional label for it which appears on the watch pane context menu.
	a = SNSetTypeDisplay("int", "[NIBREV]:MyIntHandler");
	b = SNSetTypeDisplay("class test", "[.members]a = %d{@.m_a}, b = %d{@.m_b}, c= %d{@.m_c}");
	c = SNSetTypeDisplay("class test *", "[->members]a = %d{@->m_a}, b = %d{@->m_b}, c= %d{@->m_c}");
	d = SNSetTypeDisplay("int", "[ABC]a = %d{a}, b = %d{b}, c= %d{c}");

	e = SNSetTypeDisplay("class test", "[string]pstr = %s{@.m_pstr}");

	printf("Custom int type handler is set (a=%d, b=%d, c=%d, d=%d, e=%d)\n", a,b,c,d,e);

	return 0;	// Main must return something.
}
