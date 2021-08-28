//==============================================================================
//
//  xCL_GCN_SN.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES    
//==============================================================================

#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <windows.h>
#include <time.h>

#include "xCL.hpp"
#include "Inevitable.hpp"
#include "xCL_GCN_SN.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//efine SINGLE_THREADED
#define MAX_THREADS         2

//==============================================================================
//  LOCAL STORAGE
//==============================================================================

static cmd_line_data&   C                       = g_CmdLineData;
static token_list       ObjLib;
static token_list       Section;
static char*            pBase;

static char             g_NewRName[_MAX_PATH];
static volatile int     g_CompileResult         = EXIT_SUCCESS;
static long             g_iFile                 = -1;
static long             g_nThreads              =  0;
static HANDLE           g_hEvent  [MAX_THREADS] = {0};
static HANDLE           g_hThread [MAX_THREADS] = {0};
static unsigned long    g_ThreadID[MAX_THREADS] = {0};

//==============================================================================

static
char* pINI = 
    "[ngccc]\n"
    "compiler_path=@/SN/bin\n"
    "c_include_path=@/SN/include;@/Nintendo/include;@/Other/Include\n"
    "library_path=@/SN/lib;@/Nintendo/Lib\n"
    "assembler_path=@/SN/bin\n"
    "linker_path=@/SN/bin\n"   
    "cplus_include_path=@/SN/include;@/Nintendo/include;@/Other/Include\n"
    "linker_name=ngcld.exe\n"
    "opt_assembler_name=ngcas\n"
    "assembler_name=ngcas\n"    
    "linker_script=@/SN/bin/ngc.ld\n";

//==============================================================================
//  FUNCTION ANNOUCEMENTS
//==============================================================================

static DWORD WINAPI CompileThread( LPVOID pData );

//==============================================================================
//  PRIVATE FUNCTIONS
//==============================================================================

