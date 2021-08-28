//=============================================================================
//
//  NETSOCKET.CPP
//
//=============================================================================
#include "x_files.hpp"
#include "e_Network.hpp"
#include "x_bitstream.hpp"

//=============================================================================
net_socket::net_socket(void)
{
    Clear();
}

//=============================================================================
void net_socket::Clear( void )
{
    m_Socket            = BAD_SOCKET;
    m_Flags             = 0;
    m_BytesSent         = 0;
    m_BytesReceived     = 0;
    m_PacketsSent       = 0;
    m_PacketsReceived   = 0;
}

//=============================================================================
void net_socket::SetBlocking(xbool Block)
{
    if (Block)
        m_Flags |= NET_FLAGS_BLOCKING;
    else
        m_Flags &= ~NET_FLAGS_BLOCKING;
}

//=============================================================================
xbool net_socket::GetBlocking(void) const
{
    return (m_Flags & NET_FLAGS_BLOCKING) != 0;
}

//=============================================================================
xbool net_socket::IsClosed( void )
{
    return (m_Flags & NET_FLAGS_CLOSED) != 0;
}

//=============================================================================
typedef struct s_receive_delay_buffer
{
    struct s_receive_delay_buffer *pNext;
    net_socket          Local;
    net_address         Remote;
    s32                 Length;
    byte                Buffer[1024];
    xtimer              DelayTime;
    
} receive_delay_buffer;

static receive_delay_buffer *s_DelayBuffer=NULL;


