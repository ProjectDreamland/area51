//==============================================================================
//
//  xCL_CodeWarrior.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <windows.h>

#include "xCL.hpp"
#include "xCL_CodeWarrior.hpp"
#include "Inevitable.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

#define PC  1
#define PS2 2

//==============================================================================
//  VARIABLES
//==============================================================================

static cmd_line_data&   C = g_CmdLineData;

static token_list       ObjLib;
static token_list       Section;

static char*            pBase;

static int              Which    = 0;
static char*            pLabel[] = { "<ERROR>", "PC", "PS2" };

//==============================================================================
//  FUNCTIONS
//==============================================================================

static 
bool PrepareEnvironment( void )
{
    char  Buffer[ 32784 ];
    char* pPath = NULL;

    //
    // Extra verbage.  Show the current directory.
    // 

    if( g_Verbose )
    {
        OpenSection( "Current Working Directory" );
        if( _getcwd( Buffer, 32784 ) == NULL )
            strcpy( Buffer, "<<< ERROR: Could not read current working directory. >>>" );
        else
            printf( "%s\n", Buffer );
        CloseSection();
    }

    //
    // We need to find the "xCore" directory we are going to use.  The xTB 
    // defines an environment variable "X" with the correct path.  Set its
    // path into pBase.
    //

    pBase = getenv( "X" );

    if( !pBase )
    {
        printf( "xCL -- ERROR: Unable to locate xCore directory to use.\n" );
        return( false );
    }

    //
    // Extra verbage.  Show the xCore directory to be used.
    // 

    if( g_Verbose )
    {
        OpenSection( "xCore Directory In Use" );
        printf( "%s\n", pBase );
        CloseSection();
    }

    // We need to prepend some directories to the path.  Even though the
    // user may have manually added all desired and needed paths, sometimes
    // DevStudio ends up with shorted paths...

    //
    // Add the CodeWarrior tools directory to the path.
    //       
    {
        if( Which == PC )
        {
            // Add the PC CodeWarrior path.
            sprintf( Buffer, "PATH=%s\\3rdParty\\PC\\Metrowerks\\Other Metrowerks Tools\\Command Line Tools;"
                                  "%s\\3rdParty\\PC\\Metrowerks\\bin;", 
                             pBase, pBase );
        }

        if( Which == PS2 )
        {
            // Add the PS2 CodeWarrior path.
            sprintf( Buffer, "PATH=%s\\3rdParty\\PS2\\Metrowerks\\PS2_Tools\\Command_Line_Tools;"
                                  "%s\\3rdParty\\PS2\\Metrowerks\\bin\\dvpasm;", 
                             pBase, pBase );
        }
         
        // Add the original path.
        pPath = getenv( "PATH" );
        if( pPath )
            strcat( Buffer, pPath );

        // Export the new path.
        _putenv( Buffer );
    }

    //
    // Extra verbage.  Show the search path.
    // 

    if( g_Verbose )
    {
        char* p = Buffer;
        char* q = Buffer;

        OpenSection( "Path for CodeWarrior Operation" );
        strcpy( Buffer, getenv( "PATH" ) );
        while( *p )
        {   
            if( *p == ';' )
            {
                *p = '\0';
                printf( "%s\n", q );
                q = p+1;
            }
            p++;
        }
        if( *q )
            printf( "%s\n", q );
        CloseSection();
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

    return( true );
}

//==============================================================================
//  The output from the compiler (errors and warnings and other) was sent to a 
//  file.  This function will parse through that file attempting to pick out
//  the errors and warnings.  When it finds something, it reformats the message
//  and prints it to standard out.  The new format is one that Developer Studio
//  will be able to recognize.  All text which is NOT recognized is just printed
//  out as is for the user in case it is important.

/* 
** The format for errors is:
**

   Error : <MESSAGE>
<FILENAME.EXT> line <line_number>  <offending_code>

**
** The format for warnings is the same except for the first word:
**

 Warning : <MESSAGE>
<FILENAME.EXT> line <line_number>  <offending_code>

**
** There is a tab ('\t') between the <line_number> and the <offending_code>.
** Sometimes the <MESSAGE> can span multiple lines:
**

   Error : <MESSAGE>
           <MORE_MESSAGE>
<FILENAME.EXT> line <line_number>  <offending_code>

**
** Samples:
**

   Error : undefined identifier 'rebar'
Test.cpp line 5      rebar  r;
   Error : undefined identifier 'foobar'
Test.cpp line 8      foobar f;
 Warning : variable / argument 'a' is not used in function
Test.cpp line 6      float a;

**
** Note that the PS2 compiler seems to inject a blank line.  Like so:
**

   Error : undefined identifier 'rebar'

Test.cpp line 5      rebar  r;
   Error : undefined identifier 'foobar'

Test.cpp line 8      foobar f;
 Warning : variable / argument 'a' is not used in function

Test.cpp line 6      float a;

**
*/


static
void ProcessMessages( char* pFileName )
{
    FILE* pFile;
    char* pCursor;   
    char  OriginalText    [ 8192 ];
    char  CurrentDirectory[ 8192 ];
    char  Message         [ 8192 ];
    char  SourceFile      [ 8192 ];
    char  Path            [ 8192 ];
    char  LineNumber      [   16 ];
    char* pLine2;
    
    // Open the error message file.
    pFile = fopen( pFileName, "rt" );
    if( !pFile )
    {
        printf( "xCL -- ERROR: Unable to open file for read: %s\n", pFileName );
        return;
    }   

    // Get the current directory.
    if( _getcwd( CurrentDirectory, 8192 ) == NULL )
        printf( "xCL -- WARNING: Unable to read current directory.\n" );

    // Loop until we have processed everything in the file.
    while( !feof( pFile ) )
    {
        bool  Error   = false;
        bool  Warning = false;
        char* p;
        char* q;
        int   l;

        // Read in a line from the file.  Position cursor to end of buffer, 
        // ready for more reading.
        p       = fgets( OriginalText, 8192, pFile );
        pCursor = OriginalText + strlen( OriginalText );

        // If we didn't get anything, then just continue to the next iteration,
        if( !p )
            continue;

        // Got an error message?
        if( strncmp( OriginalText, "   Error : ", 11 ) == 0 )
            Error = true;

        // Got a warning message?
        if( strncmp( OriginalText, " Warning : ", 11 ) == 0 )
            Warning = true;

        // If we have NEITHER an error NOR a warning...
        if( !(Error || Warning ) )
        {
            // Print out everything we have in our OriginalText buffer.
            printf( "%s", OriginalText );
            continue;
        }

        //
        // At this point, we seem to have either an error OR a warning.
        //

        // Copy the message portion.
        strcpy( Message, OriginalText+11 );

        // We have to handle any additional lines containing MESSAGE, or any 
        // blank lines from the PS2 tools.
        //
        // When we are done, we want "pLine2" pointing at the text (typically
        // the second line in the original text) where the line containing the
        // filename and line number are found.

        // As much as I hate it, the best way to implement this section is with
        // a damned goto.  So here is the label.
        GET_ANOTHER_LINE:

        // Nick off any trailing '\n' from the Message we are building.
        l = strlen( Message );
        if( Message[l-1] == '\n' )  
            Message[l-1] = '\0';

        // Get the next line of text from the file.
        pLine2  = pCursor;
        p       = fgets( pCursor, (pCursor - OriginalText + 8192), pFile );
        pCursor = OriginalText + strlen( OriginalText );

        // Did we get anything?
        if( !p )
        {
            // We didn't get any more text.  (Possibly EOF.)
            // Print out everything in the OriginalText buffer.
            printf( "%s", OriginalText );
            continue;
        }

        // Do we have a multi-line message?
        if( strncmp( pLine2, "           ", 11 ) == 0 )
        {
            // Add to the Message we are building, and get another line of text.
            strcat( Message, "  " );
            strcat( Message, pLine2+11 );
            goto GET_ANOTHER_LINE;
        }

        // Or did we get a blank line from a PS2 tool?
        if( pLine2[0] == '\n' )
            goto GET_ANOTHER_LINE;

        //
        // At this point, pLine2 should point to the text which is typically
        // on the second line of the original message.  This line should contain
        // the file name and line number.
        //

        // Search for " line ".
        p = strstr( pLine2, " line " );

        // If we did NOT find " line ".
        if( !p )
        {
            // There is an error or warning, but we couldn't parse out a file 
            // and line number.  Print out the text in our OriginalText buffer
            // because it may be important.
            printf( "%s", OriginalText );
            continue;
        }

        // At this point, we have an error or warning, and we have
        // found " line " in the message.  The file name we want is
        // between pLine2 and p.  Lift it out.
        strncpy( SourceFile, pLine2, (p-pLine2) );
        SourceFile[ p-pLine2 ] = '\0';

        // Advance past " line ".
        p += 6;

        // The line number is now at p.  We need to lift it out into our buffer
        // called LineNumber.
        q = LineNumber;
        while( !isspace(*p) )
        {
            *q = *p;
            p++;
            q++;
        }
        *q = '\0';

        //
        // We now have all of the parts of the original message to produce a
        // message suitable to our needs.
        //

        // Ah!  But, the filename may not include a path.  If it does not, then
        // we must provide one.  We will use the current directory.
        _splitpath( SourceFile, NULL, Path, NULL, NULL );
        if( Path[0] == '\0' )
        {
            // There is no path.  Use the current directory.
            printf( "%s\\", CurrentDirectory );
        }

        // Print our message.
        printf( "%s(%s) : %s : %s\n", 
                SourceFile, 
                LineNumber,
                (Error ? "Error" : "Warning"),
                Message );

        fflush( stdout );
    }

    fclose( pFile );
}

//==============================================================================

static int CompileSourceCode( void )
{
    int     Result = EXIT_SUCCESS;
    int     i;

    char    OutFile [ _MAX_PATH  ]; // CodeWarrior compiler output
    char    Message [ 32784 ];      // Message to show to user

    char    NewCmdLn[ 32784 ];      // New command line to execute
    char    VC6CmdLn[ 32784 ];      // VC6 command line to execute

    char    NewRName[ _MAX_PATH  ]; // New response file name
    char    VC6RName[ _MAX_PATH  ]; // VC6 response file name

    FILE*   pNewRFile;              // New response file pointer
    FILE*   pVC6RFile;              // VC6 response file pointer

    //--------------------------------------------------------------------------
    // Setup local and temporary variables.
    //--------------------------------------------------------------------------

    // Start with an empty message.  Build it as we go.
    strcpy( Message, "xCL --" );

    //--------------------------------------------------------------------------
    // Build the CodeWarrior response file.
    //--------------------------------------------------------------------------

    // Determine response file name.
    sprintf( NewRName, "%s\\_CW_ResponseFile.txt", C.m_OutputPath );

    // Open the response file.
    pNewRFile = fopen( NewRName, "wt" );
    if( pNewRFile == NULL )
    {
        printf( "xCL -- ERROR: Unable to open CodeWarrior response file: %s\n", NewRName );
        return( EXIT_FAILURE );
    }   

    // Add some standard compile options.    
    fprintf( pNewRFile, " -c           \n" );       // Compile only; don't link
    fprintf( pNewRFile, " -nodefaults  \n" );       // No default libs/includes/etc
    fprintf( pNewRFile, " -nowraplines \n" );       // Don't wrap output message lines
    fprintf( pNewRFile, " -nostderr    \n" );       // Don't use stderr for messages
    fprintf( pNewRFile, " -msgstyle IDE\n" );       // Use "IDE" message style
    fprintf( pNewRFile, " -enum int    \n" );       // Enums are integer sized (32 bit)
    fprintf( pNewRFile, " -sym on      \n" );       // Turn on debugging information
    fprintf( pNewRFile, " -Cpp_exceptions off\n" ); // Turn off exceptions

    // Add PC specific standard compile options.
    if( Which == PC )
    {
        fprintf( pNewRFile, " -proc 586\n" );
        fprintf( pNewRFile, " -align 4 \n" );
    }

    // Add PS2 specific standard compile options.
    if( Which == PS2 )
    {
        // Specify prefix file.
        fprintf( pNewRFile, " -prefix \"%s\\3rdParty\\PS2\\Metrowerks\\PS2 Support\\PREFIX_PS2_DEBUG.h\"\n",
                            pBase );
    }

    // Debug mode?
    if( C.m_DebugMode )    fprintf( pNewRFile, " -inline off\n" );
    else                   fprintf( pNewRFile, " -inline on \n" );

    // Add compile optimization specification.
    {
        char* List[] = { "off", "level=2", "level=2", "full" };
        char* pOpt;

        // Args from Microsoft:         Map to CodeWarrior:
        //  -Od = "debug"                "off"
        //  -O1 = "minimize size"        "level=2"
        //  -Ot = "default"              "level=2"
        //  -O2 = "maximize speed"       "full"

        switch( C.m_Optimization )
        {
            case 'd':   pOpt = List[0];  break;
            case '1':   pOpt = List[1];  break;
            case 't':   pOpt = List[2];  break;
            case '2':   pOpt = List[3];  break;
            default :   pOpt = List[2];  break;
        }

        // Add to message string.
        strcat( Message, " Optimize[" );
        strcat( Message, pOpt );
        strcat( Message, "]" );

        // Add to response file.
        fprintf( pNewRFile, " -opt %s\n", pOpt );
    }

    // Precompiled header.
    //if( C.m_PCHFile )
    //    fprintf( pNewRFile, " -prefix \"%s\"\n", C.m_PCHFile );

    // Add current directory as an include directory.
    fprintf( pNewRFile, " -i-\n" );
    fprintf( pNewRFile, " -i  .\n" );

    // Add the include paths.
    for( i = 0; i < C.m_IncludeDir.GetCount(); i++ )
        fprintf( pNewRFile, " -i  \"%s\" \n", C.m_IncludeDir[i] );

    // Add the Sony system includes.
    if( Which == PS2 )
    {
        char Path[] = "3rdParty\\PS2\\Sony\\sce\\ee";

        fprintf( pNewRFile, " -i \"%s\\%s\\lib\"                                    \n", pBase, Path );
        fprintf( pNewRFile, " -i \"%s\\%s\\include\"                                \n", pBase, Path );
        fprintf( pNewRFile, " -i \"%s\\%s\\gcc\\ee\\lib\"                           \n", pBase, Path );
        fprintf( pNewRFile, " -i \"%s\\%s\\gcc\\ee\\include\"                       \n", pBase, Path );
        fprintf( pNewRFile, " -i \"%s\\%s\\gcc\\lib\\gcc-lib\\ee\\2.95.2\"          \n", pBase, Path );
        fprintf( pNewRFile, " -i \"%s\\%s\\gcc\\lib\\gcc-lib\\ee\\2.95.2\\include\" \n", pBase, Path );
    }

    // Add the PS2 CodeWarrior system includes.
    if( Which == PS2 )
    {
        char Path[] = "3rdParty\\PS2\\Metrowerks\\PS2 Support";

        fprintf( pNewRFile, " -i \"%s\\%s\"                                    \n", pBase, Path );
        fprintf( pNewRFile, " -i \"%s\\%s\\Msl\\MSL_C\\MSL_MIPS\\Include\"     \n", pBase, Path );
        fprintf( pNewRFile, " -i \"%s\\%s\\Msl\\MSL_C\\MSL_Common\\Include\"   \n", pBase, Path );
        fprintf( pNewRFile, " -i \"%s\\%s\\Msl\\MSL_C++\\MSL_Common\\Include\" \n", pBase, Path );
    }

    // Add the PC CodeWarrior system includes.
    if( Which == PC )
    {
        char Path[] = "3rdParty\\PC\\Metrowerks";

        fprintf( pNewRFile, " -ir \"%s\\%s\\MSL\\MSL_C\\MSL_Common\\Include\"   \n", pBase, Path );
        fprintf( pNewRFile, " -i  \"%s\\%s\\MSL\\MSL_C\\MSL_Win32\\Include\"    \n", pBase, Path );
        fprintf( pNewRFile, " -ir \"%s\\%s\\MSL\\MSL_C\\MSL_X86\"               \n", pBase, Path );
        fprintf( pNewRFile, " -i  \"%s\\%s\\MSL\\MSL_C++\\MSL_Common\\Include\" \n", pBase, Path );
        fprintf( pNewRFile, " -ir \"%s\\%s\\Win32-x86 Support\\Headers\"        \n", pBase, Path );
    }

    // Add the defines to the response file.
    for( i = 0; i < C.m_Define.GetCount(); i++ )
        fprintf( pNewRFile, " -D%s \n", C.m_Define[i] );

    // Add the XCORE_PATH macro.
    fprintf( pNewRFile, " -DXCORE_PATH=\\\"%s\\\"", UseOnlySlashes(pBase) );

    // Add the defines to the message for the user.
    strcat( Message, "\nxCL -- Defines[ " );
    for( i = 0; i < C.m_Define.GetCount(); i++ )
    {
        if( i != 0 )  strcat( Message, ", " );
        strcat( Message, C.m_Define[i] );
    }
    strcat( Message, " ]" );

    // That's all for the new response file.
    fclose( pNewRFile );

    // We built a helpful message string for the user as we went along.
    // Might as well go ahead and display it now.
    printf( "%s\n.\n", Message );
    fflush( stdout );

    //--------------------------------------------------------------------------
    // Build the VC6 response file.
    //--------------------------------------------------------------------------

    // Determine response file name.
    sprintf( VC6RName, "%s\\_VC6_ResponseFile.txt", C.m_OutputPath );

    // Open the response file.
    pVC6RFile = fopen( VC6RName, "wt" );
    if( pVC6RFile == NULL )
    {
        printf( "xCL -- ERROR: Unable to open VC6 response file: %s\n", VC6RName );
        return( EXIT_FAILURE );
    }

    // Add some standard compile options.
    fprintf( pVC6RFile, " /c /nologo /W3 /FD\n" );
    
    // Just pre-process.  Good enough for dependency information!
    fprintf( pVC6RFile, " /P\n" );

    // Miscellaneous options.
    if( C.m_SBRPath[0] )      fprintf( pVC6RFile, " /FR\"%s\"\n", C.m_SBRPath );
    if( C.m_PDBPath[0] )      fprintf( pVC6RFile, " /Fd\"%s\"\n", C.m_PDBPath );
    if( C.m_Preprocess )      fprintf( pVC6RFile, " /C\n" );

    // In order to get the code to preprocess in VC, a few macros and such
    // must be defined.  Macros with arguments cannot be defined on the 
    // command line.  So, we will force all source files to include a tiny 
    // header file with the macros.    
    {
        unsigned long Written;
        HANDLE        Handle;
        FILETIME      FileTime;
        char          Name[ _MAX_PATH ];
        char          Buffer[] = "#define __IEEE_LITTLE_ENDIAN\r\n"
                                 "#define __option(a) 1\r\n";     
                                 
        // Make the name.
        sprintf( Name, "%s/_CW_in_VC.h", C.m_OutputPath );

        // Create the file.
        Handle = CreateFile( Name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if( Handle == INVALID_HANDLE_VALUE )
        {
            printf( "xCL -- ERROR: Unable to open file for write: %s\n", Name );
            return( EXIT_FAILURE );
        }
        
        // Write desired content.
        WriteFile( Handle, Buffer, strlen(Buffer), &Written, NULL );

        // Set the time back to avoid triggering dependencies.
        GetFileTime( Handle, &FileTime, &FileTime, &FileTime );
        FileTime.dwHighDateTime = 28000000;
        SetFileTime( Handle, &FileTime, &FileTime, &FileTime );

        // That's it!
        CloseHandle( Handle ); 

        // Force the include.
        fprintf( pVC6RFile, " /FI \"%s\"\n", Name );
    }

    // Add the include paths.
    for( i = 0; i < C.m_IncludeDir.GetCount(); i++ )
        fprintf( pVC6RFile, " /I \"%s\" \n", C.m_IncludeDir[i] );

    // Add the Sony system includes.
    if( Which == PS2 )
    {
        char Path[] = "3rdParty\\PS2\\Sony\\sce\\ee";

        fprintf( pNewRFile, " /I \"%s\\%s\\lib\"                                    \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\include\"                                \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\gcc\\ee\\lib\"                           \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\gcc\\ee\\include\"                       \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\gcc\\lib\\gcc-lib\\ee\\2.95.2\"          \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\gcc\\lib\\gcc-lib\\ee\\2.95.2\\include\" \n", pBase, Path );
    }

    // Add the PC CodeWarrior system includes.
    if( Which == PC )
    {
        char Path[] = "3rdParty\\PC\\Metrowerks";

        fprintf( pNewRFile, " /I \"%s\\%s\\MSL\\MSL_C\\MSL_X86\\AMD_K63D_NOW\"     \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\MSL\\MSL_C\\MSL_Win32\\Include\"        \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\MSL\\MSL_C\\MSL_Common\\Include\"       \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\MSL\\MSL_C++\\MSL_Common\\Include\"     \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\Win32-x86 Support\\Headers\\Win32 SDK\" \n", pBase, Path );
    }

    // Add the PS2 CodeWarrior system includes.
    if( Which == PS2 )
    {
        char Path[] = "3rdParty\\PS2\\Metrowerks";

        fprintf( pNewRFile, " /I \"%s\\%s\\PS2 Support\\Msl\\MSL_C\\MSL_MIPS\\Include\"     \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\PS2 Support\\Msl\\MSL_C\\MSL_Common\\Include\"   \n", pBase, Path );
        fprintf( pNewRFile, " /I \"%s\\%s\\PS2 Support\\Msl\\MSL_C++\\MSL_Common\\Include\" \n", pBase, Path );
    }

    // We need an extra include path to find the "forced include" header file.
    //
    // NOTE:  The forced include is something like "/FI Debug\_File.h".  This
    // only has meaning from within the current directory.  However, the file
    // we are compiling may be in a different directory.  Includes check the
    // directory with the source file, then all paths added on the command line.
    // If the source file is in another directory, using "/I Debug" won't work.
    // Add the current directory to the path (/I .), and this will resolve to
    // ".\Debug\_File.h" regardless of the location of the source file since the
    // current directory does not change.
    //
    // Using "/FI _File.h" and "/I Debug" has a flaw, too.  But I can't remember
    // what it is right now.

    fprintf( pVC6RFile, " /I . \n" );

    // Add the defines to the response file.
    for( i = 0; i < C.m_Define.GetCount(); i++ )
        fprintf( pVC6RFile, " /D %s \n", C.m_Define[i] );

    // Add the XCORE_PATH macro.
    fprintf( pVC6RFile, " /D XCORE_PATH=\\\"%s\\\"", UseOnlySlashes(pBase) );

    //
    // The CodeWarrior compiler automatically provides some definitions.
    //

    if( Which == PC )
    {
        fprintf( pVC6RFile, " /D __MWERKS__\n" );
        fprintf( pVC6RFile, " /D __INTEL__ \n" );
    }

    if( Which == PS2 )
    {
        fprintf( pVC6RFile, " /D __MWERKS__   \n" );
        fprintf( pVC6RFile, " /D __MIPS__     \n" );
        fprintf( pVC6RFile, " /D __MIPS_PSX2__\n" );

        // We have to use Microsoft's preprocessor to get our dependency 
        // information.  Their preprocessor introduces some extra defines.  
        // And this can mess things up sometimes.  So, UNDEFINE them!
        fprintf( pVC6RFile, " /U _WIN32\n" );
    }

    // That's it.
    fclose( pVC6RFile );

    //--------------------------------------------------------------------------
    // Execute!
    //--------------------------------------------------------------------------

    // We redirect the CW output to a file and then parse it later.
    // Create the name for this file.
    sprintf( OutFile, "%s\\_CompilerOutput.txt", C.m_OutputPath );

    //
    // For every file we need to compile, we must run it through VC6 
    // (preprocess only) and then through CodeWarrior.
    //

    for( i = 0; i < C.m_SourceCode.GetCount(); i++ )
    {   
        char SrcFName[ _MAX_FNAME ];

        _splitpath( C.m_SourceCode[i], NULL, NULL, SrcFName, NULL );

        //-------//
        //  VC6  //
        //-------//
        {
            sprintf( VC6CmdLn, "_cl @\"%s\" \"%s\"", VC6RName, C.m_SourceCode[i] );

            // Extra verbage?
            if( g_Verbose || g_EchoCmdLine )
                DumpCommandLineAndResponseFile( "VC6 Compile",
                                                VC6CmdLn,
                                                VC6RName );

            // Execute command for VC6 to compute dependencies.
            system( VC6CmdLn );

            // Delete .i file from the VC preprocessor.
            if( !C.m_Preprocess )
            {
                sprintf( VC6CmdLn, "del %s.i > nul", SrcFName );
                system ( VC6CmdLn );
            }
        }

        //---------------//
        //  CodeWarrior  //
        //---------------//
        {
            char* pCompiler[] = { "<ERROR>", "mwcc.exe", "mwccps2.exe" };

            sprintf( NewCmdLn, "%s @\"%s\" \"%s\" -o \"%s\\%s.obj\" > \"%s\"", 
                                pCompiler[Which],
                                NewRName, 
                                C.m_SourceCode[i], 
                                C.m_OutputPath,
                                SrcFName,
                                OutFile );

            // Extra verbage?
            if( g_Verbose || g_EchoCmdLine )
                DumpCommandLineAndResponseFile( "CodeWarrior Compile",
                                                NewCmdLn, 
                                                NewRName );

            // Execute command for CodeWarrior compiler.
            Result |= system( NewCmdLn );

            //------------------------------------------
            // Now take care of the errors and warnings.
            //------------------------------------------
            ProcessMessages( OutFile );
        }
    }

    //--------------------------------------------------------------------------
    //  We're done!
    //--------------------------------------------------------------------------
    return( Result );
}

//==============================================================================

static
int Compile_s_File( int i )
{
    char    FName   [ _MAX_FNAME ];
    char    Ext     [ _MAX_EXT   ];
    char    CmdLine [ 32784 ];
    int     Result = EXIT_SUCCESS;

    // Pick the name apart.
    _splitpath( C.m_SourceOther[i], NULL, NULL, FName, Ext );

    // Build the command line.
    sprintf( CmdLine, "asm_r5900_elf -gnu -o \"%s\\%s.obj\" \"%s\"", 
                      C.m_OutputPath,
                      FName,   
                      C.m_SourceOther[i] );

    // Verbose?
    if( g_Verbose || g_EchoCmdLine )
    {   
	    DumpCommandLineAndResponseFile( "PS2/CW Assemble",
                                        CmdLine,
                                        NULL );
    }

    // Feedback for the user.
    printf( "%s%s\n", FName, Ext );
    fflush( stdout );

    // Execute command for CW assembler.
    Result = system( CmdLine );
    return( Result );
}

//==============================================================================

static
int Compile_vu_File( int i )
{
    char    Drive   [ _MAX_DRIVE ];
    char    Dir     [ _MAX_DIR   ];
    char    FName   [ _MAX_FNAME ];
    char    Ext     [ _MAX_EXT   ];
    char    CmdLine [ 32784 ];
    char    Buffer  [ 32784 ];
    int     Result = EXIT_SUCCESS;
    int     j;

    //
    // When we proces .vu files, we will first send them through the Visual 
    // Studio C preprocessor.  Then we send the resulting file through the 
    // standard VU assembler.
    // 

    _splitpath( C.m_SourceOther[i], Drive, Dir, FName, Ext );

    //
    // First pass:  C preprocess.
    //
    {
        // Start building the command line.
        // Add standard options:
        //   /TC = Ignore input file extension.  Treat it like C code.
        //   /P  = Write preprocessor output to a file.
        // Add current directory as an include directory.
        sprintf( CmdLine, "_cl /nologo /TC /P -I ." );

        // Add the include paths.
        for( j = 0; j < C.m_IncludeDir.GetCount(); j++ )
        {
            sprintf( Buffer, " -I \"%s\"", C.m_IncludeDir[j] );
            strcat ( CmdLine, Buffer );
        }

        // Add the file name.
        sprintf( Buffer, " \"%s\"", C.m_SourceOther[i] );
        strcat ( CmdLine, Buffer );

        // Verbose?
        if( g_Verbose || g_EchoCmdLine )
        {
	        DumpCommandLineAndResponseFile( "PS2/CW VU Preprocess",
                                            CmdLine,
                                            NULL );
        }

        // Execute command for preprocess.
        Result = system( CmdLine );
    }

    // Move the .i file to the build output directory.
    sprintf( CmdLine, "move \"%s%s%s.i\" \"%s\" ",
                      Drive, Dir, FName,
                      C.m_OutputPath );

    // Do the move.
    Result = system( CmdLine );

    //
    // Second pass:  VU assemble.
    //
    {
        // Start building the command line.
        // Add standard options:
        // Add current directory as an include directory.
        sprintf( CmdLine, "ee-dvp-as.exe -g -I." );

        // Add the include paths.
        for( j = 0; j < C.m_IncludeDir.GetCount(); j++ )
        {
            sprintf( Buffer, " -I\"%s\"", C.m_IncludeDir[j] );
            strcat ( CmdLine, Buffer );
        }

        // Add the file names and redirection.
        sprintf( Buffer, " -o \"%s/%s.obj\" \"%s/%s.i\" > \"%s/%s.lst\" ", 
                          C.m_OutputPath, FName,
                          C.m_OutputPath, FName,
                          C.m_OutputPath, FName );
        strcat ( CmdLine, Buffer );

        // Verbose?
        if( g_Verbose || g_EchoCmdLine )
        {
	        DumpCommandLineAndResponseFile( "PS2/CW VU Assemble",
                                            CmdLine,
                                            NULL );
        }

        // Execute command for CW/VU assembler.
        Result = system( CmdLine );
    }

    // That's it.
    return( Result );
}

//==============================================================================
/*
static
int Compile_vu_File( int i )
{
    char    Name    [ _MAX_PATH  ];
    char    Drive   [ _MAX_DRIVE ];
    char    Dir     [ _MAX_DIR   ];
    char    FName   [ _MAX_FNAME ];
    char    Ext     [ _MAX_EXT   ];
    char    CmdLine [ 32784 ];
    int     Result = EXIT_SUCCESS;
    int     j;
    FILE*   pFile;

    //
    // When we proces .vu files, we will first send them through the Visual 
    // Studio C preprocessor.  Then we send the resulting file through the 
    // standard VU assembler.
    // 

    // Pick the name apart.
    _splitpath( C.m_SourceOther[i], Drive, Dir, FName, Ext );

    //
    // First pass:  C preprocess.
    //
    {
        // Create the response file.
        sprintf( Name, "%s\\_CW_VU_CPP_ResponseFile.txt", C.m_OutputPath );

        // Open the response file.
        pFile = fopen( Name, "wt" );
        if( pFile == NULL )
        {
            printf( "xCL -- ERROR: Unable to open CodeWarrior response file: %s\n", Name );
            return( EXIT_FAILURE );
        }   

        // Add standard command line options.
        //   /TC = Ignore input file extension.  Treat it like C code.
        //   /P  = Write preprocessor output to a file.
        fprintf( pFile, " /nologo /TC /P \n" );

        // Add current directory as an include directory.
        fprintf( pFile, " -I .\n" );

        // Add the include paths.
        for( j = 0; j < C.m_IncludeDir.GetCount(); j++ )
            fprintf( pFile, " -I \"%s\"\n", C.m_IncludeDir[j] );

        // Add the PS2 system includes.
        fprintf( pFile, " -I \"%s/3rdParty/PS2/Sony/sce/ee/gcc/ee/include\" \n", pBase );

        // Done writing file.
        fclose( pFile );

        // Build the command line.
        sprintf( CmdLine, "_cl @\"%s\" \"%s\"", 
                          Name, 
                          C.m_SourceOther[i] );

        // Verbose?
        if( g_Verbose || g_EchoCmdLine )
        {
	        DumpCommandLineAndResponseFile( "PS2/CW VU Preprocess",
                                            CmdLine,
                                            Name );
        }

        // Execute command for preprocess.
        Result = system( CmdLine );
    }

    //
    // Second pass:  VU assemble.
    //
    {
        // Create the response file.
        sprintf( Name, "%s\\_CW_VU_ASM_ResponseFile.txt", C.m_OutputPath );

        // Open the response file.
        pFile = fopen( Name, "wt" );
        if( pFile == NULL )
        {
            printf( "xCL -- ERROR: Unable to open CodeWarrior response file: %s\n", Name );
            return( EXIT_FAILURE );
        }   

        // Add standard command line options.
        fprintf( pFile, " -g\n" );

        // Add current directory as an include directory.
        fprintf( pFile, " -I.\n" );

        // Add the include paths.
        for( j = 0; j < C.m_IncludeDir.GetCount(); j++ )
            fprintf( pFile, " -I\"%s\"\n", C.m_IncludeDir[j] );

        // Add the PS2 system includes.
        fprintf( pFile, " -I\"%s/3rdParty/PS2/Sony/sce/ee/gcc/ee/include\" \n", pBase );

        // Done.
        fclose( pFile );

        // Build the command line.
        sprintf( CmdLine, "ee-dvp-as.exe @\"%s\" -o \"%s/%s.obj\" \"%s%s%s.i\" > \"%s/%s.lst\" ", 
                          Name, 
                          C.m_OutputPath, FName,
                          Drive, Dir, FName,
                          C.m_OutputPath, FName );

        // Verbose?
        if( g_Verbose || g_EchoCmdLine )
        {
	        DumpCommandLineAndResponseFile( "PS2/CW VU Assemble",
                                            CmdLine,
                                            Name );
        }

        // Execute command for CW/VU assembler.
        Result = system( CmdLine );
    }

    // Move the .i file to the build output directory.
    sprintf( CmdLine, "move \"%s%s%s.i\" \"%s\" ",
                      Drive, Dir, FName,
                      C.m_OutputPath );

    // Do the move.
    Result = system( CmdLine );

    // That's it.
    return( Result );
}
*/
//==============================================================================

static 
int CompileSourceOther( void )
{
    char    Ext[ _MAX_EXT ];
    int     Result = EXIT_SUCCESS;
    int     i;

    for( i = 0; i < C.m_SourceOther.GetCount(); i++ )
    {
        _splitpath( C.m_SourceOther[i], NULL, NULL, NULL, Ext );

        if( stricmp( ".s", Ext ) == 0 )
        {
            Result |= Compile_s_File( i );
        }

        else
        if( stricmp( ".vu", Ext ) == 0 )
        {
            Result |= Compile_vu_File( i );
        }

        else
        {
            printf( ".\n" );
            printf( "xCL -- ERROR: Do not know how to process file: %s\n", C.m_SourceOther[i] );
            printf( ".\n" );
            fflush( stdout );
        }
    }

    return( Result );
}

//==============================================================================

static 
int Compiler( void )
{
    int Result = EXIT_SUCCESS;

    if( C.m_SourceCode. GetCount() > 0 )  Result |= CompileSourceCode();
    if( C.m_SourceOther.GetCount() > 0 )  Result |= CompileSourceOther();

    return( Result );
}

//==============================================================================

static 
int Linker( void )
{
    int     i;
    int     Result;
    char    Map     [ _MAX_PATH  ];
    char    Name    [ _MAX_PATH  ];
    char    Drive   [ _MAX_DRIVE ];
    char    Dir     [ _MAX_DIR   ];
    char    FName   [ _MAX_FNAME ];
    char    Ext     [ _MAX_EXT   ];
    char    CmdLine [ 32784 ];
    FILE*   pLRF    = NULL;

    // Determine response file name and map name.
    _splitpath( C.m_OutputFile, Drive, Dir, FName,      Ext        );
    _makepath ( Name,           Drive, Dir, "_CW_Link", ".txt"     );
    _makepath ( Map,            Drive, Dir, FName,      ".map.txt" );

    // Build the command line.

    if( Which == PC  )  sprintf( CmdLine, "mwld.exe    @\"%s\"", Name );
    if( Which == PS2 )  sprintf( CmdLine, "mwldps2.exe @\"%s\"", Name );

    // Open the response file.
    pLRF = fopen( Name, "wt" );
    if( pLRF == NULL )
    {
        printf( "xCL -- ERROR: Unable to open file for write: %s\n", Name );
        return( 0 );
    }

    //
    // Write stuff to the response file.
    //

    // Output.
    fprintf( pLRF, " -o %s\n", C.m_OutputFile );

    // Add any custom link options.
    if( C.m_LinkOptions[0] != '\0' )
        fprintf( pLRF, " %s\n", C.m_LinkOptions );

    // Generic options.
    fprintf( pLRF, " -nostderr    \n" );
    fprintf( pLRF, " -nostdlib    \n" );
    fprintf( pLRF, " -nowraplines \n" );
    fprintf( pLRF, " -msgstyle IDE\n" );
    fprintf( pLRF, " -application \n" );

    // PC specific options.
    if( Which == PC )
    {
        fprintf( pLRF, " -sym codeview \n" );
        fprintf( pLRF, " -map \"%s\"   \n", Map );

        if( C.m_SubSystem == SUBSYS_WINDOWS )
            fprintf( pLRF, " -subsystem windows\n" );

        if( C.m_SubSystem == SUBSYS_WINCON )
            fprintf( pLRF, " -subsystem console\n" );
    }
   
    // PS2 specific options.
    if( Which == PS2 )
    {
        fprintf( pLRF, " -sym on         \n" );
        fprintf( pLRF, " -w off          \n" );
        fprintf( pLRF, " -map            \n" );
        fprintf( pLRF, " -main ENTRYPOINT\n" );

        // Link Command File.
        fprintf( pLRF, " PS2LinkSegment.lcf \n" );
    }

    // Add project specified system library paths.
    for( i = 0; i < C.m_LibraryDir.GetCount(); i++ )                       
        fprintf( pLRF, " -L\"%s\"", C.m_LibraryDir[i] );

    // Add each of the library files.
    for( i = 0; i < C.m_LibraryFile.GetCount(); i++ )
    {   
        char Path[ 8192 ];

        // Does the library name include a path?
        _splitpath( C.m_LibraryFile[i], NULL, Path, NULL, NULL );
        if( *Path == '\0' )
        {
            // There is no path.  Use "-l" to search library paths.
            fprintf( pLRF, " -l%s\n", C.m_LibraryFile[i] );
        }
        else
        {
            fprintf( pLRF, " %s\n", C.m_LibraryFile[i] );
        }
    }

    // Add specified objects.
    for( i = 0; i < C.m_ObjectFile.GetCount(); i++ )                       
        fprintf( pLRF, " %s\n", C.m_ObjectFile[i] );

    //
    // Add default system library paths.
    //

    if( Which == PC )
    {
        char Path[] = "3rdParty\\PC\\Metrowerks";

        fprintf( pLRF, " -L\"%s\\%s\\MSL\\MSL_C\\MSL_Win32\\Lib\\x86\"         \n", pBase, Path );
        fprintf( pLRF, " -L\"%s\\%s\\MSL\\MSL_C++\\MSL_Win32\\Lib\\x86\"       \n", pBase, Path );
        fprintf( pLRF, " -L\"%s\\%s\\Win32-x86 Support\\Libraries\\Runtime\"   \n", pBase, Path );
        fprintf( pLRF, " -L\"%s\\%s\\Win32-x86 Support\\Libraries\\Win32 SDK\" \n", pBase, Path );
    }

    if( Which == PS2 )
    {
        char Path1[] = "3rdParty\\PS2\\Sony";
        char Path2[] = "3rdParty\\PS2\\Metrowerks";

        fprintf( pLRF, " -L\"%s\\%s\\sce\\ee\\lib\"                           \n", pBase, Path1 );
        fprintf( pLRF, " -L\"%s\\%s\\sce\\ee\\gcc\\lib\"                      \n", pBase, Path1 );
        fprintf( pLRF, " -L\"%s\\%s\\sce\\ee\\gcc\\ee\\lib\"                  \n", pBase, Path1 );
        fprintf( pLRF, " -L\"%s\\%s\\sce\\ee\\gcc\\lib\\gcc-lib\\ee\\2.95.2\" \n", pBase, Path1 );

        fprintf( pLRF, " -L\"%s\\%s\\PS2 Support\\Msl\\MSL_C\\MSL_MIPS\\Lib\"   \n", pBase, Path2 );
        fprintf( pLRF, " -L\"%s\\%s\\PS2 Support\\Msl\\MSL_C++\\MSL_MIPS\\Lib\" \n", pBase, Path2 );
    }

    //
    // Add default system libraries.
    //

    if( Which == PC )
    {
        fprintf( pLRF, " -lgdi32.lib       \n" );
        fprintf( pLRF, " -luser32.lib      \n" );
        fprintf( pLRF, " -lmwcrtld.lib     \n" );
        fprintf( pLRF, " -lwsock32.lib     \n" );
        fprintf( pLRF, " -lcomdlg32.lib    \n" );
        fprintf( pLRF, " -lkernel32.lib    \n" );
        fprintf( pLRF, " -lansiCx86d.lib   \n" );
        fprintf( pLRF, " -lansiCPPx86d.lib \n" );
    }

    if( Which == PS2 )
    {
        fprintf( pLRF, " -lMSLGCC_PS2.lib  \n" );
        fprintf( pLRF, " -lANSICPP_PS2.lib \n" );
        fprintf( pLRF, " -llibgraph.lib    \n" );
        fprintf( pLRF, " -llibdma.lib      \n" );  
        fprintf( pLRF, " -llibdev.lib      \n" );  
        fprintf( pLRF, " -llibpkt.lib      \n" );  
        fprintf( pLRF, " -llibvu0.lib      \n" );
        fprintf( pLRF, " -llibmpeg.lib     \n" );
        fprintf( pLRF, " -llibipu.lib      \n" );
        fprintf( pLRF, " -llibpc.lib       \n" );
        fprintf( pLRF, " -llibkernl.lib    \n" );
        fprintf( pLRF, " -llibcdvd.lib     \n" );
        fprintf( pLRF, " -lliblout.lib     \n" );
        fprintf( pLRF, " -llibpad.lib      \n" );
        fprintf( pLRF, " -llibsdr.lib      \n" );
        fprintf( pLRF, " -llibssyn.lib     \n" );
        fprintf( pLRF, " -llibmc.lib       \n" );
        fprintf( pLRF, " -llibmsin.lib     \n" );
        fprintf( pLRF, " -llibm.lib        \n" );
        fprintf( pLRF, " -llibc.lib        \n" );
        fprintf( pLRF, " -llibgcc.lib      \n" );
    }

    //
    // Done with link response file.
    //
    fclose( pLRF );

    // Extra verbage?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "CodeWarrior Link",
                                        CmdLine,
                                        Name );
    }

    // A little more standard verbage...
    printf( "%s%s\n", FName, Ext );
    fflush( stdout );

    // Execute the command line.
    Result = system( CmdLine );

    // We're done.
    return( Result );
}

