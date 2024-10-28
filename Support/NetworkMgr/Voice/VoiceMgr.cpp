//==============================================================================
//
//  VoiceMgr.cpp
//
//==============================================================================
#include "x_types.hpp"

//==============================================================================
//  INCLUDES
//==============================================================================
#include "VoiceMgr.hpp"
#include "VoiceProxy.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/GameServer.hpp"

#include "Objects/actor/Actor.hpp"
#include "NetworkMgr/GameMgr.hpp"

#ifdef TARGET_PS2
#include "ps2/IopManager.hpp"
#endif

voice_mgr g_VoiceMgr;

//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
voice_mgr::voice_mgr( void )
{
    m_Initialized = FALSE;
    m_LocalMutedPlayers = 0;
    m_bGameIsVoiceEnabled = TRUE;
}

//==============================================================================
voice_mgr::~voice_mgr( void )
{
#ifndef TARGET_PC
    ASSERT(!m_Initialized);
#endif
}

//==============================================================================

void voice_mgr::Init( xbool LocalIsServer, xbool EnableHeadset )
#if defined ( TARGET_PC )
{
}
#endif
#if defined ( TARGET_XBOX )
{
    ASSERT(!m_Initialized);
    m_LocalIsServer         = LocalIsServer;
    m_Initialized           = TRUE;
    m_PlayerNetSlot         = -1;
    m_HeadsetEnabled        = EnableHeadset;
    m_LocalVoiceOwner       = -1;
    m_CurrentTalkType       = TALK_GLOBAL;
    m_LocalDesiredTalkMode  = TALK_NONE;

    m_LocalDirtyMutedPlayers = TRUE;

    // The headset is initialized ONCE at startup on XBox only!
    // All other platforms should initialize the headset here.
    #ifndef TARGET_XBOX
    m_Headset.Init( m_HeadsetEnabled );
    #endif

    m_MaxSpeakers = 1;

    for( s32 i = 0; i < NET_MAX_PLAYERS; i++ )
    {
        m_Speakers[ i ].PlayerNum       = i;
        m_Speakers[ i ].ActualTalkMode  = TALK_NOT_TALKING;
        m_Speakers[ i ].TalkTime        = 0.0f;

        // Set up the priority queue.
        m_SpeakerQueue[ i ] = m_Speakers + i;

        for( s32 j = 0; j < 4; j++ )
        {
            m_Listeners[ i ].PlayerNum = i;
            m_Listeners[ i ].ListeningTo[ j ] = -1; 
        }

        GameMgr.SetSpeaking( i, FALSE );
    }
}
#endif
//==============================================================================
void voice_mgr::Kill( void )
#if defined ( TARGET_PC )
{
}
#endif
#if defined ( TARGET_XBOX )
{
    ASSERT(m_Initialized);

    #ifndef TARGET_XBOX
    m_Headset.Kill();
    #endif
    m_HeadsetEnabled    = FALSE;
    m_Initialized       = FALSE;
}
#endif
//==============================================================================
void voice_mgr::Update( f32 DeltaTime )
#if defined ( TARGET_PC )
{
}
#endif
#if defined ( TARGET_XBOX )
{
    if( m_HeadsetEnabled )
    {
        m_Headset.Update( DeltaTime );
    }

    if( g_NetworkMgr.IsServer() )
    {
        DoArbitration( DeltaTime );
    }
}
#endif
//==============================================================================
s32 voice_mgr::GetLocalVoiceOwner( void )
{
    return m_LocalVoiceOwner;
}

//==============================================================================
void voice_mgr::SetVoiceOwner( s32 Listener, s32 Owner, actual_talk_mode TalkMode )
{
    ASSERT( g_NetworkMgr.IsServer() );

    s32 ClientIndex = g_NetworkMgr.GetClientIndex( Listener );

    // Is this a client we're trying to make listen?
    if( ClientIndex >= 0 )
    {
        game_server& Server = g_NetworkMgr.GetServerObject();

        if( Server.IsClientConnected( ClientIndex ) )
        {
            voice_proxy& Proxy = Server.GetVoiceProxy( ClientIndex );
            Proxy.SetVoiceOwner( Owner, TalkMode );
        }
    }

    // Must be the server we're trying to set.
    else
    {
        SetLocalVoiceOwner( Owner, TalkMode );
    }
}

