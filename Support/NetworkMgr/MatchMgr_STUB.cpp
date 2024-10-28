//==============================================================================
//
// MatchMgr.cpp - Matchup manager interface functions.
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

// this is stub match manager for PC

#if !defined(TARGET_PC)
#error This should only be included for PC gamespy support.
#endif
#include "x_files.hpp"
#include "x_threads.hpp"
#include "e_Network.hpp"
#include "NetworkMgr/MatchMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/GameSpy/callbacks.hpp"
#include "StateMgr/StateMgr.hpp"
#include "ServerVersion.hpp"
#include "Configuration/GameConfig.hpp"
#include "x_log.hpp"
#include "../Apps/GameApp/Config.hpp"

//=========================================================================
//  Defines
//=========================================================================
//#define ENABLE_LAN_LOOKUP

#define LABEL_STRING(x) case x: return ( #x );
// Authentication data filename. This is short for obfuscation reason.

const s32   GAMESPY_PRODUCT_ID  = 10384;
const char* GAMESPY_GAMENAME    = "area51ps2";
const char* GAMESPY_SECRETKEY   = "eR48fP";
const char* EMAIL_POSTFIX       = "a51";

extern s32 g_Changelist;

#ifdef GSI_UNICODE
#define _T(a) L##a
#define _tprintf wprintf
#else
#define _T(a) a
#define _tprintf x_DebugMsg
#endif

//=========================================================================
//  External function and data prototypes
//=========================================================================

extern const char* MANIFEST_LOCATION;
extern const char* HDD_MANIFEST_FILENAME;

const char* TIMESTAMP_FILENAME = "HDD:lasttime.txt";
#if !defined(ENABLE_LAN_LOOKUP)
/* todo BISCUIT - this needs data for the other product codes. */
static unsigned char SLUS_20595_pass_phrase[] = { 0xb9, 0xfd, 0xb2, 0xce, 0x5c, 0xd9, 0x0e, 0x0c };
static unsigned char SLES_52570_pass_phrase[] = { 0xb9, 0xfd, 0xb2, 0xce, 0x5c, 0xd9, 0x0e, 0x0c };
static unsigned char SLES_53075_pass_phrase[] = { 0xb9, 0xfd, 0xb2, 0xce, 0x5c, 0xd9, 0x0e, 0x0c };
#endif

static byte* s_pAuthData;
#if defined(X_DEBUG) && defined(bwatson)
// This will create a temporary patch
static void MakePatch( void );
#endif


//------------------------------------------------------------------------------
xbool match_mgr::Init( net_socket& Local, const net_address Broadcast )
{
    return TRUE;
}

//------------------------------------------------------------------------------
void  match_mgr::Kill                ( void )
{
}

//------------------------------------------------------------------------------
void  match_mgr::Update( f32 DeltaTime )
{
}

//------------------------------------------------------------------------------
void match_mgr::Reset( void )
{
}

//------------------------------------------------------------------------------
void match_mgr::UpdateState( f32 DeltaTime)
{
}

//------------------------------------------------------------------------------
// Called whenever we transition from one state to another
void match_mgr::ExitState( match_mgr_state OldState )
{
}

//------------------------------------------------------------------------------
// Called whenever we transition from one state to another
void match_mgr::EnterState( match_mgr_state NewState )
{
}

//------------------------------------------------------------------------------
void match_mgr::CheckVisibility(void)
{

}

//------------------------------------------------------------------------------
void match_mgr::StartAcquisition( match_acquire AcquisitionMode )
{
}

//------------------------------------------------------------------------------
xbool match_mgr::IsAcquireComplete( void )
{
    return FALSE;
}

//------------------------------------------------------------------------------
// SetState will change internal state and initiate any packets that are
// required for the processing of the state just entered.

void match_mgr::SetState( match_mgr_state NewState )
{
}

//------------------------------------------------------------------------------
xbool match_mgr::ReceivePacket( net_address& Remote, bitstream& Bitstream )
{
    return FALSE;
}

//------------------------------------------------------------------------------
// This append server instance is called when a fully complete response is received
// via the matchmaking service.
void match_mgr::AppendServer( const server_info& Response )
{
}

