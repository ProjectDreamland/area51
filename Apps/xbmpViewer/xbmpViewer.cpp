// xbmpViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "xbmpViewer.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString g_CmdLinePath;
CString g_CmdLineFile;

/////////////////////////////////////////////////////////////////////////////
// CXbmpViewerApp

BEGIN_MESSAGE_MAP(CXbmpViewerApp, CWinApp)
    //{{AFX_MSG_MAP(CXbmpViewerApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXbmpViewerApp construction

CXbmpViewerApp::CXbmpViewerApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CXbmpViewerApp object

CXbmpViewerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CXbmpViewerApp initialization

BOOL CXbmpViewerApp::InitInstance()
{
    AfxEnableControlContainer();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.

//    INITCOMMONCONTROLSEX    iccex;
//    iccex.dwSize = sizeof(iccex);
//    iccex.dwICC  = -1;
//    InitCommonControlsEx( &iccex );

#if _MSC_VER <= 1200 // MFC 6.0 or earlier
#ifdef _AFXDLL
    Enable3dControls();            // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();    // Call this when linking to MFC statically
#endif
#endif // MFC 6.0 or earlier

    // Change the registry key under which our settings are stored.
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization.
    SetRegistryKey(_T("Inevitable"));

    // To create the main window, this code creates a new frame window
    // object and then sets it as the application's main window object.

    // Process command line arguments
    CString CmdLine = m_lpCmdLine;
    if( CmdLine.GetLength() > 0 )
    {
        // Strip quotes
        if( (CmdLine[0] == '\"') && (CmdLine[CmdLine.GetLength()-1] == '\"') )
        {
            CmdLine = CmdLine.Mid( 1, CmdLine.GetLength()-2 );
        }

        char Buffer[1024];
        char* pFileName;
        GetFullPathName( CmdLine, 1024, Buffer, &pFileName );
        CmdLine = Buffer;

        DWORD   Attributes = GetFileAttributes( CmdLine );

        if( Attributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            g_CmdLinePath = CmdLine;
            g_CmdLineFile = "";
        }
        else
        {
            char Drive[_MAX_DRIVE];
            char Dir  [_MAX_DIR  ];
            char FName[_MAX_FNAME];
            char Ext  [_MAX_EXT  ];
            _splitpath( CmdLine, Drive, Dir, FName, Ext );

            g_CmdLinePath  = Drive;
            g_CmdLinePath += Dir;
            g_CmdLineFile  = FName;
            g_CmdLineFile += Ext;
        }
    }

    // create and load the frame with its resources
    CMainFrame* pMainFrame = new CMainFrame;
    m_pMainWnd = pMainFrame;

    pMainFrame->LoadFrame(IDR_MAINFRAME,
        WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
        NULL);

    // The one and only window has been initialized, so show and update it.
    pMainFrame->ShowWindowEx(m_nCmdShow);
    pMainFrame->UpdateWindow();

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CXbmpViewerApp message handlers





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
void CXbmpViewerApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CXbmpViewerApp message handlers

