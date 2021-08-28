//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: dosteps.c provides an example of the use of the STEP/STEVOVER functions in a debugger script
// It also shows the use of a callback to hook TARGETATN events.
//                                                                          
//****************************************************************************
//
//
// Note that this script module has no main() function so if you run this it will just load the functions
// in here but nothing will be called. To invoke the example after loading this script you can
// just enter a function call in immediate mode in a script pane.
//
// e.g. nstep(int count, int sl)
//
// int	count	is the number of steps to execute
// int	sl	is a BOOL flag to indicate whether disasm (0) or source (1) level
//
// nstep(50,0);	will cause the debugger to execute 50 disasm level steps
//
#include "snscript.h"

// The following global variables will be used to communicate between main script functions and callback code.
int	_nsteps;
int	_maxsteps;
int	_slflag;

//
// Here is the callback function.
// It uses the SM_TARGETATN callback to get control every time the target CPU stops.
//
int SUCallback(SNPARAM message, SNPARAM param0, SNPARAM param1, SNPARAM param2)
{
	snval_t	result;
	char* pexperr = 0;

	//printf("MyCallback( MSG=%08x: %08x, %08x, %08x)\n", message, param0, param1, param2);

	if(message == SM_TARGETATN)
	{
		if(param0 == 0 && !SNIsRunning(0) )	// if it's the main core CPU and it is stopped
		{
			pexperr = SNEvaluate(&result, 0, "($cause>>2)&0x1F");	// check what exception caused the update
			// perhaps we should fetch and check the opcode also to see if it is a break op and stop if it is.
			if(!pexperr && result.val.u32 == 9)
			{
				// SNUpdateWindows(M_ALL, 0);
				// printf("stepping, count=%d\n", _nsteps);
				if(++_nsteps < _maxsteps)
				{
					// printf("stepping again\n");
					if(SNStep(0, _slflag))
						return 0;	// succesful SL step so just return
				}
			}
			// if it gets here we have either completed enough steps
			//  or there is an exception (other than break)
			//  or the source level step failed
			SNSetCallback(0); // so cancel any callback and return the pane to immediate mode
		}
	}
	return 0;
}

//
// An example of how to use the new SNStep functionality in PS2DBG 1.74.12 onwards.
// There is no main. This script just adds the function below.
// After launching this script you can enter "nstep(5,1)" in immediate mode
// to perform 5 source-level single-steps.
//
void nstep(int count, int sl)
{
	SNSetCallback("SUCallback");	// set our callback handler
	SNHookMessage(SM_TARGETATN);	// hook the "target update" message

	_maxsteps = count;
	_slflag = sl;
	_nsteps = 0;
	SNStep(0, _slflag);
}
