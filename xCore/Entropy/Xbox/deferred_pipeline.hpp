#ifndef _DEFERRED_PIPELINE_HPP_
#define _DEFERRED_PIPELINE_HPP_



///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////



#include "../xCore/x_files/x_threads.hpp"
#include "xbox_private.hpp"



///////////////////////////////////////////////////////////////////////////////
// EXTERNALS
///////////////////////////////////////////////////////////////////////////////



extern IDirect3DDevice8* g_pd3dInternal;

extern void eng_SyncVBlank( void );



///////////////////////////////////////////////////////////////////////////////
// EQUATES
///////////////////////////////////////////////////////////////////////////////



#define SWITCH_MULTITHREADED 1
#define MAX_CMD_STREAM       2
#define MAX_CMD_BUFFER       262144*6
#define MAX_CMD_STACK         32768



///////////////////////////////////////////////////////////////////////////
// MULTITHREADED PUSH MANAGER: HANDLES UPDATE/RENDER THREAD SYNCHRONISATION
///////////////////////////////////////////////////////////////////////////



//! Semaphore object
/** This structure defines the semaphore object.
    */

struct semaphore
{
    // --------------------------------------------------------------------

    /** @{\name Semaphore construction ....................................
        Functionality for creating and destroying semaphores.
        */

    semaphore( s32 Count );
~   semaphore( void );

    //  @}

    // --------------------------------------------------------------------

    /** @{\name Semaphore acquisition .....................................
        Functionality for sleeping until semaphore is signalled.
        */

    //! Release semaphore.
    /** This routine decrements the semaphore count.
        */

    bool release( s32 Count=1 );

    //! Acquire semaphore.
    /** This routine puts the calling thread to sleep until
        the semaphore is signalled.
        */

    void acquire( void );

    //  @}

    // --------------------------------------------------------------------

    //! Internal data
    /** This member holds proxy required data.
        */

    void* m_pHandles[1];
};



///////////////////////////////////////////////////////////////////////////



//! Event object.
/** This structure is used to signal events between threads.
    */

struct event
{
    // ----------------------------------------------------------------------------------

    /** @{\name Event construction ..................................................
        Functionality for creating and destroying events.
        */

    event( bool );
~   event( void );

    //  @}



    // ----------------------------------------------------------------------------------

    /** @{\name Event acquisition ...................................................
        Functionality for sleeping until event is signalled.
        */

    //! Acquire event.
    /** This routine puts the calling thread to sleep until
        the event is signalled.
        */

    void acquire( void );

    //! Signal event
    /** This routine causes any threads waiting on
        this event to begin execution.
        */

    void signal( void );

    //  @}



    // ----------------------------------------------------------------------------------

private:

    //! Internal data
    /** This member holds proxy required data.
        */

    void* m_pHandles[1];
};



///////////////////////////////////////////////////////////////////////////



//! Push buffer manager
/** This class is used to synchronise the main thread with the new
    rendering thread.
    */

