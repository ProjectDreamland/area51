///////////////////////////////////////////////////////////////////////////////
//
//  Action_BootCheck.cpp
//  Wed Feb 26 11:43:28 2003
//
//  Quick note on the state machine:
//
//      The queue_machine class is an improvement over the old array and switch
//      mechanism used in the Hobbit. What it does is maintain an array of ptrs
//      to member functions inside MemCardMgr. These methods all follow similar
//      naming conventions to the previous implementation.
//
//      The only difference between the paradigms are we constantly execute the
//      top of the stack. This gets rid of any requirement for a switch. I have
//      arranged each of the actions into separate files for readability.
//
///////////////////////////////////////////////////////////////////////////////
#include "MemCardMgr.hpp"
#include "e_Memcard.hpp"
#include "StringMgr/StringMgr.hpp"
#include "Dialogs/dlg_MCMessage.hpp"



///////////////////////////////////////////////////////////////////////////////
//
//  Includes
//
///////////////////////////////////////////////////////////////////////////////



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

void MemCardMgr::MC_STATE_MOUNT( void )
{
    ASSERT( ! g_MemcardMgr.IsEngaged());
    g_MemcardMgr.Engage(NULL);

    ChangeState( __id MC_STATE_MOUNT_WAIT );

    condition& Condition = GetPendingCondition(m_iCard);
    Condition.Clear();

    g_MemcardMgr.AsyncMount(m_iCard);
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_MOUNT_WAIT( void )
{
    switch( GetMcResult())
    {
        case kPENDING:
            return;

        case kSUCCESS:
        case kRESET:        // pass "card changed" up
            break;

        case kFAILURE:
            // clear profile names
            if( GetPendingCondition(m_iCard).bNoCard )
                ClearProfileNames( m_iCard );
            // clear card settings flag
            m_bCardHasSettings[m_iCard] = FALSE;
            // check if this was the card we previously used for settings
            if( g_StateMgr.GetSettingsCardSlot() == m_iCard )
            {
                // it got changed, so clear the flag
                g_StateMgr.SetSettingsCardSlot(-1);
            }
            break;

        default:
            ASSERT(0);
    }
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_REMOUNT( void )
{
    ASSERT( ! g_MemcardMgr.IsEngaged());
    g_MemcardMgr.Engage(NULL);

    ChangeState( __id MC_STATE_REMOUNT_WAIT );

    condition& Condition = GetPendingCondition(m_iCard);
    Condition.Clear();

    g_MemcardMgr.AsyncMount(m_iCard);
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_REMOUNT_WAIT( void )
{
    switch( GetMcResult())
    {
    case kPENDING:
        return;

    case kSUCCESS:
    case kRESET:        
        break;

    case kFAILURE:
        // card changed- which is expected after rebooting the IOP
        break;

    default:
        ASSERT(0);
    }
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_CHECK_CARD_HOLD( void )
{
    // reset timer
    m_CardWait = 0;
    ChangeState( __id MC_CHECK_CARD_WAIT );
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_CHECK_CARD_WAIT( void )
{
    if( GetMcResult() != kPENDING )
    {
        PopState();
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_CHECK_CARD_HOLD_WAIT( void )
{
    // wait for "checking for card" message to timeout
    m_CardWait++;

    // timeout?
    if( m_CardWait >= 5 )
        PopState();
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_BOOT_CHECK( void )
{
    condition& Pending = GetPendingCondition(m_iCard);

    // store bytes free for this card
    Pending.BytesFree = g_MemcardHardware.GetFreeSpace();

    // scan the card if it's valid
    if( !( Pending.bDamaged || Pending.bUnformatted || Pending.bNoCard ) )
    {
        // set full flag
        Pending.bIsFull = ( Pending.BytesFree <= 1024 );

        // clear file list
        ClearProfileNames( m_iCard );

        // get files list
        g_MemcardHardware.SetRootDir  ();
        g_MemcardMgr.AsyncReadFileList();

        // Set some bits
        Pending.bCardHasChanged = false;

        // Set poll in progress flag
        m_bPollInProgress = TRUE;

        ChangeState( __id MC_STATE_BOOT_CHECK_WAIT );

        return;
    }
    else
    {
        // clear full flag
        Pending.bIsFull = FALSE;

        condition& Current = GetCondition(m_iCard);
        Pending.InfoList   = Current.InfoList;

        // clear the list
        xarray< profile_info >& InfoList = Pending.InfoList;
        s32 i,n = InfoList.GetCount();
        for( i=0;i<n;i++ )
        {
            profile_info& Info = InfoList[i];
            Info.Name.Clear();
            Info.Dir.Clear();
        }
        InfoList.Clear();
    }

    // we're done here!
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_BOOT_CHECK_WAIT( void )
{
    switch( GetMcResult())
    {
        case kPENDING:
            return;

        case kSUCCESS:
        {
            // build a list of profiles
            xarray< mc_file_info >DirList;
            g_MemcardMgr.GetFileList( DirList );
            s32 n = DirList.GetCount();
            if( n )
            {
                condition& Pending = GetPendingCondition(m_iCard);
                xarray<profile_info>& InfoList = Pending.InfoList;
                InfoList.SetCount(0);

                s32  ProfileID,i;
                for( ProfileID=i=0;i<n;i++ )
                {
                    xstring String (DirList[i].FileName);
#ifdef TARGET_XBOX
                    if(String.Find( "Profile " ) != -1 )
#else
                    if(( String.Find( m_SavePrefix ) != -1 ) && ( String.Find( "A510" ) != -1 ))
#endif
                    {
                        profile_info& Info = InfoList.Append();
                        Info.ProfileID     = ProfileID++;
                        Info.CardID        = m_iCard;
                        Info.Dir           = String;
                        Info.CreationDate  = DirList[i].CreationDate;
                        Info.ModifiedDate  = DirList[i].ModifiedDate;
                    }
                }
                m_iDir = 0;

                ChangeState( __id MC_STATE_FIND_PROFILE );
                return;
            }
            break;
        }

        case kFAILURE:
            break;

        default:
            ASSERT(0);
    }
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FIND_PROFILE( void )
{
    condition& Pending = GetPendingCondition( m_iCard );
    if( ( m_iDir < Pending.InfoList.GetCount() ) && ( !m_bFoundProfile ) )
    {
        ChangeState( __id MC_STATE_FIND_PROFILE_WAIT );
        g_MemcardMgr.AsyncSetDirectory( Pending.InfoList[m_iDir].Dir );
        return;
    }
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FIND_PROFILE_WAIT( void )
{
    // Check status
    switch( GetMcResult())
    {
        case kPENDING:
            break;

        case kSUCCESS:
        {
            condition& Pending = GetPendingCondition(m_iCard);
            AllocBuffer( 1024 );
            g_MemcardMgr.AsyncRead( Pending.InfoList[m_iDir].Dir, (byte*)m_pLoadBuffer, 0, 1024 );
            ChangeState( __id MC_STATE_FIND_PROFILE_CHECK );
            break;
        }

        case kFAILURE:
            // this one is damaged, keep looking
            ChangeState( __id MC_STATE_FIND_PROFILE );
            m_iDir++;
            break;

        case kRESET:
            ResetAction();
            break;

        default:
            ASSERT(0);
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FIND_PROFILE_CHECK( void )
{
    // Check status
    switch( GetMcResult())
    {
        case kPENDING:
            return;

        case kSUCCESS:
            // we found a valid profile
            m_bFoundProfile = TRUE;
            FreeBuffer();
            // get out of here
            break;

        case kFAILURE:
            // this is one is damaged, but keep checking the others!
            FreeBuffer();
            ChangeState( __id MC_STATE_FIND_PROFILE );
            m_iDir++;
            return;

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

void MemCardMgr::MC_STATE_UNMOUNT( void )
{
    ChangeState( __id MC_STATE_UNMOUNT_WAIT );
    g_MemcardMgr.AsyncUnmount();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_UNMOUNT_WAIT( void )
{
    // Final chance to reset **************************************************

    switch( GetMcResult())
    {
    case kPENDING:
        return;

    case kSUCCESS:
        break;

    case kFAILURE:
        break;

    case kRESET:
        ResetAction();
        return;

    default:
        ASSERT(0);
    }
    ASSERT( g_MemcardMgr.IsEngaged());
    g_MemcardMgr.Disengage();
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_BOOT_ACTION_DONE( void )
{
    condition& Condition0 = GetPendingCondition(0); // (we've already flipped conditions)
#if defined( TARGET_PC ) || defined( TARGET_XBOX )
    condition& Condition1 = Condition0;
#else
    condition& Condition1 = GetPendingCondition(1);
#endif

    if( m_bFoundProfile && m_bFoundSettings )
    {
        m_bPassedBootCheck = TRUE;
    }
    else if( Condition0.bUnformatted || Condition1.bUnformatted )
    {
        m_bPassedBootCheck = TRUE;
    }
    else if( Condition0.bIsFull && Condition1.bIsFull )
    {
        Condition0.bInsufficientSpace = FALSE;
        Condition1.bInsufficientSpace = FALSE;
    }
    else
    {
        // calculate space required
        s32 spaceRequired = 0;

        if( !m_bFoundProfile )
            spaceRequired += g_StateMgr.GetProfileSaveSize();

        if( !m_bFoundSettings )
            spaceRequired += g_StateMgr.GetSettingsSaveSize();

        if( Condition0.bNoCard || Condition0.bDamaged )
            Condition0.bInsufficientSpace = FALSE;
        else
            Condition0.bInsufficientSpace = ( spaceRequired > Condition0.BytesFree );

        if( Condition1.bNoCard || Condition1.bDamaged )
            Condition1.bInsufficientSpace = FALSE;
        else
            Condition1.bInsufficientSpace = ( spaceRequired > Condition1.BytesFree );
    }

    // On Xbox and probably PC??? we cannot put up any error messages until after the
    // Press START screen. Do we keep the info around so we can
    // display the appropriate prompt at MainMenu?

#if defined( TARGET_PC )
    // Both cards have error
    if( Condition0.ErrorCode && Condition1.ErrorCode )
    {
        // No cards in either
        if( Condition0.bNoCard && Condition1.bNoCard )
        {
            xwstring MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_CARD_FIRST_CHECK" )), (g_StateMgr.GetProfileSaveSize() + g_StateMgr.GetSettingsSaveSize())/1024 ) );
            ChangeState( __id MC_STATE_BOOT_FAILED_ASK );
            OptionBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"     ),  
                MessageText,
                g_StringTableMgr( "ui", "IDS_MEMCARD_RETRY"      ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_CONTINUE"   ),
                NULL,
                FALSE
                );
            return;
        }
    }

    // Card full (slot0)
    if( Condition0.bIsFull && Condition1.ErrorCode )
    {
        if( !m_bPassedBootCheck )
        {
            xwstring MessageText;
            if( m_bFoundSettings )
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT1" )), g_StateMgr.GetProfileSaveSize()/1024 ) );
            }
            else if( m_bFoundProfile )
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT1_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) );
            }
            else
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT1_ALL" )), (g_StateMgr.GetSettingsSaveSize() + g_StateMgr.GetProfileSaveSize())/1024 ) );
            }

            ChangeState( __id MC_STATE_BOOT_FAILED_ASK );
            OptionBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"   ),
                MessageText,
                g_StringTableMgr( "ui", "IDS_MEMCARD_RETRY"    ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_CONTINUE" ),
                NULL,
                FALSE
                );

            return;
        }
    }

    // Card full (slot1) ******************************************************

    if( Condition1.bIsFull && Condition0.ErrorCode )
    {
        if( !m_bPassedBootCheck )
        {
            xwstring MessageText;
            if( m_bFoundSettings )
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT2" )), g_StateMgr.GetProfileSaveSize()/1024 ) );
            }
            else if( m_bFoundProfile )
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT2_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) );
           }
            else
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT2_ALL" )), (g_StateMgr.GetSettingsSaveSize() + g_StateMgr.GetProfileSaveSize())/1024 ) );
            }

            ChangeState( __id MC_STATE_BOOT_FAILED_ASK );
            OptionBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"   ),
                MessageText,
                g_StringTableMgr( "ui", "IDS_MEMCARD_RETRY"    ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_CONTINUE" ),
                NULL,
                FALSE
                );
            return;
        }
    }

    // Insufficient space (slot0) *********************************************

    if( Condition0.bInsufficientSpace && Condition1.ErrorCode )
    {
        if( !m_bPassedBootCheck )
        {
            xwstring MessageText;
            if( m_bFoundSettings )
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT1" )), g_StateMgr.GetProfileSaveSize()/1024 ) );
            }
            else if( m_bFoundProfile )
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT1_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) );
            }
            else
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT1_ALL" )), (g_StateMgr.GetSettingsSaveSize() + g_StateMgr.GetProfileSaveSize())/1024 ) );
            }

            ChangeState( __id MC_STATE_BOOT_FAILED_ASK );
            OptionBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"        ),
                MessageText,
                g_StringTableMgr( "ui", "IDS_MEMCARD_RETRY"         ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_CONTINUE"      ),
                NULL,
                FALSE
                );
            return;
        }
    }

    // Insufficient space (slot1) *********************************************

    if( Condition1.bInsufficientSpace && Condition0.ErrorCode )
    {
        if( !m_bPassedBootCheck )
        {
            xwstring MessageText;
            if( m_bFoundSettings )
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT2" )), g_StateMgr.GetProfileSaveSize()/1024 ) );
            }
            else if( m_bFoundProfile )
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT2_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) );
            }
            else
            {
                MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NO_SPACE_ON_BOOT_SLOT2_ALL" )), (g_StateMgr.GetSettingsSaveSize() + g_StateMgr.GetProfileSaveSize())/1024 ) );
            }

            ChangeState( __id MC_STATE_BOOT_FAILED_ASK );
            OptionBox(
                g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER"        ),
                MessageText,
                g_StringTableMgr( "ui", "IDS_MEMCARD_RETRY"         ),
                g_StringTableMgr( "ui", "IDS_MEMCARD_CONTINUE"      ),
                NULL,
                FALSE
                );
            return;
        }
    }

