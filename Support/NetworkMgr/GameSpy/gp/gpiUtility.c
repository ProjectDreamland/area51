/*
gpiUtility.c
GameSpy Presence SDK 
Dan "Mr. Pants" Schoenblum

Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

***********************************************************************
Please see the GameSpy Presence SDK documentation for more information
**********************************************************************/

//INCLUDES
//////////
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "gpi.h"

//DEFINES
/////////
#define OUTPUT_MAX_COL     100

// Disable compiler warnings for issues that are unavoidable.
/////////////////////////////////////////////////////////////
#if defined(_MSC_VER) // DevStudio
// Level4, "conditional expression is constant". 
// Occurs with use of the MS provided macro FD_SET
#pragma warning ( disable: 4127 )
#endif // _MSC_VER

//FUNCTIONS
///////////
void
strzcpy(
  char * dest,
  const char * src,
  size_t len
)
{
	assert(dest != NULL);
	assert(src != NULL);
// BW: Got rid of warning	assert(len >= 0);

	strncpy(dest, src, len);
	dest[len - 1] = '\0';
}

void
UTF8ToUCS2StringLen(
  const char * src,
  unsigned short * dest,
  size_t len
)
{
	unsigned short * aWritePos = dest;
	size_t aNumBytesRead		= 0;
	size_t aTotalBytesRead		= 0;

	assert(dest != NULL);
	assert(src != NULL);
//BW: Got rid of warning	assert(len >= 0);

	// Loop until we find the NULL terminator
	while (*src != '\0' && aTotalBytesRead < len)
	{
		// Convert one character
		aNumBytesRead = (unsigned int)_ReadUCS2CharFromUTF8String(src, aWritePos);

		// Move InStream position to new data
		src += aNumBytesRead;
		aTotalBytesRead += aNumBytesRead;

		// Move OutStream to next write position
		aWritePos++;
	}

	// NULL terminate at the writepos if we encountered NULL
	if (aTotalBytesRead < len)
		*aWritePos = '\0';

	// Set last character in buffer to NULL, to fully mimic strzcpy
	dest[len - 1] = '\0';
}

void
gpiDebug(
  GPConnection * connection,
  const char * format,
  ...
)
{
#ifdef GPDEBUG
	GPIConnection * iconnection = *connection;
	va_list argList;
	char * debugBuffer;
	int len;
	char * str;
	int i;
	char c;
#ifdef GSI_UNICODE
	wchar_t widebuff[1024];
#endif
#ifdef DEBUG_CRAZY
	FILE * fp;
#endif

	va_start(argList, format);

	//
	// These allocations were thrashing the heap so that we could rarely
	// compact to regain memory. The result was that GPInit() grew our
	// heap by 65MB.
	//
	// Here I allocate a single global buffer that we reuse. Does this
	// have any reentrancy issues? Note that I never freeclear this...I assume
	// the local heap will be freed on exit. (28jan00/bgw)
	//
	//buffer = gsimalloc(200000);

	//
	// If the buffer has not been alloc'd yet, do it now.
	//
	debugBuffer = iconnection->debugBuffer;
	if(debugBuffer == NULL)
		return;
		
	vsprintf(debugBuffer, format, argList);

#ifdef DEBUG_CRAZY
	// Append to the log.
	/////////////////////
	fp = fopen("GP.log", "at");
#endif

	// This is very hacky line splitting and file output code.
	//////////////////////////////////////////////////////////
	str = debugBuffer;
	len = strlen(debugBuffer);
	c = debugBuffer[OUTPUT_MAX_COL + 5];
	debugBuffer[OUTPUT_MAX_COL + 5] = '\0';
#ifdef _WIN32
	#ifdef GSI_UNICODE
		MultiByteToWideChar(CP_ACP,0,debugBuffer,-1,widebuff,1024);
		OutputDebugString(widebuff);
	#else
		OutputDebugString(debugBuffer);
	#endif
#else
	fprintf(stderr,"%s",debugBuffer);
#endif

#ifdef DEBUG_CRAZY
	if(fp != NULL)
		fwrite(debugBuffer, 1, min(OUTPUT_MAX_COL + 5, len), fp);
#endif
	debugBuffer[OUTPUT_MAX_COL + 5] = c;
	for(i = OUTPUT_MAX_COL + 5 ; i < len ; i += OUTPUT_MAX_COL)
	{
#ifdef _WIN32
	OutputDebugString(TEXT("\n     "));
#else
	fprintf(stderr,"\n     ");

#endif
		
#ifdef DEBUG_CRAZY
		if(fp != NULL)
			fwrite("\n     ", 1, 6, fp);
#endif
		c = debugBuffer[i + OUTPUT_MAX_COL];
		debugBuffer[i + OUTPUT_MAX_COL] = '\0';
#ifdef _WIN32
	#ifdef GSI_UNICODE
		MultiByteToWideChar(CP_ACP,0,&debugBuffer[i],-1,widebuff,1024);
		OutputDebugString(widebuff);
	#else
		OutputDebugString(&debugBuffer[i]);
	#endif
#else
	fprintf(stderr,"%s",&debugBuffer[i]);

#endif
		
#ifdef DEBUG_CRAZY
		if(fp != NULL)
			fwrite(&debugBuffer[i], 1, min(OUTPUT_MAX_COL, len - i), fp);
#endif
		debugBuffer[i + OUTPUT_MAX_COL] = c;
	}

#ifdef DEBUG_CRAZY
	if(fp != NULL)
		fclose(fp);
#endif

//
// We don't freeclear debugBuffer on purpose, we're going to reuse it
// next time through. (28jan00/bgw)
//
//	freeclear(buffer);

	va_end(argList);
#endif

	GSI_UNUSED(connection);
	GSI_UNUSED(format);
}

