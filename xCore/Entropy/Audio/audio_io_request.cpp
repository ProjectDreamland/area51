#include "x_files.hpp"
#include "audio_io_request.hpp"

//------------------------------------------------------------------------------

audio_io_request::audio_io_request( void )
{
    m_pPrev       = NULL;
    m_pNext       = NULL;
    m_Destination = NULL;
    m_Source      = NULL;
    m_pCallback   = NULL;
    m_Status      = NOT_QUEUED;
    m_Length      = 0;
}

//------------------------------------------------------------------------------

audio_io_request::~audio_io_request( void )
{
}

//------------------------------------------------------------------------------

void audio_io_request::SetRequest( transfer_type    Type,
                                   void*            Destination, 
                                   void*            Source, 
                                   s32              Length, 
                                   request_priority Priority,
                                   callback_fn*     pCallback )
{
    // Must be unqueued!
    ASSERT( m_Status != PENDING );
    ASSERT( m_Status != IN_PROGRESS );

    // Set up the request.
    m_Type        = Type; 
    m_Destination = Destination;
    m_Source      = Source;
    m_Length      = Length;    
    m_Priority    = Priority;
    m_Status      = NOT_QUEUED;
    m_pCallback   = pCallback;
}