//==============================================================================
void voice_mgr::SetLocalVoiceOwner( s32 Owner, actual_talk_mode TalkMode )
{
    // If there is a change of voice ownership from the current local player.
    if( m_LocalVoiceOwner != Owner )
    {
        if( Owner != -1 )
        {
            if( m_LocalVoiceOwner == -1 )
            {
                LOG_MESSAGE( "voice_mgr::SetLocalVoiceOwner", "Granted to player %d", Owner );
            }
            else
            {
                GameMgr.SetSpeaking( m_LocalVoiceOwner, FALSE );
                LOG_MESSAGE( "voice_mgr::SetLocalVoiceOwner", "Stolen from player %d, granted to player %d", m_LocalVoiceOwner, Owner );
            }
        }
        else
        {
            GameMgr.SetSpeaking( m_LocalVoiceOwner, FALSE );
            LOG_MESSAGE( "voice_mgr::SetLocalVoiceOwner", "Released from player %d", m_LocalVoiceOwner );

        }
        // If the new owner is the local player, then send the beep sound to the headset.
        if( Owner==g_NetworkMgr.GetLocalPlayerSlot(0) )
        {
            LOG_WARNING( "voice_mgr::SetLocalVoiceOwner","Start to talk BEEEP Sent to local headset" );
        }

        // If the old owner is the local player, then send the release sound to the headset.
        if( m_LocalVoiceOwner==g_NetworkMgr.GetLocalPlayerSlot(0) )
        {
            LOG_WARNING( "voice_mgr::SetLocalVoiceOwner","End of talk STATIC sent to local headset" );
        }
    }

    m_LocalVoiceOwner    = Owner;
    m_LocalVoiceTalkType = TalkMode;

    GameMgr.SetSpeaking( m_LocalVoiceOwner, TRUE );
}

//==============================================================================
// This will read a chunk of data from the 'write' fifo. This is because, on a
// client, the write fifo is just used to store voice data pending to go out
// to each client.
s32 voice_mgr::ReadFromVoiceFifo( byte* pBuffer, s32 MaxLength )
{
    return m_Headset.Read( pBuffer, MaxLength );
}

//==============================================================================
void voice_mgr::WriteToVoiceFifo( const byte* pBuffer, s32 Length )
{
    m_Headset.Write( pBuffer, Length );
}

//==============================================================================

s32 voice_mgr::GetBytesInWriteFifo( void )
{
    return( m_Headset.GetNumBytesInWriteFifo() );
}

//==============================================================================
void voice_mgr::SetTalking( xbool bTalking )
{
    #ifdef TARGET_XBOX
    {
        // If we are recording a voice attachment then don't transmit voice
        if( m_Headset.GetVoiceIsRecording() == TRUE )
        {
            m_LocalDesiredTalkMode = TALK_NONE;
            return;
        }
    }
    #endif

    desired_talk_mode OldMode = m_LocalDesiredTalkMode;
    if( bTalking && m_bGameIsVoiceEnabled )
    {
        m_LocalDesiredTalkMode = m_CurrentTalkType;
    }
    else
    {
        m_LocalDesiredTalkMode = TALK_NONE;
    }

    if( m_LocalDesiredTalkMode != OldMode )
    {
#if defined(X_DEBUG)
        LOG_MESSAGE( "voice_mgr::SetTalking", "Voice mode change. Old Mode:%s, New Mode:%s", 
            GetTalkModeName(OldMode), 
            GetTalkModeName(m_LocalDesiredTalkMode) );
#endif
        
        if( m_LocalDesiredTalkMode == TALK_NONE )
        {
            m_Headset.SetTalking( FALSE );
        }
        else
        {
            m_Headset.SetTalking( TRUE );
        }    
    }
}

