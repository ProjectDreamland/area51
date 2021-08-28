//==============================================================================
//
//  GameMgr.hpp
//
//==============================================================================

#ifndef GAMEMGR_HPP
#define GAMEMGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "logic_Base.hpp"
#include "NetLimits.hpp"
#include "Objects\MP_Settings.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  TYPES
//==============================================================================

class bitstream;

class logic_base;
class logic_campaign;
class logic_dm;
class logic_tdm;
class logic_ctf;
class logic_tag;
class logic_infect;
class logic_cnh;

class spawn_point;

class debug_menu_page_multiplayer;

//==============================================================================

enum game_type
{
    GAME_CAMPAIGN = -2,
    GAME_MP       = -1, // Only used on client.  Represents ALL online types.

    GAME_DM,    //   0
    GAME_TDM,   //   1
    GAME_CTF,   //   2
    GAME_TAG,   //   3
    GAME_INF,   //   4
    GAME_CNH,   //   5

    GAME_RESERVED_06,
    GAME_RESERVED_07,
    GAME_RESERVED_08,
    GAME_RESERVED_09,
    GAME_RESERVED_10,
    GAME_RESERVED_11,
    GAME_RESERVED_12,
    GAME_RESERVED_13,
    GAME_RESERVED_14,
    GAME_RESERVED_15,

    GAME_LAST,          
    GAME_FIRST    = 0,
};

//==============================================================================

enum mutation_mode
{
    MUTATE_CHANGE_AT_WILL,
    MUTATE_HUMAN_LOCK,
    MUTATE_MUTANT_LOCK,
    MUTATE_HUMAN_VS_MUTANT,
    MUTATE_MAN_HUNT,
    MUTATE_MUTANT_HUNT,
};

//==============================================================================

enum skill_level
{
    SKILL_LEVEL_ANY,
    SKILL_LEVEL_BEGINNER,
    SKILL_LEVEL_INTERMEDIATE,
    SKILL_LEVEL_ADVANCED,
};

//==============================================================================

enum score_display
{
    SCORE_DISPLAY_NONE,
    SCORE_DISPLAY_RANKED,
    SCORE_DISPLAY_TEAM,
    SCORE_DISPLAY_CNH,
    SCORE_DISPLAY_CUSTOM,

    SCORE_DISPLAY_MAX,
};

//==============================================================================
//  TYPES
//==============================================================================

enum score_fields
{
    SCORE_POINTS  = 0x00000001,
    SCORE_KILLS   = 0x00000002,
    SCORE_DEATHS  = 0x00000004,
    SCORE_TKS     = 0x00000008,
    SCORE_FLAGS   = 0x00000010,
    SCORE_VOTES   = 0x00000020,

    SCORE_ALL     = 0x0000003F,     // MUST BE THE UNION OF ALL OF ABOVE BITS.
};

//==============================================================================

enum skin_type
{
    SKIN_NULL = -1,     // Used to flag uninitialized.

    SKIN_HAZMAT_0,
    SKIN_HAZMAT_1,
    SKIN_HAZMAT_2,
    SKIN_HAZMAT_3,

    SKIN_SPECFOR_0,
    SKIN_SPECFOR_1,
    SKIN_SPECFOR_2,
    SKIN_SPECFOR_3,

    SKIN_TECH_0,
    SKIN_TECH_1,
    SKIN_TECH_2,
    SKIN_TECH_3,

    SKIN_GREY_0,
    SKIN_GREY_1,
    SKIN_GREY_2,
    SKIN_GREY_3,

        SKIN_BEGIN_PLAYERS = SKIN_HAZMAT_0,
        SKIN_END_PLAYERS   = SKIN_GREY_3,

    SKIN_BOT_GRUNT,
    SKIN_BOT_THETA,
    SKIN_BOT_LEAPER,
    SKIN_BOT_BLACKOP,
    SKIN_BOT_MITE,
    SKIN_BOT_GRAY,
    SKIN_BOT_ALIEN,

    SKIN_COUNT
};

//==============================================================================

