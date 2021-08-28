//==============================================================================
//  
//  GlobalSettings.hpp
//  
//==============================================================================

#ifndef GLOBALSETTINGS_HPP
#define GLOBALSETTINGS_HPP

#include "x_types.hpp"
#include "Entropy.hpp"
#include "NetworkMgr/NetLimits.hpp"
#include "NetworkMgr/GameMgr.hpp"


//==============================================================================
// Constants 
//==============================================================================
const s32           MAP_CYCLE_SIZE = 32;

enum volume_controls
{
    VOLUME_SFX,
    VOLUME_MUSIC,
    VOLUME_SPEECH,
    VOLUME_SPEAKER,
    VOLUME_MIC,
    VOLUME_LAST,
};

enum headset_mode
{
    HEADSET_HEADSET_ONLY,
    HEADSET_THROUGH_SPEAKERS,
    HEADSET_DISABLED,
};

//==============================================================================
// Map settings
//==============================================================================
struct map_settings
{
    s32             m_MapCycle[MAP_CYCLE_SIZE];   
    s32             m_MapCycleIdx;                
    s32             m_MapCycleCount;              
    xbool           m_bUseDefault;                
};

//==============================================================================
// MP settings
//==============================================================================
struct multi_settings
{
    s32             m_GameTypeID;
    s32             m_ScoreLimit;
    s32             m_TimeLimit;
    mutation_mode   m_MutationMode;
    map_settings    m_MapSettings;
};

//==============================================================================
// Host settings
//==============================================================================
struct host_settings
{
    xwchar          m_ServerName[NET_SERVER_NAME_LENGTH];
    char            m_Password[NET_PASSWORD_LENGTH];
    s32             m_GameTypeID;
    s32             m_ScoreLimit;
    s32             m_TimeLimit;
    s32             m_MaxPlayers;
    s32             m_VotePassPct;
    s32             m_FFirePct;
    s32             m_Flags;
    mutation_mode   m_MutationMode;
    map_settings    m_MapSettings;
    skill_level     m_SkillLevel;
};

//==============================================================================
// Join settings
//==============================================================================
struct join_settings
{
    s32             m_GameTypeID;
    s32             m_MinPlayers;
    s32             m_MutationMode;
    s32             m_PasswordEnabled;
    s32             m_VoiceEnabled;
};

enum settings_reset_flags
{
    RESET_HEADSET   = 1,
    RESET_AUDIO     = 2,
    RESET_ALL       = -1,
};

#ifdef TARGET_XBOX
#define UI_MAX_BRIGHTNESS_RANGE 100
extern void xbox_SetBrightness( f32 Brightness );
extern f32  xbox_GetBrightness( void );
extern f32  xbox_GetDefaultBrightness( void );
#endif

class global_settings
{
public:
                        global_settings         ( void );
                       ~global_settings         ( void );
    void                Reset                   ( s32 Flags = RESET_ALL );
    void                Commit                  ( void );
    void                CommitAudio             ( void );
    multi_settings&     GetMultiplayerSettings  ( void )                        { return m_MultiplayerSettings; }
    host_settings&      GetHostSettings         ( void )                        { return m_HostSettings;        }
    join_settings&      GetJoinSettings         ( void )                        { return m_JoinSettings;        }
    map_settings&       GetMapSettings          ( void )                        { return m_MapSettings;         }
    xbool               HasChanged              ( void );
    void                MarkDirty               ( void );
    xbool               Validate                ( void );
    void                Checksum                ( void );
#ifdef TARGET_XBOX
    void                SetBrightness           ( s32 Brightness )              { m_Brightness = Brightness;    }
    s32                 GetBrightness           ( void )                        { return m_Brightness;          }
#endif
    s32                 GetVolume               ( volume_controls Control )     { return m_Volume[Control];     }
    void                SetVolume               ( volume_controls Control, s32 Value ) { m_Volume[Control] = Value; }
    void                SetSpeakerSet           ( s32 SpeakerSet )              { m_SpeakerSet = SpeakerSet;    }
    s32                 GetSpeakerSet           ( void )                        { return m_SpeakerSet;          }
    headset_mode        GetHeadsetMode          ( void );
    void                SetHeadsetMode          ( headset_mode Mode )           { m_HeadsetMode = Mode;         }
    void                UpdateHeadsetMode       ( headset_mode Mode );
    xbool               IsDefault               ( void );
    datestamp           GetDateStamp            ( void )                        { return m_DateStamp;           }
    s32                 GetContentVersion       ( void );
    void                SetContentVersion       ( s32 Version );
    void*               GetPatchBuffer          ( void )                        { return m_PatchData;           }
    void                SetPatchData            ( void* pData, s32 Length, s32 Version );
    void*               GetPatchData            ( s32& Length, s32& Version );

private:
    s32                 m_Checksum;
    datestamp           m_DateStamp;
    s32                 m_Volume[VOLUME_LAST];
    s32                 m_SpeakerSet;
    s32                 m_ContentVersion;
    s32                 m_PatchVersion;
    s32                 m_PatchLength;
    byte                m_PatchData[NET_MAX_PATCH_SIZE];
#ifdef TARGET_XBOX
    s32                 m_Brightness;
#endif
    headset_mode        m_HeadsetMode;

    multi_settings      m_MultiplayerSettings;
    host_settings       m_HostSettings;
    join_settings       m_JoinSettings;
    map_settings        m_MapSettings;
};

//==============================================================================
#endif // GLOBALSETTINGS_HPP
//==============================================================================

