#include "nninternal.h"
#include "../darray.h"
#include "../available.h"
#include <stddef.h>
#include <stdio.h>


//#define DP(a) a;
#define DP(a) 

unsigned char NNMagicData[] = {NN_MAGIC_0, NN_MAGIC_1, NN_MAGIC_2, NN_MAGIC_3, NN_MAGIC_4, NN_MAGIC_5};
struct _NATNegotiator
{
	SOCKET negotiateSock;
	SOCKET gameSock;
	int cookie;
	int clientindex;
	NegotiateState state;
	int initAckRecv[3];
	int retryCount;
	int maxRetryCount;
	unsigned long retryTime;
	unsigned int guessedIP;
	unsigned short guessedPort;
	unsigned char gotRemoteData;
	NegotiateProgressFunc progressCallback;
	NegotiateCompletedFunc completedCallback;
	void *userdata;
};

typedef struct _NATNegotiator *NATNegotiator;

static DArray negotiateList = NULL;

char *Matchup1Hostname = MATCHUP1_HOSTNAME;
char *Matchup2Hostname = MATCHUP2_HOSTNAME;


static unsigned int matchup1ip = 0;
static unsigned int matchup2ip = 0;
 

static NATNegotiator FindNegotiatorForCookie(int cookie)
{
	int i;
	if (negotiateList == NULL)
		return NULL;
	for (i = 0 ; i < ArrayLength(negotiateList) ; i++)
	{
		//we go backwards in case we need to remove one..
		NATNegotiator neg = (NATNegotiator)ArrayNth(negotiateList, i);
		if (neg->cookie == cookie)
			return neg;
	}
	return NULL;
}


static NATNegotiator AddNegotiator()
{
	
	struct _NATNegotiator _neg;
	

	memset(&_neg, 0, sizeof(_neg));

	if (negotiateList == NULL)
		negotiateList = ArrayNew(sizeof(_neg), 4, NULL);

	ArrayAppend(negotiateList, &_neg);

	return (NATNegotiator)ArrayNth(negotiateList, ArrayLength(negotiateList) - 1);
}


static void RemoveNegotiator(NATNegotiator neg)
{
	int i;
	for (i = 0 ; i < ArrayLength(negotiateList) ; i++)
	{
		//we go backwards in case we need to remove one..
		if (neg == (NATNegotiator)ArrayNth(negotiateList, i))
		{
			ArrayRemoveAt(negotiateList, i);
			return;

		}
	}


}

void NNFreeNegotiateList()
{
	if (negotiateList != NULL)
	{
		ArrayFree(negotiateList);
		negotiateList = NULL;
	}
}



static int CheckMagic(unsigned char *data)
{
	return (memcmp(data, NNMagicData, NATNEG_MAGIC_LEN) == 0);
}

static void SendPacket(SOCKET sock, unsigned int toaddr, unsigned short toport, void *data, int len)
{
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(toport);
	saddr.sin_addr.s_addr = toaddr;
	sendto(sock, (char *)data, len, 0, (struct sockaddr *)&saddr, sizeof(saddr));
}



static unsigned int GetLocalIP()
{
	int num_local_ips;
	struct hostent *phost;
	struct in_addr *addr;
	unsigned int localip = 0;
	phost = getlocalhost();
	if (phost == NULL)
		return 0;
	for (num_local_ips = 0 ; ; num_local_ips++)
	{
		if (phost->h_addr_list[num_local_ips] == 0)
			break;
		addr = (struct in_addr *)phost->h_addr_list[num_local_ips];
		if (addr->s_addr == htonl(0x7F000001))
			continue;
		localip = addr->s_addr;

		if(IsPrivateIP(addr))
			return localip;
	}
	return localip; //else a specific private address wasn't found - return what we've got
}

static unsigned short GetLocalPort(SOCKET sock)
{
	int ret;
	struct sockaddr_in saddr;
	int saddrlen = sizeof(saddr);
	ret = getsockname(sock,(struct sockaddr *)&saddr, &saddrlen);
	if (ret == SOCKET_ERROR)
		return 0;
	return saddr.sin_port;
}

