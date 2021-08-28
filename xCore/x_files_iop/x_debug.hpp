//==============================================================================
//  
//  x_debug.hpp
//  
//==============================================================================

#ifndef X_DEBUG_HPP
#define X_DEBUG_HPP

#if defined( TARGET_PS2_DVD ) || defined( TARGET_PS2_CLIENT )
//#define X_ASSERT_LITE
#endif

//==============================================================================
//  
//  This file provides basic, cross platform debugging assistance.  The 
//  following groups of functionality are present:
//
//   - Debugger message support.
//   - Compiled breakpoint via BREAK.
//   - Runtime validation via ASSERT and other related macros and functions.
//   - Custom run-time validation failure handling mechanism.
//   - TO DO: Error message and recovery support.
//   - TO DO: Thread safe.
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

//==============================================================================
//  Debugger Message
//==============================================================================
//
//  The function x_DebugMsg works like x_printf except that (a) the output goes
//  to the debugger if possible, and (b) it only works when X_DEBUG is defined.
//
//==============================================================================

#if defined(X_DEBUG)
	void        x_DebugMsg( const char* pFormatStr, ... );
	void        x_DebugLog( const char* pFormatStr, ... );
#else
	inline void x_DebugDummyPrintToMakeDebugMsgDisappear (const char *,...) {}
	#define		x_DebugMsg if (0) x_DebugDummyPrintToMakeDebugMsgDisappear 
	#define		x_DebugLog if (0) x_DebugDummyPrintToMakeDebugMsgDisappear 
#endif
void        x_DebugSetVersionString(const char *pAppVersion);
const char *x_DebugGetVersionString(void);

typedef void x_debug_crash_fn  ( char *pBuffer, s32 Length );

void        x_DebugSetCrashFunction(x_debug_crash_fn *Callback);
x_debug_crash_fn *x_DebugGetCrashFunction(void);


//==============================================================================
//  The BREAK macro.
//==============================================================================
//  
//  The macro BREAK will cause a debugger breakpoint if possible on any given 
//  platform.  If a breakpoint cannot be caused, then a divide by zero will be 
//  forced.  Note that the BREAK macro is highly platform specific.  The 
//  implementation of BREAK on some platforms prevents it from being used 
//  syntactically as an expression.  It can only be used as a statement.
//  
//==============================================================================

#ifdef BREAK
#undef BREAK
#endif

// If we can implement BREAK properly on any given platform, do it!

#ifdef TARGET_PC
#define BREAK      { __asm int 3 }
#endif

#ifdef TARGET_PS2
#define BREAK       asm("break")
#endif

#ifdef TARGET_NGC
#endif

#ifdef TARGET_XBOX
#endif

// Generic BREAK to be used if no proper version can be created.

#ifndef BREAK
extern volatile s32 DDBZ;   // Debug Divide By Zero
#define BREAK  (DDBZ=0,DDBZ=1/DDBZ)
#endif

//==============================================================================
//  Runtime validation support.  
//==============================================================================
//
//  Most of the run-time validations are one form or another of an ASSERT.  So,
//  for lack of a better name, the presence of the compile time macro X_ASSERT 
//  activates the optional run-time validations.  (And, of course, the absence 
//  of X_ASSERT deactivates them.)
//
//  The following macros and functions are all designed to perform validation of
//  expected conditions at run-time.  Each takes an expression as the first 
//  parameter.  The expression (expr) is expected to be true whenever evaluated.
//  
//      ASSERT  ( expr )
//      ASSERTS ( expr, message )
//
//      VERIFY  ( expr )
//      VERIFYS ( expr, message )
//
//      DEMAND  ( expr )
//  
//  The macros ASSERT and ASSERTS completely evaporate in when X_ASSERT is not
//  defined.  Macros VERIFY and VERIFYS, lacking X_ASSERT, still evaluate the 
//  expression, but do not validate the result.  Consider:
//  
//      ASSERT( CriticalInitialization() );    // EVIL without X_ASSERT!
//      VERIFY( CriticalInitialization() );    // Safe without X_ASSERT.
//  
//  The ASSERTS and VERIFYS macros accept a message string to assist problem
//  diagnosis.  Upon a run-time failure, the message is displayed.  For example:
//  
//      ASSERTS( Radius >= 0.0f, "Radius must be non-negative." );
//  
//  To place formatted strings within ASSERTS and VERIFYS, use the xfs class 
//  from x_string.hpp.  For example:
//  
//      pFile = x_fopen( pFileName, "rt" );
//      ASSERTS( pFile, xfs( "Failed to open file '%s'.", pFileName ) );
//  
//  For run-time validation that does NOT evaporate without X_ASSERT, use the
//  DEMAND macro which behaves exactly like ASSERT.
//
//      pFile = x_fopen( pFileName, "rt" );
//      ASSERT( pFile );    // Evaporates without X_ASSERT.
//      DEMAND( pFile );    // Never goes away.
//
//  Available options:
//
//    - As previously mentioned, the macro X_ASSERT enables the validation 
//      macros.  X_ASSERT should be always be present in debug configurations.
//  
//    - The macro X_ASSERT_LITE causes the run-time validation macros to take on
//      an alternate form which sacrifices feedback (the file name and the 
//      expression) for compiled code space.  Since it is anticipated that use 
//      of X_ASSERT_LITE will be rare, there is not likely to be a standard         
//      build configuration which defines X_ASSERT_LITE.  When needed, its 
//      definition should be temporarily forced upon the project.  (One way to 
//      do this would be to simply add #define X_ASSERT_LITE to this file.)
//
//==============================================================================

