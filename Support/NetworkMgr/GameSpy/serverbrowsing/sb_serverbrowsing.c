#include "sb_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include "../stringutil.h"
#include "sb_ascii.h"

//Future Versions:
//ICMP Ping support (icmp engine)


//internal callback proxy for server list
static void ListCallback(SBServerList *serverlist, SBListCallbackReason reason, SBServer server, void *instance)
{
	ServerBrowser sb = (ServerBrowser)instance;
	switch (reason)
	{
	case slc_serveradded:
		sb->BrowserCallback(sb, sbc_serveradded, server, sb->instance);
		if ((server->state & (STATE_BASICKEYS|STATE_FULLKEYS)) == 0) //we need to do an update
		{
			if (!sb->dontUpdate) //if this flag is set, we don't want to trigger updates
			{
				int qtype;
				if (sb->list.state == sl_lanbrowse || sb->engine.numserverkeys == 0)
					qtype = QTYPE_FULL;
				else
					qtype = QTYPE_BASIC;
				SBQueryEngineUpdateServer(&sb->engine, server, 0, qtype);
			}
		}
		break;
	case slc_serverupdated:
		if ((server->state & (STATE_BASICKEYS|STATE_FULLKEYS)) == 0) //if it was updated, but with no data, then the update failed!
			sb->BrowserCallback(sb, sbc_serverupdatefailed, server, sb->instance);
		else
			sb->BrowserCallback(sb, sbc_serverupdated, server, sb->instance);
		break;
	case slc_serverdeleted:
		if ((server->state & (STATE_PENDINGBASICQUERY|STATE_PENDINGFULLQUERY)) != 0) 
			SBQueryEngineRemoveServerFromFIFOs(&sb->engine, server);
		sb->BrowserCallback(sb, sbc_serverdeleted, server, sb->instance);
		break;
	case slc_initiallistcomplete:
		if (sb->disconnectFlag)
			SBServerListDisconnect(serverlist);
		// If there aren't any servers to query, call the completed callback
		if (ArrayLength(serverlist->servers)==0 || sb->engine.querylist.count==0)
			sb->BrowserCallback(sb, sbc_updatecomplete, NULL, sb->instance);
		break;
	case slc_disconnected:
		break; 
	case slc_queryerror:
		sb->BrowserCallback(sb, sbc_queryerror, NULL, sb->instance);
		break;
	case slc_publicipdetermined:
		SBQueryEngineSetPublicIP(&sb->engine, sb->list.mypublicip);
		break;
	}
	if (server != NULL && server->publicip == sb->triggerIP && server->publicport == sb->triggerPort)
		sb->triggerIP = 0; //clear the trigger
}

//internal callback proxy for query engine
static void EngineCallback(SBQueryEnginePtr engine, SBQueryEngineCallbackReason reason, SBServer server, void *instance)
{
	ServerBrowser sb = (ServerBrowser)instance;
	switch (reason)
	{
	case qe_updatefailed:
		sb->BrowserCallback(sb, sbc_serverupdatefailed, server, sb->instance);
		break;
	case qe_updatesuccess:
		sb->BrowserCallback(sb, sbc_serverupdated, server, sb->instance);
		break;
	case qe_engineidle:
		sb->BrowserCallback(sb, sbc_updatecomplete, server, sb->instance);
		break;
	}
	if (server != NULL && server->publicip == sb->triggerIP && server->publicport == sb->triggerPort)
		sb->triggerIP = 0; //clear the trigger
		
	GSI_UNUSED(engine);
}