static void SendInitPackets(NATNegotiator neg)
{
	char buffer[INITPACKET_SIZE + sizeof(__GSIACGamename)];
	InitPacket * p = (InitPacket *)buffer;
	unsigned int localip;
	unsigned short localport;
	int packetlen;

	memcpy(p->magic, NNMagicData, NATNEG_MAGIC_LEN);
	p->version = NN_PROTVER;
	p->packettype = NN_INIT;
	p->clientindex = neg->clientindex;
	p->cookie = htonl(neg->cookie);
	p->usegameport = (neg->gameSock == INVALID_SOCKET) ? 0 : 1;
	localip = ntohl(GetLocalIP());
	//ip
	buffer[INITPACKET_ADDRESS_OFFSET] = ((localip >> 24) & 0xFF);
	buffer[INITPACKET_ADDRESS_OFFSET+1] = ((localip >> 16) & 0xFF);
	buffer[INITPACKET_ADDRESS_OFFSET+2] = ((localip >> 8) & 0xFF);
	buffer[INITPACKET_ADDRESS_OFFSET+3] = (localip & 0xFF);
	//port (this may not be determined until the first packet goes out)
	buffer[INITPACKET_ADDRESS_OFFSET+4] = 0;
	buffer[INITPACKET_ADDRESS_OFFSET+5] = 0;
	// add the gamename to all requests
	strcpy(buffer + INITPACKET_SIZE, __GSIACGamename);
	packetlen = (INITPACKET_SIZE + strlen(__GSIACGamename) + 1);
	if (p->usegameport && !neg->initAckRecv[NN_PT_GP])
	{
		p->porttype = NN_PT_GP;

		DP(printf("Sending INIT (GP)...\n"));
		SendPacket(neg->gameSock, matchup1ip, MATCHUP_PORT, p, packetlen);
		
	}


	if (!neg->initAckRecv[NN_PT_NN1])
	{
		p->porttype = NN_PT_NN1;
		DP(printf("Sending INIT (NN1)...\n"));

		SendPacket(neg->negotiateSock, matchup1ip, MATCHUP_PORT, p, packetlen);
	}	

	//this shuold be determined now...
	localport = ntohs(GetLocalPort((p->usegameport) ?  neg->gameSock : neg->negotiateSock));
	buffer[INITPACKET_ADDRESS_OFFSET+4] = ((localport >> 8) & 0xFF);
	buffer[INITPACKET_ADDRESS_OFFSET+5] = (localport & 0xFF);

	if (!neg->initAckRecv[NN_PT_NN2])
	{
		p->porttype = NN_PT_NN2;
		DP(printf("Sending INIT (NN2)...\n"));
		SendPacket(neg->negotiateSock, matchup2ip, MATCHUP_PORT,p, packetlen);
	}

	neg->retryTime = current_time() + INIT_RETRY_TIME;
	neg->maxRetryCount = INIT_RETRY_COUNT;
}

static void SendPingPacket(NATNegotiator neg)
{
	ConnectPacket p;
	memcpy(p.magic, NNMagicData, NATNEG_MAGIC_LEN);
	p.version = NN_PROTVER;
	p.packettype = NN_CONNECT_PING;
	p.cookie = htonl(neg->cookie);
	p.remoteIP = neg->guessedIP;
	p.remotePort = htons(neg->guessedPort);
	p.gotyourdata = neg->gotRemoteData;
	p.finished = (neg->state == ns_connectping) ? 0 : 1;

	DP(printf("Sending PING to %s:%d (got remote data: %d)\n", inet_ntoa(*(struct in_addr *)&neg->guessedIP), neg->guessedPort,   neg->gotRemoteData));
	SendPacket((neg->gameSock != INVALID_SOCKET) ? neg->gameSock : neg->negotiateSock, neg->guessedIP, neg->guessedPort,  &p, CONNECTPACKET_SIZE);
	neg->retryTime = current_time() + PING_RETRY_TIME;
	neg->maxRetryCount = PING_RETRY_COUNT;
}


