/********
INCLUDES
********/
#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__mips64) && !defined(_WIN32)
#include "::nonport.h"
#include "::stringutil.h"
#else
#include "../nonport.h"
#include "../stringutil.h"
#endif
#include "qr2.h"
#include "qr2regkeys.h"
#include <stdlib.h>
#include <stdio.h>


#include <string.h>

#if defined(_PS2) && defined(GSI_UNICODE)
// A few prototypes to prevent warnings
size_t wcslen( const wchar_t *string );
wchar_t *wcscpy( wchar_t *strDestination, const wchar_t *strSource );
#endif

#ifdef __cplusplus
extern "C" {
#endif	
	
#ifdef __MWERKS__ // Codewarrior requires function prototypes
qr2_error_t qr2_initW(/*[out]*/qr2_t *qrec, const unsigned short *ip, int baseport, const unsigned short *gamename, 
					 const unsigned short *secret_key, int ispublic, int natnegotiate, qr2_serverkeycallback_t server_key_callback, 
					 qr2_playerteamkeycallback_t player_key_callback, qr2_playerteamkeycallback_t team_key_callback, qr2_keylistcallback_t key_list_callback, 
					 qr2_countcallback_t playerteam_count_callback, qr2_adderrorcallback_t adderror_callback, void *userdata);
qr2_error_t qr2_init_socketW(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const unsigned short *gamename, 
					 const unsigned short *secret_key, int ispublic, int natnegotiate, qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback, qr2_playerteamkeycallback_t team_key_callback, qr2_keylistcallback_t key_list_callback, 
					 qr2_countcallback_t playerteam_count_callback, qr2_adderrorcallback_t adderror_callback, void *userdata);
qr2_error_t qr2_init_socketA(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const char *gamename, const char *secret_key,
					 int ispublic, int natnegotiate, qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback, qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback, qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback, void *userdata);
qr2_error_t qr2_initA(/*[out]*/qr2_t *qrec, const char *ip, int baseport, const char *gamename, const char *secret_key,
					 int ispublic, int natnegotiate, qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback, qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback, qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback, void *userdata);
#endif

/********
DEFINES
********/
#define MASTER_PORT 27900
#define MASTER_ADDR "master.gamespy.com"
	//#define MASTER_ADDR "207.199.80.230"
#define FIRST_HB_TIME 10000 /* 10 sec */
#define HB_TIME 60000 /* 1 minute */
#define KA_TIME 20000 /* 20 sec */
#define MIN_STATECHANGED_HB_TIME 10000 /* 10 sec */
#define MAX_FIRST_COUNT 4 /* 4 tries */
#define MAX_DATA_SIZE 2048
#define INBUF_LEN 256
#define PUBLIC_ADDR_LEN 12
	
#define PACKET_QUERY 0x00
#define PACKET_CHALLENGE 0x01
#define PACKET_ECHO 0x02
#define PACKET_ECHO_RESPONSE 0x05
#define PACKET_HEARTBEAT 0x03
#define PACKET_ADDERROR 0x04
#define PACKET_CLIENT_MESSAGE 0x06
#define PACKET_CLIENT_MESSAGE_ACK 0x07
#define PACKET_KEEPALIVE 0x08

	
#define MAX_LOCAL_IP 5

//magic bytes for nat negotiation message
#define NATNEG_MAGIC_LEN 6
#define NN_MAGIC_0 0xFD
#define NN_MAGIC_1 0xFC
#define NN_MAGIC_2 0x1E
#define NN_MAGIC_3 0x66
#define NN_MAGIC_4 0x6A
#define NN_MAGIC_5 0xB2


/********
TYPEDEFS
********/
typedef unsigned char uchar;

struct qr2_keybuffer_s
{
	uchar keys[MAX_REGISTERED_KEYS];
	int numkeys;
};

struct qr2_buffer_s
{
	char buffer[MAX_DATA_SIZE];
	int len;
};

#define AVAILABLE_BUFFER_LEN(a) (MAX_DATA_SIZE - (a)->len)

/********
VARS
********/
struct qr2_implementation_s static_qr2_rec; //* INEV*BW need a full list of initializers! Not just for first param = {INVALID_SOCKET};
static qr2_t current_rec = &static_qr2_rec;
char qr2_hostname[64];

static int num_local_ips = 0;
static struct in_addr local_ip_list[MAX_LOCAL_IP];


/********
PROTOTYPES
********/
static void send_heartbeat(qr2_t qrec, int statechanged);
static void send_keepalive(qr2_t qrec);
static int get_sockaddrin(const char *host, int port, struct sockaddr_in *saddr, struct hostent **savehent);
static void qr2_check_queries(qr2_t qrec);
static void qr2_check_send_heartbeat(qr2_t qrec);
static void enum_local_ips();
qr2_error_t qr2_create_socket(/*[out]*/SOCKET *sock, const char *ip, /*[in/out]*/int * port);

/****************************************************************************/
/* PUBLIC FUNCTIONS */
/****************************************************************************/
// some prototypes
void qr2_parse_queryA(qr2_t qrec, char* query, int len, struct sockaddr *sender);
void qr2_parse_queryW(qr2_t qrec, unsigned short* query, int len, struct sockaddr *sender);
void qr2_buffer_addA(qr2_buffer_t outbuf, const char *value);
void qr2_buffer_addW(qr2_buffer_t outbuf, const unsigned short *value);


/* qr2_init: Initializes the sockets, etc. Returns an error value
if an error occured, or 0 otherwise */
qr2_error_t qr2_init_socketA(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const char *gamename, const char *secret_key,
					 int ispublic, int natnegotiate,
					 qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback,
					 qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback,
					 qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback,
					 void *userdata)
{
	char hostname[64];
	int ret;
	int i;
	qr2_t cr;
	if (qrec == NULL)
	{
		cr = &static_qr2_rec;		
        cr->hbsock = INVALID_SOCKET;
	}
	else
	{
		*qrec = (qr2_t)gsimalloc(sizeof(struct qr2_implementation_s));
		cr = *qrec;
	}
	srand((unsigned int)current_time());
	strcpy(cr->gamename,gamename);
	strcpy(cr->secret_key,secret_key);
	cr->qport = boundport;
	cr->lastheartbeat = 0;
	cr->lastka = 0;
	cr->hbsock = s;
	cr->listed_state = 1;
	cr->udata = userdata;
	cr->server_key_callback = server_key_callback;
	cr->player_key_callback = player_key_callback;
	cr->team_key_callback = team_key_callback;
	cr->key_list_callback = key_list_callback;
	cr->playerteam_count_callback = playerteam_count_callback;
	cr->adderror_callback = adderror_callback;
	cr->nn_callback = NULL;
	cr->cm_callback = NULL;
	cr->cdkeyprocess = NULL;
	cr->ispublic = ispublic;
	cr->read_socket = 0;
	cr->nat_negotiate = natnegotiate;
	cr->publicip = 0;
	cr->publicport = 0;
	cr->pa_callback = NULL;
	cr->userstatechangerequested = 0;

	for (i = 0 ; i < REQUEST_KEY_LEN ; i++)
		cr->instance_key[i] = (char)(rand() % 0xFF);
	for (i = 0 ; i < RECENT_CLIENT_MESSAGES_TO_TRACK ; i++)
		cr->client_message_keys[i] = -1;
	cr->cur_message_key = 0;
	
	if (num_local_ips == 0)
		enum_local_ips();
	if (ispublic)
	{
		int override = qr2_hostname[0];
		if(!override)
			sprintf(hostname, "%s.master.gamespy.com", gamename);
		ret = get_sockaddrin(override?qr2_hostname:hostname, MASTER_PORT, &(cr->hbaddr), NULL);
	}
	else //don't need to look up
		ret = 1;
	if (!ret)
		return e_qrdnserror;
	else
		return e_qrnoerror;


}

qr2_error_t qr2_init_socketW(/*[out]*/qr2_t *qrec, SOCKET s, int boundport, const unsigned short *gamename, const unsigned short *secret_key,
					 int ispublic, int natnegotiate,
					 qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback,
					 qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback,
					 qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback,
					 void *userdata)
{
	char gamename_A[255];
	char secretkey_A[255];

	UCS2ToAsciiString(gamename, gamename_A);
	UCS2ToAsciiString(secret_key, secretkey_A);

	return qr2_init_socketA(qrec, s, boundport, gamename_A, secretkey_A, ispublic, natnegotiate,
							server_key_callback, player_key_callback, team_key_callback,
							key_list_callback, playerteam_count_callback, adderror_callback, userdata);
}

qr2_error_t qr2_create_socket(/*[out]*/SOCKET *sock, const char *ip, /*[in/out]*/int * port)
{
	struct sockaddr_in saddr;	
	SOCKET hbsock;
	int maxport;
	int lasterror = 0;
	int saddrlen;
	int baseport = *port;

	SocketStartUp();
	
	hbsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == hbsock)
	{
		return e_qrwsockerror;
	}
	
	maxport = baseport + NUM_PORTS_TO_TRY;
	while (baseport < maxport)
	{
		get_sockaddrin(ip,baseport,&saddr,NULL);
		if (saddr.sin_addr.s_addr == htonl(0x7F000001)) //localhost -- we don't want that!
			saddr.sin_addr.s_addr = INADDR_ANY;
		
		
		lasterror = bind(hbsock, (struct sockaddr *)&saddr, sizeof(saddr));
		if (lasterror == 0)
			break; //we found a port
		baseport++;
	}
	
	if (lasterror != 0) //we weren't able to find a port
	{
		return e_qrbinderror;
	}
	
	if (baseport == 0) //we bound it dynamically
	{
		saddrlen = sizeof(saddr);
		lasterror = getsockname(hbsock,(struct sockaddr *)&saddr, &saddrlen);
		if (lasterror)
			return e_qrbinderror;
		baseport = ntohs(saddr.sin_port);
	}

	*sock = hbsock;
	*port = baseport;

	return e_qrnoerror;
}