ServerBrowser ServerBrowserNewA(const char *queryForGamename, const char *queryFromGamename, const char *queryFromKey, int queryFromVersion, int maxConcUpdates, int queryVersion, ServerBrowserCallback callback, void *instance)
{
	ServerBrowser sb;
	if(__GSIACResult != GSIACAvailable)
		return NULL;
	sb = (ServerBrowser)gsimalloc(sizeof(struct _ServerBrowser));
	if (sb == NULL)
		return NULL;
	sb->BrowserCallback = callback;
	sb->instance = instance;
	sb->dontUpdate = SBFalse;;
	SBServerListInit(&sb->list, queryForGamename, queryFromGamename, queryFromKey, queryFromVersion, ListCallback, sb);
	SBQueryEngineInit(&sb->engine, maxConcUpdates, queryVersion, EngineCallback, sb);
	return sb;
}
#ifdef GSI_UNICODE
ServerBrowser ServerBrowserNewW(const unsigned short *queryForGamename, const unsigned short *queryFromGamename, const unsigned short *queryFromKey, int queryFromVersion, int maxConcUpdates, int queryVersion, ServerBrowserCallback callback, void *instance)
{
	char forGameName_A[255];
	char fromGameName_A[255];
	char fromGameKey_A[255];

	assert(queryForGamename != NULL);
	assert(queryFromGamename != NULL);
	assert(queryFromKey != NULL);

	UCS2ToAsciiString(queryForGamename, forGameName_A);
	UCS2ToAsciiString(queryFromGamename, fromGameName_A);
	UCS2ToAsciiString(queryFromKey, fromGameKey_A);
	return ServerBrowserNewA(forGameName_A, fromGameName_A, fromGameKey_A, queryFromVersion, maxConcUpdates, queryVersion, callback, instance);
}
#endif

void ServerBrowserFree(ServerBrowser sb)
{
	SBServerListCleanup(&sb->list);
	SBEngineCleanup(&sb->engine);
	gsifree(sb);
}

//internal version that allows passing of additional options
SBError ServerBrowserBeginUpdate2(ServerBrowser sb, SBBool async, SBBool disconnectOnComplete, const unsigned char *basicFields, int numBasicFields, const char *serverFilter, int updateOptions, int maxServers)
{
	char keyList[MAX_FIELD_LIST_LEN] = "";	
	int listLen = 0;
	int i;
	int keylen;
	SBError err;

	sb->disconnectFlag = disconnectOnComplete;
	//clear this out in case it was already set
	sb->engine.numserverkeys = 0;
	//build the key list...
	for (i = 0 ; i < numBasicFields ; i++)
	{
		keylen = (int)strlen(qr2_registered_key_list[basicFields[i]]);
		if (listLen + keylen + 1 >= MAX_FIELD_LIST_LEN)
			break; //can't add this field, too long
		listLen += sprintf(keyList + listLen, "\\%s", qr2_registered_key_list[basicFields[i]]);
		//add to the engine query list
		SBQueryEngineAddQueryKey(&sb->engine, basicFields[i]);
	}
	err = SBServerListConnectAndQuery(&sb->list, keyList, serverFilter, updateOptions, maxServers);
	if (err != sbe_noerror)
		return err;
	
	if (!async) //loop while we are still getting the main list and the engine is updating...
	{
		while ((sb->list.state == sl_mainlist) || ((sb->engine.querylist.count > 0) && (err == sbe_noerror)))
		{
			msleep(10);
			err = ServerBrowserThink(sb);
		}
	}
	return err;
}


SBError ServerBrowserUpdateA(ServerBrowser sb, SBBool async, SBBool disconnectOnComplete, const unsigned char *basicFields, int numBasicFields, const char *serverFilter)
{
	return ServerBrowserBeginUpdate2(sb, async, disconnectOnComplete, basicFields, numBasicFields, serverFilter, 0, 0);
}
#ifdef GSI_UNICODE
SBError ServerBrowserUpdateW(ServerBrowser sb, SBBool async, SBBool disconnectOnComplete, const unsigned char *basicFields, int numBasicFields, const unsigned short *serverFilter)
{
	char serverFilter_A[1024];
	if (serverFilter != NULL)
		UCS2ToUTF8String(serverFilter, serverFilter_A);
	return ServerBrowserUpdateA(sb, async, disconnectOnComplete, basicFields, numBasicFields, (serverFilter != NULL) ? serverFilter_A:NULL);
}
#endif

