/*
gpi.c
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
#include "gpi.h"


// This is so VisualAssist will know about these functions.
///////////////////////////////////////////////////////////
#if 0
void MD5Init(MD5_CTX *);
void MD5Update(MD5_CTX *, unsigned char *, unsigned int);
void MD5Final(unsigned char [16], MD5_CTX *);
void MD5Print(unsigned char [16], char[33]);
void MD5Digest(unsigned char *, unsigned int, char[33]);
#endif

//FUNCTIONS
///////////
GPResult
gpiInitialize(
  GPConnection * connection,
  int productID,
  int namespaceID
)
{
	GPIConnection * iconnection;
	int i;
	GPResult result;

	// Set the connection to NULL in case of error.
	///////////////////////////////////////////////
	*connection = NULL;

	// Allocate the connection.
	///////////////////////////
	iconnection = (GPIConnection *)gsimalloc(sizeof(GPIConnection));
	if(iconnection == NULL)
		return GP_MEMORY_ERROR;

	// Initialize connection-specific variables.
	////////////////////////////////////////////
	memset(iconnection, 0, sizeof(GPIConnection));
	iconnection->errorString[0] = '\0';
	iconnection->errorCode = (GPErrorCode)0;
	iconnection->infoCaching = GPITrue;
	iconnection->infoCachingBuddyOnly = GPIFalse;
	iconnection->simulation = GPIFalse;
	iconnection->firewall = GPIFalse;
	iconnection->productID = productID;
	iconnection->namespaceID = namespaceID;

#ifdef GSI_UNICODE
	iconnection->errorString_W[0] = '\0';
#endif

	if(!gpiInitProfiles((GPConnection *)&iconnection))
	{
		freeclear(iconnection);
		return GP_MEMORY_ERROR;
	}
	iconnection->diskCache = NULL;
	for(i = 0 ; i < GPI_NUM_CALLBACKS ; i++)
	{
		iconnection->callbacks[i].callback = NULL;
		iconnection->callbacks[i].param = NULL;
	}
#ifdef GPDEBUG
	iconnection->debugBuffer = (char *)gsimalloc(10 * 1024);
#else
	iconnection->debugBuffer = NULL;
#endif

	// Clear the log.
	/////////////////
	gpiDebug((GPConnection *)&iconnection,
		"\n\n\n\n\n"
		"*************\n"
		"gpiInitialize\n");

	// Reset connection-specific stuff.
	///////////////////////////////////
	result = gpiReset((GPConnection *)&iconnection);
	if(result != GP_NO_ERROR)
	{
		gpiDestroy((GPConnection *)&iconnection);
		return result;
	}

	// Initialize the sockets library.
	//////////////////////////////////
	SocketStartUp();

	// Seed the random number generator.
	////////////////////////////////////
	srand((unsigned int)current_time());

#ifndef NOFILE
	// Load profiles cached on disk.
	////////////////////////////////
	result = gpiLoadDiskProfiles((GPConnection *)&iconnection);
	if(result != GP_NO_ERROR)
	{
		gpiDestroy((GPConnection *)&iconnection);
		return result;
	}
#endif

#ifndef NOFILE
	result = gpiInitTransfers((GPConnection *)&iconnection);
	if(result != GP_NO_ERROR)
	{
		gpiDestroy((GPConnection *)&iconnection);
		return result;
	}
#endif

	// Set the connection.
	//////////////////////
	*connection = (GPConnection)iconnection;

	return GP_NO_ERROR;
}

void
gpiDestroy(
  GPConnection * connection
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;

	// Cleanup connection-specific stuff.
	/////////////////////////////////////
	gpiDisconnect(connection, GPITrue);

#ifndef NOFILE
	// Write the profile info to disk.
	//     BD - Don't update if we never connected.
	//////////////////////////////////
	if(iconnection->infoCaching && iconnection->connectState != GPI_NOT_CONNECTED)
	{
		if(gpiSaveDiskProfiles(connection) != GP_NO_ERROR)
			gpiDebug(connection, "Error saving profiles to disk.");
	}
#endif

	// Free the debug buffer.
	/////////////////////////
	freeclear(iconnection->debugBuffer);

	// Free the profile list.
	/////////////////////////
	TableFree(iconnection->profileList.profileTable);

#ifndef NOFILE
	// Free the transfers.
	//////////////////////
	gpiCleanupTransfers(connection);
#endif

	// Free the memory.
	///////////////////
	freeclear(iconnection);

	// Set the connection pointer to NULL.
	//////////////////////////////////////
	*connection = NULL;
}

static GPIBool
gpiResetProfile(
  GPConnection * connection,
  GPIProfile * profile,
  void * data
)
{
	GSI_UNUSED(connection);
	GSI_UNUSED(data);

	profile->buddyStatus = NULL;
	profile->authSig = NULL;
	profile->requestCount = 0;
	profile->peerSig = NULL;

	return GPITrue;
}

GPResult
gpiReset(
  GPConnection * connection
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	iconnection->nick[0] = '\0';
	iconnection->uniquenick[0] = '\0';
	iconnection->email[0] = '\0';
	iconnection->cmSocket = INVALID_SOCKET;
	iconnection->connectState = GPI_NOT_CONNECTED;

	iconnection->socketBuffer.len = 0;
	iconnection->socketBuffer.pos = 0;
	iconnection->socketBuffer.size = 0;
	freeclear(iconnection->socketBuffer.buffer);
	iconnection->socketBuffer.buffer = NULL;

	iconnection->inputBufferSize = 0;
	freeclear(iconnection->inputBuffer);
	iconnection->inputBuffer = NULL;

	iconnection->outputBuffer.len = 0;
	iconnection->outputBuffer.pos = 0;
	iconnection->outputBuffer.size = 0;
	freeclear(iconnection->outputBuffer.buffer);
	iconnection->outputBuffer.buffer = NULL;

	iconnection->updateproBuffer.len = 0;
	iconnection->updateproBuffer.pos = 0;
	iconnection->updateproBuffer.size = 0;
	freeclear(iconnection->updateproBuffer.buffer);
	iconnection->updateproBuffer.buffer = NULL;

	iconnection->updateuiBuffer.len = 0;
	iconnection->updateuiBuffer.pos = 0;
	iconnection->updateuiBuffer.size = 0;
	freeclear(iconnection->updateuiBuffer.buffer);
	iconnection->updateuiBuffer.buffer = NULL;

	iconnection->peerSocket = INVALID_SOCKET;
	iconnection->nextOperationID = 2;
	while(iconnection->operationList != NULL)
		gpiRemoveOperation(connection, iconnection->operationList);
	iconnection->operationList = NULL;
	iconnection->profileList.numBuddies = 0;
	gpiProfileMap(connection, gpiResetProfile, NULL);
	iconnection->userid = 0;
	iconnection->profileid = 0;
	iconnection->sessKey = 0;
	iconnection->numSearches = 0;
	iconnection->fatalError = GPIFalse;
	iconnection->peerList = NULL;
	iconnection->lastStatus = (GPEnum)-1;
	iconnection->lastStatusString[0] = '\0';
	iconnection->lastLocationString[0] = '\0';

#ifdef GSI_UNICODE
	iconnection->nick_W[0] = '\0';
	iconnection->uniquenick_W[0] = '\0';
	iconnection->email_W[0] = '\0';
	iconnection->lastStatusString_W[0] = '\0';
	iconnection->lastLocationString_W[0] = '\0';
#endif

	return GP_NO_ERROR;
}

GPResult
gpiProcessConnectionManager(
  GPConnection * connection
)
{
	char * next;
	char * str;
	int id;
	GPIOperation * operation;
	char * tempPtr;
	int len;
	GPIBool connClosed = GPIFalse;
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPResult result;
	GPIBool loop;

	// Loop through the rest while waiting for any blocking operations.
	///////////////////////////////////////////////////////////////////
	do
	{
		// Add any waiting info to the output buffer.
		/////////////////////////////////////////////
		gpiAddLocalInfo(connection, &iconnection->outputBuffer);

		// Send anything that needs to be sent.
		///////////////////////////////////////
		CHECK_RESULT(gpiSendFromBuffer(connection, iconnection->cmSocket, &iconnection->outputBuffer, &connClosed, GPITrue, "CM"));

		// Read everything the connection manager sent.
		///////////////////////////////////////////////
		result = gpiRecvToBuffer(connection, iconnection->cmSocket, &iconnection->socketBuffer, &len, &connClosed, "CM");
		if(result != GP_NO_ERROR)
		{
			if(result == GP_NETWORK_ERROR)
				CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error reading from the server.");
			
			return result;
		}

		// Check if we have a completed command.
		////////////////////////////////////////
		while((next = strstr(iconnection->socketBuffer.buffer, "\\final\\")) != NULL)
		{
			// NUL terminate the command.
			/////////////////////////////
			next[0] = '\0';

			gpiDebug(connection, "CMD: %s\n", iconnection->socketBuffer.buffer);

			// Copy the command to the input buffer.
			////////////////////////////////////////
			len = (next - iconnection->socketBuffer.buffer);
			if(len > iconnection->inputBufferSize)
			{
				iconnection->inputBufferSize += max(GPI_READ_SIZE, len);
				tempPtr = (char*)gsirealloc(iconnection->inputBuffer, (unsigned int)iconnection->inputBufferSize + 1);
				if(tempPtr == NULL)
					Error(connection, GP_MEMORY_ERROR, "Out of memory.");
				iconnection->inputBuffer = tempPtr;
			}
			memcpy(iconnection->inputBuffer, iconnection->socketBuffer.buffer, (unsigned int)len + 1);

			//gpiDebug(connection, "CMD : %s\n", iconnection->inputBuffer);

			// Point to the start of the next one.
			//////////////////////////////////////
			next += 7;

			// Move the rest of the connect buffer up to the front.
			///////////////////////////////////////////////////////
			iconnection->socketBuffer.len -= (next - iconnection->socketBuffer.buffer);
			memmove(iconnection->socketBuffer.buffer, next, (unsigned int)iconnection->socketBuffer.len + 1);

			// Check for an id.
			///////////////////
			str = strstr(iconnection->inputBuffer, "\\id\\");
			if(str != NULL)
			{
				// Get the id.
				//////////////
				id = atoi(str + 4);

				// Try and match the id with an operation.
				//////////////////////////////////////////
				if(!gpiFindOperationByID(connection, &operation, id))
				{
					gpiDebug(connection, "No matching operation found for id %d\n", id);
				}
				else
				{
					// Process the operation.
					/////////////////////////
					CHECK_RESULT(gpiProcessOperation(connection, operation, iconnection->inputBuffer));
				}
			}
			// This is an unsolicited message.
			//////////////////////////////////
			else
			{
				// Check for an error.
				//////////////////////
				if(gpiCheckForError(connection, iconnection->inputBuffer, GPITrue))
				{
					return GP_SERVER_ERROR;
				}
				else if(strncmp(iconnection->inputBuffer, "\\bm\\", 4) == 0)
				{
					CHECK_RESULT(gpiProcessRecvBuddyMessage(connection, iconnection->inputBuffer));
				}
				else if(strncmp(iconnection->inputBuffer, "\\ka\\", 10) == 0)
				{
					// Ignore the keep-alive.
					/////////////////////////
				}
				else
				{
					// This is an unrecognized message.
					///////////////////////////////////
					gpiDebug(connection, "Received an unrecognized, unsolicited message.\n");
				}
			}
		}

		// Check for a closed connection.
		/////////////////////////////////
		if(connClosed)
		{
			// We've been disconnected.
			///////////////////////////
			iconnection->connectState = GPI_DISCONNECTED;
			gpiSetError(connection, GP_CONNECTION_CLOSED, "The server has closed the connection.");
			gpiCallErrorCallback(connection, GP_NETWORK_ERROR, GP_FATAL);
			return GP_NO_ERROR;
		}

		//PANTS|05.23.00 - removed sleep
		//crt - added it back 6/13/00
		//PANTS|07.10.00 - only sleep if looping
		loop = gpiOperationsAreBlocking(connection);
		if(loop)
			msleep(10);
	}
	while(loop);

	return GP_NO_ERROR;
}

GPResult
gpiProcess(
  GPConnection * connection,
  int blockingOperationID
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPIOperation * operation;
	GPIOperation * delOperation;
	GPResult result = GP_NO_ERROR;
	GPIBool loop;

	assert((iconnection->connectState == GPI_NOT_CONNECTED) ||
	       (iconnection->connectState == GPI_CONNECTING) ||
	       (iconnection->connectState == GPI_NEGOTIATING) ||
	       (iconnection->connectState == GPI_CONNECTED) ||
	       (iconnection->connectState == GPI_DISCONNECTED));

	// Check if no connection was attempted.
	////////////////////////////////////////
/*	if(iconnection->connectState == GPI_NOT_CONNECTED)
		return GP_NO_ERROR;

	// Check for a disconnection.
	/////////////////////////////
	if(iconnection->connectState == GPI_DISCONNECTED)
		return GP_NO_ERROR;
*/
	// Check if we're connecting.
	/////////////////////////////
	if(iconnection->connectState == GPI_CONNECTING)
	{
		do
		{
			result = gpiCheckConnect(connection);
			//PANTS|07.10.00 - only sleep if looping
			loop = (((result == GP_NO_ERROR) && (blockingOperationID != 0) && (iconnection->connectState == GPI_CONNECTING))) ? GPITrue:GPIFalse;
			if(loop)
				msleep(10);
		}
		while(loop);

		if(result != GP_NO_ERROR)
		{
			// Find the connect operation.
			//////////////////////////////
			if(gpiFindOperationByID(connection, &operation, 1))
			{
				operation->result = GP_SERVER_ERROR;
			}
			else
			{
				// Couldn't find the connect operation.
				///////////////////////////////////////
				assert(0);
			}
		}
	}

	// Only do this stuff if we're connected.
	/////////////////////////////////////////
	if((iconnection->connectState == GPI_CONNECTED) || (iconnection->connectState == GPI_NEGOTIATING))
	{
		// Process the connection.
		//////////////////////////
		if(result == GP_NO_ERROR)
			result = gpiProcessConnectionManager(connection);

		// Process peer messaging stuff.
		////////////////////////////////
		if(result == GP_NO_ERROR)
			result = gpiProcessPeers(connection);

#ifndef NOFILE
		// Process transfers.
		/////////////////////
		if(result == GP_NO_ERROR)
			result = gpiProcessTransfers(connection);
#endif
	}

	// Process searches.
	////////////////////
	if(result == GP_NO_ERROR)
		result = gpiProcessSearches(connection);

	// Look for failed operations.
	//////////////////////////////
	for(operation = iconnection->operationList ; operation != NULL ; )
	{
		if(operation->result != GP_NO_ERROR)
		{
			gpiFailedOpCallback(connection, operation);
			delOperation = operation;
			operation = operation->pnext;
			gpiRemoveOperation(connection, delOperation);
		}
		else
		{
			operation = operation->pnext;
		}
	}

	// Call callbacks.
	//////////////////
	CHECK_RESULT(gpiProcessCallbacks(connection, blockingOperationID));

	if(iconnection->fatalError)
	{
		gpiDisconnect(connection, GPIFalse);
	}
	else
	{
		//assert(!((result != GP_NO_ERROR) && (iconnection->connectState != GPI_CONNECTED)));
	}

	return result;
}