//==============================================================================
xbool voice_mgr::IsValidTarget( s32 Speaker, s32 Listener, actual_talk_mode TalkMode )
{
    
    if( (g_NetworkMgr.GetClientIndex( Speaker  ) >= -1) &&
        (g_NetworkMgr.GetClientIndex( Listener ) >= -1) )
    {
        actor* pSpeaker  = (actor*)NetObjMgr.GetObjFromSlot( Speaker  );
        actor* pListener = (actor*)NetObjMgr.GetObjFromSlot( Listener );

        if( !pSpeaker || !pListener )
        {
            LOG_WARNING( "voice_mgr::IsValidTarget", "Bad Speaker (%d) or Listener (%d)", Speaker, Listener );
            return FALSE;
        }

        // Have to get the appropriate speaker bits for the server or client, then check and see if the listener has him muted.
        if( ( Listener == 0 ? m_LocalMutedPlayers : 
                              g_NetworkMgr.GetVoiceProxy( g_NetworkMgr.GetClientIndex( Listener ) ).GetMutedPlayers() ) & (1 << Speaker) )
        {
            return FALSE;
        }

        // Check if the speaker is voice banned
        if( GameMgr.GetScore().Player[ Speaker ].IsVoiceAllowed == FALSE )
            return( FALSE );

        // Check if the listener is voice banned
        if( GameMgr.GetScore().Player[ Listener ].IsVoiceAllowed == FALSE )
            return( FALSE );

        switch( TalkMode )
        {
        case TALK_NEW_GLOBAL:
        case TALK_POT_GLOBAL:
        case TALK_OLD_GLOBAL:
            return TRUE;

        case TALK_NEW_TEAM:
        case TALK_POT_TEAM:
        case TALK_OLD_TEAM:
            if( pSpeaker->net_GetTeamBits() & pListener->net_GetTeamBits() )
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        case TALK_NEW_LOCAL:
        case TALK_POT_LOCAL:
        case TALK_OLD_LOCAL:
            if( (pSpeaker->GetPosition() - pListener->GetPosition()).LengthSquared() < 2560000.0f )
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
           
        default:
            return FALSE;
            break;
        }
    }

    return FALSE;
}

//==============================================================================

xbool voice_mgr::IsSpeaking( s32 Speaker )
{
    switch( m_Speakers[ Speaker ].ActualTalkMode )
    {
    case TALK_NEW_TEAM:     
    case TALK_OLD_TEAM:     
    case TALK_NEW_LOCAL:    
    case TALK_OLD_LOCAL:    
    case TALK_NEW_GLOBAL:   
    case TALK_OLD_GLOBAL:
        return TRUE;
        break;
    default:
        return FALSE;
        break;
    }
}

//==============================================================================

void voice_mgr::AgeSpeaker( s32 Speaker, f32 DeltaTime )
{
    const f32 ProtectedTime = 5.0f;

    m_Speakers[ Speaker ].TalkTime += DeltaTime;
    if( m_Speakers[ Speaker ].TalkTime > ProtectedTime )
    {
        switch( m_Speakers[ Speaker ].ActualTalkMode )
        {
        case TALK_NEW_TEAM:     
            m_Speakers[ Speaker ].ActualTalkMode = TALK_OLD_TEAM;
            break;
        case TALK_NEW_LOCAL:    
            m_Speakers[ Speaker ].ActualTalkMode = TALK_OLD_LOCAL;
            break;
        case TALK_NEW_GLOBAL:   
            m_Speakers[ Speaker ].ActualTalkMode = TALK_OLD_GLOBAL;
            break;
        default:
            break;
        }
    }
}

//==============================================================================

desired_talk_mode voice_mgr::GetDesiredTalkMode( s32 PlayerIndex )
{
    s32 ClientIndex = g_NetworkMgr.GetClientIndex( PlayerIndex );

    if( ClientIndex >= 0 )
    {
        game_server& Server = g_NetworkMgr.GetServerObject();

        if( Server.IsClientConnected( ClientIndex ) )
        {
            voice_proxy& Proxy = Server.GetVoiceProxy( ClientIndex );
            return Proxy.GetDesiredTalkMode();
        }
        else
        {
            return TALK_NONE;
        }
    }
    else if( ClientIndex == -2 )
    {
        return TALK_NONE;
    }
    else
    {
        return GetLocalDesiredTalkMode();
    }
}

