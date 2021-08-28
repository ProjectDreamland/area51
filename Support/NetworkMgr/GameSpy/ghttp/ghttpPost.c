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

#include "ghttpPost.h"
#include "ghttpMain.h"
#include "ghttpConnection.h"
#include "ghttpCommon.h"

// The border between parts in a file send.
///////////////////////////////////////////
#define GHI_MULTIPART_BOUNDARY          "Qr4G823s23d---<<><><<<>--7d118e0536"
#define GHI_MULTIPART_BOUNDARY_BASE     "--" GHI_MULTIPART_BOUNDARY
#define GHI_MULTIPART_BOUNDARY_FIRST    GHI_MULTIPART_BOUNDARY_BASE CRLF
#define GHI_MULTIPART_BOUNDARY_NORMAL   CRLF GHI_MULTIPART_BOUNDARY_BASE CRLF
#define GHI_MULTIPART_BOUNDARY_END      CRLF GHI_MULTIPART_BOUNDARY_BASE "--" CRLF

#define GHI_LEGAL_URLENCODED_CHARS      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_@-.*"
#define GHI_DIGITS                      "0123456789ABCDEF"

// POST TYPES.
//////////////
typedef enum
{
	GHIString,      // A regular string.
	GHIFileDisk,    // A file from disk.
	GHIFileMemory   // A file from memory.
} GHIPostDataType;

// POST OBJECT.
///////////////
typedef struct GHIPost
{
	DArray data;
	ghttpPostCallback callback;
	void * param;
	GHTTPBool hasFiles;
	GHTTPBool autoFree;
} GHIPost;

// POST DATA.
/////////////
typedef struct GHIPostStringData
{
	char * string;
	int len;
	GHTTPBool invalidChars;
	int extendedChars;
} GHIPostStringData;

typedef struct GHIPostFileDiskData
{
	char * filename;
	char * reportFilename;
	char * contentType;
} GHIPostFileDiskData;

typedef struct GHIPostFileMemoryData
{
	const char * buffer;
	int len;
	char * reportFilename;
	char * contentType;
} GHIPostFileMemoryData;

typedef struct GHIPostData
{
	GHIPostDataType type;
	char * name;
	union
	{
		GHIPostStringData string;
		GHIPostFileDiskData fileDisk;
		GHIPostFileMemoryData fileMemory;
	} data;
} GHIPostData;

// POST STATE.
//////////////
//typedef struct GHIPostStringState
//{
//} GHIPostStringState;

typedef struct GHIPostFileDiskState
{
	FILE * file;
	long len;
} GHIPostFileDiskState;

//typedef struct GHIPostFileMemoryState
//{
//} GHIPostFileMemoryState;

typedef struct GHIPostState
{
	GHIPostData * data;
	int pos;
	union
	{
		//GHIPostStringState string;
		GHIPostFileDiskState fileDisk;
		//GHIPostFileMemoryState fileMemory;
	} state;
} GHIPostState;

// FUNCTIONS.
/////////////
static void ghiPostDataFree
(
	void * elem
)
{
	GHIPostData * data = (GHIPostData *)elem;

	// Free the name.
	/////////////////
	gsifree(data->name);

	// Free based on type.
	//////////////////////
	if(data->type == GHIString)
	{
		// Free the string string.
		/////////////////////////
		gsifree(data->data.string.string);
	}
	else if(data->type == GHIFileDisk)
	{
		// Free the strings.
		////////////////////
		gsifree(data->data.fileDisk.filename);
		gsifree(data->data.fileDisk.reportFilename);
		gsifree(data->data.fileDisk.contentType);
	}
	else if(data->type == GHIFileMemory)
	{
		// Free the strings.
		////////////////////
		gsifree(data->data.fileMemory.reportFilename);
		gsifree(data->data.fileMemory.contentType);
	}
	else
	{
		// The type didn't match any known types.
		/////////////////////////////////////////
		assert(0);
	}
}

