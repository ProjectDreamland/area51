//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: timer.c shows how to use a callback to handle custom timer events.
//                                                                          
//****************************************************************************
//
//
#include <stdio.h>
#include <SNscript.h>

//
// This is the callback function that gets called by the timer events.
// It just increments a counter and displays the result.
//
int MyCallback(SNPARAM message, SNPARAM param0, SNPARAM param1, SNPARAM param2)
{
	static int count = 0;

	switch(message)
	{
	case SM_TIMER:
		count++;
		printf("Timer callback %08X\n", count);
		break;
	}
	return 0;
}

//
// This part sets up a callback to receive timer events and then sets up a regular 
// 1 second timer event.
//
int main(int argc, char** argv)
{
	SNSetCallback("MyCallback");	// Set our callback handler.
	SNHookMessage(SM_TIMER);	// Hook the "timer" message.
	SNSetTimer(1000);		// 1 per second.

	return 0;
}