NegotiateError NNBeginNegotiation(int cookie, int clientindex, NegotiateProgressFunc progresscallback, NegotiateCompletedFunc completedcallback, void *userdata)
{
	return NNBeginNegotiationWithSocket(INVALID_SOCKET, cookie, clientindex, progresscallback, completedcallback, userdata);
}

static unsigned int NameToIp(const char *name)
{
	unsigned int ret;
	struct hostent *hent;

	ret = inet_addr(name);
	
	if (ret == INADDR_NONE)
	{
		hent = gethostbyname(name);
		if (!hent)
			return 0;
		ret = *(unsigned int *)hent->h_addr_list[0];
	}
	return ret;
}

static int ResolveServers()
{
	if (matchup1ip == 0)
		matchup1ip = NameToIp(Matchup1Hostname);
	if (matchup2ip == 0)
		matchup2ip = NameToIp(Matchup2Hostname);

	if (matchup1ip == 0 || matchup2ip == 0)
		return 0;
	return 1;

	
}

NegotiateError NNBeginNegotiationWithSocket(SOCKET gamesocket, int cookie, int clientindex, NegotiateProgressFunc progresscallback, NegotiateCompletedFunc completedcallback, void *userdata)
{
	NATNegotiator neg;
	
	// check if the backend is available
	if(__GSIACResult != GSIACAvailable)
		return ne_socketerror;
	
	neg = AddNegotiator();
	if (!ResolveServers())
		return ne_dnserror;
	if (neg == NULL)
		return ne_allocerror;
	neg->gameSock = gamesocket;
	neg->clientindex = clientindex;
	neg->cookie = cookie;
	neg->progressCallback = progresscallback;
	neg->completedCallback = completedcallback;
	neg->userdata = userdata;
	neg->negotiateSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	neg->retryCount = 0;
	neg->gotRemoteData = 0;
	neg->guessedIP = 0;
	neg->guessedPort = 0;
	neg->maxRetryCount = 0;
	if (neg->negotiateSock == INVALID_SOCKET)
	{
		RemoveNegotiator(neg);
		return ne_socketerror;
	}
	SendInitPackets(neg);

	DP(
	{
		struct sockaddr_in saddr;
		int namelen = sizeof(saddr);
		getsockname(neg->negotiateSock, (struct sockaddr *)&saddr, &namelen);
		printf("Negotiate Socket: %d\n", ntohs(saddr.sin_port));
	}
	);

	return ne_noerror;
}

void NNCancel(int cookie)
{
	NATNegotiator neg = FindNegotiatorForCookie(cookie);
	if (neg == NULL)
		return;
	if (neg->negotiateSock != INVALID_SOCKET)
		closesocket(neg->negotiateSock);
	neg->negotiateSock = INVALID_SOCKET;
	neg->state = ns_canceled;
}

