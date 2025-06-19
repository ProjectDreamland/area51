///////////////////////////////////////////////////////////////////////////////
// 
// Action_SaveProfile.cpp
// Wed Feb 26 11:43:28 2003
// 
///////////////////////////////////////////////////////////////////////////////
#include "x_types.hpp"
#include "e_Memcard.hpp"
#include "MemCardMgr/MemCardMgr.hpp"


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

void MemCardMgr::MC_STATE_SAVE_PROFILE( void )
{
    condition& Pending = GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID);
    if( ! Pending.ErrorCode )
    {
        ChangeState( __id MC_STATE_SAVE_PROFILE_SET_DIR_WAIT );

        //  "Saving data.  Do not remove memory card"
        //  "(8MB) (for Playstation®2) in MEMORY CARD"
        //  "slot 1, reset, or switch off the console."

        const xwchar* pText;
        if( ! m_PreservedProfile[m_iPlayer].CardID )
            pText = g_StringTableMgr( "ui", "MC_OVERWRITE_DATA_SLOT1" );
        else
            pText = g_StringTableMgr( "ui", "MC_OVERWRITE_DATA_SLOT2" );
        WarningBox(
            g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"   ),  
            pText,
            FALSE
        );
#ifdef TARGET_XBOX
        g_MemcardMgr.AsyncSetDirectory( m_PreservedProfile[m_iPlayer].Dir );
#elif defined(TARGET_PC)
        g_MemcardMgr.AsyncSetDirectory( "" ); //We dont using settings folders on PC.
#endif
        m_bForcePoll[m_iCard] = true;

        return;
    }
    else
    {
        ChangeState( __id MC_STATE_SAVE_PROFILE_FAILED );
        return;
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_SAVE_PROFILE_SET_DIR_WAIT( void )
{
    switch( GetMcResult( ))
    {
        case kPENDING:
            return;

        case kSUCCESS:
        {
            ChangeState( __id MC_STATE_PROFILE_WRITE_WAIT );

            s32 RoundedSize = (sizeof( player_profile ) + 1023 ) & ~1023;
            AllocBuffer( RoundedSize );

            player_profile* pProfile = (player_profile*)m_pLoadBuffer;

            x_memcpy( m_pLoadBuffer, &g_StateMgr.GetPendingProfile(), sizeof( player_profile ) );

            pProfile->Checksum();
            g_MemcardMgr.SetIconDisplayName( pProfile->GetProfileName() );

            x_encrypt( pProfile, sizeof(player_profile), m_EncryptionKey );
            g_MemcardMgr.AsyncWriteFile( (const char* )m_PreservedProfile[m_iPlayer].Dir, (byte*)m_pLoadBuffer, RoundedSize );
            return;
        }

        case kFAILURE:
        case kRESET:
            ChangeState( __id MC_STATE_SAVE_PROFILE_FAILED );
            return;

        default:
            ASSERT(0);
    }
    PopState();
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_PROFILE_WRITE_WAIT( void )
{
    switch( GetMcResult( ))
    {
        case kPENDING:
            return;

        case kSUCCESS:
            // Actually set the checksum within the packet when the write completes
            FreeBuffer();
            g_StateMgr.GetPendingProfile().Checksum();           
            ChangeState( __id MC_STATE_SAVE_PROFILE_SUCCESS );
            return;

        case kFAILURE:
        case kRESET:
            FreeBuffer();
            ChangeState( __id MC_STATE_SAVE_PROFILE_FAILED );
            return;

        default:
            ASSERT(0);
            PopState();
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_SAVE_PROFILE_FAILED(void)
{
    xwstring MessageText;
    xwstring NavText;
    
    condition& Pending = GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID);
    
#if defined(TARGET_XBOX)
    m_BlocksRequired = ( (g_StateMgr.GetProfileSaveSize() - Pending.BytesFree) + 16383 ) / 16384;

    MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1_PROFILE_XBOX" )), m_BlocksRequired ) );
    NavText     = g_StringTableMgr( "ui", "IDS_NAV_DONT_FREE_BLOCKS" );
    
    xbool SecondOption = FALSE;
    if( GameMgr.GameInProgress() == FALSE )
    {
        NavText    += g_StringTableMgr( "ui", "IDS_NAV_FREE_MORE_BLOCKS" );
        SecondOption = TRUE;
    }
    
    PopUpBox( 
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),
        MessageText, 
        NavText, 
        TRUE, 
        SecondOption, 
        FALSE );
#elif defined(TARGET_PC)
    m_BlocksRequired = ( (g_StateMgr.GetProfileSaveSize() - Pending.BytesFree) + 1023 ) / 1024;
   
    MessageText = xwstring(xfs((const char*)xstring(g_StringTableMgr("ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1_PROFILE_XBOX")), m_BlocksRequired));
    NavText = g_StringTableMgr("ui", "IDS_OK");
    
    PopUpBox( 
        g_StringTableMgr("ui", "IDS_DISK_SPACE_HEADER"),
        MessageText, 
        NavText, 
        TRUE, 
        FALSE, 
        FALSE);
#endif

    FlushStateStack();
    PushState( __id MC_STATE_SAVE_PROFILE_FAILED_WAIT );
    PushState( __id MC_STATE_UNMOUNT                  );
    PushState( __id MC_STATE_FINISH                   );
    return;
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_SAVE_PROFILE_FAILED_WAIT( void ) 
{
    // wait for user response
    condition& Pending = GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID);

    switch( m_MessageResult )
    {
    case DLG_MCMESSAGE_IDLE:
        return;

    case DLG_MCMESSAGE_NO:
        // continue without saving
        Pending.bCancelled = TRUE;
        break;

    case DLG_MCMESSAGE_YES:
        // retry
        break;
    }

    // finish processing
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_SAVE_PROFILE_SUCCESS( void )
{
    // reset timer
    m_CardWait = 0;

    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
        g_StringTableMgr( "ui", "MC_SAVE_SUCCESS"  ),
        FALSE
        );

    ChangeState( __id MC_STATE_SAVE_PROFILE_SUCCESS_WAIT );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_SAVE_PROFILE_SUCCESS_WAIT( void )
{
    // wait for message to timeout
    m_CardWait++;

    // timeout?
    if( m_CardWait >= 5 )
    {
        // force a poll to update the profile list
        m_bForcePoll[m_iCard] = true;
        ChangeState( __id MC_STATE_TRAWL_DIRS );            
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_DONE( void )
{
    switch( m_MessageResult )
    {
        case DLG_MCMESSAGE_IDLE:
            return;

        default:
            PopState();
            break;
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_OVERWRITE_CONFIRM( void )
{
    const xwchar* pText = g_StringTableMgr( "ui", "MC_PROMPT_FOR_OVERWRITE" );

    ChangeState( __id MC_STATE_OVERWRITE_CONFIRM_WAIT );

    OptionBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
        pText,
        g_StringTableMgr( "ui", "IDS_MEMCARD_YES"    ),
        g_StringTableMgr( "ui", "IDS_MEMCARD_NO"     )
        );

    return;
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_OVERWRITE_CONFIRM_WAIT( void )
{
    switch( m_MessageResult )
    {
        case DLG_MCMESSAGE_YES:
        {
            // recheck the card before starting the format (make sure it wasn't changed etc)
            ChangeState( __id MC_STATE_OVERWRITE_RECHECK );
            g_MemcardMgr.AsyncMount( m_PreservedProfile[m_iPlayer].CardID );
            break;
        }

        case DLG_MCMESSAGE_IDLE:
            break;

        case DLG_MCMESSAGE_NO:
        {
            // Cancel overwrite
            condition& Pending = GetPendingCondition( m_PreservedProfile[m_iPlayer].CardID );
            Pending.bCancelled = TRUE;
            FlushStateStack();
            PushState( __id MC_STATE_UNMOUNT );
            PushState( __id MC_STATE_FINISH  );
            break;
        }

        default:
            break;
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_OVERWRITE_RECHECK( void )
{
    switch( GetMcResult() )
    {
        case kPENDING:
            return;

        case kRESET:
        case kFAILURE:
        {
            // Card was changed, or other error, abort operation
            ChangeState(  __id MC_STATE_SAVE_PROFILE_FAILED );
            return;
        }

        case kSUCCESS:
        {
            ChangeState(  __id MC_STATE_OVERWRITE_PROFILE );
            return;
        }
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_OVERWRITE_PROFILE( void )
{
    condition& Pending = GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID);
    if( ! Pending.ErrorCode )
    {
        ChangeState( __id MC_STATE_SAVE_PROFILE_SET_DIR_WAIT );

        //  "Saving data.  Do not remove memory card"
        //  "(8MB) (for Playstation®2) in MEMORY CARD"
        //  "slot 1, reset, or switch off the console."

        const xwchar* pText;
        if( ! m_PreservedProfile[m_iPlayer].CardID )
            pText = g_StringTableMgr( "ui", "MC_OVERWRITE_DATA_SLOT1" );
        else
            pText = g_StringTableMgr( "ui", "MC_OVERWRITE_DATA_SLOT2" );
        WarningBox(
            g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"   ),  
            pText,
            FALSE
            );

        if( GetCondition(m_PreservedProfile[m_iPlayer].CardID).InfoList.GetCount() >= m_iProfile )
        {
            m_PreservedProfile[m_iPlayer] = GetCondition(m_PreservedProfile[m_iPlayer].CardID).InfoList[m_iProfile];
            ChangeState( __id MC_STATE_OVERWRITE_SUCCESS );
            return;
        }
        else
        {
            ChangeState( __id MC_STATE_SAVE_PROFILE_FAILED );
            return;
        }

        // update fields in preserved profile with the correct info
        player_profile& Profile      = g_StateMgr.GetPendingProfile();
        m_PreservedProfile[m_iPlayer].Ver       = Profile.GetVersion();
        m_PreservedProfile[m_iPlayer].Name      = xwstring(Profile.GetProfileName());
        m_PreservedProfile[m_iPlayer].Hash      = Profile.GetHash();
        m_bForcePoll[m_iCard]        = true;

        return;
    }
    else
    {
        ChangeState( __id MC_STATE_SAVE_PROFILE_FAILED );
        return;
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_OVERWRITE_SUCCESS( void )
{
    // reset timer
    m_CardWait = 0;

    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
        g_StringTableMgr( "ui", "MC_OVERWRITE_SUCCESS"  ),
        FALSE
        );

    ChangeState( __id MC_STATE_OVERWRITE_SUCCESS_WAIT );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_OVERWRITE_SUCCESS_WAIT( void )
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
//==---------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
//  Memory card action methods ( entry point )
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_SAVE_PROFILE( void )
{
    InitAction( MEMCARD_SAVE_MODE );

    // setup for writing ******************************************************
    m_iCard        = m_PreservedProfile[m_iPlayer].CardID;
    m_CardDataMode = SM_CARDMODE_PROFILE;


    // push states for card two ***********************************************

    PushState( __id MC_STATE_MOUNT                );
    PushState( __id MC_STATE_SAVE_PROFILE         );
    PushState( __id MC_STATE_UNMOUNT              );
    PushState( __id MC_STATE_FINISH               );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_OVERWRITE_PROFILE( void )
{
    InitAction( MEMCARD_SAVE_MODE );

    // setup for writing ******************************************************
    m_iCard = m_PreservedProfile[m_iPlayer].CardID;

    // push states for card two ***********************************************

    PushState( __id MC_STATE_MOUNT             );
    PushState( __id MC_STATE_OVERWRITE_CONFIRM );
    PushState( __id MC_STATE_UNMOUNT           );
    PushState( __id MC_STATE_FINISH            );
}

//==---------------------------------------------------------------------------

