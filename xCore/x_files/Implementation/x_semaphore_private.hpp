//==============================================================================
//
//  x_SEMAPHORE_PRIVATE.hpp
//
//==============================================================================

#ifndef _X_SEMAPHORE_HPP
#define _X_SEMAPHORE_HPP

#define X_MAX_SEMAPHORES        16

//==============================================================================
// SEMAPHORES (defined in x_mutex.cpp)
//==============================================================================
// These provide the base level atomic synchronisation primitives. All higher 
// level thread synchronisation is performed using semaphores.

class xsema
{
public:
                        xsema           ( s32 count,s32 initial);
                       ~xsema           ( void );

            xbool       Acquire         ( s32 Flags );
            xbool       Release         ( s32 Flags );

            void        Acquire         ( void )            { Acquire(X_TH_BLOCK); };
            void         Release            ( void )            { Release(X_TH_BLOCK); };

protected:
            xbool       m_Initialized;
            xthreadlist m_WaitingAcquire;
            xthreadlist m_WaitingRelease;
            s32         m_Count;
            s32         m_Available;
protected:
#ifdef DEBUG_THREADS
static      xarray<xsema*> m_MasterSemaphoreList;
#endif
};

#endif