static void NegotiateThink(NATNegotiator neg)
{
	//check for any incoming data
	static char indata[NNINBUF_LEN]; //256 byte input buffer
	struct sockaddr_in saddr;
	int saddrlen = sizeof(struct sockaddr_in);
	int error;

	if (neg->state == ns_canceled) //we need to remove it
	{
		DP(printf("Removing canceled negotiator\n"));
		RemoveNegotiator(neg);
		return;
	}

	if (neg->negotiateSock != INVALID_SOCKET)
	{
		//first, socket processing
		while (1)
		{
			if(!CanReceiveOnSocket(neg->negotiateSock))
				break;
			//else we have data
			error = recvfrom(neg->negotiateSock, indata, NNINBUF_LEN, 0, (struct sockaddr *)&saddr, &saddrlen);
			if (error == SOCKET_ERROR)
			{
				DP(printf("RECV SOCKET ERROR: %d\n", GOAGetLastError(neg->negotiateSock)));
				break;
			}

			NNProcessData(indata, error, &saddr);
			if (neg->state == ns_canceled)
				break;

			if (neg->negotiateSock == INVALID_SOCKET)
				break;
		}
	}

	if (neg->state == ns_initsent || neg->state == ns_connectping) //see if we need to resend init packets
	{
		if (current_time() > neg->retryTime)
		{
			if (neg->retryCount > neg->maxRetryCount)
			{
				DP(printf("RETRY FAILED...\n"));
				neg->completedCallback(nr_inittimeout, INVALID_SOCKET, NULL, neg->userdata);
				NNCancel(neg->cookie); //mark to-be-canceled
			} else
			{
				
				neg->retryCount++;
				if (neg->state == ns_initsent) //resend init packets
					SendInitPackets(neg);
				else
					SendPingPacket(neg); //resend ping packet
				DP(printf("[retry]\n"));
			}

		}
	}

	if (neg->state == ns_finished && current_time() > neg->retryTime) //check if it is ready to be removed
	{
		if (neg->gameSock == INVALID_SOCKET) //we are using the negotiate socket, time to trigger the callback
		{
			struct sockaddr_in saddr;
			saddr.sin_family = AF_INET;
			saddr.sin_port = htons(neg->guessedPort);
			saddr.sin_addr.s_addr = neg->guessedIP;
			neg->completedCallback(nr_success, neg->negotiateSock, &saddr, neg->userdata);
			neg->negotiateSock = INVALID_SOCKET; //don't let cancel close this socket
		}

		NNCancel(neg->cookie); //mark it to be killed
	}
	
	if (neg->state == ns_initack && current_time() > neg->retryTime) //see if the partner has timed out
	{
		neg->completedCallback(nr_deadbeatpartner, INVALID_SOCKET, NULL, neg->userdata);
		NNCancel(neg->cookie); //mark to-be-canceled
	}

	
	
}

void NNThink()
{
	int i;
	if (negotiateList == NULL)
		return;
	for (i = ArrayLength(negotiateList) - 1 ; i >= 0 ; i--)
	{
		//we go backwards in case we need to remove one..
		NegotiateThink((NATNegotiator)ArrayNth(negotiateList, i));
	}
}

static void SendConnectAck(NATNegotiator neg, struct sockaddr_in *toaddr)
{
	InitPacket p;

	memcpy(p.magic, NNMagicData, NATNEG_MAGIC_LEN);
	p.version = NN_PROTVER;
	p.packettype = NN_CONNECT_ACK;
	p.clientindex = neg->clientindex;
	p.cookie = htonl(neg->cookie);

	DP(printf("Sending connect ack...\n"));
	SendPacket(neg->negotiateSock, toaddr->sin_addr.s_addr, ntohs(toaddr->sin_port), &p, INITPACKET_SIZE);
	
	

}

static void ProcessConnectPacket(NATNegotiator neg, ConnectPacket *p, struct sockaddr_in *fromaddr)
{
	DP(printf("Got connect packet (finish code: %d), guess: %s:%d\n", p->finished, inet_ntoa(*(struct in_addr *)&p->remoteIP), ntohs(p->remotePort)));
	//send an ack..
	if (p->finished == FINISHED_NOERROR) //don't need to ack any errors
		SendConnectAck(neg, fromaddr);

	
	if (neg->state >= ns_connectping)
		return; //don't process it any further

	if (p->finished != FINISHED_NOERROR) //call the completed callback with the error code
	{
		NegotiateResult errcode;
		errcode = nr_unknownerror; //default if unknown
		if (p->finished == FINISHED_ERROR_DEADBEAT_PARTNER)
			errcode = nr_deadbeatpartner;
		else if (p->finished == FINISHED_ERROR_INIT_PACKETS_TIMEDOUT)
			errcode = nr_inittimeout;
		neg->completedCallback(errcode, INVALID_SOCKET, NULL, neg->userdata);
		NNCancel(neg->cookie); //mark to-be-canceled
		return;
	}

	neg->guessedIP = p->remoteIP;
	neg->guessedPort = ntohs(p->remotePort);
	neg->retryCount = 0;

	neg->state = ns_connectping;
	neg->progressCallback(neg->state, neg->userdata);

	SendPingPacket(neg);
	
}


