/*
gpiPeer.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpi.h"

//FUNCTIONS
///////////
static GPResult
gpiProcessPeerInitiatingConnection(
  GPConnection * connection,
  GPIPeer * peer
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	int state;
	char * str;
	int len;
	GPIBool connClosed;
	GPIProfile * pProfile;
	GPResult result;
	
	// Check the state.
	///////////////////
	switch(peer->state)
	{
	case GPI_PEER_GETTING_SIG:
		// Do nothing - we're waiting for getinfo to get the sig.
		/////////////////////////////////////////////////////////
		break;

	case GPI_PEER_GOT_SIG:
		// Start the connect.
		/////////////////////
		CHECK_RESULT(gpiPeerStartConnect(connection, peer));

		break;
		
	case GPI_PEER_CONNECTING:
		// Check if the connect finished.
		/////////////////////////////////
		CHECK_RESULT(gpiCheckSocketConnect(connection, peer->sock, &state));
		if(state == GPI_DISCONNECTED)
		{
			Error(connection, GP_NETWORK_ERROR, "Error connecting to a peer.");
		}
		if(state == GPI_CONNECTED)
		{
			GPIPeer * pcurr;
			GPIBool freePeerSig = GPITrue;

			// Get the profile object.
			//////////////////////////
			if(!gpiGetProfile(connection, peer->profile, &pProfile))
				Error(connection, GP_NETWORK_ERROR, "Error connecting to a peer.");

			// Send the auth.
			/////////////////
			gpiAppendStringToBuffer(connection, &peer->outputBuffer, "\\auth\\");
			gpiAppendStringToBuffer(connection, &peer->outputBuffer, "\\pid\\");
			gpiAppendIntToBuffer(connection, &peer->outputBuffer, iconnection->profileid);
			gpiAppendStringToBuffer(connection, &peer->outputBuffer, "\\nick\\");
			gpiAppendStringToBuffer(connection, &peer->outputBuffer, iconnection->nick);
			gpiAppendStringToBuffer(connection, &peer->outputBuffer, "\\sig\\");
			gpiAppendStringToBuffer(connection, &peer->outputBuffer, pProfile->peerSig);
			gpiAppendStringToBuffer(connection, &peer->outputBuffer, "\\final\\");

			// Are there any other peers still connecting?
			//////////////////////////////////////////////
			for(pcurr = iconnection->peerList ; pcurr != NULL ; pcurr = pcurr->pnext)
				if((pcurr->profile == peer->profile) && (pcurr != peer))
					if(pcurr->state <= GPI_PEER_CONNECTING)
						freePeerSig = GPIFalse;

			// freeclear it?
			///////////
			if(freePeerSig)
			{
				freeclear(pProfile->peerSig);
				if(gpiCanFreeProfile(pProfile))
					gpiRemoveProfile(connection, pProfile);
			}

			// Update the state.
			////////////////////
			peer->state = GPI_PEER_WAITING;
		}
		
		break;
		
	case GPI_PEER_WAITING:
		// Check for a response.
		////////////////////////
		CHECK_RESULT(gpiRecvToBuffer(connection, peer->sock, &peer->inputBuffer, &len, &connClosed, "PR"));
		
		// Check for a final.
		/////////////////////
		str = strstr(peer->inputBuffer.buffer, "\\final\\");
		if(str != NULL)
		{
			str[0] = '\0';
			str += 7;
			
			// Was it rejected?
			///////////////////
			if(strncmp(peer->inputBuffer.buffer, "\\anack\\", 7) == 0)
			{
				// Rejected.
				////////////
				peer->nackCount++;
				
				// Is this more than once?
				//////////////////////////
				if(peer->nackCount > 1)
				{
					// Give up already.
					///////////////////
					Error(connection, GP_NETWORK_ERROR, "Error getting buddy authorization.");
				}
				
				// Try getting the latest sig.
				//////////////////////////////
				CHECK_RESULT(gpiPeerGetSig(connection, peer));
			}
			else if(strncmp(peer->inputBuffer.buffer, "\\aack\\", 6) != 0)
			{
				// Unknown message.
				///////////////////
				Error(connection, GP_NETWORK_ERROR, "Error parsing buddy message.");
			}
			
			// The connection has been established.
			///////////////////////////////////////
			peer->state = GPI_PEER_CONNECTED;
			peer->inputBuffer.len = 0;
		}
		
		break;
		
	default:
		assert(0);
	}

	// Send stuff that needs to be sent.
	////////////////////////////////////
	if(peer->outputBuffer.len > 0)
	{
		result = gpiSendFromBuffer(connection, peer->sock, &peer->outputBuffer, &connClosed, GPITrue, "PR");
		if(connClosed || (result != GP_NO_ERROR))
			peer->state = GPI_PEER_DISCONNECTED;
	}
	
	return GP_NO_ERROR;
}

static GPResult
gpiProcessPeerAcceptingConnection(
  GPConnection * connection,
  GPIPeer * peer
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	char * str;
	int len;
	GPIBool connClosed;
	char intValue[16];
	int pid;
	char nick[GP_NICK_LEN];
	char sig[33];
	char sigCheck[33];
	char buffer[256];

	// Check the state.
	///////////////////
	assert(peer->state == GPI_PEER_WAITING);

	// Read any pending info.
	/////////////////////////
	CHECK_RESULT(gpiRecvToBuffer(connection, peer->sock, &peer->inputBuffer, &len, &connClosed, "PR"));

	// Check for a closed connection.
	/////////////////////////////////
	if(connClosed)
	{
		peer->state = GPI_PEER_DISCONNECTED;
		return GP_NO_ERROR;
	}

	// Check for a final.
	/////////////////////
	str = strstr(peer->inputBuffer.buffer, "\\final\\");
	if(str != NULL)
	{
		str[0] = '\0';
		str += 7;

		// Is it an auth?
		/////////////////
		if(strncmp(peer->inputBuffer.buffer, "\\auth\\", 6) == 0)
		{
			// Get the pid.
			///////////////
			if(!gpiValueForKey(peer->inputBuffer.buffer, "\\pid\\", intValue, sizeof(intValue)))
			{
				peer->state = GPI_PEER_DISCONNECTED;
				return GP_NO_ERROR;
			}
			pid = atoi(intValue);

			// Get the nick.
			////////////////
			if(!gpiValueForKey(peer->inputBuffer.buffer, "\\nick\\", nick, sizeof(nick)))
			{
				peer->state = GPI_PEER_DISCONNECTED;
				return GP_NO_ERROR;
			}

			// Get the sig.
			///////////////
			if(!gpiValueForKey(peer->inputBuffer.buffer, "\\sig\\", sig, sizeof(sig)))
			{
				peer->state = GPI_PEER_DISCONNECTED;
				return GP_NO_ERROR;
			}

			// Compute what the sig should be.
			//////////////////////////////////
			sprintf(buffer, "%s%d%d",
				iconnection->password,
				iconnection->profileid,
				pid);
			MD5Digest((unsigned char *)buffer, strlen(buffer), sigCheck);

			// Check the sig.
			/////////////////
			if(strcmp(sig, sigCheck) != 0)
			{
				// Bad sig.
				///////////
				gpiAppendStringToBuffer(connection, &peer->outputBuffer, "\\anack\\");
				gpiAppendStringToBuffer(connection, &peer->outputBuffer, "\\final\\");

				peer->state = GPI_PEER_DISCONNECTED;
				return GP_NO_ERROR;
			}

			// Send an ack.
			///////////////
			gpiAppendStringToBuffer(connection, &peer->outputBuffer, "\\aack\\");
			gpiAppendStringToBuffer(connection, &peer->outputBuffer, "\\final\\");
			
			peer->state = GPI_PEER_CONNECTED;
			peer->profile = (GPProfile)pid;
		}
		else
		{
			// Unrecognized command.
			////////////////////////
			peer->state = GPI_PEER_DISCONNECTED;
			return GP_NO_ERROR;
		}
		
		// Update the buffer length.
		////////////////////////////
		peer->inputBuffer.len = 0;
	}

	return GP_NO_ERROR;
}

GPResult
gpiPeerSendMessages(
  GPConnection * connection,
  GPIPeer * peer
)
{
	GPIBool connClosed;
	GPIMessage * message;
	GPResult result;

	// Only send messages if there's nothing waiting in the output buffer.
	//////////////////////////////////////////////////////////////////////
	if(peer->outputBuffer.len)
		return GP_NO_ERROR;

	// Send outgoing messages.
	//////////////////////////
	while(ArrayLength(peer->messages))
	{
		// Get the first message.
		/////////////////////////
		message = (GPIMessage *)ArrayNth(peer->messages, 0);

		// Send as much as possible.
		////////////////////////////
		result = gpiSendFromBuffer(connection, peer->sock, &message->buffer, &connClosed, GPIFalse, "PR");
		if(connClosed || (result != GP_NO_ERROR))
		{
			peer->state = GPI_PEER_DISCONNECTED;
			return GP_NO_ERROR;
		}

		// Did we not send it all?
		//////////////////////////
		if(message->buffer.pos != message->buffer.len)
			break;

		// Remove the message.
		//////////////////////
		ArrayDeleteAt(peer->messages, 0);
	}

	return GP_NO_ERROR;
}

static GPResult
gpiProcessPeerConnected(
  GPConnection * connection,
  GPIPeer * peer
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	int len;
	GPIBool connClosed;
	GPICallback callback;
	char * buffer;
	int type;
	int messageLen;
	GPResult result;

	// Send stuff.
	//////////////
	if(peer->outputBuffer.len)
	{
		result = gpiSendFromBuffer(connection, peer->sock, &peer->outputBuffer, &connClosed, GPITrue, "PR");
		if(connClosed || (result != GP_NO_ERROR))
		{
			peer->state = GPI_PEER_DISCONNECTED;
			return GP_NO_ERROR;
		}
	}

	// Send outgoing messages.
	//////////////////////////
	if(!peer->outputBuffer.len)
	{
		CHECK_RESULT(gpiPeerSendMessages(connection, peer));
		if(peer->state == GPI_PEER_DISCONNECTED)
			return GP_NO_ERROR;
	}

	// Read messages.
	/////////////////
	result = gpiRecvToBuffer(connection, peer->sock, &peer->inputBuffer, &len, &connClosed, "PR");
	if(result != GP_NO_ERROR)
	{
		peer->state = GPI_PEER_DISCONNECTED;
		return GP_NO_ERROR;
	}
	if(len > 0)
	{
		peer->timeout = (time(NULL) + GPI_PEER_TIMEOUT);
	}

	// Grab the message header.
	///////////////////////////
	do
	{
		// Read a message.
		//////////////////
		CHECK_RESULT(gpiReadMessageFromBuffer(connection, &peer->inputBuffer, &buffer, &type, &messageLen));
		if(buffer != NULL)
		{
			// Got a message!
			/////////////////
			switch(type)
			{
			case GPI_BM_MESSAGE:
				callback = iconnection->callbacks[GPI_RECV_BUDDY_MESSAGE];
				if(callback.callback != NULL)
				{
					GPRecvBuddyMessageArg * arg;

					arg = (GPRecvBuddyMessageArg *)gsimalloc(sizeof(GPRecvBuddyMessageArg));
					if(arg == NULL)
						Error(connection, GP_MEMORY_ERROR, "Out of memory.");
					arg->profile = peer->profile;
#ifndef GSI_UNICODE
					arg->message = goastrdup(buffer);
#else
					arg->message = UTF8ToUCS2StringAlloc(buffer);
#endif
					arg->date = (unsigned int)time(NULL);
					CHECK_RESULT(gpiAddCallback(connection, callback, arg, NULL, GPI_ADD_MESSAGE));
				}
				break;

			case GPI_BM_PING:
				// Send back a pong.
				////////////////////
				gpiSendBuddyMessage(connection, peer->profile, GPI_BM_PONG, "1");

				break;

#ifndef NOFILE
			case GPI_BM_PONG:
				// Lets the transfers handle this.
				//////////////////////////////////
				gpiTransfersHandlePong(connection, peer->profile, peer);
#endif
				break;

			case GPI_BM_FILE_SEND_REQUEST:
			case GPI_BM_FILE_SEND_REPLY:
			case GPI_BM_FILE_BEGIN:
			case GPI_BM_FILE_END:
			case GPI_BM_FILE_DATA:
			case GPI_BM_FILE_SKIP:
			case GPI_BM_FILE_TRANSFER_THROTTLE:
			case GPI_BM_FILE_TRANSFER_CANCEL:
			case GPI_BM_FILE_TRANSFER_KEEPALIVE:
				// Handle a transfer protocol message.
				//////////////////////////////////////
				gpiHandleTransferMessage(connection, peer, type, peer->inputBuffer.buffer, buffer, messageLen);

				break;

			default:
				break;
			}

			// Remove it from the buffer.
			/////////////////////////////
			gpiClipBufferToPosition(connection, &peer->inputBuffer);
		}
	}
	while(buffer);

	if(connClosed)
		peer->state = GPI_PEER_DISCONNECTED;
	
	return GP_NO_ERROR;
}

static GPResult
gpiProcessPeer(
  GPConnection * connection,
  GPIPeer * peer
)
{
	GPResult result = GP_NO_ERROR;
	
	// This state should never get out of initialization.
	/////////////////////////////////////////////////////
	assert(peer->state != GPI_PEER_NOT_CONNECTED);

	// If we're not connected yet.
	//////////////////////////////
	if(peer->state != GPI_PEER_CONNECTED)
	{
		if(peer->initiated)
			result = gpiProcessPeerInitiatingConnection(connection, peer);
		else
			result = gpiProcessPeerAcceptingConnection(connection, peer);
	}

	// If we're connected.
	//////////////////////
	if((result == GP_NO_ERROR) && (peer->state == GPI_PEER_CONNECTED))
	{
		result = gpiProcessPeerConnected(connection, peer);
	}

	return result;
}

void
gpiDestroyPeer(
  GPConnection * connection,
  GPIPeer * peer
)
{
#ifndef NOFILE
	// Cleanup any transfers that use this peer.
	////////////////////////////////////////////
	gpiTransferPeerDestroyed(connection, peer);
#endif

	shutdown(peer->sock, 2);
	closesocket(peer->sock);
	freeclear(peer->inputBuffer.buffer);
	freeclear(peer->outputBuffer.buffer);
	if(peer->messages)
	{
		ArrayFree(peer->messages);
		peer->messages = NULL;
	}
	freeclear(peer);
	
	GSI_UNUSED(connection);
}

void
gpiRemovePeer(
  GPConnection * connection,
  GPIPeer * peer
)
{
	GPIPeer * pprev;
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPIMessage * message;

	assert(peer != NULL);

	// Check if this is the first peer.
	///////////////////////////////////
	if(iconnection->peerList == peer)
	{
		iconnection->peerList = peer->pnext;
	}
	else
	{
		// Find the previous peer.
		//////////////////////////
		for(pprev = iconnection->peerList ; pprev->pnext != peer ; pprev = pprev->pnext)
		{
			if(pprev->pnext == NULL)
			{
				// Can't find this peer in the list!
				////////////////////////////////////
				assert(0);
				gpiDebug(connection, "Tried to remove peer not in list.");
				return;
			}
		}
		pprev->pnext = peer->pnext;
	}

	// Check for pending messages.
	//////////////////////////////
	while(ArrayLength(peer->messages))
	{
		// Get the next message.
		////////////////////////
		message = (GPIMessage *)ArrayNth(peer->messages, 0);

		// Don't forward protocol messages.
		///////////////////////////////////
		if(message->type < 100)
			gpiSendServerBuddyMessage(connection, peer->profile, message->type, message->buffer.buffer + message->start);

		// Remove the message.
		//////////////////////
		ArrayDeleteAt(peer->messages, 0);
	}

	gpiDestroyPeer(connection, peer);
}

static void gpiSetPeerSocketSizes(SOCKET sock)
{
	int size;

	SetReceiveBufferSize(sock, 16 * 1024);
	SetReceiveBufferSize(sock, 32 * 1024);
	SetReceiveBufferSize(sock, 64 * 1024);
	SetReceiveBufferSize(sock, 128 * 1024);
	SetReceiveBufferSize(sock, 256 * 1024);
	//SetReceiveBufferSize(sock, 512 * 1024);
	//SetReceiveBufferSize(sock, 1024 * 1024);
	SetSendBufferSize(sock, 16 * 1024);
	SetSendBufferSize(sock, 32 * 1024);
	SetSendBufferSize(sock, 64 * 1024);
	//SetSendBufferSize(sock, 128 * 1024);
	//SetSendBufferSize(sock, 256 * 1024);
	//SetSendBufferSize(sock, 512 * 1024);
	//SetSendBufferSize(sock, 1024 * 1024);

	size = GetReceiveBufferSize(sock);
	size = GetSendBufferSize(sock);
}

GPResult
gpiProcessPeers(
  GPConnection * connection
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPIPeer * nextPeer;
	GPIPeer * peer;
	SOCKET incoming;
	GPResult result;

	// Check for incoming peer connections.
	///////////////////////////////////////
	if(iconnection->peerSocket != INVALID_SOCKET)
	{
		// Have to manually check if accept is possible since
		// PS2 Insock only supports blocking sockets.
		if (CanReceiveOnSocket(iconnection->peerSocket))
		{
			incoming = accept(iconnection->peerSocket, NULL, NULL);
			if(incoming != INVALID_SOCKET)
			{
				// This is a new peer.
				//////////////////////
				peer = gpiAddPeer(connection, -1, GPIFalse);
				if(peer)
				{
					peer->state = GPI_PEER_WAITING;
					peer->sock = incoming;
					SetSockBlocking(incoming, 0);
					gpiSetPeerSocketSizes(peer->sock);
				}
				else
				{
					closesocket(incoming);
				}
			}
		}
	}

	// Got through the list of peers.
	/////////////////////////////////
	for(peer = iconnection->peerList ; peer != NULL ; peer = nextPeer)
	{
		// Store the next peer.
		///////////////////////
		nextPeer = peer->pnext;
		
		// Process the peer.
		////////////////////
		result = gpiProcessPeer(connection, peer);

		// Check for a disconnection or a timeout.
		//////////////////////////////////////////
		if((peer->state == GPI_PEER_DISCONNECTED) || (result != GP_NO_ERROR) || (time(NULL) > peer->timeout))
		{
			// Remove it.
			/////////////
			gpiRemovePeer(connection, peer);
		}
	}

	return GP_NO_ERROR;
}

GPIPeer *
gpiGetConnectedPeer(
  GPConnection * connection,
  int profileid
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	GPIPeer * pcurr;

	// Go through the list of peers.
	////////////////////////////////
	for(pcurr = iconnection->peerList ; pcurr != NULL ; pcurr  = pcurr->pnext)
	{
		// Check for a match.
		/////////////////////
		if(pcurr->profile == profileid)
		{
			// Check if the connection is established.
			//////////////////////////////////////////
			if(pcurr->state == GPI_PEER_CONNECTED)
			{
				// Got it.
				//////////
				return pcurr;
			}
		}
	}

	return NULL;
}

static void gpiFreeMessage(void * elem)
{
	GPIMessage * message = (GPIMessage *)elem;

	freeclear(message->buffer.buffer);
}

GPIPeer *
gpiAddPeer(
  GPConnection * connection,
  int profileid,
  GPIBool initiate
)
{
	GPIPeer * peer;
	GPIConnection * iconnection = (GPIConnection*)*connection;

	// Create a new peer.
	/////////////////////
	peer = (GPIPeer *)gsimalloc(sizeof(GPIPeer));
	if(peer == NULL)
		return NULL;
	memset(peer, 0, sizeof(GPIPeer));
	peer->state = GPI_PEER_NOT_CONNECTED;
	peer->initiated = initiate;
	peer->sock = INVALID_SOCKET;
	peer->profile = profileid;
	peer->timeout = (time(NULL) + GPI_PEER_TIMEOUT);
	peer->pnext = iconnection->peerList;
	peer->messages = ArrayNew(sizeof(GPIMessage), 0, gpiFreeMessage);
	iconnection->peerList = peer;

	return peer;
}

GPResult
gpiPeerGetSig(
  GPConnection * connection,
  GPIPeer * peer
)
{
	GPIOperation * operation;

	// Start a get info operation to get the sig.
	/////////////////////////////////////////////
	CHECK_RESULT(gpiAddOperation(connection, GPI_GET_INFO, NULL, &operation, GP_NON_BLOCKING, NULL, NULL));

	// Send the get info.
	/////////////////////
	CHECK_RESULT(gpiSendGetInfo(connection, peer->profile, operation->id));

	// Set the state.
	/////////////////
	peer->state = GPI_PEER_GETTING_SIG;

	return GP_NO_ERROR;
}

GPResult
gpiPeerStartConnect(
  GPConnection * connection,
  GPIPeer * peer
)
{
	int rcode;
	struct sockaddr_in address;
	GPIProfile * profile;

	// Get the profile object.
	//////////////////////////
	if(!gpiGetProfile(connection, peer->profile, &profile))
		Error(connection, GP_NETWORK_ERROR, "Error connecting to a peer.");

	// Create the socket.
	/////////////////////
	peer->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(peer->sock == INVALID_SOCKET)
		CallbackError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error creating a socket.");

	// Make it non-blocking.
	////////////////////////
	rcode = SetSockBlocking(peer->sock, 0);
	if(rcode == 0)
		CallbackError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error making a socket non-blocking.");

	// Bind the socket.
	///////////////////
/* 
// BD: PS2 Insock has bug with binding to port 0
// No sockets after the first will be able to bind

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	rcode = bind(peer->sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	if(rcode == SOCKET_ERROR)
		CallbackError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error binding a socket.");
*/
	// Set the socket sizes.
	////////////////////////
	gpiSetPeerSocketSizes(peer->sock);

	// Connect the socket.
	//////////////////////
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = profile->buddyStatus->ip;
	address.sin_port = (u_short)profile->buddyStatus->port;
	rcode = connect(peer->sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	if(rcode == SOCKET_ERROR)
	{
#if 0
		int error = GOAGetLastError(peer->sock);
		if((error != WSAEWOULDBLOCK) && (error != WSAEINPROGRESS) && (error != WSAETIMEDOUT) )
		{
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error connecting a socket.");
		}
#else
        assert(FALSE);
#endif
	}

	// We're waiting for the connect to complete.
	/////////////////////////////////////////////
	peer->state = GPI_PEER_CONNECTING;

	return GP_NO_ERROR;
}