//=============================================================================
xbool net_socket::Receive( net_address& Remote, void* pBuffer, s32& BufferSize )
{
    receive_delay_buffer *pDelayBuffer,*pPrevDelayBuffer,*pOldBuffer;
    f32 time;

    ASSERT(m_Socket != BAD_SOCKET);
    ASSERT(GetPort());
    ASSERT(GetIP());
    g_NetStats.ReceiveTime.Start();

    xbool Return;

    if (ISettings.LatencyMs)
    {
        // First check to see if we have anything in the delay buffer that has time expired
        pDelayBuffer = s_DelayBuffer;
        pPrevDelayBuffer = NULL;

        while (pDelayBuffer)
        {
            time = pDelayBuffer->DelayTime.ReadMs();
            if( (pDelayBuffer->Remote.GetPort() == GetPort()) && (time > ISettings.LatencyMs) )
            {
                if( pPrevDelayBuffer )
                {
                    pPrevDelayBuffer->pNext = pDelayBuffer->pNext;
                }
                else
                {
                    s_DelayBuffer = pDelayBuffer->pNext;
                }
                break;
            }
            else
            {
                // If something has been in the buffer for WAY too long, let's just discard it (10 seconds)
                if (time > 10000.0f)
                {
                    if (pPrevDelayBuffer)
                    {
                        pPrevDelayBuffer->pNext = pDelayBuffer->pNext;
                    }
                    else
                    {
                        s_DelayBuffer = pDelayBuffer->pNext;
                    }
                    pOldBuffer = pDelayBuffer;
                    pDelayBuffer = pDelayBuffer->pNext;
                    x_free(pOldBuffer);
                }
                else
                {
                    pPrevDelayBuffer = pDelayBuffer;
                    pDelayBuffer = pDelayBuffer->pNext;
                }
            }
        }
        //
        // If we have latency emulation going, let's put any new packets in the pending
        // buffer and leave them there.
        //
        Return =  sys_net_Receive( *this, Remote, pBuffer, BufferSize );
        if (Return)
        {
            receive_delay_buffer *pDelay;

            pDelay = (receive_delay_buffer *)x_malloc(sizeof(receive_delay_buffer));
            ASSERTS(pDelay,"Too many requests got queued for too long");
            pDelay->Local   = *this;
            pDelay->Remote  = Remote;
            pDelay->Length  = BufferSize;
            pDelay->pNext   = NULL;
            pDelay->DelayTime.Reset();
            pDelay->DelayTime.Start();
            x_memcpy(pDelay->Buffer,pBuffer,BufferSize);
            //
            // Insert this buffer in to the delay list at the end
            //
            if (s_DelayBuffer)
            {
                pPrevDelayBuffer = s_DelayBuffer;
                while (pPrevDelayBuffer->pNext)
                {
                    pPrevDelayBuffer = pPrevDelayBuffer->pNext;
                }
                pPrevDelayBuffer->pNext = pDelay;
            }
            else
            {
                s_DelayBuffer = pDelay;
            }
        }
        // If we had something in the delay buffer, we just return that for now.
        if (pDelayBuffer)
        {
//            x_DebugMsg("netlib: Delayed a packet for a total of %2.2fms\n",pDelayBuffer->DelayTime.ReadMs());
            Remote = pDelayBuffer->Remote;
            x_memcpy(pBuffer,pDelayBuffer->Buffer,pDelayBuffer->Length);
            BufferSize = pDelayBuffer->Length;
            x_free(pDelayBuffer);
            Return = TRUE;
        }
        else
        {
            Return = FALSE;
        }
    }
    else
    {
        BufferSize = MAX_PACKET_SIZE;
        Return =  sys_net_Receive( *this, Remote, pBuffer, BufferSize );
        if (!Return)
        {
            if( BufferSize == -1 )
            {
                m_Flags |= NET_FLAGS_CLOSED;
            }
            BufferSize = 0;
        }
        else
        {
#if defined(VERBOSE_NETWORK)
            char textbuff[768];
            u16* p16 = (u16*)pBuffer; 
            LOG_MESSAGE("net_socket::Receive",
                        "Size:%d -- IP:%s",
                        BufferSize, Remote.GetStrAddress(textbuff) );
            s32 j = 0;
            for( s32 i=0 ; (i<BufferSize) && (j<700) ; i+= 8 )
            {
                x_memcpy( &textbuff[j], xfs( "%04x:%04x:%04x:%04x:", p16[0], p16[1], p16[2], p16[3] ), 20 );
                p16 +=  4;
                j   += 20;
            }
            ASSERT( j<768 );
            textbuff[j] = 0;
            LOG_MESSAGE( "net_socket::Receive(data)", textbuff );
#endif
        }

    }   

    g_NetStats.ReceiveTime.Stop();

    if( (ISettings.BlockReceives || (ISettings.Random.irand(0,100) < ISettings.PercReceivesLost)) && ((m_Flags & NET_FLAGS_TCP)==0) )
    {
        #ifdef SHOW_ISETTINGS_ACTIONS
        x_DebugMsg("NetCommon: RECEIVE DROPPED\n");
        #endif
        return FALSE;
    }

    if (Return)
    {
        g_NetStats.BytesReceived    += BufferSize;
        g_NetStats.PacketsReceived  += 1;
        m_BytesReceived             += BufferSize;
        m_PacketsReceived           += 1;
        net_UpdateHistory(0,BufferSize);
    }
    else
    {
        net_UpdateHistory(0,0);
    }

    ASSERT( BufferSize <= MAX_PACKET_SIZE );
    //if( BufferSize > 0 )
        //x_DebugMsg("net::RECV %1d\n",BufferSize);
    return Return;
}

//==============================================================================
// We have a receive on a bitstream struct which has it's own embedded data
// pointer. So we use reference to that for target data address and the size
// of the resulting buffer.
xbool net_socket::Receive( net_address& Remote, bitstream& Bitstream)
{
    s32     Size;
    xbool   ok;
    byte*   pData;

    Bitstream.Clear();
    Size = Bitstream.GetNBytes();
    pData = Bitstream.GetDataPtr();
    ok = Receive(Remote,pData,Size);

    if (ok)
    {
        Bitstream.Init(pData,Size);
    }
    return ok;
}


//==============================================================================
// This receive is for a TCP bound socket only
xbool net_socket::Receive( void* pBuffer, s32& Length )
{
    net_address Dummy;

    ASSERT( m_Flags & NET_FLAGS_TCP );
    return Receive(Dummy,pBuffer,Length);

}

//==============================================================================
// This send is for a TCP bound socket only
xbool net_socket::Send( const void* pBuffer, s32 Length )
{
    net_address Dummy(-1,-1);
    ASSERT( m_Flags & NET_FLAGS_TCP );
    return Send( Dummy, pBuffer, Length );
}

//==============================================================================

xbool net_socket::Send( const net_address& Remote, bitstream& BitStream)
{
    return Send( Remote, BitStream.GetDataPtr(), BitStream.GetNBytesUsed() );
}

