#include "sb_serverbrowsing.h"
#include "sb_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#ifdef GSI_MANIC_DEBUG
// Make sure the server isn't already in the fifo
void FIFODebugCheckAdd(SBServerFIFO *fifo, SBServer server)
{
	SBServer aServer = fifo->first;
	while(aServer != NULL)
	{
		assert(aServer != server);
		aServer = aServer->next;
	}
}


// Verify the contents of the fifo
void FIFODebugCheck(SBServerFIFO *fifo)
{
	int i=0;
	SBServer aServer;

	assert(fifo != NULL);
	aServer = fifo->first;
	for (i=0; i < fifo->count; i++)
	{
		assert(aServer != NULL);
		aServer = aServer->next;
	}
}
#else
#define FIFODebugCheckAdd(a,b)
#define FIFODebugCheck(a)
#endif

//FIFO Queue management functions
static void FIFOAddRear(SBServerFIFO *fifo, SBServer server)
{
	FIFODebugCheckAdd(fifo, server);

	if (fifo->last != NULL)
		fifo->last->next = server;
	fifo->last = server;
	server->next = NULL;
	if (fifo->first == NULL)
		fifo->first = server;
	fifo->count++;

	FIFODebugCheck(fifo);
}

static void FIFOAddFront(SBServerFIFO *fifo, SBServer server)
{
	FIFODebugCheckAdd(fifo, server);

	server->next = fifo->first;
	fifo->first = server;
	if (fifo->last == NULL)
		fifo->last = server;
	fifo->count++;

	FIFODebugCheck(fifo);
}

static SBServer FIFOGetFirst(SBServerFIFO *fifo)
{
	SBServer hold;
	hold = fifo->first;
	if (hold != NULL)
	{
		fifo->first = hold->next;
		if (fifo->first == NULL)
			fifo->last = NULL;
		fifo->count--;
	}

	FIFODebugCheck(fifo);
	return hold;
}

static SBBool FIFORemove(SBServerFIFO *fifo, SBServer server)
{
	SBServer hold, prev;
	prev = NULL;
	hold = fifo->first;
	while (hold != NULL)
	{
		if (hold == server) //found
		{
			if (prev != NULL) //there is a previous..
				prev->next = hold->next;
			if (fifo->first == hold)
				fifo->first = hold->next;
			if (fifo->last == hold)
				fifo->last = prev;
			fifo->count--;
		//	assert((fifo->count == 0 && fifo->first == NULL && fifo->last == NULL) || fifo->count > 0);
			return SBTrue;
		}
		prev = hold;
		hold = hold->next;
	}

	FIFODebugCheck(fifo);
	return SBFalse;
}

static void FIFOClear(SBServerFIFO *fifo)
{
	fifo->first = fifo->last = NULL;
	fifo->count = 0;

	FIFODebugCheck(fifo);
}


static void QEStartQuery(SBQueryEngine *engine, SBServer server)
{
	unsigned char queryBuffer[256];
	int queryLen;
	struct sockaddr_in saddr;
	int i;
	//add it to the query list
	FIFOAddRear(&engine->querylist, server);
	server->updatetime = current_time();
	if (engine->queryversion == QVERSION_QR2)
	{
		//set the header
		queryBuffer[0] = QR2_MAGIC_1;
		queryBuffer[1] = QR2_MAGIC_2;
		queryBuffer[2] = 0;
		//set the request key
		memcpy(&queryBuffer[3], &server->updatetime, 4);		
		if (server->state & STATE_PENDINGBASICQUERY) 
		{
			queryBuffer[7] = (unsigned char)engine->numserverkeys;
			for (i = 0 ; i < engine->numserverkeys ; i++)
				queryBuffer[8 + i] = engine->serverkeys[i];
			//don't request any player or team keys
			queryBuffer[8 + engine->numserverkeys] = 0x00;
			queryBuffer[9 + engine->numserverkeys] = 0x00;
			queryLen = 10 + engine->numserverkeys;			

		} else  //request all keys for everyone
		{
			queryBuffer[7] = 0xFF;
			queryBuffer[8] = 0xFF;
			queryBuffer[9] = 0xFF;
			queryLen = 10;
		}
	} else //GOA
	{
		if (server->state & STATE_PENDINGBASICQUERY) //original - do a \basic\info\ query
		{
			memcpy(queryBuffer, BASIC_GOA_QUERY, BASIC_GOA_QUERY_LEN);
			queryLen = BASIC_GOA_QUERY_LEN;
		} else //original - do a \status\ query
		{
			memcpy(queryBuffer, FULL_GOA_QUERY, FULL_GOA_QUERY_LEN);
			queryLen = FULL_GOA_QUERY_LEN;		
		}
	}
	saddr.sin_family = AF_INET;
	if (server->publicip == engine->mypublicip && (server->flags & PRIVATE_IP_FLAG)) //try querying the private IP
	{
		saddr.sin_addr.s_addr = server->privateip;
		saddr.sin_port = server->privateport;
	} else
	{
		saddr.sin_addr.s_addr = server->publicip;
		saddr.sin_port = server->publicport;
	}
#ifdef SN_SYSTEMS
	i = sendto(engine->querysock, (char *)queryBuffer, queryLen, 0, (struct sockaddr *)&saddr, sizeof(saddr));
#else
	i = (int)sendto(engine->querysock, (char *)queryBuffer, (unsigned int)queryLen, 0, (struct sockaddr *)&saddr, sizeof(saddr));
#endif
}