extern class push_mgr
{
    //! Queue class
    /** This is a slightly different implementation from the queue
        class seen in the memory card manager. It's a bit simpler. 
        */

    template< class t,s32 Capacity >struct queue
    {
        // ----------------------------------------------------------------

        //! Default constructor.
        /** This routine is designed for use by other defaults.
            */

        queue( void )
        {
        #ifndef X_RETAIL
            x_memset( m_Pool,-1,sizeof(t)*Capacity );
        #endif
            m_Count = 0;
        }

        //! Destructor.
        /** This is the place to destroy all those lovely little
            member function pointers.
            */

    ~   queue( void )
        {
        }

        // ----------------------------------------------------------------

        /** @{\name State operations ......................................
            Functionality for pushing,popping,and querying the state queue.
            */

        //! Bracket operator
        /** This routine returns the Nth byte from
            the top of the stack.
            */

        t& operator [] ( s32 Index )
        {
            return m_Pool[Capacity-Index];
        }

        //! Push state
        /** This routine pushes the current state onto the stack.
            DO NOT PUSH IN REVERSE ORDER!!
            */

        t* alloc( u32 Count )
        {
            s32 Index = Capacity-(m_Count += Count);
            t* Result = m_Pool+Index;
            ASSERT( Index >-1 );
            return Result;
        }

        //! Flush states
        /** This routine empties the queue.
            */

        void flush( void )
        {
            m_Count=0;
        }

        //! Get queue count
        /** This routine returns the number of items in the queue.
            */

        s32 count( void )
        {
            return m_Count;
        }

        //! Get size of queue
        /** This routine returns the size of the queue in bytes.
            */

        u32 size( void )
        {
            return sizeof(t)*m_Count;
        }

        //  @}

        // ----------------------------------------------------------------

    private:

        //! Stack elements
        /** This member contains all the objects in the queue.
            */

        t m_Pool[Capacity];

        //! Item count.
        /** This member tells how many objects are maintained
            in the queue.
            */

        s32 m_Count;
    };



    ///////////////////////////////////////////////////////////////////////

public:

    //! Default constructor
    /** This routine does little else than initialise the
        semaphore and all internal objects.
        */

    push_mgr( void );



    ///////////////////////////////////////////////////////////////////////



    //! Rendering thread
    /** This routine is responsible for feeding the GPU when the
        kick() method is called.
        */

    static void GpuFeed( void );



    ///////////////////////////////////////////////////////////////////////

public:

    //! Write pointer param
    /** This routine is similar to the previous write command,
        the only difference being that the data the pointer
        references will be allocated later. The Arg pointer
        is used for pointer fixup.
        */

    template< class t >void* __fastcall write( void* Arg,t* Ptr )
    {
        void* pResult = write( &Ptr, sizeof(t*) );
        if( Ptr )
            *((void**)pResult)=Arg;
        return pResult;
    }

    //! Write data
    /** The passed data is copied into the pending alloc buffer.
        */

    void* __fastcall write( const void* pData,s32 Count );

    //! Write function param
    /** The passed data is written to the pending alloc buffer
        so it can be copied onto the stack later.
        */

    template< class t >void* __fastcall write( t& Param )
    {
        return write( &Param,sizeof(t) );
    }

    //! Write pointer param
    /** This routine is similar to the previous write command,
        the only difference being that the data the pointer
        references will be allocated later.
        */

    template< class t >void* __fastcall write( t* Ptr )
    {
        return write( &Ptr, sizeof(t*) );
    }

    //! Exchange alloc buffers
    /** This routine flips the current and pending
        alloc buffers. IT IS CALLED FROM THE UPDATE
        THREAD.
        */

    void flip( void );

    //! Commit all pending commands.
    /** This routine causes the render thread to wake up
        and start spitting commands out to the GPU. MUST
        BE CALLED FROM THE RENDER THREAD.
        */

    void kick( void );



    ///////////////////////////////////////////////////////////////////////////

    //! Front stream
    /** This routine returns the stream for processing.
        */

    queue<u8,MAX_CMD_BUFFER>& front( void )
    {
        SemCritical.acquire();
        queue<u8,MAX_CMD_BUFFER>& Front = m_pStream[(m_StreamIndex-1) % MAX_CMD_STREAM];
        SemCritical.release();
        return Front;
    }

    //! Back stream
    /** This routine returns the back stream.
        */

    queue<u8,MAX_CMD_BUFFER>& back( void )
    {
        SemCritical.acquire();
        queue<u8,MAX_CMD_BUFFER>& Back = m_pStream[m_StreamIndex % MAX_CMD_STREAM];
        SemCritical.release();
        return Back;
    }

    //! Back stream
    /** This routine returns the back stream.
        */

    void* back( s32 Index )
    {
        return &back()[Index];
    }

    //! Allocate packet
    /** This routine allocates a new packet for execution in the render thread.
        The packet is structured: STACK-SIZE,FN-PTR,PARAMS[,extra],PACKET-SIZE.
        */

    template< class t >void __fastcall begin( t Fn )
    {
        SemGPU.acquire     ();
        Base = back().count();
        Mark = BytesUsed = 0;
        APID = s32( Fn );
        Lock = true;

        write( BytesUsed );
    }

    //! End packet construction
    /** This routine wraps up the packet by updating the size.
        */

    void __fastcall end( s32 ParamCount );



    ///////////////////////////////////////////////////////////////////////

private:

    //! Begin/end data
    /** These members are used to store information between begin and end.
        */

    s32 Lock,Base,BytesUsed,Mark,APID;



    ///////////////////////////////////////////////////////////////////////

    //! Stream semaphore.
    /** This member protects any stream manipulation. So we don't
        ever get the situation where back and front streams lose
        their meaning.
        */

    semaphore SemCritical;

    //! GPU semamphore
    /** This member protects the rendering commands buffers
        from overwrites by an over zealous update thread.
        */

    semaphore SemGPU;

    //! Synchronisation flare.
    /** This member controls when the render thread wakes
        up to start feeding the GPU. Set in flip().
        */

    event Flare;



    ///////////////////////////////////////////////////////////////////////



    //! Push buffer.
    /** This construct is used for serialising between the update and render
        threads. This buffering technique ensures that the GPU never stalls.
        Duplicated data (such as ris and vert arrays) are allocated inline
        with commands.
        */

    queue<u8,MAX_CMD_BUFFER>*m_pStream;

    //! Stream page
    /** This member tells us which stream is being written to.
        */

    u32 m_StreamIndex;
}
g_Cmds;