GPResult
gpiEnable(
  GPConnection * connection, 
  GPEnum state
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;

	// Enable the state.
	////////////////////
	switch(state)
	{
	case GP_INFO_CACHING:
		iconnection->infoCaching = GPITrue;
		break;

	case GP_SIMULATION:
		iconnection->simulation = GPITrue;
		break;

	case GP_INFO_CACHING_BUDDY_ONLY:
		iconnection->infoCachingBuddyOnly = GPITrue;
		break;

	default:
		Error(connection, GP_PARAMETER_ERROR, "Invalid state.");
	}

	return GP_NO_ERROR;
}

static GPIBool gpiFreeProfileInfo(
  GPConnection * connection,
  GPIProfile * profile,
  void * data
)
{
	GSI_UNUSED(data);

	gpiFreeInfoCache(profile);
	freeclear(profile->peerSig);

	if(gpiCanFreeProfile(profile))
	{
		gpiRemoveProfile(connection, profile);
		return GPIFalse;
	}

	return GPITrue;
}

GPResult
gpiDisable(
  GPConnection * connection, 
  GPEnum state
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;

	if(state == GP_INFO_CACHING)
	{
		iconnection->infoCaching = GPIFalse;

		// freeclear everyone's info.
		////////////////////////
		while(!gpiProfileMap(connection, gpiFreeProfileInfo, NULL))  { };
	}
	else if(state == GP_SIMULATION)
	{
		iconnection->simulation = GPIFalse;
	}
	else if(state == GP_INFO_CACHING_BUDDY_ONLY)
	{
		iconnection->infoCachingBuddyOnly = GPIFalse;
	}
	else
	{
		Error(connection, GP_PARAMETER_ERROR, "Invalid state.");
	}

	return GP_NO_ERROR;
}

