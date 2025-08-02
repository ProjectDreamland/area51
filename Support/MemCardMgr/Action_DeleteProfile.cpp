///////////////////////////////////////////////////////////////////////////////
// 
// Action_DeleteProfile.cpp
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

void MemCardMgr::MC_STATE_DELETE_PROFILE( void )
{
    // delete profile *********************************************************

    condition& Pending = GetPendingCondition(m_iCard);
    if( ! Pending.ErrorCode )
    {
        ChangeState( __id MC_STATE_DELETE_PROFILE_WAIT );

        if( ! m_PreservedProfile[0].CardID )
        {
            WarningBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),
                g_StringTableMgr( "ui", "MC_DELETING_DATA_SLOT1" ),
                FALSE
            );
        }
        else
        {
            WarningBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
                g_StringTableMgr( "ui", "MC_DELETING_DATA_SLOT2" ),
                FALSE
            );
        }
#ifdef TARGET_XBOX
        g_MemcardMgr.AsyncDeleteDirectory( m_PreservedProfile[0].Dir );
#elif defined(TARGET_PC)
        g_MemcardMgr.AsyncDeleteFile( m_PreservedProfile[0].Dir ); //We dont using settings folders on PC.
#endif
        return;
    }
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_DELETE_PROFILE_WAIT( void )
{
    switch( GetMcResult( ))
    {
        case kPENDING:
            return;

        case kSUCCESS:
            ChangeState( __id MC_STATE_DELETE_PROFILE_SUCCESS );
            return;

        case kFAILURE:
            ChangeState( __id MC_STATE_DELETE_PROFILE_FAILED );
            return;

        case kRESET:
            ChangeState( __id MC_STATE_DELETE_PROFILE_FAILED );
            return;

        default:
            ASSERT(0);
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_DELETE_PROFILE_FAILED( void )
{
    const xwchar* pText;
    if( ! m_PreservedProfile[0].CardID )
        pText = g_StringTableMgr( "ui", "MC_DELETE_FAILED_RETRY_SLOT1" );
    else
        pText = g_StringTableMgr( "ui", "MC_DELETE_FAILED_RETRY_SLOT2" );
    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),
        pText,
        TRUE
    );

    FlushStateStack();
    PushState( __id MC_STATE_DONE    );
    PushState( __id MC_STATE_UNMOUNT );
    PushState( __id MC_STATE_FINISH  );
    return;
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_DELETE_PROFILE_SUCCESS( void )
{
    // reset timer
    m_CardWait = 0;

    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
        g_StringTableMgr( "ui", "MC_DELETE_SUCCESS"  ),
        FALSE
        );

    ChangeState( __id MC_STATE_DELETE_PROFILE_SUCCESS_WAIT );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_DELETE_PROFILE_SUCCESS_WAIT( void )
{
    // wait for message to timeout
    m_CardWait++;

    // timeout?
    if( m_CardWait >= 5 )
    {
        m_bForcePoll[m_iCard] = true;
        ChangeState( __id MC_STATE_TRAWL_DIRS );
    }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Memory card action methods ( entry point )
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_DELETE_PROFILE( void )
{
    m_iCard         = m_PreservedProfile[0].CardID;
    m_CardDataMode  = SM_CARDMODE_PROFILE;

    InitAction( MEMCARD_DELETE_MODE );

    // push states ************************************************************

    PushState( __id MC_STATE_MOUNT                  );
    PushState( __id MC_STATE_DELETE_PROFILE         );
    PushState( __id MC_STATE_UNMOUNT                );
    PushState( __id MC_STATE_FINISH                 );
}
