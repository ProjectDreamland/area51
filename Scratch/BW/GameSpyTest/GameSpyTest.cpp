#include "x_files.hpp"
#include "e_network.hpp"
#include "Entropy.hpp"
#include "NetworkMgr/MatchMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "IOManager\io_mgr.hpp"

net_socket  g_LocalSocket;

#include "NetworkMgr/GameSpy/serverbrowsing/sb_serverbrowsing.h"
#include "NetworkMgr/GameSpy/available.h"
#include "NetworkMgr/GameSpy/qr2/qr2regkeys.h"
#include "NetworkMgr/GameSpy/qr2/qr2.h"
#include "NetworkMgr/GameSpy/gp/gp.h"

#include "NetworkMgr/Downloader/Downloader.hpp"
#include "NetworkMgr/Downloader/archive.hpp"

xbool g_bInsideRTF = FALSE;

char g_FullPath[]="Blah";
char g_LevelToLoad[]="blash";
s32 g_game_running;
xbool g_first_person;
xbool g_GameLogicDebug;
xbool g_FreeCam;
xbool g_UnlimitedAmmo;
xbool g_View;
xbool g_MirrorWeapon;
xbool g_AimAssist_Render_Player_Pills;
xbool g_AimAssist_Render_Reticle;
xbool g_AimAssist_Render_Bullet;
xbool g_AimAssist_Render_Bullet_Angle;
xbool g_AimAssist_Render_Turn;

xbool g_InvertYAxis;

xbool ActivateNetwork(void);
void SimpleDialog( const char* pText, f32 Timeout=0.2f);

#ifdef GSI_UNICODE
	#define _T(a) L##a
	#define _tprintf wprintf
#else
	#define _T(a) a
	#define _tprintf printf
#endif


#define NUM_SIMUL_QUERIES 30
static void SBCallback(ServerBrowser sb, SBCallbackReason reason, SBServer server, void *instance)
{
	int updatePercent = 0;
	gsi_char anAddress[20] = { '\0' };
#ifdef GSI_UNICODE
	if (server)
		AsciiToUCS2String(SBServerGetPublicAddress(server),anAddress);
#else
	if (server)
		strcpy(anAddress, SBServerGetPublicAddress(server));
#endif

	switch (reason)
	{
	case sbc_serveradded:
		if (SBFalse == SBServerHasBasicKeys(server))
			_tprintf(_T("Server Added: %s:%d\n"), anAddress, SBServerGetPublicQueryPort(server));
		else
		{
			_tprintf(_T("Firewall Server Added: %s:%d [%d] - %s - %d/%d (%d%%)\n"), anAddress, SBServerGetPublicQueryPort(server), SBServerGetPing(server),
			SBServerGetStringValue(server, _T("hostname"), _T("")), SBServerGetIntValue(server, _T("numplayers"), 0), SBServerGetIntValue(server, _T("maxplayers"), 0), updatePercent);
		}

		break;
	case sbc_serverupdated:
		//determine our update percent
		if (ServerBrowserCount(sb) > 0)
			updatePercent = (ServerBrowserCount(sb) - ServerBrowserPendingQueryCount(sb)) * 100 / ServerBrowserCount(sb);
		_tprintf(_T("Server Updated: %s:%d [%d] - %s - %d/%d (%d%%)\n"), anAddress, SBServerGetPublicQueryPort(server), SBServerGetPing(server),
			SBServerGetStringValue(server, _T("hostname"), _T("")), SBServerGetIntValue(server, _T("numplayers"), 0), SBServerGetIntValue(server, _T("maxplayers"), 0), updatePercent);
		break;
	case sbc_serverupdatefailed:
		_tprintf(_T("Update Failed: %s:%d\n"), anAddress, SBServerGetPublicQueryPort(server));
		break;
	case sbc_updatecomplete:
		_tprintf(_T("Server Browser Update Complete\r\n")); 
		break;
	case sbc_queryerror:
		_tprintf(_T("Query Error: %s\n"), ServerBrowserListQueryError(sb));
		break;
	default:
		break;
	}

	GSI_UNUSED(instance);
}

