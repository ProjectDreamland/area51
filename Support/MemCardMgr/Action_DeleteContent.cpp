///////////////////////////////////////////////////////////////////////////////
// 
// Action_SaveContent.cpp
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

void MemCardMgr::MC_STATE_DELETE_CONTENT( void )
{

    map_list& MapList           = GetManifest( m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID );
    const map_info* pMapInfo    = MapList.GetMapInfo( m_PreservedProfile[MAX_PLAYER_SLOTS-1].ProfileID );
    ASSERT( pMapInfo );

    m_PreservedProfile[MAX_PLAYER_SLOTS-1].Dir      = pMapInfo->Filename;

    MapList.RemoveByMapID( m_PreservedProfile[MAX_PLAYER_SLOTS-1].ProfileID );

    g_MemcardMgr.AsyncSetDirectory( xfs( "%s%s", m_SavePrefix, m_ContentPostfix ) );
    ChangeState( __id MC_STATE_DELETE_CONTENT_SET_DIR_WAIT );

    m_bForcePoll[m_iCard] = true;
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_DELETE_CONTENT_SET_DIR_WAIT( void )
{
    switch( GetMcResult( ))
    {
    case kPENDING:
        return;

    case kSUCCESS:
        {
            // Now we actually delete the file we wanted to delete.
            g_MemcardMgr.AsyncDeleteFile( (const char*)m_PreservedProfile[MAX_PLAYER_SLOTS-1].Dir );
            ChangeState( __id MC_STATE_DELETE_CONTENT_DELETE_WAIT );
            return;
        }
        break;

    case kFAILURE:
        break;

    case kRESET:
        ResetAction();
        return;

    default:
        ASSERT(0);
    }

    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_DELETE_CONTENT_DELETE_WAIT( void )
{
    // We are hijacking the SAVE_CONTENT_WRITE_WAIT as this performs exactly what we need
    // to do once the delete has completed. i.e. the manifest is serialized, then saved
    // to the memory card.
    MC_STATE_SAVE_CONTENT_WRITE_WAIT();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_DELETE_CONTENT_CONFIRM( void )
{
    map_list& MapList           = GetManifest( m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID );
    const map_info* pMapInfo    = MapList.GetMapInfo( m_PreservedProfile[MAX_PLAYER_SLOTS-1].ProfileID );
    ASSERT( pMapInfo );

    ChangeState( __id MC_STATE_DELETE_CONTENT_CONFIRM_WAIT );
    xwstring Title;

    Title = g_StringTableMgr( "ui", "IDS_DELETE" );
    Title += ' ';
    Title += xwstring(pMapInfo->DisplayName);

    OptionBox( Title,  
                g_StringTableMgr( "ui", "IDS_DL_DELETE_CONFIRM" ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_YES"    ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_NO"     )
        );

    return;
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_DELETE_CONTENT_CONFIRM_WAIT( void )
{
    switch( m_MessageResult )
    {
    case DLG_MCMESSAGE_YES:
        {
            map_list& MapList           = GetManifest( m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID );
            const map_info* pMapInfo    = MapList.GetMapInfo( m_PreservedProfile[MAX_PLAYER_SLOTS-1].ProfileID );
            ASSERT( pMapInfo );

            const xwchar* pText;
            xwstring Title;

            if( ! m_iCard )
                pText = g_StringTableMgr( "ui", "MC_DELETING_DATA_SLOT1" );
            else
                pText = g_StringTableMgr( "ui", "MC_DELETING_DATA_SLOT2" );

            Title = g_StringTableMgr( "ui", "IDS_DELETE" );
            Title += ' ';
            Title += xwstring(pMapInfo->DisplayName);

            WarningBox( Title, pText, FALSE );

            // Go on to next state for deleting
            PopState();
            return;
            break;
        }

    case DLG_MCMESSAGE_IDLE:
        break;

    case DLG_MCMESSAGE_NO:
        {
            // Cancel delete
            condition& Pending = GetPendingCondition( m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID );
            Pending.bCancelled = TRUE;
            FlushStateStack();
            PushState( __id MC_STATE_FINISH  );
            break;
        }

    default:
        break;
    }
}

//==---------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//  Memory card action methods ( entry point )
//
///////////////////////////////////////////////////////////////////////////////

//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_DELETE_CONTENT( void )
{
    InitAction( MEMCARD_SAVE_MODE );
    m_iCard = m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID;
    m_Manifest.Clear();
    // Put up warning .....................................................

    PushState( __id MC_STATE_DELETE_CONTENT_CONFIRM );
    PushState( __id MC_STATE_MOUNT                  );
    PushState( __id MC_STATE_LOAD_MANIFEST          );
    PushState( __id MC_STATE_DELETE_CONTENT         );
    PushState( __id MC_STATE_UNMOUNT                );
    PushState( __id MC_STATE_FINISH                 );
}