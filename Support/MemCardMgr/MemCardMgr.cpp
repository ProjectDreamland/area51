///////////////////////////////////////////////////////////////////////////////
// 
// MemCardMgr.cpp
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
//  Globals
//
///////////////////////////////////////////////////////////////////////////////



char MemCardErrorStrings[21][32] =
{
    "MEMCARD_SUCCESS",           
    "MEMCARD_IN_PROGRESS",       
    "MEMCARD_FATAL_ERROR",       
    "MEMCARD_BUSY",              
    "MEMCARD_NOT_A_MEMCARD",     
    "MEMCARD_NO_CARD",           
    "MEMCARD_WORN_OUT",          
    "MEMCARD_WRONG_REGION",      
    "MEMCARD_DAMAGED",           
    "MEMCARD_FILE_NOT_FOUND",    
    "MEMCARD_FILE_ALREADY_EXISTS",
    "MEMCARD_NO_FILES_AVAILABLE",
    "MEMCARD_NOT_ENOUGH_SPACE",  
    "MEMCARD_ACCESS_DENIED",     
    "MEMCARD_PAST_END_OF_FILE",  
    "MEMCARD_FILENAME_TOO_LONG", 
    "MEMCARD_IO_CANCELED",       
    "MEMCARD_INCOMPATIBLE",      
    "MEMCARD_CARD_CHANGED",      
    "MEMCARD_UNFORMATTED",
    "MEMCARD_FULL"
};

MemCardMgr      g_UIMemCardMgr;
static s32      s_CardWait = 0; 
xbool           g_bIsCardIn;




///////////////////////////////////////////////////////////////////////////////
//
//  Construction
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::Init( void )
{
    g_MemcardMgr.Init( );

    x_strcpy( m_OptionsPostfix, "SETTINGS" );
    x_strcpy( m_ContentPostfix, "CONTENT" );
    x_strcpy( m_SavePrefix, "AREA-51-" );

    EnableProgress( FALSE );

    m_bFoundProfile     = FALSE;
    m_bFoundSettings    = FALSE;
    m_bPassedBootCheck  = FALSE;
    m_bPollInProgress   = FALSE;
    m_BlocksRequired    = 0;
}



//==---------------------------------------------------------------------------

void MemCardMgr::Kill( void )
{
    g_MemcardMgr.Kill();
}

//==---------------------------------------------------------------------------

MemCardMgr::MemCardMgr( void )
    :
    queue_machine< 64 >( this,__id MC_STATE_IDLE ),
    m_Action           ( this,__id MC_NO_ACTION )
{
    for( s32 i=0;i<MAX_CARD_SLOTS;i++ )
    {
        condition& Pending = GetPendingCondition(i);
        Pending.InfoList.SetCapacity( MAX_PROFILE_CAPACITY );
    }

    m_pLoadBuffer = NULL;
    m_iProfile    = -1;
    m_iDir        =  0;
    m_iPlayer     = -1;
    m_PollTurn[0] =  0;
    m_PollTurn[1] =  0;
    m_iCard       =  0;
    m_iSlot       = -1;

    bWasPolling = false;
    bPolling    = false;
    bAlways     = true;
    random  Random;
    Random.srand( PROFILE_VERSION_NUMBER );

    m_EncryptionKey[0] = (Random.rand() << 16) || Random.rand();
    m_EncryptionKey[1] = (Random.rand() << 16) || Random.rand();
    m_EncryptionKey[2] = (Random.rand() << 16) || Random.rand();
    m_EncryptionKey[3] = (Random.rand() << 16) || Random.rand();
}



//==---------------------------------------------------------------------------

MemCardMgr::~MemCardMgr( void )
{
    if( m_pLoadBuffer )
    {
        x_free( m_pLoadBuffer );
    }
}



///////////////////////////////////////////////////////////////////////////////
//
//  State machine code
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_FINISH( void )
{
    // kill any message dialogs
    if( m_pMessage && ( m_MessageResult == DLG_MCMESSAGE_IDLE ))
    {                      
        s_CardWait = 0;
        g_UiMgr->EndDialog( g_UiUserID, TRUE );
        m_DialogsAllocated--;
        m_pMessage =NULL;
    }
    ActionComplete();
    PopState();
}



///////////////////////////////////////////////////////////////////////////////
//
//  MC Actions
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_NO_ACTION( void )
{
}



///////////////////////////////////////////////////////////////////////////////
//
//  MC States
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_IDLE( void )
{
    if( m_Closure )
    {
        FlushStateStack();
        m_Closure = FALSE;
    }
}



///////////////////////////////////////////////////////////////////////////////
//
//  Other methods
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

xbool MemCardMgr::GetMemCardEngaged( void )
{
    return g_MemcardMgr.IsEngaged();
}



