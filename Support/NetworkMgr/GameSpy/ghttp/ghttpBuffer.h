 /*
GameSpy GHTTP SDK 
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com
*/

#ifndef _GHTTPBUFFER_H_
#define _GHTTPBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ghttpMain.h"

// A data buffer.
/////////////////
typedef struct GHIBuffer
{
	struct GHIConnection * connection;  // The connection.
	char * data;         // The actual bytes of data.
	int size;            // The number of bytes allocated for data.
	int len;             // The number of actual data bytes filled in.
	int pos;             // A marker to keep track of position.
	int sizeIncrement;   // How much to increment the buffer by when needed.
	GHTTPBool fixed;     // If true, don't resize the buffer.
	GHTTPBool dontFree;  // Don't free the data when the buffer is cleaned up.
} GHIBuffer;

// Initializes a buffer and allocates the initial data bytes.
// The initialSize and sizeIncrement must both be >0.
/////////////////////////////////////////////////////////////
GHTTPBool ghiInitBuffer
(
	struct GHIConnection * connection,  // The connection.
	GHIBuffer * buffer,  // The buffer to init.
	int initialSize,     // The initial size of the buffer.
	int sizeIncrement    // The size increment for the buffer.
);

// Initializes a fixed-size buffer.  This will not get resized.
///////////////////////////////////////////////////////////////
GHTTPBool ghiInitFixedBuffer
(
	struct GHIConnection * connection,  // The connection.
	GHIBuffer * buffer,  // The buffer to init.
	char * userBuffer,   // The user-buffer to use.
	int size             // The size of the buffer.
);

// Free's a buffer's allocated memory (does
// not free the actual GHIBuffer structure).
////////////////////////////////////////////
void ghiFreeBuffer
(
	GHIBuffer * buffer
);

// Appends data to the buffer.
// If data is a NUL-terminated string, 0 can be
// used for dataLen to use the length of the string.
////////////////////////////////////////////////////
GHTTPBool ghiAppendDataToBuffer
(
	GHIBuffer * buffer,  // The buffer to append to.
	const char * data,   // The data to append.
	int dataLen          // The number of bytes of data to append, 0 for NUL-terminated string.
);

// Appends a header to the buffer.
// Both the name and value must be NUL-terminated.
// The header will be added to the buffer as:
// <name>: <value>\n
//////////////////////////////////////////////////
GHTTPBool ghiAppendHeaderToBuffer
(
	GHIBuffer * buffer,  // The buffer to append to.
	const char * name,   // The name of the header.
	const char * value   // The value of the header.
);

// Appends a single character to the buffer.
////////////////////////////////////////////
GHTTPBool ghiAppendCharToBuffer
(
	GHIBuffer * buffer,  // The buffer to append to.
	int c                // The char to append.
);

// Converts the int to a string and appends it to the buffer.
/////////////////////////////////////////////////////////////
GHTTPBool ghiAppendIntToBuffer
(
	GHIBuffer * buffer,  // The buffer to append to.
	int i                // The int to append.
);

// Resets a buffer.
// Does this by setting both len and pos to 0.
//////////////////////////////////////////////
void ghiResetBuffer
(
	GHIBuffer * buffer   // The buffer to reset.
);

// Sends as much buffer data as it can.
// Returns false if there was an error.
///////////////////////////////////////
GHTTPBool ghiSendBuffer
(
	GHIBuffer * buffer,
	struct GHIConnection * connection
);

#ifdef __cplusplus
}
#endif

#endif
