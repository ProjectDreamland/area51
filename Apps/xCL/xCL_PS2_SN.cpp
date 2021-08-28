//==============================================================================
//
//  xCL_PS2_SN.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES    
//==============================================================================

#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <windows.h>

#include "xCL.hpp"
#include "Inevitable.hpp"
#include "xCL_PS2_SN.hpp"

#include "MakeDep.hpp"
#include "md5.h"

//==============================================================================
//  DEFINES
//==============================================================================

//efine SINGLE_THREADED
#define MAX_THREADS         2

// This should only be defined for external projects such as MoH3.
//#define LAZY_WARNINGS

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
        "[ps2cc]\n"
        "compiler_path=@/SN/EE/bin\n"
        "c_include_path=@/Sony/EE/include;@/SN/EE/include\n"
        "cplus_include_path=@/Sony/EE/include;@/SN/EE/include\n"
        "library_path=@/Sony/EE/lib;@/SN/EE/lib\n"
        "assembler_path=@/SN/EE/bin\n"
        "linker_path=@/SN/bin\n"
        "linker_script=@/SN/EE/lib/ps2.lk\n"
        "linker_name=ps2ld\n"
        "opt_assembler_name=ps2eeas\n" //xAS\n" //ps2eeas\n"
        "assembler_name=ps2eeas\n" //xAS\n" //ps2eeas\n"
        "dvp_asm_name=ee-dvp-as\n"
        "startup_module=@/SN/EE/lib/crt0.s\n"
        "dvp_include_path=@/SN/EE/include\n"
        "dvp_assembler_path=@/SN/EE/bin\n"
        "iop_bin_path=@/SN/IOP/bin\n"
        "iop_lib_path=@/Sony/IOP/lib;@/SN/IOP/lib\n"
        "ilb_lib_path=@/Sony/IOP/lib;@/SN/IOP/lib\n"
        "iop_linker_path=@/SN/IOP/bin\n"
        "iop_compiler_path=@/SN/IOP/bin\n"
        "iop_c_include_path=@/Sony/IOP/include;@/SN/IOP/include\n"
        "iop_cplus_include_path=@/Sony/IOP/include;@/SN/IOP/include\n"
        "iop_asm_name=as\n"
        "iop_linker_name=ld\n"
        "iop_stdilb=iop.ilb libsd.ilb cdvdman.ilb inet.ilb inetctl.ilb netcnf.ilb\n"
        "[ps2link]\n"
        "library_path=@/Sony/EE/lib;@/SN/EE/lib\n";

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
    // Add the SN and Sony tools directories to the path & remove C:\xBase\xCL
    //
    {
        // Add the SN and Sony paths.
        sprintf( Buffer, "PATH=%s\\3rdParty\\PS2\\SN\\bin;"
                              "%s\\3rdParty\\PS2\\SN\\EE\\bin;"
                              "%s\\3rdParty\\PS2\\SN\\IOP\\bin;"
                              "%s\\3rdParty\\PS2\\Sony\\EE\\bin;"
                              "%s\\3rdParty\\PS2\\Sony\\IOP\\bin;", 
                          pBase, pBase, pBase, pBase, pBase );

        // Add the original path.
        pPath = getenv( "PATH" );
        if( pPath )
        {
            if( strnicmp( pPath, "C:\\xBase\\xCL;", 13 ) == 0 )
                pPath += 13;

            strcat( Buffer, pPath );
        }

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

        OpenSection( "Path for PS2/SN Operation" );
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

    pFile = fopen( "SN.ini", "rt" );
    if( pFile )
    {
        // Export an SN_PATH environment variable.
        _putenv( "SN_PATH=." );
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
        s = sprintf( Path, "%s\\3rdParty\\PS2", UseOnlyBackSlashes(pBase) );

        // Export a SN_PATH environment variable.
        sprintf( Buffer, "SN_PATH=%s\\3rdParty\\PS2\\SN", pBase );
        _putenv( Buffer );

        // Open the file.
        sprintf( Buffer, "%s\\3rdParty\\PS2\\SN\\SN.ini", pBase );
        pFile = fopen( Buffer, "wt" );
        if( pFile == NULL )
        {
            printf( "xCL -- ERROR: Failed to open file for write: %s\n", Buffer );
            return( false );
        }

        //
        // Print the contents of the file.  Replace each '@' with the 
        // path to the Sony files.
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
        char* p;

        OpenSection( "SN_PATH and SN.INI file" );
        p = getenv( "SN_PATH" );
        if( !p )
            printf( "<<< SN_PATH environment variable not defined. >>>\n" );
        else
        {
            printf( "SN_PATH=%s\n", p );
            DivideSection();
            sprintf( Buffer, "%s\\SN.ini", p );
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

class source_file
{
public:
    xstring     m_PathName;
    s32         m_NodeIndex;
    bool        m_Cached;
    u8          m_MD5Digest[16];
};

xarray<source_file> g_SourceFiles;

static 
int CompileSourceCode( void )
{
    int     Result = EXIT_SUCCESS;
    int     i;

    char    Message [ 32784 ];      // Message to show to user    
    char    NewCmdLn[ 32784 ];      // New command line to execute
    char    VC6CmdLn[ 32784 ];      // VC6 command line to execute
    char    VC6RName[ _MAX_PATH  ]; // VC6 response file name

    xstring RF( 32768 );

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

    // Add some standard compile options.
    RF += "-c\n";                       // Compile only; don't link
    RF += "-fdevstudio\n";              // DevStudio output format

    //?? With GNU compiler, all debugging information is stored outside of 
    //?? the object files.  So, no harm comes from generating the debug 
    //?? information, even if it was not specifically requested.
    RF += "-g\n";
    strcat( Message, " DebugInfo[ON]" );

    // EE vs IOP.
    if( C.m_Target == BUILD_TARGET_PS2_IOP )
    {
        // Add some standard compile options.
        RF += "-G0\n";                  // No data in sbss segment
        RF += "-Wall\n";                // Warnings
        RF += "-W\n";
        RF += "-msoft-float\n";
//        RF += "-msingle-float\n";
        RF += "-finline\n";             // Pay attention to the inline keyword
        RF += "-finline-functions\n";   // Auto inline small functions
        RF += "-fno-exceptions\n";      // Disable exceptions
    }
    else
    {
        // Add some standard compile options.
        RF += "-G0\n";                  // No data in sbss segment; causes problems
        RF += "-fno-common\n";          // Put un-init data in BSS
//        RF += "-finline\n";             // Pay attention to the inline keyword
//        RF += "-finline-functions\n";   // Auto inline small functions

        RF += "-fno-exceptions\n";      // Disable exceptions
        RF += "-fno-rtti\n";            // Disable rtti

        RF += "-mvu0-use-vf0-vf31\n";   // Enable vu0 register allocations
        RF += "-fopt-stack\n";

//        RF += "-fstrict-aliasing\n";
        RF += "-ffast-math\n";

        RF += "-fdefault-single-float\n";
        RF += "-fforce-link-once\n";

        // Warnings.
        RF += "-Wall\n";                // "All" warnings
        RF += "-Wno-switch\n";          // Allow no default case
        RF += "-Werror\n";              // Warnings as errors
        RF += "-fopt-stabs\n"; 

// For MOH only.
#if defined( LAZY_WARNINGS )
        RF += "-Wno-unused\n";           // Allow unused vars
        RF += "-Wno-multichar\n"; 
        RF += "-mvu0-use-vf0-vf31\n";
        RF += "-fno-strict-aliasing\n"; 
        RF += "-ffast-math\n";
        RF += "-fshort-wchar\n"; 
#else
        RF += "-W\n";                   // More warnings
#endif

        // Allow friend functions to template classes.
//      fprintf( pNewRFile, " -Wno-non-template-friend  \n" );   
    }

    // Add compile optimization specification.
    {
        char* List[] = { "0", "1", "2", "2" };
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
        RF += xfs( "-O%s\n", pOpt );

        if( pOpt[0] != '0' )
        {
            RF += "-fno-default-inline\n";
            RF += "-finline-limit-50\n";
        }
    }

    // Add current directory as an include directory.
    RF += "-I.\n";
   
    // Add the include paths.
    for( i = 0; i < C.m_IncludeDir.GetCount(); i++ )
        RF += xfs( "-I\"%s\"\n", C.m_IncludeDir[i] );

    // Normally we would add the system includes here with code like that shown
    // below.  But, the system includes are handled by the SN.INI file.
    //{
    //  char Path[] = "3rdParty/PS2/Sony";
    //  fprintf( pNewRFile, " -I\"%s/%s/include\" \n", pBase, Path );
    //}

    // Add the defines to the response file.
    for( i = 0; i < C.m_Define.GetCount(); i++ )
        RF += xfs( "-D%s \n", C.m_Define[i] );

    // Add the XCORE_PATH macro.
    RF += xfs( "-D\"XCORE_PATH=\\\"%s\\\"\"", UseOnlySlashes(pBase) );

    // Add the defines to the message for the user.
    strcat( Message, "\nxCL -- Defines[ " );
    for( i = 0; i < C.m_Define.GetCount(); i++ )
    {
        if( i != 0 )  strcat( Message, ", " );
        strcat( Message, C.m_Define[i] );
    }
    strcat( Message, " ]" );

    // That's all for the new response file.
    if( !RF.SaveFile( g_NewRName ) )
    {
        printf( "xCL -- ERROR: Unable to open SN response file: %s\n", g_NewRName );
        return( EXIT_FAILURE );
    }

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
    fprintf( pVC6RFile, " /c /nologo /W3 /FD \n" );

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
    if( C.m_SBRPath[0] )      fprintf( pVC6RFile, " /FR\"%s\" \n", C.m_SBRPath );
    if( C.m_PDBPath[0] )      fprintf( pVC6RFile, " /Fd\"%s\" \n", C.m_PDBPath );
    if( C.m_Preprocess )      fprintf( pVC6RFile, " /P /C     \n" );

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

    // Add the PS2 and SN system includes.
    if( C.m_Target == BUILD_TARGET_PS2_IOP )
    {
        // TARGET_PS2_IOP

        char Path[] = "3rdParty/PS2/Sony";

        fprintf( pVC6RFile, " /I \"%s/%s/IOP/include\" \n", pBase, Path );
        fprintf( pVC6RFile, " /I \"%s/%s/include\"     \n", pBase, Path );
    }
    else
    {
        // TARGET_PS2 (EE)

        char Path[] = "3rdParty/PS2";

        fprintf( pVC6RFile, " /I \"%s/%s/Sony/EE/include\" \n", pBase, Path );
        fprintf( pVC6RFile, " /I \"%s/%s/Sony/include\"    \n", pBase, Path );
        fprintf( pVC6RFile, " /I \"%s/%s/SN/EE/include\"   \n", pBase, Path );
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
    fprintf( pVC6RFile, " /D \"XCORE_PATH=\\\"%s\\\"\"", UseOnlySlashes(pBase) );

    // That's it.
    fclose( pVC6RFile );

    //--------------------------------------------------------------------------
    // Dependency Build & MD5 generation for caching
    //--------------------------------------------------------------------------

    if( g_DistributedBuild )
    {
        // TODO: Add include paths
        dep_AddIncludePath( xstring(".") );
        for( i = 0; i < C.m_IncludeDir.GetCount(); i++ )
            dep_AddIncludePath( xstring(C.m_IncludeDir[i]) );
        if( C.m_Target == BUILD_TARGET_PS2_IOP )
        {
            // TARGET_PS2_IOP
            char Path[] = "3rdParty/PS2/Sony";
            xstring s;
            s.Format( "%s/%s/IOP/include", pBase, Path ); dep_AddIncludePath( s );
            s.Format( "%s/%s/include", pBase, Path ); dep_AddIncludePath( s );
        }
        else
        {
            // TARGET_PS2 (EE)
            char Path[] = "3rdParty/PS2";
            xstring s;
            s.Format( "%s/%s/Sony/EE/include", pBase, Path ); dep_AddIncludePath( s );
            s.Format( "%s/%s/Sony/include", pBase, Path ); dep_AddIncludePath( s );
            s.Format( "%s/%s/SN/EE/include", pBase, Path ); dep_AddIncludePath( s );
        }

        // Get source files into an array of structs we can work on and run dependency check on them
        for( i=0 ; i<C.m_SourceCode.GetCount() ; i++ )
        {
            source_file& Source = g_SourceFiles.Append();
            Source.m_PathName = C.m_SourceCode[i];
            Source.m_Cached   = false;

            Source.m_NodeIndex = dep_ProcessFullyQualifiedFile( Source.m_PathName, Source.m_PathName );
            if( Source.m_NodeIndex != -1 )
            {
                // Get MD5 for source file including all dependencies
                MD5Context md5;
                MD5Init( &md5 );
                MD5Update( &md5, (const u8*)&RF[0], RF.GetLength() );
                MD5Update( &md5, g_DepNodes[Source.m_NodeIndex].m_MD5Digest, 16 );
                for( s32 j=0 ; j<g_DepNodes[Source.m_NodeIndex].m_Includes.GetCount() ; j++ )
                {
                    MD5Update( &md5, g_DepNodes[g_DepNodes[Source.m_NodeIndex].m_Includes[j]].m_MD5Digest, 16 );
                }

                // TODO: Factor parts of the command line into the MD5, like defines and options (not list of source files or includes)

                MD5Final( &md5, Source.m_MD5Digest );

                // Check if this file is available in the cache
                xstring CacheName = "C:\\tmp\\cache\\";
                CacheName += MD5ToString( Source.m_MD5Digest );
                X_FILE* pFile = x_fopen( CacheName, "rb" );
                if( pFile )
                {
                    x_fclose( pFile );

                    // Get object filename
                    char FName[ _MAX_FNAME ];
                    _splitpath( g_SourceFiles[i].m_PathName, NULL, NULL, FName, NULL );
                    xstring ObjectName;
                    ObjectName.Format( "%s\\%s.obj", C.m_OutputPath, FName );

                    // Copy from cache
                    if( CopyFile( CacheName, ObjectName, FALSE ) )
                    {
                        printf( "Cached: %s\n", Source.m_PathName );
                        Source.m_Cached = true;
                    }
                    else
                    {
                        printf( "Copy Failed from Cached: %s\n", Source.m_PathName );
                    }
                }
                else
                {
                    printf( "Not Cached: %s\n", Source.m_PathName );
                }
            }
            else
            {
                printf( "Source not found for: %s\n", Source.m_PathName );
            }
        }
    }

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
            sprintf( VC6CmdLn, "cl @\"%s\" \"%s\"", VC6RName, C.m_SourceCode[i] );

            // Extra verbage?
            if( g_Verbose || g_EchoCmdLine )
                DumpCommandLineAndResponseFile( "VC6 Compile",
                                                VC6CmdLn,
                                                VC6RName );

            // Execute command for VC6 to compute dependencies.
            //system( VC6CmdLn );
            ExecuteCMD( VC6CmdLn );

            // Delete .i file from the VC preprocessor.
            if( !C.m_Preprocess )
            {
                sprintf( VC6CmdLn, "%s.i", SrcFName );
                DeleteFile( VC6CmdLn );
            }
        }

        //------//
        //  SN  //
        //------//
        {
            sprintf( NewCmdLn, "ps2cc.exe %s @\"%s\" \"%s\" -o \"%s\\%s.obj\"", 
                                (C.m_Target == TARGET_PS2_IOP) ? "-iop" : "",
                                g_NewRName, 
                                C.m_SourceCode[i],
                                C.m_OutputPath,
                                SrcFName );

            // Extra verbage?
            if( g_Verbose || g_EchoCmdLine )
                DumpCommandLineAndResponseFile( "PS2/SN Compile",
                                                NewCmdLn, 
                                                g_NewRName );

            // Execute command for SN compiler.
            //Result |= system( NewCmdLn );
            Result |= ExecuteCMD( NewCmdLn );
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
        // Skip if this file was cached
        if( g_DistributedBuild && g_SourceFiles[i].m_Cached )
            continue;

        char SrcFName[ _MAX_FNAME ];

        _splitpath( C.m_SourceCode[i], NULL, NULL, SrcFName, NULL );

        //-------//
        //  VC6  //
        //-------//
        {
            sprintf( VC6CmdLn, "cl @\"%s\" \"%s\"", VC6RName, C.m_SourceCode[i] );

            // Extra verbage?
            if( g_Verbose || g_EchoCmdLine )
                DumpCommandLineAndResponseFile( "VC6 Compile",
                                                VC6CmdLn,
                                                VC6RName );

            // Execute command for VC6 to compute dependencies.
            ExecuteCMD( VC6CmdLn );

            // Delete .i file from the VC preprocessor.
            if( !C.m_Preprocess )
            {
                sprintf( VC6CmdLn, "%s.i", SrcFName );
                DeleteFile( VC6CmdLn );
            }
        }
    }

    // Wait for all compile threads to complete.
    WaitForMultipleObjects( MAX_THREADS, &g_hEvent[0], TRUE, INFINITE );

    // Delete events.
    for( i=0; i<MAX_THREADS; i++ )
    {
        CloseHandle( g_hEvent[i] );
    }

    //--------------------------------------------------------------------------
    //  Add compiled files to cache
    //--------------------------------------------------------------------------

    if( g_DistributedBuild )
    {
        for( i=0 ; i<g_SourceFiles.GetCount() ; i++ )
        {
            // Get object filename
            char FName[ _MAX_FNAME ];
            _splitpath( g_SourceFiles[i].m_PathName, NULL, NULL, FName, NULL );
            xstring ObjectName;
            ObjectName.Format( "%s\\%s.obj", C.m_OutputPath, FName );

            // Does the file exist
            X_FILE* pFile = x_fopen( ObjectName, "rb" );
            if( pFile )
            {
                x_fclose( pFile );

                xstring CacheName( "C:\\tmp\\cache\\" );
                CacheName += MD5ToString( g_SourceFiles[i].m_MD5Digest );

                // Copy to the cache
                CopyFile( ObjectName, CacheName, FALSE );
            }
        }
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
    int     iFile   = 0;
    char    NewCmdLn[ 32784 ];      // New command line to execute.

    // Loop until all files are compiled.
    while( iFile < C.m_SourceCode.GetCount() )
    {
        // Allocate a file to compile.
        iFile = InterlockedIncrement( &g_iFile );

        // Valid file number? and not cached
        if( (iFile >= 0) && (iFile < C.m_SourceCode.GetCount()) && (!g_DistributedBuild || !g_SourceFiles[iFile].m_Cached) )
        {
            char SrcFName[ _MAX_FNAME ];

            // Get source filename.
            _splitpath( C.m_SourceCode[iFile], NULL, NULL, SrcFName, NULL );

            // Build command line.
            sprintf( NewCmdLn, "ps2cc.exe %s @\"%s\" \"%s\" -o \"%s\\%s.obj\"", 
                                (C.m_Target == BUILD_TARGET_PS2_IOP) ? "-iop" : "",
                                g_NewRName, 
                                C.m_SourceCode[iFile],
                                C.m_OutputPath,
                                SrcFName );

            // Extra verbage?
            if( g_Verbose || g_EchoCmdLine )
                DumpCommandLineAndResponseFile( "PS2/SN Compile",
                                                NewCmdLn, 
                                                g_NewRName );

            // Execute command for SN compiler.
//            g_CompileResult |= system( NewCmdLn );
            g_CompileResult |= ExecuteCMD( NewCmdLn );
        }
    }

    // Signal thread completion event.
    SetEvent( hEvent );

    // Exit
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
    sprintf( CmdLine, "ps2cc -g -xassembler-with-cpp -I." );

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
//  sprintf( Buffer, " -D\"XCORE_PATH=\\\"%s\\\"\"", UseOnlySlashes(pBase) );
//  strcat ( CmdLine, Buffer );

    // Add the file names.
    sprintf( Buffer, " -c \"%s\" -o \"%s\\%s.obj\"",
                      C.m_SourceOther[i],
                      C.m_OutputPath,
                      FName );
    strcat ( CmdLine, Buffer );

    // Verbose?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "PS2/SN Assemble",
                                        CmdLine,
                                        NULL );
    }

    // Feedback for the user.
    printf( "%s%s\n", FName, Ext );
    fflush( stdout );

    // Execute command for SN assembler.
    //Result = system( CmdLine );
    Result = ExecuteCMD( CmdLine );
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
        sprintf( CmdLine, "cl /nologo /TC /P /I ." );

        // Add the include paths.
        for( j = 0; j < C.m_IncludeDir.GetCount(); j++ )
        {
            sprintf( Buffer, " /I \"%s\"", C.m_IncludeDir[j] );
            strcat ( CmdLine, Buffer );
        }

        // Add the file name.
        sprintf( Buffer, " \"%s\"", C.m_SourceOther[i] );
        strcat ( CmdLine, Buffer );

        // Verbose?
        if( g_Verbose || g_EchoCmdLine )
        {
            DumpCommandLineAndResponseFile( "PS2/SN VU Preprocess",
                                            CmdLine,
                                            NULL );
        }

        // Execute command for preprocess.
        //Result = system( CmdLine );
        Result = ExecuteCMD( CmdLine );
    }

    // Move the .i file to the build output directory.
    sprintf( CmdLine, "%s.i", FName );
    sprintf( Buffer, "%s/%s.i", C.m_OutputPath, FName );
    DeleteFile( Buffer );
    BOOL Moved = MoveFile( CmdLine, Buffer );
    if( !Moved )
        printf( "Error moving file \"%s\" to \"%s\"\n", CmdLine, Buffer );

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
            DumpCommandLineAndResponseFile( "PS2/SN VU Assemble",
                                            CmdLine,
                                            NULL );
        }

        // Execute command for SN/VU assembler.
        //Result = system( CmdLine );
        Result = ExecuteCMD( CmdLine );
    }

    // That's it.
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
// Creates a relocatable module for the PS2. All function calls must be internal
// to the module as no symbols are allowed to be imported at the moment.
static 
int LinkerDLL( void )
{
    int     i;
    int     Result;
    char    Name        [ _MAX_PATH  ];
    char    Map         [ _MAX_PATH  ];
    char    Exports     [ _MAX_PATH  ];
    char    LinkerScript[ _MAX_PATH  ]; 
    char    Drive       [ _MAX_DRIVE ];
    char    Dir         [ _MAX_DIR   ];
    char    FName       [ _MAX_FNAME ];
    char    Ext         [ _MAX_EXT   ];
    char    CmdLine     [ 32784 ];
    FILE*   pLRF    = NULL;
    FILE*   pExports;

    // Determine response file name and map name.
    _splitpath( C.m_OutputFile, Drive, Dir, FName,              Ext    );
    _makepath ( Name,           Drive, Dir, "_PS2_SN_DLL_Link", ".txt" );
    _makepath ( Map,            Drive, Dir, FName,          ".map.txt" );

    strcpy(Exports, "symbols.export" );

    // Build the command line.
    // The command line syntax is ps2dlllk <script> <linker commandline>
    sprintf( CmdLine, "ps2dlllk \"%s\" ps2cc @\"%s\" -o \"%s\" ", Exports, Name, C.m_OutputFile );

    pExports = fopen( Exports, "rb" );
    if( pExports == NULL )
    {
        printf( "xCL -- WARNING: Unable to open exported symbol file '%s'. Will export ALL symbols.\n", Exports );
        _makepath ( Exports,          Drive, Dir, FName,          ".export" );
        pExports = fopen( Exports, "wt" );
        if( pExports == NULL )
        {
            printf( "xCL -- ERROR: Unable to open file for write: %s\n", Exports );
            return( 0 );
        }
    }

    fclose(pExports);

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
    fprintf( pLRF, "-nostdlib\n");
    //fprintf( pLRF, "-Wl,-strip-unused,-sn-full-map\n");
    fprintf( pLRF, "-Wl,-sn-full-map\n");

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
        fprintf( pLRF, " \"%s\"\n", C.m_ObjectFile[i] );

    // Add library paths.
    for( i = 0; i < C.m_LibraryDir.GetCount(); i++ )                       
        fprintf( pLRF, " -l \"%s\"\n", C.m_LibraryDir[i] );

    fprintf( pLRF, " --start-group\n" );

    // Add standard library paths.
    fprintf( pLRF, " -L \"%s\\3rdParty\\PS2\\Sony\\EE\\lib\" \n", pBase );
    fprintf( pLRF, " -L \"%s\\3rdParty\\PS2\\SN\\EE\\lib\"   \n", pBase );

    // Add all libraries.
    for( i = 0; i < C.m_LibraryFile.GetCount(); i++ )
    {
        fprintf( pLRF, " \"%s\"\n", C.m_LibraryFile[i] );
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
            strcpy(LinkerScript,"PS2-Debug.ld");
        }
        else
        {
            strcpy(LinkerScript,"PS2-Release.ld");
        }
        handle = fopen(LinkerScript,"rb");
        if (handle)
        {
            fclose(handle);
        }
        else
        {
            // if we can't find any other, just use the default
            strcpy(LinkerScript,"ps2.ld");
        }
    }

    // Add the remaining information.
    fprintf( pLRF, " --end-group -T \"%s\" \n", LinkerScript );
    fprintf( pLRF, " -Wl,-Map \"%s\" \n", Map );



    // Close the response file.
    fclose( pLRF );
    // For some reason the new linker does not automatically create the
    // map file if it doesn't exist so we create a dummy one anyway.
    pLRF = fopen(Map,"rb");
    if (!pLRF)
    {
        pLRF = fopen(Map,"wb");
        if (pLRF)
        {
            fclose(pLRF);
        }
    }
    else
    {
        fclose(pLRF);
    }
    //
    // Done with response file.
    //

    // Extra verbage?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "PS2/SN DLL Link",
                                        CmdLine,
                                        Name );
    }

    // A little more standard verbage...
    printf( "%s%s\n", FName, Ext );
    fflush( stdout );

    // Execute the command line.
    Result = system( CmdLine );

    // If that worked, now run ps2dllcheck to make sure that we have no undefined symbols.

    if( Result == 0 )
    {
        _makepath ( Name,           Drive, Dir, FName, ".rel" );

        sprintf( CmdLine, "ps2dllcheck \"%s\" ", Name );
        Result = system( CmdLine );
        if( Result != 0 )
        {
            // If we had an error, delete the output file which will force it to be rebuilt next time
            DeleteFile( C.m_OutputFile );
        }
    }

    // We're done.
    return( Result );
}

