//==============================================================================
//  
//  x_context.hpp
//  
//==============================================================================

#ifndef X_CONTEXT_HPP
#define X_CONTEXT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

#ifndef X_TIME_HPP
#include "x_time.hpp"
#endif

//==============================================================================
//  SWITCHES
//==============================================================================
/*
#if !defined(X_RETAIL) && !defined(TARGET_PS2_DVD) && !defined(TARGET_GCN_DVD)
#define ENABLE_CONTEXT_TRACKING
#endif
*/
/*
#if defined(rbrannon)
#undef  ENABLE_CONTEXT_TRACKING
#define ENABLE_CONTEXT_TRACKING
#endif
*/

//==============================================================================
//  TYPES
//==============================================================================

//==============================================================================
//  FUNCTIONS
//==============================================================================
//==============================================================================
//  MACROS
//==============================================================================

#if defined(ENABLE_CONTEXT_TRACKING)

    void x_ContextInit                  ( void );                       // Init context tracking
    void x_ContextKill                  ( void );                       // Kill context tracking

    void x_ContextEnableProfiling       ( void );                       // Enable profiling
    void x_ContextDisableProfiling      ( void );                       // Disable profiling
    void x_ContextResetProfile          ( void );                       // Reset profile
    void x_ContextSaveProfile           ( const char* pPathName );      // Save profile
    void x_ContextPrintProfile          ( void );                       // Print profile using x_DebugMsg

    void x_ContextClearSubstringFilter  ( xbool On );
    void x_ContextSubstringFilter       ( const char* pSubstring );

    void x_ContextDisplayStack          ( xbool DumpToScreen = TRUE,    // Display using x_printfxy
                                          xbool DumpToTTY    = FALSE ); // Display using x_DebugMsg

    #define CONTEXT( a )    xcontext __profile__( a )

#else // defined(ENABLE_CONTEXT_TRACKING)

    #if defined(VENDOR_SN) && !defined(_MSC_VER)

        // This is the way to make the calls disappear with GNU
        #define x_ContextInit(...)                  ((void)0)
        #define x_ContextKill(...)                  ((void)0)
        #define x_ContextEnableProfiling(...)       ((void)0)
        #define x_ContextDisableProfiling(...)      ((void)0)
        #define x_ContextResetProfile(...)          ((void)0)
        #define x_ContextSaveProfile(...)           ((void)0)
        #define x_ContextPrintProfile(...)          ((void)0)
        #define x_ContextClearSubstringFilter(...)  ((void)0)
        #define x_ContextSubstringFilter(...)       ((void)0)
        #define x_ContextDisplayStack(...)          ((void)0)

        #define CONTEXT( a )

    #else // defined(VENDOR_SN) && !defined(_MSC_VER)

        // This is the way to make the calls disappear with VC
        inline void x_ContextNULL( ... ) {}
        #define x_ContextInit                   x_ContextNULL
        #define x_ContextKill                   x_ContextNULL
        #define x_ContextEnableProfiling        x_ContextNULL
        #define x_ContextDisableProfiling       x_ContextNULL
        #define x_ContextResetProfile           x_ContextNULL
        #define x_ContextSaveProfile            x_ContextNULL
        #define x_ContextPrintProfile           x_ContextNULL
        #define x_ContextClearSubstringFilter   x_ContextNULL
        #define x_ContextSubstringFilter        x_ContextNULL
        #define x_ContextDisplayStack           x_ContextNULL

        #define CONTEXT( a )

    #endif // defined(VENDOR_SN) && !defined(_MSC_VER)

#endif // defined(ENABLE_CONTEXT_TRACKING)




#if defined(ENABLE_CONTEXT_TRACKING)


//==============================================================================
//  DEBUG STRUCT
//==============================================================================

struct xcontext_debug
{
    xbool   bProfilingEnabled;
    xbool   bDisplayEnabled;
    xbool   bDisplaySummaryEnabled;
    xbool   bDisplayHierarchyEnabled;

    xbool   bShowAllOverrideEnabled;            // TRUE - overrides all filters and shows nodes

    xbool   bShowSubstringFilteredEnabled;      // TRUE - shows tagged substring nodes
    xbool   bHideSubstringFilteredEnabled;      // TRUE - hides tagged substring nodes
    xbool   bSubstringParentFilterEnabled;      // TRUE - hides/shows nodes hidden having parent with substring

    xbool   bMSFilterEnabled;                   // TRUE - culls nodes with (time < MSThreshold)
    xbool   bCallsFilterEnabled;                // TRUE - shows nodes with (call >= CallsThreshold)

    // Fill out SubstringBuffer then set one of the command bools to TRUE
    // The operation will be applied to nodes that contain the substring
    xbool   bCommandSetAllSubstringToOn;
    xbool   bCommandSetAllSubstringToOff;
    xbool   bCommandFilterSubstring;
    char    SubstringBuffer[64];

    // When MSFilter is enabled, contexts with the total time < threshold will be culled
    f32     TotalMSThreshold;

    // Will do an immediate dump if total MS > SpikeMs
    f32     DisplaySpikeMSThreshold;

    // When CallsFilter is enabled, contexts with hits >= threshold will be shown
    s32     TotalCallsThreshold;

    // This is how many calls to PrintProfile there must be before it actually
    // displays
    s32     nFramesBetweenDisplays;

    // Private - should not be touched directly
    xbool   bBuildTree;
};

extern xcontext_debug g_Context;

//==============================================================================
//  xcontext class
//==============================================================================

#define XCONTEXT_FLAG_VISIBLE                   (1<<0)
#define XCONTEXT_FLAG_SUBSTRING_FILTER_DIRECT   (1<<1)
#define XCONTEXT_FLAG_SUBSTRING_FILTER_PARENT   (1<<2)

struct xcontext_node
{
    const char*     pName;
    xcontext_node*  pParent;
    xcontext_node*  pNext;
    xcontext_node*  pChildren;

    // Profile data
    xtick           Ticks;
    s32             Hits;

    // For summary data
    xcontext_node*  pDuplicate;
    xtick           ChildrenTicks;

    // Flags for visibility in display
    u32             Flags;
};

class xcontext
{
public:
                    xcontext        ( const char* pName );
                   ~xcontext        ( );

protected:
    xtick           m_StartTicks;
};

#endif //defined(ENABLED_CONTEXT_TRACKING)

//==============================================================================
#endif // X_CONTEXT_HPP
//==============================================================================
