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

#include "ghttpBuffer.h"
#include "ghttpConnection.h"
#include "ghttpMain.h"
#include "ghttpCommon.h"

// Resize the buffer.
/////////////////////
static GHTTPBool ghiResizeBuffer
(
	GHIBuffer * buffer,
	int sizeIncrement
)
{
	char * tempPtr;
	int newSize;

	assert(buffer);
	assert(sizeIncrement > 0);

	// Check args.
	//////////////
	if(!buffer)
		return GHTTPFalse;
	if(sizeIncrement <= 0)
		return GHTTPFalse;

	// Reallocate with the bigger size.
	///////////////////////////////////
	newSize = (buffer->size + sizeIncrement);
	tempPtr = (char *)gsirealloc(buffer->data, newSize);
	if(!tempPtr)
		return GHTTPFalse;

	// Set the new info.
	////////////////////
	buffer->data = tempPtr;
	buffer->size = newSize;

	return GHTTPTrue;
}

GHTTPBool ghiInitBuffer
(
	struct GHIConnection * connection,
	GHIBuffer * buffer,
	int initialSize,
	int sizeIncrement
)
{
	GHTTPBool bResult;

	assert(connection);
	assert(buffer);
	assert(initialSize > 0);
	assert(sizeIncrement > 0);

	// Check args.
	//////////////
	if(!connection)
		return GHTTPFalse;
	if(!buffer)
		return GHTTPFalse;
	if(initialSize <= 0)
		return GHTTPFalse;
	if(sizeIncrement <= 0)
		return GHTTPFalse;

	// Init the struct.
	///////////////////
	buffer->connection = connection;
	buffer->data = NULL;
	buffer->size = 0;
	buffer->len = 0;
	buffer->pos = 0;
	buffer->sizeIncrement = sizeIncrement;
	buffer->fixed = GHTTPFalse;
	buffer->dontFree = GHTTPFalse;

	// Do the initial resize.
	/////////////////////////
	bResult = ghiResizeBuffer(buffer, initialSize);
	if(!bResult)
		return GHTTPFalse;

	// Start with an empty string.
	//////////////////////////////
	*buffer->data = '\0';

	return GHTTPTrue;
}

GHTTPBool ghiInitFixedBuffer
(
	struct GHIConnection * connection,
	GHIBuffer * buffer,
	char * userBuffer,
	int size
)
{
	assert(connection);
	assert(buffer);
	assert(userBuffer);
	assert(size > 0);

	// Check args.
	//////////////
	if(!connection)
		return GHTTPFalse;
	if(!buffer)
		return GHTTPFalse;
	if(!userBuffer)
		return GHTTPFalse;
	if(size <= 0)
		return GHTTPFalse;

	// Init the struct.
	///////////////////
	buffer->connection = connection;
	buffer->data = userBuffer;
	buffer->size = size;
	buffer->len = 0;
	buffer->sizeIncrement = 0;
	buffer->fixed = GHTTPTrue;
	buffer->dontFree = GHTTPTrue;

	// Start with an empty string.
	//////////////////////////////
	*buffer->data = '\0';

	return GHTTPTrue;
}

void ghiFreeBuffer
(
	GHIBuffer * buffer
)
{
	assert(buffer);

	// Check args.
	//////////////
	if(!buffer)
		return;
	if(!buffer->data)
		return;

	// Cleanup the struct.
	//////////////////////
	if(!buffer->dontFree)
		gsifree(buffer->data);
	memset(buffer, 0, sizeof(GHIBuffer));
}

GHTTPBool ghiAppendDataToBuffer
(
	GHIBuffer * buffer,
	const char * data,
	int dataLen
)
{
	GHTTPBool bResult;
	int newLen;

	assert(buffer);
	assert(data);
	assert(dataLen >= 0);

	// Check args.
	//////////////
	if(!buffer)
		return GHTTPFalse;
	if(!data)
		return GHTTPFalse;
	if(dataLen < 0)
		return GHTTPFalse;

	// Get the string length if needed.
	///////////////////////////////////
	if(dataLen == 0)
		dataLen = strlen(data);

	// Get the new length.
	//////////////////////
	newLen = (buffer->len + dataLen);

	// Make sure the array is big enough.
	/////////////////////////////////////
	while(newLen >= buffer->size)
	{
		// Check for a fixed buffer.
		////////////////////////////
		if(buffer->fixed)
		{
			buffer->connection->completed = GHTTPTrue;
			buffer->connection->result = GHTTPBufferOverflow;
			return GHTTPFalse;
		}

		bResult = ghiResizeBuffer(buffer, buffer->sizeIncrement);
		if(!bResult)
		{
			buffer->connection->completed = GHTTPTrue;
			buffer->connection->result = GHTTPOutOfMemory;
			return GHTTPFalse;
		}
	}

	// Add the data.
	////////////////
	memcpy(buffer->data + buffer->len, data, dataLen);
	buffer->len = newLen;
	buffer->data[buffer->len] = '\0';

	return GHTTPTrue;
}

