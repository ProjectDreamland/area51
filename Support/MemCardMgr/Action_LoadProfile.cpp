///////////////////////////////////////////////////////////////////////////////
// 
// Action_LoadProfile.cpp
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

void MemCardMgr::MC_STATE_LOAD_PROFILE_SET_DIR( void )
{
    condition& Pending = GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID);
    if( ! Pending.ErrorCode )
    {
        // get profile info from front condition ..............................
        // preserve object for later ..................................
        ChangeState( __id MC_STATE_LOAD_PROFILE_SET_DIR_WAIT );
#ifdef TARGET_XBOX
        g_MemcardMgr.AsyncSetDirectory( m_PreservedProfile[m_iPlayer].Dir );
#elif defined(TARGET_PC)
        g_MemcardMgr.AsyncSetDirectory( "" ); //We dont using settings folders on PC.
#endif
        return;
    }
    else
    {
        ChangeState( __id MC_STATE_LOAD_PROFILE_FAILED );
        return;
    }
}


//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_PROFILE_SET_DIR_WAIT( void )
{
    switch( GetMcResult( ))
    {
        case kPENDING:
            return;

        case kSUCCESS:
        {
            ChangeState( __id MC_STATE_PROFILE_READ_WAIT );

            s32 RoundedSize = (sizeof( player_profile )+1023)&~1023;;
            AllocBuffer( RoundedSize );

            g_MemcardMgr.AsyncReadFile( (const char* )m_PreservedProfile[m_iPlayer].Dir, (byte*)m_pLoadBuffer, RoundedSize );
            return;
        }

        case kFAILURE:
        case kRESET:
            ChangeState( __id MC_STATE_LOAD_PROFILE_FAILED );
            return;

        default:
            ASSERT(0);
    }
    PopState();
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_PROFILE_READ_WAIT( void )
{
    switch( GetMcResult( ))
    {
        case kPENDING:
            return;

        case kSUCCESS:
        {
            player_profile& Profile = g_StateMgr.GetPendingProfile();
            {
                player_profile* pLoadedProfile=(player_profile*)m_pLoadBuffer;
                // copy into pending profile ..................................
                x_decrypt( pLoadedProfile, sizeof(player_profile), m_EncryptionKey );
                if( pLoadedProfile->Validate() == FALSE )
                {
                    condition& Pending = GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID);
                    Pending.bDamaged = TRUE;
                    ChangeState( __id MC_STATE_LOAD_PROFILE_FAILED );
                    FreeBuffer();
                    return;
                }
                else
                {
                    x_memcpy( &Profile, pLoadedProfile, sizeof(player_profile) );
                    FreeBuffer();
                }

            }
            break;
        }

        case kFAILURE:
        case kRESET:
        {
            FreeBuffer();
            condition& Pending = GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID);
            Pending.bDamaged = TRUE;
            ChangeState( __id MC_STATE_LOAD_PROFILE_FAILED );
            return;
        }

        default:
            ASSERT(0);
    }
    PopState();
}


//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_PROFILE_FAILED( void )
{
    condition& Pending = GetPendingCondition(m_PreservedProfile[m_iPlayer].CardID);
    const xwchar* pText;

    //if( Pending.bDamaged )
    //{
    //    if( ! m_PreservedProfile[m_iPlayer].CardID )
    //        pText = g_StringTableMgr( "ui", "MC_LOAD_FAILED_FAULTY_SLOT1" );
    //    else
    //        pText = g_StringTableMgr( "ui", "MC_LOAD_FAILED_FAULTY_SLOT2" );
    //}
    //else 
    if( Pending.bFileNotFound )
    {
        if( ! m_PreservedProfile[m_iPlayer].CardID )
            pText = g_StringTableMgr( "ui", "MC_DATA_CORRUPT_SLOT1" );
        else
            pText = g_StringTableMgr( "ui", "MC_DATA_CORRUPT_SLOT2" );
    }
    else
    {
        if( ! m_PreservedProfile[m_iPlayer].CardID )
            pText = g_StringTableMgr( "ui", "MC_LOAD_FAILED_RETRY_SLOT1" );
        else
            pText = g_StringTableMgr( "ui", "MC_LOAD_FAILED_RETRY_SLOT2" );
    }

    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),
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

void MemCardMgr::MC_STATE_LOAD_PROFILE_SUCCESS( void )
{
    // reset timer
    m_CardWait = 0;

    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),  
        g_StringTableMgr( "ui", "MC_LOAD_SUCCESS"  ),
        FALSE
        );

    ChangeState( __id MC_STATE_LOAD_PROFILE_SUCCESS_WAIT );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_PROFILE_SUCCESS_WAIT( void )
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

void MemCardMgr::MC_ACTION_LOAD_PROFILE( void )
{
    InitAction( MEMCARD_LOAD_MODE );
    m_iCard = m_PreservedProfile[m_iPlayer].CardID;

    //  "Loading data. Do not remove memory card"
    //  "(8MB) (for Playstation®2) in MEMORY CARD"
    //  "slot 1, reset, or switch off the console."

    const xwchar* pText;
    if( ! m_iCard )
        pText = g_StringTableMgr( "ui", "MC_LOADING_FROM_MEMCARD_SLOT1" );
    else
        pText = g_StringTableMgr( "ui", "MC_LOADING_FROM_MEMCARD_SLOT2" );
    WarningBox(
        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"   ),  
        pText,
        FALSE
    );

    // setup for writing ******************************************************

    // push states for card two ***********************************************

    PushState( __id MC_STATE_MOUNT                );
    PushState( __id MC_STATE_LOAD_PROFILE_SET_DIR );
    PushState( __id MC_STATE_LOAD_PROFILE_SUCCESS );
    PushState( __id MC_STATE_UNMOUNT              );
    PushState( __id MC_STATE_FINISH               );
}
