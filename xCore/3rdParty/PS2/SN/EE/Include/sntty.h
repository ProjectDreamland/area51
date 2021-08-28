//----------------------------------------------------------------------------
//
// Copyright © 2001 SN Systems Software Limited
// All Rights Reserved
//
// Author: M.Bush
//
// sntty.h : Declaration of ProView for PlayStation 2 'EE' tty functions.
//
//----------------------------------------------------------------------------
#ifndef _SN_TTY_INCLUDED
#define _SN_TTY_INCLUDED

#if defined(_cplusplus) || defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)

extern "C"
{
#endif //#if defined(_cplusplus) || defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)

//----------------------------------------------------------------------------
#ifndef NO_PRINTF_EMU		//define this to prevent printf replacement!
#define printf snprintf
#endif
//----------------------------------------------------------------------------
	
//----------------------------------------------------------------------------
// snputs - replacement EE TTY puts function.
//
// snputs() is provided as a temporary replacement for puts. Internally, it
// blocks on a semaphore allow simutaneous use from multiple threads and has
// no limit on the amount of data that can be passed.
//
// returns: number of characters transmitted or -1 for error.
// 
// snputs will attempt to bind to the IOP on first use, if it fails to bind
// this can be taken to mean that the ELF is not running under ProView
//
//----------------------------------------------------------------------------
int snputs(const char* pszStr);

//----------------------------------------------------------------------------
// snprintf - replacement EE TTY printf function.
//
// snprintf(..) is provided as a simple replacement for printf and has 
// limited capability. Internally it has a buffer of 512 bytes and you should 
// ensure that your string does not exceed this length or unexpected results 
// may occur. If you need to send more TTY than this at a time you should call
// snputs() directly or make your own version of printf with a larger buffer.
//
// The source for snprintf is provided below:
// 
//	int snprintf(const char *format, ...)
//	{
//		char	buff[0x200];
//		va_list	arg;
//
//		va_start(arg, format);
//		vsprintf(buff, format, arg);
//		va_end(arg);
//	
//		return snputs(buff);
//	}
//
// Caution: snprintf allocates memory bytes on the stack to allow it to be
// called safely from multiple threads simutaneously.
//
//----------------------------------------------------------------------------

#if defined(_cplusplus) || defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif //#if defined(_cplusplus) || defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)

#endif //#ifndef _SN_TTY_INCLUDED