SBError ServerBrowserLimitUpdateA(ServerBrowser sb, SBBool async, SBBool disconnectOnComplete, const unsigned char *basicFields, int numBasicFields, const char *serverFilter, int maxServers)
{
	return ServerBrowserBeginUpdate2(sb, async, disconnectOnComplete, basicFields, numBasicFields, serverFilter, LIMIT_RESULT_COUNT, maxServers);
}
#ifdef GSI_UNICODE
SBError ServerBrowserLimitUpdateW(ServerBrowser sb, SBBool async, SBBool disconnectOnComplete, const unsigned char *basicFields, int numBasicFields, const unsigned short *serverFilter, int maxServers)
{
	char serverFilter_A[1024];
	if (serverFilter != NULL)
		UCS2ToUTF8String(serverFilter, serverFilter_A);
	return ServerBrowserLimitUpdateA(sb, async, disconnectOnComplete, basicFields, numBasicFields, (serverFilter != NULL) ? serverFilter_A:NULL, maxServers);
}
#endif

SBError ServerBrowserLANUpdate(ServerBrowser sb, SBBool async, unsigned short startSearchPort, unsigned short endSearchPort)
{
	SBError err = sbe_noerror;
	ServerBrowserHalt(sb);
	SBServerListGetLANList(&sb->list, startSearchPort, endSearchPort, sb->engine.queryversion);
	if (!async)
	{
		while ((sb->list.state == sl_lanbrowse) || ((sb->engine.querylist.count > 0) && (err == sbe_noerror)))
		{
			msleep(10);
			err = ServerBrowserThink(sb);
		}
	}
	return err;
}

static SBError WaitForTriggerUpdate(ServerBrowser sb, SBBool viaMaster)
{
	SBError err = sbe_noerror;
	//wait until info is received for the triggerIP
	while (sb->triggerIP != 0 && err == sbe_noerror)
	{
		msleep(10);
		err = ServerBrowserThink(sb);
		if (viaMaster && sb->list.state == sb_disconnected) //we were supposed to get from master, and it's disconnected
			break;		
	}
	return err;

}


SBError ServerBrowserSendMessageToServerA(ServerBrowser sb, const char *ip, unsigned short port, const char *data, int len)
{
	return SBSendMessageToServer(&sb->list, inet_addr(ip), htons(port), data, len);
}
#ifdef GSI_UNICODE
SBError ServerBrowserSendMessageToServerW(ServerBrowser sb, const unsigned short *ip, unsigned short port, const char *data, int len)
{
	char ip_A[128];
	UCS2ToAsciiString(ip, ip_A);
	return ServerBrowserSendMessageToServerA(sb, ip_A, port, data, len);
}
#endif

SBError ServerBrowserSendNatNegotiateCookieToServerA(ServerBrowser sb, const char *ip, unsigned short port, int cookie)
{
	return SBSendNatNegotiateCookieToServer(&sb->list, inet_addr(ip), htons(port), cookie);
}
#ifdef GSI_UNICODE
SBError ServerBrowserSendNatNegotiateCookieToServerW(ServerBrowser sb, const unsigned short *ip, unsigned short port, int cookie)
{
	char ip_A[128];
	UCS2ToAsciiString(ip, ip_A);
	return ServerBrowserSendNatNegotiateCookieToServerA(sb, ip_A, port, cookie);
}
#endif