GHTTPPost ghiNewPost
(
	void
)
{
	GHIPost * post;

	// Allocate the post object.
	////////////////////////////
	post = (GHIPost *)gsimalloc(sizeof(GHIPost));
	if(!post)
		return NULL;

	// Initialize it.
	/////////////////
	memset(post, 0, sizeof(GHIPost));
	post->autoFree = GHTTPTrue;

	// Create the array of data objects.
	////////////////////////////////////
	post->data = ArrayNew(sizeof(GHIPostData), 0, ghiPostDataFree);
	if(!post->data)
	{
		gsifree(post);
		return NULL;
	}

	return (GHTTPPost)post;
}

void ghiPostSetAutoFree
(
	GHTTPPost _post,
	GHTTPBool autoFree
)
{
	GHIPost * post = (GHIPost *)_post;

	post->autoFree = autoFree;
}

GHTTPBool ghiIsPostAutoFree
(
	GHTTPPost _post
)
{
	GHIPost * post = (GHIPost *)_post;

	return post->autoFree;
}

void ghiFreePost
(
	GHTTPPost _post
)
{
	GHIPost * post = (GHIPost *)_post;

	// Free the array of data objects.
	//////////////////////////////////
	ArrayFree(post->data);

	// Free the post object.
	////////////////////////
	gsifree(post);
}

GHTTPBool ghiPostAddString
(
	GHTTPPost _post,
	const char * name,
	const char * string
)
{
	GHIPost * post = (GHIPost *)_post;
	GHIPostData data;
	int len;
	int rcode;

	// Copy the strings.
	////////////////////
	name = strdup(name);
	string = strdup(string);
	if(!name || !string)
	{
		gsifree((char *)name);
		gsifree((char *)string);
		return GHTTPFalse;
	}

	// Set the data.
	////////////////
	memset(&data, 0, sizeof(GHIPostData));
	data.type = GHIString;
	data.name = (char *)name;
	data.data.string.string = (char *)string;
	len = strlen(string);
	data.data.string.len = len;
	data.data.string.invalidChars = GHTTPFalse;

	// Are there any invalid characters?
	////////////////////////////////////
	rcode = strspn(string, GHI_LEGAL_URLENCODED_CHARS);
	if(rcode != len)
	{
		int i;
		int count = 0;

		data.data.string.invalidChars = GHTTPTrue;

		// Count the number, not including spaces.
		//////////////////////////////////////////
		for(i = 0 ; string[i] ; i++)
			if(!strchr(GHI_LEGAL_URLENCODED_CHARS, string[i]) && (string[i] != ' '))
				count++;

		data.data.string.extendedChars = count;
	}

	// Add it.
	//////////
	ArrayAppend(post->data, &data);

	return GHTTPTrue;
}

GHTTPBool ghiPostAddFileFromDisk
(
	GHTTPPost _post,
	const char * name,
	const char * filename,
	const char * reportFilename,
	const char * contentType
)
{
	GHIPost * post = (GHIPost *)_post;
	GHIPostData data;

	// Copy the strings.
	////////////////////
	name = strdup(name);
	filename = strdup(filename);
	reportFilename = strdup(reportFilename);
	contentType = strdup(contentType);
	if(!name || !filename || !reportFilename || !contentType)
	{
		gsifree((char *)name);
		gsifree((char *)filename);
		gsifree((char *)reportFilename);
		gsifree((char *)contentType);
		return GHTTPFalse;
	}

	// Set the data.
	////////////////
	memset(&data, 0, sizeof(GHIPostData));
	data.type = GHIFileDisk;
	data.name = (char *)name;
	data.data.fileDisk.filename = (char *)filename;
	data.data.fileDisk.reportFilename = (char *)reportFilename;
	data.data.fileDisk.contentType = (char *)contentType;

	// Add it.
	//////////
	ArrayAppend(post->data, &data);

	// We have files.
	/////////////////
	post->hasFiles = GHTTPTrue;

	return GHTTPTrue;
}

