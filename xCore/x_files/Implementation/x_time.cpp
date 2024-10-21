//==============================================================================
//  
//  x_time.cpp
//
//  TODO - Make nice and inline happy for performance.
//
//==============================================================================

#include "..\x_context.hpp"

#ifndef X_PLUS_HPP
#include "..\x_time.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "..\x_debug.hpp"
#endif

//==============================================================================
//  PLATFORM SPECIFIC CODE
//==============================================================================
//
//  Must provide the following:
//    - Functions x_TimeInit(), x_TimeKill(), and x_GetTime().
//    - Definition for XTICKS_PER_MS which is one millisecond in xticks.
//  
//==============================================================================

//******************************************************************************
#ifdef TARGET_PS2
//==============================================================================
// This is defined as BUSCLK / 256 / 1000 (BUSCLK == 147.456Mhz)
#define XTICKS_PER_MS       576
#define XTICKS_PER_SECOND   (XTICKS_PER_MS * 1000)

#include "eeregs.h"

static volatile xtick s_TimerWrapCount;
static s32 s_HandlerId;

//==============================================================================
static s32 s_TimerWrapCallback(s32)
{
    // Overflow flag MUST have been set
    ASSERT(DGET_T0_MODE() & (1<<11) );
    // Make sure timer will fire off an interrupt when it wraps again
    DPUT_T0_MODE( DGET_T0_MODE());
    s_TimerWrapCount++;
    ExitHandler();
    return 0;
}

//==============================================================================
void x_TimeInit( void )
{
    s_HandlerId = AddIntcHandler(INTC_TIM0,s_TimerWrapCallback,0);
    ASSERT(s_HandlerId >= 0);
    s_TimerWrapCount=0;
    DPUT_T0_COUNT(0);
    DPUT_T0_MODE(   (2<<0)|         // Clock is BUSCLK/256
                    (1<<7)|         // Continue counting
                    (1<<9)          // Overflow will generate an interrupt
                    );
    EnableIntc(INTC_TIM0);
}

//==============================================================================
void x_TimeKill( void )
{    
    DisableIntc(INTC_TIM0);
    RemoveIntcHandler(INTC_TIM0,s_HandlerId);
}

//==============================================================================
xtick x_GetTime( void )
{
    xtick Ret;
    xtick start;
    u16   time;

    
    start = s_TimerWrapCount;

    time = *T0_COUNT;
    // If an interrupt occured between the wrap read and the hw timer read,
    // then we reload everything.
    if (start!= s_TimerWrapCount)
    {
        start = s_TimerWrapCount;
        time = *T0_COUNT;
    }

    Ret = (start << 16) | time;

    return( Ret );
}

//==============================================================================
#endif // TARGET_PS2
//******************************************************************************

//******************************************************************************
#if defined(TARGET_PC) || defined(TARGET_XBOX)
//==============================================================================

#if defined(TARGET_PC)
#include <windows.h>
#else
#ifdef CONFIG_RETAIL
#   define D3DCOMPILE_PUREDEVICE 1	
#endif
#include <xtl.h>
#endif

#define XTICKS_PER_MS       s_PCFrequency
#define XTICKS_PER_SECOND   s_PCFrequency2

static f64 s_PCFrequency;
static f64 s_PCFrequency2;
static xtick s_BaseTimeTick;

//==============================================================================

void x_TimeInit( void )
{    
    LARGE_INTEGER C;

    QueryPerformanceCounter( &C );

    LARGE_INTEGER F;
    QueryPerformanceFrequency( &F );
    s_PCFrequency  = (f64)F.QuadPart / 1000.0;
    s_PCFrequency2 = (f64)F.QuadPart;
    s_BaseTimeTick = (xtick)C.QuadPart;
}

//==============================================================================

void x_TimeKill( void )
{    
}

//==============================================================================

xtick x_GetTime( void )
{
    static xtick LastTicks = 0;
    
    xtick Ticks;
    LARGE_INTEGER C;

    QueryPerformanceCounter( &C );
    Ticks = (xtick)(C.QuadPart)-s_BaseTimeTick;

    if( Ticks < LastTicks )     
        Ticks = LastTicks + 1;

    LastTicks = Ticks;

    return( Ticks );
}

//==============================================================================

//==============================================================================
#endif // TARGET_PC
//******************************************************************************

//******************************************************************************
#ifdef TARGET_GCN
//==============================================================================