//==---------------------------------------------------------------------------

s32 MemCardMgr::ResetAction( void )
{
    if( g_MemcardMgr.IsEngaged())
        g_MemcardMgr.Disengage();
    m_Action.bDirty = true;
    return 0;
}



//==---------------------------------------------------------------------------

action MemCardMgr::PollState( void )
{
    return m_Action.GetState();
}



///////////////////////////////////////////////////////////////////////////////
//
//  Memory card state handlers
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_NEXT_CARD( void )
{
    #if( MAX_CARD_SLOTS==1 )
    ASSERT(0);
    #endif

    ASSERT( m_iCard < MAX_CARD_SLOTS );
    m_iCard ++;
    PopState();
}



///////////////////////////////////////////////////////////////////////////////
//
//  Memory card callbacks
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::GetProfileNames( xarray< profile_info* >& Result )
{
    ASSERT( Result.GetCapacity()==MAX_PROFILE_CAPACITY );

    // Clear old list *********************************************************

    Result.SetCount(0);
    for( s32 j=0;j<MAX_CARD_SLOTS;j++ )
    {
        xarray< profile_info >& InfoList = GetCondition(j).InfoList;
        s32 n = InfoList.GetCount();
        for( s32 i=0;i<n;i++ )
        {
            profile_info& Info = InfoList[i];
            Result.Append( &Info );
        }
    }
}



//==---------------------------------------------------------------------------

s32 MemCardMgr::Update( f32 DeltaTime )
{
    m_Action.Exec( );
             Exec( );

    g_MemcardMgr.Update( DeltaTime );

    return FALSE;
}



//==---------------------------------------------------------------------------

void MemCardMgr::InitAction( memcard_mode MemcardMode )
{
    FlushStateStack();

    // clear other states .....................................................

    m_MessageResult  = 0;
    m_MemcardMode    = MemcardMode;
    m_ActionDone     = FALSE;
    m_pMessage       = 0;
    m_Closure        = 0;

    // clear error/success states
    condition& Pending = GetPendingCondition(m_iCard);
    Pending.Clear();

    // clear global checksum ..................................................

}



//==---------------------------------------------------------------------------

void MemCardMgr::ActionComplete( void )
{
    // flip conditions ********************************************************

    for( s32 i=0;i<MAX_CARD_SLOTS;i++ )
    {
        condition& Condition = GetCondition(i);
        condition& Pending   = GetPendingCondition(i);
        if( ! Pending.ErrorCode )
              Pending.bComplete = true;

        // retrieve the info for the correct card
        g_MemcardHardware.SetRequestedCard(i);
        // update bytes free
        Condition.BytesFree = Pending.BytesFree = g_MemcardHardware.GetFreeSpace();

        FlipCondition(i);
    }

    // notify callback ********************************************************

    m_bPollInProgress = FALSE;
    m_ActionDone      = TRUE;
    m_Closure         = TRUE;
    bPolling          = 0;

    m_Action.PopState(); // fires off callback
}



//==---------------------------------------------------------------------------

xbool MemCardMgr::HandleFaultyIoOp( void )
{
    condition& Pending = GetPendingCondition(m_iCard);
    if( Pending.bDamaged )
    {
        const xwchar* pText;
        if( ! m_iCard )
            pText = g_StringTableMgr( "ui", "MC_LOAD_FAILED_FAULTY_SLOT1" );
        else
            pText = g_StringTableMgr( "ui", "MC_LOAD_FAILED_FAULTY_SLOT2" );

        ChangeState( __id MC_STATE_DONE );
        WarningBox(
            g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),
            pText,
            TRUE
        );
        return true;
    }

    if( Pending.bFileNotFound )
    {
        const xwchar* pText;
        if( ! m_iCard )
            pText = g_StringTableMgr( "ui", "MC_DATA_CORRUPT_SLOT1" );
        else
            pText = g_StringTableMgr( "ui", "MC_DATA_CORRUPT_SLOT2" );

        ChangeState( __id MC_STATE_DONE );
        WarningBox(
            g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ),
            pText,
            TRUE
        );
        return true;
    }

    return false;
}



//==---------------------------------------------------------------------------

