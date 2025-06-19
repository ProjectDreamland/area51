///////////////////////////////////////////////////////////////////////////////
// 
// 
// MemCardMgr.hpp
// Wed Feb 26 11:43:28 2003
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MEMCARDMGR__ 
#define __MEMCARDMGR__ 

///////////////////////////////////////////////////////////////////////////////
//
//  Includes
//
///////////////////////////////////////////////////////////////////////////////

#include "StateMgr\StateMgr.hpp"
#include "MemCardMgr\MemCardMgr.hpp"
#include "Dialogs\dlg_download.hpp"

class dlg_mcmessage;

///////////////////////////////////////////////////////////////////////////////
//
//  Externals
//
///////////////////////////////////////////////////////////////////////////////

enum memcard_mode
{
    MEMCARD_FORMAT_MODE,
    MEMCARD_CREATE_MODE,
    MEMCARD_DELETE_MODE,
    MEMCARD_CHECK_MODE,
    MEMCARD_SAVE_MODE,
    MEMCARD_LOAD_MODE,
    MEMCARD_IDLE_MODE
} ;

///////////////////////////////////////////////////////////////////////////////
//
//  Defines
//
///////////////////////////////////////////////////////////////////////////////

#if defined(TARGET_XBOX)
#   define MC_TEXT_BOX_POPUP_SIZE_W 320
#   define MC_TEXT_BOX_POPUP_SIZE_H 240
#elif defined(TARGET_PC)
#   define MC_TEXT_BOX_POPUP_SIZE_W 640
#   define MC_TEXT_BOX_POPUP_SIZE_H 480
#else
#   define MC_TEXT_BOX_POPUP_SIZE_W 400
#   define MC_TEXT_BOX_POPUP_SIZE_H 280
#endif

#define MC_STATE_STACK_SIZE      10
#define MEMCARD_VERSION          0x10000000
#define MEMCARD_CORRUPT          0xFFFFFFFF

#define MAX_PROFILE_CAPACITY     32

#if defined(TARGET_PC) || defined(TARGET_XBOX)
#   define MAX_CARD_SLOTS   1 // hdd
#   define MAX_PLAYER_SLOTS 5
#else
#   define MAX_CARD_SLOTS   1
#   define MAX_PLAYER_SLOTS 3
#endif 

#ifndef __id
#define __id &MemCardMgr::
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  Prototypes
//
///////////////////////////////////////////////////////////////////////////////

struct MemCardMgr;

//! Queue machine class
/** This is the event dispatching class.
    */