//==============================================================================

void voice_mgr::ToggleTalkMode( void )
{
    // We don't want them changing their talk type in the middle of talking!
    if( GetLocalDesiredTalkMode() == TALK_NONE )
    {
        if( g_NetworkMgr.IsOnline() )
        {
            const game_score& ScoreData = GameMgr.GetScore();

            switch( m_CurrentTalkType )
            {
            case TALK_GLOBAL:
                m_CurrentTalkType = TALK_LOCAL;
                break;
            case TALK_LOCAL:
                m_CurrentTalkType = ScoreData.IsTeamBased ? TALK_TEAM : TALK_GLOBAL;
                break;
            case TALK_TEAM:
                m_CurrentTalkType = TALK_GLOBAL;
                break;
            default:
                break;
            }
        }
    }
}

//==============================================================================

void voice_mgr::DoArbitration( f32 DeltaTime )
{
    s32 CurrentMax = 1;

    // First, create an array to hold how many people 
    // each person is currently listening to.

    if( m_Headset.IsEnabled() == FALSE )
    {
        return;
    }
    s32 NumSpeakers          [ NET_MAX_PLAYERS ];
    u32 WantedListeners      [ NET_MAX_PLAYERS ];
    s32 NumWantedListeners   [ NET_MAX_PLAYERS ];

    u32 PotentialListeners   [ NET_MAX_PLAYERS ];
    s32 NumPotentialListeners[ NET_MAX_PLAYERS ];

    // Clear everything out.
    for( s32 i = 0; i < NET_MAX_PLAYERS; i++ )
    {
        NumSpeakers             [ i ] = 0;

        PotentialListeners      [ i ] = 0;
        NumPotentialListeners   [ i ] = 0;

        WantedListeners         [ i ] = 0;
        NumWantedListeners      [ i ] = 0;
    
        SetVoiceOwner( i, -1, TALK_NOT_TALKING );

        if( IsSpeaking( i ) )
        {
            AgeSpeaker( i, DeltaTime );
        }
    }

    //
    // For the people who desire to speak, but aren't, set them to potential talkers.
    //
    {
        for( s32 i = 0; i < NET_MAX_PLAYERS; i++ )
        {
            desired_talk_mode DesiredTalkMode = GetDesiredTalkMode( i );
            if( DesiredTalkMode != TALK_NONE )
            {
                // If they're not speaking now, but they desire to be,
                // set them as potential speakers.
                if( m_Speakers[ i ].ActualTalkMode == TALK_NOT_TALKING )
                {
                    switch( DesiredTalkMode )
                    {
                    case TALK_GLOBAL:
                        m_Speakers[ i ].ActualTalkMode = TALK_POT_GLOBAL;
                        break;
                    case TALK_TEAM:
                        m_Speakers[ i ].ActualTalkMode = TALK_POT_TEAM;
                        break;
                    case TALK_LOCAL:
                        m_Speakers[ i ].ActualTalkMode = TALK_POT_LOCAL;
                        break;
                    default:
                        break; 
                    }
                }
            }

            // If they don't desire to speak, 
            // we certainly aren't going to make them.
            else
            {
                if( IsSpeaking( i ) )
                {
                    // They willingly gave up their speaking rights,
                    // but they're still going to the end of the queue.
                    m_Speakers[ i ].MoveToEndOfQueue = TRUE;
                }

                m_Speakers[ i ].TalkTime = 0.0f;
                m_Speakers[ i ].ActualTalkMode = TALK_NOT_TALKING;
            }
        }
    }

    //
    // Go through and set up the speaker bitfields with 
    // the desired recipients.
    //
    for( s32 i = 0; i < NET_MAX_PLAYERS; i++ )
    {
        if( m_Speakers[ i ].ActualTalkMode != TALK_NOT_TALKING )
        {
            for( s32 j = 0; j < NET_MAX_PLAYERS; j++ )
            {
                if( IsValidTarget( i, j, m_Speakers[ i ].ActualTalkMode ) )
                {
                    // Turn on the bit, this speaker wants to talk to this player.
                    WantedListeners[ i ] |= (1 << j);
                    NumWantedListeners[ i ]++;
                }
            }
        }
    }

    //
    // We know who wants to talk to whom, 
    // now figure out who can talk to whom.
    // Priority is determined by talk mode, then priority queue.
    // 
    for( s32 TalkMode = TALK_MODE_FIRST; TalkMode <= TALK_MODE_LAST; TalkMode++ )
    {
        // This is defined so we don't cover the same entry twice
        // when the queue is rearranged.
        s32 Top = NET_MAX_PLAYERS;

        // Tie breaks are by priority queue.
        for( s32 iSpeaker = 0; iSpeaker < Top; iSpeaker++ )
        {
            speaker& Speaker = *m_SpeakerQueue[ iSpeaker ];
            if( (Speaker.ActualTalkMode == TalkMode) )
            {
                // Go through the listener bitfield here and look for listeners
                // who aren't listening to anyone else, and that we also decided
                // we wanted to talk to above.
                for( s32 ListenerNum = 0; ListenerNum < NET_MAX_PLAYERS; ListenerNum++ )
                {
                    if( (NumSpeakers[ ListenerNum ] < m_MaxSpeakers) &&
                        (WantedListeners[ Speaker.PlayerNum ] & (1 << ListenerNum)) )
                    {
                        if( NumSpeakers[ ListenerNum ] < CurrentMax )
                        {
                            NumPotentialListeners[ Speaker.PlayerNum ]++;
                            PotentialListeners[ Speaker.PlayerNum ] |= (1 << ListenerNum);
                        }
                    }
                }

                // Now check to see that we can talk to at least half the
                // people we wanted to originally.  Also, you need to be 
                // able to talk to yourself, but not just yourself, cause
                // that's a sure sign of madness.
                if( (NumPotentialListeners[ Speaker.PlayerNum ] > (NumWantedListeners[ Speaker.PlayerNum ] / 2)) &&
                    (NumPotentialListeners[ Speaker.PlayerNum ] > 1) &&
                    (PotentialListeners[ Speaker.PlayerNum ] & (1 << Speaker.PlayerNum)) )
                {
                    // If this is his first frame talking, upgrade him from a potential talker.
                    switch( Speaker.ActualTalkMode )
                    {
                    case TALK_POT_GLOBAL:
                        Speaker.ActualTalkMode = TALK_NEW_GLOBAL;
                        break;
                    case TALK_POT_LOCAL:
                        Speaker.ActualTalkMode = TALK_NEW_LOCAL;
                        break;
                    case TALK_POT_TEAM:
                        Speaker.ActualTalkMode = TALK_NEW_TEAM;
                        break;
                    default:
                        break;
                    }
                                                
                    for( s32 ListenerNum = 0; ListenerNum < NET_MAX_PLAYERS; ListenerNum++ )
                    {
                        if( (PotentialListeners[ Speaker.PlayerNum ] & (1 << ListenerNum)) )
                        {
                            NumSpeakers[ ListenerNum ]++;
                            SetVoiceOwner( ListenerNum, Speaker.PlayerNum, m_Speakers[ Speaker.PlayerNum ].ActualTalkMode );
                        }
                    }
                }

                // Guess you don't get to speak this time, better luck next frame.
                else
                {
                    // If he still desires to speak, it will get sorted out at the beginning
                    // of the next DoArbitration, but he'll have to start from scratch.
                    
                    if( IsSpeaking( Speaker.PlayerNum ) )
                    {
                        // Move them to the end of the queue.
                        Speaker.MoveToEndOfQueue = TRUE;
                    }

                    Speaker.TalkTime = 0.0f;
                    Speaker.ActualTalkMode = TALK_NOT_TALKING;
                }
            }
        }
    }

    // Move people who finished speaking 
    // this frame to the end of the priority queue.
    s32 NumMoves = 0;
    for( s32 i = 0; i + NumMoves < NET_MAX_PLAYERS; )
    {
        if( m_SpeakerQueue[ i ]->MoveToEndOfQueue )
        {
            // Put this entry at the end and shift everything else forward one.
            speaker* pTempSpeaker = m_SpeakerQueue[ i ];

            for( s32 j = i; j < (NET_MAX_PLAYERS - 1); j++ )
            {
                m_SpeakerQueue[ j ] = m_SpeakerQueue[ j + 1 ];
            }

            pTempSpeaker->MoveToEndOfQueue = FALSE;
            m_SpeakerQueue[ NET_MAX_PLAYERS - 1 ] = pTempSpeaker;
            NumMoves++;
        }
        else
        {
            i++;
        }
    }
}

