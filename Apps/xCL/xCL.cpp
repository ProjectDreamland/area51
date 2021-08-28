//==============================================================================
//
//  xCL - Cross Platform Compile and Link
//
//==============================================================================

//==============================================================================
//  INCLUDES    
//==============================================================================

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "xCL.hpp"

#include "xCL_Thru.hpp"
#include "xCL_CodeWarrior.hpp"
#include "xCL_PS2_SN.hpp"
#include "xCL_GCN_SN.hpp"

//==============================================================================
//  GLOBAL VARIABLES
//==============================================================================

cmd_line_data   g_CmdLineData;

bool            g_Verbose           = false;
bool            g_ShowTimes         = false;
bool            g_EchoCmdLine       = false;
bool            g_StripSymbols      = false;
bool            g_DistributedBuild  = false;

//==============================================================================
//  PUBLIC FUNCTIONS
//==============================================================================

//==============================================================================
//  OpenSection
//==============================================================================
//  I've been reading the verbose output so much, that I was ready for a little
//  change.  So I jazzed it up a bit.  This should make it easier to read, too.
//  I'm making the "horizontal lines" 75 characters long so they are nice and
//  wide, but they should still fit in a plain text email message.
//
/*

         1    1    2    2    3    3    4    4    5    5    6    6    7    7    8
....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0

   ____
__/    \___________________________________________________________________
===========================================================================

   _____
__/  A  \__________________________________________________________________
===========================================================================

   __________________  
__/  This is a test  \_____________________________________________________
===========================================================================

   ______________________________________________________________________
__/  Command Line and Response file for: VC6 Compile  yadda yadda yadda  \_
===========================================================================

   ________________________________________________________________________  
__/  Command Line and Response file for: VC6 Compile yadda yadda yadda yadda yadda yadda 
===========================================================================

aaaaaaaabbbbbbbbbbbbb
aaaaaTHIS IS A TESTbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
333333333333333333333333333333333333333333333333333333333333333333333333333
*///============================================================================

void OpenSection( char* pTitle )
{
    int  w;
    int  Len = strlen( pTitle );
    char L1a[] = "   ____";
    char L1b[] = "____________________________________________________________________";
    char L2a[] = "__/  ";
    char L2b[] = "  \\___________________________________________________________________";
    char L3 [] = "===========================================================================";
                  
    w = ((Len > 68) ? 68 : Len);
    L1b[   w] = '\0';
    L2b[68-w] = '\0';

    printf( ".\n" );
    printf( "%s%s\n", L1a, L1b );
    printf( "%s%s%s\n", L2a, pTitle, L2b );
    printf( "%s\n", L3 );
    fflush( stdout );
}

//==============================================================================

void DivideSection( void )
{
    printf( "---------------------------------------------------------------------------\n" );
    fflush( stdout );
}

//==============================================================================

void CloseSection( void )
{
    printf( "===========================================================================\n" );
    printf( ".\n" );
    fflush( stdout );
}

//==============================================================================

void DumpTextFileContent( char* pFileName )
{
    char  Buffer[ 8192 ];
    FILE* pFile = fopen( pFileName, "rt" );

	if( pFile )
	{
        bool NL;
	    while( fgets( Buffer, 8192, pFile ) != NULL )
        {
            NL = ( Buffer[ strlen(Buffer) ] == '\n' );
	        printf( "%s", Buffer );
        }
	    fclose( pFile );
        if( !NL )
            printf( "\n" );
	}
	else
	{
        printf( "<<< Failed to open file '%s' for read. >>>\n", pFileName );
	}        
}

//==============================================================================

void DumpCommandLineAndResponseFile( char* pTitle,
                                     char* pCmdLine,
                                     char* pResponseFileName )
{
    char* pBegin;
    char* pEnd;
    char  FileName[ MAX_PATH ];

    OpenSection( pTitle );
    printf( "%s\n", pCmdLine );

    if( pResponseFileName )
    {
        DivideSection();
        DumpTextFileContent( pResponseFileName );
    }
    else
    {
        //
        // Search for response file arguments.
        //

        pBegin = pCmdLine;

        while( *pBegin )
        {
            // Clear out the file name.
            memset( FileName, 0, MAX_PATH );
            
            // Advance until we have non-white space.
            while( *pBegin && isspace( *pBegin ) )
                pBegin++;

            // Make sure we haven't run off the end.
            if( *pBegin == '\0' )
                break;

            // Now, see if we have an '@' symbol.
            if( *pBegin == '@' )
            {
                // Find the end of this argument.
                pEnd = pBegin;
                while( *pEnd && !isspace( *pEnd ) )
                    pEnd++;

                // Extract the file name.
                // Careful about quotes.
                if( (pBegin[1] == '"') && (pEnd[-1] == '"') && (((pBegin-pEnd)-3) > 0) )
                {
                    strncpy( FileName, pBegin+2, (pEnd-pBegin)-3 );
                }
                else
                {   
                    strncpy( FileName, pBegin+1, (pEnd-pBegin)-1 );
                }

                // We have the file name.  Display its content.
                DivideSection();
                DumpTextFileContent( FileName );

                // And move on.
                pBegin = pEnd;
            }
            else
            {
                // No '@'.  Advance to next white space.
                while( *pBegin && !isspace( *pBegin ) )
                    pBegin++;
            }           
        }
    }

    CloseSection();
}

