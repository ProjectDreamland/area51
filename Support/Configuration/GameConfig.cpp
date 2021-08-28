//==============================================================================
//  
//  GameConfig.hpp
//  
//==============================================================================
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment Inc.
//
//==============================================================================
//
//  This is just a repository of information about the state of the game 
//  configuration.  The only thing functions in this file should do is to 
//  validate parameters passed in.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"
#include "GameConfig.hpp"
#include "StateMgr/MapList.hpp"
#include "stringmgr\stringmgr.hpp"


//==============================================================================
//  STORAGE
//==============================================================================

game_config g_ActiveConfig;
game_config g_PendingConfig;

//==============================================================================
//  FUNCTIONS
//==============================================================================

game_config::game_config( void )
{
    Invalidate();
}

//==============================================================================

game_config::~game_config( void )
{
}

//==============================================================================

void game_config::Invalidate( void )
{
    u8* pThis = (u8*)this;
    s32 i;

    ASSERT( (sizeof(this) & 0x03) == 0 );

    for( i=0;i<sizeof(this);i+=4 )
    {
        *pThis++=0xde;
        *pThis++=0xad;
        *pThis++=0xbe;
        *pThis++=0xef;
    }
}

//==============================================================================

void game_config::Commit( const server_info& Config )
{

    g_ActiveConfig.m_Config = Config;
    ASSERT( g_ActiveConfig.IsValid() );
    // We need to validate the game configuration at this point.
}

//==============================================================================

void game_config::Commit( void )
{
    ASSERT( g_PendingConfig.IsValid() );
    Commit( g_PendingConfig.m_Config );
}

//==============================================================================

void game_config::Reset ( void )
{
}

//==============================================================================

xbool game_config::IsValid ( void )
{
    //
    // First, assert everything is valid.
    //

    if( m_Config.GameTypeID == GAME_MP )
    {
        // This means we have yet to receive information about the target gametype. This will happen
        // when we try to follow a friend in to a game or accept an invite.
        return TRUE;
    }
    ASSERT( IN_RANGE(             1000, m_Config.Level     , 9000        ) );
    ASSERT( IN_RANGE(    GAME_CAMPAIGN, m_Config.GameTypeID, GAME_LAST-1 ) );
    ASSERT( IN_RANGE(                0, m_Config.Players   , 32          ) );
    ASSERT( IN_RANGE( m_Config.Players, m_Config.MaxPlayers, 32          ) );

    //
    // Levels 4000 to 5000 are downloadable maps. They may not be in the maplist at this moment in time.
    //
    ASSERT( (g_MapList.Find( m_Config.Level ) != NULL) || 
            ((m_Config.Level >= 4000) && (m_Config.Level < 5000)) );

    //
    // And, in case asserts are off, return a FALSE if anything is invalid.
    //

    if( !IN_RANGE(             1000, m_Config.Level     , 9000        ) ) return( FALSE );
    if( !IN_RANGE(    GAME_CAMPAIGN, m_Config.GameTypeID, GAME_LAST-1 ) ) return( FALSE );
    if( !IN_RANGE(                0, m_Config.Players   , 32          ) ) return( FALSE );
    if( !IN_RANGE( m_Config.Players, m_Config.MaxPlayers, 32          ) ) return( FALSE );

    if( (g_MapList.Find( m_Config.Level ) == NULL) && 
        ((m_Config.Level < 4000) && (m_Config.Level >= 5000)) )
    {
        return( FALSE );
    }

    //
    // Looks like everything is cool.
    //

    return( TRUE );
}

//==============================================================================

xbool game_config::SetLevelID ( s32 LevelID )
{
    m_Config.Level = LevelID;
    const map_entry* pMapEntry;

    pMapEntry = g_MapList.Find( LevelID );
    if( pMapEntry == NULL )
    {
        return FALSE;
    }
    x_mstrcpy( m_Config.MissionName, pMapEntry->GetDisplayName() );
    return TRUE;
}

//==============================================================================

s32 game_config::GetLevelID ( void )
{
    return m_Config.Level;
}

//==============================================================================

void game_config::SetPlayerCount ( s32 Players )
{
    m_Config.Players = Players;
}

//==============================================================================

s32 game_config::GetPlayerCount ( void )
{
    return m_Config.Players;
}

//==============================================================================

void game_config::SetMaxPlayerCount ( s32 MaxPlayers )
{
    m_Config.MaxPlayers = MaxPlayers;
}

//==============================================================================