qr2_error_t qr2_initA(/*[out]*/qr2_t *qrec, const char *ip, int baseport, const char *gamename, const char *secret_key,
					 int ispublic, int natnegotiate,
					 qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback,
					 qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback,
					 qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback,
					 void *userdata)
{
	SOCKET hbsock;
	qr2_error_t ret;

	ret = qr2_create_socket(&hbsock, ip, &baseport);
	if(ret != e_qrnoerror)
	{
		SocketShutDown();
		return ret;
	}

	ret = qr2_init_socketA(qrec, hbsock, baseport, gamename, secret_key, ispublic, natnegotiate, server_key_callback, player_key_callback, team_key_callback, key_list_callback, playerteam_count_callback, adderror_callback,  userdata);
	if (qrec == NULL)
		qrec = &current_rec;
	(*qrec)->read_socket = 1;
	
	return ret;
}

qr2_error_t qr2_initW(/*[out]*/qr2_t *qrec, const unsigned short *ip, int baseport, const unsigned short *gamename, const unsigned short *secret_key,
					 int ispublic, int natnegotiate,
					 qr2_serverkeycallback_t server_key_callback,
					 qr2_playerteamkeycallback_t player_key_callback,
					 qr2_playerteamkeycallback_t team_key_callback,
					 qr2_keylistcallback_t key_list_callback,
					 qr2_countcallback_t playerteam_count_callback,
					 qr2_adderrorcallback_t adderror_callback,
					 void *userdata)
{
	char ip_A[255];
	char gamename_A[255];
	char secretkey_A[255];

	if (ip != NULL) // NULL value is valid for IP
		UCS2ToAsciiString(ip, ip_A);
	UCS2ToAsciiString(gamename, gamename_A);
	UCS2ToAsciiString(secret_key, secretkey_A);

	return qr2_initA(qrec, (ip!=NULL)?ip_A:NULL, baseport, gamename_A, secretkey_A, ispublic, natnegotiate, server_key_callback, player_key_callback, team_key_callback, key_list_callback, playerteam_count_callback, adderror_callback,  userdata);
}

