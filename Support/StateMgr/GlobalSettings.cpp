//==============================================================================
//  
//  GlobalSettings.cpp
//  
//==============================================================================

#include "GlobalSettings.hpp"
#include "Configuration/GameConfig.hpp"
#include "AudioMgr/AudioMgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"
#include "StateMgr.hpp"
#include "NetworkMgr/Voice/VoiceMgr.hpp"
#include "StringMgr\StringMgr.hpp"

const s32 DEFAULT_CHECKSUM = 0x4afb0001;

#if defined(TARGET_DEV)

#if defined(bwatson)
const char*     DEFAULT_NAME="DEV.BISCUIT";
#elif defined(cgalley)
const char*     DEFAULT_NAME="DEV.CRAIG";
#elif defined(mtraub)
const char*     DEFAULT_NAME="DEV.MICHAEL";
#elif defined(rbrannon)
const char*     DEFAULT_NAME="DEV.ROB";
#elif defined(athyssen)
const char*     DEFAULT_NAME="DEV.ANDY";
#elif defined(sbroumley)
const char*     DEFAULT_NAME="DEV.STEVE";
#elif defined(mbillington)
const char*     DEFAULT_NAME="DEV.MARKB";
#elif defined(aharp)
const char*     DEFAULT_NAME="DEV.AHARP";
#elif defined(mreed)
const char*     DEFAULT_NAME="DEV.MREED";
#elif defined(jpcossigny)
const char*     DEFAULT_NAME="DEV.JP";
#else
const char*     DEFAULT_NAME="DEV.Undefined";
#endif

#else

#ifdef LAN_PARTY_BUILD
const char*     DEFAULT_NAME="A51 LAN Party";
#else
const char*     DEFAULT_NAME="SERVER NAME";
#endif

#endif

//==============================================================================
global_settings::global_settings( void )
{
    Reset( RESET_ALL );
}

//==============================================================================
global_settings::~global_settings( void )
{
    x_memset( this, -1, sizeof(this) );
}

//==============================================================================
void global_settings::Reset( s32 ResetFlags )
{
    s32 i;

    if( ResetFlags == RESET_ALL )
    {
        x_memset( this, 0, sizeof(this) );

        m_Checksum  = DEFAULT_CHECKSUM;

        // initialize game configs
        m_MultiplayerSettings.m_GameTypeID                  = GAME_DM;
        m_MultiplayerSettings.m_ScoreLimit                  = -1;
        m_MultiplayerSettings.m_TimeLimit                   = -1;
        m_MultiplayerSettings.m_MutationMode                = MUTATE_CHANGE_AT_WILL;
        m_MultiplayerSettings.m_MapSettings.m_bUseDefault   = TRUE;
        m_MultiplayerSettings.m_MapSettings.m_MapCycleCount = 0;
        m_MultiplayerSettings.m_MapSettings.m_MapCycleIdx   = 0;
        for( i=0; i<MAP_CYCLE_SIZE; i++ )
        {
            m_MultiplayerSettings.m_MapSettings.m_MapCycle[i]    = -1;
        }

#if defined(TARGET_DEV)
        x_mstrcpy( m_HostSettings.m_ServerName, DEFAULT_NAME );
#else
        x_wstrcpy( m_HostSettings.m_ServerName, g_StringTableMgr( "ui", "IDS_HOST_SERVER_NAME" ) );
#endif
        x_strcpy ( m_HostSettings.m_Password,   "" );

        m_HostSettings.m_GameTypeID                     = GAME_DM;
        m_HostSettings.m_ScoreLimit                     = -1;
#ifdef LAN_PARTY_BUILD
        m_HostSettings.m_TimeLimit                      = 600;
        m_HostSettings.m_MaxPlayers                     = 8;
#else
        m_HostSettings.m_TimeLimit                      = -1;
        m_HostSettings.m_MaxPlayers                     = 16;
#endif
        m_HostSettings.m_VotePassPct                    = 50;
        m_HostSettings.m_Flags                          = SERVER_ENABLE_MAP_SCALING | SERVER_VOICE_ENABLED;
        m_HostSettings.m_MutationMode                   = MUTATE_CHANGE_AT_WILL;
        m_HostSettings.m_MapSettings.m_bUseDefault      = TRUE;
        m_HostSettings.m_MapSettings.m_MapCycleCount    = 0;
        m_HostSettings.m_MapSettings.m_MapCycleIdx      = 0;
        for( i=0; i<MAP_CYCLE_SIZE; i++ )
        {
            m_HostSettings.m_MapSettings.m_MapCycle[i]  = -1;
        }

        // GAME_MP of a gametypeid means any mulitplayer game.
        m_JoinSettings.m_GameTypeID                     = GAME_MP;
        m_JoinSettings.m_MinPlayers                     = -1;
        m_JoinSettings.m_MutationMode                   = (mutation_mode)-1;
        m_JoinSettings.m_PasswordEnabled                = -1;
        m_JoinSettings.m_VoiceEnabled                   = -1;
    }

    if( ResetFlags & RESET_HEADSET )
    {
        m_Volume[VOLUME_MIC]                            = 50;
        m_Volume[VOLUME_SPEAKER]                        = 100;
        m_HeadsetMode                                   = HEADSET_HEADSET_ONLY;
    }
    if( ResetFlags & RESET_AUDIO )
    {
        m_Volume[VOLUME_SFX]                            = 100;
        m_Volume[VOLUME_MUSIC]                          = 75;
        m_Volume[VOLUME_SPEECH]                         = 100;
        m_SpeakerSet                                    = SPEAKERS_STEREO;
    }

#ifdef TARGET_XBOX
    SetBrightness( xbox_GetDefaultBrightness() * UI_MAX_BRIGHTNESS_RANGE );
#endif
}

