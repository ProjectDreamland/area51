//==============================================================================
//
//  Physics.hpp
//
//  Common header file included by physics components
//
//==============================================================================

#ifndef __PHYSICS_HPP__
#define __PHYSICS_HPP__

//==============================================================================
// INCLUDES
//==============================================================================

#include "x_files.hpp"

//==============================================================================
// DEBUG CONTROL
//==============================================================================

// Turn on physics profiling and debug draw for non-retail builds
// (they are accessible via the general page on the games debug menu)
#if !defined( X_RETAIL ) || defined( sbroumley )
#define ENABLE_PHYSICS_DEBUG
#endif


//==============================================================================
// PROFILE MACROS - Used to track stats in non-retail builds
//==============================================================================

// Turn on profile macros?
#ifdef ENABLE_PHYSICS_DEBUG

#define PHYSICS_DEBUG_RESET_TIMER( __timer__ )        (__timer__).Reset()
#define PHYSICS_DEBUG_START_TIMER( __timer__ )        (__timer__).Start()
#define PHYSICS_DEBUG_STOP_TIMER( __timer__ )         (__timer__).Stop()

#define PHYSICS_DEBUG_SET_COUNT( __var__, __value__ ) (__var__) = (__value__)
#define PHYSICS_DEBUG_ZERO_COUNT( __var__ )           (__var__) = 0
#define PHYSICS_DEBUG_INC_COUNT( __var__ )            (__var__)++
#define PHYSICS_DEBUG_ADD_COUNT( __var__, __add__ )   (__var__) += (__add__)

#define PHYSICS_DEBUG_STATIC_MEM_ALLOC( __amount__ )  g_PhysicsMgr.m_Profile.m_StaticMemoryUsed += (__amount__)
#define PHYSICS_DEBUG_STATIC_MEM_FREE( __amount__ )   g_PhysicsMgr.m_Profile.m_StaticMemoryUsed -= (__amount__)

#define PHYSICS_DEBUG_DYNAMIC_MEM_ALLOC( __amount__ ) g_PhysicsMgr.m_Profile.m_DynamicMemoryUsed += (__amount__)
#define PHYSICS_DEBUG_DYNAMIC_MEM_FREE( __amount__ )  g_PhysicsMgr.m_Profile.m_DynamicMemoryUsed -= (__amount__)

#else

#define PHYSICS_DEBUG_RESET_TIMER( __timer__ )        ( (void)0 )
#define PHYSICS_DEBUG_START_TIMER( __timer__ )        ( (void)0 )
#define PHYSICS_DEBUG_STOP_TIMER( __timer__ )         ( (void)0 )

#define PHYSICS_DEBUG_SET_COUNT( __var__, __value__ ) ( (void)0 )
#define PHYSICS_DEBUG_ZERO_COUNT( __var__ )           ( (void)0 )
#define PHYSICS_DEBUG_INC_COUNT( __var__ )            ( (void)0 )
#define PHYSICS_DEBUG_ADD_COUNT( __var__, __add__ )   ( (void)0 )

#define PHYSICS_DEBUG_STATIC_MEM_ALLOC( __amount__ )  ( (void)0 )
#define PHYSICS_DEBUG_STATIC_MEM_FREE( __amount__ )   ( (void)0 )

#define PHYSICS_DEBUG_DYNAMIC_MEM_ALLOC( __amount__ ) ( (void)0 )
#define PHYSICS_DEBUG_DYNAMIC_MEM_FREE( __amount__ )  ( (void)0 )

#endif

//==============================================================================

#endif  //#define __PHYSICS_HPP__
