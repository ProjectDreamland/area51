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

#include "ghttpCallbacks.h"
#include "ghttpPost.h"

void ghiCallCompletedCallback
(
	GHIConnection * connection
)
{
	GHTTPBool freeBuffer;
	char * buffer;
	GHTTPByteCount bufferLen;

	assert(connection);

	// Check for no callback.
	/////////////////////////
	if(!connection->completedCallback)
		return;

	// Figure out the buffer/bufferLen parameters.
	//////////////////////////////////////////////
	if(connection->type != GHIGET)
	{
		buffer = NULL;
		bufferLen = 0;
	}
	else
	{
		buffer = connection->getFileBuffer.data;
		bufferLen = connection->fileBytesReceived;
	}

	// Call the callback.
	/////////////////////
	freeBuffer = connection->completedCallback(
		connection->request,
		connection->result,
		buffer,
		bufferLen,
		connection->callbackParam);

	// Check for gsifree.
	//////////////////
	if(buffer && !freeBuffer)
		connection->getFileBuffer.dontFree = GHTTPTrue;
}

void ghiCallProgressCallback
(
	GHIConnection * connection,
	const char * buffer,
	GHTTPByteCount bufferLen
)
{	
	assert(connection);

	// Check for no callback.
	/////////////////////////
	if(!connection->progressCallback)
		return;

	// Call the callback.
	/////////////////////
	connection->progressCallback(
		connection->request,
		connection->state,
		buffer,
		bufferLen,
		connection->fileBytesReceived,
		connection->totalSize,
		connection->callbackParam
		);
}

void ghiCallPostCallback
(
	GHIConnection * connection
)
{
	assert(connection);

	// Check for no callback.
	/////////////////////////
	if(!connection->postingState.callback)
		return;

	// Call the callback.
	/////////////////////
	connection->postingState.callback(
		connection->request,
		connection->postingState.bytesPosted,
		connection->postingState.totalBytes,
		connection->postingState.index,
		ArrayLength(connection->postingState.states),
		connection->callbackParam
		);
}