//==============================================================================

void voice_mgr::Distribute( s32 TalkerIndex, const byte* pBuffer, s32 Length, s32 Mode )
{
    s32     PlayerIndex;
    s32     ClientIndex;
    (void)TalkerIndex;

    (void)Mode;
    ASSERT( g_NetworkMgr.IsServer() );
    for( PlayerIndex=0; PlayerIndex < NET_MAX_PLAYERS; PlayerIndex++ )
    {
        ClientIndex = g_NetworkMgr.GetClientIndex( PlayerIndex );
        if( ClientIndex >= 0 )
        {
            voice_proxy& Proxy = g_NetworkMgr.GetVoiceProxy( ClientIndex );

            if( (Proxy.GetVoiceOwner() == TalkerIndex) && (TalkerIndex != PlayerIndex) )
            {
                LOG_MESSAGE( "voice_mgr::Distribute","Voice data sent. Player:%d, Client:%d, Length:%d", TalkerIndex, ClientIndex, Length );
                Proxy.Write( pBuffer, Length );
            }
        }
    }
    // If the 'local' voice channel is owned by this player, then send the data to the local headset.
    if( (GetLocalVoiceOwner() == TalkerIndex) && (g_NetworkMgr.GetLocalPlayerSlot(0) != TalkerIndex) )
    {
        LOG_MESSAGE( "voice_mgr::Distribute","Voice data queued locally. Player:%d, Length:%d", TalkerIndex, Length );
        WriteToVoiceFifo( pBuffer, Length );
    }
}