template< s32 Capacity >struct queue_machine
{
    // ------------------------------------------------------------------------

    typedef void( MemCardMgr::*ID )(void);

    union CbMethod
    {
        void run( void )
        {
            __asm // Curse you .NET! This code works 90%
            {     // of the time under Microsoft's .POO!
                  // #pragma( biscuit_lint:disable )
                mov eax,[ecx+8] // Fn
                mov ecx,[ecx]   // Me
                call eax
            }
        }

        void* Data[4];
        struct
        {
            void* Me;
            ID    Fn;
        };
    };

    // ------------------------------------------------------------------------

    //! Default constructor.
    /** This routine is designed for use by other defaults.
        */

    queue_machine( MemCardMgr* pParent,ID Idle )
    {
        x_memset( this,0,sizeof(*this) );

        m_pParent = pParent;
        m_Idle    = Idle;

        FlushStateStack();

        Control  = 0;
    }

    //! Destructor.
    /** This is the place to destroy all those lovely little
        member function pointers.
        */

~   queue_machine( void )
    {
    }

    // ------------------------------------------------------------------------

    //! Flush states
    /** This routine empties the state stack.
        */

    void FlushStateStack( void )
    {
        m_iTail = m_iHead;
    }

    //! Change state
    /** This routine changes the current state.
        */

    void ChangeState( ID Id )
    {
        ASSERT( m_iHead != m_iTail );
        m_MethodQueue[m_iHead].Fn=Id;
        m_MethodQueue[m_iHead].Me=this;
    }

    //! Push state
    /** This routine pushes the current state onto the stack.
        DO NOT PUSH IN REVERSE ORDER!!
        */

    template< class t >void PushState( ID Id,t* CbThis,void(t::*Cb)(void) )
    {
        m_CbMethod[m_iTail].Fn = *(ID*)&Cb;
        m_CbMethod[m_iTail].Me = CbThis;
        PushState( Id );
    }

    //! Push state
    /** This routine pushes the current state onto the stack.
        DO NOT PUSH IN REVERSE ORDER!!
        */

    void PushState( ID Id )
    {
        // add ID .............................................................

        m_MethodQueue[m_iTail].Me = m_pParent;
        m_MethodQueue[m_iTail].Fn = Id;

        // update tail ........................................................

        if( m_iTail==m_iHead )
            bDirty = true;
        if( m_iTail+1 < Capacity )
            m_iTail++;
        else
            m_iTail=0;

        // report end of space ................................................

        ASSERT( m_iTail != m_iHead );
    }

    //! Pop state
    /** This routine pops the current state and
        sets the engine to idle when it's empty.
        */

    void PopState( void )
    {
        if( m_iTail == m_iHead )
            return;

        // run callback .......................................................

        if( m_CbMethod[m_iHead].Me )
        {
            m_CbMethod[m_iHead].run();
            m_CbMethod[m_iHead].Me=0;
            m_CbMethod[m_iHead].Fn=0;
        }


        // move head ..........................................................

        if( m_iHead+1 < Capacity )
            m_iHead++;
        else
            m_iHead = 0;
        bDirty=true;
    }

    //! Clear all callbacks.
    /** This routine clears all callback methods and owner pointers.
        */

    void ClearCBs( void )
    {
        x_memset( m_CbMethod,0,Capacity*sizeof( CbMethod ));
    }

    //! Return current state.
    /** This routine returns the current ID.
        */

    ID GetState( void )
    {
        if( m_iHead != m_iTail )
            return m_MethodQueue[m_iHead].Fn;
        return m_Idle;
    }

    //! Execute id.
    /** This routine executes the current state by literally
        calling into the m_Id member function pointer.
        Very sneaky.
        */

    void Exec( void )
    {
        if( bDirty || bAlways )
        {
            if( m_iHead != m_iTail )
                m_MethodQueue[m_iHead].run();
            else
                (m_pParent->*m_Idle)();
            bDirty = false;
        }
    }

    // ------------------------------------------------------------------------

    union
    {
        struct
        {
            u32 bWasPolling:1;
            u32 bPolling   :1;
            u32 bAlways    :1;
            u32 bDirty     :1; // Used for single stepping execution
        };
        u32 Control;
    };

    ///////////////////////////////////////////////////////////////////////////

    //! Stack elements
    /** This member is an array of pointers to memeber functions
        in the class "MemCardMgr". Instead of having named identifiers
        to represent states we'll name the functions. Thus
        the code can look the same as the pre-A51 days but
        with much greater efficiency.
        */

    CbMethod m_MethodQueue[Capacity];

    //! Callback queue.
    /** This member is an array of member function callbacks
        corresponding to the m_MethodQueue.
        */

    CbMethod m_CbMethod[Capacity];

    ///////////////////////////////////////////////////////////////////////////

    //! Parent class.
    /** This member contains the parent to which the methods belong.
        */

    MemCardMgr* m_pParent;

    //! Head index.
    /** This is the current execution point of the stack.
        */

    s32 m_iHead;

    //! Tail index.
    /** This is the end of the stack. When you PopState m_iHead
        is incremented. The stack is said to be empty when it
        equals m_iTail. When you PushState() m_iTail is
        incremented, wrapping to zero if it passes
        the array capacity.
        */

    s32 m_iTail;

    //! Idle routine.
    /** This member points to the idle routine.
        */

    ID m_Idle;
};

typedef queue_machine<  8 >::ID action;
typedef queue_machine< 64 >::ID mc_id;

///////////////////////////////////////////////////////////////////////////////

struct MemCardMgr: public queue_machine< 64 >
{
    // ------------------------------------------------------------------------
    //                                                      Memory card actions
    // ------------------------------------------------------------------------

    void MC_NO_ACTION                            ( void );
    void MC_STATE_FINISH                         ( void );
    void MC_STATE_DONE                           ( void );

    void MC_ACTION_CREATE_PROFILE                ( void );
    void MC_ACTION_DELETE_PROFILE                ( void );
    void MC_ACTION_SAVE_PROFILE                  ( void );
    void MC_ACTION_OVERWRITE_PROFILE             ( void );
    void MC_ACTION_LOAD_PROFILE                  ( void );
    void MC_ACTION_CREATE_SETTINGS               ( void );
    void MC_ACTION_SAVE_SETTINGS                 ( void );
    void MC_ACTION_OVERWRITE_SETTINGS            ( void );
    void MC_ACTION_LOAD_SETTINGS                 ( void );
    void MC_ACTION_FORMAT                        ( void );

