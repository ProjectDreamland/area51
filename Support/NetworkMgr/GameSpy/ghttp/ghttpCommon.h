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

#ifndef _GHTTPCOMMON_H_
#define _GHTTPCOMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ghttp.h"
#include "ghttpConnection.h"

// HTTP Line-terminator.
////////////////////////
#define CRLF    "\xD\xA"

// Default HTTP port.
/////////////////////
#define GHI_DEFAULT_PORT                      80
#define GHI_DEFAULT_THROTTLE_BUFFER_SIZE      125
#define GHI_DEFAULT_THROTTLE_TIME_DELAY       250

// Proxy server.
////////////////
extern char * ghiProxyAddress;
extern unsigned short ghiProxyPort;

// Throttle settings.
/////////////////////
extern int ghiThrottleBufferSize;
extern gsi_time ghiThrottleTimeDelay;

// Our thread lock.
///////////////////
void ghiCreateLock(void);
void ghiFreeLock(void);
void ghiLock(void);
void ghiUnlock(void);

// Do logging.
//////////////
#ifdef HTTP_LOG
void ghiLog
(
	char * buffer,
	int len
);
#else
#define ghiLog(b, c)
#endif

// Check a socket for read/write/error.
///////////////////////////////////////
GHTTPBool ghiSocketSelect
(
	SOCKET socket,
	GHTTPBool * readFlag,
	GHTTPBool * writeFlag,
	GHTTPBool * exceptFlag
);

// Possible results from ghiDoReceive.
//////////////////////////////////////
typedef enum
{
	GHIRecvData,    // Data was received.
	GHINoData,      // No data was available.
	GHIConnClosed,  // The connection was closed.
	GHIError        // There was a socket error.
} GHIRecvResult;

// Receive some data.
/////////////////////
GHIRecvResult ghiDoReceive
(
	GHIConnection * connection,
	char buffer[],
	int * bufferLen
);

// Do a send on the connection's socket.
// Returns number of bytes sent (0 or more).
// If error, returns SOCKET_ERROR (-1).
////////////////////////////////////////////
int ghiDoSend
(
	GHIConnection * connection,
	const char * buffer,
	int len
);

// Results for ghtTrySendThenBuffer.
////////////////////////////////////
typedef enum
{
	GHITrySendError,     // There was an error sending.
	GHITrySendSent,      // Everything was sent.
	GHITrySendBuffered   // Some or all of the data was buffered.
} GHITrySendResult;

// Sends whatever it can on the socket.
// Buffers whatever can't be sent in the sendBuffer.
////////////////////////////////////////////////////
GHITrySendResult ghiTrySendThenBuffer
(
	GHIConnection * connection,
	const char * buffer,
	int len
);

// Set the proxy server.
////////////////////////
GHTTPBool ghiSetProxy
(
	const char * server
);

// Set the throttle settings.
/////////////////////////////
void ghiThrottleSettings
(
	int bufferSize,
	gsi_time timeDelay
);

#ifdef UNDER_CE
// CE doesn't have isspace().
/////////////////////////////
int isspace(int c);
#endif

#ifdef __cplusplus
}
#endif

#endif