#ifdef _DEBUG
static int nProfiles;
static int nUserID;
static int nBuddyStatus;
static int nBuddyMemory;
static int nInfoCache;
static int nInfoMemory;
static int nAuthSig;
static int nPeerSig;
static int nTotalMemory;

GPIBool
gpiReportProfile(
  GPConnection * connection,
  GPIProfile * profile,
  void * data
)
{
	int temp;

	GSI_UNUSED(connection);
	GSI_UNUSED(data);

	nProfiles++;
	nTotalMemory += sizeof(GPIProfile);
	if(profile->userId) nUserID++;
	if(profile->buddyStatus)
	{
		nBuddyStatus++;
		temp = sizeof(GPIBuddyStatus);
		if(profile->buddyStatus->statusString)
			temp += (int)(strlen(profile->buddyStatus->statusString) + 1);
		if(profile->buddyStatus->locationString)
			temp += (int)(strlen(profile->buddyStatus->locationString) + 1);
#ifdef GSI_UNICODE
//		if(profile->buddyStatus->statusString_W)
//			temp += (wcslen(profile->buddyStatus->statusString_W) + 2);
//		if(profile->buddyStatus->locationString_W)
//			temp += (wcslen(profile->buddyStatus->locationString_W) + 2);
#endif
		nBuddyMemory += temp;
		nTotalMemory += temp;
	}
	if(profile->cache)
	{
		nInfoCache++;
		temp = sizeof(GPIInfoCache);
		if(profile->cache->nick)
			temp += (int)(strlen(profile->cache->nick) + 1);
		if(profile->cache->uniquenick)
			temp += (int)(strlen(profile->cache->uniquenick) + 1);
		if(profile->cache->email)
			temp += (int)(strlen(profile->cache->email) + 1);
		if(profile->cache->firstname)
			temp += (int)(strlen(profile->cache->firstname) + 1);
		if(profile->cache->lastname)
			temp += (int)(strlen(profile->cache->lastname) + 1);
		if(profile->cache->homepage)
			temp += (int)(strlen(profile->cache->homepage) + 1);
		nInfoMemory += temp;
		nTotalMemory += temp;
	}
	if(profile->authSig) nAuthSig++;
	if(profile->peerSig) nPeerSig++;

	return GPITrue;
}