s32 game_config::GetMaxPlayerCount ( void )
{
    return m_Config.MaxPlayers;
}

//==============================================================================

void game_config::SetGameTypeID ( game_type GameType )
{
    m_Config.GameTypeID = GameType;
    // set short game type
    switch( GameType )
    {
    case GAME_DM:   
        SetGameType( g_StringTableMgr( "ui", "IDS_GAMETYPE_DM" ) );
        SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_DM"  ) );     
        break;
    case GAME_TDM:
        SetGameType( g_StringTableMgr( "ui", "IDS_GAMETYPE_TDM" ) );
        SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_TDM" ) );
        break;
    case GAME_CTF:
        SetGameType( g_StringTableMgr( "ui", "IDS_GAMETYPE_CTF" ) );
        SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_CTF" ) );
        break;
    case GAME_TAG:  
        SetGameType( g_StringTableMgr( "ui", "IDS_GAMETYPE_TAG" ) );
        SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_TAG" ) );
        break;
    case GAME_INF:  
        SetGameType( g_StringTableMgr( "ui", "IDS_GAMETYPE_INF" ) );
        SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_INF" ) );     
        break;
    case GAME_CNH:  
        SetGameType( g_StringTableMgr( "ui", "IDS_GAMETYPE_CNH" ) );
        SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_CNH" ) );     
        break;
    case GAME_CAMPAIGN:  
        SetGameType( g_StringTableMgr( "ui", "IDS_GAMETYPE_CAMPAIGN" ) );
        SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_CAMPAIGN" ) );     
        break;
    case GAME_MP:
        break;
    default:    ASSERTS(FALSE, "Unknown gametype");
    }
}

//==============================================================================

game_type game_config::GetGameTypeID ( void )
{
    return (game_type)m_Config.GameTypeID;
}

//==============================================================================

void game_config::SetShortGameType ( const char* pShortGameType )
{
    x_mstrcpy( m_Config.ShortGameType, pShortGameType );
}

//==============================================================================

void game_config::SetShortGameType ( const xwchar* pShortGameType )
{
    x_wstrcpy( m_Config.ShortGameType, pShortGameType );
}

//==============================================================================

const xwchar* game_config::GetShortGameType ( void )
{
    return m_Config.ShortGameType;
}

//==============================================================================

void game_config::SetGameType ( const char* pGameType )
{
    x_mstrcpy( m_Config.GameType, pGameType );
}

//==============================================================================

void game_config::SetGameType ( const xwchar* pGameType )
{
    x_wstrcpy( m_Config.GameType, pGameType );
}

//==============================================================================

const xwchar* game_config::GetGameType ( void )
{
    return m_Config.GameType;
}

//==============================================================================

void game_config::SetAIType ( s32 AIType )
{
    m_Config.AIType = AIType;
}

//==============================================================================

s32 game_config::GetAIType ( void )
{
    return m_Config.AIType;
}

//==============================================================================

void game_config::SetNumberOfRounds ( s32 NumberOfRounds )
{
    m_Config.NumRounds = NumberOfRounds;
}

//==============================================================================

s32 game_config::GetNumberOfRounds ( void )
{
    return m_Config.NumRounds;
}

//==============================================================================

void game_config::SetFirePercent ( s32 FirePercent )
{
    m_Config.FirePercent = FirePercent;
}

//==============================================================================

s32 game_config::GetFirePercent ( void )
{
    return m_Config.FirePercent;
}

//==============================================================================

void game_config::SetScoreLimit ( s32 ScoreLimit )
{
    m_Config.ScoreLimit = ScoreLimit;
}

//==============================================================================

s32 game_config::GetScoreLimit ( void )
{
    return m_Config.ScoreLimit;
}

//==============================================================================

void game_config::SetVotePercent ( s32 VotePercent )
{
    m_Config.VotePassPercent = VotePercent;
}

//==============================================================================

s32 game_config::GetVotePercent ( void )
{
    return m_Config.VotePassPercent;
}

//==============================================================================

void game_config::SetEliminationMode ( xbool Enabled )
{
    SetFlag( SERVER_ELIMINATION_MODE, Enabled );
}

//==============================================================================

xbool game_config::GetEliminationMode ( void )
{
    return GetFlag( SERVER_ELIMINATION_MODE );
}

//==============================================================================

void game_config::SetGameTime ( s32 Time )
{
    m_Config.GameTime = Time;
}

//==============================================================================

s32 game_config::GetGameTime ( void )
{
    return m_Config.GameTime;
}

