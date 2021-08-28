//==============================================================================
//
//  xCL_Thru.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "xCL.hpp"
#include "xCL_Thru.hpp"
#include "Inevitable.hpp"

//==============================================================================
//  VARIABLES
//==============================================================================

static cmd_line_data&  C = g_CmdLineData;

//==============================================================================
//  FUNCTIONS
//==============================================================================

/* CJ:Incremental

void StripObjectFiles( char* pFileName )
{
    // Open the file and get its size
    FILE* pFile = fopen( pFileName, "rb" );
    fseek( pFile, 0, SEEK_END );
    long Size = ftell( pFile );
    fseek( pFile, 0, SEEK_SET );

    // Allocate space and read the file
    char* pBuffer1 = (char*)malloc( Size + 1 );
    char* pBuffer2 = (char*)malloc( Size + 1 );
    fread( pBuffer1, 1, Size, pFile );
    pBuffer1[Size] = 0;
    fclose( pFile );

    // Setup pointers
    char* p1 = pBuffer1;
    char* p2 = pBuffer2;

    // Copy the file, removing .obj files
    while( *p1 )
    {
        if( *p1 == '"' )
        {
            // Find end of path
            char* pStart = p1;
            p1++;
            while( *p1 && (*p1 != '"' ) )
                p1++;

            if( ( ((p1-(pStart+1)) >= 4) && (strnicmp( p1-4, ".obj", 4 ) == 0) ) ||
                ( ((p1-(pStart+1)) >= 9) && (strnicmp( pStart+1, "\\Projects", 9 ) == 0) ) )
            {
                p1++;
            }
            else
            {
                memcpy( p2, pStart, (p1-pStart) );
                p2 += (p1-pStart );
                *p2++ = *p1++;
            }
        }
        else
        {
            // Just copy it
            *p2++ = *p1++;
        }
    }
    *p2++ = 0;

    // Write out the file
    pFile = fopen( pFileName, "wb" );
    fwrite( pBuffer2, 1, (p2-pBuffer2), pFile );
    fclose( pFile );

    // Write out the file
    pFile = fopen( "c:\\t2.txt", "wb" );
    fwrite( pBuffer2, 1, (p2-pBuffer2), pFile );
    fclose( pFile );

    // Free the memory
    free( pBuffer1 );
    free( pBuffer2 );
}
*/

