#include "entropy.hpp"
#include "e_Memcard.hpp"

//------------------------------------------------------------------------------

static xbool s_Initialized = FALSE;

//------------------------------------------------------------------------------

memcard_mgr      g_MemcardMgr;
memcard_hardware g_MemcardHardware;


//------------------------------------------------------------------------------

memcard_mgr::memcard_mgr( void )
{
}

//------------------------------------------------------------------------------

memcard_mgr::~memcard_mgr( void )
{
}

//------------------------------------------------------------------------------

void memcard_mgr::ResetSystem( void )
{
    // Clear the connection bits.
    m_PrevConnectionBits =
    m_CurrConnectionBits = 0;

    // Clear the callback.
    m_pCardStatusUpdate = NULL;
}

//------------------------------------------------------------------------------

void memcard_mgr::Init( void )
{
    ASSERT( !s_Initialized );

    // Initialize the memory card.
    if( !s_Initialized )
    {
        // Reset.
        ResetSystem();

        // Init the hardware.
        g_MemcardHardware.Init();

        // Its initialized.
        s_Initialized = TRUE;
    }
}

//------------------------------------------------------------------------------

void memcard_mgr::Kill( void )
{
    // Error check.
    if( s_Initialized )
    {
        // Clear everything.
        ResetSystem();

        // TODO: Check order of operation here.

        // Nuke the hardware.
        g_MemcardHardware.Kill();

        // No longer initialized
        s_Initialized = FALSE;
    }
}

//------------------------------------------------------------------------------

u32 memcard_mgr::GetConnectionBits( void )
{
    u32 Result = 0;

    // Error check.
    ASSERT( s_Initialized );

    // Check each card...
    for( s32 i=0 ; i<GetMaxCards() ; i++ )
    {
        // Is the card connected?
        if( IsCardConnected( i ) )
        {
            // Set the bit.
            Result |= (1<<i);
        }
    }

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

void memcard_mgr::Engage( card_status_update_fn* pCallback )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( !m_bIsEngaged );

    // Reset.
    ResetSystem();
    
    // TODO: Allocate memory for the directory entries.

    // Set the callback.
    m_pCardStatusUpdate = pCallback;

    // System is now engaged.
    m_bIsEngaged = TRUE;
}

//------------------------------------------------------------------------------

void memcard_mgr::Disengage( void )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );
    ASSERT( g_MemcardHardware.GetOperation() == MEMCARD_OP_IDLE );

    // TODO: Free memory for the directory entries.

    // Reset.
    ResetSystem();

    // Free up the IO Buffer.
    g_MemcardHardware.FreeIOBuffer();

    // System is no longer engaged.
    m_bIsEngaged = FALSE;
}

//------------------------------------------------------------------------------

void memcard_mgr::Update( f32 DeltaTime )
{
    (void)DeltaTime;

    // Error check.
    ASSERT( s_Initialized );

#ifdef TARGET_PS2
    // Only if the system is engaged.
    if( m_bIsEngaged )
    {
        // process ps2 async callback
        g_MemcardHardware.ProcessHardwareCallback();
    }
#else
    // Only if the system is engaged.
    if( m_bIsEngaged )
    {
        // Get current connection bits.
        m_CurrConnectionBits = GetConnectionBits();

        // Did something change?
        if( m_CurrConnectionBits != m_PrevConnectionBits )
        {
            // Callback enabled?
            if( m_pCardStatusUpdate )
            {
                // Which ones changed?
                u32 Changed = m_CurrConnectionBits ^ m_PrevConnectionBits;

                for( s32 i=0 ; i<GetMaxCards() ; i++ )
                {
                    // This one change?
                    if( Changed & (1<<i) )
                    {
                        // Do the callback.
                        m_pCardStatusUpdate( i );
                    }
                }
            }
        }

        // New previous.
        m_PrevConnectionBits = m_CurrConnectionBits;
    }
#endif
}

//------------------------------------------------------------------------------

s32 memcard_mgr::GetMaxCards( void )
{
    // Error check.
    ASSERT( s_Initialized );

    // Tell the world.
    return g_MemcardHardware.GetMaxCards();
}

//------------------------------------------------------------------------------

xbool memcard_mgr::IsCardConnected( s32 CardID )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );
    ASSERT( VALID_CARDID( CardID ) );

    // Get state from the hardware.
    return g_MemcardHardware.IsCardConnected( CardID );
}

//------------------------------------------------------------------------------

memcard_error memcard_mgr::GetAsyncState( void )
{
    return g_MemcardHardware.GetState();
}

//------------------------------------------------------------------------------

s32 memcard_mgr::GetFileList( xarray<mc_file_info>& FileList )
{
    return g_MemcardHardware.GetFileList( FileList );
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncMount( s32 CardID )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );
    ASSERT( VALID_CARDID( CardID ) );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_MOUNT );

    // Set the requested card.
    g_MemcardHardware.SetRequestedCard( CardID );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncUnmount( void )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // TDOD: Deal with purging the directory.

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_UNMOUNT );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncReadFile( const char* pFileName, void* const pBuffer, s32 nBytes )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the IO parameters.
    g_MemcardHardware.SetIOParams( pFileName, (byte*)pBuffer, 0, nBytes );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_READ_FILE );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncWriteFile( const char* pFileName, const void* const pBuffer, s32 nBytes )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the IO parameters.
    g_MemcardHardware.SetIOParams( pFileName, (byte*)pBuffer, 0, nBytes );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_WRITE_FILE );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncDeleteFile( const char* pFileName )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );
    
    // Set the IO parameters.
    g_MemcardHardware.SetIOParams( pFileName, NULL, 0, 0 );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_DELETE_FILE );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncFormat( void )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_FORMAT );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncReadFileList( void ) 
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );
    
    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_READ_FILE_LIST );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncPurgeFileList( void ) 
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_PURGE_FILE_LIST );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncGetFileLength( const char* pFileName ) 
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_GET_FILE_LENGTH );

    // Tell it which file.
    g_MemcardHardware.SetIOParams( pFileName, NULL, 0, 0 );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

