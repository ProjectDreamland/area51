//==============================================================================
// CONVERSATION_MANAGER.CPP
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "ConversationMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Audio\audio_stream_controller.hpp"

audio_stream_controller g_ConversationStreamControl;

//==============================================================================
// EXTERNALS
//==============================================================================
conversation_mgr    g_ConverseMgr;

//==============================================================================

conversation_mgr::conversation_mgr    ( void )
{
}

//==============================================================================

conversation_mgr::~conversation_mgr   ( void )
{
}

//==============================================================================

void conversation_mgr::Init ( void )
{
    g_ConversationStreamControl.ReserveStreams( 2 );
}

//==============================================================================

void conversation_mgr::Kill ( void )
{
    g_ConversationStreamControl.ReserveStreams( 0 );
}


//==============================================================================
/*
void conversation_mgr::Reset( void )
{
}
*/
//==============================================================================

xbool conversation_mgr::HasDialog( const char* pDescriptor )
{
    return g_AudioMgr.IsValidDescriptor(pDescriptor);
}

//==============================================================================

xbool conversation_mgr::HasDialog( const char* pObjectName, const char* pAction )
{
    char DescName[MAX_DESCRIPTOR_NAME];
    x_sprintf( DescName, "%s_%s", pObjectName, pAction );
    return g_AudioMgr.IsValidDescriptor(DescName);
}

//==============================================================================

u32 conversation_mgr::PlayStream( const char* pObjectName, const char* pAction, guid Guid,
                             s16 ZoneID, const vector3& Position, f32 MaxInQueueTime, xbool AutoStart, u8 Flags )
{
    (void)Guid;
    (void)ZoneID;
    (void)MaxInQueueTime;
    ASSERT( pObjectName );
    ASSERT( pAction );
    if( (!pObjectName[0]) || (!pAction[0]) )
        return 0;

    char Descriptor[MAX_DESCRIPTOR_NAME];
    x_sprintf( Descriptor, "%s_%s", pObjectName, pAction );

    return PlayStream( Descriptor, Position, Guid, ZoneID, MaxInQueueTime, AutoStart, Flags );
}

//==============================================================================

u32 conversation_mgr::PlayStream( const char* pDescriptor, const vector3& Position, guid Guid, s16 ZoneID, 
                                  f32 MaxInQueueTime, xbool AutoStart, u8 Flags )
{
    (void)Guid;
    (void)ZoneID;
    (void)MaxInQueueTime;
    ASSERT( pDescriptor );
    if( !pDescriptor[0] )
        return 0;
    
    u32 Result = 0;

    if( Flags & PLAY_2D )
        Result = g_ConversationStreamControl.Play( pDescriptor, AutoStart );
    else
        Result = g_ConversationStreamControl.Play( pDescriptor, Position, ZoneID, AutoStart );

    return Result;
}

//==============================================================================

u32 conversation_mgr::PlayHotVoice( const char* pObjectName, const char* pAction,
                                    const vector3& Position, s16 ZoneID, f32 Pitch, xbool AutoStart, u8 Flags )
{
    char Descriptor[MAX_DESCRIPTOR_NAME];
    x_sprintf( Descriptor, "%s_%s", pObjectName, pAction );
    
    return PlayHotVoice( Descriptor, Position, ZoneID, Pitch, AutoStart, Flags );
}

//==============================================================================

u32 conversation_mgr::PlayHotVoice( const char* pDescriptor, const vector3& Position, s16 ZoneID, f32 Pitch, 
                                    xbool AutoStart, u8 Flags )
{
    voice_id Result;

    (void)ZoneID;

    // Check if we are going 2D.
    if( Flags & PLAY_2D )
    {
        Result = g_AudioMgr.Play( pDescriptor, AutoStart );
    }
    else
    {
        Result = g_AudioMgr.PlayVolumeClipped( pDescriptor, Position, ZoneID, AutoStart );
    }

    g_AudioMgr.SetPitch( Result, Pitch );
        
    return Result;
}

//==============================================================================

u32 conversation_mgr::PlayHotVoice( const char* pDescriptor, const vector3& Position, s16 ZoneID, xbool AutoStart, u8 Flags )
{
    return PlayHotVoice( pDescriptor, Position, ZoneID, 1.0f, AutoStart, Flags );
}

//==============================================================================

xbool conversation_mgr::IsActive( u32 VoiceID )
{
    return g_AudioMgr.IsValidVoiceId( VoiceID );
}

//==============================================================================

void conversation_mgr::StartSound( u32 VoiceID )
{
    g_AudioMgr.Start( VoiceID );
}

//==============================================================================

void conversation_mgr::SetPosition( u32 VoiceID, vector3& Pos, s16 ZoneID )
{
    g_AudioMgr.SetPosition( VoiceID, Pos, ZoneID );
}

//==============================================================================

void conversation_mgr::SetFalloff( u32 VoiceID, f32 NearScale, f32 FarScale )
{
    g_AudioMgr.SetFalloff( VoiceID, NearScale, FarScale );
}

//==============================================================================

f32 conversation_mgr::GetVolume( u32 VoiceID )
{
    return g_AudioMgr.GetVolume( VoiceID );
}

//==============================================================================

void conversation_mgr::SetVolume( u32 VoiceID, f32 Volume )
{
    g_AudioMgr.SetVolume( VoiceID, Volume);
}

//==============================================================================

xbool conversation_mgr::IsPlaying( u32 VoiceID )
{
    return g_AudioMgr.IsValidVoiceId( VoiceID );
}

//==============================================================================

xbool conversation_mgr::IsReadyToPlay( u32 VoiceID )
{
    return g_AudioMgr.IsVoiceReady( VoiceID );
}

//==============================================================================

void conversation_mgr::Stop ( u32 VoiceID, f32 ReleaseTime )
{
    g_AudioMgr.Release( VoiceID, ReleaseTime );
}

//==============================================================================

void conversation_mgr::Update ( f32 DeltaTime )
{
    (void)DeltaTime;
}
