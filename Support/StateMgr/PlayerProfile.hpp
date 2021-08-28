//==============================================================================
//  
//  PlayerProfile.hpp
//  
//==============================================================================

#ifndef PLAYER_PROFILE_HPP
#define PLAYER_PROFILE_HPP

#include "x_types.hpp"
#include "GlobalSettings.hpp"
#include "CheckPointMgr/CheckpointMgr.hpp"
#include "LoreList.hpp"

//==============================================================================
//  Defines
//==============================================================================

// NOTE: if you change this value, increase the profile version, add a note, and let MarkB know. Thanks!
#define MAX_SAVED_LEVELS 20

//********************************************************
//** PROFILE VERSION HISTORY
//********************************************************
// 1006 - BW: Added checkpoints to saved profiles.
// 1007 - BW: Cleaned up profile list use in memory card manager. Preserved selected profile info.
//            Fixed memory fragmentation.
// 1008 - MB: Removed buddy lists.
// 1009 - BW: Added CRC checksum to player profile.
// 1010 - BW: Modified the way profiles are being saved and loaded.
// 1011 - MB: Added secrets data (and fixed bad merge)
// 1012 - MB: Increased number of lore items and fixed bad initialization of lore collected.
// 1013 - MB: Added a count for entering a cinema whilst mutated.
// 1014 - MB: Defaulted map scaling to enabled. (and added some LAN party defaults)
// 1015 - MB: Added a variable to track the players mutation status.
// 1016 - MB: Added difficulty level setting
// 1017 - BW: Added flag whether or not to appear visible while online.
// 1018 - KS: Upped MAX_SAVED_LEVELS because there are 19 levels and Welcome to Dreamland was overwritten by Last Exit
// 1019 - KS: MAX_SAVED_LEVELS back down to 20 since it's a HUGE memory hog
// 1020 - MB: Added autosave flag
// 1021 - MB: Added flag to enable use of the alien avatars
// 1022 - CJ: Reduced patch size to save RAM
// 1023 - MB: Reverted increase in MAX_SAVED_LEVELS. 
// 1024 - MB: Added some new flags for tracking difficulty and awarding a secret
// 1025 - MB: Size of profile name changed
// 1026 - MB: Added a flag for verifying the player's age before going online (COPA requirement)

#define PROFILE_VERSION_NUMBER  1027

enum profile_states
{
    PROFILE_OK,
    PROFILE_CORRUPT,
    PROFILE_EXPIRED,
};

enum difficulty_level
{
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD,
};

//==============================================================================
// profile
//==============================================================================
class player_profile
{
public:
                        player_profile          ( void );
    void                Reset                   ( void );
    u32                 GetVersion              ( void )                            { return m_Version; }

    void                RestoreControlDefaults  ( void );
    void                RestoreAudioDefaults    ( void );
    void                RestoreHeadsetDefaults  ( void );

    void                SetProfileName          ( const char* pProfileName );
    const char*         GetProfileName          ( void )const                       { return ( const char* )m_pProfileName; }

    void                SetHash                 ( void );
    u32                 GetHash                 ( void )                            { return m_HashString; }

    u32                 GetSensitivity          ( u32 Index )const                  { ASSERT( Index < 2 ); return m_Sensitivity[ Index ]; }
    void                SetSensitivity          ( u32 Index, u32 Sensitivity )      { ASSERT( Index < 2 ); m_Sensitivity[Index] = Sensitivity; }

    u8                  GetVolume               ( u32 Index )const                  { ASSERT( Index < 5 ); return m_Volume[ Index ]; }
    void                SetVolume               ( u32 Index, u8 Volume )            { ASSERT( Index < 5 ); m_Volume[Index] = Volume; }

    s32                 GetAvatarID             ( void )                            { return m_AvatarID;     }
    void                SetAvatarID             ( s32 AvatarID )                    { m_AvatarID = AvatarID; }

    xbool               GetLoreAcquired         ( u32 Vault, s32 Index );
    void                SetLoreAcquired         ( u32 Vault, u32 Index );

    void                AcquireSecret           ( void );

    u32                 GetTotalLoreAcquired    ( void )                            { return m_LoreTotal;          }
    xbool               IsNewLoreUnlocked       ( void )                            { return m_bNewLoreUnlocked;   }
    void                ClearNewLoreUnlocked    ( void )                            { m_bNewLoreUnlocked = FALSE;  }