SBError ServerBrowserAuxUpdateIPA(ServerBrowser sb, const char *ip, unsigned short port, SBBool viaMaster, SBBool async, SBBool fullUpdate)
{
	
	SBError err = sbe_noerror;
	sb->dontUpdate = SBTrue;
	if (!viaMaster) //do an engine query
	{
		SBServer server;
		int i;
		//need to see if the server exists...
		i = SBServerListFindServerByIP(&sb->list, inet_addr(ip), htons(port));
		if (i == -1)
		{
			server = SBQueryEngineUpdateServerByIP(&sb->engine, ip, port, 1, (fullUpdate) ? QTYPE_FULL : QTYPE_BASIC);
			SBServerListAppendServer(&sb->list, server);
		}
		else
		{
			server = SBServerListNth(&sb->list, i);
			SBQueryEngineUpdateServer(&sb->engine, server, 1, (fullUpdate) ? QTYPE_FULL : QTYPE_BASIC);
		}
	} else //do a master update
	{
		err = SBGetServerRulesFromMaster(&sb->list, inet_addr(ip), htons(port));	
		//this will add the server itself..
	}
	if (!async && err == sbe_noerror)
	{
		sb->triggerIP = inet_addr(ip);
		sb->triggerPort = htons(port);
		err = WaitForTriggerUpdate(sb, viaMaster);
	}
	sb->dontUpdate = SBFalse;
	return err;
}
#ifdef GSI_UNICODE
SBError ServerBrowserAuxUpdateIPW(ServerBrowser sb, const unsigned short *ip, unsigned short port, SBBool viaMaster, SBBool async, SBBool fullUpdate)
{
	char ip_A[128];
	UCS2ToAsciiString(ip, ip_A);
	return ServerBrowserAuxUpdateIPA(sb, ip_A, port, viaMaster, async, fullUpdate);
}
#endif

SBError ServerBrowserAuxUpdateServer(ServerBrowser sb, SBServer server, SBBool async, SBBool fullUpdate)
{
	SBBool viaMaster;
	SBError err = sbe_noerror;

	sb->dontUpdate = SBTrue;

	if (server->flags & UNSOLICITED_UDP_FLAG) //do an engine query
	{
		//remove from the existing update lists if present
		SBQueryEngineRemoveServerFromFIFOs(&sb->engine, server);
		SBQueryEngineUpdateServer(&sb->engine, server, 1, (fullUpdate) ? QTYPE_FULL : QTYPE_BASIC);
		viaMaster = SBFalse;
	} else //do a master update
	{
		err = SBGetServerRulesFromMaster(&sb->list, server->publicip, server->publicport);	
		viaMaster = SBTrue;
	}
	if (!async && err == sbe_noerror)
	{
		sb->triggerIP = server->publicip;
		sb->triggerPort = server->publicport;
		err = WaitForTriggerUpdate(sb, viaMaster);
	}
	sb->dontUpdate = SBFalse;
	return err;
}

void ServerBrowserRemoveIPA(ServerBrowser sb, const char *ip, unsigned short port)
{
	int i = SBServerListFindServerByIP(&sb->list, inet_addr(ip), htons(port));
	if (i != -1)
		SBServerListRemoveAt(&sb->list, i);
}
#ifdef GSI_UNICODE
void ServerBrowserRemoveIPW(ServerBrowser sb, const unsigned short *ip, unsigned short port)
{
	char ip_A[128];
	UCS2ToAsciiString(ip, ip_A);
	ServerBrowserRemoveIPA(sb, ip_A, port);
}
#endif

void ServerBrowserRemoveServer(ServerBrowser sb, SBServer server)
{
	int i = SBServerListFindServer(&sb->list, server);
	if (i != -1)
		SBServerListRemoveAt(&sb->list, i);
}

SBError ServerBrowserThink(ServerBrowser sb)
{
	SBQueryEngineThink(&sb->engine);
	return SBListThink(&sb->list);
}

void ServerBrowserHalt(ServerBrowser sb)
{
	//stop the list
	SBServerListDisconnect(&sb->list);
	//stop the query engine...
	SBEngineHaltUpdates(&sb->engine);

}

void ServerBrowserClear(ServerBrowser sb)
{
	ServerBrowserHalt(sb);
	SBServerListClear(&sb->list);
}

