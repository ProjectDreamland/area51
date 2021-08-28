//==============================================================================
//
//  VoiceMgr.hpp
// On the game server, this handles arbitration and distribution of voice data to
// all game clients. On the client, it will handle distribution to the local
// machine.
//
//==============================================================================

#ifndef VOICEMGR_HPP
#define VOICEMGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "Network/NetStream.hpp"
#include "Headset.hpp"
#include "NetworkMgr/netlimits.hpp"

//==============================================================================
//  DEFINES
//==============================================================================
#define VOICE_CHANNELS 2


//==============================================================================
//  TYPES
//==============================================================================

enum desired_talk_mode
{
    TALK_TEAM,
    TALK_GLOBAL,
    TALK_LOCAL,
    TALK_NONE,
};

enum actual_talk_mode {
    TALK_NEW_TEAM,     
    TALK_POT_TEAM,     
    TALK_OLD_TEAM,     

    TALK_NEW_LOCAL,    
    TALK_POT_LOCAL,    
    TALK_OLD_LOCAL,    

    TALK_NEW_GLOBAL,   
    TALK_POT_GLOBAL,   
    TALK_OLD_GLOBAL,   

    TALK_NOT_TALKING,         
    TALK_MODE_FIRST = TALK_NEW_TEAM,   
    TALK_MODE_LAST  = TALK_OLD_GLOBAL,    
};

struct speaker 
{
    s32                 PlayerNum;
    f32                 TalkTime;
    xbool               MoveToEndOfQueue;
    actual_talk_mode    ActualTalkMode;
};

struct listener
{
    s32 PlayerNum;
    s32 ListeningTo[ 4 ];
};

class voice_mgr
{
public:
                            voice_mgr               ( void );
                           ~voice_mgr               ( void );

        void                Init                    ( xbool LocalIsServer, xbool EnableHeadset );
        void                Kill                    ( void );
        void                Update                  ( f32 DeltaTime );
        xbool               IsTalking               ( void )                                                    { return m_Headset.IsTalking();             }
        void                SetTalking              ( xbool bTalking );
        void                Distribute              ( s32 TalkerIndex, const byte* pBuffer, s32 Length, s32 TalkMode );
        
        xbool               IsValidTarget           ( s32 Speaker, s32 Listener, actual_talk_mode TalkMode );
        void                DoArbitration           ( f32 DeltaTime );
        void                Arbitrate               ( s32 DesiredOwner, s32 TalkMode );
        void                AcceptUpdate            ( netstream& BitStream );
        void                ProvideUpdate           ( netstream& BitStream );

        s32                 ReadFromVoiceFifo       ( byte* pBuffer, s32 MaxLength );
        void                WriteToVoiceFifo        ( const byte* pBuffer, s32 Length );
        s32                 GetBytesInWriteFifo     ( void );
        void                SetLocalVoiceOwner      ( s32 Owner, actual_talk_mode TalkMode );
        s32                 GetLocalVoiceOwner      ( void );
        actual_talk_mode    GetLocalVoiceTalkType   ( void )                                                    { return m_LocalVoiceTalkType;              }
        xbool               IsSpeaking              ( s32 Speaker );
        void                AgeSpeaker              ( s32 Speaker,  f32 DeltaTime );
        void                SetSpeaking             ( s32 Speaker,  u32 Listeners );
        void                SetVoiceOwner           ( s32 Listener, s32 Speaker, actual_talk_mode TalkMode );
        desired_talk_mode   GetDesiredTalkMode      ( s32 PlayerIndex );
        void                ToggleTalkMode          ( void );