//==============================================================================
// The format of the data sent out here should mirror the voice_proxy::AcceptUpdate
// function. This can be used by any client to send to the server.
//
void voice_mgr::ProvideUpdate( netstream& BitStream )
{
    ASSERT( g_NetworkMgr.IsClient() );

    // Update the voice peripheral status
    BitStream.WriteFlag( !IsVoiceBanned()  );
    BitStream.WriteFlag(  IsVoiceCapable() );

    if( BitStream.WriteFlag( m_LocalDirtyMutedPlayers ) )
    {
        BitStream.WriteU32( m_LocalMutedPlayers );
        m_LocalDirtyMutedPlayers = FALSE;
    }

    // If we are the owner, or we want to be the owner, then we send an update
    // otherwise, we send nothing (except a flag saying there is no data).
    if( m_LocalDesiredTalkMode != TALK_NONE )
    {
        if( GetLocalVoiceOwner() == g_NetworkMgr.GetLocalPlayerSlot( 0 ) )
        {
#if defined(X_DEBUG)
            LOG_MESSAGE( "voice_mgr::ProvideUpdate", "Requesting voice ownership. Talk Mode:%s", GetTalkModeName(m_LocalDesiredTalkMode) );
#endif
            // Write a flag to say data is present.
            BitStream.WriteRangedS32( m_LocalDesiredTalkMode, TALK_MODE_FIRST, TALK_MODE_LAST );
            m_Headset.ProvideUpdate( BitStream, 128 );
        }
        else
        {
            // Release the headset. We do this, at the moment, by sending no data for
            // up to a half second. We will change this so that we send a player
            // slot of -1, until the network manager confirms someone else owns the
            // voice.
            BitStream.WriteRangedS32( m_LocalDesiredTalkMode, TALK_MODE_FIRST, TALK_MODE_LAST );
            m_Headset.ProvideUpdate( BitStream, 0 );
        }
    }
    else
    {
        //
        // No one from this machine is interested in speaking!
        //
        BitStream.WriteRangedS32( TALK_NONE, TALK_MODE_FIRST, TALK_MODE_LAST );
#if defined(TARGET_XBOX)
        // Now tell the bitstream that it's only got a byte in it.
        s32 Cursor = BitStream.GetCursor();
        s32 BytesUsed = (Cursor+7) >> 3;
        BitStream.SetCursor( 0 );
        BitStream.WriteU32( BytesUsed-2, 8 );
        BitStream.WriteU32( 0, 8 );
        BitStream.SetCursor( Cursor );
#endif
    }
}

