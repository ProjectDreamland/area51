//==============================================================================
//
//  Inevitable.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#include "Inevitable.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

static char SubDirectory[ _MAX_PATH ] = "\0";

char* FindSubDirectory( char* pName )
{
    struct _finddata_t Finder;
    int                Handle;
    char*              p;
    bool               Found = false;

    // If we have already found this, then just return the previous results.
    if( strstr( SubDirectory, pName ) )
        return( SubDirectory );

    // Attempt to get the current working directory.
    p = _getcwd( SubDirectory, _MAX_PATH );
    if( !p )
        return( NULL );

    // Look for the desired subdirectory.
    // If not found, back up a level and try again.

    // Point p at the '\0' at the end of the current path.
    p = &SubDirectory[ strlen(SubDirectory) ];

    while( (!Found) && (p >= SubDirectory) )
    {
        // Terminate the string.  (May not be the first iteration!)
        *p = '\0';

        // Append the directory name to the candidate path.
        *p = '\\';
        strcpy( p+1, pName );

        // Start searching...
        if( (Handle = _findfirst( SubDirectory, &Finder )) != -1 )
        {
            // Found something with name in pName.  Is it a directory?
            do
            {
                if( Finder.attrib & _A_SUBDIR )
                    Found = true;
            } while( (!Found) && (_findnext( Handle, &Finder ) == 0) );
        }

        // If not found, back up a directory.
        if( !Found )
        {
            do
            {
                p--;
            } while( (p > SubDirectory) && 
                     (*p != '\\') &&
                     (*p != '/') );
        }
    }

    if( !Found )
        return( NULL );

    return( SubDirectory );
}

//==============================================================================
