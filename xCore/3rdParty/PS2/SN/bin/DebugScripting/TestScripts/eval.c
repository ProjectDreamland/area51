//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: eval.c shows an example of the use of the builtin function SNEvaluate()
// to evaluate expressions in the context of the target CPU.
//                                                                          
//****************************************************************************
//
// 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SNscript.h>

void dumpreg(char* regname)
{
	snval_t	result;
	unsigned int* addr;
	char* pexperr = NULL;
	unsigned int * pquad;
	char	namebuff[16] = {'$', 0};

	strcpy(namebuff+1, regname);

	pexperr = SNEvaluate(&result, 0, namebuff);

	if(pexperr)	// if error
		printf("%s: %s\n", regname, pexperr);
	else
	{
		pquad = result.val.u128.word;
		printf("%s = %08X_%08X_%08X_%08X\n", regname, pquad[3], pquad[2], pquad[1], pquad[0]);
	}
}

int main(int argc, char** argv)
{
	snval_t	result;
	char* pexperr = NULL;
	char* fn_name = "main";

	SNTxtHome();	// Home the cursor (in TTY mode this also clears the TTY scrollback buffer).

	dumpreg("at");
	dumpreg("v0");
	dumpreg("a0");
	dumpreg("a1");
	dumpreg("a2");
	dumpreg("a3");
	dumpreg("z0");

	pexperr = SNEvaluate(&result, 0, fn_name);

	if(pexperr)	// if error
		printf("Expression \"%s\" Error: %s\n", fn_name, pexperr);
	else
		printf("main: type=%08X  address=%08X\n", result.type, result.address);

	return 0;
}
