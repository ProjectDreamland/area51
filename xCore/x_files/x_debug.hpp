//==============================================================================
//  
//  x_debug.hpp
//  
//==============================================================================

#ifndef X_DEBUG_HPP
#define X_DEBUG_HPP

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

typedef void x_debug_msg_fn     ( const char* pBuffer );
typedef void x_debug_crash_fn   ( char *pBuffer, s32 Length );
#ifdef X_RETAIL
    #if defined( TARGET_XBOX ) || defined( TARGET_PC )
        //    The #define approach doesn't work on VC 7.x
        inline void         x_DebugMsg( ... ){ }
        inline void         x_DebugLog( ... ){ }
        inline void         x_DebugMsgSetFunction( x_debug_msg_fn* CallBack ){ }
        inline const char*  x_DebugGetVersionString( void ){ return ""; }
        inline void         x_DebugSetVersionString( ... ){ }
    #else
        // Evaporate functions on GCN / PS2
        inline void         x_DebugNull( ... ){ }
        inline void         x_DebugMsg ( ... ){ }
        #define             x_DebugMsg if(0)x_DebugNull
        #define             x_DebugLog if(0)x_DebugNull
        #define             x_DebugMsgSetFunction if(0)x_DebugNull
        inline const char*  x_DebugGetVersionString( ){ return ""; }
        inline void         x_DebugSetVersionString( ... ){ }
    #endif
#else
void        x_DebugMsg              ( const char* pFormatStr, ... );
void        x_DebugMsg              ( s32 Channel, const char* pFormatStr, ... );
void        x_DebugMsgSetFunction   ( x_debug_msg_fn* CallBack );

void        x_DebugLog( const char* pFormatStr, ... );
void        x_DebugSetVersionString( const char* pAppVersion );
const char* x_DebugGetVersionString( void );
#endif

void                x_DebugSetCrashFunction( x_debug_crash_fn* pCallbackFn );
x_debug_crash_fn*   x_DebugGetCrashFunction( void );


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
#define BREAK       asm("break 1");
#endif

#ifdef TARGET_GCN
#ifdef TARGET_GCN_DEV
    #define BREAK       asm(".long 0");
#else
    extern "C" void VIFlush(void);
    #define BREAK       while(1){VIFlush();};
#endif
#endif

#ifdef TARGET_XBOX
#define BREAK      { __asm int 3 }
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

#ifdef TARGET_XBOX
#pragma warning( disable : 4127 ) // conditional expression is constant
#endif

#ifdef X_ASSERT

  #ifdef X_ASSERT_LITE             
    #define ASSERT(expr)        do{ if( !(expr) && RTFHandler( NULL, __LINE__, NULL,  NULL ) )  BREAK; } while( FALSE )
    #define ASSERTS(expr,str)   do{ if( !(expr) && RTFHandler( NULL, __LINE__, NULL,  NULL ) )  BREAK; } while( FALSE )
    #define VERIFY(expr)        do{ if( !(expr) && RTFHandler( NULL, __LINE__, NULL,  NULL ) )  BREAK; } while( FALSE )
    #define VERIFYS(expr,str)   do{ if( !(expr) && RTFHandler( NULL, __LINE__, NULL,  NULL ) )  BREAK; } while( FALSE )
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

//------------------------------------------------------------------------------

template< class T, int Count >
struct save_array
{
    T Item[ Count ];
    inline T& operator[] ( int Index )
    {
        ASSERT( Index >= 0 );
        ASSERT( Index < Count );
        return Item[ Index ];
    }
};

//------------------------------------------------------------------------------
/*
template< class T >
struct db_parray
{
    T* pItem;

    inline void operator = ( T* pMem )
    {
        pItem = pMem;
        ASSERT( pItem );
    }

    inline T& operator[] ( s32 Index )
    {
        ASSERT( Index >= 0 );
        ASSERT( x_MemPointerValidation( pItem, sizeof(T)*Index ) );
        return pItem[ Index ];
    }
};
*/

//------------------------------------------------------------------------------

#ifdef X_DEBUG
    #define array(  Type, VarName, Count ) save_array < Type, Count > VarName
    #define parray( Type, VarName        ) db_parray< Type >        VarName
#else
    #define array(  Type, VarName, Count ) Type  VarName[ Count ]
    #define parray( Type, VarName        ) Type* VarName
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
//  x_SetRTFMailer  - Use the given RTF mailer.
//  x_GetRTFMailer  - Get a pointer to the current RTF mailer.
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

typedef void rtfmailer_fn( const char* pSubject, const char* pReport );

void            x_SetRTFHandler( rtf_fn* pRTFHandler );
rtf_fn*         x_GetRTFHandler( void );
void            x_SetLogHandler( log_fn *pLogHandler);
log_fn*         x_GetLogHandler(void);
void            x_SetRTFMailer ( rtfmailer_fn* pRTFHandler );
rtfmailer_fn*   x_GetRTFMailer ( void );

