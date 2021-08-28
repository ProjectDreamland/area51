//==============================================================================
//
//  VoiceProxy.cpp
//
// This contains a local instance, on a server, of the data that is needed to
// be sent to that client. The server will issue data to this fifo, this will then
// be output on the next available update cycle.
//
//==============================================================================
#include "x_types.hpp"

//==============================================================================
//  INCLUDES
//==============================================================================
#include "VoiceProxy.hpp"
#include "Network/NetStream.hpp"

#ifdef TARGET_PS2
#include "ps2/IopManager.hpp"
#endif

#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/NetLimits.hpp"
#include "NetworkMgr/GameMgr.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
voice_proxy::voice_proxy( void )
{
    m_Initialized = FALSE;
}

//==============================================================================
voice_proxy::~voice_proxy( void )
{
    ASSERT(!m_Initialized);
}

//==============================================================================
void voice_proxy::Init( s32 PlayerNetSlot )
{
    ASSERT(!m_Initialized);
    m_Initialized           = TRUE;
    m_PlayerNetSlot         = PlayerNetSlot;
    m_DesiredTalkMode       = TALK_NONE;
    m_VoiceOwner            = -1;
    m_MutedPlayers          = 0;
    m_Outgoing.Init( m_OutgoingBuffer, 256 );
    m_Incoming.Init( m_IncomingBuffer, 256 );
}

//==============================================================================
void voice_proxy::Kill( void )
{
    ASSERT(m_Initialized);
    m_Initialized = FALSE;
    m_Outgoing.Kill();
    m_Incoming.Kill();
}

//==============================================================================
// This bitstream format should mirror voice_mgr::AcceptUpdate(). This goes from
// the server to the client. The client needs to know if someone is talking, and
// if so, whom.
void voice_proxy::ProvideUpdate( netstream& BitStream )
{
    xbool DataPresent;

    ASSERT( g_NetworkMgr.IsServer() );

    DataPresent = (m_VoiceOwner != -1);
    // If there is some data waiting to go from the server to the client, then
    // send it. Each client has an independent voice owner.
    BitStream.WriteFlag( DataPresent );
    if( DataPresent )
    {
#if defined(X_DEBUG)
        s32 OriginalLength;
        s32 ClientIndex;
        OriginalLength = m_Outgoing.GetBytesUsed();
        ClientIndex = g_NetworkMgr.GetClientIndex( m_PlayerNetSlot );

#endif
        BitStream.WriteRangedS32( m_VoiceOwner, 0, NET_MAX_PLAYERS );
        BitStream.WriteRangedS32( m_VoiceTalkMode, TALK_MODE_FIRST, TALK_MODE_LAST );

        // If this client's player is the voice owner, we really shouldn't pull any data
        // from it's outgoing buffer. However, in most cases, this should be empty anyway
        // so sending this data will do no harm.
        m_Outgoing.ProvideUpdate( BitStream, 128, g_VoiceMgr.GetEncodeBlockSize() );
#if defined(X_DEBUG)
        LOG_MESSAGE( "voice_proxy::ProvideUpdate", "Sending voice data to client:%d, Owner:%d, Length:%d", ClientIndex, m_VoiceOwner, OriginalLength - m_Outgoing.GetBytesUsed() );
#endif
    }
    else
    {
#if defined(TARGET_XBOX)
        // hackity hack hack hack!
        // This will align the bitstream to a byte boundary.
        s32 BitPos = BitStream.GetCursor() % 8;
        if( BitPos )
        {
            BitStream.WriteU32( 0, 8-BitPos );
        }
        s32 Cursor = BitStream.GetCursor();
        ASSERT( (Cursor & 0x07)==0 );
        BitStream.SetCursor(0);
        BitStream.WriteU32( 1, 8 );
        BitStream.WriteU32( 0, 8 );
        BitStream.SetCursor( Cursor );
#endif
    }
}