struct player_stats
{
    s32     KillsAsHuman;
    s32     KillsAsMutant;
    s32     Deaths;
    s32     PlayTime;   // In minutes.
    s32     Games;      // Only count games completed.
    s32     Wins;       // Only count 'valid' participation on winning team.
    s32     Gold;
    s32     Silver;
    s32     Bronze;
    s32     Kicks;
    s32     VotesStarted;
};

//==============================================================================

struct player_score
{
    xwchar  Name [ NET_NAME_LENGTH ];   // PlayerBits
    char    NName[ NET_NAME_LENGTH ];   //  *derived
    xbool   IsConnected;                // PlayerBits
    xbool   IsInGame;                   // PlayerBits
    xbool   IsBot;                      //
    s32     Team;                       // PlayerBits
    s32     ClientIndex;                // PlayerBits -- TO DO -- Not sent now.
    u64     Identifier;

    s32     MusicLevel;                 // ScoreBits
    xbool   Speaking;
    xbool   IsVoiceAllowed;
    xbool   IsVoiceCapable;             // TRUE when headset is plugged in and enabled
                                        
    s32     Score;                      // ScoreBits if SCORE_POINTS
    s32     Kills;                      // ScoreBits if SCORE_KILLS 
    s32     Deaths;                     // ScoreBits if SCORE_DEATHS
    s32     TKs;                        // ScoreBits if SCORE_TKS   
    s32     Flags;                      // ScoreBits if SCORE_FLAGS 
    s32     Votes;                      // ScoreBits if SCORE_VOTES 

    f32     TeamTime;
    s32     Games;    
    s32     Wins;     
    s32     Gold;
    s32     Silver;
    s32     Bronze;

    s32     Rank;                       //  *Not sent.

    void    ClearAll    ( void );
    void    ClearScore  ( void );
};

//------------------------------------------------------------------------------

inline
void player_score::ClearAll( void )
{
    IsConnected = FALSE;
    IsInGame    = FALSE;
    IsBot       = FALSE;
    Team        = -1;
    ClientIndex = -1;

    IsVoiceAllowed    = FALSE;
    IsVoiceCapable    = FALSE;

    ClearScore();
}

//------------------------------------------------------------------------------

inline
void player_score::ClearScore( void )
{
    Score  = 0;
    Kills  = 0;
    Deaths = 0;
    TKs    = 0;
    Flags  = 0;
    Votes  = 0;

    TeamTime = 0.0f;
    Games    = 0;
    Wins     = 0;
    Gold     = 0;
    Silver   = 0;
    Bronze   = 0;

    MusicLevel = 0;
}

//==============================================================================

struct team_score
{
    xwchar  Name [ NET_NAME_LENGTH ];   // DIRTY_TEAM_NAMES -- TO DO
    char    NName[ NET_NAME_LENGTH ];   //  *derived
    s32     Size;                       // DIRTY_TEAMS -- TO DO?
    s32     Score;                      // DIRTY_TEAM_SCORES -- TO DO

    void    ClearAll    ( void );
    void    ClearScore  ( void );
};

//------------------------------------------------------------------------------

inline
void team_score::ClearAll( void )
{
    Name [0] = '\0';
    NName[0] = '\0';
    Size     = 0;

    ClearScore();
}

//------------------------------------------------------------------------------

inline
void team_score::ClearScore( void )
{
    Score = 0;
}

//==============================================================================

struct game_score
{
    xwchar          MissionName[ NET_NAME_LENGTH ];     // DIRTY_INIT -- TO DO
    xbool           IsGameOver;                         //  *derived
    xbool           IsTeamBased;                        // DIRTY_INIT -- TO DO
    u32             ScoreFieldMask;                     // DIRTY_INIT -- TO DO
    u32             HighLightMask;                      // DIRTY_HIGHLIGHT -- TO DO 
    s32             NConnected;                         //  *derived
    s32             NPlayers;                           //  *derived
    team_score      Team[2];                            // (see above)
    player_score    Player[ NET_MAX_PLAYERS ];          // (see above)

    void            ComputeRanks    ( void );
};

//==============================================================================

struct player_options
{
    s32     Skin;

    // Setup defaults
    player_options( void )
    {
        Skin = SKIN_HAZMAT_0;
    }
};