int PassThru( void )
{
    int     i;
    bool    Compiler = false;
    bool    Linker   = false;
    int     Length   = 0;
    char*   pCmdLn   = NULL;
    char    Buffer[32768] = "PATH=";

    //
    // Remove C:\xBase\xCL from PATH
    //
    {
        // Add the original path.
        const char* pPath = getenv( "PATH" );
        if( pPath )
        {
            if( strnicmp( pPath, "C:\\xBase\\xCL;", 13 ) == 0 )
                pPath += 13;
            strcat( Buffer, pPath );
        }

        // Export the new path.
        _putenv( Buffer );
    }

    // We are going to call the original cl.exe or link.exe.  We need to do a 
    // few things:
    //  (1) We must place quotes around any argument with spaces.
    //  (2) We must define XCORE_PATH on compile command lines.

    // Sum up the size of the original command line.
    for( i = 0; i < C.m_argc; i++ )
    {
        Length += strlen( C.m_argv[i] );
    }

    // Allow enough room for: all possible edits, spaces between args, and 
    // addition of XCORE_PATH macro, and addition of new response file for .obj's.
    Length += (4 * C.m_argc);
    Length += _MAX_PATH + 64;
    Length += _MAX_PATH + 64;

    // Allocate the command line space.
    pCmdLn = (char*)malloc( Length );
    pCmdLn[0] = 0;

    // First, take care of argument 0, which is the cl or link invokation.
    {
        bool  AddQuotes = false;
        char  Whole[ _MAX_PATH   ];
        char  Dir  [ _MAX_DIR    ];
        char  Drive[ _MAX_DRIVE  ];
        char  FName[ _MAX_FNAME  ];
        char  Ext  [ _MAX_EXT    ];
        char* pNewFName = FName;

        // Is there a space anywhere in argument 0?  If so, we must quote it.
        if( strchr( C.m_argv[0], ' ' ) )
            AddQuotes = true;

        // Break argument 0 into its various parts.
        _splitpath( C.m_argv[0], Drive, Dir, FName, Ext );

        // Need to know (for later) if this is a compiler call.
        if( stricmp( FName, "CL" ) == 0 )
        {
            Compiler  = true;
        }

        // Need to know (for later) if this is a linker call.
        if( stricmp( FName, "LINK" ) == 0 )
        {
            Linker  = true;
        }

/* CJ: Don't need this with .net
        // And now for a special case...  
        // Change "DUMPBIN" to "LINK".
        if( stricmp( FName, "DUMPBIN" ) == 0 )
            pNewFName = "DUMPBIN";
*/

        // Put it all back together.
        _makepath( Whole, Drive, Dir, pNewFName, Ext );

        //
        // Put the modified version of argument 0 in the command line.
        //

        if( AddQuotes )
            strcat( pCmdLn, "\"" );

        strcat( pCmdLn, Whole );

        if( AddQuotes )
            strcat( pCmdLn, "\"" );
    }       

/* CJ:Incremental
    //
    // Add new response file for .obj files in entire project to enable
    // Incremental linking.
    //
    if( Linker )
    {
        strcat( pCmdLn, " @\"C:\\Test.txt\"" );
    }
*/
    //
    // Add two macro definitions to the command line:
    //  - XCORE_PATH="$(x)"
    //  - $(username)
    //
    if( Compiler )
    {
        char* pX = getenv( "X" );
        if( pX )
        {
            char Buffer[ _MAX_PATH ];
            sprintf( Buffer, " /D XCORE_PATH=\"%s\"", UseOnlySlashes(pX) );
            strcat( pCmdLn, Buffer );
        }

        char* pUserName = getenv( "USERNAME" );
        if( pUserName && pUserName[0] )
        {
            char Buffer[ _MAX_PATH ];
            sprintf( Buffer, " /D %s", pUserName );
            strcat( pCmdLn, Buffer );
        }
    }

    //
    // For each argument, copy it over to the new command line adding 
    // quotes where needed.
    //      
    for( i = 1; i < C.m_argc; i++ )
    {
/* CJ:Incremental
        if( Linker && (C.m_argv[i][0] == '@') )
            StripObjectFiles( &C.m_argv[i][1] );
*/

        bool AddQuotes = false;
      
        if( strchr( C.m_argv[i], ' ' ) )
            AddQuotes = true;

        strcat( pCmdLn, " " );

        if( AddQuotes )
            strcat( pCmdLn, "\"" );

        strcat( pCmdLn, C.m_argv[i] );

        if( AddQuotes )
            strcat( pCmdLn, "\"" );
    }

/*
    //
    // Extra verbage.  Show the environment variables.
    //
    if( g_Verbose )
    {
        system( "set > \"C:/Environment Variables.txt\"" );
        OpenSection( "Environment Variables" );
        DumpTextFileContent( "C:/Environment Variables.txt" );
        CloseSection();
        system( "del \"C:/Environment Variables.txt\"" );
    }
*/

    // Extra verbage?  Show the xCore directory to be used.
    if( g_Verbose )
    {
        OpenSection( "xCore Directory In Use" );
        printf( "%s\n", getenv( "X" ) );
        CloseSection();
    }

    // Extra verbage?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "Pass Thru",
                                        pCmdLn,
                                        NULL );
    }

    //
    // Now that we have our command line, invoke it!
    //
    {
        int Result;
        Result = system( pCmdLn );
        return( Result );    
    }
}

//==============================================================================