//==============================================================================

static 
int Librarian( void )
{
    int     i;
    int     Result;
    char    Name    [ _MAX_PATH  ];
    char    Drive   [ _MAX_DRIVE ];
    char    Dir     [ _MAX_DIR   ];
    char    CmdLine [ 32784 ];
    FILE*   pFile    = NULL;

    // Determine response file name.
    _splitpath( C.m_OutputFile, Drive, Dir, NULL,      NULL   );
    _makepath ( Name,           Drive, Dir, "_CW_Lib", ".txt" );

    // Open the response file.
    pFile = fopen( Name, "wt" );
    if( pFile == NULL )
    {
        printf( "xCL -- ERROR: Unable to open file for write: %s\n", Name );
        return( 0 );
    }

    // Build the command line.
    if( Which == PC  )  sprintf( CmdLine, "mwld.exe    @%s", Name );
    if( Which == PS2 )  sprintf( CmdLine, "mwldps2.exe @%s", Name );

    //
    // Write stuff to the response file.
    //

    // Output.
    fprintf( pFile, " -o %s\n", C.m_OutputFile );

    //
    // Other options.
    //

    fprintf( pFile, " -library      \n" );
    fprintf( pFile, " -nostderr     \n" );
    fprintf( pFile, " -nostdlib     \n" );
    fprintf( pFile, " -nowraplines  \n" );
    fprintf( pFile, " -msgstyle IDE \n" );

    if( Which == PC )
    {
        fprintf( pFile, " -subsystem windows \n" );
        fprintf( pFile, " -sym codeview      \n" );
    }

    if( Which == PS2 )
    {
        fprintf( pFile, " -sym on \n" );
    }

    // Add each of the object files.
    for( i = 0; i < C.m_ObjectFile.GetCount(); i++ )
        fprintf( pFile, " %s\n", C.m_ObjectFile[i] );
    
    // Add each of the library files.
    for( i = 0; i < C.m_LibraryFile.GetCount(); i++ )
        fprintf( pFile, " %s\n", C.m_LibraryFile[i] );

    // We're done with the response file.
    fclose( pFile );

    // Extra verbage?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "CodeWarrior Librarian", 
                                        CmdLine, 
                                        Name );
    }

    // Execute the command.
    Result = system( CmdLine );

    // We're done!
    return( Result );
}

//==============================================================================

static
int Process_CodeWarrior( void )
{
    int Result = EXIT_FAILURE;

    // Warn user about any unknown parameters.
    {   
        int i;
        for( i = 0; i < C.m_UnknownOption.GetCount(); i++ )
            printf( "xCL -- Warning: Option not processed: %s\n", C.m_UnknownOption[i] );
    }
    
    // Self explanitory.
    if( !PrepareEnvironment() )
        return( Result );

    // Based on the desired tool, call a function to deal with it.
    switch( C.m_Tool )
    {
    case TOOL_COMPILER:     Result = Compiler ();  break;
    case TOOL_LINKER:       Result = Linker   ();  break;
    case TOOL_LIBRARIAN:    Result = Librarian();  break;
    default:                
        printf( "xCL -- ERROR: Unable to determine tool for CodeWarrior operation.\n" );
        break;
    }

    // We're done.
    return( Result );
}

//==============================================================================

int Process_PC_CodeWarrior( void )
{
    Which = PC;
    return( Process_CodeWarrior() );
}

//==============================================================================

int Process_PS2_CodeWarrior( void )
{
    Which = PS2;
    return( Process_CodeWarrior() );
}

//==============================================================================