    // ------------------------------------------------------------------------

    void MC_ACTION_POLL_CARDS                    ( void );
    void MC_ACTION_REPOLL_CARDS                  ( void );
    void MC_ACTION_BOOT_CHECK                    ( void );
    void MC_ACTION_REBOOT_CHECK                  ( void );

    void MC_ACTION_POLL_CONTENT                  ( void );
    void MC_ACTION_SAVE_CONTENT                  ( void );
    void MC_ACTION_LOAD_CONTENT                  ( void );
    void MC_ACTION_DELETE_CONTENT                ( void );

    /* polling states */
    void MC_STATE_TRAWL_DIRS                     ( void );
    void MC_STATE_TRAWL_DIRS_WAIT                ( void );
    void MC_STATE_GET_PROFILE_NAMES              ( void );
    void MC_STATE_GET_PROFILE_NAMES_WAIT         ( void );
    void MC_STATE_PROFILE_NAME_WAIT              ( void );
    void MC_STATE_FIND_SETTINGS                  ( void );
    void MC_STATE_FIND_SETTINGS_WAIT             ( void );
    void MC_STATE_TRAWL_CHECK_CARD               ( void );
    void MC_STATE_TRAWL_CHECK_CARD_WAIT          ( void );
    void MC_STATE_TRAWL_CHECK_CARD_HOLD          ( void );

    /* boot check */
    void MC_CHECK_CARD_HOLD                      ( void );
    void MC_CHECK_CARD_WAIT                      ( void );
    void MC_CHECK_CARD_HOLD_WAIT                 ( void );
    void MC_STATE_NO_CARD_ASK                    ( void );

    void MC_STATE_BOOT_CHECK                     ( void );
    void MC_STATE_BOOT_CHECK_WAIT                ( void );
    void MC_STATE_FIND_PROFILE                   ( void );
    void MC_STATE_FIND_PROFILE_WAIT              ( void );
    void MC_STATE_FIND_PROFILE_CHECK             ( void );
    void MC_STATE_BOOT_FAILED_ASK                ( void );

    void MC_STATE_UNMOUNT                        ( void );
    void MC_STATE_UNMOUNT_WAIT                   ( void );
    void MC_STATE_MOUNT                          ( void );
    void MC_STATE_MOUNT_WAIT                     ( void );
    void MC_STATE_BOOT_ACTION_DONE               ( void );
    void MC_STATE_REMOUNT                        ( void );
    void MC_STATE_REMOUNT_WAIT                   ( void );

    /* create profile */
    void MC_STATE_CREATE_PROFILE_CREATE_DIR_WAIT ( void );
    void MC_STATE_CREATE_PROFILE_SET_DIR_WAIT    ( void );
    void MC_STATE_CREATE_PROFILE                 ( void );
    void MC_STATE_CREATE_PROFILE_FAILED          ( void );
    void MC_STATE_CREATE_PROFILE_FAILED_WAIT     ( void );

    /* delete profile */
    void MC_STATE_DELETE_PROFILE                 ( void );
    void MC_STATE_DELETE_PROFILE_WAIT            ( void );
    void MC_STATE_DELETE_PROFILE_FAILED          ( void );
    void MC_STATE_DELETE_PROFILE_SUCCESS         ( void );
    void MC_STATE_DELETE_PROFILE_SUCCESS_WAIT    ( void );

    /* load profile */
    void MC_STATE_LOAD_PROFILE_SET_DIR           ( void );
    void MC_STATE_LOAD_PROFILE_SET_DIR_WAIT      ( void );
    void MC_STATE_PROFILE_READ_WAIT              ( void );
    void MC_STATE_LOAD_PROFILE_FAILED            ( void );
    void MC_STATE_LOAD_PROFILE_SUCCESS           ( void );
    void MC_STATE_LOAD_PROFILE_SUCCESS_WAIT      ( void );

    /* load manifest and patch */
    void MC_STATE_LOAD_MANIFEST                  ( void );
    void MC_STATE_LOAD_MANIFEST_SET_DIR_WAIT     ( void );
    void MC_STATE_LOAD_MANIFEST_READ_WAIT        ( void );

