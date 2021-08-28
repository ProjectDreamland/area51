#ifndef AUDIO_RAM_MGR_HPP
#define AUDIO_RAM_MGR_HPP

#include "x_types.hpp"
#include "audio_io_request.hpp"
#include "x_threads.hpp"

class audio_ram_mgr
{

friend void AudioProcessEndOfRequest( void );
friend void AudioRequestDispatcher( void );
friend void AudioServiceQueue( void );
friend void AudioServiceCurrentRequest( void );

//------------------------------------------------------------------------------
// Public typedefs.

public:

typedef void* ram_handle;

//------------------------------------------------------------------------------
// Public enums.

enum allocation_method 
{
        BEST_FIT = 0,
        FROM_TOP,
};

enum heap_type 
{
        AUDIO_RAM = 0,
        AUXILARY_RAM,
        IOP_RAM,
        MAIN_RAM,
        NUM_HEAPS,
};

//------------------------------------------------------------------------------
// Public types.

public:

struct reference            
{                           
        ram_handle          Handle;
        s32                 ReferenceCount;
};

//------------------------------------------------------------------------------
// Public functions.

public:

                            audio_ram_mgr           ( void );
                           ~audio_ram_mgr           ( void );
        void                Init                    ( void );
        void                Kill                    ( void );

        ram_handle          NewHandle               ( heap_type HeapType, s32 Count );
        void                FreeHandle              ( ram_handle Handle );
        void*               GetAddress              ( ram_handle Handle );
        heap_type           GetRamType              ( ram_handle Handle );
        s32                 GetBlockSize            ( ram_handle Handle );
        s32                 GetUserSize             ( ram_handle Handle );
        reference*          GetReference            ( ram_handle Handle );
        void                SetReference            ( ram_handle Handle, reference* pReference );
    
        s32                 GetFreeMemory           ( heap_type HeapType );
        s32                 GetUsedMemory           ( heap_type HeapType );
        s32                 GetTotalMemory          ( heap_type HeapType );
        s32                 GetBigBlock             ( heap_type HeapType );
        allocation_method   GetAllocationMethod     ( heap_type HeapType );
        s32                 SetAllocationMethod     ( heap_type HeapType, allocation_method Method );
        
        void                QueueRequest            ( audio_io_request* pRequest );    
        void                CancelRequest           ( audio_io_request* pRequest );
};

extern audio_ram_mgr g_AudioRamMgr;

#endif