rtf_fn   RTFHandler;

//==============================================================================
// DEFINE EXCEPTION HANDLING
//==============================================================================
//
//  x_try           - Enable the exception handling from here down. It also 
//                    creates an scope. Such any variable declare between 
//                    this and a "terminator" will only exits inside that scope.
//
// * After the x_try you must put one of the fallowing "terminators":
//
//  x_catch_display - This terminates the exception handling, and displays
//                    the message to the user. It doesn't throw the
//                    exception anywhere else.
//
//  x_catch_append  - Appends a string and make the exception continue.
//
//  x_catch_begin   - Terminates the exception handling, and begins a block
//                    where the exceptions that are thrown between x_trys
//                    and x_catch_begin can be handle. This requires x_catch_end.
//
//  x_catch_end     - This command comes after the x_catch_begin and it is used
//                    to indicate that the handling block is over. This doesn't
//                    continue the exception. 
//
//  x_catch_end_ret - This command comes after the x_catch_begin and it is used
//                    to indicate that the handling block is over. Then it 
//                    continues the exception up.
//
//  x_append        - This command can only be use inside the x_catch_ begin/end. It is
//                    use to continue the exception going up the stack. But also
//                    allows to add more information about an exception been
//                    thrown. For instance you can put the function name where
//                    the block is so the user can get a call-stack back. EX:
//                    x_append_throw( fs( "CString::malloc( %d )", nBytes) ); or
//                    you can simple use x_append_throw( NULL ); is you have nothing 
//                    to add for the exception.
//
// * You can throw any error any time you want, must you must pay attention
//   where you actually throw them. If you doing between the x_trys and any 
//   of the "terminators" you will be able to handle your own throw, if not it
//   will go up to the next function. 
//
//  x_throw         - This command is the only one that the system understand 
//                    for getting errors. The use is simple: x_throw( Error message );
//                    The error message must be a String. You can of course use the 
//                    fs class to build a more complex message. x_throw should 
//                    not be use inside a x_catch_begin unless your intencion is to 
//                    terminate an ongoing exception and start a new one.
//    
//==============================================================================
// EXAMPLES:
//==============================================================================
// 
//  void Test01( void )
//  {
//      char* pAlloc01 = NULL;  // Keep variables that may need to be 
//      char* pAlloc02 = NULL;  // clean up at the top of the file.
//  
//      
//  x_try;                // From here down we can catch exceptions
//
//      pAlloc01 = new char[1000];
//      if( pAlloc01 == NULL ) 
//          x_throw( "Meaning full message here" );
//               
//      pAlloc02 = new char[1];
//      if( pAlloc02 == NULL ) 
//          x_throw( "Meaning full message here" );
//
//      
//  x_catch_begin;              // Begin handle the errors
//
//      if( pAlloc01 ) delete []pAlloc01;
//      if( pAlloc02 ) delete []pAlloc02;
//
//  x_catch_end_ret;            // End and 
//
//      x_memset( pAlloc01, 0, 1000 );
//  }
//
//==============================================================================
//
//  void Main( void )
//  {
//
//  x_try;                  // We want to catch any errors
//
//      Test01();           // We want to stop app if this ones fails
//      Test01();
//                          // Handle the next one individually
//      x_try;              // Lets catch the error if it throws any
//         Test01();
//      x_catch_begin;      // Begin handle 
//         ReleaseMemory();
//      x_catch_end;        // Must always finish the block
//      
//
//  x_catch_display;        // Nothing to clean up. This is the top level so
//                          // Lets display any error
//
//      x_printf( "We didn't crash!"); 
//  }  
//
//==============================================================================

#ifdef TARGET_PC
    #define E_PARAM int     // Let developer studio handle some of the exceptions
#else
    #define E_PARAM ...     
#endif

#ifdef X_EXCEPTIONS

#ifndef TARGET_PC
#error Exceptions are only allowed for PC Targets!
#endif