    /* save profile */
    void MC_STATE_SAVE_PROFILE                   ( void );
    void MC_STATE_SAVE_PROFILE_SET_DIR_WAIT      ( void );
    void MC_STATE_PROFILE_WRITE_WAIT             ( void );
    void MC_STATE_SAVE_PROFILE_FAILED            ( void );
    void MC_STATE_SAVE_PROFILE_FAILED_WAIT       ( void );
    void MC_STATE_SAVE_PROFILE_SUCCESS           ( void );
    void MC_STATE_SAVE_PROFILE_SUCCESS_WAIT      ( void );

    /* create settings */
    void MC_STATE_CREATE_SETTINGS                ( void );
    void MC_STATE_CREATE_SETTINGS_CREATE_DIR_WAIT( void );

    /* save settings */
    void MC_STATE_SAVE_SETTINGS                  ( void );
    void MC_STATE_SAVE_SETTINGS_SET_DIR_WAIT     ( void );
    void MC_STATE_SAVE_SETTINGS_WRITE_WAIT       ( void );
    void MC_STATE_SAVE_SETTINGS_FAILED           ( void );
    void MC_STATE_SAVE_SETTINGS_FAILED_WAIT      ( void );
    void MC_STATE_SAVE_SETTINGS_SUCCESS          ( void );
    void MC_STATE_SAVE_SETTINGS_SUCCESS_WAIT     ( void );
    
    /* overwrite settings */
    void MC_STATE_OVERWRITE_SETTINGS_CONFIRM     ( void );
    void MC_STATE_OVERWRITE_SETTINGS_CONFIRM_WAIT( void );
    void MC_STATE_OVERWRITE_SETTINGS_RECHECK     ( void );

    /* load settings */
    void MC_STATE_LOAD_SETTINGS                  ( void );
    void MC_STATE_LOAD_SETTINGS_SET_DIR_WAIT     ( void );
    void MC_STATE_LOAD_SETTINGS_READ_WAIT        ( void );

    void MC_STATE_LOAD_CONTENT                   ( void );
    void MC_STATE_SAVE_CONTENT                   ( void );
    void MC_STATE_POLL_CONTENT                   ( void );
    void MC_STATE_DELETE_CONTENT                 ( void );

    void MC_STATE_LOAD_CONTENT_SET_DIR_WAIT      ( void );
    void MC_STATE_LOAD_CONTENT_GET_SIZE_WAIT     ( void );
    void MC_STATE_LOAD_CONTENT_READ_WAIT         ( void );
    void MC_STATE_LOAD_CONTENT_FAILED            ( void );

    void MC_STATE_DELETE_CONTENT_CONFIRM         ( void );
    void MC_STATE_DELETE_CONTENT_CONFIRM_WAIT    ( void );
    void MC_STATE_DELETE_CONTENT_SET_DIR_WAIT    ( void );
    void MC_STATE_DELETE_CONTENT_DELETE_WAIT     ( void );

    void MC_STATE_SAVE_CONTENT_CREATE_DIR_WAIT   ( void );
    void MC_STATE_SAVE_CONTENT_SET_DIR_WAIT      ( void );
    void MC_STATE_SAVE_CONTENT_WRITE_WAIT        ( void );
    void MC_STATE_SAVE_CONTENT_FAILED            ( void );
    void MC_STATE_SAVE_MANIFEST_WRITE_WAIT       ( void );

    void MC_STATE_POLL_SET_DIR_WAIT              ( void );
    void MC_STATE_POLL_READ_WAIT                 ( void );

    /* overwrite profile */
    void MC_STATE_OVERWRITE_CONFIRM              ( void );
    void MC_STATE_OVERWRITE_CONFIRM_WAIT         ( void );
    void MC_STATE_OVERWRITE_RECHECK              ( void );
    void MC_STATE_OVERWRITE_PROFILE              ( void );
    void MC_STATE_OVERWRITE_SUCCESS              ( void );
    void MC_STATE_OVERWRITE_SUCCESS_WAIT         ( void );

    /* formatting */

    void MC_STATE_INIT_FORMAT                    ( void );
    void MC_STATE_ASK_FORMAT                     ( void );
    void MC_STATE_FORMAT_CONFIRM                 ( void );
    void MC_STATE_WAIT_RECHECK_FORMAT            ( void );
    void MC_STATE_FORMAT_WAIT                    ( void );
    void MC_STATE_FORMAT_CANCEL                  ( void );
    void MC_STATE_FORMAT_FAILED                  ( void );
    void MC_STATE_FORMAT_SUCCESS                 ( void );
    void MC_STATE_FORMAT_SUCCESS_WAIT            ( void );