GHTTPBool ghiAppendHeaderToBuffer
(
	GHIBuffer * buffer,
	const char * name,
	const char * value
)
{
	if(!ghiAppendDataToBuffer(buffer, name, 0))
		return GHTTPFalse;
	if(!ghiAppendDataToBuffer(buffer, ": ", 2))
		return GHTTPFalse;
	if(!ghiAppendDataToBuffer(buffer, value, 0))
		return GHTTPFalse;
	if(!ghiAppendDataToBuffer(buffer, CRLF, 2))
		return GHTTPFalse;

	return GHTTPTrue;
}

GHTTPBool ghiAppendCharToBuffer
(
	GHIBuffer * buffer,
	int c
)
{
	GHTTPBool bResult;

	assert(buffer);

	// Check args.
	//////////////
	if(!buffer)
		return GHTTPFalse;

	// Make sure the array is big enough.
	/////////////////////////////////////
	if((buffer->len + 1) >= buffer->size)
	{
		// Check for a fixed buffer.
		////////////////////////////
		if(buffer->fixed)
		{
			buffer->connection->completed = GHTTPTrue;
			buffer->connection->result = GHTTPBufferOverflow;
			return GHTTPFalse;
		}

		bResult = ghiResizeBuffer(buffer, buffer->sizeIncrement);
		if(!bResult)
		{
			buffer->connection->completed = GHTTPTrue;
			buffer->connection->result = GHTTPOutOfMemory;
			return GHTTPFalse;
		}
	}

	// Add the char.
	////////////////
	buffer->data[buffer->len] = (char)(c & 0xFF);
	buffer->len++;
	buffer->data[buffer->len] = '\0';

	return GHTTPTrue;
}

GHTTPBool ghiAppendIntToBuffer
(
	GHIBuffer * buffer,
	int i
)
{
	char intValue[16];

	sprintf(intValue, "%d", i);

	return ghiAppendDataToBuffer(buffer, intValue, 0);
}

void ghiResetBuffer
(
	GHIBuffer * buffer
)
{
	assert(buffer);

	buffer->len = 0;
	buffer->pos = 0;

	// Start with an empty string.
	//////////////////////////////
	*buffer->data = '\0';
}

GHTTPBool ghiSendBuffer
(
	GHIBuffer * buffer,
	struct GHIConnection * connection
)
{
	int rcode;
	GHTTPBool writeFlag;
	GHTTPBool exceptFlag;
	GHTTPBool result;
	char * data;
	int len;

	// Loop while we can send.
	//////////////////////////
	do
	{
		result = ghiSocketSelect(connection->socket, NULL, &writeFlag, &exceptFlag);
		if(!result || exceptFlag)
		{
			connection->completed = GHTTPTrue;
			connection->result = GHTTPSocketFailed;
			connection->socketError = GOAGetLastError(connection->socket);
			return GHTTPFalse;
		}
		if(!writeFlag)
		{
			// Can't send anything.
			///////////////////////
			return GHTTPTrue;
		}

		// Figure out what, and how much, to send.
		//////////////////////////////////////////
		data = (connection->sendBuffer.data + connection->sendBuffer.pos);
		len = (connection->sendBuffer.len - connection->sendBuffer.pos);

		// Do the send.
		///////////////
		rcode = ghiDoSend(connection, data, len);
		if(rcode == SOCKET_ERROR)
			return GHTTPFalse;

		// Update the position.
		///////////////////////
		connection->sendBuffer.pos += rcode;
	}
	while(connection->sendBuffer.pos < connection->sendBuffer.len);

	return GHTTPTrue;

	// Silence unused variable warnings	
	GSI_UNUSED(buffer);
}
