///////////////////////////////////////////////////////////////////////////////
// 
// Action_PollCards.cpp
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

void MemCardMgr::MC_STATE_TRAWL_CHECK_CARD( void )
{
    // reset timer
    m_CardWait = 0;
    ChangeState( __id MC_STATE_TRAWL_CHECK_CARD_WAIT );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_TRAWL_CHECK_CARD_WAIT( void )
{
    if( GetMcResult() != kPENDING )
    {
        ChangeState( __id MC_STATE_TRAWL_DIRS_WAIT );
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_TRAWL_CHECK_CARD_HOLD( void )
{
    // wait for "checking for card" message to timeout
    m_CardWait++;

    // timeout?
    if( m_CardWait >= 5 )
        PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::ClearProfileNames( s32 CardID )
{
    condition& Pending = GetPendingCondition(CardID);
    xarray< profile_info >& InfoList = Pending.InfoList;
    s32 n = InfoList.GetCount();
    if( n )
    {
        for( s32 i=0;i<n;i++ )
        {
            profile_info& Info = InfoList[i];
            Info.Name.Clear();
        }
        InfoList.Clear();
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_TRAWL_DIRS( void )
{
    condition& Pending = GetPendingCondition(m_iCard);

    // store bytes free for this card
    Pending.BytesFree = g_MemcardHardware.GetFreeSpace();

    // update availability states *****************************************
    if( m_MemcardMode == MEMCARD_CHECK_MODE )
    {
        if( Pending.bUnformatted || Pending.bNoCard || Pending.bDamaged )
        {
            Pending.bInsufficientSpace = false;
            Pending.bIsFull            = false;
        }
        else
        {
            u32 spaceRequired = 0;

            switch( m_CardDataMode )
            {
            case SM_CARDMODE_PROFILE:
                spaceRequired = g_StateMgr.GetProfileSaveSize();
                break;
            case SM_CARDMODE_SETTINGS:
                spaceRequired = g_StateMgr.GetSettingsSaveSize();
                break;
            case SM_CARDMODE_CONTENT:
                spaceRequired = 100 *1024; // TODO: Need size of DLC to be passed in
                break;
            }

            Pending.bInsufficientSpace = !( g_MemcardMgr.IsSpaceAvailable( spaceRequired ) );
            Pending.bIsFull            = !( g_MemcardMgr.IsSpaceAvailable( 1024 ) );
        }
    }

    // if memory card changed *************************************************
    if( ( Pending.bCardHasChanged || m_bForcePoll[m_iCard] ) && (!( Pending.bDamaged || Pending.bUnformatted || Pending.bNoCard )) )
    {
        // Clear file list ....................................................
        ClearProfileNames( m_iCard );

        // Get files list .....................................................
        g_MemcardHardware.SetRootDir  ();
        g_MemcardMgr.AsyncReadFileList();

        // Set availability bits ..............................................
        Pending.bCardHasChanged = false;
        m_bForcePoll[m_iCard] = false;

        // initialize card settings data
        for(s32 i=0; i<MAX_CARD_SLOTS; i++)
        {
            m_bCardHasSettings[i] = FALSE;
        }

        // Set poll in progress flag
        m_bPollInProgress = TRUE;

        ChangeState( __id MC_STATE_TRAWL_CHECK_CARD );

        return;
    }
    else
    {
        // because we're skipping the whole process of trawling
        // all the directories we need to duplicate everything
        // that isn't state related to the pending condition.

        condition& Current = GetCondition(m_iCard);
        Pending.InfoList   = Current.InfoList;

        // clear poll flag
        m_bForcePoll[m_iCard] = false;
    }

    // if no card clear list **************************************************

    if( Pending.ErrorCode )
    {
        if( !( Pending.bFull || Pending.bInsufficientSpace ) )
        {
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
    }

    // onto the next state (tally ho) *****************************************

    PopState();
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_TRAWL_DIRS_WAIT( void )
{
    condition& Pending = GetPendingCondition(m_iCard);

    switch( GetMcResult())
    {
    case kPENDING:
        return;

    case kSUCCESS:
        {
            // grab filenames *************************************************

            xarray< mc_file_info >DirList;
            g_MemcardMgr.GetFileList( DirList );
            s32 n = DirList.GetCount();
            if( n )
            {
                ChangeState( __id MC_STATE_GET_PROFILE_NAMES );

                xarray<profile_info>& InfoList = Pending.InfoList;
                InfoList.SetCount(0);

                s32  ProfileID,i;
                for( ProfileID=i=0;i<n;i++ )
                {
                    xstring String (DirList[i].FileName);
#ifdef TARGET_XBOX
                    if(String.Find( "Profile " ) != -1 )
#elif defined(TARGET_PC)
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
                return;
            }
            else
            {
                // no profiles, still look for settings!
                ChangeState( __id MC_STATE_FIND_SETTINGS );
                return;
            }
            break;
        }

    case kFAILURE:
        ChangeState( __id MC_STATE_TRAWL_CHECK_CARD_HOLD );
        return;

    case kRESET:
        ResetAction();
        return;

    default:
        ASSERT(0);
    }
    PopState();
}



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_GET_PROFILE_NAMES( void )
{
    condition& Pending = GetPendingCondition(m_iCard);
    if( m_iDir < Pending.InfoList.GetCount())
    {
        ChangeState( __id MC_STATE_GET_PROFILE_NAMES_WAIT );
#ifdef TARGET_XBOX
        g_MemcardMgr.AsyncSetDirectory( Pending.InfoList[m_iDir].Dir );
#elif defined(TARGET_PC)
        g_MemcardMgr.AsyncSetDirectory( "" ); //We dont using folders on PC.
#endif
        return;
    }

    ChangeState( __id MC_STATE_FIND_SETTINGS );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_GET_PROFILE_NAMES_WAIT( void )
{
    // Get profile info *******************************************************

    condition& Pending = GetPendingCondition(m_iCard);
    profile_info& Info = Pending.InfoList[m_iDir];

    // Check status ***********************************************************

    switch( GetMcResult())
    {
    case kPENDING:
        break;

    case kSUCCESS:
        {
            ChangeState( __id MC_STATE_PROFILE_NAME_WAIT );
            s32 RoundedSize = (sizeof( player_profile )+1023)&~1023;;
            AllocBuffer( RoundedSize );
            g_MemcardMgr.AsyncReadFile( Pending.InfoList[m_iDir].Dir, (byte*)m_pLoadBuffer, RoundedSize );
            break;
        }

    case kFAILURE:
        Info.bDamaged = true;
#ifdef TARGET_XBOX
        Pending.ErrorCode = 0;
#endif
        // keep looking
        ChangeState( __id MC_STATE_PROFILE_NAME_WAIT );
        break;

    case kRESET:
        ResetAction();
        break;

    default:
        ASSERT(0);
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_PROFILE_NAME_WAIT( void )
{
    // Get profile info *******************************************************

    condition   & Pending = GetPendingCondition(m_iCard);
    profile_info& Info    = Pending.InfoList[ m_iDir ];
    player_profile* pProfile = (player_profile*)m_pLoadBuffer;

    // Check status ***********************************************************

    switch( GetMcResult())
    {
    case kPENDING:
        return;

    case kSUCCESS:
        // decrypt the whole file.  Need to detect corruption early
        x_decrypt( pProfile, sizeof(player_profile), m_EncryptionKey );
        if( pProfile->Validate() == FALSE )
        {
            // damaged, get the name from the directory instead
            s32 Count = x_strlen( Info.Dir );
            Count -= 8;
            Info.Name = Info.Dir.Right(Count);
            Info.bDamaged = true;
            Pending.ErrorCode = 0;
            FreeBuffer();
        }
        else
        {
            Info.Name       = xwstring( pProfile->GetProfileName() );
            Info.Hash       = pProfile->GetHash();
            Info.Ver        = pProfile->GetVersion();
            Info.bDamaged   = false;
            FreeBuffer();
        }

        // get the next one
        ChangeState( __id MC_STATE_GET_PROFILE_NAMES );
        m_iDir++;
        return;

    case kFAILURE:
        {
            s32 Count = x_strlen( Info.Dir );
            Count -= 8;
            Info.Name = Info.Dir.Right(Count);
            Pending.ErrorCode = 0;
        }
        if( !Info.bDamaged )
        {
            Info.bDamaged = true;
        }
        // this is one is damaged, but keep checking the others!
        FreeBuffer();
        ChangeState( __id MC_STATE_GET_PROFILE_NAMES );
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

void MemCardMgr::MC_STATE_FIND_SETTINGS( void )
{
#ifdef TARGET_XBOX
        g_MemcardMgr.AsyncSetDirectory( "Game Settings" );
#elif defined(TARGET_PC)
        g_MemcardMgr.AsyncSetDirectory( "" ); //We dont using folders on PC.
#endif
    ChangeState( __id MC_STATE_FIND_SETTINGS_WAIT );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FIND_SETTINGS_WAIT( void )
{
    // Check status ***********************************************************
    condition& Pending = GetPendingCondition(m_iCard);

    switch( GetMcResult())
    {
    case kPENDING:
        return;

    case kSUCCESS:
        {
            // set found settings flag (even if there is an error because bad data can be overwritten)
            m_bFoundSettings = TRUE;
            // set settings found for this card
            m_bCardHasSettings[m_iCard] = TRUE;
        }

    case kFAILURE:
    case kRESET:
        // We don't care if the settings are not found.
        Pending.bFileNotFound = 0;
        break;

    default:
        ASSERT(0);
    }

    ChangeState( __id MC_STATE_TRAWL_CHECK_CARD_HOLD );
}

//==---------------------------------------------------------------------------


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

void MemCardMgr::MC_ACTION_POLL_CARDS( void )
{
    InitAction( MEMCARD_CHECK_MODE );

    // setup for polling
    m_iProfile = 0;
    m_iCard    = 0;
    
    // push states for card one
    PushState( __id MC_STATE_MOUNT      );
    PushState( __id MC_STATE_TRAWL_DIRS );
    PushState( __id MC_STATE_UNMOUNT    );
    PushState( __id MC_STATE_FINISH     );
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_ACTION_REPOLL_CARDS( void )
{
    InitAction( MEMCARD_CHECK_MODE );
    m_iCard    = 0;

    // push states for card one
    PushState( __id MC_STATE_MOUNT      );
    PushState( __id MC_STATE_UNMOUNT    );
    PushState( __id MC_STATE_FINISH     );
}