#ifdef X_EXCEPTIONS_LITE             

    #define x_try                   try{ ((void)0)
    #define x_catch_display         } catch( E_PARAM ) { static xbool bSkipDialog = FALSE; if( xExceptionCatchHandler( NULL, __LINE__, NULL, bSkipDialog) ){ BREAK }; } ((void)0)
    #define x_catch_display_msg(S)  } catch( E_PARAM ) { static xbool bSkipDialog = FALSE; if( xExceptionCatchHandler( NULL, __LINE__, NULL, bSkipDialog) ){ BREAK }; } ((void)0)
    #define x_display_exception         { static xbool bSkipDialog = FALSE; if( xExceptionCatchHandler( NULL, __LINE__, NULL, bSkipDialog) ){ BREAK }; }
    #define x_display_exception_msg(S)  { static xbool bSkipDialog = FALSE; if( xExceptionCatchHandler( NULL, __LINE__, NULL, bSkipDialog) ){ BREAK }; }
    #define x_catch_begin           } catch( E_PARAM ) { ((void)0)
    #define x_catch_end             } ((void)0)
    #define x_throw(S)              do{ static xbool bSkip = FALSE; if( xExceptionThrowHandler( NULL, __LINE__, NULL, FALSE, bSkip    ) ){ BREAK }; throw(0); } while(0)
    #define x_append_throw(S)       do{ static xbool bSkip = FALSE; if( S ) if( xExceptionThrowHandler( NULL, __LINE__, NULL, TRUE, bSkip ) )}{ BREAK }; throw(0); } while(0)
    #define x_catch_append(S)       x_catch_begin; x_append_throw(S); x_catch_end
    #define x_catch_end_ret         x_append_throw(NULL); x_catch_end   

#else

    #define x_try                   try{ ((void)0)
    #define x_catch_display         } catch( E_PARAM ) { static xbool bSkipDialog = FALSE; if( xExceptionCatchHandler( NULL, __LINE__, NULL, bSkipDialog) ){ BREAK }; } ((void)0)
    #define x_catch_display_msg(S)  } catch( E_PARAM ) { static xbool bSkipDialog = FALSE; if( xExceptionCatchHandler( __FILE__, __LINE__, S, bSkipDialog) ){ BREAK }; } ((void)0)
    #define x_display_exception         { static xbool bSkipDialog = FALSE; if( xExceptionCatchHandler( NULL, __LINE__, NULL, bSkipDialog) ){ BREAK }; }
    #define x_display_exception_msg(S)  { static xbool bSkipDialog = FALSE; if( xExceptionCatchHandler( __FILE__, __LINE__, S, bSkipDialog) ){ BREAK }; }
    #define x_catch_begin           } catch( E_PARAM ) { ((void)0)
    #define x_catch_end             } ((void)0)
    #define x_throw(S)              do{ static xbool bSkip = FALSE; if( xExceptionThrowHandler( __FILE__, __LINE__, S, FALSE, bSkip    ) ){ BREAK }; throw(0); } while(0)
    #define x_append_throw(S)       do{ static xbool bSkip = FALSE; if( S ) if( xExceptionThrowHandler( __FILE__, __LINE__, S, TRUE, bSkip ) ){ BREAK }; throw(0); } while(0)
    #define x_catch_append(S)       x_catch_begin; x_append_throw(S); x_catch_end
    #define x_catch_end_ret         x_append_throw(NULL); x_catch_end   

#endif

#else // X_EXCEPTIONS

    #define x_try                   if( 1 ) {
    #define x_catch_display         }
    #define x_catch_display_msg(S)  }
    #define x_catch_begin           } if( 0 ) {
    #define x_catch_end             }

    #if defined( TARGET_XBOX ) || defined( TARGET_PC )
    inline void x_throw(const char* pString) { (void)pString; ASSERT(0); }
    inline void x_append_throw(const char* pString) { (void)pString; ASSERT(0); }
    #else
    #define x_throw( S ) ASSERT(0)
    #define x_append_throw( S ) ASSERT(0)
    #endif

    #define x_catch_append(S)       x_catch_begin; x_append_throw(S); x_catch_end
    #define x_catch_end_ret         x_append_throw(NULL); x_catch_end

#endif // X_EXCEPTIONS

//==============================================================================

extern  xbool g_bErrorBreak;
xbool   xExceptionThrowHandler( const char* pFileName, s32 LineNum, const char* pMessage, xbool bConcatenate, xbool& bSkip );
xbool   xExceptionThrowHandler( const char* pFileName, s32 LineNum, const char* pMessage, xbool bConcatenate, s32 Code, xbool& bSkip );
xbool   xExceptionCatchHandler( const char* pFileName, s32 LineNum, const char* pMessage, xbool& bSkipDialog );
const char* xExceptionGetErrorString( void );

//==============================================================================
//  Stack Walking helpers
//==============================================================================
//
//  Functions to help walk the stack and dump a trace of the callstack
//  at this point of invokation.
//
//  Functions:
//
//  x_DebugGetCallStack         - Get CallStack as an array of u32 values
//  x_DebugGetCallStackString   - Get CallStack as a \ seperated string or NULL
//
//==============================================================================

xbool       x_DebugGetCallStack         ( s32& CallStackDepth, u32*& pCallStack );
const char* x_DebugGetCallStackString   ( void );

//==============================================================================
#endif // X_DEBUG_HPP
//==============================================================================