void qr2_register_natneg_callback(qr2_t qrec, qr2_natnegcallback_t nncallback)
{
	if (qrec == NULL)
		qrec = current_rec;
	qrec->nn_callback = nncallback;

}
void qr2_register_clientmessage_callback(qr2_t qrec, qr2_clientmessagecallback_t cmcallback)
{
	if (qrec == NULL)
		qrec = current_rec;
	qrec->cm_callback = cmcallback;
}
void qr2_register_publicaddress_callback(qr2_t qrec, qr2_publicaddresscallback_t pacallback)
{
	if (qrec == NULL)
		qrec = current_rec;
	qrec->pa_callback = pacallback;
}


/* qr2_think: Processes any waiting queries, and sends a
heartbeat if needed  */
void qr2_think(qr2_t qrec)
{
	if (qrec == NULL)
		qrec = current_rec;
	if (qrec->ispublic)
		qr2_check_send_heartbeat(qrec);
	qr2_check_queries(qrec);
}

/* qr2_check_queries: Processes any waiting queries */
void qr2_check_queries(qr2_t qrec)
{
	static char indata[INBUF_LEN]; //256 byte input buffer
	struct sockaddr_in saddr;
	int saddrlen = sizeof(struct sockaddr_in);
	int error;

	if (!qrec->read_socket)
		return; //not our job

	while(CanReceiveOnSocket(qrec->hbsock))
	{
		//else we have data
		#ifdef SN_SYSTEMS
		error = recvfrom(qrec->hbsock, indata, INBUF_LEN - 1, 0, (struct sockaddr *)&saddr, &saddrlen);
		#else
		error = (int)recvfrom(qrec->hbsock, indata, (unsigned int)INBUF_LEN - 1, 0, (struct sockaddr *)&saddr, &saddrlen);
		#endif
		if (error != SOCKET_ERROR)
		{
			indata[error] = '\0';
			qr2_parse_queryA(qrec, indata, error, (struct sockaddr *)&saddr);
		}
	}
}

/* check_send_heartbeat: Perform any scheduled outgoing
heartbeats */
void qr2_check_send_heartbeat(qr2_t qrec)
{
	gsi_time tc = current_time();

	if (INVALID_SOCKET == qrec->hbsock)
		return; //no sockets to work with!

	//check if we need to send a heartbet
	if (qrec->listed_state > 0 && tc - qrec->lastheartbeat > FIRST_HB_TIME) 
	{ //check to see if we haven't gotten a query yet
		if (qrec->listed_state >= MAX_FIRST_COUNT)
		{
			qrec->listed_state = 0; //we failed to get a challenge! let them know
#ifndef GSI_UNICODE
			qrec->adderror_callback(e_qrnochallengeerror, "No challenge value was received from the master server.", qrec->udata);
#else
			qrec->adderror_callback(e_qrnochallengeerror, L"No challenge value was received from the master server.", qrec->udata);
#endif
			return;
		} else
		{
			send_heartbeat(qrec, 3);
			qrec->listed_state++;
		}
	}
	else if (qrec->userstatechangerequested && (tc - qrec->lastheartbeat > MIN_STATECHANGED_HB_TIME))
		send_heartbeat(qrec,1);  // Send out pending statechange request
	else if (tc - qrec->lastheartbeat > HB_TIME || qrec->lastheartbeat == 0 || tc < qrec->lastheartbeat)
		send_heartbeat(qrec,0);  // Send out a normal hearbeat
	
	if (tc - qrec->lastka > KA_TIME) //send a keep alive (to keep NAT port mappings the same if possible)
		send_keepalive(qrec);
}