GPResult
gpiPeerAddMessage(
  GPConnection * connection,
  GPIPeer * peer,
  int type,
  const char * message
)
{
	GPIMessage gpiMessage;
	int len;

	assert(peer != NULL);
	assert(message != NULL);

	// Get the length.
	//////////////////
	len = (int)strlen(message);

	// Clear the message.
	/////////////////////
	memset(&gpiMessage, 0, sizeof(GPIMessage));

	// Copy the type.
	/////////////////
	gpiMessage.type = type;

	// Copy the header to the buffer.
	/////////////////////////////////
	CHECK_RESULT(gpiAppendStringToBuffer(connection, &gpiMessage.buffer, "\\m\\"));
	CHECK_RESULT(gpiAppendIntToBuffer(connection, &gpiMessage.buffer, type));
	CHECK_RESULT(gpiAppendStringToBuffer(connection, &gpiMessage.buffer, "\\len\\"));
	CHECK_RESULT(gpiAppendIntToBuffer(connection, &gpiMessage.buffer, len));
	CHECK_RESULT(gpiAppendStringToBuffer(connection, &gpiMessage.buffer, "\\msg\\\n"));

	// Copy the message to the buffer.
	//////////////////////////////////
	gpiMessage.start = gpiMessage.buffer.len;
	CHECK_RESULT(gpiAppendStringToBufferLen(connection, &gpiMessage.buffer, message, len));
	CHECK_RESULT(gpiAppendCharToBuffer(connection, &gpiMessage.buffer, '\0'));

	// Add it to the list.
	//////////////////////
	ArrayAppend(peer->messages, &gpiMessage);

	// Reset the timeout.
	/////////////////////
	peer->timeout = (time(NULL) + GPI_PEER_TIMEOUT);

	return GP_NO_ERROR;
}

