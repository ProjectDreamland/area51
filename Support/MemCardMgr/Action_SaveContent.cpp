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

void MemCardMgr::MC_STATE_SAVE_CONTENT( void )
{
    condition& Pending = GetPendingCondition(m_iCard);

    // handle unformatted cards ***********************************************

    if( Pending.bUnformatted )
    {
        //  "Memory card (8MB) (for Playstation®2)\n"
        //  "in MEMORY CARD slot 1 is unformatted.\n"
        //  "Format memory card (8MB)(for Playstation®2)?\n"),

        const xwchar* pText;
        if( ! m_iCard )
            pText = g_StringTableMgr( "ui", "MC_FORMAT_PROMPT_SLOT1" );
        else
            pText = g_StringTableMgr( "ui", "MC_FORMAT_PROMPT_SLOT2" );

        ChangeState( __id MC_STATE_ASK_FORMAT );
        OptionBox(
            g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
            pText,
            g_StringTableMgr( "ui", "IDS_MEMCARD_YES"    ),
            g_StringTableMgr( "ui", "IDS_MEMCARD_NO"     )
        );
        return;
    }

    // Enter content directory *********************************************************

    if( ! Pending.ErrorCode )
    {
       
        ChangeState( __id MC_STATE_SAVE_CONTENT_SET_DIR_WAIT );

        g_MemcardMgr.AsyncSetDirectory( xfs( "%s%s", m_SavePrefix, m_ContentPostfix ) );

        // Put up warning .....................................................

        m_bForcePoll[m_iCard] = true;
        return;
    }
    PopState();
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_SAVE_CONTENT_CREATE_DIR_WAIT( void )
{
    switch( GetMcResult( ))
    {
        case kPENDING:
            return;

        case kSUCCESS:
        {
            MC_STATE_SAVE_CONTENT_SET_DIR_WAIT();
            return;
        }

        case kFAILURE:
        case kRESET:
        {
            // we failed! display a message to the user!
            ChangeState( __id MC_STATE_CREATE_PROFILE_FAILED );
            return;
        }

        default:
            ASSERT(0);
    }
    PopState();
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_SAVE_CONTENT_SET_DIR_WAIT( void )
{
    switch( GetMcResult( ))
    {
        case kPENDING:
            return;

        case kSUCCESS:
        {
            ChangeState( __id MC_STATE_SAVE_CONTENT_WRITE_WAIT );
            s32 Length;

            void* pData = g_MatchMgr.GetDownloadData( Length );
            ASSERT( pData );
            ASSERT( Length );

            g_MemcardMgr.SetIconDisplayName( "Content"  );
            g_MemcardMgr.AsyncWriteFile( m_PreservedProfile[MAX_PLAYER_SLOTS-1].Dir, (byte*)pData, Length );
            return;
        }

        case kRESET:
        case kFAILURE:
            g_MemcardMgr.AsyncCreateDirectory( xfs( "%s%s", m_SavePrefix, m_ContentPostfix ) );
            ChangeState( __id MC_STATE_SAVE_CONTENT_CREATE_DIR_WAIT );
            return;

        default:
            ASSERT(0);
    }
    PopState();
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_SAVE_CONTENT_WRITE_WAIT( void )
{
    //
    // Render progress bar for save.

    // Render a rectangle.
    UpdateProgress( g_MemcardMgr.GetProgress() );
    switch( GetMcResult( ))
    {
        case kPENDING:
            return;

        case kSUCCESS:
        {
            // Now we write out the manifest.
            map_list& Manifest = m_CardManifest[m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID];
            s32         i;

            Manifest.SetVersion( m_Manifest.GetVersion() );
            // Then we copy the entries associated with the map that we have just downloaded and are about
            // to save.
            for( i=0; i<m_Manifest.GetCount(); i++ )
            {
                if( m_Manifest[i].GetMapID() == m_PreservedProfile[MAX_PLAYER_SLOTS-1].ProfileID )
                {
                    Manifest.Append( m_Manifest[i], &m_Manifest );
                }
            }

            xstring Temp = Manifest.Serialize();
            m_Manifest.Clear();

            AllocBuffer( MAX_MANIFEST_SIZE );
            ASSERT( x_strlen(Temp) < MAX_MANIFEST_SIZE );
            x_strcpy( (char*)m_pLoadBuffer, (const char*)Temp );
            g_MemcardMgr.AsyncWriteFile( "A51MANIFEST", (byte*)m_pLoadBuffer, MAX_MANIFEST_SIZE );
            ChangeState( __id MC_STATE_SAVE_MANIFEST_WRITE_WAIT );
        }
        return;

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

void MemCardMgr::MC_STATE_SAVE_MANIFEST_WRITE_WAIT( void )
{
    switch( GetMcResult( ))
    {
    case kPENDING:
        return;

    case kSUCCESS:
        {
            // Now we write out the manifest.
            m_bForcePoll[m_iCard] = TRUE;
            FreeBuffer();
        }
        break;

    case kFAILURE:
        FreeBuffer();
        break;

    case kRESET:
        FreeBuffer();
        ResetAction();
        return;

    default:
        ASSERT(0);
    }

    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_SAVE_CONTENT_FAILED( void )
{
    const xwchar* pText;
    if( ! m_iCard )
        pText = g_StringTableMgr( "ui", "MC_SAVE_FAILED_RETRY_SLOT1" );
    else
        pText = g_StringTableMgr( "ui", "MC_SAVE_FAILED_RETRY_SLOT2" );

    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"   ),  
        pText,
        TRUE
        );

    FlushStateStack();
    PushState( __id MC_STATE_DONE           );
    PushState( __id MC_STATE_UNMOUNT        );
    PushState( __id MC_STATE_FINISH         );
    return;
}

//==---------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//  Memory card action methods ( entry point )
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_SAVE_CONTENT( void )
{
    InitAction( MEMCARD_SAVE_MODE );

    // setup for writing ******************************************************

    m_iCard    = m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID;

    const xwchar* pText;
    if( ! m_iCard )
        pText = g_StringTableMgr( "ui", "MC_SAVING_TO_MEMCARD_SLOT1" );
    else
        pText = g_StringTableMgr( "ui", "MC_SAVING_TO_MEMCARD_SLOT2" );

    xwstring        Title;
    const map_info* pMapInfo    = m_Manifest.GetMapInfo(  m_PreservedProfile[MAX_PLAYER_SLOTS-1].ProfileID );
    ASSERT( pMapInfo );

    Title = g_StringTableMgr( "ui", "IDS_SAVE" );
    Title += ' ';
    Title += xwstring(pMapInfo->DisplayName);

    WarningBox( Title, pText, FALSE );

    // push states for card two ***********************************************

    PushState( __id MC_STATE_MOUNT             );
    PushState( __id MC_STATE_LOAD_MANIFEST     );
    PushState( __id MC_STATE_SAVE_CONTENT      );
    PushState( __id MC_STATE_UNMOUNT           );
    PushState( __id MC_STATE_FINISH            );
}