void serverkey_callback(int keyid, qr2_buffer_t outbuf, void *userdata);
void playerkey_callback(int keyid, int index, qr2_buffer_t outbuf, void *userdata);
void teamkey_callback(int keyid, int index, qr2_buffer_t outbuf, void *userdata);
void keylist_callback(qr2_key_type keytype, qr2_keybuffer_t keybuffer, void *userdata);
int count_callback(qr2_key_type keytype, void *userdata);
void adderror_callback(qr2_error_t error, gsi_char *errmsg, void *userdata);
void nn_callback(int cookie, void *userdata);
void cm_callback(gsi_char *data, int len, void *userdata);
void DoGameStuff(void);
int test_main(int argc, char **argp);

void serverkey_callback(int keyid, qr2_buffer_t outbuf, void *userdata)
{
	switch (keyid)
	{
	case HOSTNAME_KEY:
		qr2_buffer_add(outbuf, "Biscuits-PC");
		break;
	case GAMEVER_KEY:
		qr2_buffer_add(outbuf, "2.00");
		break;
	case HOSTPORT_KEY:
		qr2_buffer_add_int(outbuf, 3845);
		break;
	case MAPNAME_KEY:
		qr2_buffer_add(outbuf, "This is our map");
		break;
	case GAMETYPE_KEY:
		qr2_buffer_add(outbuf, "TDM");
		break;
	case NUMPLAYERS_KEY:
		qr2_buffer_add_int(outbuf, 1);
		break;
	case NUMTEAMS_KEY:
		qr2_buffer_add_int(outbuf, 1);
		break;
	case MAXPLAYERS_KEY:
		qr2_buffer_add_int(outbuf, 16);
		break;
	case GAMEMODE_KEY:
		qr2_buffer_add(outbuf, "blah");
		break;
	case TEAMPLAY_KEY:
		qr2_buffer_add_int(outbuf, 0);
		break;
	case FRAGLIMIT_KEY:
		qr2_buffer_add_int(outbuf, 10);
		break;
	case TIMELIMIT_KEY:
		qr2_buffer_add_int(outbuf, 10);
		break;
	default:
		qr2_buffer_add(outbuf, _T(""));
	}
	
	GSI_UNUSED(userdata);
}

void playerkey_callback(int keyid, int index, qr2_buffer_t outbuf, void *userdata)
{
	//check for valid index
	if (index >= 1)
	{
		qr2_buffer_add(outbuf, _T(""));
		return;
	}
	switch (keyid)
	{
	case PLAYER__KEY:
		qr2_buffer_add(outbuf, xfs("Player %d",index) );
		break;
	case SCORE__KEY:
		qr2_buffer_add_int(outbuf, 10);
		break;
	case DEATHS__KEY:
		qr2_buffer_add_int(outbuf, 5);
		break;
	case PING__KEY:
		qr2_buffer_add_int(outbuf, 100);
		break;
	case TEAM__KEY:
		qr2_buffer_add_int(outbuf, 1);
		break;
	default:
		qr2_buffer_add(outbuf, _T(""));
		break;		
	}
	
	GSI_UNUSED(userdata);
}

void teamkey_callback(int keyid, int index, qr2_buffer_t outbuf, void *userdata)
{
	//check for valid index
	if (index >= 1)
	{
		qr2_buffer_add(outbuf, _T(""));
		return;
	}
	switch (keyid)
	{
	case TEAM_T_KEY:
		qr2_buffer_add(outbuf, xfs("Team %d",index));
		break;
	case SCORE_T_KEY:
		qr2_buffer_add_int(outbuf, index*100);
		break;
	default:
		qr2_buffer_add(outbuf, _T(""));
		break;		
	}
	
	GSI_UNUSED(userdata);
}	