///////////////////////////////////////////////////////////////////////////////
// FORCED INLINE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//
//  This routine copies a block of data like x_memcpy up to 512-bits at a time.
//  So all push manager writes are 64-bit aligned.
//
__forceinline __declspec( naked )void __fastcall fastcopy( const void* pDst,const void* pSrc,s32 Bytes )
{
    __asm
    {
        // skip void lengths --------------------------------------------------

        mov     eax,[esp+4]
        test    eax,eax
        jne    _cont
                ret     4

        // pick best copy method ----------------------------------------------

_cont:  push    edi
        push    esi
                mov     edi,ecx
                mov     esi,edx
                mov     ecx,eax
                cmp     ecx,64
                jl     _copy

                // 512-bit fast method ........................................

                shr     ecx,0x6
                mov     edx,ecx
        _loop:
                movq    mm0,[esi   ]
                movq    mm1,[esi+ 8]
                movq    mm2,[esi+16]
                movq    mm3,[esi+24]
                movq    mm4,[esi+32]
                movq    mm5,[esi+40]
                movq    mm6,[esi+48]
                movq    mm7,[esi+56]
                movq    [edi       ],mm0
                movq    [edi    + 8],mm1
                movq    [edi    +16],mm2
                movq    [edi    +24],mm3
                movq    [edi    +32],mm4
                movq    [edi    +40],mm5
                movq    [edi    +48],mm6
                movq    [edi    +56],mm7
                lea     esi,[esi+64]
                lea     edi,[edi+64]
                dec     ecx
	            jnz    _loop

                emms

                shl     edx,0x6
                mov     ecx,eax
                sub     ecx,edx
        _copy:
                shr     ecx,0x2
                rep     movsd
        pop     esi
        pop     edi
        ret     4
    }
}

///////////////////////////////////////////////////////////////////////////////

__forceinline void* __fastcall push_mgr::write( const void* pData,s32 Bytes )
{
    ASSERT( Lock );
    u8* pResult= 0;
    {
        if( pData )
        {
            // MMX has 64-bit registers
            Bytes=( Bytes + 3) & ~3;

            pResult = back().alloc( Bytes );
            fastcopy( pResult,pData,Bytes );
            BytesUsed += Bytes; // last
        }
    }
    return pResult;
}

///////////////////////////////////////////////////////////////////////////////

__forceinline void __fastcall push_mgr::end( s32 Args )
{
    write( APID ); // Function pointer
    write( Args ); // Parameter mark
    // Indexes right to left
    *((s32*)back(Base+4))=BytesUsed-4;
    Lock = false;
#if !SWITCH_MULTITHREADED
    kick();
#endif
    SemGPU.release();
}

#endif