/* qr2_send_statechanged: Sends a statechanged heartbeat, call when
your gamemode changes */
void qr2_send_statechanged(qr2_t qrec)
{
	if (qrec == NULL)
		qrec = current_rec;
	if (!qrec->ispublic)
		return;
	if (current_time() - qrec->lastheartbeat < MIN_STATECHANGED_HB_TIME)
	{
		// Queue up the statechange and send later
		qrec->userstatechangerequested = 1;
		return;  // don't allow the server to spam statechanges
	}
	send_heartbeat(qrec, 1);
	qrec->userstatechangerequested = 0; // clear the flag in case a queued statechange was still pending
}


/* qr2_shutdown: Cleans up the sockets and shuts down */
void qr2_shutdown(qr2_t qrec)
{
	if (qrec == NULL)
		qrec = current_rec;
	if (qrec->ispublic)
		send_heartbeat(qrec, 2);
	if (INVALID_SOCKET != qrec->hbsock && qrec->read_socket) //if we own the socket
	{
		closesocket(qrec->hbsock);
	}
	qrec->hbsock = INVALID_SOCKET;
	qrec->lastheartbeat = 0;
	if(qrec->read_socket) //if we own the socket
	{
		SocketShutDown();
	}
	if (qrec != &static_qr2_rec) //need to gsifree it, it was dynamically allocated
	{
		gsifree(qrec);
	}

	// BD: Removed - Peer SDK repeatedly calls qr2_shutdown, but
	//               keys should only be deallocated once.

	// Developers should call this manually
	// qr2_internal_key_list_free();
}


void qr2_keybuffer_add(qr2_keybuffer_t keybuffer, int keyid)
{
	if (keybuffer->numkeys >= MAX_REGISTERED_KEYS)
		return;
	if (keyid < 1 || keyid > MAX_REGISTERED_KEYS)
		return;
	keybuffer->keys[keybuffer->numkeys++] = (uchar)keyid;
}

void qr2_buffer_add_int(qr2_buffer_t outbuf, int value)
{
	char temp[20];
	sprintf(temp, "%d", value);
	qr2_buffer_addA(outbuf, temp);

}

void qr2_buffer_addA(qr2_buffer_t outbuf, const char *value)
{
	int copylen;
	copylen = (int)strlen(value) + 1;
	if (copylen > AVAILABLE_BUFFER_LEN(outbuf))
		copylen = AVAILABLE_BUFFER_LEN(outbuf); //max length we can fit in the buffer
	if (copylen == 0)
		return; //no space
	memcpy(outbuf->buffer + outbuf->len, value, (unsigned int)copylen);
	outbuf->len += copylen;
	outbuf->buffer[outbuf->len - 1] = 0; //make sure it's null terminated
}

void qr2_buffer_addW(qr2_buffer_t outbuf, const unsigned short *value)
{
	char value_A[4096];
	UCS2ToUTF8String(value, value_A);
	qr2_buffer_addA(outbuf, value_A);
}


static void enum_local_ips()
{
	struct hostent *phost;
	phost = getlocalhost();
	if (phost == NULL)
		return;
	for (num_local_ips = 0 ; num_local_ips < MAX_LOCAL_IP ; num_local_ips++)
	{
		if (phost->h_addr_list[num_local_ips] == 0)
			break;
		memcpy(&local_ip_list[num_local_ips], phost->h_addr_list[num_local_ips], sizeof(struct in_addr));
	}
}

/****************************************************************************/


/* Return a sockaddrin for the given host (numeric or DNS) and port)
Returns the hostent in savehent if it is not NULL */
static int get_sockaddrin(const char *host, int port, struct sockaddr_in *saddr, struct hostent **savehent)
{
	struct hostent *hent = NULL;

	saddr->sin_family = AF_INET;
	saddr->sin_port = htons((unsigned short)port);
	if (host == NULL)
		saddr->sin_addr.s_addr = INADDR_ANY;
	else
		saddr->sin_addr.s_addr = inet_addr(host);
	
	if (saddr->sin_addr.s_addr == INADDR_NONE && strcmp(host,"255.255.255.255") != 0)
	{
		hent = gethostbyname(host);
		if (!hent)
			return 0;
		saddr->sin_addr.s_addr = *(unsigned int *)hent->h_addr_list[0];
	}
	if (savehent != NULL)
		*savehent = hent;
	return 1;

} 



/*****************************************************************************/
/* Various encryption / encoding routines */

static void swap_byte ( uchar *a, uchar *b )
{
	uchar swapByte; 
	
	swapByte = *a; 
	*a = *b;      
	*b = swapByte;
}

static uchar encode_ct ( uchar c )
{
	if (c <  26) return (uchar)('A'+c);
	if (c <  52) return (uchar)('a'+c-26);
	if (c <  62) return (uchar)('0'+c-52);
	if (c == 62) return (uchar)('+');
	if (c == 63) return (uchar)('/');
	
	return 0;
}