//==============================================================================

void game_config::SetSystemID ( s32 SystemID )
{
    m_Config.ID = SystemID;
}

//==============================================================================

s32 game_config::GetSystemID ( void )
{
    return m_Config.ID;
}

//==============================================================================

void game_config::SetVersion ( s32 Version )
{
    m_Config.Version = Version;
}

//==============================================================================

s32 game_config::GetVersion ( void )
{
    return m_Config.Version;
}

//==============================================================================

void game_config::SetFlag( server_flags FlagMask, xbool Value )
{
    if( Value )
    {
        m_Config.Flags |= FlagMask;
    }
    else
    {
        m_Config.Flags &= ~FlagMask;
    }
}

//==============================================================================

xbool game_config::GetFlag( server_flags FlagMask )
{
    return (m_Config.Flags & FlagMask)!=0;
}

//==============================================================================

void game_config::SetVoiceEnabled ( xbool IsEnabled )
{
    m_Config.VoiceEnabled = IsEnabled;
    SetFlag( SERVER_VOICE_ENABLED, IsEnabled );
}

//==============================================================================

xbool game_config::IsVoiceEnabled ( void )
{
    return GetFlag( SERVER_VOICE_ENABLED );
}

//==============================================================================

void game_config::SetMapScalingEnabled ( xbool IsEnabled )
{
    SetFlag( SERVER_ENABLE_MAP_SCALING, IsEnabled );
}

//==============================================================================

xbool game_config::IsMapScalingEnabled ( void )
{
    return GetFlag( SERVER_ENABLE_MAP_SCALING );
}

//==============================================================================

void game_config::SetMutationMode( mutation_mode MutationMode )
{
    m_Config.MutationMode = MutationMode;
}

//==============================================================================

mutation_mode game_config::GetMutationMode( void )
{
    return m_Config.MutationMode;
}


//==============================================================================
 
void game_config::SetPrivateServer( xbool bIsPrivate )
{
    if( bIsPrivate )
        m_Config.Flags |= SERVER_IS_PRIVATE;
    else
        m_Config.Flags &= ~SERVER_IS_PRIVATE;    
}

//==============================================================================

xbool game_config::GetPrivateServer( void )
{
    return m_Config.Flags & SERVER_IS_PRIVATE;
}

//==============================================================================

void game_config::SetSkillLevel( skill_level SkillLevel )
{
    m_Config.SkillLevel = SkillLevel;
}

//==============================================================================

skill_level game_config::GetSkillLevel( void )
{
    return m_Config.SkillLevel;
}

//==============================================================================

void game_config::SetPasswordEnabled ( xbool IsEnabled )
{
    m_Config.PasswordEnabled = IsEnabled;
    SetFlag( SERVER_HAS_PASSWORD, IsEnabled );
}

//==============================================================================

xbool game_config::IsPasswordEnabled ( void )
{
    return GetFlag( SERVER_HAS_PASSWORD );
}

//==============================================================================

void game_config::SetFriendlyFire ( xbool IsEnabled )
{
    SetFlag( SERVER_FRIENDLY_FIRE, IsEnabled );
}

//==============================================================================

xbool game_config::IsFriendlyFireEnabled( void )
{
    return GetFlag( SERVER_FRIENDLY_FIRE );
}

//==============================================================================

const xwchar* game_config::GetLevelName ( void )
{
    return m_Config.MissionName;
}

//==============================================================================

void game_config::SetPassword ( const char* pPassword )
{
    ASSERT( x_strlen( pPassword ) < sizeof(m_Config.Password) );
    x_strcpy( m_Config.Password, pPassword );
}

//==============================================================================

const char* game_config::GetPassword ( void )
{
    return m_Config.Password;
}

//==============================================================================

void game_config::SetServerName ( const xwchar* pName )
{
    ASSERT( x_wstrlen(pName) < sizeof(m_Config.Name)/sizeof(xwchar) );
    x_wstrcpy( m_Config.Name, pName );
}

//==============================================================================

const xwchar* game_config::GetServerName ( void )
{
    return m_Config.Name;
}

//==============================================================================

void game_config::SetRemoteAddress ( const net_address& Address )
{
    m_Config.Remote = Address;
}

//==============================================================================

const net_address& game_config::GetRemoteAddress ( void )
{
    return m_Config.Remote;
}

//==============================================================================

void game_config::SetFragLimit ( s32 Limit )
{
    m_Config.FragLimit = Limit;
}

//==============================================================================