    // ------------------------------------------------------------------------

#if defined(TARGET_XBOX)
    void MC_ACTION_XBOX_CHECK_BLOCK_AVAIL        ( void );
#endif

    // ------------------------------------------------------------------------
    //                                            Actionable memory card states
    // ------------------------------------------------------------------------

    void MC_STATE_NEXT_CARD                      ( void );
    void MC_STATE_IDLE                           ( void );

    // ------------------------------------------------------------------------
    // XBOX XBOX XBOX XBOX                        Actionable memory card states
    // ------------------------------------------------------------------------

#if defined( TARGET_XBOX ) || defined( TARGET_PC )

    void MC_STATE_MESSAGE_OUT_OF_BLOCKS_ASK      ( void );
    void MC_STATE_MESSAGE_OUT_OF_BLOCKS          ( void );

#endif

    // ------------------------------------------------------------------------

    enum return_values
    {
        kNO_RETURN_VALUE = 0,
        kACTION_IN_PROCESS,
        kNEW_FILE_ERROR,
        kDELETE_ERROR,
        kFORMAT_ERROR,
        kUSER_CANCEL,
        kSAVE_ERROR,
        kLOAD_ERROR,
        kPICKAGAIN,
        kRETURN_OK,
        kNO_CARD,
    };

    // ------------------------------------------------------------------------

    void                ClearProfileNames   ( s32 CardID );
    void                GetProfileNames     ( xarray< profile_info* >& Result ); // Get names
    s32                 Update              ( f32 DeltaTime );
    void                Init                ( void );
    void                Kill                ( void );
    void                SetCard             ( s32 CardId )                      { m_iCard = CardId;                 }
    s32                 GetCard             ( void )                            { return m_iCard;                   }
    void                SetManifest         ( const map_list& Manifest )        { m_Manifest = Manifest;            }
    map_list&           GetManifest         ( void )                            { return m_Manifest;                }
    map_list&           GetManifest         ( s32 Card )                        { return m_CardManifest[Card];      }
    void*               GetBuffer           ( void )                            { return m_pLoadBuffer;             }
    s32                 GetBufferLength     ( void )                            { return m_LoadBufferSize;          }
    void                FreeBuffer          ( void );

                        MemCardMgr          ( void );
                      ~ MemCardMgr          ( void );

public:

    // ------------------------------------------------------------------------

    template< class t >void LoadProfile( profile_info& Info,s32 PlayerID,t* pThis,void(t::* pMethod)(void))
    {
        m_Action.PushState( __id MC_ACTION_LOAD_PROFILE,pThis,pMethod );

        ASSERT( PlayerID >= 0 );
        ASSERT( PlayerID < MAX_PLAYER_SLOTS );
        m_PreservedProfile[PlayerID] = Info;
        m_iPlayer    = PlayerID;
        bWasPolling  = bPolling;
        m_iSlot      = -1;
    }

    // ------------------------------------------------------------------------

    template< class t >void SaveProfile( profile_info& Info,s32 PlayerID,t* pThis,void(t::* pMethod)(void))
    {
        m_Action.PushState( __id MC_ACTION_SAVE_PROFILE,pThis,pMethod );

        ASSERT( PlayerID >= 0 );
        ASSERT( PlayerID < MAX_PLAYER_SLOTS );
        m_PreservedProfile[PlayerID] = Info;
        m_iPlayer    = PlayerID;
        bWasPolling  = bPolling;
        m_iSlot      = -1;
    }

    // ------------------------------------------------------------------------

    template< class t >void CreateSettings( t* pThis, void(t::* pMethod)(void) )
    {
        m_Action.PushState( __id MC_ACTION_CREATE_SETTINGS, pThis, pMethod );
    }

    // ------------------------------------------------------------------------

    template< class t >void SaveSettings( t* pThis, void(t::* pMethod)(void) )
    {
        m_Action.PushState( __id MC_ACTION_SAVE_SETTINGS, pThis, pMethod );
    }

    // ------------------------------------------------------------------------

    template< class t >void OverwriteSettings( t* pThis, void(t::* pMethod)(void) )
    {
        m_Action.PushState( __id MC_ACTION_OVERWRITE_SETTINGS, pThis, pMethod );
    }

    // ------------------------------------------------------------------------

    template< class t>void LoadManifest( s32 CardId, map_list& Manifest, t* pThis, void(t::* pMethod)(void))
    {
        SetCard( CardId );
        m_Action.PushState( __id MC_ACTION_LOAD_MANIFEST, pThis, pMethod );
        bDirty = TRUE;
    }