GHTTPBool ghiPostAddFileFromMemory
(
	GHTTPPost _post,
	const char * name,
	const char * buffer,
	int bufferLen,
	const char * reportFilename,
	const char * contentType
)
{
	GHIPost * post = (GHIPost *)_post;
	GHIPostData data;

	// Copy the strings.
	////////////////////
	name = strdup(name);
	reportFilename = strdup(reportFilename);
	contentType = strdup(contentType);
	if(!name || !reportFilename || !contentType)
	{
		gsifree((char *)name);
		gsifree((char *)reportFilename);
		gsifree((char *)contentType);
		return GHTTPFalse;
	}

	// Set it.
	//////////
	memset(&data, 0, sizeof(GHIPostData));
	data.type = GHIFileMemory;
	data.name = (char *)name;
	data.data.fileMemory.buffer = (char *)buffer;
	data.data.fileMemory.len = bufferLen;
	data.data.fileMemory.reportFilename = (char *)reportFilename;
	data.data.fileMemory.contentType = (char *)contentType;

	// Add it.
	//////////
	ArrayAppend(post->data, &data);

	// We have a file.
	//////////////////
	post->hasFiles = GHTTPTrue;

	return GHTTPTrue;
}

void ghiPostSetCallback
(
	GHTTPPost _post,
	ghttpPostCallback callback,
	void * param
)
{
	GHIPost * post = (GHIPost *)_post;

	// Set the callback and param.
	//////////////////////////////
	post->callback = callback;
	post->param = param;
}

const char * ghiPostGetContentType
(
	struct GHIConnection * connection
)
{
	GHIPost * post = connection->post;

	assert(post);
	if(!post)
		return "";

	// Report content-type based on if we are sending files or not.
	///////////////////////////////////////////////////////////////
	if(post->hasFiles)
		return ("multipart/form-data; boundary=" GHI_MULTIPART_BOUNDARY);

	return "application/x-www-form-urlencoded";
}

static int ghiPostGetNoFilesContentLength
(
	struct GHIConnection * connection
)
{
	GHIPost * post = connection->post;
	GHIPostData * data;
	int i;
	int num;
	int total = 0;

	num = ArrayLength(post->data);
	if(!num)
		return 0;

	for(i = 0 ; i < num ; i++)
	{
		data = (GHIPostData *)ArrayNth(post->data, i);
		assert(data->type == GHIString);

		total += strlen(data->name);
		total += data->data.string.len;
		total += (data->data.string.extendedChars * 2);
		total++;  // '='
	}

	total += (num - 1);  // '&'

	return total;
}

static int ghiPostGetHasFilesContentLength
(
	struct GHIConnection * connection
)
{
	GHIPost * post = connection->post;
	GHIPostData * data;
	int i;
	int num;
	int total = 0;
	static int boundaryLen;
	static int stringBaseLen;
	static int fileBaseLen;
	static int endLen;

	if(!boundaryLen)
	{
		boundaryLen = strlen(GHI_MULTIPART_BOUNDARY_BASE);

		stringBaseLen = (boundaryLen + 47);  // + name + string

		fileBaseLen = (boundaryLen + 76);  // + name + filename + content-type + file

		endLen = (boundaryLen + 4);
	}

	num = ArrayLength(post->data);

	for(i = 0 ; i < num ; i++)
	{
		data = (GHIPostData *)ArrayNth(post->data, i);

		if(data->type == GHIString)
		{
			total += stringBaseLen;
			total += strlen(data->name);
			total += data->data.string.len;
		}
		else if(data->type == GHIFileDisk)
		{
			GHIPostState * state;

			total += fileBaseLen;
			total += strlen(data->name);
			total += strlen(data->data.fileDisk.reportFilename);
			total += strlen(data->data.fileDisk.contentType);
			state = (GHIPostState *)ArrayNth(connection->postingState.states, i);
			assert(state);
			total += state->state.fileDisk.len;
		}
		else if(data->type == GHIFileMemory)
		{
			total += fileBaseLen;
			total += strlen(data->name);
			total += strlen(data->data.fileMemory.reportFilename);
			total += strlen(data->data.fileMemory.contentType);
			total += data->data.fileMemory.len;
		}
		else
		{
			assert(0);
			return 0;
		}
	}

	// Add the end.
	///////////////
	total += endLen;

	return total;
}