s32 game_config::GetFragLimit ( void )
{
    return m_Config.FragLimit;
}

//==============================================================================

s32 game_config::GetFlags ( void )
{
    return m_Config.Flags;
}

//==============================================================================

const char* game_config::GetLevelPath( void )
{
    const map_entry* pEntry;

    pEntry = g_MapList.Find( m_Config.Level );
    ASSERT( pEntry );
    return pEntry->GetFilename();
}


//==============================================================================

s32 game_config::GetLevelChecksum( void )
{
    return m_LevelChecksum;
}

//==============================================================================

void game_config::SetLevelChecksum( s32 Checksum )
{
    m_LevelChecksum = Checksum;
}

//==============================================================================

void game_config::SetExitReason( exit_reason Reason )
{
    if( Reason != GAME_EXIT_CONTINUE )
    {
        LOG_MESSAGE( "game_config::SetExitReason", "Exit reason set to:%s", GetExitReasonName(Reason) );
    }   
    // NETWORK_DOWN overrides ALL error conditions. The Xbox will probably need to override the
    // duplicate login stuff too. The only thing that resets it is GAME_EXIT_CONTINUE.
    if( m_ExitReason==GAME_EXIT_NETWORK_DOWN )
    {
        if( Reason==GAME_EXIT_CONTINUE )
        {
            m_ExitReason = GAME_EXIT_CONTINUE;
        }
    }
    else
    {
        m_ExitReason = Reason;
    }
}

//==============================================================================

exit_reason game_config::GetExitReason( void )
{
    return m_ExitReason;
}

//==============================================================================

const char* GetExitReasonName(exit_reason Reason)
{
    switch(Reason)
    {
    case GAME_EXIT_ADVANCE_LEVEL:       return "GAME_EXIT_ADVANCE_LEVEL";
    case GAME_EXIT_RELOAD_LEVEL:        return "GAME_EXIT_RELOAD_LEVEL";
    case GAME_EXIT_RELOAD_CHECKPOINT:   return "GAME_EXIT_RELOAD_CHECKPOINT";
    case GAME_EXIT_PLAYER_DROPPED:      return "GAME_EXIT_PLAYER_DROPPED";
    case GAME_EXIT_PLAYER_KICKED:       return "GAME_EXIT_PLAYER_KICKED";
    case GAME_EXIT_CONNECTION_LOST:     return "GAME_EXIT_CONNECTION_LOST";
    case GAME_EXIT_LOGIN_REFUSED:       return "GAME_EXIT_LOGIN_REFUSED";
    case GAME_EXIT_BAD_PASSWORD:        return "GAME_EXIT_BAD_PASSWORD";
    case GAME_EXIT_CLIENT_BANNED:       return "GAME_EXIT_CLIENT_BANNED";
    case GAME_EXIT_SERVER_FULL:         return "GAME_EXIT_SERVER_FULL";
    case GAME_EXIT_PLAYER_QUIT:         return "GAME_EXIT_PLAYER_QUIT";
    case GAME_EXIT_GAME_COMPLETE:       return "GAME_EXIT_GAME_COMPLETE";
    case GAME_EXIT_INVALID_MISSION:     return "GAME_EXIT_INVALID_MISSION";
    case GAME_EXIT_CANNOT_CONNECT:      return "GAME_EXIT_CANNOT_CONNECT";
    case GAME_EXIT_NETWORK_DOWN:        return "GAME_EXIT_NETWORK_DOWN";
    case GAME_EXIT_SERVER_BUSY:         return "GAME_EXIT_SERVER_BUSY";
    case GAME_EXIT_SERVER_SHUTDOWN:     return "GAME_EXIT_SERVER_SHUTDOWN";
    case GAME_EXIT_FOLLOW_BUDDY:        return "GAME_EXIT_FOLLOW_BUDDY";
    case GAME_EXIT_SECURITY_QUERY:      return "GAME_EXIT_SECURITY_QUERY";
    case GAME_EXIT_SECURITY_FAILED:     return "GAME_EXIT_SECURITY_FAILED";
    case GAME_EXIT_SESSION_ENDED:       return "GAME_EXIT_SESSION_ENDED";
    case GAME_EXIT_DUPLICATE_LOGIN:     return "GAME_EXIT_DUPLICATE_LOGIN";
    case GAME_EXIT_CONTINUE:            return "GAME_EXIT_CONTINUE";
    default:                            ASSERT(FALSE);  return "<unknown>";
    }
}

//==============================================================================
