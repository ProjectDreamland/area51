#ifndef E_PROFILE_HPP
#define E_PROFILE_HPP

#define ENABLE_CPU_TIMER    0

//=============================================================================

extern xbool g_EnableCPUTimer;

//=============================================================================

class cpu_timer
{
public:
            cpu_timer( void );
    f32     GetTimeMs( xbool DoReset = TRUE );

protected:

    #if ENABLE_CPU_TIMER == 1
    xtimer m_Timer;
    #endif
};

//=============================================================================

inline
cpu_timer::cpu_timer( void )
{
    #if ENABLE_CPU_TIMER == 1
    
    if( g_EnableCPUTimer == TRUE )
    {
        m_Timer.Start();
    }
    
    #endif
}

//=============================================================================

inline
f32 cpu_timer::GetTimeMs( xbool DoReset )
{
    #if ENABLE_CPU_TIMER == 1
    
    if( g_EnableCPUTimer == TRUE )
    {
        f32 Time = m_Timer.ReadMs();
        
        if( DoReset == TRUE )
        {
            m_Timer.Reset();
            m_Timer.Start();
        }
        
        return( Time );
    }
    
    #endif
    (void)DoReset;
    
    return( 0.0f );
}

//=============================================================================

#endif