//==============================================================================
// Data has been received from the client. This function will strip out any
// voice data that may be in the bitstream. It will also attempt to acquire
// control of a voice channel.
//
// The server can determine who wants to have charge of the headset from the client
// ID that it comes from. This bitstream format should mirror voice_mgr::ProvideUpdate()
//
void voice_proxy::AcceptUpdate( netstream& BitStream )
{
    desired_talk_mode   DesiredTalkMode;
    byte                Buffer[512];
    s32                 Length;

    ASSERT( g_NetworkMgr.IsServer() );

    // Determine the voice status of the client
    {
        xbool IsVoiceAllowed;
        xbool IsVoiceCapable;
        BitStream.ReadFlag( IsVoiceAllowed );
        BitStream.ReadFlag( IsVoiceCapable );

        GameMgr.SetVoiceAllowed( m_PlayerNetSlot, IsVoiceAllowed );
        GameMgr.SetVoiceCapable( m_PlayerNetSlot, IsVoiceCapable );
    }

    if( BitStream.ReadFlag() )
    {
        BitStream.ReadU32( m_MutedPlayers );
    }

    BitStream.ReadRangedS32( (s32&)DesiredTalkMode, TALK_MODE_FIRST, TALK_MODE_LAST );

#if defined(X_DEBUG)
    if( m_DesiredTalkMode != DesiredTalkMode )
    {
        LOG_MESSAGE( "voice_proxy::AcceptUpdate","Talk mode change. Old:%s, New:%s", GetTalkModeName(m_DesiredTalkMode), GetTalkModeName(DesiredTalkMode) );
    }
#endif
    m_DesiredTalkMode = DesiredTalkMode;

/*-----
    g_VoiceMgr.Arbitrate( m_PlayerNetSlot, m_TalkMode );
    This is now to be done in the DoArbitrate
    // If there is no data, just bail now.
    if( m_TalkMode == TALK_NONE )
    {
        if( m_VoiceOwner == m_PlayerNetSlot )
        {
            m_VoiceOwner = -1;
        }
        return;
    }

    *** INCORRECT*** m_VoiceOwner = m_PlayerNetSlot;
-----*/

    if( m_DesiredTalkMode != TALK_NONE )
    {
        m_Incoming.AcceptUpdate( BitStream, g_VoiceMgr.GetEncodeBlockSize() );

        Length = Read( Buffer, sizeof(Buffer) );
        g_VoiceMgr.Distribute( m_PlayerNetSlot, Buffer, Length, m_DesiredTalkMode );
    }
}

//==============================================================================
// This will read a chunk of data from the incoming data fifo. This fifo will be
// filled by AcceptUpdate above when called on a client instance.
s32 voice_proxy::Read( byte* pBuffer, s32 MaxLength )
{
    s32 Length;

    Length = MIN(MaxLength,m_Incoming.GetBytesUsed());
    m_Incoming.Remove( pBuffer, Length, g_VoiceMgr.GetEncodeBlockSize() );
    return Length;
}

//==============================================================================
// This will write a block of data to the outgoing fifo. This will be consumed by
// ProvideUpdate when called from a client instance.
void voice_proxy::Write( const byte* pBuffer, s32 Length )
{
    m_Outgoing.Insert( pBuffer, Length, g_VoiceMgr.GetEncodeBlockSize() );
}

//==============================================================================
void voice_proxy::SetPlayerSlot( s32 PlayerSlot )
{
    m_PlayerNetSlot = PlayerSlot;
}

//==============================================================================
s32 voice_proxy::GetPlayerSlot( void )
{
    return m_PlayerNetSlot;
}

//==============================================================================
void voice_proxy::SetVoiceOwner( s32 PlayerSlot, actual_talk_mode TalkMode )
{
    m_VoiceOwner      = PlayerSlot;
    m_VoiceTalkMode   = TalkMode;
}

//==============================================================================
s32 voice_proxy::GetVoiceOwner( void )
{
    return m_VoiceOwner;
}