static void gs_encode ( uchar *ins, int size, uchar *result )
{
	int    i,pos;
	uchar  trip[3];
	uchar  kwart[4];
	
	i=0;
	while (i < size)
	{
		for (pos=0 ; pos <= 2 ; pos++, i++)
			if (i < size) trip[pos] = *ins++;
			else trip[pos] = '\0';
			kwart[0] = (unsigned char)(  (trip[0])       >> 2);
			kwart[1] = (unsigned char)((((trip[0]) &  3) << 4) + ((trip[1]) >> 4));
			kwart[2] = (unsigned char)((((trip[1]) & 15) << 2) + ((trip[2]) >> 6));
			kwart[3] = (unsigned char)(  (trip[2]) & 63);
			for (pos=0; pos <= 3; pos++) *result++ = encode_ct(kwart[pos]);
	}
	*result='\0';
}

static void gs_encrypt ( uchar *key, int key_len, uchar *buffer_ptr, int buffer_len )
{ 
	short counter;     
	uchar x, y, xorIndex;
	uchar state[256];       
	
	for ( counter = 0; counter < 256; counter++) state[counter] = (uchar) counter;
	
	x = 0; y = 0;
	for ( counter = 0; counter < 256; counter++)
	{
		y = (uchar)((key[x] + state[counter] + y) % 256);
		x = (uchar)((x + 1) % key_len);
		swap_byte ( &state[counter], &state[y] );
	}
	
	x = 0; y = 0;
	for ( counter = 0; counter < buffer_len; counter ++)
	{
		x = (uchar)((x + buffer_ptr[counter] + 1) % 256);
		y = (uchar)((state[x] + y) % 256);
		swap_byte ( &state[x], &state[y] );
		xorIndex = (uchar)((state[x] + state[y]) % 256);
		buffer_ptr[counter] ^= state[xorIndex];
	}
}
/*****************************************************************************/


static void qr_add_packet_header(qr2_buffer_t buf, char ptype, char *reqkey)
{
	buf->buffer[0] = ptype;
	memcpy(buf->buffer + 1, reqkey, REQUEST_KEY_LEN);
	buf->len = REQUEST_KEY_LEN  + 1;
}

#define MAX_CHALLENGE 64
static void compute_challenge_response(qr2_t qrec, qr2_buffer_t buf, char *challenge, int challengelen)
{
	char encrypted_val[MAX_CHALLENGE + 1]; //don't need to null terminate
	
	if(challengelen < 1)
		return; // invalid, need room for the NUL
	if (challengelen > (MAX_CHALLENGE + 1))
		return; //invalid
	if (challenge[challengelen - 1] != 0)
		return; //invalid - must be NTS
	
	strcpy(encrypted_val, challenge);
	gs_encrypt((uchar *)qrec->secret_key, (int)strlen(qrec->secret_key), (uchar *)encrypted_val, challengelen - 1);
	gs_encode((uchar *)encrypted_val,challengelen - 1, (uchar *)(buf->buffer + buf->len));
	buf->len += (int)strlen(buf->buffer + buf->len) + 1;
	
}

static void handle_public_address(qr2_t qrec, char * buffer)
{
	unsigned int ip;
	unsigned int portTemp;
	unsigned short port;

	// get the public ip and port as the master server sees it
	sscanf(buffer, "%08X%04X", &ip, &portTemp);
	port = (unsigned short)portTemp;
	ip = htonl(ip);

	// sanity check
	if((ip == 0) || (port == 0))
		return;

	// has anything changed?
	if((qrec->publicip != ip) || (qrec->publicport != port))
	{
		qrec->publicip = ip;
		qrec->publicport = port;
		qrec->pa_callback(ip, port, qrec->udata);
	}
}

static void qr_build_partial_query_reply(qr2_t qrec, qr2_buffer_t buf, qr2_key_type keytype, int keycount, uchar *keys)
{
	struct qr2_keybuffer_s kb;
	int playerteamcount;
	unsigned short cttemp;
	int i;
	int pindex;
	const char *k;
	int len;

	kb.numkeys = 0;
	if (keycount == 0)
		return; //no keys wanted
	
	if (keytype == key_player || keytype == key_team) //need to add the player/team counts
	{
		if (AVAILABLE_BUFFER_LEN(buf) < sizeof(cttemp))
			return; //no more space
		playerteamcount = qrec->playerteam_count_callback(keytype, qrec->udata);
		cttemp = htons((unsigned short)playerteamcount);
		memcpy(buf->buffer + buf->len, &cttemp, sizeof(cttemp));
		buf->len += sizeof(cttemp);
	} else
		playerteamcount = 1;

	if (keycount == 0xFF) //need to get the list of keys
	{
		qrec->key_list_callback(keytype, &kb, qrec->udata);
		//add all the keys
		for (i = 0 ; i < kb.numkeys ; i++)
		{
			k = qr2_registered_key_list[kb.keys[i]];
			if (k == NULL)
				k = "unknown";
			qr2_buffer_addA(buf, k);
			if (keytype == key_server) //add the server values
			{
				len = buf->len;
				qrec->server_key_callback(kb.keys[i], buf, qrec->udata);
				if(len == buf->len)
					qr2_buffer_addA(buf, "");
			}
		}
		//add an extra null
		if (AVAILABLE_BUFFER_LEN(buf) < 1)
			return; //no space
		buf->buffer[buf->len++] = 0;
		keycount = kb.numkeys;
		keys = kb.keys;
		if (keytype == key_server)
			return;	//already added the keys
	}
	for (pindex = 0 ; pindex < playerteamcount ; pindex++)
	{
		for (i = 0 ; i < keycount ; i++)
		{
			len = buf->len;
			if (keytype == key_server) //add the server keys
				qrec->server_key_callback(keys[i], buf, qrec->udata);
			else if (keytype == key_player)
				qrec->player_key_callback(keys[i], pindex, buf, qrec->udata);
			else if (keytype == key_team)
				qrec->team_key_callback(keys[i], pindex, buf, qrec->udata);
			if(len == buf->len)
				qr2_buffer_addA(buf, "");
		}
	}		
}

