// ReadPipe.cpp : implementation file
//

#include "stdafx.h"
#include "editor.h"
#include "ReadPipe.h"
#include "..\EDRscDesc\RSCDesc.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DEFINES

#define MAX_CMD_LINE_LEN    ( 1024*10 )     // Max DOS command line length


/////////////////////////////////////////////////////////////////////////////
// CReadPipe

IMPLEMENT_DYNCREATE(CReadPipe, CWinThread)


BEGIN_MESSAGE_MAP(CReadPipe, CWinThread)
	//{{AFX_MSG_MAP(CReadPipe)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VARS
/////////////////////////////////////////////////////////////////////////////

s32                 CReadPipe::s_nPipes = 0;
CRITICAL_SECTION    CReadPipe::s_CriticalSec;

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

CReadPipe::CReadPipe( CRichEditCtrl& Output, CProgressCtrl& MainProgress, 
                      CProgressCtrl& SubProgress,  xbool bVerbose, const char* pTempDir ) : 
    m_pOutput       ( &Output ), 
    m_pMainProgress ( &MainProgress ), 
    m_pSubProgress  ( &SubProgress ), 
    m_bVerbose      ( bVerbose ),
    m_pxThread      ( NULL )
{
    EnterCriticalSection( &s_CriticalSec );
    s_nPipes++;
    LeaveCriticalSection( &s_CriticalSec );
    m_TempDir = pTempDir;
};

//===========================================================================

CReadPipe::CReadPipe()
{
    // Must call the other contructor
    ASSERT( 0 );
}

//===========================================================================

CReadPipe::~CReadPipe()
{
    EnterCriticalSection( &s_CriticalSec );

    s_nPipes--;
    if( s_nPipes == 0 )
    {
        if( g_RescDescMGR.IsStopBuild() == FALSE )
        {
            m_pMainProgress->SetRange32 ( 0, g_RescDescMGR.GetRscDescCount() );
            m_pMainProgress->SetPos     ( g_RescDescMGR.GetRscDescCount() );
        }
        g_RescDescMGR.EndCompiling();

        CTime t = CTime::GetCurrentTime();
        CString s = t.Format( "%H:%M:%S %A, %B %d, %Y" );

        s = "End of compilation: [" + s + "]\n";
        OutputString( s );
    }
    
    // Clear up the x_files stuff
    if( m_pxThread )
        delete m_pxThread;

    LeaveCriticalSection( &s_CriticalSec );
}

//===========================================================================

BOOL CReadPipe::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

//===========================================================================

int CReadPipe::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

//===========================================================================

void CReadPipe::GlobalInit( void )  
{
    static xbool bInit = FALSE;
    if( bInit == FALSE )
    {
        bInit = TRUE;
        InitializeCriticalSection( &s_CriticalSec );
    }
}

//===========================================================================

int CReadPipe::Run() 
{
    //SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
    //
    // Create a happy x files thread
    //
    m_pxThread = new xthread(4096,"CReadPipe::Run");
    if( m_pxThread == NULL )
        return ExitInstance();

    xfs("dsfdfds");

    HANDLE                  hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES     sa;

    sa.nLength                 = sizeof(sa);
    sa.bInheritHandle          = TRUE;
    sa.lpSecurityDescriptor    = NULL;

    if(!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) 
    {
        //TOMAS: m_pDlg->MessageBox("Failed to create pipe!");
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



// Since we've redirected the standard input, output and error handles
// of the child process, we create it without a console of its own.
// (That's the `DETACHED_PROCESS' part of the call.)  Other
// possibilities include passing 0 so the child inherits our console,
// or passing CREATE_NEW_CONSOLE so the child gets a console of its
// own.
//


    //
    // Loop here in the future to read the next compiler "GetNextIndex"
    //
    s32 Index;
    xstring  CmdLine;
    xstring  Output;
    while( (Index = GetNextIndex( (const char*)m_TempDir, CmdLine, Output )) != -1 )
    {
        char        buffer[MAX_CMD_LINE_LEN+1] ;
        DWORD       dwRead=0;
        s32         Line = 0;
        BOOL        ret;

        m_FmtData.Clear();
        if( m_bVerbose )
        {
            ProcessString( CmdLine );
            ProcessString( "\n"    );
        }

        ProcessString( Output  );
        m_pMainProgress->SetRange32( 0, g_RescDescMGR.GetRscDescCount() );
        m_pMainProgress->SetPos( Index );

        ret = CreateProcess( NULL,                  // LPCTSTR: Module name
                             &CmdLine[0],           // @script_256.txt",        // LPTSTR: cmd line
                             NULL,                  // LPSECURITY_ATTRIBUTES: process security
                             NULL,                  // thread security
                             TRUE,                  // BOOL: inherit handles
                             DETACHED_PROCESS | BELOW_NORMAL_PRIORITY_CLASS, 
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

            sprintf( buffer, "Error executing the command line. %s\n[%s]\n", lpMsgBuf, &CmdLine[0] );
            OutputString( buffer );

            // Clean the pipe
            CloseHandle( hWritePipe );
            CloseHandle( hReadPipe  ); 

            // TODO: may want to stop the compilation process
            return ExitInstance();
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
                ProcessString( buffer );
            }
            else     
            {
                // no we don't have anything in the buffer
                // maybe the program exited
                if(WaitForSingleObject(pi.hProcess,0) == WAIT_OBJECT_0)
                    break;        // so we should exit either

                Sleep(2);
                if( g_RescDescMGR.IsStopBuild() )
                   bAbort = TRUE;
             }

            // continue otherwise
        }

        // Output the data
        OutputString( m_FmtData );
        
        // If the user abourted then lets just quit
        if( bAbort )
            break;
    }

    //
    // Notify the user if he cancel the operation
    //
    if( g_RescDescMGR.IsStopBuild() )
    {
        OutputString( "*************** Abort by user ***************\n" );
    }

    // Since we don't need the handle to the child's thread, close it to
    // save some resources.
    CloseHandle( pi.hThread  );
    CloseHandle( pi.hProcess );

    // Close the pipe when done
    CloseHandle( hWritePipe );
    CloseHandle( hReadPipe  ); 

    // Done with the thread
    return ExitInstance();
}

