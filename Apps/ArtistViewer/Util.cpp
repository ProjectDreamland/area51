//==============================================================================
//  INCLUDES
//==============================================================================
#include "Util.hpp"


//==============================================================================
// UTIL FUNCTIONS
//==============================================================================

// Returns TRUE if file exists
xbool Util_DoesFileExist( const char* pName, const char* pError )
{
    // Try open for reading
    xbool bFound = FALSE;
    X_FILE* pFile = x_fopen(pName, "rb") ;
    if (pFile)
    {
        bFound = (x_flength(pFile) > 0) ;
        x_fclose(pFile) ;
    }

    // Show error if not found?
    if( ( !bFound ) && ( pError ) )
        x_printf("%s \"%s\" does not exist!\n", pError, pName) ;

    return bFound;
}

//==============================================================================

// Makes sure there is a "\" at the end of the path
void Util_FixPath( char* P )
{
    // Get length
    s32 L = x_strlen(P);
    if (!L)
        return;

    // Attach "\" ?
    if (P[L-1] != '\\')
        x_strcat(P, "\\");
}

//==============================================================================
