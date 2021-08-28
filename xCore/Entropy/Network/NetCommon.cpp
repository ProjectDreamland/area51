//=============================================================================
//
//  NETLIB.CPP
//
//=============================================================================
#ifdef WIN32
#include <winsock.h>
#endif

#include "x_files.hpp"
#include "e_Network.hpp"
#include "network/netstream.hpp"

static random  Random;

s32             g_SendHistory[NET_HISTORY_SIZE];
s32             g_ReceiveHistory[NET_HISTORY_SIZE];
s32             g_HistoryIndex;
s32             g_SendAverage;
s32             g_ReceiveAverage;
xtimer          g_HistoryTimer;

//#define VERBOSE_NETWORK

//=============================================================================
//=============================================================================
//=============================================================================
//  INTERNET EMULATION
//=============================================================================
//=============================================================================
//=============================================================================

#if defined(athyssen)
//#define SHOW_ISETTINGS_ACTIONS
#endif

internet_settings   ISettings;
net_stats           g_NetStats;

struct net_buffer
{
    net_address     Caller;
    net_address     Dest;
    byte            Buffer[MAX_PACKET_SIZE];
    s32             BufferSize;
    xtimer          SendTimer;
};

#define MAX_BUFFERED_SENDS  10

net_buffer      SendBuffers[MAX_BUFFERED_SENDS];
s32             SendBufferTail;
s32             SendBufferHead;
s32             SendBufferValid;
/*
static stream_chunk* s_pBuffer = NULL;
static s32           s_FirstSendBuffer = -1;
static s32           s_LastSendBuffer  = -1;
static s32           s_FirstRecvBuffer = -1;
static s32           s_LastRecvBuffer  = -1;
static s32           s_FirstFreeBuffer = -1;
*/

//=============================================================================

static s32 s_VersionKey = 0xDEADBEEF;
static s32 s_InitCount  = 0;

//=============================================================================

void net_Init(void)
{
    if( s_InitCount == 0 )
    {
        ISettings.BlockReceives      = FALSE;
        ISettings.BlockSends         = FALSE;
        ISettings.LatencyMs          = 0;
        ISettings.PercReceivesLost   = 0;
        ISettings.PercSendsLost      = 0;
        ISettings.PercPacketsDamaged = 0;

        SendBufferHead  = 0;
        SendBufferTail  = 0;
        SendBufferValid = 0;

        sys_net_Init();
        g_HistoryTimer.Start();
    }
    s_InitCount++;
#if defined(athyssen)
    //ISettings.PercReceivesLost = 10;
    //ISettings.PercSendsLost = 10;
#endif
}

//=============================================================================

void net_Kill(void)
{
    s_InitCount--;
    if( s_InitCount == 0 )
    {
        net_ActivateConfig(FALSE);
        sys_net_Kill();
    }
}

//=============================================================================

void net_UpdateHistory(s32 Sent, s32 Received)
{
    f32 Time;

    Time = g_HistoryTimer.ReadMs();
    if (Time >= NET_HISTORY_INTERVAL)
    {
        g_SendAverage = g_SendHistory[0];
        g_ReceiveAverage = g_ReceiveHistory[0];
#if 0
        s32 i,rec,send;

        rec = 0;
        send = 0;
        for (i=0;i<NET_HISTORY_SIZE;i++)
        {
            rec = rec + g_ReceiveHistory[i];
            send=send + g_SendHistory[i];
        }

        g_SendAverage    = (s32)((send * Time) / NET_HISTORY_SIZE);
        g_ReceiveAverage = (s32)((rec  * Time) / NET_HISTORY_SIZE);

        while (Time >= NET_HISTORY_INTERVAL)
        {
            g_HistoryIndex +=1;
            if (g_HistoryIndex >= NET_HISTORY_SIZE)
            {
                g_HistoryIndex = 0;
            }
            g_ReceiveHistory[g_HistoryIndex]=0;
            g_SendHistory[g_HistoryIndex]=0;
            Time -= NET_HISTORY_INTERVAL;
        }
#endif
        g_HistoryIndex      = 0;
        g_SendHistory[0]    = 0;
        g_ReceiveHistory[0] = 0;

        g_HistoryTimer.Trip();
    }
    g_ReceiveHistory[g_HistoryIndex] += Received;
    g_SendHistory   [g_HistoryIndex] += Sent;
}

//=============================================================================
void net_GetHistory(s32& SentPerSec, s32& ReceivedPerSec)
{
    net_UpdateHistory(0,0);
    SentPerSec      = g_SendAverage;
    ReceivedPerSec  = g_ReceiveAverage;
}


//=============================================================================
void net_SetVersionKey(s32 Version)
{
    s_VersionKey = Version;
}

//=============================================================================
s32  net_GetVersionKey(void)
{
    return s_VersionKey;
}

//==============================================================================

void    net_ClearStats      ( void )
{
    g_NetStats.BytesReceived    = 0;
    g_NetStats.PacketsReceived  = 0;
    g_NetStats.BytesSent        = 0;
    g_NetStats.PacketsSent      = 0;
    g_NetStats.SendTime.Reset();
    g_NetStats.ReceiveTime.Reset();
}

//==============================================================================

void    net_GetStats    ( s32& PacketsSent,
                          s32& BytesSent, 
                          s32& PacketsReceived,
                          s32& BytesReceived,
                          s32& NAddressesBound,
                          f32& SendTime,
                          f32& ReceiveTime )
{
    PacketsSent         = (s32)g_NetStats.PacketsSent;
    PacketsReceived     = (s32)g_NetStats.PacketsReceived;
    BytesSent           = (s32)g_NetStats.BytesSent;
    BytesReceived       = (s32)g_NetStats.BytesReceived;
    NAddressesBound     = g_NetStats.AddressesBound;
    SendTime            = g_NetStats.SendTime.ReadMs();
    ReceiveTime         = g_NetStats.ReceiveTime.ReadMs();
}