//==============================================================================

char* UseOnlyBackSlashes( char* pPath )
{
    char* p = pPath;
    while( *p )
    {
        if( *p == '/' )
            *p = '\\';
        p++;
    }
    return( pPath );
}

//==============================================================================

char* UseOnlySlashes( char* pPath )
{
    char* p = pPath;
    while( *p )
    {
        if( *p == '\\' )
            *p = '/';
        p++;
    }
    return( pPath );
}

//==============================================================================
//  PRIVATE FUNCTIONS
//==============================================================================

static
void EchoCommandLine( int argc, char** argv )
{
    int i;

    //
    // Echo command line to standard out.
    //

    OpenSection( "Command Line Echo" );
    for( i = 0; i < argc; i++ )
    {
        printf( "[%03d]  %s\n", i, argv[i] );
    }    
    CloseSection();

    // Look for anything that looks like a response file.  Echo it's content.

    for( i = 1; i < argc; i++ )
    {
        char Label[ _MAX_PATH ];

        if( argv[i][0] != '@' )
            continue;

        sprintf( Label, "Response file: %s", argv[i]+1 );
        OpenSection( Label );
        DumpTextFileContent( argv[i]+1 );
        CloseSection();
    }
}

//==============================================================================

static
void ReadOptionsFromRegistry( void )
{
    HKEY  Key = 0;
    DWORD Value;
    DWORD Size;

    RegOpenKeyEx( HKEY_CURRENT_USER,
                  "software\\Inevitable\\xTB", 
                  0, 
                  KEY_READ,
                  &Key );

    if( RegQueryValueEx( Key, "Verbose", 
                         0, NULL, (unsigned char*)(&Value), &Size ) == ERROR_SUCCESS )
    {
        if( Value == 1 )  
            g_Verbose = true;
    }

    if( RegQueryValueEx( Key, "CommandLineEcho", 
                         0, NULL, (unsigned char*)(&Value), &Size ) == ERROR_SUCCESS )
    {
        if( Value == 1 )  
            g_EchoCmdLine = true;
    }

    if( RegQueryValueEx( Key, "ShowCompileTimes", 
                         0, NULL, (unsigned char*)(&Value), &Size ) == ERROR_SUCCESS )
    {
        if( Value == 1 )  
            g_ShowTimes = true;
    }

    if( RegQueryValueEx( Key, "StripSymbols", 
                         0, NULL, (unsigned char*)(&Value), &Size ) == ERROR_SUCCESS )
    {
        if( Value == 1 )  
            g_StripSymbols = true;
    }

    if( RegQueryValueEx( Key, "EnableDistributedBuild", 0, NULL, (unsigned char*)(&Value), &Size ) == ERROR_SUCCESS )
	{
	    if( Value == 1 )  
    	    g_DistributedBuild = true;
	}
}

//==============================================================================