static int ghiPostGetContentLength
(
	struct GHIConnection * connection
)
{
	GHIPost * post = connection->post;

	assert(post);
	if(!post)
		return 0;

	if(post->hasFiles)
		return ghiPostGetHasFilesContentLength(connection);

	return ghiPostGetNoFilesContentLength(connection);
}

static GHTTPBool ghiPostStateInit
(
	GHIPostState * state
)
{
	GHIPostDataType type;

	// The type.
	////////////
	type = state->data->type;

	// Set the position to sending header.
	//////////////////////////////////////
	state->pos = -1;

	// Init based on type.
	//////////////////////
	if(type == GHIString)
	{
	}
	else if(type == GHIFileDisk)
	{
		// Open the file.
		/////////////////
		state->state.fileDisk.file = fopen(state->data->data.fileDisk.filename, "rb");
		if(!state->state.fileDisk.file)
			return GHTTPFalse;

		// Get the file length.
		///////////////////////
		if(fseek(state->state.fileDisk.file, 0, SEEK_END) != 0)
			return GHTTPFalse;
		state->state.fileDisk.len = ftell(state->state.fileDisk.file);
		if(state->state.fileDisk.len == EOF)
			return GHTTPFalse;
		rewind(state->state.fileDisk.file);
	}
	else if(type == GHIFileMemory)
	{
	}
	else
	{
		// The type didn't match any known types.
		/////////////////////////////////////////
		assert(0);

		return GHTTPFalse;
	}

	return GHTTPTrue;
}

static void ghiPostStateCleanup
(
	GHIPostState * state
)
{
	GHIPostDataType type;

	// The type.
	////////////
	type = state->data->type;

	// Init based on type.
	//////////////////////
	if(type == GHIString)
	{
	}
	else if(type == GHIFileDisk)
	{
		if(state->state.fileDisk.file)
			fclose(state->state.fileDisk.file);
		state->state.fileDisk.file = NULL;
	}
	else if(type == GHIFileMemory)
	{
	}
	else
	{
		// The type didn't match any known types.
		/////////////////////////////////////////
		assert(0);
	}
}

GHTTPBool ghiPostInitState
(
	struct GHIConnection * connection
)
{
	int i;
	int len;
	GHIPostData * data;
	GHIPostState state;
	GHIPostState * pState;

	assert(connection->post);
	if(!connection->post)
		return GHTTPFalse;

	// Create an array for the states.
	//////////////////////////////////
	connection->postingState.index = 0;
	connection->postingState.bytesPosted = 0;
	connection->postingState.totalBytes = 0;
	connection->postingState.callback = connection->post->callback;
	connection->postingState.param = connection->post->param;
	len = ArrayLength(connection->post->data);
	connection->postingState.states = ArrayNew(sizeof(GHIPostState), len, NULL);
	if(!connection->postingState.states)
		return GHTTPFalse;

	// Setup all the states.
	////////////////////////
	for(i = 0 ; i < len ; i++)
	{
		// Get the data object for this index.
		//////////////////////////////////////
		data = (GHIPostData *)ArrayNth(connection->post->data, i);

		// Initialize the state's members.
		//////////////////////////////////
		memset(&state, 0, sizeof(GHIPostState));
		state.data = data;

		// Call the init function.
		//////////////////////////
		if(!ghiPostStateInit(&state))
		{
			// We need to cleanup everything we just initialized.
			/////////////////////////////////////////////////////
			for(i-- ; i >= 0 ; i--)
			{
				pState = (GHIPostState *)ArrayNth(connection->postingState.states, i);
				ghiPostStateCleanup(pState);
			}

			// Free the array.
			//////////////////
			ArrayFree(connection->postingState.states);
			connection->postingState.states = NULL;

			return GHTTPFalse;
		}

		// Add it to the array.
		///////////////////////
		ArrayAppend(connection->postingState.states, &state);
	}

	// If this asserts, there aren't the same number of state objects as data objects.
	// There should be a 1-to-1 mapping between data and states.
	//////////////////////////////////////////////////////////////////////////////////
	assert(ArrayLength(connection->post->data) == ArrayLength(connection->postingState.states));

	// Get the total number of bytes.
	/////////////////////////////////
	connection->postingState.totalBytes = ghiPostGetContentLength(connection);

	return GHTTPTrue;
}

