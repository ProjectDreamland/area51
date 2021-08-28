//==============================================================================
//  
//  GameConfig.hpp
//  
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

#ifndef GAMECONFIG_HPP
#define GAMECONFIG_HPP

#include "NetworkMgr/MatchMgr.hpp"

enum exit_reason
{
    GAME_EXIT_ADVANCE_LEVEL=-256,
    GAME_EXIT_RELOAD_LEVEL,
    GAME_EXIT_RELOAD_CHECKPOINT,
    GAME_EXIT_PLAYER_DROPPED,
    GAME_EXIT_PLAYER_KICKED,
    GAME_EXIT_CONNECTION_LOST,
    GAME_EXIT_LOGIN_REFUSED,
    GAME_EXIT_BAD_PASSWORD,
    GAME_EXIT_CLIENT_BANNED,
    GAME_EXIT_SERVER_FULL,
    GAME_EXIT_PLAYER_QUIT,
    GAME_EXIT_GAME_COMPLETE,
    GAME_EXIT_INVALID_MISSION,
    GAME_EXIT_CANNOT_CONNECT,
    GAME_EXIT_NETWORK_DOWN,
    GAME_EXIT_SERVER_BUSY,
    GAME_EXIT_SERVER_SHUTDOWN,
    GAME_EXIT_FOLLOW_BUDDY,
    GAME_EXIT_SECURITY_QUERY,
    GAME_EXIT_SECURITY_FAILED,
    GAME_EXIT_SESSION_ENDED,
    GAME_EXIT_DUPLICATE_LOGIN,
    GAME_EXIT_CONTINUE=0,
};

class game_config
{
public:
                        game_config         ( void );
                       ~game_config         ( void );
        void            Reset               ( void );
        void            Invalidate          ( void );

        server_info&    GetConfig           ( void )                        { return m_Config; }
static  void            Commit              ( void );
static  void            Commit              ( const server_info& Config );
        xbool           IsValid             ( void );

        void            SetExitReason       ( exit_reason Reason );
        exit_reason     GetExitReason       ( void );

        xbool           SetLevelID          ( s32 LevelID );            // Returns TRUE only if the map was in the maplist
        s32             GetLevelID          ( void );

        void            SetPlayerCount      ( s32 Players );
        s32             GetPlayerCount      ( void );

        void            SetMaxPlayerCount   ( s32 MaxPlayers );
        s32             GetMaxPlayerCount   ( void );

        void            SetGameTypeID       ( game_type GameType );
        game_type       GetGameTypeID       ( void );

        void            SetShortGameType    ( const char* pShortGameType );
        void            SetShortGameType    ( const xwchar* pShortGameType );
        const xwchar*   GetShortGameType    ( void );

        void            SetGameType         ( const char* pGameType );
        void            SetGameType         ( const xwchar* pGameType );
        const xwchar*   GetGameType         ( void );

        void            SetAIType           ( s32 AIType );
        s32             GetAIType           ( void );

        void            SetNumberOfRounds   ( s32 NumberOfRounds );
        s32             GetNumberOfRounds   ( void );

        void            SetFirePercent      ( s32 FirePercent );
        s32             GetFirePercent      ( void );

        void            SetScoreLimit       ( s32 ScoreLimit );
        s32             GetScoreLimit       ( void );

        void            SetVotePercent      ( s32 VotePercent );
        s32             GetVotePercent      ( void );

        void            SetEliminationMode  ( xbool Enabled );
        xbool           GetEliminationMode  ( void );

        void            SetGameTime         ( s32 Time );
        s32             GetGameTime         ( void );

        void            SetSystemID         ( s32 SystemID );
        s32             GetSystemID         ( void );

        void            SetVersion          ( s32 Version );
        s32             GetVersion          ( void );

        void            SetVoiceEnabled     ( xbool IsEnabled );
        xbool           IsVoiceEnabled      ( void );

        void            SetMapScalingEnabled( xbool IsEnabled );
        xbool           IsMapScalingEnabled ( void );

        void            SetMutationMode     ( mutation_mode MutationMode );
        mutation_mode   GetMutationMode     ( void );

        void            SetPrivateServer    ( xbool bIsPrivate );
        xbool           GetPrivateServer    ( void );

        void            SetSkillLevel       ( skill_level SkillLevel );
        skill_level     GetSkillLevel       ( void );

        void            SetPasswordEnabled  ( xbool IsEnabled );
        xbool           IsPasswordEnabled   ( void );

        void            SetFriendlyFire     ( xbool IsEnabled );
        xbool           IsFriendlyFireEnabled( void );

        const xwchar*   GetLevelName        ( void );

        void            SetPassword         ( const char* pPassword );
        const char*     GetPassword         ( void );

        void            SetServerName       ( const xwchar* pName );
        const xwchar*   GetServerName       ( void );

        void            SetRemoteAddress    ( const net_address& Address );
     const net_address& GetRemoteAddress    ( void );

        void            SetFragLimit        ( s32 Limit );
        s32             GetFragLimit        ( void );

        s32             GetFlags            ( void );
        const char*     GetLevelPath        ( void );
        s32             GetLevelChecksum    ( void );
        void            SetLevelChecksum    ( s32 Checksum );

private:
        server_info     m_Config;
        exit_reason     m_ExitReason;
        s32             m_LevelChecksum;
        void            SetFlag             ( server_flags FlagMask, xbool Value );
        xbool           GetFlag             ( server_flags FlagMask );
};

//==============================================================================
//  functions
//==============================================================================
extern const char* GetExitReasonName   (exit_reason Reason);

extern game_config  g_PendingConfig;
extern game_config  g_ActiveConfig;
//==============================================================================
#endif // MAPLIST_HPP
//==============================================================================