s32 memcard_mgr::GetFileLength( void )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Tell the world.
    return g_MemcardHardware.GetFileLength();
}

//------------------------------------------------------------------------------

xbool memcard_mgr::IsSpaceAvailable( u32 nBytes )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    u32 FreeSpace = g_MemcardHardware.GetFreeSpace();
    return ( nBytes <= FreeSpace );
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncRead( const char* pFileName, void* const pBuffer, s32 Offset, s32 nBytes )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the IO parameters.
    g_MemcardHardware.SetIOParams( pFileName, (byte*)pBuffer, Offset, nBytes );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_READ );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncWrite( const char* pFileName, const void* const pBuffer, s32 Offset, s32 nBytes )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the IO parameters.
    g_MemcardHardware.SetIOParams( pFileName, (byte*)pBuffer, Offset, nBytes );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_WRITE );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncCreateFile( const char* pFileName, s32 nBytes )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the IO parameters.
    g_MemcardHardware.SetIOParams( pFileName, NULL, 0, nBytes );

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_CREATE_FILE );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncCreateDirectory( const char* pDirName )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the IO parameters.
#if defined(TARGET_PS2)
    g_MemcardHardware.SetIOParams( (const char*)xfs("/%s",pDirName), NULL, 0, 0 );
#else
    g_MemcardHardware.SetIOParams( pDirName, NULL, 0, 0 );
#endif

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_CREATE_DIR );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncSetDirectory( const char* pDirName )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the IO parameters.
#if defined(TARGET_PS2)
    g_MemcardHardware.SetIOParams( (const char*)xfs("/%s",pDirName), NULL, 0, 0 );
#else
    g_MemcardHardware.SetIOParams( pDirName, NULL, 0, 0 );
#endif
    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_SET_DIR );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::AsyncDeleteDirectory( const char* pDirName )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( m_bIsEngaged );

    // Set the IO parameters.
#if defined(TARGET_PS2)
    g_MemcardHardware.SetIOParams( (const char*)xfs("/%s",pDirName), NULL, 0, 0 );
#else
    g_MemcardHardware.SetIOParams( pDirName, NULL, 0, 0 );
#endif

    // Set the current operation
    g_MemcardHardware.SetOperation( MEMCARD_OP_DELETE_DIR );

    // Do it.
    g_MemcardHardware.InitiateOperation();
}

//------------------------------------------------------------------------------

void memcard_mgr::SetIconDisplayName( const char* pName )
{
#ifdef TARGET_PS2
    g_MemcardHardware.SetIconDisplayName( pName );
#endif
}

//------------------------------------------------------------------------------

#define SWITCH_LABEL(x) case x: return #x;

const char* GetStateName( memcard_error State )
{
    switch( State )
    {
        SWITCH_LABEL( MEMCARD_END_OF_LIST);   
        SWITCH_LABEL( MEMCARD_SUCCESS);        
        SWITCH_LABEL( MEMCARD_IN_PROGRESS);    
        SWITCH_LABEL( MEMCARD_FATAL_ERROR);    
        SWITCH_LABEL( MEMCARD_BUSY);           
        SWITCH_LABEL( MEMCARD_NOT_A_MEMCARD);  
        SWITCH_LABEL( MEMCARD_NO_CARD);        
        SWITCH_LABEL( MEMCARD_WORN_OUT);       
        SWITCH_LABEL( MEMCARD_WRONG_REGION);   
        SWITCH_LABEL( MEMCARD_DAMAGED);            
        SWITCH_LABEL( MEMCARD_FILE_NOT_FOUND);     
        SWITCH_LABEL( MEMCARD_FILE_ALREADY_EXISTS);
        SWITCH_LABEL( MEMCARD_NO_FILES_AVAILABLE); 
        SWITCH_LABEL( MEMCARD_NOT_ENOUGH_SPACE);   
        SWITCH_LABEL( MEMCARD_ACCESS_DENIED);      
        SWITCH_LABEL( MEMCARD_PAST_END_OF_FILE);   
        SWITCH_LABEL( MEMCARD_FILENAME_TOO_LONG);  
        SWITCH_LABEL( MEMCARD_IO_CANCELED);        
        SWITCH_LABEL( MEMCARD_INCOMPATIBLE);       
        SWITCH_LABEL( MEMCARD_CARD_CHANGED);       
        SWITCH_LABEL( MEMCARD_UNFORMATTED);        
        SWITCH_LABEL( MEMCARD_FULL);               
    default: return "<unknown>";
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::SetState( memcard_error NewState )
{ 
    if( m_Error != NewState )
    {
    //  LOG_MESSAGE( "memcard_mgr::SetState","Error state set to %s", GetStateName(NewState) );
    }
    m_Error = NewState; 
}

//------------------------------------------------------------------------------
f32 memcard_mgr::GetProgress( void )
{
    if( g_MemcardHardware.GetFileLength() )
    {
        return (f32)g_MemcardHardware.GetFilePosition() / (f32)g_MemcardHardware.GetFileLength();
    }
    return 0.0f;
}