#include <dolphin/os.h>

#define XTICKS_PER_MS       (OS_TIMER_CLOCK / 1000)
#define XTICKS_PER_SECOND   (OS_TIMER_CLOCK)

OSTime s_BaseTick;
//==============================================================================

void x_TimeInit( void )
{    

    s_BaseTick = OSGetTime();
}

//==============================================================================

void x_TimeKill( void )
{    
}

//==============================================================================

xtick x_GetTime( void )
{   
    xtick Ticks;

    OSTime  T = (OSGetTime()-s_BaseTick);
    
    Ticks = (s64)T;

    return( Ticks );
}

//==============================================================================

//==============================================================================
#endif // TARGET_GCN
//******************************************************************************



//==============================================================================
//  PLATFORM INDEPENDENT CODE
//==============================================================================


#ifdef TARGET_PS2
#define ONE_HOUR ((s32)XTICKS_PER_MS * 1000 * 60 * 60)
#if XTICKS_PER_MS != 576
#error Craig: XTICKS_PER_MS has changed from 576 x_time will no longer work correctly
#endif
#else
#define ONE_HOUR ((s64)XTICKS_PER_MS * 1000 * 60 * 60)
#endif


#define ONE_DAY  ((s64)XTICKS_PER_MS * 1000 * 60 * 60 * 24)

//
// This is so we can see these values in the debugger
//
xtick s_XTICKS_PER_MS   = (xtick)XTICKS_PER_MS;
xtick s_XTICKS_PER_DAY  = (xtick)ONE_DAY;
xtick s_XTICKS_PER_HOUR = (xtick)ONE_HOUR;

//==============================================================================

s64 x_GetTicksPerMs ( void )
{
    return( (s64)XTICKS_PER_MS );
}

//==============================================================================

s64 x_GetTicksPerSecond ( void )
{
    return( (s64)XTICKS_PER_SECOND );
}

//==============================================================================

f32 x_TicksToMs( xtick Ticks )
{
#ifdef TARGET_PS2
    ASSERT( Ticks < ONE_HOUR );
    return( ((s32)Ticks) / (f32)XTICKS_PER_MS );
#else
    #ifndef X_EDITOR
        ASSERT( Ticks < ONE_DAY );
    #endif
    // We do the multiple casting here to try and preserve as much accuracy as possible
    //return( (f32)(     Ticks) / (f32)XTICKS_PER_MS );
    return f32(f64(Ticks)/f64(s_PCFrequency2)*1000);
#endif
}

//==============================================================================

f64 x_TicksToSec( xtick Ticks )
{
#ifdef TARGET_PS2
    return( ((u32)Ticks) / ((f32)XTICKS_PER_MS * 1000.0f) );
#else
    return( ((f64)Ticks) / ((f64)(XTICKS_PER_MS * 1000)) );
#endif
}

//==============================================================================

f64 x_GetTimeSec( void )
{
    return( x_TicksToSec( x_GetTime() ) );
}

//==============================================================================

xtimer::xtimer( void )
{
    m_Running   = FALSE;
    m_StartTime = 0;
    m_TotalTime = 0;
    m_NSamples  = 0;
}

//==============================================================================
#include "..\x_context.hpp"
void xtimer::Start( void )
{
    //CONTEXT( "xtimer::Start" );
    if( !m_Running )
    {
        m_StartTime = x_GetTime();
        m_Running   = TRUE;        
        m_NSamples++;
    }
}

//==============================================================================

void xtimer::Reset( void )
{
    //CONTEXT( "xtimer::Reset" );
    m_Running   = FALSE;
    m_StartTime = 0;
    m_TotalTime = 0;
    m_NSamples  = 0;
}

//==============================================================================

xtick xtimer::Stop( void )
{
    //CONTEXT( "xtimer::Stop" );
    if( m_Running )
    {
        m_TotalTime += x_GetTime() - m_StartTime;
        m_Running    = FALSE;
    }
    return( m_TotalTime );
}

//==============================================================================

f32 xtimer::StopMs( void )
{
    //CONTEXT( "xtimer::StopMs" );
    if( m_Running )
    {
        m_TotalTime += x_GetTime() - m_StartTime;
        m_Running    = FALSE;
    }

#ifdef TARGET_PS2
    ASSERT( m_TotalTime < ONE_HOUR );
    return( ((s32)m_TotalTime) / (f32)XTICKS_PER_MS );
#else
    #ifndef X_EDITOR
        ASSERT( m_TotalTime < ONE_DAY );
    #endif
    return( (f32)(m_TotalTime) / (f32)XTICKS_PER_MS );
#endif
}