//===========================================================================

int CReadPipe::RunDirectly( void )
{
    xfs("dsfdfds");

    HANDLE                  hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES     sa;

    sa.nLength                 = sizeof(sa);
    sa.bInheritHandle          = TRUE;
    sa.lpSecurityDescriptor    = NULL;

    if(!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) 
    {
        //TOMAS: m_pDlg->MessageBox("Failed to create pipe!");
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



    // Since we've redirected the standard input, output and error handles
    // of the child process, we create it without a console of its own.
    // (That's the `DETACHED_PROCESS' part of the call.)  Other
    // possibilities include passing 0 so the child inherits our console,
    // or passing CREATE_NEW_CONSOLE so the child gets a console of its
    // own.
    //


    //
    // Loop here in the future to read the next compiler "GetNextIndex"
    //
    s32 Index;
    xstring  CmdLine;
    xstring  Output;
    while( (Index = GetNextIndex( (const char*)m_TempDir, CmdLine, Output )) != -1 )
    {
        char        buffer[MAX_CMD_LINE_LEN+1] ;
        DWORD       dwRead=0;
        s32         Line = 0;
        BOOL        ret;

        m_FmtData.Clear();
        if( m_bVerbose )
        {
            ProcessString( CmdLine );
            ProcessString( "\n"    );
        }

        ProcessString( Output  );
        m_pMainProgress->SetRange32( 0, g_RescDescMGR.GetRscDescCount() );
        m_pMainProgress->SetPos( Index );

        ret = CreateProcess( NULL,                  // LPCTSTR: Module name
            &CmdLine[0],           // @script_256.txt",        // LPTSTR: cmd line
            NULL,                  // LPSECURITY_ATTRIBUTES: process security
            NULL,                  // thread security
            TRUE,                  // BOOL: inherit handles
            DETACHED_PROCESS | BELOW_NORMAL_PRIORITY_CLASS, 
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

            sprintf( buffer, "Error executing the command line. %s\n[%s]\n", lpMsgBuf, &CmdLine[0] );
            OutputString( buffer );

            // Clean the pipe
            CloseHandle( hWritePipe );
            CloseHandle( hReadPipe  ); 

            // TODO: may want to stop the compilation process
            return 0;
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
                ProcessString( buffer );
            }
            else     
            {
                // no we don't have anything in the buffer
                // maybe the program exited
                if(WaitForSingleObject(pi.hProcess,0) == WAIT_OBJECT_0)
                    break;        // so we should exit either

                Sleep(2);
                if( g_RescDescMGR.IsStopBuild() )
                    bAbort = TRUE;
            }

            // continue otherwise
        }

        // Output the data
        OutputString( m_FmtData );

        // If the user abourted then lets just quit
        if( bAbort )
            break;
    }

    //
    // Notify the user if he cancel the operation
    //
    if( g_RescDescMGR.IsStopBuild() )
    {
        OutputString( "*************** Abort by user ***************\n" );
    }

    // Since we don't need the handle to the child's thread, close it to
    // save some resources.
    CloseHandle( pi.hThread  );
    CloseHandle( pi.hProcess );

    // Close the pipe when done
    CloseHandle( hWritePipe );
    CloseHandle( hReadPipe  ); 

    // Done with compiling
    return 0;
}

//===========================================================================