    template< class t>void LoadContent( const map_entry& Manifest, t* pThis, void(t::* pMethod)(void))
    {
        //
        // There is an assumption made here that the information about the manifest entry is contained
        // within the global map list, so we don't need to pass that in to get the filename for this
        // manifest entry.
        //
        m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID       = Manifest.GetLocation();
        m_PreservedProfile[MAX_PLAYER_SLOTS-1].Dir          = Manifest.GetFilename();
        m_Action.PushState( __id MC_ACTION_LOAD_CONTENT, pThis, pMethod );
        bDirty = TRUE;
    }

    // ------------------------------------------------------------------------
    template< class t>void SaveContent( map_list& Manifest, map_entry& Entry, t* pThis, void(t::* pMethod)(void))
    {
        const map_info* pMapInfo        = Manifest.GetMapInfo( Entry.GetMapID() );
        // The manifest for this card will have already been rebuilt to include the new maps to be
        // saved, so this map info should exist always!
        ASSERT( pMapInfo );
        m_PreservedProfile[MAX_PLAYER_SLOTS-1].Dir          = pMapInfo->Filename;
        m_PreservedProfile[MAX_PLAYER_SLOTS-1].ProfileID    = Entry.GetMapID();
        m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID       = Entry.GetLocation();
        m_Manifest                      = Manifest;
        m_Action.PushState( __id MC_ACTION_SAVE_CONTENT, pThis, pMethod );
        bDirty = TRUE;
    }

    // ------------------------------------------------------------------------
    template< class t>void DeleteContent( const map_entry& Manifest, t* pThis, void(t::* pMethod)(void))
    {
        //
        // The content gets deleted from the actual manifest list when
        // the manifest list gets serialized in the delete phase.
        //
        m_PreservedProfile[MAX_PLAYER_SLOTS-1].ProfileID    = Manifest.GetMapID();
        m_PreservedProfile[MAX_PLAYER_SLOTS-1].CardID       = Manifest.GetLocation();
        m_Action.PushState( __id MC_ACTION_DELETE_CONTENT, pThis, pMethod );
        bDirty = TRUE;
    }


    // ------------------------------------------------------------------------

    template< class t >void OverwriteProfile( profile_info& Info,s32 PlayerID,t* pThis,void(t::* pMethod)(void))
    {
        m_Action.PushState( __id MC_ACTION_OVERWRITE_PROFILE,pThis,pMethod );
        
        ASSERT( PlayerID >= 0 );
        ASSERT( PlayerID < MAX_PLAYER_SLOTS );
        m_PreservedProfile[PlayerID] = Info;
        m_iPlayer    = PlayerID;
        bWasPolling  = bPolling;
        m_iSlot      = -1;
    }

    // ------------------------------------------------------------------------

    profile_info& GetProfileInfo( s32 PlayerID )
    {
        ASSERT( PlayerID >= 0 );
        ASSERT( PlayerID < MAX_PLAYER_SLOTS );
        return m_PreservedProfile[PlayerID];
    }

    // ------------------------------------------------------------------------

    template< class t >void DeleteProfile( profile_info& Info,t* pThis,void(t::* pMethod)(void))
    {
        m_Action.PushState( __id MC_ACTION_DELETE_PROFILE,pThis,pMethod );

        m_PreservedProfile[0] = Info;
        bWasPolling = bPolling;
        m_iSlot     = -1;
    }

    // ------------------------------------------------------------------------

    template< class t >void CreateProfile( s32 CardID,s32 PlayerID,t* pThis,void(t::* pMethod)(void))
    {
        m_Action.PushState( __id MC_ACTION_CREATE_PROFILE,pThis,pMethod );

        ASSERT( PlayerID >= 0 );
        ASSERT( PlayerID < MAX_PLAYER_SLOTS );
        bWasPolling = bPolling;
        m_iPlayer   = PlayerID;
        m_PreservedProfile[PlayerID].CardID = CardID;
        m_iSlot     = -1;
        m_iProfile  = 0;
    }

    // ------------------------------------------------------------------------

    template< class t >void Format( s32 CardID, s32 PlayerID, card_data_mode Mode, t* pThis,void(t::* pMethod)(void))
    {
        m_Action.PushState( __id MC_ACTION_FORMAT,pThis,pMethod );

        ASSERT( PlayerID >= 0 );
        ASSERT( PlayerID < MAX_PLAYER_SLOTS );
        bWasPolling = bPolling;
        m_PreservedProfile[PlayerID].CardID = CardID;
        m_iPlayer       = PlayerID;
        m_iSlot         = -1;
        m_iProfile      = 0;
        m_CardDataMode  = Mode;
    }