//==============================================================================

xbool net_socket::Send( const net_address& Remote, const void* pBuffer, s32 BufferSize )
{
    ASSERT( BufferSize <= MAX_PACKET_SIZE );
    ASSERT( m_Socket != BAD_SOCKET );
    ASSERT( GetIP() );
    ASSERT( GetPort() );
    ASSERT( Remote.GetIP() );
    ASSERT( Remote.GetPort() );

    if( (m_Flags & NET_FLAGS_TCP)==0 )
    {
        if( (ISettings.BlockSends) || 
            (ISettings.Random.irand(0,100) < ISettings.PercSendsLost) )
        {
            #ifdef SHOW_ISETTINGS_ACTIONS
            x_DebugMsg("NetCommon: SEND DROPPED\n");
            #endif
            return FALSE;
        }

        if( (ISettings.pPacketSwapBuffer == NULL) && 
            (ISettings.Random.irand(0,100) < ISettings.PercPacketsSwapped) )
        {
            ISettings.pPacketSwapBuffer = (byte*)x_malloc(BufferSize);
            x_memcpy(ISettings.pPacketSwapBuffer,pBuffer,BufferSize);

            ISettings.PacketSwapCaller = *this;
            ISettings.PacketSwapDest   = Remote;
            ISettings.PacketSwapSize   = BufferSize;
            return FALSE;
        }

        if( (ISettings.PercPacketsDamaged) && 
            (ISettings.Random.irand(0,100) < ISettings.PercPacketsDamaged) )
        {
            if( BufferSize > 8 )
            {
                s32  Count = BufferSize-4 / 4;
                s32  Which = ISettings.Random.irand( 0, Count );
                s32  Bit   = ISettings.Random.irand( 0, 31 );
                u32& Munge = *(((u32*)pBuffer) + Which);

                Munge ^= (1 << Bit);
            }
        }
    }

    g_NetStats.SendTime.Start();
#if defined(VERBOSE_NETWORK)
    char textbuff[768];
    u16* p16 = (u16*)pBuffer; 
    Remote.GetStrAddress( textbuff );
    LOG_MESSAGE( "net_socket::Send", 
                 "Size:%d -- IP:%s", 
                 BufferSize, textbuff );

    s32 j = 0;
    for( s32 i=0 ; (i<BufferSize) && (j<700) ; i+= 8 )
    {
        x_memcpy( &textbuff[j], xfs( "%04x:%04x:%04x:%04x:", p16[0], p16[1], p16[2], p16[3] ), 20 );
        p16 +=  4;
        j   += 20;
    }
    ASSERT( j < 768 );
    textbuff[j] = 0;
    LOG_MESSAGE( "net_socket::Send(data)", textbuff );
#endif
    sys_net_Send( *this, Remote, pBuffer, BufferSize );

    g_NetStats.SendTime.Stop();
    g_NetStats.BytesSent    += BufferSize;
    g_NetStats.PacketsSent  += 1;
    m_BytesSent             += BufferSize;
    m_PacketsSent           += 1;

    net_UpdateHistory(BufferSize,0);

    if( (m_Flags & NET_FLAGS_TCP)==0 )
    {
        if( ISettings.pPacketSwapBuffer != NULL )
        {
            x_DebugMsg("!!!Sending Swapped Buffer!!!\n");
            g_NetStats.SendTime.Start();
            sys_net_Send( ISettings.PacketSwapCaller, ISettings.PacketSwapDest, ISettings.pPacketSwapBuffer, ISettings.PacketSwapSize );

            g_NetStats.SendTime.Stop();
            g_NetStats.BytesSent    += BufferSize;
            g_NetStats.PacketsSent  += 1;

            m_BytesSent             += BufferSize;
            m_PacketsSent           += 1;

            x_free(ISettings.pPacketSwapBuffer);
            ISettings.pPacketSwapBuffer = NULL;
        }
    }
#ifdef athyssen
    //SendTimer.Start();
    //x_DebugMsg("net::SEND %04d %8.3f\n",NET_NSendPackets,SendTimer.ReadSec());
#endif
    return TRUE;
}

//==============================================================================

xbool net_socket::IsConnected( void )
{
    return (m_Socket != BAD_SOCKET);
}