static void qr_build_query_reply(qr2_t qrec, qr2_buffer_t buf, int serverkeycount, uchar *serverkeys, int playerkeycount, uchar *playerkeys, int teamkeycount, uchar *teamkeys)
{
	qr_build_partial_query_reply(qrec, buf, key_server, serverkeycount, serverkeys);
	qr_build_partial_query_reply(qrec, buf, key_player, playerkeycount, playerkeys);
	qr_build_partial_query_reply(qrec, buf, key_team, teamkeycount, teamkeys);
}

static void qr_process_query(qr2_t qrec, qr2_buffer_t buf, uchar *qdata, int len)
{
	uchar serverkeycount;
	uchar playerkeycount;
	uchar teamkeycount;
	
	uchar *serverkeys = NULL;
	uchar *playerkeys = NULL;
	uchar *teamkeys = NULL;
	if (len < 3)
		return; //invalid
	serverkeycount = qdata[0];
	qdata++;
	len--;
	if (serverkeycount != 0 && serverkeycount != 0xFF)
	{
		serverkeys = qdata;
		qdata += serverkeycount;
		len -= serverkeycount;
	}
	if (len < 2)
		return; //invalid
	playerkeycount = qdata[0];
	qdata++;
	len--;
	if (playerkeycount != 0 && playerkeycount != 0xFF)
	{
		playerkeys = qdata;
		qdata += playerkeycount;
		len -= playerkeycount;
	}
	if (len < 1)
		return; //invalid
	teamkeycount = qdata[0];
	qdata++;
	len--;
	if (teamkeycount != 0 && teamkeycount != 0xFF)
	{
		teamkeys = qdata;
		qdata += teamkeycount;
		len -= teamkeycount;
	}
	if (len < 0)
		return; //invalid
	qr_build_query_reply(qrec, buf, serverkeycount, serverkeys, playerkeycount, playerkeys, teamkeycount, teamkeys);

}


static void qr_build_partial_old_query_reply(qr2_t qrec, qr2_buffer_t buf, qr2_key_type keytype)
{
	char tempkeyname[128];
	struct qr2_keybuffer_s kb;
	int playerteamcount;
	int i;
	int pindex;
	const char *k;
	int len;

	kb.numkeys = 0;

	if (keytype == key_player || keytype == key_team) //need to add the player/team counts
	{
		playerteamcount = qrec->playerteam_count_callback(keytype, qrec->udata);
	} else
		playerteamcount = 1;

	qrec->key_list_callback(keytype, &kb, qrec->udata);
	//add all the keys
	for (i = 0 ; i < kb.numkeys ; i++)
	{
		k = qr2_registered_key_list[kb.keys[i]];
		if (k == NULL)
			k = "unknown";
		if (keytype == key_server) //add the server values
		{
			qr2_buffer_addA(buf, k);
			buf->buffer[buf->len - 1] = '\\';
			len = buf->len;
			qrec->server_key_callback(kb.keys[i], buf, qrec->udata);
			if(len == buf->len)
				qr2_buffer_addA(buf, "");
			buf->buffer[buf->len - 1] = '\\';
		} else //need to look it up for each player/team
		{

			for (pindex = 0 ; pindex < playerteamcount ; pindex++)
			{
				sprintf(tempkeyname, "%s%d", k, pindex);
				qr2_buffer_addA(buf, tempkeyname);
				buf->buffer[buf->len - 1] = '\\';
				len = buf->len;
				if (keytype == key_player)
					qrec->player_key_callback(kb.keys[i], pindex, buf, qrec->udata);
				else if (keytype == key_team)
					qrec->team_key_callback(kb.keys[i], pindex, buf, qrec->udata);
				if(len == buf->len)
					qr2_buffer_addA(buf, "");
				buf->buffer[buf->len - 1] = '\\';
			}		
		}
	}
	
	
}

//we just build a status reply, since we don't have equivalent callbacks
static void qr_process_old_query(qr2_t qrec, qr2_buffer_t buf)
{
	buf->len = 1;
	buf->buffer[0] = '\\';

	qr_build_partial_old_query_reply(qrec, buf, key_server);
	qr_build_partial_old_query_reply(qrec, buf, key_player);
	qr_build_partial_old_query_reply(qrec, buf, key_team);
	qr2_buffer_addA(buf, "final\\\\queryid\\1.1");
	buf->len--; //remove the final null;
}