GPIBool
gpiCheckForError(
  GPConnection * connection,
  const char * input,
  GPIBool callErrorCallback
)
{
	char buffer[16];
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	if(strncmp(input, "\\error\\", 7) == 0)
	{
		// Get the err code.
		////////////////////
		if(gpiValueForKey(input, "\\err\\", buffer, sizeof(buffer)))
			iconnection->errorCode = (GPErrorCode)atoi(buffer);
		
		// Get the error string.
		////////////////////////
		if(!gpiValueForKey(input, "\\errmsg\\", iconnection->errorString, sizeof(iconnection->errorString)))
			iconnection->errorString[0] = '\0';

#ifdef GSI_UNICODE
		// Update the UNICODE version
		UTF8ToUCS2String(iconnection->errorString, iconnection->errorString_W);
#endif
		// Call the error callback?
		///////////////////////////
		if(callErrorCallback)
		{
			GPIBool fatal = (GPIBool)(strstr(input, "\\fatal\\") != NULL);
			gpiCallErrorCallback(connection, GP_SERVER_ERROR, fatal ? GP_FATAL : GP_NON_FATAL);
		}
		
		return GPITrue;
	}

	return GPIFalse;
}

GPIBool
gpiValueForKey(
  const char * command,
  const char * key,
  char * value,
  int len
)
{
	char delimiter;
	char * start;
	int i;
	char c;

	// Check for NULL.
	//////////////////
	assert(command != NULL);
	assert(key != NULL);
	assert(value != NULL);
	assert(len > 0);

	// Find which char is the delimiter.
	////////////////////////////////////
	delimiter = key[0];

	// Find the key.
	////////////////
	start = strstr(command, key);
	if(start == NULL)
		return GPIFalse;

	// Get to the start of the value.
	/////////////////////////////////
	start += strlen(key);

	// Copy in the value.
	/////////////////////
	len--;
	for(i = 0 ; (i < len) && ((c = start[i]) != '\0') && (c != delimiter) ; i++)
	{
		value[i] = c;
	}
	value[i] = '\0';

	return GPITrue;
}

char *
gpiValueForKeyAlloc(
  const char * command,
  const char * key
)
{
	char delimiter;
	char * start;
	char c;
	char * value;
	int len;

	// Check for NULL.
	//////////////////
	assert(command != NULL);
	assert(key != NULL);

	// Find which char is the delimiter.
	////////////////////////////////////
	delimiter = key[0];

	// Find the key.
	////////////////
	start = strstr(command, key);
	if(start == NULL)
		return NULL;

	// Get to the start of the value.
	/////////////////////////////////
	start += strlen(key);

	// Find the key length.
	///////////////////////
	for(len = 0 ; ((c = start[len]) != '\0') && (c != delimiter) ; len++)  { };

	// Allocate the value.
	//////////////////////
	value = (char *)gsimalloc((unsigned int)len + 1);
	if(!value)
		return NULL;

	// Copy in the value.
	/////////////////////
	memcpy(value, start, (unsigned int)len);
	value[len] = '\0';

	return value;
}