MemCardMgr::op_code MemCardMgr::GetMcResult( void )
{
    m_McResult = kFAILURE;

    condition& Pending = GetPendingCondition(m_iCard);

    // Process condition ******************************************************

    switch( g_MemcardMgr.GetAsyncState( ))
    {
        // --------------------------------------------------------------------

        //  Ready to start the next operation.
        case MEMCARD_SUCCESS:
            m_McResult = kSUCCESS;
            break;

        //  Busy -- retry action
        case MEMCARD_BUSY:
            Pending.bBusy = true;
            m_McResult = kPENDING;
            ASSERT(0);
            break;

        //  Operation is in progress.
        case MEMCARD_IN_PROGRESS:
            m_McResult = kPENDING;
            break;

        // --------------------------------------------------------------------

        //  Error due to program design (e.g., parameter range error, etc.)
        case MEMCARD_FATAL_ERROR:
            Pending.bFatalError = true;
            break;

        //  The filename about to be created/renamed already exists. 
        case MEMCARD_FILE_ALREADY_EXISTS:
            Pending.bFileAlreadyExists = true;
            break;

        //  No more free directory entries. 
        case MEMCARD_NO_FILES_AVAILABLE:
            Pending.bNoFilesAvailable = true;
            break;

        //  The filename about to be created/renamed is too long. 
        case MEMCARD_FILENAME_TOO_LONG:
            Pending.bFileNameTooLong = true;
            break;

        //  Tried to read/write over the file size limit. 
        case MEMCARD_PAST_END_OF_FILE:
            Pending.bPastEof = true;
            break;

        //  Insufficient free space in data blocks. 
        case MEMCARD_NOT_ENOUGH_SPACE:
            Pending.bNotEnoughSpace = true;
            break;

        //  Specified file was not found.
        case MEMCARD_FILE_NOT_FOUND:
            Pending.bFileNotFound = true;
            break;

        //  No file access permission. 
        case MEMCARD_ACCESS_DENIED:
            Pending.bAccessDenied = true;
            break;

        //  A device is detected, but it is not a memory card. 
        case MEMCARD_NOT_A_MEMCARD:
            Pending.bNotAMemcard = true;
            break;

        //  non-8k sector sized card was found. 
        case MEMCARD_INCOMPATIBLE:
            Pending.bIncompatible = true;
            break;

        //  the memory card was changed
        case MEMCARD_CARD_CHANGED:
            Pending.bCardHasChanged = true;
            break;

        //  Character set encoding is mismatched.     
        case MEMCARD_WRONG_REGION:
            Pending.bWrongRegion = true;
            break;

        //  the memory card is unformatted
        case MEMCARD_UNFORMATTED:
            Pending.bUnformatted = true;
            break;

        //  The read/write operation is canceled.
        case MEMCARD_IO_CANCELED:
            Pending.bIoCancelled = true;
            break;

        //  Memory card has reached limit of useable life. 
        case MEMCARD_WORN_OUT:
            Pending.bWornOut = true;
            break;

        //  Memory card is not detected (or not mounted yet).
        case MEMCARD_NO_CARD:
            Pending.bNoCard = true;
            break;

        //  File system is broken.
        case MEMCARD_DAMAGED:
            Pending.bDamaged = true;
            break;

        //  the memory card is FULL (different to not enough space)
        case MEMCARD_FULL:
            Pending.bFull = true;
            break;

        default:
            ASSERT(0);
    }

    // Push damaged message ***************************************************

    // todo: push an action to display nasty message.

    return m_McResult;
}

//==---------------------------------------------------------------------------

void MemCardMgr::Clear( void )
{
    while( IsActionDone() == FALSE )
    {
        Update( 0.001f );
    }

    s32 i;
    for( i=0; i<MAX_CARD_SLOTS; i++ )
    {
        // force a poll for this card
        m_bForcePoll[i] = true;

        s32 j;
        condition& Pending = GetPendingCondition(i);
        for( j=0; j<Pending.InfoList.GetCount(); j++ )
        {
            Pending.InfoList[j].Name.Clear();
            Pending.InfoList[j].Dir.Clear();
        }
        Pending.InfoList.Clear();

        condition& Active  = GetCondition(i);
        for( j=0; j<Active.InfoList.GetCount(); j++ )
        {
            Active.InfoList[j].Name.Clear();
            Active.InfoList[j].Dir.Clear();
        }
        Active.InfoList.Clear();
    }
    m_CardManifest[0].Clear();
    m_CardManifest[1].Clear();
    m_Manifest.Clear();
    g_MemcardHardware.InvalidateFileList();
}


//==---------------------------------------------------------------------------
s32 MemCardMgr::AllocBuffer( s32 Size )
{
    s32 RoundedSize;

    RoundedSize = (Size+1023)&~1023;
    ASSERT( m_pLoadBuffer == NULL );
    m_pLoadBuffer = x_malloc( RoundedSize );
    ASSERT( m_pLoadBuffer );
    x_memset( m_pLoadBuffer, 0, RoundedSize );
    return RoundedSize;
}

//==---------------------------------------------------------------------------
void MemCardMgr::FreeBuffer( void )
{
    ASSERT( m_pLoadBuffer );
    x_free( m_pLoadBuffer );
    m_pLoadBuffer = NULL;
    m_LoadBufferSize = 0;
}