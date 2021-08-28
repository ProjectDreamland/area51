#include "x_files.hpp"
#include "e_network.hpp"
#include "Entropy.hpp"
#include "NetworkMgr/MatchMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"

match_mgr   g_MatchMgr;
net_socket  g_LocalSocket;

xbool ActivateNetwork(void);
void SimpleDialog( const char* pText, f32 Timeout=0.2f);

const char* GetStateName(match_mgr_state State);

void MatchLoop( void )
{
    bitstream   BitStream;
    net_address Remote;
    xbool       Received;

    g_MatchMgr.Update(1.0f/32.0f);
    x_DelayThread(32);
    BitStream.Init(1024);
    Received = g_LocalSocket.Receive( Remote, BitStream );
    if( Received )
    {
        g_MatchMgr.ReceivePacket( Remote, BitStream );
    }
}
//------------------------------------------------------------------------
void AppMain(s32, char**)
{
    net_address     Broadcast;
    net_address     Remote;
    interface_info  Info;
    s32             i;
    char            Buffer[1024];
    s32             Length;

    eng_Init();             // Init Entropy
    net_Init();             // Init Network
    ActivateNetwork();      // Establish connection
    g_LocalSocket.Bind( NET_GAME_PORT,NET_FLAGS_BROADCAST );

    net_GetInterfaceInfo(-1,Info);
    Broadcast = net_address(Info.Broadcast, g_LocalSocket.GetPort() );

    g_MatchMgr.Init( g_LocalSocket, Broadcast );

    // Dummy values for pretend registration
    server_info& ServerInfo = g_MatchMgr.GetActiveConfig();

    x_wstrcpy(ServerInfo.Name,          L"A51");
    x_wstrcpy(ServerInfo.MissionName,   L"Test");
    x_wstrcpy(ServerInfo.GameType,      L"Deathmatch");
    x_wstrcpy(ServerInfo.ShortGameType, L"TDM");
    ServerInfo.MaxPlayers = 16;
    ServerInfo.Players    = 3;

    while( g_MatchMgr.GetState() != MATCH_IDLE )
    {
        SimpleDialog((const char*)xfs("MatchMaker connecting...\n"
                    "Status: %s",GetStateName(g_MatchMgr.GetState())),0.0f);
        MatchLoop();
    }

    g_MatchMgr.SetState( MATCH_BECOME_SERVER );
    while( g_MatchMgr.GetState() != MATCH_SERVER_ACTIVE )
    {
        SimpleDialog((const char*)xfs("MatchMaker connecting...\n"
                    "Status: %s",GetStateName(g_MatchMgr.GetState())),0.0f);
        MatchLoop();
    }

    g_MatchMgr.SetState( MATCH_ACQUIRE_SERVERS );
 
    while( g_MatchMgr.GetState() != MATCH_ACQUIRE_IDLE )
    {
        SimpleDialog((const char*)xfs("MatchMaker connecting...\n"
                    "Status: %s",GetStateName(g_MatchMgr.GetState())),0.0f);
        MatchLoop();
    }
    s32 j;

    for( j = 0; j < 5; j++ )
    {
        for( i = 0; i < g_MatchMgr.GetServerCount(); i++ )
        {
            const server_info& Info = g_MatchMgr.GetServerInfo(i);
            x_printfxy(5,i+4,"N:%16s, M:%2d, P:%2d, IP:%s",(const char*)xstring(Info.Name), Info.MaxPlayers, Info.Players, Info.Remote.GetStrAddress() );
            eng_PageFlip();
        }
    }

    // This is a hack. Try and establish a connection to that particular server.
    if( g_MatchMgr.GetServerCount() )
    {
        s32                 Status;
        const server_info&  Info = g_MatchMgr.GetServerInfo(0);

        net_socket Socket;

        Socket.Bind();

        Status = XNetRegisterKey( &Info.SessionID, &Info.ExchangeKey );
        ASSERT( Status == S_OK );

        s32 count;
        count = 0;
        while( TRUE )
        {
            const char TestString[]="This is a block of test data going off to the other machine";

            x_DelayThread(32);
            g_MatchMgr.Update( 0.033f );
            count++;
            if( count > 100 )
            {
                Socket.Send( Info.Remote, TestString, sizeof(TestString) );
                count = 0;
            }

            Length = sizeof(Buffer);
            if( g_LocalSocket.Receive( Remote, Buffer, Length ))
            {
                x_DebugMsg("Packet Received\n");
                BREAK;
            }

        }
    }

    BREAK;
}



//------------------------------------------------------------------------
// This is the code we need to make the network actually start up on PS2
//------------------------------------------------------------------------
xbool ActivateNetwork( void )
{
    xtimer          Timeout;
    interface_info  Info;

    s32             error;
    xbool           Done;

    SimpleDialog("Please wait...\n"
                 "Establishing Network Connection.",0.25f);


    net_BeginConfig();
    net_ActivateConfig(FALSE);
    net_EndConfig();

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

        eng_Begin("Screen Clear");
        rect Rect(0,0,640,480);
        draw_Rect(Rect,XCOLOR_BLACK,FALSE);
        eng_End();

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
