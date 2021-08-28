#ifndef AUDIO_IO_REQUEST_HPP
#define AUDIO_IO_REQUEST_HPP

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

class audio_io_request
{

friend void AudioProcessEndOfRequest( void );
friend void AudioRequestDispatcher( void );
friend void AudioServiceQueue( void );
friend void AudioServiceCurrentRequest( void );
friend class audio_ram_mgr;

//------------------------------------------------------------------------------
// Public types

public:

enum request_priority
{
            HIGH_PRIORITY = 0,
            MEDIUM_PRIORITY,     
            LOW_PRIORITY,
            NUM_PRIORITIES,
};

enum request_status
{
            NOT_QUEUED = 0,
            PENDING,
            IN_PROGRESS,
            COMPLETED,
            FAILED,
};

enum transfer_type
{
            UPLOAD = 0, // UPLOAD:   Source = MAIN RAM,   Destination = ram_handle
            DOWNLOAD,   // DOWNLOAD: Source = ram_handle, Destination = MAIN RAM
            COPY,       // COPY:     Source = ram_handle, Destination = ram_handle
};

typedef     void                callback_fn     ( audio_io_request* pRequest );

//------------------------------------------------------------------------------
// Private data

private:

            audio_io_request*   m_pPrev;
            audio_io_request*   m_pNext;
            void*               m_Destination;
            void*               m_Source;
            s32                 m_Length;
            transfer_type       m_Type;
            request_priority    m_Priority;
            callback_fn*        m_pCallback;
            s32                 m_Sequence;
volatile    request_status      m_Status;

//------------------------------------------------------------------------------
// Public functions

public:

                                audio_io_request( void );
                               ~audio_io_request( void );

            void                SetRequest      ( transfer_type     Type,
                                                  void*             Destination, 
                                                  void*             Source, 
                                                  s32               Length, 
                                                  request_priority  Priority,
                                                  callback_fn*      pCallback = NULL );
            
inline      transfer_type       GetType         ( void )    { return m_Type; }          
inline      void*               GetDestination  ( void )    { return m_Destination; }
inline      void*               GetSource       ( void )    { return m_Source; }
inline      s32                 GetLength       ( void )    { return m_Length; }
inline      request_priority    GetPriority     ( void )    { return m_Priority; }
inline      callback_fn*        GetCallback     ( void )    { return m_pCallback; }
inline      request_status      GetStatus       ( void )    { return m_Status; }
};

#endif