void ghiPostCleanupState
(
	struct GHIConnection * connection
)
{
	int i;
	int len;
	GHIPostState * state;

	// Loop through and call the cleanup function.
	//////////////////////////////////////////////
	if(connection->postingState.states)
	{
		len = ArrayLength(connection->postingState.states);
		for(i = 0 ; i < len ; i++)
		{
			state = (GHIPostState *)ArrayNth(connection->postingState.states, i);
			ghiPostStateCleanup(state);
		}

		// Free the array.
		//////////////////
		ArrayFree(connection->postingState.states);
		connection->postingState.states = NULL;
	}

	// Free the post.
	/////////////////
	if(connection->post && connection->post->autoFree)
	{
		ghiFreePost(connection->post);
		connection->post = NULL;
	}
}

static GHIPostingResult ghiPostStringStateDoPosting
(
	GHIPostState * state,
	GHIConnection * connection
)
{
	int rcode;
	int len;
	GHTTPBool result;

	assert(state->pos >= 0);

	// Is this an empty string?
	///////////////////////////
	if(state->data->data.string.len == 0)
		return GHIPostingDone;

	assert(state->pos < state->data->data.string.len);

	// If we're doing a simple post, we need to fix invalid characters.
	///////////////////////////////////////////////////////////////////
	if(!connection->post->hasFiles && state->data->data.string.invalidChars)
	{
		int i;
		int c;
		const char * string = state->data->data.string.string;
		char hex[4] = "%00";

		// This could probably be done a lot better.
		////////////////////////////////////////////
		for(i = 0 ; (c = string[i]) != 0 ; i++)
		{
			if(strchr(GHI_LEGAL_URLENCODED_CHARS, c))
			{
				// Legal.
				/////////
				result = ghiAppendCharToBuffer(&connection->sendBuffer, c);
			}
			else if(c == ' ')
			{
				// Space.
				/////////
				result = ghiAppendCharToBuffer(&connection->sendBuffer, '+');
			}
			else
			{
				// To hex.
				//////////
				assert((c / 16) < 16);
				hex[1] = GHI_DIGITS[c / 16];
				hex[2] = GHI_DIGITS[c % 16];
				result = ghiAppendDataToBuffer(&connection->sendBuffer, hex, 3);
			}
		}

		// Done.
		////////
		return GHIPostingDone;
	}

	// Send what we can.
	////////////////////
	len = (state->data->data.string.len - state->pos);
	rcode = ghiDoSend(connection, state->data->data.string.string, len);
	if(rcode == SOCKET_ERROR)
		return GHIPostingError;

	// Update the pos.
	//////////////////
	state->pos += rcode;

	// Did we send it all?
	//////////////////////
	if(rcode == len)
		return GHIPostingDone;

	return GHIPostingPosting;
}