#endif

    // force a poll next time
    for( s32 i=0; i<MAX_CARD_SLOTS; i++ )
    {
        m_bForcePoll[i] = TRUE;
    }

    // finished!
    PopState();
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_BOOT_FAILED_ASK( void )
{
    switch( m_MessageResult )
    {
        case DLG_MCMESSAGE_IDLE:
            break;

        case DLG_MCMESSAGE_YES:
            ResetAction();
            break;

        case DLG_MCMESSAGE_NO:
            // force a poll next time
            for( s32 i=0; i<MAX_CARD_SLOTS; i++ )
            {
                m_bForcePoll[i] = TRUE;
            }
            // keep marching on
            PopState();
            break;
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_NO_CARD_ASK( void )
{
    switch( m_MessageResult )
    {
    case DLG_MCMESSAGE_IDLE:
        break;

    case DLG_MCMESSAGE_YES:
        ResetAction();
        break;

    case DLG_MCMESSAGE_NO:
        PopState();
        break;
    }
}



///////////////////////////////////////////////////////////////////////////////
//
//  Memory card action methods ( entry point )
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_BOOT_CHECK( void )
{
    InitAction( MEMCARD_CHECK_MODE );

    m_iCard                 = 0;
    m_LastSettingsDatestamp = 0;
    m_bFoundProfile         = FALSE;
    m_bFoundSettings        = FALSE;
    m_bPassedBootCheck      = FALSE;

    // initialize card settings data
    for(s32 i=0; i<MAX_CARD_SLOTS; i++)
    {
        m_bCardHasSettings[i] = FALSE;
    }

    // push states for card one ***********************************************
    PushState( __id MC_STATE_MOUNT            );
    PushState( __id MC_CHECK_CARD_HOLD        );
    PushState( __id MC_STATE_BOOT_CHECK       );
    PushState( __id MC_STATE_LOAD_SETTINGS    );
    PushState( __id MC_CHECK_CARD_HOLD_WAIT   );
    PushState( __id MC_STATE_UNMOUNT          );

    // push states for card two ***********************************************
    PushState( __id MC_STATE_BOOT_ACTION_DONE );
    PushState( __id MC_STATE_FINISH           );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_REBOOT_CHECK( void )
{
    InitAction( MEMCARD_CHECK_MODE );
    m_iCard                 = 0;

    // push states for card one ***********************************************
    PushState( __id MC_STATE_REMOUNT          );
    PushState( __id MC_STATE_UNMOUNT          );

    // push states for card two ***********************************************
    PushState( __id MC_STATE_FINISH           );
}

//========================================================================================