//==============================================================================
void global_settings::Commit( void )
{
    // Commit any settings required while hosting a game.
    g_PendingConfig.SetServerName( m_HostSettings.m_ServerName );

#ifdef TARGET_XBOX
    // set the brightness
    // todo: bhapgood! remember remember remember!
#endif

    CommitAudio();

    if( g_NetworkMgr.IsServer() )
    {
        g_VoiceMgr.SetIsGameVoiceEnabled( m_HostSettings.m_Flags & SERVER_VOICE_ENABLED );
    }

    UpdateHeadsetMode( GetHeadsetMode() );

    g_MatchMgr.SetLocalManifestVersion( m_ContentVersion );
    Checksum();
}

//==============================================================================

void global_settings::CommitAudio( void )
{
    // set up audio controls
    g_AudioMgr.SetSFXVolume  ( (f32)GetVolume( VOLUME_SFX    ) / 100.0f );
    g_AudioMgr.SetMusicVolume( (f32)GetVolume( VOLUME_MUSIC  ) / 100.0f );
    g_AudioMgr.SetVoiceVolume( (f32)GetVolume( VOLUME_SPEECH ) / 100.0f );

    // Setup headset volumes
    f32 HeadsetVolume    = (f32)GetVolume( VOLUME_SPEAKER ) / 100.0f;
    f32 MicrophoneVolume = (f32)GetVolume( VOLUME_MIC     ) / 100.0f;
    g_VoiceMgr.GetHeadset().SetVolume( HeadsetVolume, MicrophoneVolume );
#ifdef TARGET_PS2
    // set speaker config
    f32 SpeakerVolume = (f32)GetVolume( VOLUME_SPEAKER ) / 100.0f;
    g_VoiceMgr.GetHeadset().SetSpeakerVolume( SpeakerVolume );
    g_AudioMgr.SetSpeakerConfig( GetSpeakerSet() );
#endif
}

//==============================================================================
xbool global_settings::HasChanged ( void )
{
    s32 ActualChecksum;
    s32 DesiredChecksum;

    DesiredChecksum = m_Checksum;
    m_Checksum = 0;
    ActualChecksum = x_chksum( this, sizeof(global_settings) );
    m_Checksum = DesiredChecksum;
    return( DesiredChecksum != ActualChecksum );
}

//=========================================================================
void global_settings::MarkDirty( void )
{
    m_Checksum = 0;
}

//==============================================================================
xbool global_settings::Validate ( void )
{
    return !HasChanged();
}

//==============================================================================
void global_settings::Checksum ( void )
{
    m_Checksum  = 0; // Needs to be 0 prior to performing the crc32.
    m_DateStamp = eng_GetDate();
    m_Checksum  = x_chksum( this, sizeof(global_settings) );
}

//==============================================================================
xbool global_settings::IsDefault( void )
{
    return m_Checksum == DEFAULT_CHECKSUM;
}

//==============================================================================

headset_mode global_settings::GetHeadsetMode( void )
{
    return( m_HeadsetMode );
}

//=========================================================================

void global_settings::UpdateHeadsetMode( headset_mode HeadsetMode )
{
    // Always enable headset since we don't have an option to turn it off
    xbool IsVoiceEnabled    = TRUE;
    xbool IsVoiceAudible    = FALSE;
    xbool IsThroughSpeakers = FALSE;

    switch( HeadsetMode )
    {
        case HEADSET_HEADSET_ONLY       : IsVoiceAudible = TRUE;  IsThroughSpeakers = FALSE; IsVoiceEnabled = TRUE;     break;
        case HEADSET_THROUGH_SPEAKERS   : IsVoiceAudible = TRUE;  IsThroughSpeakers = TRUE;  IsVoiceEnabled = TRUE;     break;
        case HEADSET_DISABLED           : IsVoiceAudible = FALSE; IsThroughSpeakers = FALSE; IsVoiceEnabled = FALSE;    break;
        default                         : ASSERT( FALSE );
    };

    g_VoiceMgr.SetVoiceEnabled     ( IsVoiceEnabled    );
    g_VoiceMgr.SetVoiceAudible     ( IsVoiceAudible    );
    g_VoiceMgr.SetVoiceThruSpeakers( IsThroughSpeakers );

    // On the server we must update the game manager's voice peripheral status directly
    if( g_NetworkMgr.IsServer() == TRUE )
    {
        GameMgr.SetVoiceCapable( 0, g_VoiceMgr.IsVoiceCapable() );
    }
}

//==============================================================================
void global_settings::SetPatchData( void* pPatchData, s32 Size, s32 Version )
{
    m_PatchVersion = Version;
    m_PatchLength  = Size;
    x_memcpy( m_PatchData, pPatchData, Size );
}

//==============================================================================
void global_settings::SetContentVersion( s32 Version )
{
    m_ContentVersion = Version;
}

//==============================================================================
s32 global_settings::GetContentVersion( void )
{
    return m_ContentVersion;
}

//==============================================================================
void* global_settings::GetPatchData( s32& Length, s32& Version )
{
    Length = m_PatchLength;
    Version = m_PatchVersion;
    return m_PatchData;
}