//==============================================================================

struct gm_utility
{
    f32     GreetingTimer;
};

//==============================================================================

struct zone_state
{
    s32     Minimum : 7;
    s32     Maximum : 7;
    u32     Warned  : 1;
    u32     Locked  : 1;

    zone_state( void )
    {
        Minimum =  0;
        Maximum = 32;
        Warned  = FALSE;
        Locked  = FALSE;
    }
};

//==============================================================================
//  CLASS game_mgr
//==============================================================================

class game_mgr
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

protected:

    enum dirty_bits
    {
        DIRTY_INIT          = 0x00000001,
        DIRTY_TEAM_NAMES    = 0x00000002,
        DIRTY_TEAM_SCORES   = 0x00000004, // TO DO - Review use of this.  Team size?
        DIRTY_HIGHLIGHT     = 0x00000008,
        DIRTY_CLOCK         = 0x00000010,
        DIRTY_CLOCK_CONFIG  = 0x00000020,
        DIRTY_VOTE_MODE     = 0x00000040,
        DIRTY_VOTE_UPDATE   = 0x00000080,
        DIRTY_CIRCUIT_BITS  = 0x00000100,
        DIRTY_ZONE_STATE    = 0x00000200,
        DIRTY_SCORE_DISPLAY = 0x00000400,

        DIRTY_ALL           = 0x000007FF,   // MUST BE THE UNION OF ALL ABOVE BITS.

        /*
        DIRTY_CLOCK         = 0x00000002,
        DIRTY_CLOCK_CONFIG  = 0x00000004,
        DIRTY_TEAM_NAMES    = 0x00000010,
        DIRTY_TEAM_SIZES    = 0x00000020,
        DIRTY_TEAM_SCORES   = 0x00000040,
        */
    };

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

public:

                        game_mgr        ( void );
                       ~game_mgr        ( void );

        void            Init            ( void );
        void            Kill            ( void );
        void            DebugRender     ( void );
                        
        void            Logic           ( f32 DeltaTime );
                        
        void            BeginGame       ( void );
        void            EndGame         ( xbool Complete = FALSE );
        xbool           GameInProgress  ( void ) { return( m_GameInProgress ); }
                        
        void            SetGameType     ( game_type GameType );
        game_type       GetGameType     ( void );
                        
        void            SetMutationMode ( mutation_mode MutationMode );
        mutation_mode   GetMutationMode ( void );

        void            SetClock        ( f32 Seconds );
        void            SetClockLimit   ( s32 Seconds );
        void            SetClockMode    ( s32 Mode    ); // -1=down,0=off,+1=up
        f32             GetClock        ( void );        // in seconds
        s32             GetClockLimit   ( void );        // in seconds
        s32             GetClockMode    ( void );        // -1=down,0=off,+1=up

        void            SetScoreLimit   ( s32 ScoreLimit );
        s32             GetScoreLimit   ( void ) { return( m_ScoreLimit ); }

        void            SetMaxPlayers   ( s32 MaxPlayers );                        
        void            SetVotePercent  ( s32 VotePercent );  // s32: 0 - 100

        f32             GetGameProgress ( void );

        xbool           RoomForPlayers  (       s32     NPlayers );
        void            Connect         (       s32&    PlayerIndex,
                                                s32     ClientIndex,
                                                u64     Identifier,
                                          const xwchar* pName,
                                                s32     Skin,
                                                xbool   Bot );
        void            Disconnect      ( s32 PlayerIndex );
        void            EnterGame       ( s32 PlayerIndex );
        void            ExitGame        ( s32 PlayerIndex );
                        
        void            KickPlayer      ( s32 PlayerIndex, xbool ByAdmin = TRUE );
        void            ChangeTeam      ( s32 PlayerIndex, xbool ByAdmin = TRUE );
        void            AdminKill       ( s32 PlayerIndex );
        void            PlayerSuicide   ( void );
        
        void            StartVoteKick   ( s32 PlayerIndex, s32 StarterIndex );
        void            StartVoteMap    ( s32 MapIndex,    s32 StarterIndex );
        void            CastVote        ( s32 PlayerIndex, s32 Vote );

        xbool           GetVoteData     ( const xwchar*& pMessage1, 
                                          const xwchar*& pMessage2, 
                                                s32&     Yes, 
                                                s32&     No, 
                                                s32&     Missing, 
                                                s32&     PercentNeeded );

        u32             GetTeamBits     ( s32 PlayerIndex );
        u32             GetCircuitBits  ( s32 Circuit );
        void            SetCircuitBits  ( s32 Circuit, u32 Bits );

        xbool           IsZoneOpen      ( s32 Z    ) { return( !m_Zone[Z].Warned && !m_Zone[Z].Locked ); };
        xbool           IsZoneWarned    ( s32 Zone ) { return( m_Zone[Zone].Warned ); };
        xbool           IsZoneLocked    ( s32 Zone ) { return( m_Zone[Zone].Locked ); };
        xbool           IsZoneColored   ( s32 Zone );
        void            SetZoneLimits   ( s32 Zone, s32 Min, s32 Max );
        void            SetZoneMinimum  ( s32 Minimum );
        void            SetMapScaling   ( xbool MapScaling );
        xbool           GetMapScaling   ( void );
        xcolor          GetZoneColor    ( void ) { return( m_ZoneColor ); }

        void            SetTeamDamage   ( f32 TeamDamage );
        f32             GetTeamDamage   ( void );