    // ------------------------------------------------------------------------

    template< class t >void Poll( card_data_mode Mode, t* pThis,void(t::* pMethod)(void))
    {
        if( ! bPolling )
        {
            m_CardDataMode = Mode;
            m_Action.PushState( __id MC_ACTION_POLL_CARDS,pThis,pMethod );
            bPolling = true;
        }
    }

    // ------------------------------------------------------------------------

    template< class t >void Repoll( t* pThis,void(t::* pMethod)(void))
    {
        m_Action.PushState( __id MC_ACTION_REPOLL_CARDS,pThis,pMethod );
        bPolling = true;
    }

    // ------------------------------------------------------------------------

    template< class t >void PollContent( xbool ForcePoll, t* pThis, void(t::* pMethod)(void) )
    {
        if( ForcePoll )
        {
            s32 i;
            for( i=0; i<MAX_CARD_SLOTS; i++ )
            {
                m_bForcePoll[i] = TRUE;
            }
        }
        if( !bPolling || ForcePoll )
        {
            m_Action.PushState( __id MC_ACTION_POLL_CONTENT, pThis, pMethod );
            bPolling = true;
        }
    }

    // ------------------------------------------------------------------------

    void BootCheck( void )
    {
        m_Action.PushState( __id MC_ACTION_BOOT_CHECK );
    }

    // ------------------------------------------------------------------------

    void RebootCheck( void )
    {
        m_Action.PushState( __id MC_ACTION_REBOOT_CHECK );
    }

    // ------------------------------------------------------------------------

    void ActionComplete( void );

    // ------------------------------------------------------------------------

    void ClearCallback( void )
    {
        m_Action.ClearCBs();
    }

    // ------------------------------------------------------------------------

    xbool IsActionDone( void )
    {
        return m_ActionDone;
    }

    // ------------------------------------------------------------------------

private:

    void                SelectCard          ( s32 iCardSelect ){ m_iCard = iCardSelect; }
    void                SetProfile          ( s32 iProfile ) { m_iProfile = iProfile; }
    void                InitAction          ( memcard_mode Mode );

    s32                 ResetAction         ( void );
    action              PollState           ( void );

    xbool               HandleFaultyIoOp    ( void );
    xbool               GetMemCardEngaged   ( void );

    void                WarningBox          ( const char *pTitle,const char *pMessage,xbool AllowPrematureExit=TRUE );
    void                OptionBox           ( const char *pTitle,const char *pMessage,const xwchar* pYes = NULL,const xwchar* pNo = NULL,const xbool DefaultToNo=TRUE,const xbool AllowCancel=FALSE );

    void                WarningBox          ( const xwchar* pTitle,const xwchar* pMessage, xbool AllowPrematureExit=TRUE );
    void                OptionBox           ( const xwchar* pTitle,const xwchar* pMessage, const xwchar *pYes = NULL, const xwchar *pNo = NULL, const xwchar *pMaybe = NULL, const xbool DefaultToNo=TRUE, const xbool AllowCancel=FALSE );

    void                PopUpBox            ( const xwchar* pTitle,const xwchar* pMessage, const xwchar* pNavText, const xbool bYes = TRUE, const xbool bNo = FALSE, const xbool bMaybe = FALSE );

    void                EnableProgress      ( xbool Enabled );
    void                UpdateProgress      ( f32 Progress );
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------

    #define MAX_PROFILE_COUNT 32

    // ------------------------------------------------------------------------

public:
    void                Clear               ( void );
    xbool               IsPolling           ( void )        { return( m_bPollInProgress | m_bForcePoll[0] | m_bForcePoll[1] ); }
    xbool               FoundProfile        ( void )        { return m_bFoundProfile; }
    xbool               FoundSettings       ( void )        { return m_bFoundSettings; }
    xbool               CardHasSettings     ( s32 CardID )  { ASSERT(CardID>=0); ASSERT(CardID<MAX_CARD_SLOTS); return m_bCardHasSettings[CardID]; }

    // ------------------------------------------------------------------------

    struct condition
    {
        xarray< profile_info >InfoList;

        s32 BytesFree;

        // ActionComplete is set to zero when the action begins
        // and will contain a value if there was an I/O error,
        // the action was cancelled or everything was good.
        // It will be zero at the beginning. (SHOULD NEVER
        // BE NON-ZERO! A GOOD RULE TO ASSERT.)