static void qr_process_client_message(qr2_t qrec, char *buf, int len)
{
	unsigned char natNegBytes[NATNEG_MAGIC_LEN] = {NN_MAGIC_0,NN_MAGIC_1,NN_MAGIC_2,NN_MAGIC_3,NN_MAGIC_4,NN_MAGIC_5};
	int i;
	int isnatneg = 1;
	//see if it's a natneg request..
	if (len >= NATNEG_MAGIC_LEN + 4)
	{
		for (i = 0 ; i < NATNEG_MAGIC_LEN ; i++)
			if ((unsigned char)buf[i] != natNegBytes[i])
			{
				isnatneg = 0;
				break;
			}
	} else
		isnatneg = 0;
	if (isnatneg)
	{
		int cookie;
		memcpy(&cookie, buf + NATNEG_MAGIC_LEN, 4);
		if (qrec->nn_callback)
			qrec->nn_callback((int)ntohl((unsigned int)cookie), qrec->udata); 
	} else
		if (qrec->cm_callback)
		{
#ifndef GSI_UNICODE
			qrec->cm_callback(buf, len, qrec->udata);
#else
			unsigned short* buf_W;
			buf_W = UTF8ToUCS2StringAlloc(buf);
			qrec->cm_callback(buf_W, wcslen(buf_W), qrec->udata);
#endif
		}
}

static int qr_got_recent_message(qr2_t qrec, int msgkey)
{
	int i;
	for (i = 0 ; i < RECENT_CLIENT_MESSAGES_TO_TRACK ; i++)
	{
		if (qrec->client_message_keys[i] == msgkey)
			return 1;
	}
	//else, add it to the list
	qrec->cur_message_key = (qrec->cur_message_key + 1) % RECENT_CLIENT_MESSAGES_TO_TRACK;
	qrec->client_message_keys[qrec->cur_message_key] = msgkey;
	return 0;
}

/* parse_query: parse an incoming query and reply to each query */
void qr2_parse_queryA(qr2_t qrec, char *query, int len, struct sockaddr *sender)
{
	struct qr2_buffer_s buf;
	char ptype;
	char *reqkey;
	char *pos;
	int i;
	
	buf.len = 0;

/* DEBUG CODE
	{
		char* buf = inet_ntoa(((struct sockaddr_in*)sender)->sin_addr);
		unsigned port = ntohs(((struct sockaddr_in*)sender)->sin_port);
		printf("Received query from %s:%d\r\n", buf, port);
	}
*/

	if (qrec == NULL)
		qrec = current_rec;
	if (query[0] == 0x3B) /* a cdkey query */
	{
		if (qrec->cdkeyprocess != NULL)
			qrec->cdkeyprocess(query, len, sender);
		return;
	}
	if (query[0] == '\\') //it's a QR1-style query
	{
		qr_process_old_query(qrec, &buf);
#ifdef SN_SYSTEMS
		sendto(qrec->hbsock, buf.buffer, buf.len, 0, sender, sizeof(struct sockaddr_in));
#else 
		sendto(qrec->hbsock, buf.buffer, (unsigned int)buf.len, 0, sender, sizeof(struct sockaddr_in));
#endif
		return;
	}

	if (len < 7)
		return; //too small to be valid
	//check the magic...
	if ((uchar)query[0] != QR_MAGIC_1 || (uchar)query[1] != QR_MAGIC_2)
		return;

//#define SIMULATE_HARD_FIREWALL
#if defined(SIMULATE_HARD_FIREWALL) && defined(WIN32)
#pragma message(" ")
#pragma message("QR2 BROKEN - SIMULATE_HARD_FIREWALL DEFINED")
#pragma message(" ")
	// ignore all QR2 packets on this port
	// generates "no challenge" error
	return;
#endif

//#define SIMULATE_FIREWALL
#if defined(SIMULATE_FIREWALL) && defined(WIN32)
#pragma message(" ")
#pragma message("QR2 WARNING - SIMULATE_FIREWALL DEFINED")
#pragma message(" ")
	// ignore messages that are not from the gamespy master
	{
		unsigned int aMasterAddr = 0x220826cf;
		if (((SOCKADDR_IN*)sender)->sin_addr.s_addr != aMasterAddr)
			return;
	}
#endif
	if (qrec->listed_state > 0)
		qrec->listed_state = 0;
	

	ptype = query[2];
	reqkey = &query[3];
	pos = query + 7;
	len -= 7;
	
	
	qr_add_packet_header(&buf, ptype, reqkey);
	switch (ptype)
	{
	case PACKET_QUERY:
		qr_process_query(qrec, &buf, (uchar *)pos, len);

		break;
	case PACKET_CHALLENGE:
		//calculate the challenge
		if(qrec->pa_callback && (len >= (PUBLIC_ADDR_LEN + 1)))
			handle_public_address(qrec, pos + len - (PUBLIC_ADDR_LEN + 1));
		compute_challenge_response(qrec, &buf, pos, len);
		break;
	case PACKET_ECHO:
		//now add the echo data
		if (len > 32)
			len = 32; //max 32 bytes
		buf.buffer[0] = PACKET_ECHO_RESPONSE;
		memcpy(buf.buffer + buf.len, pos, (unsigned int)len);
		buf.len += len;
		break;

	case PACKET_ADDERROR:
		if (qrec->listed_state == -1)
			return; //we already got an error message
		//verify the instance code
		for (i = 0 ; i < REQUEST_KEY_LEN ; i++)
		{
			if (reqkey[i] != qrec->instance_key[i])
				return; //not a valid instance key
		}
		if (len < 2)
			return; //not a valid message
		qrec->listed_state = -1;
#ifndef GSI_UNICODE
		qrec->adderror_callback((qr2_error_t)*pos, pos + 1, qrec->udata);
#else
		{
			unsigned short* message_W;
			message_W = UTF8ToUCS2StringAlloc(pos+1);
			qrec->adderror_callback((qr2_error_t)*pos, message_W, qrec->udata);
			gsifree(message_W);
		}
#endif
		return; //we don't need to send anything back for this type of message
	case PACKET_CLIENT_MESSAGE:
		//verify the instance code
		for (i = 0 ; i < REQUEST_KEY_LEN ; i++)
		{
			if (reqkey[i] != qrec->instance_key[i])
				return; //not a valid instance key
		}
		if (len < 4) //no message key?
			return;
		buf.buffer[0] = PACKET_CLIENT_MESSAGE_ACK;
		//add the msg key
		memcpy(buf.buffer + buf.len, pos, 4);
		buf.len += 4;
		//see if we've recently gotten this same message, to help avoid dupes
		memcpy(&i, pos, 4);
		if (!qr_got_recent_message(qrec, i))
			qr_process_client_message(qrec, pos + 4, len - 4);
		//send an ack response
		break;
	case PACKET_KEEPALIVE:
		return; //if we get a keep alive, ignore it and return (just used to tell us the server knows about us)
	default:
		return; //not valid type

	}
	//send the reply
#ifdef SN_SYSTEMS
	sendto(qrec->hbsock, buf.buffer, buf.len, 0, sender, sizeof(struct sockaddr_in));
#else
	sendto(qrec->hbsock, buf.buffer, (unsigned int)buf.len, 0, sender, sizeof(struct sockaddr_in));
#endif
}

