#include "io_request.hpp"
#include "io_filesystem.hpp"

//==============================================================================

io_request::io_request( void ) : m_Semaphore(1,0)
{
    m_pPrev      = NULL;
    m_pNext      = NULL;
    m_pOpenFile  = NULL;
    m_pBuffer    = NULL;
    m_pCallback  = NULL;
    m_Status     = NOT_QUEUED;
    m_Offset     = 0;
    m_Length     = 0;
    m_Sequence   = 0;
    m_UserData   = 0;
}

//==============================================================================

io_request::~io_request( void )
{
}

//==============================================================================

void io_request::SetRequest( io_open_file*   pOpenFile, 
                             void*           pBuffer,
                             s32             Offset,
                             s32             Length,
                             priority        Priority,
                             xbool           UseSema,
                             u32             Destination,
                             u32             UserData,
                             operation       Operation, 
                             callback_fn*    pCallback )
{
    // Must be unqueued!
    ASSERT( m_Status != QUEUED );
    ASSERT( m_Status != PENDING );
    ASSERT( m_Status != IN_PROGRESS );

    // Better not be in a list...
    ASSERT( m_pNext == NULL );
    ASSERT( m_pPrev == NULL );

    // Has to be 32k alligned for crc's
    if( pOpenFile->bEnableChecksum )
    {
        ASSERT( (Offset & 0x7fff) == 0 );
        ASSERT( (Length & 0x7fff) == 0 );
    }

    // Set up the request.
    m_pOpenFile   = pOpenFile; 
    m_pBuffer     = pBuffer;
    m_Offset      = Offset;    
    m_Length      = Length;    
    m_Priority    = Priority;
    m_Status      = NOT_QUEUED;
    m_UseSema     = UseSema;
    m_Destination = Destination,
    m_UserData    = UserData;
    m_Operation   = Operation;
    m_pCallback   = pCallback; 
}
