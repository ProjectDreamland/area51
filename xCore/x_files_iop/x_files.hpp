//==============================================================================
//
//  x_files.hpp
//
//==============================================================================

#ifndef X_FILES_HPP
#define X_FILES_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TARGET_HPP
#include "x_target.hpp"
#endif

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

#ifndef X_STDIO_HPP
#include "x_stdio.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "x_memory.hpp"
#endif

#ifndef X_STRING_HPP
#include "x_string.hpp"
#endif

#ifndef X_ARRAY_HPP
#include "x_array.hpp"
#endif

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_TIME_HPP
#include "x_time.hpp"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
//  Initialization and shut down.
//==============================================================================
//
//  Unlike the standard C library, the x_files must be activated and shut down 
//  explicitly in code.  Furthermore, the x_files must be activated and shut 
//  down in every thread that uses x_files.
//  
//  It is not unreasonable for x_Init and x_Kill to be the first and last lines
//  of the main function.  If an engine is being used, then the engine itself
//  should take care of the activation and shut down of the x_files (at least
//  within the primary thread).
//
//  *** WARNING *** 
//  Avoid creating global objects with non-trivial constructors.  All global 
//  objects are constructed BEFORE main, and therefor, before the x_files are 
//  activated.  Global objects should either be allocated once at run-time, or 
//  given trivial constructors with supporting activation functions.
//  *** WARNING *** 
//  
//  x_Init  - Activate x_files within current thread.
//  x_Kill  - Shut down x_files within current thread.
//
//==============================================================================

// NOTE: These are extern "C" to facilitate their use in CRT0 replacements.
//       DO NOT CHANGE THIS

extern "C" void    x_Init  ( void );
extern "C" void    x_Kill  ( void );

//==============================================================================
#endif // X_FILES_HPP
//==============================================================================