GPResult
gpiPeerStartTransferMessage(
  GPConnection * connection,
  GPIPeer * peer,
  int type,
  GPITransferID_st transferID
)
{
	char buffer[64];
	GPITransferID * tid = (GPITransferID *)transferID;

	assert(transferID);

	// Start the message.
	/////////////////////
	sprintf(buffer, "\\m\\%d\\xfer\\%d %u %u", type, tid->profileid, tid->count, tid->time);

	return gpiSendOrBufferString(connection, peer, buffer);
}

GPResult
gpiPeerFinishTransferMessage(
  GPConnection * connection,
  GPIPeer * peer,
  const char * message,
  int len
)
{
	char buffer[32];

	assert(peer != NULL);

	// Check the message.
	/////////////////////
	if(!message)
		message = "";
	if(len == -1)
		len = (int)strlen(message);

	// Set the len and the message.
	///////////////////////////////
	sprintf(buffer, "\\len\\%d\\msg\\\n", len);
	CHECK_RESULT(gpiSendOrBufferString(connection, peer, buffer));

	// Copy the message to the buffer.
	//////////////////////////////////
	CHECK_RESULT(gpiSendOrBufferStringLen(connection, peer, message, len));
	CHECK_RESULT(gpiSendOrBufferChar(connection, peer, '\0'));

	// Reset the timeout.
	/////////////////////
	peer->timeout = (time(NULL) + GPI_PEER_TIMEOUT);

	return GP_NO_ERROR;
}
