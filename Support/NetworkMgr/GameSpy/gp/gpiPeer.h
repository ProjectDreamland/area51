/*
gpiPeer.h
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

#ifndef _GPIPEER_H_
#define _GPIPEER_H_

//INCLUDES
//////////
#include "gpi.h"

//DEFINES
/////////
// Peer states.
///////////////
#define GPI_PEER_NOT_CONNECTED       100
#define GPI_PEER_GETTING_SIG         101
#define GPI_PEER_GOT_SIG             102
#define GPI_PEER_CONNECTING          103
#define GPI_PEER_WAITING             104
#define GPI_PEER_CONNECTED           105
#define GPI_PEER_DISCONNECTED        106

// Timeout for a peer connection, in seconds.
/////////////////////////////////////////////
#define GPI_PEER_TIMEOUT               (5 * 60)

typedef struct GPITransferID_s * GPITransferID_st;

//TYPES
///////
// A peer message.
//////////////////
typedef struct GPIMessage
{
	GPIBuffer buffer;
	int type;
	int start;
} GPIMessage;

// A peer connection.
/////////////////////
typedef struct GPIPeer_s
{
	int state;
	GPIBool initiated;
	SOCKET sock;
	GPProfile profile;
	time_t timeout;
	int nackCount;
	GPIBuffer inputBuffer;
	GPIBuffer outputBuffer;
	DArray messages;
	struct GPIPeer_s * pnext;
} GPIPeer;

//FUNCTIONS
///////////
GPResult
gpiProcessPeers(
  GPConnection * connection
);

GPResult
gpiPeerGetSig(
  GPConnection * connection,
  GPIPeer * peer
);

GPResult
gpiPeerStartConnect(
  GPConnection * connection,
  GPIPeer * peer
);

GPIPeer *
gpiGetConnectedPeer(
  GPConnection * connection,
  int profileid
);

GPIPeer *
gpiAddPeer(
  GPConnection * connection,
  int profileid,
  GPIBool initiate
);

void
gpiDestroyPeer(
  GPConnection * connection,
  GPIPeer * peer
);

void
gpiRemovePeer(
  GPConnection * connection,
  GPIPeer * peer
);

GPResult
gpiPeerAddMessage(
  GPConnection * connection,
  GPIPeer * peer,
  int type,
  const char * message
);

GPResult
gpiPeerStartTransferMessage(
  GPConnection * connection,
  GPIPeer * peer,
  int type,
  GPITransferID_st transferID
);

GPResult
gpiPeerFinishTransferMessage(
  GPConnection * connection,
  GPIPeer * peer,
  const char * message,
  int len
);

GPResult
gpiPeerSendMessages(
  GPConnection * connection,
  GPIPeer * peer
);

#endif