GPResult
gpiCheckSocketConnect(
  GPConnection * connection,
  SOCKET sock,
  int * state
)
{
#if 0
	int aWriteFlag  = 0;
	int aExceptFlag = 0;
	int aReturnCode = 0;

	// Check if the connect is completed.
	/////////////////////////////////////
	aReturnCode = GSISocketSelect(sock, NULL, &aWriteFlag, &aExceptFlag);
	if (aReturnCode == SOCKET_ERROR)
	{
		gpiDebug(connection, "Error connecting\n");
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error checking for a completed connection.");
	}

	if (aReturnCode > 0)
	{
		// Check for a failed attempt.
		//////////////////////////////
		if(aExceptFlag)
		{
			gpiDebug(connection, "Connection rejected\n");
			*state = GPI_DISCONNECTED;
			return GP_NO_ERROR;
		}

		// Check for a successful attempt.
		//////////////////////////////////
		if(aWriteFlag)
		{
			gpiDebug(connection, "Connection accepted\n");
			*state = GPI_CONNECTED;
			return GP_NO_ERROR;
		}
	}

#else
    // need to attempt to connect socket again.
    struct hostent* host;
    int             rcode;
    struct sockaddr_in address;
#define GPI_CONNECTION_MANAGER_PORT    29900

    host = gethostbyname( GPConnectionManagerHostname );
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = *(unsigned int *)host->h_addr_list[0];
    assert(address.sin_addr.s_addr != 0);
    address.sin_port = htons(GPI_CONNECTION_MANAGER_PORT);
    rcode = connect(sock, (struct sockaddr*)&address,sizeof(struct sockaddr_in));
    if( rcode!=SOCKET_ERROR )
    {
        *state = GPI_CONNECTED;
        return GP_NO_ERROR;
    }
    (void)state;
    (void)connection;
#endif
	// Not connected yet.
	/////////////////////
	*state = GPI_NOT_CONNECTED;
	return GP_NO_ERROR;
}

GPResult
gpiReadKeyAndValue(
  GPConnection * connection,
  const char * buffer,
  int * index,
  char key[512],
  char value[512]
)
{
	int c;
	int i;
	char * start;

	assert(buffer != NULL);
	assert(key != NULL);
	assert(value != NULL);

	buffer += *index;
	start = (char *)buffer;

	if(*buffer++ != '\\')
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");

	for(i = 0 ; (c = *buffer++) != '\\' ; i++)
	{
		if(c == '\0')
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");
		if(i == 511)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");
		*key++ = (char)c;
	}
	*key = '\0';

	for(i = 0 ; ((c = *buffer++) != '\\') && (c != '\0') ; i++)
	{
		if(i == 511)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");
		*value++ = (char)c;
	}
	*value = '\0';

	*index += (buffer - start - 1);

	return GP_NO_ERROR;
}

void
gpiSetError(
  GPConnection * connection,
  GPErrorCode errorCode,
  const char * errorString
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	// Copy the string.
	///////////////////
	strzcpy(iconnection->errorString, errorString, GP_ERROR_STRING_LEN);

#ifdef GSI_UNICODE
	// Update the unicode version
	UTF8ToUCS2StringLen(iconnection->errorString, iconnection->errorString_W, GP_ERROR_STRING_LEN);
#endif

	// Set the code.
	////////////////
	iconnection->errorCode = errorCode;
}

void
gpiSetErrorString(
  GPConnection * connection,
  const char * errorString
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	// Copy the string.
	///////////////////
	strzcpy(iconnection->errorString, errorString, GP_ERROR_STRING_LEN);

#ifdef GSI_UNICODE
	// Update the unicode version
	UTF8ToUCS2StringLen(iconnection->errorString, iconnection->errorString_W, GP_ERROR_STRING_LEN);
#endif

}

// Re-enable previously disabled compiler warnings
///////////////////////////////////////////////////
#if defined(_MSC_VER)
#pragma warning ( default: 4127 )
#endif // _MSC_VER

