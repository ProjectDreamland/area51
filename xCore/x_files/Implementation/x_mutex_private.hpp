//==============================================================================
//
//  x_threads.hpp
//
//==============================================================================

#ifndef _X_MUTEX_HPP
#define _X_MUTEX_HPP

#include "x_types.hpp"
#include "x_semaphore_private.hpp"

#define X_MAX_MUTEXES           16

//==============================================================================
// MUTEX
// Since the implementation of semaphores is significantly more optimal than
// using a system call, the additional overhead for implementing a mutex by 
// using a single entry semaphore is minimal. To keep code down, let's just make
// it a semaphore! Only difference is that mutexes should have been Entered()
// before they can be Exited()
//==============================================================================
// When a mutex is initially constructed, it is unlocked
class xmutex
{
public:
                        xmutex          ( void );
                       ~xmutex          ( void );

            xbool        Enter            ( s32 Flags );
            xbool       Exit            ( s32 Flags );

            void        Enter           ( void )            { Enter(X_TH_BLOCK); };
            void        Exit            ( void )            { Exit(X_TH_BLOCK);  };

            void        Acquire         ( void )            { Enter(X_TH_BLOCK); };          // Same functionality as enter
            void        Release         ( void )            { Exit(X_TH_BLOCK);  };          // Same functionality as exit

            xbool       Acquire         ( s32 Flags )       { return Enter(Flags);  };
            xbool       Release         ( s32 Flags )       { return Exit(Flags);  };
            xbool       IsLocked        ( void )            { return m_EnterCount > 0; };

protected:
            xthread*    m_pOwner;
            s32         m_EnterCount;
            xbool       m_Initialized;
            xsema       m_Semaphore;
protected:
#ifdef DEBUG_THREADS
static      xarray<xmutex*> m_MasterMutexList;
#endif

};

#endif
