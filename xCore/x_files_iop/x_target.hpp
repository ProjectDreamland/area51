//==============================================================================
//
//  x_target.hpp
//
//==============================================================================

#ifndef X_TARGET_HPP
#define X_TARGET_HPP

#ifndef X_ASSERT
//#define X_ASSERT
#endif

//==============================================================================
//  
//  This file assists with cross platform development by providing a standard
//  set of platform definitions with anticipated variations.  The selection of a
//  platform and variation is based on a key macro definition.
//  
//  The target macro is provided in different ways depending on the development 
//  environment.
//  
//    Target Macro .................... Description ................ MFC Safe?
//    --------------------------------------------------------------------------
//    <none> .......................... Assumes TARGET_PC_MFC .......... Y
//    TARGET_PC ....................... Generic PC application
//    TARGET_PC_MFC ................... Generic PC application ......... Y
//    TARGET_PS2_DEV .................. Sony PlayStation 2 DevKit
//    TARGET_PS2_CLIENT ............... Sony PlayStation 2 "Debug Station"
//    TARGET_PS2_DVD .................. Sony PlayStation 2 console
//    TARGET_GCN_DEV .................. Nintendo GameCube DevKit
//    TARGET_GCN_DVD .................. Nintendo GameCube console
//    TARGET_XBOX_DEV ................. Microsoft X-Box DevKit
//    TARGET_XBOX_DVD ................. Microsoft X-Box console
//    TARGET_UNIX_TEXT ................ Unix/Linux with text only
//    TARGET_UNIX_OGL .... ............ Unix/Linux with OpenGL graphics
//  
//  Most subsystems within the x_files are largely platform independent and are
//  generally not affected by the target specification, although platform 
//  specific optimizations are present where possible.  The following subsystems
//  are significantly affected by target:  file I/O, time, and debug.
//  
//  Targets which are "MFC safe" (and have _MFC in the macro) will disable the
//  x_files version of operators new and delete.  Graphic engines prepared for
//  MFC configurations should not provide a main() function.
//  
//==============================================================================
//  
//  When present, each of the primary target macros in the list above will, in
//  turn, cause other secondary macros to be defined.
//  
//    Target Macro ........ Secondary Macros
//    --------------------------------------------------------------------------
//    <none> .............. TARGET_PC      TARGET_MFC
//    TARGET_PC ...........                               
//    TARGET_PC_MFC ....... TARGET_PC      TARGET_MFC
//                                            
//    TARGET_UNIX_TEXT .... TARGET_UNIX
//    TARGET_UNIX_OGL  .... TARGET_UNIX                 TARGET_OGL
//    
//    TARGET_PS2_DEV ...... TARGET_PS2     TARGET_DEV
//    TARGET_PS2_CLIENT ... TARGET_PS2                              TARGET_CLIENT
//    TARGET_PS2_DVD ...... TARGET_PS2                  TARGET_DVD
//    TARGET_GCN_DEV ...... TARGET_GCN     TARGET_DEV
//    TARGET_GCN_DVD ...... TARGET_GCN                  TARGET_DVD
//    TARGET_XBOX_DEV ..... TARGET_XBOX    TARGET_DEV
//    TARGET_XBOX_DVD ..... TARGET_XBOX                 TARGET_DVD
//    
//==============================================================================

//==============================================================================
//  
//  Check for ambiguous or insufficiently qualified target specification.
//  
//  *** IF YOU GOT AN ERROR HERE ***, then you specified a target platform 
//  without sufficiently qualifying the target.
//  
//==============================================================================

#ifdef TARGET_PS2
#error TARGET_PS2 is not a sufficient target specification.
#error Use either TARGET_PS2_DEV, TARGET_PS2_CLIENT, or TARGET_PS2_DVD.
#endif

#ifdef TARGET_GCN
#error TARGET_GCN is not a sufficient target specification.
#error Use either TARGET_GCN_DEV or TARGET_GCN_DVD.
#endif

#ifdef TARGET_XBOX
#error TARGET_XBOX is not a sufficient target specification.
#error Use either TARGET_XBOX_DEV or TARGET_XBOX_DVD.
#endif

//==============================================================================
//  Playstation 2 Targets
//==============================================================================

#ifdef TARGET_PS2_DEV
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PS2
        #define TARGET_DEV
        #define VALID_TARGET
    #endif
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_PS2_CLIENT
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PS2
        #define TARGET_CLIENT
        #define VALID_TARGET
    #endif
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_PS2_DVD
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PS2
        #define TARGET_DVD
        #define VALID_TARGET
    #endif
