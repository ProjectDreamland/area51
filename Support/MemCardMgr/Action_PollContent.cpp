///////////////////////////////////////////////////////////////////////////////
// 
// Action_PollContent.cpp
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



///////////////////////////////////////////////////////////////////////////////
//
//  Memory card action methods ( entry point )
//
//  The polling action works by initially reading all the directory names. This
//  has the fortunate side effect of establishing condition for both cards. Our
//  next task is to change directory and load the profile names. It is achieved
//  by utilising two separate actions.
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_POLL_CONTENT( void )
{
    InitAction( MEMCARD_CHECK_MODE );
    m_iCard    = 0;

    PushState( __id MC_STATE_MOUNT          );
    PushState( __id MC_STATE_LOAD_MANIFEST  );
    PushState( __id MC_STATE_UNMOUNT        );
    PushState( __id MC_STATE_FINISH         );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_MANIFEST( void )
{
    condition& Pending = GetPendingCondition(m_iCard);

    if( ( Pending.bCardHasChanged || m_bForcePoll[m_iCard] ) && (!( Pending.bDamaged || Pending.bUnformatted )) )
    {
            // First, enter the manifest directory
        g_MemcardMgr.AsyncSetDirectory( xfs("%s%s", m_SavePrefix, m_ContentPostfix) );
        ChangeState( __id MC_STATE_LOAD_MANIFEST_SET_DIR_WAIT );
        m_CardManifest[ m_iCard ].Clear();
        // because we're skipping the whole process of trawling
        // all the directories we need to duplicate everything
        // that isn't state related to the pending condition.

        condition& Current = GetCondition(m_iCard);
        Pending.InfoList   = Current.InfoList;
    }
    else
    {
        PopState();
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_MANIFEST_SET_DIR_WAIT( void )
{
    switch( GetMcResult( ))
    {
    case kPENDING:
        return;

    case kSUCCESS:
        {
            AllocBuffer( MAX_MANIFEST_SIZE );
            g_MemcardMgr.AsyncReadFile( "A51MANIFEST", (byte*)m_pLoadBuffer, MAX_MANIFEST_SIZE );
            ChangeState( __id MC_STATE_LOAD_MANIFEST_READ_WAIT );
            return;
        }

        // We don't care if the profile scan fails.
    case kFAILURE:
    case kRESET:
        break;

    default:
        ASSERT(0);
    }
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_MANIFEST_READ_WAIT( void )
{
    global_settings& Settings = g_StateMgr.GetActiveSettings();

    switch( GetMcResult( ))
    {
    case kPENDING:
        return;

    case kSUCCESS:
        m_CardManifest[m_iCard].Parse( (const char*)m_pLoadBuffer, MF_DOWNLOAD_MAP, m_iCard );
        if( m_CardManifest[m_iCard].GetVersion() > Settings.GetContentVersion() )
        {
            Settings.SetContentVersion( m_CardManifest[m_iCard].GetVersion() );
            g_MatchMgr.SetLocalManifestVersion( m_CardManifest[m_iCard].GetVersion() );
        }
        break;

        // We don't care if the profile scan fails.
    case kFAILURE:
    case kRESET:
        break;

    default:
        ASSERT(0);
    }
    FreeBuffer();
    PopState();
}
