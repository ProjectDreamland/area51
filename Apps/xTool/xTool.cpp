// xTool.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "xTool.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "xToolDoc.h"
#include "xToolView.h"

#include "x_files.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Globals

debug_connection_list   g_ConnectionList;
CPtrArray               g_Documents;

/////////////////////////////////////////////////////////////////////////////
// Pipe connection thread

UINT ListenThread( LPVOID pParam )
{
    // Listen for connections on our named pipe
    while( 1 )
    {
        HANDLE hPipe = CreateNamedPipe( _T("\\\\.\\pipe\\xlink"),
                                        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                        PIPE_UNLIMITED_INSTANCES,
                                        32768, 32768,
                                        INFINITE,
                                        NULL );

        // Created?
        if( hPipe != INVALID_HANDLE_VALUE )
        {
            // Wait for the client to connect; if it succeeds,
            // the function returns a nonzero value. If the function returns
            // zero, GetLastError returns ERROR_PIPE_CONNECTED.
            BOOL fConnected = ConnectNamedPipe( hPipe, NULL ) ? TRUE : ( GetLastError() == ERROR_PIPE_CONNECTED );

            // Did a connection occur
            if( fConnected )
            {
                // Add this pipe to the queue of pipes that need servicing
                g_ConnectionList.Append( new debug_connection( hPipe ) );

                // Post a Create new document message which should in turn create a comms thread with the next pipe on the list
                CWnd* pMainWnd = AfxGetMainWnd();
                ASSERT( pMainWnd );
                pMainWnd->PostMessage( WM_COMMAND, ID_FILE_NEW );
            }
            else
            {
                // Not connected, so close the pipe handle
                CloseHandle( hPipe );
            }
        }
        else
        {
            // Pipe create failed
            ::MessageBox( NULL, CResString(IDS_ERROR_PIPE_CREATE), CResString(IDS_ERROR), MB_ICONSTOP );
        }
    }

    // Exit thread
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// TCP connection thread

UINT ListenThreadTCP( LPVOID pParam )
{
    SOCKET s;

    // Create socket to listen on
    s = WSASocket( PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
    if( s != SOCKET_ERROR )
    {
        sockaddr_in addr;
        ZeroMemory( &addr, sizeof(addr) );

        addr.sin_family             = PF_INET;
        addr.sin_port               = htons(5120);
        addr.sin_addr.S_un.S_addr   = htonl(ADDR_ANY);

        int Ret = bind( s, (sockaddr*)&addr, sizeof(addr) );
        if( Ret == 0 )
        {
            while( 1 )
            {
                Ret = listen( s, 5 );
                if( Ret != SOCKET_ERROR )
                {
                    sockaddr_in addr;
                    int         addrLen = sizeof(addr);
                    SOCKET NewSocket = WSAAccept( s, (sockaddr*)&addr, &addrLen, NULL, 0 );
                    if( NewSocket != INVALID_SOCKET )
                    {
                        ::MessageBox( NULL, "Connection", "Success", MB_ICONSTOP );

                        // Add this pipe to the queue of pipes that need servicing
                        g_ConnectionList.Append( new debug_connection( (HANDLE)NewSocket ) );

                        // Post a Create new document message which should in turn create a comms thread with the next pipe on the list
                        CWnd* pMainWnd = AfxGetMainWnd();
                        ASSERT( pMainWnd );
                        pMainWnd->PostMessage( WM_COMMAND, ID_FILE_NEW );
                    }
                    else
                    {
                        OutputDebugString( "." );
                    }
                }
                else
                {
                    // Socket create failed
                    ::MessageBox( NULL, CResString(IDS_ERROR_SOCKET_CREATE), CResString(IDS_ERROR), MB_ICONSTOP );
                    break;
                }
            }
        }
        else
        {
            // Socket create failed
            ::MessageBox( NULL, CResString(IDS_ERROR_SOCKET_CREATE), CResString(IDS_ERROR), MB_ICONSTOP );
        }
    }
    else
    {
        // Socket create failed
        ::MessageBox( NULL, CResString(IDS_ERROR_SOCKET_CREATE), CResString(IDS_ERROR), MB_ICONSTOP );
    }

    // Exit thread
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CxToolApp

BEGIN_MESSAGE_MAP(CxToolApp, CWinApp)
	//{{AFX_MSG_MAP(CxToolApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TIPOFTHEDAY, OnUpdateViewTipoftheday)
	ON_COMMAND(ID_VIEW_TIPOFTHEDAY, OnViewTipoftheday)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(IDC_TIPOFTHEDAY, OnTipOfTheDay)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CxToolApp construction

CxToolApp::CxToolApp()
{
    xtAfxData.bXPMode = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CxToolApp object

CxToolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CxToolApp initialization

BOOL CxToolApp::InitInstance()
{
    WSADATA WSAData;

    CoInitialize( NULL );

    if( 0 != WSAStartup( MAKEWORD(2,0), &WSAData ) )
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MSC_VER <= 1200 // MFC 6.0 or earlier
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif // MFC 6.0 or earlier

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("Inevitable"));

	LoadStdProfileSettings(8);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_XTOOLTYPE,
		RUNTIME_CLASS(CxToolDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CxToolView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if( !pMainFrame->LoadFrame(IDR_MAINFRAME) )
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

    // CJ: Removed to prevent creation of an initial document
	// Dispatch commands specified on the command line
//	if (!ProcessShellCommand(cmdInfo))
//		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindowEx(m_nCmdShow);
	pMainFrame->UpdateWindow();

	// This method handles the 'Tip of the Day' component.
	ShowTipAtStartup();

    // Start the Listen thread to listen for incoming connections
    AfxBeginThread( ListenThread   , NULL );
//    AfxBeginThread( ListenThreadTCP, NULL );

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CxToolApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CxToolApp message handlers

void CxToolApp::ShowTipAtStartup()
{
	// This method handles the 'Tip of the Day' component.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_bShowSplash)
	{
		CXTTipOfTheDay dlg( _T("xToolTips.txt") );
		if (dlg.m_bStartup)
			dlg.DoModal();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CxToolApp::OnTipOfTheDay() 
{
	// This method handles the 'Tip of the Day' component.
	CXTTipOfTheDay dlg( _T("xToolTips.txt") );
	dlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////

void CxToolApp::OnUpdateViewTipoftheday(CCmdUI* pCmdUI) 
{
    CXTRegistryManager regManager;
	UINT bShow = regManager.GetProfileInt( _T("Tip"), _T("StartUp"), 0);
	pCmdUI->SetCheck( !bShow );
}

/////////////////////////////////////////////////////////////////////////////

void CxToolApp::OnViewTipoftheday() 
{
    CXTRegistryManager regManager;
	UINT bShow = regManager.GetProfileInt( _T("Tip"), _T("StartUp"), 0);
    regManager.WriteProfileInt( _T("Tip"), _T("StartUp"), !bShow );
}

/////////////////////////////////////////////////////////////////////////////