#endif

#ifdef TARGET_PS2_IOP
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PS2
        #define TARGET_DEV
        #define VALID_TARGET
    #endif
#endif

//==============================================================================
//  GameCube Targets
//==============================================================================

#ifdef TARGET_GCN_DEV
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_GCN
        #define TARGET_DEV
        #define VALID_TARGET
    #endif
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_GCN_DVD
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_GCN
        #define TARGET_DVD
        #define VALID_TARGET
    #endif
#endif

//==============================================================================
//  X-Box Targets
//==============================================================================

#ifdef TARGET_XBOX_DEV
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_XBOX
        #define TARGET_DEV
        #define VALID_TARGET
    #endif
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_XBOX_DVD
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_XBOX
        #define TARGET_DVD
        #define VALID_TARGET
    #endif
#endif

//==============================================================================
//  Unix/Linux Targets
//==============================================================================

#ifdef TARGET_UNIX_TEXT
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_UNIX
        #define VALID_TARGET
    #endif
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_UNIX_OGL
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_UNIX
        #define TARGET_OGL
        #define VALID_TARGET
    #endif
#endif


//==============================================================================
//  PC Targets
//==============================================================================

#ifdef TARGET_PC
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define VALID_TARGET
    #endif
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_PC_MFC
    #ifdef VALID_TARGET
        #define MULTIPLE_TARGETS
    #else
        #define TARGET_PC
        #define TARGET_MFC
        #define VALID_TARGET
    #endif
#endif

//------------------------------------------------------------------------------
// Check for the "target macro is <none>" case when on a PC.  For now, we will
// assume that only the Microsoft compiler is in use on the PC.

#ifndef VALID_TARGET            // If we haven't already got a target,
    #ifdef _MSC_VER             // and we are using the Microsoft compiler...
        #define TARGET_PC
        #define VALID_TARGET
    #endif
#endif

//==============================================================================
//
//  Make sure we found a proper target specification.  If you get a compilation 
//  error here, then your compilation environment is not specifying one of the
//  target macros.
//
//==============================================================================

#ifndef VALID_TARGET
#error Target specification invalid or not found.
#error The compilation environment must define one of the macros listed in x_targets.hpp.
#endif

//==============================================================================
//
//  Make sure we did not somehow get multiple targer platform specifications.
//  *** IF YOU GOT AN ERROR HERE ***, then you have defined more than one of
//  the target specification macros.
//
//==============================================================================

#ifdef MULTIPLE_TARGETS
#error Multiple target specification definition macros were detected.
#error The compilation environment must define only one of the macros listed in x_targets.hpp.
#endif

//==============================================================================
//
//  Endian Designation
//
//  For all configurations, either BIG_ENDIAN or LITTLE_ENDIAN will be defined.
//
//==============================================================================

#ifdef TARGET_PC
#define LITTLE_ENDIAN
#endif

#ifdef TARGET_PS2
#define LITTLE_ENDIAN
#endif

#ifdef TARGET_GCN
#define BIG_ENDIAN
#endif

#ifdef TARGET_XBOX
#define LITTLE_ENDIAN
#endif

#if( !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN) )
    #error Endian is not defined.
#endif

#if(  defined(BIG_ENDIAN) &&  defined(LITTLE_ENDIAN) )
    #error Both Endian specifications are defined!
#endif

//==============================================================================
//
//  Platform specific data structure alignment.
//
//==============================================================================

#if (defined( TARGET_PS2 ) || defined(TARGET_PS2_IOP) ) && defined( VENDOR_SN )
#define PS2_ALIGNMENT(a)   __attribute__( (aligned(a)) )
#else
#define PS2_ALIGNMENT(a)
#endif

//==============================================================================
//  
//  Compatibility with Microsoft's Developer Studio
//
//  DevStudio defines "_DEBUG" for debug build configurations.  If we are on a
//  PC and _DEBUG is defined, go ahead and define appropriate x_files values.
//
//  If TARGET_MFC is defined, then define USE_SYSTEM_NEW_DELETE.
//
//==============================================================================

#if defined( TARGET_PC ) && defined( _DEBUG )

    #ifndef X_ASSERT
    #define X_ASSERT
    #endif

    #ifndef X_DEBUG
    #define X_DEBUG
    #endif

#endif           

#ifdef TARGET_MFC
    #define USE_SYSTEM_NEW_DELETE
#endif

//==============================================================================
#endif // X_TARGET_HPP
//==============================================================================