//==============================================================================
// Data came in from the server. This tells us who actually has control of the
// headset. It *may* be us! The format of this data should mirror 
// voice_proxy::ProvideUpdate()
//
void voice_mgr::AcceptUpdate( netstream& BitStream )
{
    xbool   DataPresent;
    s32     Owner;
    s32     TalkMode;

    ASSERT( g_NetworkMgr.IsClient() );
    
    DataPresent = BitStream.ReadFlag();
    //
    // We have the data coming in from a bitstream. This is only called on a client
    // and means that the server has sent some data to it. This is responsible for
    // distributing this data to the headset. Since this is the client, they should
    // play anything that's sent down this channel since the server just won't send
    // any useful data to the client that is talking.
    //
    if( !DataPresent )
    {
        //
        // Clear out the voice owner.
        //
        if( GetLocalVoiceOwner() != -1 )
        {
            LOG_MESSAGE( "voice_mgr::AcceptUpdate", "Released voice. Old speaker:%d", GetLocalVoiceOwner() );
        }
        SetLocalVoiceOwner( -1, TALK_NOT_TALKING );
        return;
    }


    BitStream.ReadRangedS32( Owner, 0, NET_MAX_PLAYERS );
    BitStream.ReadRangedS32( TalkMode, TALK_MODE_FIRST, TALK_MODE_LAST );

    if( GetLocalVoiceOwner() != Owner )
    {
        LOG_MESSAGE( "voice_mgr::AcceptUpdate", "New voice owner, old speaker:%d, new speaker:%d", GetLocalVoiceOwner(), Owner );
        SetLocalVoiceOwner( Owner, (actual_talk_mode)TalkMode );
    }
    m_Headset.AcceptUpdate( BitStream );
}

//==============================================================================

headset& voice_mgr::GetHeadset( void )
{
    return( m_Headset );
}

//==============================================================================

xbool voice_mgr::IsVoiceCapable( void )
{
    xbool   IsCapable = IsHeadsetPresent() & IsVoiceEnabled();
    return( IsCapable );
}

//==============================================================================

#if defined(X_DEBUG)        
const char* GetTalkModeName( s32 Mode )
{
    switch(Mode)
    {
    case TALK_NONE:             return "TALK_NONE";
    case TALK_GLOBAL:           return "TALK_GLOBAL";
    case TALK_LOCAL:            return "TALK_LOCAL";
    case TALK_TEAM:             return "TALK_TEAM";
    default:                    ASSERT(FALSE);
    }
    return "<unknown>";
}
#endif

#if defined(X_DEBUG) && (defined(bwatson) || defined(jpcossigny) || defined(Biscuit))
#if defined(TARGET_PS2)
#include "Audio/Hardware/audio_hardware_ps2_private.hpp"
void VoiceTestCode( void )
{
    g_NetworkMgr.SetOnline( TRUE );
    g_VoiceMgr.Init( TRUE, TRUE );
    s_ChannelManager.SetPCMVolume( 1.0f );
    g_VoiceMgr.SetVoiceThruSpeakers( TRUE );
    g_VoiceMgr.SetLoopback( TRUE );
    g_VoiceMgr.SetTalking( TRUE );
    while( TRUE )
    {
        g_VoiceMgr.Update( 1.0f/32.0f );
        x_DelayThread( 32 );
    }
    g_VoiceMgr.Kill();
}
#endif