void SBQueryEngineInit(SBQueryEngine *engine, int maxupdates, int queryversion, SBEngineCallbackFn callback, void *instance)
{
	if(__GSIACResult != GSIACAvailable)
		return;
	SocketStartUp();
	engine->queryversion = queryversion;
	engine->maxupdates = maxupdates;
	engine->numserverkeys = 0;
	engine->ListCallback = callback;
	engine->instance = instance;
	engine->mypublicip = 0;
	engine->querysock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	FIFOClear(&engine->pendinglist);
	FIFOClear(&engine->querylist);
}

void SBQueryEngineSetPublicIP(SBQueryEngine *engine, goa_uint32 mypublicip)
{
	engine->mypublicip = mypublicip;
}

void SBEngineHaltUpdates(SBQueryEngine *engine)
{
	FIFOClear(&engine->pendinglist);
	FIFOClear(&engine->querylist);
}


void SBEngineCleanup(SBQueryEngine *engine)
{
	closesocket(engine->querysock);
	engine->querysock = INVALID_SOCKET;
	FIFOClear(&engine->pendinglist);
	FIFOClear(&engine->querylist);
}

void SBQueryEngineUpdateServer(SBQueryEngine *engine, SBServer server, int addfront, int querytype)
{
	//NOTE: the server must not be in the pending or update list currently!
	if((server->state & (STATE_PENDINGBASICQUERY|STATE_PENDINGFULLQUERY|STATE_QUERYFAILED)))
		return;
	
	server->state &= (unsigned char)~(STATE_PENDINGBASICQUERY|STATE_PENDINGFULLQUERY|STATE_QUERYFAILED); //clear out these flags
	if (querytype == QTYPE_BASIC)
		server->state |= STATE_PENDINGBASICQUERY;
	else if (querytype == QTYPE_FULL)
		server->state |= STATE_PENDINGFULLQUERY;
	
	if (engine->querylist.count < engine->maxupdates) //add it now..
	{
		QEStartQuery(engine, server);
		return;
	}
	//else need to queue it
	
	if (addfront)
		FIFOAddFront(&engine->pendinglist, server);
	else
		FIFOAddRear(&engine->pendinglist, server);
}

SBServer SBQueryEngineUpdateServerByIP(SBQueryEngine *engine, const char *ip, unsigned short queryport, int addfront, int querytype)
{
	//need to create a new server
	SBServer server;
	goa_uint32 ipaddr;
	ipaddr = inet_addr(ip);
	server = SBAllocServer(NULL, ipaddr, htons(queryport));
	server->flags = UNSOLICITED_UDP_FLAG; //we assume we can talk directly to it
	SBQueryEngineUpdateServer(engine, server, addfront, querytype);
	return server;
}


static void ParseSingleQR2Reply(SBQueryEngine *engine, SBServer server, char *data, int len)
{
	int i;
	int dlen;

	if (data[0] != 0)
		return;

	//we could test the request key here for added security, or skip
	data += 5;
	len -= 5;
	if (server->state & STATE_PENDINGBASICQUERY)
	{
		//need to pick out the keys they selected
		for (i = 0 ; i < engine->numserverkeys ; i++)
		{
			dlen = NTSLengthSB(data, len);
			if (dlen < 0)
				break;
			//add the value
			SBServerAddKeyValue(server, qr2_registered_key_list[engine->serverkeys[i]], data);
			data += dlen;
			len -= dlen;
		}
		server->state |= STATE_BASICKEYS;
	}
	else //need to parse out all the keys
	{
		SBServerParseQR2FullKeys(server, data, len);
		server->state |= STATE_FULLKEYS|STATE_BASICKEYS;
	}
	server->state &= (unsigned char)~(STATE_PENDINGBASICQUERY|STATE_PENDINGFULLQUERY);
	server->updatetime = current_time() - server->updatetime;
	FIFORemove(&engine->querylist, server);
	engine->ListCallback(engine, qe_updatesuccess, server, engine->instance);
}

