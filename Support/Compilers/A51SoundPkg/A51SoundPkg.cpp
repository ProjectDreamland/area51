// A51SoundPkg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Auxiliary\CommandLine\CommandLine.hpp"
#include "x_threads.hpp"

//=========================================================================
// GLOBALS
//=========================================================================
#define MAX_PLATFORM        3
#define MAX_CMD_LINE_LEN    (1024*10)

enum{
    EXPORT_XBOX,
    EXPORT_PS2,
    EXPORT_PC,
};

//=========================================================================
// MAIN
//=========================================================================
int main(int argc, char* argv[])
{
	
    command_line    CommandLine;
    
    CommandLine.AddOptionDef( "TEMPPATH",   command_line::STRING );
    CommandLine.AddOptionDef( "XBOX",       command_line::STRING );
    CommandLine.AddOptionDef( "PS2",        command_line::STRING );
    CommandLine.AddOptionDef( "PC",         command_line::STRING );
    CommandLine.AddOptionDef( "CMDLINE",    command_line::SWITCH );

    CommandLine.Parse( argc, argv );

    xstring TempPath;
    xstring Release[ MAX_PLATFORM ];
    s32     iPath[ MAX_PLATFORM ]   = {-1};
    xbool   bCmdLine                 = FALSE;

    s32 TPath            = CommandLine.FindOption( xstring("TEMPPATH") );
    iPath[ EXPORT_XBOX ] = CommandLine.FindOption( xstring("XBOX") );
    iPath[ EXPORT_PS2 ]  = CommandLine.FindOption( xstring("PS2") );
    iPath[ EXPORT_PC ]   = CommandLine.FindOption( xstring("PC") );
    
    if( CommandLine.FindOption( xstring("CMDLINE") ) != -1 )
        bCmdLine = TRUE;                                  
    
    if( TPath != -1 )
        TempPath    = CommandLine.GetOptionString( TPath );

    //
    // Setup the read pipe.
    //
    //----------------------------------------------------------------------------------------------
    HANDLE                  hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES     sa;

    sa.nLength                 = sizeof(sa);
    sa.bInheritHandle          = TRUE;
    sa.lpSecurityDescriptor    = NULL;

    if(!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) 
    {
        x_printf( "Failed to create a pipe\n" );
        return -1;
    }

    // CReadPipe will auto-delete itself which includes closing the handle.
    STARTUPINFO             si;
    PROCESS_INFORMATION     pi;

    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    si.cb              = sizeof(STARTUPINFO);
    si.dwFlags         = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.hStdInput	   = INVALID_HANDLE_VALUE;
    si.hStdOutput      = hWritePipe;
    si.hStdError       = hWritePipe;
    si.wShowWindow     = SW_HIDE;
    //----------------------------------------------------------------------------------------------

    for( s32 i = 0; i < MAX_PLATFORM; i++ )
    {
        if( iPath[ i ] != -1 )
        {
            Release[ i ] = CommandLine.GetOptionString( iPath[i] );

            xstring TempFile;
            X_FILE* pFile;

            //x_printf("Processing command line script [A51SoundPkg]");

            TempFile = (const char*)xfs("%s\\Temp_%d.txt", TempPath, (x_GetCurrentThread()->GetSystemId() + i));
            pFile = x_fopen( TempFile, "wt" );

            if (!pFile)
            {
                x_printf("ERROR: Unable to create temporary file\n");
                x_printf("[%s]", TempFile );
                continue;
            }

            for( s32 j = 0; j < CommandLine.GetNumArguments(); j++ )
            { 
                const xstring& Storage = CommandLine.GetArgument( j );
                x_fwrite( Storage, sizeof(char) ,Storage.GetLength(), pFile );
                x_fwrite( "\n", sizeof(char) ,1, pFile );
            }

            xstring OptName( CommandLine.GetOptionName( iPath[i] ) );
            OptName = " -" + OptName;
            //x_fwrite( OptName, sizeof(char), OptName.GetLength(), pFile );

            //Release[i] = " \"" + Release[i] + "\"";
            Release[i] = " " + Release[i];
            x_fwrite( Release[i], sizeof(char), Release[i].GetLength(), pFile );

            x_fclose( pFile );

            // The name of the sound packager is the first argument.
            xstring CmdLine( "SoundPackager.exe" );//CommandLine.GetArgument( 0 ) );

            CmdLine += " " + OptName + " " + TempFile;
            
            // Create the process.
            //----------------------------------------------------------------------------------------------
            char        buffer[MAX_CMD_LINE_LEN];
            DWORD       dwRead=0;
            s32         Line = 0;
            BOOL        ret;

                ret = CreateProcess( NULL,                  // LPCTSTR: Module name
                                     &CmdLine[0],           // @script_256.txt",        // LPTSTR: cmd line
                                     NULL,                  // LPSECURITY_ATTRIBUTES: process security
                                     NULL,                  // thread security
                                     TRUE,                  // BOOL: inherit handles
                                     DETACHED_PROCESS | NORMAL_PRIORITY_CLASS, 
                                     NULL,                  // LPVOID: environment block
                                     NULL,                  // LPCTSTR: current directory
                                     &si, 
                                     &pi );


            if( FALSE == ret ) 
            {
                LPVOID lpMsgBuf;
                FormatMessage( 
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                    FORMAT_MESSAGE_FROM_SYSTEM | 
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL 
                );

                // Catch any buffer overruns!
                ASSERT((CmdLine.GetLength()+1) < sizeof(buffer)) ;

                x_printf( "Error executing the command line. %s\n[%s]\n", lpMsgBuf, &CmdLine[0] );

                // TODO: may want to stop the compilation process
                continue;
            }

            xbool bAbort = FALSE;
            while( bAbort == FALSE )
            { 
                dwRead = 0;
                if( !PeekNamedPipe( hReadPipe, NULL, 0, NULL, &dwRead, NULL) )   
                    break;

                if( dwRead )
                {                 // yes we do, so read it and print out to the edit ctrl
                    if( !ReadFile(hReadPipe, &buffer, sizeof(buffer)-1, &dwRead, NULL) )
                            break;

                    buffer[dwRead] = 0;
                    x_printf( "%s", buffer );
                }
                else     
                {
                    // no we don't have anything in the buffer
                    // maybe the program exited
                    if(WaitForSingleObject(pi.hProcess,0) == WAIT_OBJECT_0)
                        break;        // so we should exit either

                    Sleep(500);
                 }

                // continue otherwise
            }
            //----------------------------------------------------------------------------------------------
        }
    }

    // Clean the pipe
    CloseHandle( hWritePipe );
    CloseHandle( hReadPipe  ); 

	return 0;
}