        void Clear( void )
        {
            SuccessCode = 0;
            ErrorCode   = 0;
        }

        // The ErrorCode will contain a value if something
        // bad happened. What *exactly* went south will be
        // represented by the individual error state fields.

        union
        {
            u32 ErrorCode;
            struct
            {
                u32 bInsufficientSpace:1;
                u32 bIsFull           :1;
                u32 bBusy             :1;

                u32 bFileAlreadyExists:1;
                u32 bNoFilesAvailable :1;
                u32 bFileNameTooLong  :1;
                u32 bCardHasChanged   :1;
                u32 bNotEnoughSpace   :1;
                u32 bFileNotFound     :1;
                u32 bIncompatible     :1;
                u32 bAccessDenied     :1;
                u32 bNotAMemcard      :1;
                u32 bIoCancelled      :1;
                u32 bUnformatted      :1;
                u32 bWrongRegion      :1;
                u32 bFatalError       :1;
                u32 bDamaged          :1;
                u32 bPastEof          :1;
                u32 bWornOut          :1;
                u32 bNoCard           :1;
                u32 bFull             :1;
            };
        };

        // The SuccessCode will contain a value when the
        // action is said to have completed. This goes
        // for both cancelled actions and finished
        // ones. The current (front) condition
        // should never be zero. Assert this!

        union
        {
            u32 SuccessCode;
            struct
            {
                u32 bCancelled  :1;
                u32 bComplete   :1;
            };
        };
    };

    condition& GetPendingCondition( s32 CardID )
    {
        ASSERT( CardID < MAX_CARD_SLOTS );
        return m_Condition[CardID][(m_PollTurn[CardID]&1)^1];
    }

    condition& GetCondition( s32 CardID )
    {
        ASSERT( CardID < MAX_CARD_SLOTS );
        return m_Condition[CardID][m_PollTurn[CardID]&1];
    }

    void FlipCondition( s32 CardID )
    {
        m_PollTurn[CardID]++;
    }

    enum op_code
    {
        kFAILURE, // operation failed
        kSUCCESS, // operation succeeded
        kPENDING, // operation still pending
        kRESET    // card pulled
    };

    op_code GetMcResult( void );

    op_code m_McResult;


    // ------------------------------------------------------------------------

private:

    s32                     AllocBuffer( s32 Size );
    condition               m_Condition[ MAX_CARD_SLOTS ][2];
    s32                     m_PollTurn [ MAX_CARD_SLOTS ];
    profile_info            m_PreservedProfile[ MAX_PLAYER_SLOTS ];
    map_list                m_CardManifest[ MAX_CARD_SLOTS ];
    map_list                m_Manifest;
    s32                     m_iCard;
    s32                     m_iDir;
    s32                     m_iProfile;
    s32                     m_iPlayer;
    s32                     m_iSlot;
    char                    m_OptionsPostfix[16];
    char                    m_SavePrefix[16];                   // For PS2, will be something like BASLUS-20595
    char                    m_ContentPostfix[16];
    s32                     m_LastCardState[ MAX_CARD_SLOTS ];
    card_data_mode          m_CardDataMode;

    void*                   m_pLoadBuffer;
    s32                     m_LoadBufferSize;
    s32                     m_bForcePoll[ MAX_CARD_SLOTS ];
    xbool                   m_bPollInProgress;

                        
    s16                     m_DialogsAllocated;
    s32                     m_MessageResult;
    f32                     m_MessageDelay;
    s32                     m_ActionDone;
    s32                     m_Closure;
                            
    s32                     m_SlotArea;
    dlg_mcmessage*          m_pMessage;
    xwstring                m_Message;
    char*                   m_pBuffer;
    s32                     m_nBytes;
    u32                     m_EncryptionKey[4];
    u64                     m_LastSettingsDatestamp;
    s32                     m_CardWait;
    memcard_mode            m_MemcardMode;
    queue_machine< 8 >      m_Action;

    xbool                   m_bFoundProfile;
    xbool                   m_bFoundSettings;
    xbool                   m_bPassedBootCheck;
    xbool                   m_bCardHasSettings[ MAX_CARD_SLOTS ];

    s32                     m_BlocksRequired;
};

///////////////////////////////////////////////////////////////////////////////

extern MemCardMgr g_UIMemCardMgr;

#ifdef _MSC_VER
#pragma warning( disable:4355 )
#endif

#endif // MEMCARDMGR