void keylist_callback(qr2_key_type keytype, qr2_keybuffer_t keybuffer, void *userdata)
{
	//need to add all the keys we support
	switch (keytype)
	{
	case key_server:
		qr2_keybuffer_add(keybuffer, HOSTNAME_KEY);
		qr2_keybuffer_add(keybuffer, GAMEVER_KEY);
		qr2_keybuffer_add(keybuffer, HOSTPORT_KEY);
		qr2_keybuffer_add(keybuffer, MAPNAME_KEY);
		qr2_keybuffer_add(keybuffer, GAMETYPE_KEY);
		qr2_keybuffer_add(keybuffer, NUMPLAYERS_KEY);
		qr2_keybuffer_add(keybuffer, NUMTEAMS_KEY);
		qr2_keybuffer_add(keybuffer, MAXPLAYERS_KEY);
		qr2_keybuffer_add(keybuffer, GAMEMODE_KEY);
		qr2_keybuffer_add(keybuffer, TEAMPLAY_KEY);
		qr2_keybuffer_add(keybuffer, FRAGLIMIT_KEY);
		qr2_keybuffer_add(keybuffer, TIMELIMIT_KEY);
		break;
	case key_player:
		qr2_keybuffer_add(keybuffer, PLAYER__KEY);
		qr2_keybuffer_add(keybuffer, SCORE__KEY);
		qr2_keybuffer_add(keybuffer, DEATHS__KEY);
		qr2_keybuffer_add(keybuffer, PING__KEY);
		qr2_keybuffer_add(keybuffer, TEAM__KEY);
		break;
	case key_team:
		qr2_keybuffer_add(keybuffer, TEAM_T_KEY);
		qr2_keybuffer_add(keybuffer, SCORE_T_KEY);
		break;
	}
	
	GSI_UNUSED(userdata);
}

int count_callback(qr2_key_type keytype, void *userdata)
{
	if (keytype == key_player)
		return 1;
	else if (keytype == key_team)
		return 1;
	else
		return 0;
		
	GSI_UNUSED(userdata);
}

void adderror_callback(qr2_error_t error, gsi_char *errmsg, void *userdata)
{
	_tprintf(_T("Error adding server: %d, %s\n"), error, errmsg);
	
	GSI_UNUSED(userdata);
}