//==============================================================================

static 
int LinkerIOP( void )
{
    int     i;
    int     Result;
    char    Name    [ _MAX_PATH  ];
    char    Drive   [ _MAX_DRIVE ];
    char    Dir     [ _MAX_DIR   ];
    char    FName   [ _MAX_FNAME ];
    char    Ext     [ _MAX_EXT   ];
    char    CmdLine [ 32784 ];
    FILE*   pLRF    = NULL;

    // Determine response file name and map name.
    _splitpath( C.m_OutputFile, Drive, Dir, FName,              Ext    );
    _makepath ( Name,           Drive, Dir, "_PS2_SN_IOP_Link", ".txt" );

    // Build the command line.
    sprintf( CmdLine, "ps2cc @\"%s\"", Name );

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

    // Verbose output?
    if( g_Verbose || g_EchoCmdLine )
    {
        fprintf( pLRF, " -v \n" );
    }

    // Add standard switches.
    fprintf( pLRF, " -iop -fdevstudio \n" );

    // Add output file name.
    if (g_StripSymbols)
    {
        fprintf( pLRF, " -o \"%s\" \n", C.m_OutputFile );
    }
    else
    {
        fprintf( pLRF, " -r \"%s\" \n", C.m_OutputFile );
    }

    // Add any custom link options.
    if( C.m_LinkOptions[0] != '\0' )
        fprintf( pLRF, " %s\n", C.m_LinkOptions );

    // Add all objects.
    for( i = 0; i < C.m_ObjectFile.GetCount(); i++ )                       
        fprintf( pLRF, " \"%s\"\n", C.m_ObjectFile[i] );

    // Add all libraries.
    for( i = 0; i < C.m_LibraryFile.GetCount(); i++ )
        fprintf( pLRF, " \"%s\"\n", C.m_LibraryFile[i] );

    // Add library paths.
    for( i = 0; i < C.m_LibraryDir.GetCount(); i++ )                       
        fprintf( pLRF, " -L \"%s\"\n", C.m_LibraryDir[i] );

    // Add standard library paths.
    fprintf( pLRF, " -L \"%s/3rdParty/PS2/Sony/IOP/lib\" \n", pBase );
    fprintf( pLRF, " -L \"%s/3rdParty/PS2/SN/IOP/bin\" \n", pBase );  // This is needed for a path to nm.exe
    fprintf( pLRF, " -L \"%s/3rdParty/PS2/SN/IOP/lib\"   \n", pBase );

    // Add standard libraries.
    //fprintf( pLRF, " -ilb=cdvdman.ilb \n" );
    //fprintf( pLRF, " -ilb=iop.ilb \n" );
    //fprintf( pLRF, " -ilb=libsd.ilb \n" );

    fprintf( pLRF, " \"%s/3rdparty/ps2/sn/iop/lib/libsniop.a\"\n", pBase );

    // Close the response file.
    fclose( pLRF );

    //
    // Done with response file.
    //

    // Extra verbage?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "PS2/SN IOP Linker",
                                        CmdLine,
                                        Name );
    }

    // A little more standard verbage...
    printf( "%s%s\n", FName, Ext );
    fflush( stdout );

    // Execute the command line.
    //Result = system( CmdLine );
    Result = ExecuteCMD( CmdLine );

    // We're done.
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

    // Watch out for IOP targets.
    if( C.m_Target == BUILD_TARGET_PS2_IOP )
        return( LinkerIOP() );

    if( C.m_Target == BUILD_TARGET_PS2_DLL )
    {
        return( LinkerDLL() );
    }

    // Determine response file name and map name.
    _splitpath( C.m_OutputFile, Drive, Dir, FName,          Ext        );
    _makepath ( Name,           Drive, Dir, "_PS2_SN_Link", ".txt"     );
    _makepath ( Map,            Drive, Dir, FName,          ".map.txt" );

    // Build the command line.
    sprintf( CmdLine, "ps2cc.exe @\"%s\"", Name );

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
    fprintf( pLRF, "-nostdlib\n");
    fprintf( pLRF, "-Wl,-strip-unused,-sn-full-map\n");

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
        fprintf( pLRF, " \"%s\"\n", C.m_ObjectFile[i] );

    // Add library paths.
    for( i = 0; i < C.m_LibraryDir.GetCount(); i++ )                       
        fprintf( pLRF, " -l \"%s\"\n", C.m_LibraryDir[i] );

    fprintf( pLRF, " --start-group\n" );

    // Add standard library paths.
    fprintf( pLRF, " -L \"%s\\3rdParty\\PS2\\Sony\\EE\\lib\" \n", pBase );
    fprintf( pLRF, " -L \"%s\\3rdParty\\PS2\\SN\\EE\\lib\"   \n", pBase );

    // Add all libraries.
    for( i = 0; i < C.m_LibraryFile.GetCount(); i++ )
    {
        fprintf( pLRF, " \"%s\"\n", C.m_LibraryFile[i] );
    }

    // Add standard libraries.
    fprintf( pLRF, " libsn.lib    \n" );
    fprintf( pLRF, " libc.lib     \n" );
    fprintf( pLRF, " libcdvd.lib  \n" );
    fprintf( pLRF, " libdev.lib   \n" );  
    fprintf( pLRF, " libdma.lib   \n" );  
    fprintf( pLRF, " libgcc.lib   \n" );
    fprintf( pLRF, " libgraph.lib \n" );
    fprintf( pLRF, " libipu.lib   \n" );
    fprintf( pLRF, " libkernl.lib \n" );
    fprintf( pLRF, " liblout.lib  \n" );
    fprintf( pLRF, " libm.lib     \n" );
    fprintf( pLRF, " libmc.lib    \n" );
    fprintf( pLRF, " libmpeg.lib  \n" );
    fprintf( pLRF, " libmsin.lib  \n" );
    fprintf( pLRF, " libpad.lib   \n" );
    fprintf( pLRF, " libpc.lib    \n" );
    fprintf( pLRF, " libpkt.lib   \n" );  
    fprintf( pLRF, " libsdr.lib   \n" );
    fprintf( pLRF, " libssyn.lib  \n" );
    fprintf( pLRF, " libvu0.lib   \n" );
    fprintf( pLRF, " libexcep.lib \n" );

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
            strcpy(LinkerScript,"PS2-Debug.ld");
        }
        else
        {
            strcpy(LinkerScript,"PS2-Release.ld");
        }
        handle = fopen(LinkerScript,"rb");
        if (handle)
        {
            fclose(handle);
        }
        else
        {
            // if we can't find any other, just use the default
            strcpy(LinkerScript,"ps2.ld");
        }
    }

    // Add the remaining information.
    fprintf( pLRF, " --end-group -T \"%s\" \n", LinkerScript );
    fprintf( pLRF, " -o \"%s\" -Wl,-Map \"%s\" \n", C.m_OutputFile, Map );

    // Close the response file.
    fclose( pLRF );
    // For some reason the new linker does not automatically create the
    // map file if it doesn't exist so we create a dummy one anyway.
    pLRF = fopen(Map,"rb");
    if (!pLRF)
    {
        pLRF = fopen(Map,"wb");
        if (pLRF)
        {
            fclose(pLRF);
        }
    }
    else
    {
        fclose(pLRF);
    }

    //
    // Done with response file.
    //

    // Extra verbage?
    if( g_Verbose || g_EchoCmdLine )
    {
        DumpCommandLineAndResponseFile( "PS2/SN Link",
                                        CmdLine,
                                        Name );
    }

    // A little more standard verbage...
    printf( "%s%s\n", FName, Ext );
    fflush( stdout );

    // Execute the command line.
    //Result = system( CmdLine );
    Result = ExecuteCMD( CmdLine );

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
    DeleteFile( C.m_OutputFile );

    // Determine response file name and map name.
    _splitpath( C.m_OutputFile, Drive, Dir, FName,         Ext    );
    _makepath ( Name,           Drive, Dir, "_PS2_SN_Lib", ".txt" );

    // Build the command line.
    if (C.m_Target == BUILD_TARGET_PS2_IOP)
    {
        sprintf( CmdLine, "iop-ar -M < \"%s\"", Name );
    }
    else
    {
        sprintf( CmdLine, "ee-ar -M < \"%s\"", Name );
    }


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
        DumpCommandLineAndResponseFile( "PS2/SN Librarian",
                                        CmdLine,
                                        Name );
    }

    // A little more standard verbage...
    printf( "%s%s\n", FName, Ext );
    fflush( stdout );

    // Execute the command line.
    //Result = system( CmdLine );
    Result = ExecuteCMD( CmdLine );

    // We're done.
    return( Result );
}

//==============================================================================

int Process_PS2_SN( void )
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
        printf( "xCL -- ERROR: Unable to determine tool for PS2/SN operation (%d).\n", C.m_Tool );
        break;
    }

    // We're done.
    return( Result );
}

//==============================================================================