static GHIPostingResult ghiPostFileDiskStateDoPosting
(
	GHIPostState * state,
	GHIConnection * connection
)
{
	char buffer[4096];
	int len;
	GHITrySendResult result;

	assert(state->pos >= 0);
	assert(state->pos < state->state.fileDisk.len);
	assert(state->pos == (int)ftell(state->state.fileDisk.file));

	// Loop while data is being sent.
	/////////////////////////////////
	do
	{
		// Read some data from the file.
		////////////////////////////////
		len = fread(buffer, 1, sizeof(buffer), state->state.fileDisk.file);
		if(len <= 0)
		{
			connection->completed = GHTTPTrue;
			connection->result = GHTTPFileReadFailed;
			return GHIPostingError;
		}

		// Update our position.
		///////////////////////
		state->pos += len;

		// Check for too much.
		//////////////////////
		if(state->pos > state->state.fileDisk.len)
		{
			connection->completed = GHTTPTrue;
			connection->result = GHTTPFileReadFailed;
			return GHIPostingError;
		}

		// Send.
		////////
		result = ghiTrySendThenBuffer(connection, buffer, len);
		if(result == GHITrySendError)
			return GHIPostingError;

		// Check if we've handled everything.
		/////////////////////////////////////
		if(state->pos == state->state.fileDisk.len)
			return GHIPostingDone;
	}
	while(result == GHITrySendSent);

	return GHIPostingPosting;
}

static GHIPostingResult ghiPostFileMemoryStateDoPosting
(
	GHIPostState * state,
	GHIConnection * connection
)
{
	int rcode;
	int len;

	assert(state->pos >= 0);

	// Is this an empty file?
	/////////////////////////
	if(state->data->data.fileMemory.len == 0)
		return GHIPostingDone;

	assert(state->pos < state->data->data.fileMemory.len);

	// Send what we can.
	////////////////////
	do
	{
		len = (state->data->data.fileMemory.len - state->pos);
		len = min(len, 32 * 1024);
		rcode = ghiDoSend(connection, state->data->data.fileMemory.buffer + state->pos, len);
		if(rcode == SOCKET_ERROR)
			return GHIPostingError;

		// Update the pos.
		//////////////////
		state->pos += rcode;

		// Did we send it all?
		//////////////////////
		if(state->data->data.fileMemory.len == state->pos)
			return GHIPostingDone;
	}
	while(rcode);

	return GHIPostingPosting;
}

static GHIPostingResult ghiPostStateDoPosting
(
	GHIPostState * state,
	GHIConnection * connection,
	GHTTPBool first
)
{
	int len;
	GHITrySendResult result;

	// Check for sending the header.
	////////////////////////////////
	if(state->pos == -1)
	{
		char buffer[2048];

		// Bump up the position so we only send the header once.
		////////////////////////////////////////////////////////
		state->pos = 0;

		// Check if this is a simple post.
		//////////////////////////////////
		if(!connection->post->hasFiles)
		{
			// Simple post only supports strings.
			/////////////////////////////////////
			assert(state->data->type == GHIString);

			// Format the header.
			/////////////////////
			if(first)
				sprintf(buffer, "%s=", state->data->name);
			else
				sprintf(buffer, "&%s=", state->data->name);
		}
		else
		{
			// Format the header based on string or file.
			/////////////////////////////////////////////
			if(state->data->type == GHIString)
			{
				sprintf(buffer,
					"%s"
					"Content-Disposition: form-data; "
					"name=\"%s\"" CRLF CRLF,
					first?GHI_MULTIPART_BOUNDARY_FIRST:GHI_MULTIPART_BOUNDARY_NORMAL,
					state->data->name);
			}
			else if((state->data->type == GHIFileDisk) || (state->data->type == GHIFileMemory))
			{
				const char * filename;
				const char * contentType;

				if(state->data->type == GHIFileDisk)
				{
					filename = state->data->data.fileDisk.reportFilename;
					contentType = state->data->data.fileDisk.contentType;
				}
				else
				{
					filename = state->data->data.fileMemory.reportFilename;
					contentType = state->data->data.fileMemory.contentType;
				}

				sprintf(buffer,
					"%s"
					"Content-Disposition: form-data; "
					"name=\"%s\"; "
					"filename=\"%s\"" CRLF
					"Content-Type: %s" CRLF CRLF,
					first?GHI_MULTIPART_BOUNDARY_FIRST:GHI_MULTIPART_BOUNDARY_NORMAL,
					state->data->name,
					filename,
					contentType);
			}
			else
			{
				assert(0);
			}
		}

		// Try sending.
		///////////////
		len = strlen(buffer);
		result = ghiTrySendThenBuffer(connection, buffer, len);
		if(result == GHITrySendError)
			return GHIPostingError;

		// If it was buffered, don't try anymore.
		/////////////////////////////////////////
		if(result == GHITrySendBuffered)
			return GHIPostingPosting;
	}

	// Post based on type.
	//////////////////////
	if(state->data->type == GHIString)
		return ghiPostStringStateDoPosting(state, connection);

	if(state->data->type == GHIFileDisk)
		return ghiPostFileDiskStateDoPosting(state, connection);

	assert(state->data->type == GHIFileMemory);
	return ghiPostFileMemoryStateDoPosting(state, connection);
}