    u32                 GetNumSecretsUnlocked   ( void )                            { return m_NumSecretsUnlocked; }
    xbool               IsNewSecretUnlocked     ( void )                            { return m_bNewSecretUnlocked; }
    void                ClearNewSecretUnlocked  ( void )                            { m_bNewSecretUnlocked = FALSE;}
    
    void                SetUniqueId             ( const byte* pUniqueId, s32 IdLength );
    const byte*         GetUniqueId             ( s32& IdLength );

    xbool               DisplayCinemaMutatedMsg ( void );

    void                SetPlayerIsMutated      ( xbool bIsMutated )                { m_bIsMutated = bIsMutated;   }
    xbool               IsPlayerMutated         ( void )                            { return m_bIsMutated;         }

    void                SetDifficultyLevel      ( u8 Difficulty )                   { m_DifficultyLevel = Difficulty; }
    u8                  GetDifficultyLevel      ( void )                            { return m_DifficultyLevel;       }

    void                SetWeaponAutoSwitch     ( xbool bAutoSwitch )               { m_bWeaponAutoSwitch = bAutoSwitch; }
    xbool               GetWeaponAutoSwitch     ( void )                            { return m_bWeaponAutoSwitch;        }
 
    level_check_points& GetCheckpoint           ( s32 Index )                       { return m_Checkpoints[Index]; }
    level_check_points* GetCheckpointByMapID    ( s32 MapID );

    void                Checksum                ( void );
    xbool               Validate                ( void );
    xbool               HasChanged              ( void );
    void                MarkDirty               ( void );

#ifndef CONFIG_RETAIL
    void                UnlockAll               ( void );
#endif

private:
    s32                 m_Checksum;                                                     // CRC32 of the profile
    u32                 m_Version;                                                      // profile version control
    char                m_pProfileName[32];                                             // the nickname (MUST BE FIRST)
    u32                 m_HashString;                                                   // hash of the profile name and the time created
    s32                 m_AvatarID;                                                     // avatar
    u32                 m_Sensitivity[2];                                               // controller sensitivity
    u8                  m_Volume[5];                                                    // volume controls
    u8                  m_Lore[NUM_VAULTS];                                             // lore acquired flags
    u32                 m_LoreTotal;                                                    // total lore acquired
    xbool               m_bNewLoreUnlocked;                                             // flag set when new piece of lore is unlocked
    u32                 m_NumSecretsUnlocked;                                           // number of secrets unlocked
    xbool               m_bNewSecretUnlocked;                                           // flag set when a new secret is unlocked
    byte                m_UniqueId[64];
    s32                 m_UniqueIdLength;
    xbool               m_bIsMutated;                                                   // is the player currently in mutant form
    s8                  m_CinemaMutatedMsgCount;                                        // counts how many times we should display a msg
    u8                  m_DifficultyLevel;                                              // campaign game difficulty level
    xbool               m_bWeaponAutoSwitch;                                            // if on/true, will auto-switch to a weapon with a > rating

    level_check_points  m_Checkpoints[ MAX_SAVED_LEVELS ];

public:
    // members 
    u32                 m_bLookspringOn         : 1;  // toggle lookspring
    u32                 m_bCrouchOn             : 1;  // toggle crouch
    u32                 m_bIsActive             : 1;  // is currently active 
    u32                 m_bInvertY              : 1;  // invert Y axis
    u32                 m_bVibration            : 1;  // Force feedback/vibration enabled
    u32                 m_bIsVisibleOnline      : 1;  // Report full status when online
    u32                 m_bAutosaveOn           : 1;  // Autosave is enabled
    u32                 m_bAlienAvatarsOn       : 1;  // Alien avatars are selectable
    u32                 m_bSecretAwarded        : 1;  // Give a secret away after deep underground
    u32                 m_bHardUnlocked         : 1;  // Difficulty level hard is available
    u32                 m_bDifficultyChanged    : 1;  // Was the difficulty level changed during a campaign
    u32                 m_bAgeVerified          : 1;  // The player's age has been verified for this profile (COPA requirement)
    u32                 m_bGameFinished         : 1;  // Player has finished the game.
};

//==============================================================================
//  profile_info
//==============================================================================

struct profile_info
{
    xbool       bDamaged;
    s32         ProfileID;
    s32         CardID;
    xwstring    Name;
    u32         Hash;           // unique identifier for this profile
    u32         Ver;
    xstring     Dir;
    datestamp   CreationDate;   // in platform specific format
    datestamp   ModifiedDate;   // in platform specific format
};


//==============================================================================
#endif // PLAYER_PROFILE_HPP
//==============================================================================