static GPProfile s_Profile;
static void ConnectResponse(GPConnection * pconnection, GPConnectResponseArg * arg, void * param)
{
    if(arg->result == GP_NO_ERROR)
        printf("CONNECTED\n");
    else
        printf( "CONNECT FAILED\n");

    s_Profile = arg->profile;

    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

static void Error(GPConnection * pconnection, GPErrorArg * arg, void * param)
{
    char * errorCodeString;
    char * resultString;

#define RESULT(x) case x: resultString = #x; break;
    switch(arg->result)
    {
        RESULT(GP_NO_ERROR)
            RESULT(GP_MEMORY_ERROR)
            RESULT(GP_PARAMETER_ERROR)
            RESULT(GP_NETWORK_ERROR)
            RESULT(GP_SERVER_ERROR)
    default:
        resultString = "Unknown result!\n";
    }

#define ERRORCODE(x) case x: errorCodeString = #x; break;
    switch(arg->errorCode)
    {
        ERRORCODE(GP_GENERAL)
            ERRORCODE(GP_PARSE)
            ERRORCODE(GP_NOT_LOGGED_IN)
            ERRORCODE(GP_BAD_SESSKEY)
            ERRORCODE(GP_DATABASE)
            ERRORCODE(GP_NETWORK)
            ERRORCODE(GP_FORCED_DISCONNECT)
            ERRORCODE(GP_CONNECTION_CLOSED)
            ERRORCODE(GP_LOGIN)
            ERRORCODE(GP_LOGIN_TIMEOUT)
            ERRORCODE(GP_LOGIN_BAD_NICK)
            ERRORCODE(GP_LOGIN_BAD_EMAIL)
            ERRORCODE(GP_LOGIN_BAD_PASSWORD)
            ERRORCODE(GP_LOGIN_BAD_PROFILE)
            ERRORCODE(GP_LOGIN_PROFILE_DELETED)
            ERRORCODE(GP_LOGIN_CONNECTION_FAILED)
            ERRORCODE(GP_LOGIN_SERVER_AUTH_FAILED)
            ERRORCODE(GP_NEWUSER)
            ERRORCODE(GP_NEWUSER_BAD_NICK)
            ERRORCODE(GP_NEWUSER_BAD_PASSWORD)
            ERRORCODE(GP_UPDATEUI)
            ERRORCODE(GP_UPDATEUI_BAD_EMAIL)
            ERRORCODE(GP_NEWPROFILE)
            ERRORCODE(GP_NEWPROFILE_BAD_NICK)
            ERRORCODE(GP_NEWPROFILE_BAD_OLD_NICK)
            ERRORCODE(GP_UPDATEPRO)
            ERRORCODE(GP_UPDATEPRO_BAD_NICK)
            ERRORCODE(GP_ADDBUDDY)
            ERRORCODE(GP_ADDBUDDY_BAD_FROM)
            ERRORCODE(GP_ADDBUDDY_BAD_NEW)
            ERRORCODE(GP_ADDBUDDY_ALREADY_BUDDY)
            ERRORCODE(GP_AUTHADD)
            ERRORCODE(GP_AUTHADD_BAD_FROM)
            ERRORCODE(GP_AUTHADD_BAD_SIG)
            ERRORCODE(GP_STATUS)
            ERRORCODE(GP_BM)
            ERRORCODE(GP_BM_NOT_BUDDY)
            ERRORCODE(GP_GETPROFILE)
            ERRORCODE(GP_GETPROFILE_BAD_PROFILE)
            ERRORCODE(GP_DELBUDDY)
            ERRORCODE(GP_DELBUDDY_NOT_BUDDY)
            ERRORCODE(GP_DELPROFILE)
            ERRORCODE(GP_DELPROFILE_LAST_PROFILE)
            ERRORCODE(GP_SEARCH)
            ERRORCODE(GP_SEARCH_CONNECTION_FAILED)
    default:
        errorCodeString = "Unknown error code!\n";
    }

    if(arg->fatal)
    {
        printf( "-----------\n");
        printf( "FATAL ERROR\n");
        printf( "-----------\n");
    }
    else
    {
        printf( "-----\n");
        printf( "ERROR\n");
        printf( "-----\n");
    }
    printf( "RESULT: %s (%d)\n", resultString, arg->result);
    printf( "ERROR CODE: %s (0x%X)\n", errorCodeString, arg->errorCode);
    printf( "ERROR STRING: %s\n", arg->errorString);

    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

gsi_char whois[GP_NICK_LEN];
static void Whois(GPConnection * pconnection, GPGetInfoResponseArg * arg, void * param)
{
    if(arg->result == GP_NO_ERROR)
    {
        x_DebugMsg( "Result:%d, nick:%s, email:%s\n",arg->result,arg->nick, arg->email);
        x_DebugMsg( "FirstName:%s, LastName:%s, HomePage:%s\n",arg->firstname, arg->lastname, arg->homepage );
        x_DebugMsg( "icquin:%d, zipcode:%s, countrycode:%s\n", arg->icquin, arg->zipcode, arg->countrycode );
        x_DebugMsg( "Birthday: %d/%d/%d, gender: %d, publicmask:%d", arg->birthday,arg->birthmonth, arg->birthyear, arg->sex, arg->publicmask );
#ifndef GSI_UNICODE
        strcpy(whois, arg->nick);
        strcat(whois, "@");
        strcat(whois, arg->email);
#else
        wcscpy(whois, arg->nick);
        wcscat(whois, L"@");
        wcscat(whois, arg->email);
#endif
    }
    else
        printf( "WHOIS FAILED\n");

    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

GPProfile other;

static void RecvBuddyMessage(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
    static int msgCount = 0;
    msgCount++;
    VERIFY( gpGetInfo( pconnection, arg->profile, GP_DONT_CHECK_CACHE, GP_BLOCKING, (GPCallback)Whois, NULL) == GP_NO_ERROR );
    printf("MESSAGE (%d): %s: %s\n", msgCount,whois, arg->message);

    if(msgCount < 5)
    {
        VERIFY(gpSendBuddyMessage(pconnection, other, _T("HELLO!")) == GP_NO_ERROR );
    }

    GSI_UNUSED(pconnection);
    GSI_UNUSED(param);
}

static void s_SearchResult( GPConnection* , GPProfileSearchResponseArg* pResult, void* )
{
    s32 i;
    GPProfileSearchMatch* pMatch;

    x_DebugMsg("Search Result: %d, Matches:%d, More:%s\n", pResult->result, pResult->numMatches, pResult->more?"TRUE":"FALSE" );
    for( i=0; i < pResult->numMatches; i++ )
    {
        pMatch = &pResult->matches[i];
        x_DebugMsg("Match:%d, Nick:%s, UniqueNick:%s, Name:%s,%s, Email:%s\n",i,pMatch->nick, pMatch->uniquenick, pMatch->firstname, pMatch->lastname, pMatch->email );
    }
}

void MatchLoop( void )
{
    bitstream   BitStream;
    net_address Remote;
    xbool       Received;

    x_DelayThread(32);
    BitStream.Init(1024);
    Received = g_LocalSocket.Receive( Remote, BitStream );
    if( Received )
    {
        g_MatchMgr.ReceivePacket( Remote, BitStream );
    }
}
//------------------------------------------------------------------------
#include "e_Memcard.hpp"

void AppMain(s32, char**)
{
    net_address     Broadcast;
    interface_info  Info;
    f32             LastComplete;
    X_FILE*         Handle;
    s32             Length;

    eng_Init();             // Init Entropy
    g_IoMgr.Init();
    net_Init();             // Init Network
    g_MemcardHardware.Init();
    ActivateNetwork();      // Establish connection

    // Start an HTTP transfer request

    downloader Downloader;
again:
    x_DebugMsg("Initialized downloader. %d Bytes free\n",x_MemGetFree() );

    x_MemSanity();
    Downloader.Init("http://www.inevitable.com/DownloadTest/archive.gz");
    x_MemSanity();

    while( Downloader.GetStatus() == DL_STAT_BUSY )
    {
        Downloader.Update( 1.0f /32.0f );
        x_DelayThread( 10 );
        if( LastComplete != Downloader.GetProgress() )
        {
            LastComplete = Downloader.GetProgress();
            x_DebugMsg("Downloading..... Completed: %2.02f%%\n", LastComplete*100.0f );
        }
        x_MemSanity();
    }

    ASSERT( Downloader.GetStatus() == DL_STAT_OK );
    x_DebugMsg("File downloaded. %d bytes free.\n",x_MemGetFree() );

    archive Archive;

    x_MemSanity();
    xbool Ok = Archive.Init( (byte*)Downloader.GetFileData(), Downloader.GetFileLength() );
    x_DebugMsg("Archive initialized. %d bytes free.\n",x_MemGetFree() );
    x_MemSanity();
    ASSERT( Ok );
    Downloader.Kill();
    x_MemSanity();
    x_DebugMsg("Downloader killed. %d bytes free.\n",x_MemGetFree() );
    s32 i;

    x_DebugMsg("Archive contains %d files\n", Archive.GetMemberCount() );
    for( i=0; i< Archive.GetMemberCount(); i++ )
    {
        x_DebugMsg("Archive Member: %s, Length:%d\n",Archive.GetMemberFilename( i ), Archive.GetMemberLength( i ) );
        x_MemSanity();
        Handle = x_fopen( xfs("Directory\\OtherDirectory\\BaseDirectory\\%d\\%s", i, Archive.GetMemberFilename( i ) ), "wb" );
        ASSERT( Handle );
        Length = x_fwrite( Archive.GetMemberData( i ), 1, Archive.GetMemberLength( i ), Handle );
        ASSERT( Length == Archive.GetMemberLength( i ) );
        x_fclose( Handle );
        LOG_FLUSH();
    }

    x_MemSanity();
    Archive.Kill();
    x_DebugMsg("Archive killed. %d bytes free.\n",x_MemGetFree() );
    x_MemSanity();

    x_DelayThread(5000);

    ASSERT( FALSE );
    //
    // Not worth going down here. Nothing will work properly anymore!
    //
    char url[256];
    char* pBuffer;
    char* pData;

    Length = Downloader.GetFileLength();
    pBuffer = (char*)x_malloc(Length);

    pData = pBuffer;
    s32 FileIndex=0;

    while( Length )
    {
        char* pUrl = url;
        while( Length && (*pData !='\n') )
        {
            *pUrl++ = *pData++;
            Length--;
        }

        *pUrl = 0x0;

        if( Length )
        {
            pData++;
            Length--;
        }
        x_DebugMsg("Starting download of %s\n",url);
        xtimer t;

        t.Start();
        Downloader.Init( url );
        while( Downloader.GetStatus() == DL_STAT_BUSY )
        {
            Downloader.Update( 1.0f / 32.0f );
            x_DelayThread(10);
            if( LastComplete != Downloader.GetProgress() )
            {
                LastComplete = Downloader.GetProgress();
                //x_DebugMsg("Downloading..... Completed: %2.02f%%\n", LastComplete*100.0f );
            }
        }
        t.Stop();
        ASSERT( Downloader.GetStatus() == DL_STAT_OK );
        x_DebugMsg("Downloaded %d bytes in %2.02f seconds, %d bytes/sec\n",Downloader.GetFileLength(), t.ReadSec(), (s32)(Downloader.GetFileLength() / t.ReadSec()) );
        Handle = x_fopen( xfs("Downloaded_file_%d.bin", FileIndex), "wb" );
        ASSERT( Handle );
        x_fwrite( Downloader.GetFileData(), Downloader.GetFileLength(), 1, Handle );
        x_fclose( Handle );
        Downloader.Kill();
    }

    goto again;

    BREAK;
    g_LocalSocket.Bind( NET_GAME_PORT );

    net_GetInterfaceInfo(-1,Info);
    Broadcast = net_address(Info.Broadcast, g_LocalSocket.GetPort() );

    g_MatchMgr.Init( g_LocalSocket, Broadcast );

    while( g_MatchMgr.GetState() != MATCH_IDLE )
    {
        MatchLoop();
    }

    BREAK;
    xtimer t;

    t.Start();
#if 0
    g_MatchMgr.StartAcquisition( ACQUIRE_SERVERS );
    while( g_MatchMgr.IsAcquireComplete() == FALSE )
    {
        MatchLoop();
        if( t.ReadSec() > 5.0f )
        {
            break;
        }
    }
#endif

#define GPTC_PRODUCTID 0

    GPConnection    Connection;
    char            Nickname[64];
    char            Email[64];
    char            Password[64];
    const byte*     UniqueId;
    s32             IdLength;

    // MatchMgr should have a system ID derived from the machine ID if DNAS has not
    // yet been performed.
    UniqueId = g_MatchMgr.GetUniqueId(IdLength);

    x_sprintf( Nickname, "UserDefinedName" );
    ASSERT( x_strlen( (const char*)UniqueId ) > 0 );
    x_sprintf( Email, "%s@area51.midway.com", UniqueId );
    x_sprintf( Password, "%s@area51.midway.com", UniqueId );

    GPResult Status;

    gpInitialize( &Connection, GPTC_PRODUCTID, 0 );

    Status = gpSetCallback(&Connection, GP_ERROR, (GPCallback)Error, NULL );
    ASSERT( Status == GP_NO_ERROR );
    Status = gpSetCallback(&Connection, GP_RECV_BUDDY_MESSAGE, (GPCallback)RecvBuddyMessage, NULL );
    ASSERT( Status == GP_NO_ERROR );
    Status = gpConnect(&Connection, Nickname, Email, Password, GP_FIREWALL, GP_BLOCKING, (GPCallback)ConnectResponse, NULL );

    BREAK;

    if( Status == GP_SERVER_ERROR )
    {
        GPErrorCode ErrorCode;

        gpGetErrorCode( &Connection, &ErrorCode );
        ASSERT( ErrorCode == GP_LOGIN_BAD_EMAIL );

        // User account may have not existed so we try to create one.
        Status = gpConnectNewUser(&Connection, Nickname, NULL, Email, Password, "", GP_FIREWALL, GP_BLOCKING, (GPCallback)ConnectResponse, NULL );
        ASSERT( Status == GP_NO_ERROR );

        gpGetErrorCode( &Connection, &ErrorCode );
        // Do we need to login now?
    }

    gpSetInfos( &Connection, GP_FIRSTNAME, "Area51" );
    ASSERT( Status == GP_NO_ERROR );

    Status = gpGetInfo( &Connection, s_Profile, GP_DONT_CHECK_CACHE, GP_BLOCKING, (GPCallback)Whois, NULL );


    Status = gpProfileSearch( &Connection,  // Connection
                              "",           // Nickname
                              "",           // UniqueNick
                              "",           // Email
                              "Area51",     // Firstname
                              "",           // Lastname
                              0,            // icquin
                              GP_BLOCKING,  // IsBlocking
                              (GPCallback)s_SearchResult,
                              NULL );

    ASSERT( Status == GP_NO_ERROR );

    g_MatchMgr.StartAcquisition( ACQUIRE_BUDDIES );
    while( g_MatchMgr.IsAcquireComplete() == FALSE )
    {
        MatchLoop();
        if( t.ReadSec() > 5.0f )
        {
            break;
        }
    }

    BREAK;
	ServerBrowser sb;
	unsigned char basicFields[] = {HOSTNAME_KEY, GAMETYPE_KEY,  MAPNAME_KEY, NUMPLAYERS_KEY, MAXPLAYERS_KEY};
	int numFields = sizeof(basicFields) / sizeof(basicFields[0]);
	GSIACResult result;

	// check that the game's backend is available
	GSIStartAvailableCheck(_T("gmtest"));
	while((result = GSIAvailableCheckThink()) == GSIACWaiting)
		msleep(5);
	if(result != GSIACAvailable)
	{
		printf("The backend is not available\n");
		BREAK;
	}

	//create a new server browser object
	sb = ServerBrowserNew (_T("gmtest"), _T("gmtest"), _T("HA6zkS"), 0, 40, QVERSION_GOA, SBCallback, NULL);
	
    // Initialize query & reporting
#if 1
    qr2_error_t QueryError;

    QueryError = qr2_init(   
                NULL,               // query record?
                NULL,               // Local IP?
                24576,              // Local Port?
                "gmtest",           // Game Name
                "HA6zkS",           // Secret Key
                SBTrue,             // Is Public
                SBTrue,             // Perform NAT negotiation
                serverkey_callback, // Server key probe
                playerkey_callback, // Player key probe
                teamkey_callback,   // Team key probe
                keylist_callback,   // Give our list of keys?
                count_callback,     // Player counts?
                adderror_callback,  // Some error?
                NULL);              // User data
#endif
    // Register with master server
    
	//begin the update (async)
	//ServerBrowserLANUpdate(sb, SBTrue, 11111, 11111);
	//ServerBrowserAuxUpdateIP(sb, _T("209.190.4.30"), 1717, SBFalse, SBTrue, SBTrue);
	ServerBrowserUpdate(sb, SBTrue, SBTrue, basicFields, numFields, NULL);
	//ServerBrowserUpdate(sb, SBTrue, SBFalse, basicFields, numFields, NULL);
	
	//think while the update is in progress
    while ( TRUE )
    {
        SBError error;
        qr2_think(NULL);
        error = ServerBrowserThink(sb);
        if( error != sbe_noerror )
            break;
        x_DelayThread(32);
    }

    ServerBrowserFree(sb); //how to clean up, if we were to actually get here

    BREAK;
}



//------------------------------------------------------------------------
// This is the code we need to make the network actually start up on PS2
//------------------------------------------------------------------------
xbool ActivateNetwork( void )
{
    xtimer          Timeout;
    interface_info  Info;

    s32             status;
    s32             error;
    xbool           Done;

    SimpleDialog("Please wait...\n"
                 "Establishing Network Connection.",0.25f);


#if defined(TARGET_PS2)
    char            Path[64];
    s32             ConfigIndex;
    net_config_list ConfigList;

    net_BeginConfig();
    net_ActivateConfig(FALSE);
    // First, try to find ANY network configuration
    x_strcpy(Path,"mc0:BWNETCNF/BWNETCNF");
    status = net_GetConfigList(Path,&ConfigList);
    if ( (status < 0) || (ConfigList.Count <= 0) )
    {
        x_strcpy(Path,"mc1:BWNETCNF/BWNETCNF");
        status = net_GetConfigList(Path,&ConfigList);
        if ( (status < 0) || (ConfigList.Count <= 0) )
        {
            SimpleDialog("No network configurations present.",1.5f);
            return FALSE;
        }
    }

    // Just use the first one.
    ConfigIndex = 0;
    status = net_SetConfiguration(Path,ConfigIndex);
    ASSERT(status >=0);
#endif

    Timeout.Reset();
    Timeout.Start();
    Done = FALSE;

    while (Timeout.ReadSec() < 30.0f)
    {
        error = 0;
        error = net_GetAttachStatus(error);
        if ( (error==ATTACH_STATUS_CONFIGURED) ||
             (error==ATTACH_STATUS_ATTACHED) )
        {
            net_ActivateConfig(TRUE);

            // Wait until DHCP assigns us an address
            while ( Timeout.ReadSec() < 30.0f )
            {
                SimpleDialog((const char*)xfs("Please wait...\n"
                             "Connecting to the network.\n"
                             "Timeout remaining: %d",30 - (s32)Timeout.ReadSec()) );
                net_GetInterfaceInfo(-1,Info);
                if (Info.Address)
                {
                    Done = TRUE;
                    break;
                }
            }
            ASSERT(Done);
            break;
        }
        else
        {
            // Invalid net config file - its fatal!
            ASSERT( 0 );
        }
    }
    net_EndConfig();

    return TRUE;
}

//==============================================================================
void SimpleDialog( const char* pText, f32 Timeout)
{
    xtimer      t;
    const char* pString;
    s32         LineCount;
    s32         y;

    t.Start();

    LineCount = 1;

    pString = pText;
    while (*pString)
    {
        if (*pString=='\n')
        {
            LineCount++;
        }
        pString++;
    }


    do
    {
        char    TextBuffer[128];
        char*   pTextBuffer;

        pString = pText;
        y       = 10 - (LineCount/2);

        while (*pString)
        {
            pTextBuffer = TextBuffer;

            while ( (*pString != 0) && (*pString !='\n') )
            {
                *pTextBuffer++ = *pString++;
            }

            if (*pString == '\n')
            {
                pString++;
            }
            *pTextBuffer = 0x0;
            x_printfxy((40-x_strlen(TextBuffer))/2,y,TextBuffer);
            y++;
        }

        eng_PageFlip();
    } while ( t.ReadSec() < Timeout );
}

