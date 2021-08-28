///////////////////////////////////////////////////////////////////////////////
// 
// Action_Format.cpp
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

void MemCardMgr::MC_STATE_INIT_FORMAT( void )
{

    //  "Memory card (8MB) (for Playstation®2)\n"
    //  "in MEMORY CARD slot 1 is unformatted.\n"
    //  "Format memory card (8MB)(for Playstation®2)?\n"),
    const xwchar* pText;
    
    if( ! m_PreservedProfile[m_iPlayer].CardID )
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

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_ASK_FORMAT( void )
{
    switch( m_MessageResult )
    {
        case DLG_MCMESSAGE_YES:
        {
            // Start the format.
            // recheck the card before starting the format

            const xwchar* pText;
            if( ! m_PreservedProfile[m_iPlayer].CardID )
                pText = g_StringTableMgr( "ui", "MC_FORMAT_ASK_SLOT1" );
            else
                pText = g_StringTableMgr( "ui", "MC_FORMAT_ASK_SLOT2" );

            ChangeState( __id MC_STATE_FORMAT_CONFIRM );
            OptionBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
                pText,
                g_StringTableMgr( "ui", "IDS_MEMCARD_YES"    ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_NO"     )
                );
            break;
        }

        case DLG_MCMESSAGE_IDLE:
            break;

        case DLG_MCMESSAGE_NO:
            //-- User Does not want to format
            ChangeState( __id MC_STATE_FORMAT_CANCEL );
            //  "Cancel Format ?"
            OptionBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
                g_StringTableMgr( "ui", "MC_FORMAT_CANCEL"   ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_YES"    ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_NO"     )
                );
            break;

        default:
            break;
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FORMAT_CONFIRM( void )
{
    switch( m_MessageResult )
    {
        case DLG_MCMESSAGE_YES:
        {
            // recheck the card before starting the format (make sure it wasn't changed etc)
            ChangeState( __id MC_STATE_WAIT_RECHECK_FORMAT );
            // clear error codes
            GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID).ErrorCode = 0;
            g_MemcardMgr.AsyncMount( m_PreservedProfile[m_iPlayer].CardID );
            break;
        }

        case DLG_MCMESSAGE_IDLE:
            break;

        case DLG_MCMESSAGE_NO:
        {
            //-- User Does not want to format
            ChangeState( __id MC_STATE_FORMAT_CANCEL );

            //-- Cancel Format ?
            OptionBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
                g_StringTableMgr( "ui", "MC_FORMAT_CANCEL"   ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_YES"    ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_NO"     )
                );
            break;
        }

        default:
            break;
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_WAIT_RECHECK_FORMAT( void )
{
    switch( GetMcResult() )
    {
        case kPENDING:
            return;

        case kRESET:
        {
            // Card was changed, abort operation
            ChangeState(  __id MC_STATE_FORMAT_FAILED );
            return;
        }

        case kFAILURE:
        case kSUCCESS:
        {
            if( GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID).bUnformatted )
            {
                //-- "Formatting memory card"
                //-- "(8MB) (for Playstation®2) in MEMORY"
                //-- "CARD slot 1. Do not remove memory card"
                //-- "(8MB) (for Playstation®2), reset, or"
                //-- "switch off console."

                const xwchar* pText;
                if( ! m_PreservedProfile[m_iPlayer].CardID )
                    pText = g_StringTableMgr( "ui", "MC_FORMAT_FORMATTING_SLOT1" );
                else
                    pText = g_StringTableMgr( "ui", "MC_FORMAT_FORMATTING_SLOT2" );
                WarningBox(
                    g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"   ),  
                    pText,
                    FALSE
                    );

                ChangeState( __id MC_STATE_FORMAT_WAIT );
                g_MemcardMgr.AsyncFormat();
                return;
            }
            else
            {
                // Card is not unformatted, abort operation
                ChangeState(  __id MC_STATE_FORMAT_FAILED );
                return;
            }
        }
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FORMAT_WAIT( void )
{
    switch( GetMcResult( ))
    {
        case kPENDING:
            return;

        case kRESET:
        case kFAILURE:
        {
            ChangeState(  __id MC_STATE_FORMAT_FAILED );
            return;
        }

        case kSUCCESS:
        {
            // Format successful
            GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID).bUnformatted = false;
            PopState();
            return;
        }

        default:
            ASSERT(0);
    }
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FORMAT_CANCEL( void )
{
    condition& Pending = GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID);

    switch( m_MessageResult )
    {
        case DLG_MCMESSAGE_YES:
            Pending.bCancelled = TRUE;
            FlushStateStack();
            PushState( __id MC_STATE_UNMOUNT        );
            PushState( __id MC_STATE_FINISH         );
            break;

        case DLG_MCMESSAGE_IDLE:
            break;

        case DLG_MCMESSAGE_NO:
            //-- User Does want to format
            ChangeState( __id MC_STATE_INIT_FORMAT );
            break;
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FORMAT_FAILED( void )
{
    const xwchar* pText;

    if( ! m_PreservedProfile[m_iPlayer].CardID )
        pText = g_StringTableMgr( "ui", "MC_FORMAT_FAILED_SLOT1" );
    else
        pText = g_StringTableMgr( "ui", "MC_FORMAT_FAILED_SLOT2" );

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

void MemCardMgr::MC_STATE_FORMAT_SUCCESS( void )
{
    // reset timer
    m_CardWait = 0;

    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
        g_StringTableMgr( "ui", "MC_FORMAT_SUCCESS"  ),
        FALSE
        );

    ChangeState( __id MC_STATE_FORMAT_SUCCESS_WAIT );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FORMAT_SUCCESS_WAIT( void )
{
    // wait for message to timeout
    m_CardWait++;

    // timeout?
    if( m_CardWait >= 5 )
        PopState();
}


//==---------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Memory card action methods ( entry point )
//
///////////////////////////////////////////////////////////////////////////////

//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_FORMAT( void )
{
    InitAction( MEMCARD_FORMAT_MODE );

    // setup for formatting ***************************************************
    m_iCard = m_PreservedProfile[m_iPlayer].CardID;

    // clear user error states
    condition& Pending = GetPendingCondition(m_iCard);
    Pending.bCancelled = FALSE;

    // push states for this action ********************************************
    PushState( __id MC_STATE_MOUNT          );
    PushState( __id MC_STATE_INIT_FORMAT    );
    PushState( __id MC_STATE_FORMAT_SUCCESS );
    PushState( __id MC_STATE_UNMOUNT        );
    PushState( __id MC_STATE_MOUNT          );

    switch( m_CardDataMode )
    {        
        case SM_CARDMODE_PROFILE:
            PushState( __id MC_STATE_CREATE_PROFILE );
            break;
        case SM_CARDMODE_CONTENT:
            PushState( __id MC_STATE_LOAD_MANIFEST     );
            PushState( __id MC_STATE_SAVE_CONTENT      );
            break;
        case SM_CARDMODE_SETTINGS:
            PushState( __id MC_STATE_SAVE_SETTINGS  );
            break;
        default:
            // bad mode!
            ASSERT(FALSE);
    }

    PushState( __id MC_STATE_UNMOUNT        );
    PushState( __id MC_STATE_FINISH         );
}
