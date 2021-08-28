#ifndef __UTIL_HPP__
#define __UTIL_HPP__

//==============================================================================
//  INCLUDES
//==============================================================================
#include "Entropy.hpp"

//==============================================================================
// FUNCTIONS
//==============================================================================

// Returns TRUE if file exists
xbool Util_DoesFileExist( const char* pName, const char* pError = NULL ) ;

// Makes sure there is a "\" at the end of the path
void Util_FixPath( char* P );

//==============================================================================

#endif  //#ifndef __UTIL_HPP__