const   game_score&     GetScore        ( void );
const   player_stats&   GetPlayerStats  ( void ) { return( m_PlayerStats  ); };
        score_display   GetScoreDisplay ( void ) { return( m_ScoreDisplay ); };

        void            GetClearDirty   (       u32& DirtyBits, 
                                                u32& ScoreBits, 
                                                u32& PlayerBits );
        void            AcceptUpdate    ( const bitstream& BitStream );
        void            ProvideUpdate   (       bitstream& BitStream,
                                                s32        ClientIndex,
                                                u32&       DirtyBits, 
                                                u32&       ScoreBits, 
                                                u32&       PlayerBits );

        void            ProvideFinalScore   ( bitstream& BitStream, s32 ClientIndex );
        void            AcceptFinalScore    ( bitstream& BitStream );

        xbool           IsGameOnline        ( void );
        xbool           IsGameMultiplayer   ( void );
        void            GetSkinValues       ( s32 Skin, s32& Mesh, s32& Texture );
        const char*     GetSkinAudioPrefix  ( s32 Skin, s32 VoiceActor, xbool bMutated );
        void            ScoreDisplayLogic   ( void );
        void            SetSpeaking         ( s32 PlayerIndex, xbool Speaking );

        void            DebugSetSkin        ( s32 Skin );
        void            DebugSetVoiceActor  ( s32 VoiceActor );

        void            SetVoiceAllowed     ( s32 PlayerIndex, xbool IsVoiceAllowed    );
        void            SetVoiceCapable     ( s32 PlayerIndex, xbool IsVoiceCapable    );

        void            AddKickStat         ( void );
        void            ResetPlayerStats    ( void ) { x_memset( &m_PlayerStats, 0, sizeof(m_PlayerStats) ); }

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------

protected:

public: // HACKOMOTRON HACKOMOTRON HACKOMOTRON

        void            CreatePlayer        ( s32 PlayerIndex );
        void            RegisterName        ( s32 PlayerIndex, const xwchar* pName );
const   char*           GetSkinBlueprint    ( s32 Skin );
        spawn_point*    SelectSpawnPoint    ( u32 TeamBits );
        void            BuildVoteMessage    ( void );
        void            ExecuteVote         ( void );
        void            VoteResult          ( xbool Passed );
        void            ShutDownVote        ( void );
        void            UpdateVoteRights    ( void );
        u32             GetVoteKickMask     ( s32 PlayerIndex );
        void            SetScoreDisplay     ( score_display ScoreDisplay );
        void            MsgConnect          ( s32 PlayerIndex );
        void            MsgEnterGame        ( s32 PlayerIndex );
        void            MsgDisconnect       ( s32 PlayerIndex );
        void            ZoneLogicAll        ( f32 DeltaTime );
        void            ZoneLogicServer     ( f32 DeltaTime );
        void            UpdatePlayerStats   ( void );