const char *ServerBrowserErrorDescA(ServerBrowser sb, SBError error)
{
	switch (error)
	{
	case sbe_noerror:
		return "None";
		break;
	case sbe_socketerror:
		return "Socket creation error";
		break;
	case sbe_dnserror:
		return "DNS lookup error";
		break;
	case sbe_connecterror:
		return "Connection failed";
		break;
	case sbe_dataerror:
		return "Data stream error";
		break;
	case sbe_allocerror:
		return "Memory allocation error";
		break;
	case sbe_paramerror:
		return "Function parameter error";
		break;
	}
	return "";
	
	GSI_UNUSED(sb);
}
#ifdef GSI_UNICODE
const unsigned short *ServerBrowserErrorDescW(ServerBrowser sb, SBError error)
{
	switch (error)
	{
	case sbe_noerror:
		return L"None";
		break;
	case sbe_socketerror:
		return L"Socket creation error";
		break;
	case sbe_dnserror:
		return L"DNS lookup error";
		break;
	case sbe_connecterror:
		return L"Connection failed";
		break;
	case sbe_dataerror:
		return L"Data stream error";
		break;
	case sbe_allocerror:
		return L"Memory allocation error";
		break;
	case sbe_paramerror:
		return L"Function parameter error";
		break;
	}
	return L"";
	
	GSI_UNUSED(sb);
}
#endif

const char *ServerBrowserListQueryErrorA(ServerBrowser sb)
{
	return SBLastListErrorA(&sb->list);
}
#ifdef GSI_UNICODE
const unsigned short *ServerBrowserListQueryErrorW(ServerBrowser sb)
{
	return SBLastListErrorW(&sb->list);
}
#endif

SBState ServerBrowserState(ServerBrowser sb)
{
	if (sb->engine.querylist.count > 0)
		return sb_querying;
	if (sb->list.state == sl_mainlist || sb->list.state == sl_lanbrowse)
		return sb_listxfer;
	if (sb->list.state == sl_disconnected)
		return sb_disconnected;
	return sb_connected;
}

int ServerBrowserPendingQueryCount(ServerBrowser sb)
{
	return sb->engine.querylist.count + sb->engine.pendinglist.count;
}

SBServer ServerBrowserGetServer(ServerBrowser sb, int index)
{
	return SBServerListNth(&sb->list, index);
}

SBServer ServerBrowserGetServerByIPA(ServerBrowser sb, const char* ip, unsigned short port)
{
	int anIndex = -1;
	goa_uint32 anIP = 0;
	unsigned short aPortNBO = htons(port);

	anIP = inet_addr(ip);
	anIndex = SBServerListFindServerByIP(&sb->list, anIP, aPortNBO);
	if (anIndex != -1)
		return SBServerListNth(&sb->list, anIndex);
	return NULL;
}
#ifdef GSI_UNICODE
SBServer ServerBrowserGetServerByIPW(ServerBrowser sb, const unsigned short* ip, unsigned short port)
{
	char ip_A[20];

	if(ip == NULL || wcslen(ip) > 16)
		return NULL;

	UCS2ToAsciiString(ip, ip_A);
	return ServerBrowserGetServerByIPA(sb, ip_A, port);
}
#endif

int ServerBrowserCount(ServerBrowser sb)
{
	return SBServerListCount(&sb->list);
}


void ServerBrowserSortA(ServerBrowser sb, SBBool ascending, char *sortkey, SBCompareMode comparemode)
{
	SBServerListSort(&sb->list, ascending, sortkey, comparemode);
}
#ifdef GSI_UNICODE
void ServerBrowserSortW(ServerBrowser sb, SBBool ascending, unsigned short *sortkey, SBCompareMode comparemode)
{
	char sortkey_A[255];
	UCS2ToUTF8String(sortkey, sortkey_A);
	ServerBrowserSortA(sb, ascending, sortkey_A, comparemode);
}
#endif

char *ServerBrowserGetMyPublicIP(ServerBrowser sb)
{
	return inet_ntoa(*(struct in_addr *)&sb->list.mypublicip);
}

unsigned int ServerBrowserGetMyPublicIPAddr(ServerBrowser sb)
{
	return sb->list.mypublicip;
}

void ServerBrowserDisconnect(ServerBrowser sb)
{
	SBServerListDisconnect(&sb->list);
}

// Allows the user to specify a broadcast address for LAN hosting
void ServerBrowserLANSetLocalAddr(ServerBrowser sb, const char* theAddr)
{
	sb->list.mLanAdapterOverride = theAddr;
}