int main( int argc, char** argv )
{   
    int     Result  = EXIT_FAILURE;
    bool    Unknown = false;    
    char*   pTarget = NULL;

    ReadOptionsFromRegistry();

    // Verbose output?
    if( g_Verbose )
    {
        printf( "\n.\n" );
        printf( "***************************************************************************\n" );
        printf( "xCL -- VERBOSE MODE ACTIVATED\n" );
        printf( "***************************************************************************\n" );
    }

    // Echo command line?
    if( g_EchoCmdLine )
    {
        printf( "\n.\nxCL -- COMMAND LINE ECHO MODE ACTIVATED\n" );
        EchoCommandLine( argc, argv );
    }

    // Show the start time?
    if( g_ShowTimes )
    {
       time_t ltime;
       time( &ltime );
       printf( "\\\\\\\n >>> BEGIN %s///\n", ctime( &ltime ) );
    }

    // Absorb the command line into our data structure.
    g_CmdLineData.Activate( argc, argv );

    // Add the current username to the defines list.
    {
        char* pUserName = getenv( "USERNAME" );
        if( pUserName )
            g_CmdLineData.m_Define.Add( pUserName );
    }

    //
    // Based on the target and vendor found in the command line data, branch 
    // off to the appropriate processing function.
    //

    switch( g_CmdLineData.m_Target )
    {
    //--------------------------------------------------------------------------
    case BUILD_TARGET_UNKNOWN:  Result = PassThru();
                                break;

    //--------------------------------------------------------------------------
    case BUILD_TARGET_PC:       switch( g_CmdLineData.m_Vendor )
                                {                                                     
                                case VENDOR_MS:     Result = PassThru();
                                                    break;

                                case VENDOR_MW:     Result = Process_PC_CodeWarrior();
                                                    break;

                                default:                
                                    // We got BUILD_TARGET_PC, but no known vendor.  
                                    // Assume it is VENDOR_MS.
                                    g_CmdLineData.m_Vendor = VENDOR_MS;
                                    g_CmdLineData.m_Define.Add( "VENDOR_MS" );
                                    strcpy( g_CmdLineData.m_VendorString, "VENDOR_MS" );
                                    Result = PassThru();
                                    break;                            
                                }   
                                break;

    //--------------------------------------------------------------------------
    case BUILD_TARGET_XBOX:     switch( g_CmdLineData.m_Vendor )
                                {                                                     
                                case VENDOR_MS:     Result = PassThru();
                                                    break;
                                                        
                                default:            Unknown = true;
                                                    break;
                                }
                                break;

    //--------------------------------------------------------------------------
    case BUILD_TARGET_PS2_DLL:
    case BUILD_TARGET_PS2:      switch( g_CmdLineData.m_Vendor )
                                {                                                     
                                case VENDOR_SN:     Result = Process_PS2_SN();
                                                    break;
                                                        
                                case VENDOR_MW:     Result = Process_PS2_CodeWarrior();
                                                    break;
                                                        
                                default:            
                                    // We got TARGET_PS2, but no known vendor.
                                    // Assume it is VENDOR_SN.
                                    g_CmdLineData.m_Vendor = VENDOR_SN;
                                    g_CmdLineData.m_Define.Add( "VENDOR_SN" );
                                    strcpy( g_CmdLineData.m_VendorString, "VENDOR_SN" );
                                    Result = Process_PS2_SN();
                                    break;
                                }                                                     
                                break;                                                

    //--------------------------------------------------------------------------
    case BUILD_TARGET_PS2_IOP:  switch( g_CmdLineData.m_Vendor )
                                {                                                     
                                case VENDOR_SN:     Result = Process_PS2_SN();
                                                    break;
                                                        
                                default:            
                                    // We got TARGET_PS2_IOP, but no known vendor.
                                    // Assume it is VENDOR_SN.
                                    g_CmdLineData.m_Vendor = VENDOR_SN;
                                    g_CmdLineData.m_Define.Add( "VENDOR_SN" );
                                    strcpy( g_CmdLineData.m_VendorString, "VENDOR_SN" );
                                    Result = Process_PS2_SN();
                                    break;
                                }                                                     
                                break;                                              
                            
    //--------------------------------------------------------------------------
    case BUILD_TARGET_GCN:      switch( g_CmdLineData.m_Vendor )
                                {                                                     
                                case VENDOR_SN:     Result = Process_GCN_SN();
                                                    break;
                                                        
                                case VENDOR_MW:     Unknown = true;
                                                    break;
                                                        
                                default:            Unknown = true;
                                                    break;
                                }                                                     
                                break; 
                            
    //--------------------------------------------------------------------------
    default:                Unknown = true;
                            break;
    }

    // Unknown target?
    if( Unknown )
    {
        printf( "\n.\n" );
        printf( "xCL -- ERROR: Target/Vendor combination not known: %s/%s\n", 
                    g_CmdLineData.m_TargetString, 
                    g_CmdLineData.m_VendorString );
        printf( "xCL -- ERROR: Processing aborted.\n.\n" );
    }

    // Show the end time?
    if( g_ShowTimes )
    {
       time_t ltime;
       time( &ltime );
       printf( "\\\\\\\n >>>   END %s///\n", ctime( &ltime ) );
    }

    return( Result );
}

//==============================================================================

int ExecuteCMD( const char* pCommand )
{
    SetPriorityClass( GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS );

    return system( pCommand );
/*
    // Get current process info
    STARTUPINFO     isi;
    ZeroMemory( &isi, sizeof(isi) );
    isi.cb = sizeof(isi);
    GetStartupInfo( &isi );

    // Setup the process
    STARTUPINFO             si;
    PROCESS_INFORMATION     pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb           = sizeof(si);
    si.dwFlags      = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.wShowWindow  = SW_HIDE;
    si.hStdError    = isi.hStdError;
    si.hStdInput    = isi.hStdInput;
    si.hStdOutput   = isi.hStdOutput;
    ZeroMemory( &pi, sizeof(pi) );

    // Start the process
    if( !CreateProcess( NULL,                  // LPCTSTR: Module name
                        (LPSTR)pCommand,       // LPTSTR: cmd line
                        NULL,                  // LPSECURITY_ATTRIBUTES: process security
                        NULL,                  // thread security
                        TRUE,                  // BOOL: inherit handles
                        DETACHED_PROCESS | NORMAL_PRIORITY_CLASS, 
                        NULL,                  // LPVOID: environment block
                        NULL,                  // LPCTSTR: current directory
                        &si, 
                        &pi ) )
    {
        // Return an error
        return -1;
    }

    // Wait until child process exits
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Read the exit code
    DWORD ExitCode = -1;
    if( !GetExitCodeProcess( pi.hProcess, &ExitCode ) )
        ExitCode = -1;

    // Close process and thread handles
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

    // Return exit code
    return ExitCode;
*/
}

//==============================================================================