//==============================================================================

f32 xtimer::StopSec( void )
{
    //CONTEXT( "xtimer::StopSec" );
    if( m_Running )
    {
        m_TotalTime += x_GetTime() - m_StartTime;
        m_Running    = FALSE;
    }

#ifdef TARGET_PS2
    ASSERT( m_TotalTime < ONE_HOUR );
    return( ((s32)m_TotalTime) / ((f32)XTICKS_PER_MS * 1000.0f) );
#else
    #ifndef X_EDITOR
        ASSERT( m_TotalTime < ONE_DAY );
    #endif
    return( (f32)(m_TotalTime) / ((f32)XTICKS_PER_MS * 1000.0f) );
#endif
}

//==============================================================================

xtick xtimer::Read( void ) const
{
    if( m_Running )  return( m_TotalTime + (x_GetTime() - m_StartTime) );
    else             return( m_TotalTime );
}

//==============================================================================

f32 xtimer::ReadMs( void ) const
{
    xtick Ticks;
    if( m_Running )  Ticks = m_TotalTime + (x_GetTime() - m_StartTime);
    else             Ticks = m_TotalTime;

#ifdef TARGET_PS2
    ASSERT( Ticks < ONE_HOUR );
    return( ((s32)Ticks) / (f32)XTICKS_PER_MS );
#else
    #ifndef X_EDITOR
        ASSERT( Ticks < ONE_DAY );
    #endif
    return( (f32)(Ticks) / (f32)XTICKS_PER_MS );
#endif
}

//==============================================================================

f32 xtimer::ReadSec( void ) const
{
    xtick Ticks;
    if( m_Running )  Ticks = m_TotalTime + (x_GetTime() - m_StartTime);
    else             Ticks = m_TotalTime;

#ifdef TARGET_PS2
    ASSERT( Ticks < ONE_HOUR );
    return( ((s32)Ticks) / ((f32)XTICKS_PER_MS * 1000.0f) );
#else
    return( (f32)(Ticks) / ((f32)XTICKS_PER_MS * 1000.0f) );
#endif
}

//==============================================================================

xtick xtimer::Trip( void )
{
    xtick Ticks = 0;
    if( m_Running )
    {
        xtick Now = x_GetTime();
        Ticks = m_TotalTime + (Now - m_StartTime);
        m_TotalTime = 0;
        m_StartTime = Now;
        m_NSamples++;
    }
    return( Ticks );
}

//==============================================================================

f32 xtimer::TripMs( void )
{
    xtick Ticks = 0;
    if( m_Running )
    {
        xtick Now = x_GetTime();
        Ticks = m_TotalTime + (Now - m_StartTime);
        m_TotalTime = 0;
        m_StartTime = Now;
        m_NSamples++;
    }

#ifdef TARGET_PS2
    ASSERT( Ticks < ONE_HOUR );
    return( ((s32)Ticks) / (f32)XTICKS_PER_MS );
#else
    #ifndef X_EDITOR
        ASSERT( Ticks < ONE_DAY );
    #endif
    return( (f32)(Ticks) / (f32)XTICKS_PER_MS );
#endif
}

//==============================================================================

f32 xtimer::TripSec( void )
{
    xtick Ticks = 0;
    if( m_Running )
    {
        xtick Now = x_GetTime();
        Ticks = m_TotalTime + (Now - m_StartTime);
        m_TotalTime = 0;
        m_StartTime = Now;
        m_NSamples++;
    }

#ifdef TARGET_PS2
    ASSERT( Ticks < ONE_HOUR );
    return( ((s32)Ticks) / ((f32)XTICKS_PER_MS * 1000.0f) );
#else
    #ifndef X_EDITOR
        ASSERT( Ticks < ONE_DAY );
    #endif
    return( (f32)(Ticks) / ((f32)XTICKS_PER_MS * 1000.0f) );
#endif
}

//==============================================================================

s32 xtimer::GetNSamples( void ) const
{
    return( m_NSamples );
}

//==============================================================================

f32 xtimer::GetAverageMs( void ) const
{
    if( m_NSamples <= 0 ) 
        return( 0 );

    return( ReadMs() / m_NSamples );
}

//==============================================================================