//------------------------------------------------------------------------------
void match_mgr::LocalLookups(f32 DeltaTime)
{
}
//------------------------------------------------------------------------------
f32 match_mgr::GetPingTime(s32 Index)
{
    return 0.0f;
}

//------------------------------------------------------------------------------
void match_mgr::DisconnectFromMatchmaker(void)
{
}
//------------------------------------------------------------------------------
void match_mgr::ConnectToMatchmaker( match_mgr_state PendingState )
{
}

//==============================================================================

xbool match_mgr::ValidateLobbyInfo( const lobby_info &info )
{
    return TRUE;
}

//==============================================================================
static s32 GetHex( const char* &pChallenge, s32 Count )
{
    s32 Value;
    s32 Digit;

    Value = 0;
    while( Count )
    {
        Digit = *pChallenge-'0';
        if( Digit < 0 )
        {
            return 0;
        }
        if( Digit > 9 )
        {
            Digit -= ('a'-'9'-1);
        }
        if( Digit > 15 )
        {
            return 0;
        }

        Value = Value * 16 + Digit;
        Count--;
        pChallenge++;
    }
    return Value;
}

//==============================================================================
xbool match_mgr::CheckSecurityChallenge( const char* pChallenge )
{
    return FALSE;
}

//==============================================================================

void match_mgr::IssueSecurityChallenge(void)
{
}

//==============================================================================

void match_mgr::NotifyKick(const char* UniqueId)
{
    (void)UniqueId;
}

//==============================================================================

void match_mgr::RemoteLookups( f32 DeltaTime )
{
    (void)DeltaTime;
}

//==============================================================================

void match_mgr::SetUserAccount( s32 UserIndex )
{
}

//==============================================================================

s32 match_mgr::GetAuthResult( char* pLabelBuffer )
{
    return 0;
}

//==============================================================================

void match_mgr::BecomeClient( void )
{
}

//==============================================================================

void match_mgr::MarkBuddyPresent( const net_address& Remote )
{
}

//==============================================================================

const extended_info* match_mgr::GetExtendedServerInfo( s32 Index )
{
    return NULL;
}

//==============================================================================

void match_mgr::InitDownload( const char* pURL )
{
}

//==============================================================================

void match_mgr::KillDownload( void )
{
}

//==============================================================================

download_status match_mgr::GetDownloadStatus( void )
{
    return DL_STAT_OK;
}

//==============================================================================

f32 match_mgr::GetDownloadProgress( void )
{
    return 0.0f;
}

//==============================================================================

void* match_mgr::GetDownloadData( s32& Length )
{
    return nullptr;
}

//==============================================================================

xbool match_mgr::AddBuddy( const buddy_info& Buddy )
{
    return TRUE;
}

//==============================================================================

xbool match_mgr::DeleteBuddy( const buddy_info& Buddy )
{
    xbool Result;
    Result = FALSE;
    return Result;
}

//==============================================================================

void match_mgr::AnswerBuddyRequest( buddy_info& Buddy, buddy_answer Answer )
{
}

//==============================================================================

xbool match_mgr::InviteBuddy( buddy_info& Buddy )
{
    return FALSE;
}

//==============================================================================

void match_mgr::CancelBuddyInvite( buddy_info& Buddy )
{
}

//==============================================================================

xbool match_mgr::AnswerBuddyInvite( buddy_info& Buddy, buddy_answer Answer )
{
    return FALSE;
}

//==============================================================================
xbool match_mgr::JoinBuddy( buddy_info& Buddy )
{
    return FALSE;
}

//==============================================================================
const char* match_mgr::GetConnectErrorMessage( void )
{
    return "";
}

//==============================================================================

xbool match_mgr::IsPlayerMuted( u64 Identifier )
{
    (void)Identifier;
    return( FALSE );
}

//==============================================================================

void match_mgr::SetIsPlayerMuted( u64 Identifier, xbool IsMuted )
{
    (void)Identifier;
    (void)IsMuted;
}

//==============================================================================
void match_mgr::SendFeedback( u64 Identifier, const char* pName, player_feedback Type )
{
    (void)Identifier;
    (void)pName;
    (void)Type;

    ASSERTS( FALSE, "Not implemented yet" );
}

//==============================================================================
void match_mgr::StartLogin(void)
{
}

//==============================================================================
void match_mgr::StartIndirectLookup(void)
{
}