#if defined(TARGET_PC)
void VoiceTestCode( void )
{
}
#endif

//=============================================================================
#if defined(TARGET_XBOX)
//=============================================================================

void PrintProgress( xbool IsMic, f32 T )
{
    xstring String;

    if( IsMic == TRUE )
        String = "Read : <               >\n";
    else
        String = "Play : <               >\n";

    s32 Index = 7 + (s32)(15.0f * T);

    String[ Index ] = 'X';

    x_DebugMsg( String );
}

//=============================================================================

void VoiceTestCode( void )
#if defined ( TARGET_PC )
{
}
#endif
#if defined ( TARGET_XBOX )
{
    g_NetworkMgr.SetOnline( TRUE );
    g_VoiceMgr.Init( TRUE, TRUE );

    // We must delay for a while until the xbox detects a headset is connected!
    for( s32 i=0; i < 32; i++ )
    {
        g_VoiceMgr.Update( 1.0f / 32.0f );
        x_DelayThread( 32 );
    }

    // Headset should now be registered, so we can enable talking
    g_VoiceMgr.SetTalking( TRUE );
    g_VoiceMgr.SetVolume( 1.0f, 1.0f );

    //g_VoiceMgr.SetVoiceThruSpeakers( TRUE );

    headset& Headset = g_VoiceMgr.GetHeadset();

    xbool IsInitialized = FALSE;
    xbool IsRecording   = TRUE;

    while( 1 )
    {
        g_VoiceMgr.Update( 1.0f / 32.0f );

        if( 0 )
        {
            //
            // Loopback Test
            //

            byte Buffer[ 256 ];
            s32 nBytes = g_VoiceMgr.ReadFromVoiceFifo( Buffer, sizeof( Buffer ) );
            if( nBytes > 0 )
                g_VoiceMgr.WriteToVoiceFifo( Buffer, nBytes );
        }
        else
        {
            //
            // Voice Message Recording/Playback Test
            //

            if( IsRecording == TRUE )
            {
                if( IsInitialized == FALSE )
                {
                    IsInitialized = TRUE;
                    Headset.InitVoiceRecording();
                    Headset.StartVoiceRecording();
                }

                // Wait until recording has finished
                if( Headset.GetVoiceIsRecording() == FALSE )
                {
                    // Switch to playback mode
                    IsRecording = FALSE;

                    s32     NumBytes      = Headset.GetVoiceNumBytesRec();
                    s32     DurationMS    = Headset.GetVoiceDurationMS();
                    byte*   pVoiceMessage = Headset.GetVoiceMessageRec();
                    ASSERT( pVoiceMessage != NULL );

                    Headset.StartVoicePlaying( pVoiceMessage, DurationMS, NumBytes );

                    X_FILE* pFile = x_fopen( "\\Audio.bin", "wb" );
                    ASSERT( pFile != NULL );
                    x_fwrite( pVoiceMessage, 1, NumBytes, pFile );
                    x_fclose( pFile );
                }
                else
                {
                    PrintProgress( TRUE, Headset.GetVoiceRecordingProgress() );
                }
            }
            else
            {
                // Wait until voice has finished playing
                if( Headset.GetVoiceIsPlaying() == FALSE )
                {
                    IsInitialized = FALSE;
                    IsRecording   = TRUE;

                    // Finished with the voice message so destroy it
                    Headset.KillVoiceRecording();
                }
                else
                {
                    PrintProgress( FALSE, Headset.GetVoicePlayingProgress() );
                }
            }
        }

        x_DelayThread( 32 );
    }

    Headset.KillVoiceRecording();

    g_VoiceMgr.Kill();
}
#endif
//=============================================================================
#endif
//=============================================================================

#endif