void
gpiReport(
  GPConnection * connection,
  void (* report)(const char * output)
)
{
	char buf[128];

	nProfiles = 0;
	nUserID = 0;
	nBuddyStatus = 0;
	nBuddyMemory = 0;
	nInfoCache = 0;
	nInfoMemory = 0;
	nAuthSig = 0;
	nPeerSig = 0;
	nTotalMemory = 0;

	report("START PROFILE MAP");
	report("-----------------");
	gpiProfileMap(connection, gpiReportProfile, NULL);

	sprintf(buf, "%d profiles %d bytes (%d avg)", nProfiles, nTotalMemory, nTotalMemory / max(nProfiles, 1));
	report(buf);
	if(nProfiles)
	{
		sprintf(buf, "UserID: %d (%d%%)", nUserID, nUserID * 100 / nProfiles);
		report(buf);
		sprintf(buf, "BuddyStatus: %d (%d%%) %d bytes (%d avg)", nBuddyStatus, nBuddyStatus * 100 / nProfiles, nBuddyMemory, nBuddyMemory / max(nBuddyStatus, 1));
		report(buf);
		sprintf(buf, "InfoCache: %d (%d%%) %d bytes (%d avg)", nInfoCache, nInfoCache * 100 / nProfiles, nInfoMemory, nInfoMemory / max(nInfoCache, 1));
		report(buf);
		sprintf(buf, "AuthSig: %d (%d%%)", nAuthSig, nAuthSig * 100 / nProfiles);
		report(buf);
		sprintf(buf, "PeerSig: %d (%d%%)", nPeerSig, nPeerSig * 100 / nProfiles);
		report(buf);
	}

	report("---------------");
	report("END PROFILE MAP");


}
#endif