void qr2_parse_queryW(qr2_t qrec, unsigned short *query, int len, struct sockaddr *sender)
{
	char query_A[4096];
	UCS2ToUTF8String(query, query_A);
	qr2_parse_queryA(qrec, query_A, len, sender);
}


/* send_keepalive: Send a keepalive packet to the hbmaster3 */
static void send_keepalive(qr2_t qrec)
{
	struct qr2_buffer_s buf;
	buf.len = 0;
	qr_add_packet_header(&buf, PACKET_KEEPALIVE, qrec->instance_key);
#ifdef SN_SYSTEMS
	sendto(qrec->hbsock, buf.buffer, buf.len, 0, (struct sockaddr *)&(qrec->hbaddr), sizeof(struct sockaddr_in));
#else
	sendto(qrec->hbsock, buf.buffer, (unsigned int)buf.len, 0, (struct sockaddr *)&(qrec->hbaddr), sizeof(struct sockaddr_in));
#endif
	//set the ka time to now
	qrec->lastka = current_time();


}

/* send_heartbeat: Sends a heartbeat to the gamemaster,
adds \statechanged\ if statechanged != 0 */
static void send_heartbeat(qr2_t qrec, int statechanged)
{
	struct qr2_buffer_s buf;
	int ret;
	int i;
	char ipkey[20];

	buf.len = 0;
	qr_add_packet_header(&buf, PACKET_HEARTBEAT, qrec->instance_key);
	//now we add our special keys
	for (i = 0 ; i < num_local_ips ; i++)
	{
		sprintf(ipkey, "localip%d", i);
		qr2_buffer_addA(&buf, ipkey);
		qr2_buffer_addA(&buf, inet_ntoa(local_ip_list[i]));
	}
	qr2_buffer_addA(&buf, "localport");
	qr2_buffer_add_int(&buf, qrec->qport);
	qr2_buffer_addA(&buf, "natneg");
	qr2_buffer_addA(&buf, qrec->nat_negotiate ? "1" : "0");
	if(statechanged)
	{
		qr2_buffer_addA(&buf, "statechanged");
		qr2_buffer_add_int(&buf, statechanged);
	}
	qr2_buffer_addA(&buf, "gamename");
	qr2_buffer_addA(&buf, qrec->gamename);
	if(qrec->pa_callback)
	{
		qr2_buffer_addA(&buf, "publicip");
		qr2_buffer_add_int(&buf, (int)qrec->publicip);
		qr2_buffer_addA(&buf, "publicport");
		qr2_buffer_add_int(&buf, qrec->publicport);
	}
	
	//add the rest of our keys
	if (statechanged != 2) //don't need if we are exiting
		qr_build_query_reply(qrec, &buf, 0xFF, NULL, 0xFF, NULL, 0xFF, NULL);
	else
	{
		// PANTS - 2002.6.28
		// add an extra NUL to end the server keys
		if (AVAILABLE_BUFFER_LEN(&buf) >= 1)
			buf.buffer[buf.len++] = 0;
	}

#ifdef SN_SYSTEMS
	ret = sendto(qrec->hbsock, buf.buffer, buf.len, 0, (struct sockaddr *)&(qrec->hbaddr), sizeof(struct sockaddr_in));
#else
	ret = (int)sendto(qrec->hbsock, buf.buffer, (unsigned int)buf.len, 0, (struct sockaddr *)&(qrec->hbaddr), sizeof(struct sockaddr_in));
#endif
	//set the ka time and hb time to now
	qrec->lastka = qrec->lastheartbeat = current_time();

	// clear the pending heartbeat request flag
	if (statechanged != 0)
		qrec->userstatechangerequested = 0;

}

#ifdef __cplusplus
}
#endif