static void ProcessPingPacket(NATNegotiator neg, ConnectPacket *p, struct sockaddr_in *fromaddr)
{
	
	if (neg->state < ns_connectping)
		return;
	//update our guessed ip and port
	DP(printf("Got ping from: %s:%d (gotmydata: %d, finished: %d)\n", inet_ntoa(fromaddr->sin_addr), ntohs(fromaddr->sin_port), p->gotyourdata, p->finished));

	neg->guessedIP = fromaddr->sin_addr.s_addr;
	neg->guessedPort = ntohs(fromaddr->sin_port);
	neg->gotRemoteData = 1;

	if (!p->gotyourdata) //send another packet until they have our data
		SendPingPacket(neg);
	else //they have our data, and we have their data - it's a connection!
	{
		if (neg->state == ns_connectping) //advance it
		{
			DP(printf("CONNECT FINISHED\n"));

			//we need to leave it around for a while to process any incoming data.
			neg->state = ns_finished;			
			neg->retryTime = current_time() + FINISHED_IDLE_TIME;
			if (neg->gameSock != INVALID_SOCKET) //we can trigger the callback right now
				neg->completedCallback(nr_success, neg->gameSock, fromaddr, neg->userdata);

			
			
		} else if (!p->finished)
			SendPingPacket(neg);
	}



}

static void ProcessInitPacket(NATNegotiator neg, InitPacket *p, struct sockaddr_in *fromaddr)
{
	switch (p->packettype)
	{
	case NN_INITACK:
		//mark our init as ack'd
		if (p->porttype > 2)
			return; //invalid port
		DP(printf("Got init ack for port %d\n", p->porttype));
		neg->initAckRecv[p->porttype] = 1;
		if (neg->state == ns_initsent) //see if we can advance to negack
		{
			if (neg->initAckRecv[NN_PT_NN1] != 0 && neg->initAckRecv[NN_PT_NN2] != 0 && 
				(neg->gameSock == INVALID_SOCKET ||  neg->initAckRecv[NN_PT_GP] != 0))
			{
				neg->state = ns_initack;
				neg->retryTime = current_time() + PARTNER_WAIT_TIME;
				neg->progressCallback(neg->state, neg->userdata);
			}
		}
		break;

	case NN_ERTTEST:
		//we just send the packet back where it came from..
		DP(printf("Got ERT\n"));
		p->packettype = NN_ERTACK;
		SendPacket(neg->negotiateSock, fromaddr->sin_addr.s_addr, ntohs(fromaddr->sin_port), p, INITPACKET_SIZE);
		break;
	}
}



void NNProcessData(char *data, int len, struct sockaddr_in *fromaddr)
{
	ConnectPacket cp;
	InitPacket ip;
	NATNegotiator neg;
	unsigned char ptype;

	DP(printf("Process data, %d bytes (%s:%d)\n", len, inet_ntoa(fromaddr->sin_addr), ntohs(fromaddr->sin_port)));

	if (!CheckMagic(data))
		return; //invalid packet

	ptype = *(unsigned char *)(data + offsetof(InitPacket, packettype));
	if (ptype == NN_CONNECT || ptype == NN_CONNECT_PING)
	{ //it's a connect packet
		if (len < CONNECTPACKET_SIZE)
			return;
		memcpy(&cp, data, CONNECTPACKET_SIZE);		
		neg = FindNegotiatorForCookie(ntohl(cp.cookie));
		if (neg)
		{
			if (ptype == NN_CONNECT)
				ProcessConnectPacket(neg, &cp, fromaddr);
			else
				ProcessPingPacket(neg, &cp, fromaddr);
		}
			

	} else //it's an init packet
	{
		if (len < INITPACKET_SIZE)
			return;
		memcpy(&ip, data, INITPACKET_SIZE);		
		neg = FindNegotiatorForCookie(ntohl(ip.cookie));
		if (neg)
			ProcessInitPacket(neg, &ip, fromaddr);

	}
}