//------------------------------------------------------------------------------
//  Clear out any pre-existing definitions of the run-time validation macros.
//------------------------------------------------------------------------------

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef ASSERTS
#undef ASSERTS
#endif

#ifdef VERIFY
#undef VERIFY
#endif

#ifdef VERIFYS
#undef VERIFYS
#endif

#ifdef DEMAND
#undef DEMAND
#endif

//------------------------------------------------------------------------------
//  Define the run-time validation macros.
//------------------------------------------------------------------------------

#ifdef X_ASSERT

  #ifdef X_ASSERT_LITE             
    #define ASSERT(expr)        do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, NULL,  NULL ) )  BREAK; } while( FALSE )
    #define ASSERTS(expr,str)   do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, NULL,  NULL ) )  BREAK; } while( FALSE )
    #define VERIFY(expr)        do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, NULL,  NULL ) )  BREAK; } while( FALSE )
    #define VERIFYS(expr,str)   do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, NULL,  NULL ) )  BREAK; } while( FALSE )
  #else                                         
    #define ASSERT(expr)        do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, #expr, NULL ) )  BREAK; } while( FALSE )
    #define ASSERTS(expr,str)   do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, #expr, str  ) )  BREAK; } while( FALSE )
    #define VERIFY(expr)        do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, #expr, NULL ) )  BREAK; } while( FALSE )
    #define VERIFYS(expr,str)   do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, #expr, str  ) )  BREAK; } while( FALSE )
  #endif

#else

  #if defined( TARGET_PC ) && defined( VENDOR_MS )
    #define ASSERT(expr)        (__assume(expr))
    #define ASSERTS(expr,str)   (__assume(expr))
  #else
    #define ASSERT(expr)        ((void)   0  )
    #define ASSERTS(expr,str)   ((void)   0  )
  #endif
    #define VERIFY(expr)        ((void)(expr))
    #define VERIFYS(expr,str)   ((void)(expr))

#endif

#ifdef X_ASSERT_LITE             
    #define DEMAND(expr)        do{ if( !(expr) && RTFHandler( NULL,     __LINE__, NULL,  NULL ) )  BREAK; } while( FALSE )
#else                                         
    #define DEMAND(expr)        do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, #expr, NULL ) )  BREAK; } while( FALSE )
#endif

//==============================================================================
//  Customizable run-time failure (RTF) behavior.
//==============================================================================
//
//  When a run-time validation check fails, a "run-time failure" (or RTF) 
//  handler is called to examine and manage the situation.  It is possible for 
//  the RTF handler to decide to even dismiss the failure and allow the program 
//  execution to continue.  It is the RTF handler's responsibility to report the
//  failure as best as possible.
//
//  Default RTF handlers are provided by the x_files.  On PC's, the default RTF
//  handler uses Microsoft's stuff which is pretty good.  On consoles, the 
//  default version simply prints an error message (via x_printf) and BREAKs.  
//  More sophisticated RTF handlers can be registered.  Such handlers should 
//  probably be provided by the engine in use.
//
//  A custom RTF handler could, for example, bring up an on-screen menu for user
//  response.  Some options could allow the program to essentially ignore the
//  failure and keep running the program.  Other options could attempt to divert
//  program execution into specialized diagnostic modes.
//
//  Functions:
//
//  x_SetRTFHandler - Use the given RTF handler.
//  x_GetRTFHandler - Get a pointer to the current RTF handler.
//
//==============================================================================

//------------------------------------------------------------------------------
//  Define a type for the RTF handler function named "rtf_fn".  For a pointer to
//  such a function, just use "rtf_fn*".
//
//  The return value for RTF handlers is interpretted as follows:
//      TRUE  - Abort!  The failure is fatal!
//      FALSE - Do NOT abort.  The failure is NOT fatal.
//  Think of the return as an "Abort?" flag.
//
//  Any of the string arguments to an RTF handler could potentially be NULL.  
//  All RTF handlers must be prepared for such cases.
//------------------------------------------------------------------------------

typedef xbool rtf_fn( const char* pFileName,
                      s32         LineNumber,
                      const char* pExprString,
                      const char* pMessageString );

typedef void log_fn( const char *pString);

void     x_SetRTFHandler( rtf_fn* pRTFHandler );
rtf_fn*  x_GetRTFHandler( void );
void     x_SetLogHandler( log_fn *pLogHandler);
log_fn*  x_GetLogHandler(void);

rtf_fn   RTFHandler;

//==============================================================================
#endif // X_DEBUG_HPP
//==============================================================================