static 
bool PrepareEnvironment( void )
{
    char  Buffer[ 32784 ];
    char* pPath = NULL;
    FILE* pFile;

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
    // Set up paths for all the Nintendo GDEV build tools required to generate
    // the dvd emulation image
    {
        _putenv("DOLPHIN_PLATFORM=HW2");
        _putenv("DOLPHIN_VERSION=1.0");

        sprintf(Buffer,"DOLPHIN_ROOT=%s\\3rdparty\\gcn\\Nintendo",pBase);   _putenv(Buffer);
        sprintf(Buffer,"DOLPHIN_X86_TOOLS=%s\\bin");                        _putenv(Buffer);            //_putenv("DOLPHIN_X86_TOOLS=%DOLPHIN_ROOT%\\bin");
        sprintf(Buffer,"NPDP_ROOT=%s",getenv("DOLPHIN_ROOT"));              _putenv(Buffer);   //_putenv("NPDP_ROOT=%DOLPHIN_ROOT%");
        sprintf(Buffer,"ODEMUSDKBIN=%s\\bin",getenv("NPDP_ROOT"));          _putenv(Buffer);
        sprintf(Buffer,"BUILDTOOLS_ROOT=%s\\Build",getenv("ODEMUSDKBIN"));  _putenv(Buffer);

        if (g_Verbose)
        {
            OpenSection("DolphinSDK environment variables");
            printf("DOLPHIN_PLATFORM=%s\n",getenv("DOLPHIN_PLATFORM"));
            printf("DOLPHIN_VERSION=%s\n",getenv("DOLPHIN_VERSION"));
            printf("DOLPHIN_ROOT=%s\n",getenv("DOLPHIN_ROOT"));
            printf("NPDP_ROOT=%s\n",getenv("NPDP_ROOT"));
            printf("ODEMUSDKBIN=%s\n",getenv("ODEMUSDKBIN"));
            printf("BUILDTOOLS_ROOT=%s\n",getenv("BUILDTOOLS_ROOT"));

            CloseSection();
        }

    }
    //
    // Add the SN and Nintendo tools directories to the path.
    //                                 
    {
        // Add the SN and Nintendo paths.
        sprintf( Buffer, "PATH=%s\\3rdParty\\GCN\\SN;"
                              "%s\\3rdParty\\GCN\\SN\\bin;", 
                          pBase, pBase, pBase );

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

        OpenSection( "Path for GCN/SN Operation" );
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

    //
    // Identify the SN.INI file we will use.
    //
    // First, see if there is an SN.INI file in the current directory.  If there 
    // is, then just use that one.  Otherwise, create one.

    pFile = fopen( "sn.ini", "rt" );
    if( pFile )
    {
        // Export an SN_PATH environment variable.
        _putenv( "SN_PATH=." );
        _putenv( "SN_NGC_PATH=.");
    }
    else
    {
        //
        // Create the SN.INI file.
        //

        char* p1;
        char* p2;
        char  Path[ 32784 ];
        int   s;

        // Build the path that we will substitute into the INI file.
        s = sprintf( Path, "%s\\3rdParty\\GCN", UseOnlyBackSlashes(pBase) );

        // Export a SN_PATH environment variable.
        sprintf( Buffer, "SN_PATH=%s\\3rdParty\\GCN\\SN", pBase );
        _putenv( Buffer );

        sprintf( Buffer, "SN_NGC_PATH=%s\\3rdParty\\GCN\\SN", pBase );
        _putenv( Buffer );

        // Open the file.
        sprintf( Buffer, "%s\\3rdParty\\GCN\\SN\\sn.ini", pBase );
        pFile = fopen( Buffer, "wt" );
        if( pFile == NULL )
        {
            printf( "xCL -- ERROR: Failed to open file for write: %s\n", Buffer );
            return( false );
        }

        //
        // Print the contents of the file.  Replace each '@' with the 
        // path to the Nintendo files.
        // 
        // Two pointers are used to avoid writing to the file one byte at 
        // a time.
        //

        p1 = p2 = pINI;
        while( *p2 != '\0' )
        {
            if( *p2 == '@' )
            {
                fwrite( p1,   1, p2-p1, pFile );    // Write everything we haven't written so far.
                fwrite( Path, 1, s,     pFile );    // Write replacement for '@'.
                p2++;
                p1 = p2;
            }
            else
            {
                p2++;
            }
        }
        fwrite( p1, 1, p2-p1, pFile );  // Write everything we haven't written so far.

        // We're done.
        fclose( pFile );
    }                 

    //
    // Extra verbage.  Show the SN.INI file.
    // 

    if( g_Verbose )
    {
        char* pSN;
        char* pNGC;

        OpenSection( "SN_PATH and SN.INI file" );
        pSN  = getenv( "SN_PATH" );
        pNGC = getenv( "SN_NGC_PATH");
        if( !pSN )
            printf( "<<< SN_PATH environment variable not defined. >>>\n" );
        else
        {
            printf( "SN_PATH     = %s\n", pSN  );
            printf( "SN_NGC_PATH = %s\n", pNGC );
            DivideSection();
            sprintf( Buffer, "%s\\sn.ini", pSN );
            DumpTextFileContent( Buffer );
        }
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

static 
int CompileSourceCode( void )
{
    int     Result = EXIT_SUCCESS;
    int     i;

    char    Message [ 32784 ];      // Message to show to user
    char    NewCmdLn[ 32784 ];      // New command line to execute
    char    VC6CmdLn[ 32784 ];      // VC6 command line to execute
    char    VC6RName[ _MAX_PATH  ]; // VC6 response file name

    FILE*   pNewRFile;              // New response file pointer
    FILE*   pVC6RFile;              // VC6 response file pointer

    //--------------------------------------------------------------------------
    // Setup local and temporary variables.
    //--------------------------------------------------------------------------

    // Start with an empty message.  Build it as we go.
    strcpy( Message, "xCL --" );

    //--------------------------------------------------------------------------
    // Build the SN response file.
    //--------------------------------------------------------------------------

    // Determine response file name.
    sprintf( g_NewRName, "%s\\_SN_ResponseFile.txt", C.m_OutputPath );

    // Open the response file.
    pNewRFile = fopen( g_NewRName, "wt" );
    if( pNewRFile == NULL )
    {
        printf( "xCL -- ERROR: Unable to open SN response file: %s\n", g_NewRName );
        return( EXIT_FAILURE );
    }   

    // Add some standard compile options.
    fprintf( pNewRFile, " -c\n"              );   // Compile only; don't link
    fprintf( pNewRFile, " -fdevstudio\n"     );   // DevStudio output format
    //rintf( pNewRFile, " -fexceptions\n"    );   // Enable exceptions
    fprintf( pNewRFile, " -fno-exceptions\n" );   // Disable exceptions
    fprintf( pNewRFile, " -fno-rtti\n"       );   // Disable rtti

    //?? With GNU compiler, all debugging information is stored outside of 
    //?? the object files.  So, no harm comes from generating the debug 
    //?? information, even if it was not specifically requested.
    fprintf( pNewRFile, " -g \n" );
    strcat( Message, " DebugInfo[ON]" );

    // Add some standard compile options.
    fprintf( pNewRFile, " -fno-common \n" );        // Put un-init data in BSS
    fprintf( pNewRFile, " -finline-functions \n" ); // Auto inline small functions.

    // Warnings.
    fprintf( pNewRFile, " -Wall       \n" );    // "All" warnings
    fprintf( pNewRFile, " -W          \n" );    // More warnings
    fprintf( pNewRFile, " -Wno-switch \n" );    // Allow no default case
    fprintf( pNewRFile, " -Werror     \n" );    // Warnings as errors
    //fprintf( pNewRFile, " -fopt-stabs\n"  );    // Optimize symbol table during compile.

    // Allow friend functions to template classes.
    fprintf( pNewRFile, " -Wno-non-template-friend\n" );   

    // Add compile optimization specification.
    {
        char* List[] = { "0", "1", "2", "3" };
        char* pOpt;

        // Args from Microsoft:         Map to GNU arg:
        //  -Od = "debug"                "0"  
        //  -O1 = "minimize size"        "1"  
        //  -Ot = "default"              "2"  
        //  -O2 = "maximize speed"       "3"  

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
        fprintf( pNewRFile, " -O%s\n", pOpt );
    }

    // Add current directory as an include directory.
    fprintf( pNewRFile, " -I.\n" );
   
    // Add the include paths.
    for( i = 0; i < C.m_IncludeDir.GetCount(); i++ )
        fprintf( pNewRFile, " -I\"%s\"\n", C.m_IncludeDir[i] );

    // Normally we would add the system includes here with code like that shown
    // below.  But, the system includes are handled by the SN.INI file.
    //{
    //  char Path[] = "3rdParty/GCN/Nintendo";
    //  fprintf( pNewRFile, " -I\"%s/%s/include\" \n", pBase, Path );
    //}

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
    //
    // Also...
    //
    // Update: I was undefining _WIN32, but this caused problems when 
    // compiling STL on GCN during the VC6 phase.  So, I've stopped
    // undefining _WIN32 for now until I find out the exact reason I
    // did it in the first place.  Current large PS2 projects compile
    // fine regardless of _WIN32.
    // **** REMOVED
    // ** We have to use Microsoft's preprocessor to get our dependency 
    // ** information.  Their preprocessor introduces some extra defines.
    // ** And this can mess things up sometimes.  So, UNDEFINE them!
    // **** REMOVED "/U _WIN32"
    fprintf( pVC6RFile, " /P \n" );

    // Miscellaneous options.
    if( C.m_SBRPath[0] )      fprintf( pVC6RFile, " /FR\"%s\"\n", C.m_SBRPath );
    if( C.m_PDBPath[0] )      fprintf( pVC6RFile, " /Fd\"%s\"\n", C.m_PDBPath );
    if( C.m_Preprocess )      fprintf( pVC6RFile, " /P /C\n" );

    // In order to get the code to preprocess in VC, a few macros and such
    // must be defined.  Macros with arguments cannot be defined on the 
    // command line.  So, we will force all source files to include a tiny 
    // header file with the macros.    
    {
        unsigned long Written;
        HANDLE        Handle;
        FILETIME      FileTime;
        char          Name[ _MAX_PATH ];
        char          Buffer[] = "#define __attribute__(a)\r\n"
                                 "#define __IEEE_LITTLE_ENDIAN\r\n"
                                 "typedef unsigned int u_long128;\r\n";

        // Make the name.
        sprintf( Name, "%s/_SN_in_VC.h", C.m_OutputPath );

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

    // Add the GCN and SN system includes.
    {
        char Path[] = "3rdParty/GCN";

        fprintf( pVC6RFile, " -I\"%s/%s/Nintendo/include\" \n", pBase, Path );
        fprintf( pVC6RFile, " -I\"%s/%s/SN/include\"       \n", pBase, Path );
        fprintf( pVC6RFile, " -I\"%s/%s/Other/include\"    \n", pBase, Path );
    }

    // We need an extra include path to find the "forced include" header file.
    //
    // NOTE:  The forced include is something like "/FI Debug\_File.h".  This
    // only has meaning from within the current directory (which is the project 
    // directory).  However, the file we are compiling may be in a different 
    // directory.  Includes check the directory with the source file, then all 
    // paths added on the command line.  If the source file is in another 
    // directory, using "/I Debug" won't work.  Add the current directory to the
    // path (/I .), and this will resolve to ".\Debug\_File.h" regardless of the
    // location of the source file since the current directory does not change.
    //
    // Using "/FI _File.h" and "/I Debug" has a flaw, too.  But I can't remember
    // what it is right now.

    fprintf( pVC6RFile, " /I . \n" );

    // Add the defines to the response file.
    for( i = 0; i < C.m_Define.GetCount(); i++ )
        fprintf( pVC6RFile, " /D %s \n", C.m_Define[i] );

    // Add the XCORE_PATH macro.
    fprintf( pVC6RFile, " /D XCORE_PATH=\\\"%s\\\"", UseOnlySlashes(pBase) );

    // That's it.
    fclose( pVC6RFile );

    //--------------------------------------------------------------------------
    // Execute!
    //--------------------------------------------------------------------------

    //
    // For every file we need to compile, we must run it through VC6 
    // (preprocess only) and then through the SN compiler.
    //

#ifdef SINGLE_THREADED

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

        //------//
        //  SN  //
        //------//
        {
            sprintf( NewCmdLn, "ngccc.exe @\"%s\" \"%s\" -o \"%s\\%s.obj\"", 
                                g_NewRName, 
                                C.m_SourceCode[i],
                                C.m_OutputPath,
                                SrcFName );

            // Extra verbage?
            if( g_Verbose || g_EchoCmdLine )
                DumpCommandLineAndResponseFile( "GCN/SN Compile",
                                                NewCmdLn, 
                                                g_NewRName );

            // Execute command for SN compiler.
            Result |= system( NewCmdLn );
        }
    }

    //--------------------------------------------------------------------------
    //  We're done!
    //--------------------------------------------------------------------------
    return( Result );

#else

    (void)NewCmdLn;

    // Create compile threads.
    for( i=0; i<MAX_THREADS; i++ )
    {
        g_hEvent [i] = CreateEvent ( NULL, TRUE, FALSE, NULL );
        g_hThread[i] = CreateThread( NULL, 0, CompileThread, (void*)&g_hEvent[i], 0, &g_ThreadID[i] );
    }

    // Run through VC for dependency info.
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
    }

    // Wait for all compile threads to complete.
    WaitForMultipleObjects( MAX_THREADS, &g_hEvent[0], TRUE, INFINITE );

    // Delete events.
    for( i=0 ; i<MAX_THREADS ; i++ )
    {
        CloseHandle( g_hEvent[i] );
    }

    //--------------------------------------------------------------------------
    //  We're done!
    //--------------------------------------------------------------------------
    return( g_CompileResult );

#endif
}

//==============================================================================

static DWORD WINAPI CompileThread( LPVOID pData )
{
    HANDLE  hEvent = *(HANDLE*)pData;
    int     iFile = 0;
    char    NewCmdLn[ 32784 ];      // New command line to execute

    // Loop until all files are compiled.
    while( iFile < C.m_SourceCode.GetCount() )
    {
        // Allocate a file to compile.
        iFile = InterlockedIncrement( &g_iFile );

        // Valid file number?
        if( (iFile >= 0) && (iFile < C.m_SourceCode.GetCount()) )
        {
            char SrcFName[ _MAX_FNAME ];

            // Get source filename.
            _splitpath( C.m_SourceCode[iFile], NULL, NULL, SrcFName, NULL );

            // Build command line.
            sprintf( NewCmdLn, "ngccc.exe @\"%s\" \"%s\" -o \"%s\\%s.obj\"", 
                                g_NewRName, 
                                C.m_SourceCode[iFile],
                                C.m_OutputPath,
                                SrcFName );

            // Extra verbage?
            if( g_Verbose || g_EchoCmdLine )
                DumpCommandLineAndResponseFile( "GCN/SN Compile",
                                                NewCmdLn, 
                                                g_NewRName );

            // Execute command for SN compiler.
            g_CompileResult |= system( NewCmdLn );
        }
    }

    // Signal thread completion event.
    SetEvent( hEvent );

    // Exit.
    return( 0 );
}

//==============================================================================

static
int Compile_s_File( int i )
{
    char    FName   [ _MAX_FNAME ];
    char    Ext     [ _MAX_EXT   ];
    char    CmdLine [ 32784 ];
    char    Buffer  [ 32784 ];
    int     Result = EXIT_SUCCESS;
    int     j;

    // Pick the name apart.
    _splitpath( C.m_SourceOther[i], NULL, NULL, FName, Ext );

    // Start building the command line.
    sprintf( CmdLine, "ngccc -g -xassembler-with-cpp -I." );

    // Add the include paths.
    for( j = 0; j < C.m_IncludeDir.GetCount(); j++ )
    {
        sprintf( Buffer, " -I\"%s\"", C.m_IncludeDir[j] );
        strcat ( CmdLine, Buffer );
    }

    // Add the defines to the command line.
    for( j = 0; j < C.m_Define.GetCount(); j++ )
    {
        sprintf( Buffer, " -D%s", C.m_Define[j] );
        strcat ( CmdLine, Buffer );
    }

    // Add the XCORE_PATH macro.
    sprintf( Buffer, " -DXCORE_PATH=\\\"%s\\\"", UseOnlySlashes(pBase) );
    strcat ( CmdLine, Buffer );

    // Add the file names.
    sprintf( Buffer, " -c \"%s\" -o \"%s\\%s.obj\"",
                      C.m_SourceOther[i],
                      C.m_OutputPath,
                      FName );
    strcat ( CmdLine, Buffer );

    // Verbose?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "GCN/SN Assemble",
                                        CmdLine,
                                        NULL );
    }

    // Feedback for the user.
    printf( "%s%s\n", FName, Ext );
    fflush( stdout );

    // Execute command for SN assembler.
    Result = system( CmdLine );
    return( Result );
}

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
    char    LinkerScript [ _MAX_PATH];
    char    CmdLine [ 32784 ];
    FILE*   pLRF    = NULL;

    // Determine response file name and map name.
    _splitpath( C.m_OutputFile, Drive, Dir, FName,          Ext        );
    _makepath ( Name,           Drive, Dir, "_GCN_SN_Link", ".txt"     );
    _makepath ( Map,            Drive, Dir, FName,          ".map.txt" );

    // Build the command line.
    // NOTE: Using ngccc.exe to link rather than the linker.  Couldn't get
    // the linker to work.
    sprintf( CmdLine, "ngccc.exe @\"%s\"", Name );

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

    // Strip during the link?
    if( g_StripSymbols )
    {
        printf( "xCL -- Stripping debug info and symbols from executable image.\n" );
        fprintf( pLRF, " -s\n" );
    }

    // Add any custom link options.
    if( C.m_LinkOptions[0] != '\0' )
        fprintf( pLRF, " %s\n", C.m_LinkOptions );

    // Add all objects.
    for( i = 0; i < C.m_ObjectFile.GetCount(); i++ )                       
        fprintf( pLRF, " %s\n", C.m_ObjectFile[i] );

    // Add library paths.
    for( i = 0; i < C.m_LibraryDir.GetCount(); i++ )                       
        fprintf( pLRF, " -l %s\n", C.m_LibraryDir[i] );

    fprintf( pLRF, " --start-group -lsn -lm -lc -lgcc\n" );

    // Add standard library paths.
    fprintf( pLRF, " -L %s/3rdParty/GCN/SN/lib       \n", pBase );
    fprintf( pLRF, " -L %s/3rdParty/GCN/Nintendo/lib \n", pBase );
    fprintf( pLRF, " -L %s/3rdParty/GCN/Other/lib \n", pBase );
    
    // Add all libraries.
    for( i = 0; i < C.m_LibraryFile.GetCount(); i++ )
        fprintf( pLRF, " %s\n", C.m_LibraryFile[i] );

    // Add standard libraries.
    //
    // There are debug and release versions.  If the string "Release" is in the
    // output file name ("GCN-DevKit-Release\MyApp.elf") then use the release
    // versions.
    if( strstr( C.m_OutputFile, "Release" ) )
    {
        fprintf( pLRF, " libsn.a       \n" );
        fprintf( pLRF, " base.a        \n" );
        fprintf( pLRF, " os.a          \n" );  
        fprintf( pLRF, " db.a          \n" );  
        fprintf( pLRF, " mtx.a         \n" );
        fprintf( pLRF, " dvd.a         \n" );
        fprintf( pLRF, " vi.a          \n" );
        fprintf( pLRF, " si.a          \n" );
        fprintf( pLRF, " exi.a         \n" );
        fprintf( pLRF, " demo.a        \n" );
        fprintf( pLRF, " pad.a         \n" );
        fprintf( pLRF, " gx.a          \n" );
        fprintf( pLRF, " g2d.a         \n" );
        fprintf( pLRF, " ai.a          \n" );
        fprintf( pLRF, " ax.a          \n" );
        fprintf( pLRF, " ar.a          \n" );
        fprintf( pLRF, " dsp.a         \n" );
        fprintf( pLRF, " mix.a         \n" );
        fprintf( pLRF, " sp.a          \n" );
        fprintf( pLRF, " texpalette.a  \n" );
        fprintf( pLRF, " odenotstub.a  \n" );
        fprintf( pLRF, " amcstubs.a    \n" );
        fprintf( pLRF, " odemuexi2.a   \n" );
        fprintf( pLRF, " filecache.a   \n" );
        fprintf( pLRF, " support.a     \n" );
        fprintf( pLRF, " card.a        \n" );
    }
    else
    {
        fprintf( pLRF, " libsn.a       \n" );
        fprintf( pLRF, " baseD.a       \n" );
        fprintf( pLRF, " osD.a         \n" );  
        fprintf( pLRF, " dbD.a         \n" );  
        fprintf( pLRF, " mtxD.a        \n" );
        fprintf( pLRF, " dvdD.a        \n" );
        fprintf( pLRF, " viD.a         \n" );
        fprintf( pLRF, " siD.a         \n" );
        fprintf( pLRF, " exiD.a        \n" );
        fprintf( pLRF, " demoD.a       \n" );
        fprintf( pLRF, " padD.a        \n" );
        fprintf( pLRF, " gxD.a         \n" );
        fprintf( pLRF, " g2dD.a        \n" );
        fprintf( pLRF, " aiD.a         \n" );
        fprintf( pLRF, " axD.a         \n" );
        fprintf( pLRF, " arD.a         \n" );
        fprintf( pLRF, " mixD.a        \n" );
        fprintf( pLRF, " spD.a         \n" );
        fprintf( pLRF, " dspD.a        \n" );
        fprintf( pLRF, " texpaletteD.a \n" );
        fprintf( pLRF, " odenotstubD.a \n" );
        fprintf( pLRF, " amcstubsD.a   \n" );
        fprintf( pLRF, " odemuexi2D.a  \n" );
        fprintf( pLRF, " filecacheD.a  \n" );
        fprintf( pLRF, " supportD.a    \n" );
        fprintf( pLRF, " cardD.a       \n" );
    }

    // Figure out which linker script to use
    FILE* handle;
    int   length;


    // First try the output dir name with .ld appended (removing the
    // preceding slash first). We should get gcn-devkit-release.ld or
    // gcn-dvd-debug.ld etc.
    strcpy(LinkerScript,Dir);
    length = strlen(LinkerScript);

    if ( length && ((LinkerScript[length-1]=='\\') ||
                    (LinkerScript[length-1]=='/')) )
    {
        LinkerScript[strlen(LinkerScript)-1]=0x0;
    }
    strcat(LinkerScript,".ld");
    handle = fopen(LinkerScript,"rb");
    if (handle)
    {
        fclose(handle);
    }
    else
    {
        // If we didn't find a file then we mangle the name that is there
        // so we change it to gcn-debug.ld or gcn-release.ld
        strlwr(LinkerScript);
        if (strstr(LinkerScript,"-debug"))
        {
            strcpy(LinkerScript,"GCN-Debug.ld");
        }
        else
        {
            strcpy(LinkerScript,"GCN-Release.ld");
        }
        handle = fopen(LinkerScript,"rb");
        if (handle)
        {
            fclose(handle);
        }
        else
        {
            // if we can't find any other, just use the default
            strcpy(LinkerScript,"ngc.ld");
        }
    }

    // Add the remaining information.
    fprintf( pLRF, " --end-group -T %s \n", LinkerScript );
    fprintf( pLRF, " -o %s -Wl,-Map %s \n", C.m_OutputFile, Map );

    // Add a symbol which is unique to this link number. This is used 
    // to make sure the swap file and main code body are from the same
    // build cycle.
    time_t ltime;
    time( &ltime );

    fprintf( pLRF, " -Wl,-defsym,__VM_UNIQUE_ID__=%d\n",ltime);

    // Close the response file.
    fclose( pLRF );

    //
    // Done with response file.
    //

    // Extra verbage?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "GCN/SN Link",
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
    int     Result;
    int     i;
    FILE*   pLRF;
    char    Name     [ _MAX_PATH  ];
    char    Drive    [ _MAX_DRIVE ];
    char    Dir      [ _MAX_DIR   ];
    char    FName    [ _MAX_FNAME ];
    char    Ext      [ _MAX_EXT   ];
    char    CmdLine  [ 32784      ];

    // We must delete the library before we create the new one.
    sprintf( CmdLine, "if exist %s del %s > nul", 
                      C.m_OutputFile, 
                      C.m_OutputFile );
    system ( CmdLine );

    // Determine response file name and map name.
    _splitpath( C.m_OutputFile, Drive, Dir, FName,         Ext    );
    _makepath ( Name,           Drive, Dir, "_GCN_SN_Lib", ".txt" );

    // Build the command line.
    sprintf( CmdLine, "snarl -M < \"%s\"", Name );

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
    
    // Create the archive.
    fprintf( pLRF, "create %s\n", UseOnlySlashes( C.m_OutputFile ) );

    // Add all object modules to it.
    for( i = 0; i < C.m_ObjectFile.GetCount(); i++ )
    {
        fprintf( pLRF, "addmod %s\n", UseOnlySlashes( (char*)C.m_ObjectFile[i] ) );
    }

    // Add all libraries to it.
    for( i = 0; i < C.m_LibraryFile.GetCount(); i++ )
    {
        fprintf( pLRF, "addlib %s\n", UseOnlySlashes( (char*)C.m_LibraryFile[i] ) );
    }

    // That's it for the Librarian Response File.
    fprintf( pLRF, "save\n" );
    fprintf( pLRF, "end \n" );

    // Close the response file.
    fclose( pLRF );

    //
    // Done with response file.
    //

    // Extra verbage?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "GCN/SN Librarian",
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

int Process_GCN_SN( void )
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
        printf( "xCL -- ERROR: Unable to determine tool for GCN/SN operation.\n" );
        break;
    }

    // We're done.
    return( Result );
}

//==============================================================================