        desired_talk_mode   GetLocalTalkType        ( void )                                                    { return m_CurrentTalkType;                 }
        desired_talk_mode   GetLocalDesiredTalkMode ( void )                                                    { return m_LocalDesiredTalkMode;            }
        s32                 GetEncodeBlockSize      ( void )                                                    { return m_Headset.GetEncodedBlockSize();   }
        xbool               IsHeadsetPresent        ( void )                                                    { return m_Headset.IsHardwarePresent();     }
        xbool               IsVoiceBanned           ( void )                                                    { return m_Headset.IsBanned();              }
        xbool               IsVoiceEnabled          ( void )                                                    { return m_Headset.IsEnabled();             }
        xbool               IsVoiceAudible          ( void )                                                    { return m_Headset.IsAudible();             }
        xbool               IsThroughSpeaker        ( void )                                                    { return m_Headset.IsThroughSpeaker();      }
        xbool               IsVoiceCapable          ( void );

        void                SetVoiceBanned          ( xbool IsBanned  )                                         { m_Headset.SetVoiceBanned   ( IsBanned  ); }
        void                SetVoiceEnabled         ( xbool IsEnabled )                                         { m_Headset.SetVoiceEnabled  ( IsEnabled ); }
        void                SetVoiceAudible         ( xbool IsAudible )                                         { m_Headset.SetVoiceAudible  ( IsAudible ); }
        void                SetVoiceThruSpeakers    ( xbool IsEnabled )                                         { m_Headset.SetThroughSpeaker( IsEnabled ); }
        void                SetLoopback             ( xbool IsEnabled )                                         { m_Headset.SetLoopback      ( IsEnabled ); }
        void                SetVolume               ( f32 Head, f32 Mic )                                       { m_Headset.SetVolume( Head, Mic );         }
        void                GetVolume               ( f32& Head, f32& Mic )                                     { m_Headset.GetVolume( Head, Mic );         }

        xbool               GetLocallyMuted         ( s32 PlayerIndex )                                         { return !!(m_LocalMutedPlayers & (1 << PlayerIndex)); }
        void                SetLocallyMuted         ( s32 PlayerIndex, xbool bMuted )                           { 
                                                                                                                  m_LocalDirtyMutedPlayers = TRUE; 
                                                                                                                  m_LocalMutedPlayers = 
                                                                                                                      (m_LocalMutedPlayers & ~(1 << PlayerIndex)); 
                                                                                                                  if( bMuted )
                                                                                                                  {
                                                                                                                      m_LocalMutedPlayers = m_LocalMutedPlayers | (1 << PlayerIndex);
                                                                                                                  }
                                                                                                                }
        u32                 GetLocallyMutedBits     ( void )                                                    { return m_LocalMutedPlayers; }
        u32                 GetSpeakingBits         ( void )                                                    { return ((m_LocalVoiceOwner >= 0) ? ( 1 << m_LocalVoiceOwner ) : 0); }

        headset&            GetHeadset              ( void );
        
        void                SetIsGameVoiceEnabled   ( xbool Enabled )                                           { m_bGameIsVoiceEnabled = Enabled; }
        xbool               IsGameVoiceEnabled      ( void )                                                    { return m_bGameIsVoiceEnabled; }

private:
        xbool               m_bGameIsVoiceEnabled;
        xbool               m_Initialized;
        xbool               m_LocalIsServer;
        xbool               m_HeadsetEnabled;
        headset             m_Headset;
        desired_talk_mode   m_CurrentTalkType;
        desired_talk_mode   m_LocalDesiredTalkMode;
        s32                 m_PlayerNetSlot;

        s32                 m_LocalVoiceOwner;
        actual_talk_mode    m_LocalVoiceTalkType;

        s32                 m_MaxSpeakers;

        u32                 m_LocalMutedPlayers;
        xbool               m_LocalDirtyMutedPlayers;

        speaker             m_Speakers      [ NET_MAX_PLAYERS ];
        speaker*            m_SpeakerQueue  [ NET_MAX_PLAYERS ];
        listener            m_Listeners     [ NET_MAX_PLAYERS ];
};

extern voice_mgr g_VoiceMgr;

#if defined(X_DEBUG)
extern const char* GetTalkModeName( s32 Mode );
#endif
//==============================================================================
#endif // VOICEMGR_HPP
//==============================================================================
