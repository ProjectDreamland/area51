///////////////////////////////////////////////////////////////////////////////
// 
// Action_LoadContent.cpp
// Wed Feb 26 11:43:28 2003
// 
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//
//  Includes
//
///////////////////////////////////////////////////////////////////////////////
#include "MemCardMgr.hpp"
#include "e_Memcard.hpp"
#include "StringMgr/StringMgr.hpp"
#include "Dialogs/dlg_MCMessage.hpp"



///////////////////////////////////////////////////////////////////////////////
//
// Globals
//
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//
//  Memory card state methods
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_CONTENT( void )
{
    condition& Pending = GetPendingCondition(m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID);
    if( ! Pending.ErrorCode )
    {
        ChangeState( __id MC_STATE_LOAD_CONTENT_SET_DIR_WAIT );
        g_MemcardMgr.AsyncSetDirectory( (const char*)xfs("%s%s", m_SavePrefix, m_ContentPostfix) );
        return;
    }
    else
    {
        ChangeState( __id MC_STATE_LOAD_CONTENT_FAILED );
        return;
    }
}


//==---------------------------------------------------------------------------
void MemCardMgr::MC_STATE_LOAD_CONTENT_READ_WAIT( void )
{
    switch( GetMcResult( ))
    {
    case kPENDING:
        return;

    case kSUCCESS:
        break;

        // We don't care if the profile scan fails.
    case kFAILURE:
    case kRESET:
        FreeBuffer();
        ChangeState( __id MC_STATE_LOAD_CONTENT_FAILED );
        break;

    default:
        ASSERT(0);
    }
    PopState();
}

//==---------------------------------------------------------------------------
void MemCardMgr::MC_STATE_LOAD_CONTENT_GET_SIZE_WAIT( void )
{
    switch( GetMcResult( ))
    {
    case kPENDING:
        return;

    case kSUCCESS:
        {

            s32 RoundedLength = (g_MemcardMgr.GetFileLength() + 1023) &~1023;
            AllocBuffer( RoundedLength );
            m_LoadBufferSize = g_MemcardMgr.GetFileLength();
            g_MemcardMgr.AsyncReadFile( (const char* )m_PreservedProfile[MAX_PLAYER_SLOTS-1].Dir, (byte*)m_pLoadBuffer, RoundedLength );
            ChangeState( __id MC_STATE_LOAD_CONTENT_READ_WAIT );
            return;
        }

        // If the get size fails, we're screwed.
    case kFAILURE:
    case kRESET:
        ChangeState( __id MC_STATE_LOAD_CONTENT_FAILED );
        return;
        break;

    default:
        ASSERT(0);
    }
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_CONTENT_FAILED( void )
{

    if( m_pMessage )
    {
        ASSERT( m_pMessage == (ui_dialog*)g_UiMgr->GetTopmostDialog( g_UiUserID ) );
        g_UiMgr->EndDialog( g_UiUserID, TRUE );
        m_pMessage = NULL;
    }
    g_ActiveConfig.SetExitReason( GAME_EXIT_INVALID_MISSION );
    FlushStateStack();
    PushState( __id MC_STATE_DONE           );
    PushState( __id MC_STATE_UNMOUNT        );
    PushState( __id MC_STATE_FINISH         );
    return;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Memory card action methods ( entry point )
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_LOAD_CONTENT( void )
{
    InitAction( MEMCARD_LOAD_MODE );

    m_iCard = m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID;

    //  "Loading data. Do not remove memory card"
    //  "(8MB) (for Playstation®2) in MEMORY CARD"
    //  "slot 1, reset, or switch off the console."

    const xwchar* pText;
    if( ! m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID )
        pText = g_StringTableMgr( "ui", "MC_LOADING_FROM_MEMCARD_SLOT1" );
    else
        pText = g_StringTableMgr( "ui", "MC_LOADING_FROM_MEMCARD_SLOT2" );
    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"   ),  
        pText,
        FALSE
    );

    // push states for card two ***********************************************

    PushState( __id MC_STATE_MOUNT                );
    PushState( __id MC_STATE_LOAD_MANIFEST        );
    PushState( __id MC_STATE_LOAD_CONTENT         );
    PushState( __id MC_STATE_UNMOUNT              );
    PushState( __id MC_STATE_FINISH               );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_CONTENT_SET_DIR_WAIT( void )
{
    switch( GetMcResult( ))
    {
    case kPENDING:
        return;

    case kSUCCESS:
        {
            g_MemcardMgr.AsyncGetFileLength( m_PreservedProfile[MAX_PLAYER_SLOTS-1].Dir );
            ChangeState( __id MC_STATE_LOAD_CONTENT_GET_SIZE_WAIT );
            return;
        }

        // We don't care if the profile scan fails.
    case kFAILURE:
    case kRESET:
        ChangeState( __id MC_STATE_LOAD_CONTENT_FAILED );
        return;
        break;

    default:
        ASSERT(0);
    }
    PopState();
}