//------------------------------------------------------------------------------
//  Private Storage
//------------------------------------------------------------------------------

protected:

    // The Initialized flag is static to ensure that only one game manager is 
    // ever instantiated.

static  xbool           m_Initialized;
                        
        game_type       m_GameType;                 //  *Not sent.
        mutation_mode   m_MutationMode;             //  *Not sent.
        score_display   m_ScoreDisplay;             // SCORE_DISPLAY

        xbool           m_DebugRender;
                        
        game_score      m_Score;                    //  *various
        game_score      m_FinalScore;               // Only used between games.
        player_stats    m_PlayerStats;              // Stats for local player.
                        
        s32             m_MaxPlayers;               // Not sent.
                        
        f32             m_Clock;                    // CLOCK           
        f32             m_ClockUpdate;              //  *Not sent.
        s32             m_ClockMode;                // CLOCK_CONFIG    
        s32             m_ClockLimit;               // CLOCK_CONFIG    

        f32             m_PlayTime;                 //  *Not sent.

        xbool           m_GameInProgress;           //  *Not sent.
        f32             m_TeamDamageFactor;         // DIRTY_INIT
        s32             m_ScoreLimit;               //  *Not sent.
            
        player_options  m_Options[NET_MAX_PLAYERS]; //  *Not sent.
        gm_utility      m_Utility[NET_MAX_PLAYERS]; //  *Not sent.
            
        u32             m_DirtyBits;                //  *Not sent.
        u32             m_ScoreBits;                //  *Not sent.
        u32             m_PlayerBits;               //  *Not sent.

        s32             m_LastIntensity;            //  *Not sent.

        u32             m_CircuitBits[MAX_CIRCUITS];// DIRTY_CIRCUIT_BITS

        zone_state      m_Zone[256];                // DIRTY_ZONE_STATE
        f32             m_ZoneTimer;                //  *Not sent.
        s32             m_ZoneMinimum;              //  *Not sent.
        xbool           m_MapScaling;               //  *Not sent.

        xbool           m_VoteInProgress;           // DIRTY_VOTE_MODE (and UPDATE)
        s32             m_VotePercent;              // DIRTY_VOTE_MODE
        xwchar          m_VoteMsg[2][64];           // DIRTY_VOTE_MODE
        s32             m_VotesYes;                 // DIRTY_VOTE_UPDATE
        s32             m_VotesNo;                  // DIRTY_VOTE_UPDATE
        s32             m_VotesMissing;             // DIRTY_VOTE_UPDATE
        xbool           m_VoteHasPassed;            //  *Not sent.
        f32             m_VoteTimer;                //  *Not sent.
        s32             m_VoteKick;                 //  *Not sent.
        s32             m_VoteMap;                  //  *Not sent.
        u32             m_VoteCanCast;              //  *Not sent.
        u32             m_VoteCanSee;               //  *Not sent.
        xbool           m_UpdateVoteRights;         //  *Not sent.

        xbool           m_Minutes5;                 //  *Not sent.
        xbool           m_Minutes2;                 //  *Not sent.
        xbool           m_Seconds60;                //  *Not sent.
        xbool           m_Seconds30;                //  *Not sent.
        xbool           m_Seconds10;                //  *Not sent.
        xbool           m_Seconds05;                //  *Not sent.

        xcolor          m_ZoneColor;
        f32             m_ZonePulse;
        s32             m_ZonePhase;

//------------------------------------------------------------------------------
//  Friends
//------------------------------------------------------------------------------

    friend logic_base;
    friend logic_campaign;
    friend logic_dm;
    friend logic_tdm;
    friend logic_ctf;
    friend logic_tag;
    friend logic_infect;
    friend logic_cnh;

    friend debug_menu_page_multiplayer;
};

//==============================================================================
//  GLOBAL VARIABLES
//==============================================================================

extern game_mgr     GameMgr;
extern logic_base*  pGameLogic;

//==============================================================================
#endif // GAMEMGR_HPP
//==============================================================================