GHIPostingResult ghiPostDoPosting
(
	struct GHIConnection * connection
)
{
	GHIPostingResult postingResult;
	GHITrySendResult trySendResult;
	GHIPostingState * postingState;
	GHIPostState * postState;
	int len;

	assert(connection);
	assert(connection->post);
	assert(connection->postingState.states);
	assert(ArrayLength(connection->post->data) == ArrayLength(connection->postingState.states));
	assert(connection->postingState.index >= 0);
	assert(connection->postingState.index <= ArrayLength(connection->postingState.states));

	// Cache some stuff.
	////////////////////
	postingState = &connection->postingState;
	len = ArrayLength(postingState->states);

	// Check for buffered data.
	///////////////////////////
	if(connection->sendBuffer.len)
	{
		// Send the buffered data.
		//////////////////////////
		if(!ghiSendBuffer(&connection->sendBuffer, connection))
			return GHIPostingError;

		// Check if we couldn't send it all.
		////////////////////////////////////
		if(connection->sendBuffer.pos < connection->sendBuffer.len)
			return GHIPostingPosting;

		// We sent it all, so reset the buffer.
		///////////////////////////////////////
		ghiResetBuffer(&connection->sendBuffer);

		// Was that all that's left?
		////////////////////////////
		if(connection->postingState.index == len)
			return GHIPostingDone;
	}

	// Loop while there's data to upload.
	/////////////////////////////////////
	while(postingState->index < len)
	{
		// Get the current data state.
		//////////////////////////////
		postState = (GHIPostState *)ArrayNth(postingState->states, postingState->index);
		assert(postState);

		// Upload the current data.
		///////////////////////////
		postingResult = ghiPostStateDoPosting(postState, connection, postingState->index == 0);

		// Check for error.
		///////////////////
		if(postingResult == GHIPostingError)
		{
			// Make sure we already set the error stuff.
			////////////////////////////////////////////
			assert(connection->completed && connection->result);

			return GHIPostingError;
		}

		// Check for still posting.
		///////////////////////////
		if(postingResult == GHIPostingPosting)
			return GHIPostingPosting;

		// One more done.
		/////////////////
		postingState->index++;
	}

	// Send or buffer the end marker.
	/////////////////////////////////
	if(connection->post->hasFiles)
	{
		trySendResult = ghiTrySendThenBuffer(connection, GHI_MULTIPART_BOUNDARY_END, strlen(GHI_MULTIPART_BOUNDARY_END));
		if(trySendResult == GHITrySendError)
			return GHIPostingError;
	}

	// We're not done if there's stuff in the buffer.
	/////////////////////////////////////////////////
	if(connection->sendBuffer.len)
		return GHIPostingPosting;

	return GHIPostingDone;
}