s32 CReadPipe::GetNextIndex( const char* pTempDir, xstring& CmdLine, xstring& Output )
{
    EnterCriticalSection( &s_CriticalSec );

    s32 Index = -1;
    if( g_RescDescMGR.IsStopBuild() == FALSE )
    {
        // Lookup command line
        Index = g_RescDescMGR.NextCompiling();
        g_RescDescMGR.GetMakeRules( Index, CmdLine, Output );

        // Lookup resource name
        xstring Resource;
        if( Index != -1 )
        {
            rsc_desc_mgr::node& Node = g_RescDescMGR.GetRscDescIndex( Index );
            if( Node.pDesc )
            {
                Resource = Node.pDesc->GetName();
                Resource.MakeLower();
            }
        }
                        
        // SB - CRASH FIX - 
        // Always use script file for .anim, .rigidgeom, and .skingeom resources so
        // that if they crash the compilers, I can copy them to my machine and debug
        // Also - if the command line is too big then use a script file otherwise
        // DOS simply truncates the command line and this will crash the compilers.
        if(     ( CmdLine.GetLength() >= MAX_CMD_LINE_LEN )
             || ( Resource.Find( ".anim"      ) != -1 )
             || ( Resource.Find( ".rigidgeom" ) != -1 )
             || ( Resource.Find( ".skingeom"  ) != -1 ) )
        {
            // Make sure there are no spaces in the temp directory!
            ASSERTS( x_strstr( pTempDir, " " ) == FALSE, xfs( "Fatal Error: Temp directory '%s' must not contain spaces!", pTempDir ) );
            
            // Remove spaces from resource name
            s32 iSpace = Resource.Find( ' ' );
            while( iSpace != -1 )
            {
                Resource.Delete( iSpace, 1 );
                iSpace = Resource.Find( ' ' );
            }                
        
            // Create unique script file name from resource name and resource index
            xstring IndexString;
            IndexString.Format( "%d", Index );
            xstring ScriptFile = xstring( pTempDir ) + "\\" + Resource + "." + IndexString + ".txt";
            
            // Find the first space so we can isolate the .exe command
            s32 FirstSpace = CmdLine.Find(' ') ;
            ASSERT(FirstSpace != -1) ;

            // Just incase no asserts are defined
            if (FirstSpace == -1)
                FirstSpace = 0 ;

            // Write out the command line parameters as a script file
            X_FILE* pFile ;
            pFile = x_fopen( (const char*)ScriptFile, "wt") ;
            if (pFile)
            {
                // Write out script file and make sure to flush it ready for the compilers to read!
                x_fwrite( &CmdLine[FirstSpace], 1, CmdLine.GetLength() - FirstSpace, pFile );
                x_fflush( pFile );
                x_fclose( pFile ) ;
            }
            else
            {
                ASSERTS( 0, xfs( "Fatal Error: Looks like temp files in '%s' are write protected", pTempDir ) );
            }

#ifdef X_DEBUG
            OutputDebugString(CmdLine) ;
#endif

            // Start with just the exe command
            CmdLine = CmdLine.Left(FirstSpace) ;

            // Now add the response file as a parameter
            CmdLine += xstring( " @" ) + ScriptFile;
        }
    }

    LeaveCriticalSection( &s_CriticalSec );

    return Index;
}

//===========================================================================

void CReadPipe::OutputString( const char* pString )
{
    EnterCriticalSection( &s_CriticalSec );

    m_pOutput->SetSel( -1, -1 );
    m_pOutput->ReplaceSel( pString, FALSE );

    // Determine the number of visible lines in the control
    CRect r;
    m_pOutput->GetRect( &r );
    s32 Height      = r.Height();
    s32 nVisible    = 0;
    s32 i           = m_pOutput->GetFirstVisibleLine();
    s32 nLines      = m_pOutput->GetLineCount();
    while( (i < nLines) && (m_pOutput->GetCharPos( m_pOutput->LineIndex( i ) ).y < Height) )
    {
        nVisible++;
        i++;
    }

    // Scroll the last line into view
    s32 LineCount  = m_pOutput->GetLineCount() - nVisible;
    s32 FirstVisible = m_pOutput->GetFirstVisibleLine();
    s32 Delta = LineCount - FirstVisible;
    m_pOutput->LineScroll( Delta );

    ParseErrorsToLog();

    LeaveCriticalSection( &s_CriticalSec );
}

//===========================================================================

void CReadPipe::ProcessString( const char* pString )
{
    m_FmtData += pString;
}

//===========================================================================

void CReadPipe::ParseErrorsToLog( void )
{
    //Pipe Compile to Log Errors Here

    char CompileErrorFlag[]="ERROR:";
    xstring ErrorOutPut;
    s32 nError = 0, EndOfLine = 0;
    CString StrErrors;
    s32 DelimterLength = strlen(CompileErrorFlag); 

    while(nError !=-1)
    {
        nError = m_FmtData.Find(CompileErrorFlag, nError);
        if(nError !=-1)
        {
            EndOfLine = m_FmtData.Find("\n", nError);
            if(EndOfLine !=-1 )
            {
                ErrorOutPut = m_FmtData.Mid(nError + DelimterLength, EndOfLine - nError - DelimterLength -1);
                nError = EndOfLine;

                log_LOCK();
                log_ERROR( "Compile Error", ErrorOutPut);
                LOG_FLUSH();

            }
        }  
    }
}