static void ParseSingleGOAReply(SBQueryEngine *engine, SBServer server, char *data, int len)
{
	int isfinal;
	//need to check before parse as it will modify the string
	isfinal = (strstr(data,"\\final\\") != NULL);
	SBServerParseKeyVals(server, data);
	if (isfinal)
	{
		if (server->state & STATE_PENDINGBASICQUERY)
			server->state |= STATE_BASICKEYS;
		else
			server->state |= STATE_FULLKEYS;
		server->state &= (unsigned char)~(STATE_PENDINGBASICQUERY|STATE_PENDINGFULLQUERY);
		server->updatetime = current_time() - server->updatetime;
		FIFORemove(&engine->querylist, server);
		engine->ListCallback(engine, qe_updatesuccess, server, engine->instance);
	}
	
	GSI_UNUSED(len);
}


static void ProcessIncomingReplies(SBQueryEngine *engine)
{
	int i;
	char indata[MAX_RECVFROM_SIZE]; 
	struct sockaddr_in saddr;
	int saddrlen = sizeof(saddr);
	SBServer server;
	
	// Process all information in the socket buffer 
	while(CanReceiveOnSocket(engine->querysock))
	{
		i = (int)recvfrom(engine->querysock, indata, sizeof(indata) - 1, 0, (struct sockaddr *)&saddr, &saddrlen);
		if (i == SOCKET_ERROR)
			break;
		indata[i] = 0;
		//find the server in our query list
		for (server = engine->querylist.first ; server != NULL ; server = server->next)
		{
			if ((server->publicip == saddr.sin_addr.s_addr && server->publicport == saddr.sin_port) || //if it matches public
				(server->publicip == engine->mypublicip && (server->flags & PRIVATE_IP_FLAG) && server->privateip == saddr.sin_addr.s_addr && server->privateport == saddr.sin_port)) //or has a private, and matches
			{
				if (engine->queryversion == QVERSION_QR2)
					ParseSingleQR2Reply(engine, server, indata, i);
				else
					ParseSingleGOAReply(engine, server, indata, i);
				break;
			}
		}		
	}
}

static void TimeoutOldQueries(SBQueryEngine *engine)
{
	gsi_time ctime = current_time();
	while (engine->querylist.first != NULL)
	{
		if (ctime > engine->querylist.first->updatetime + MAX_QUERY_MSEC)
		{
			engine->querylist.first->flags |= STATE_QUERYFAILED;
			engine->querylist.first->flags  &= (unsigned char)~(STATE_PENDINGBASICQUERY|STATE_PENDINGFULLQUERY);
			engine->ListCallback(engine, qe_updatefailed, engine->querylist.first, engine->instance);			
			FIFOGetFirst(&engine->querylist);
		} else
			break; //since servers are added in FIFO order, nothing later can have already expired
	}
}

static void QueueNextQueries(SBQueryEngine *engine)
{
	while (engine->querylist.count < engine->maxupdates && engine->pendinglist.count > 0)
	{
		SBServer server = FIFOGetFirst(&engine->pendinglist);
		QEStartQuery(engine, server);
	}
}

void SBQueryEngineThink(SBQueryEngine *engine)
{
	if (engine->querylist.count == 0) //not querying anything - we can go away
		return;
	ProcessIncomingReplies(engine);
	TimeoutOldQueries(engine);
	if (engine->pendinglist.count > 0)
		QueueNextQueries(engine);
	if (engine->querylist.count == 0) //we are now idle..
		engine->ListCallback(engine, qe_engineidle, NULL, engine->instance);
}

void SBQueryEngineAddQueryKey(SBQueryEngine *engine, unsigned char keyid)
{
	if (engine->numserverkeys < MAX_QUERY_KEYS)
		engine->serverkeys[engine->numserverkeys++] = keyid;
}


//remove a server from our update FIFOs
void SBQueryEngineRemoveServerFromFIFOs(SBQueryEngine *engine, SBServer server)
{
	SBBool ret;

	ret = FIFORemove(&engine->querylist, server);
	if (ret)
		return;
	FIFORemove(&engine->pendinglist, server);
}
